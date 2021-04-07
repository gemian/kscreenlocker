/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

Copyright (C) 2015 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "globalaccel.h"

#include <KKeyServer>
#include <netwm.h>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QX11Info>
#include <QDebug>
#include <QDBusMetaType>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xkbcommon/xkbcommon.h>
#include <X11/keysym.h>

static const QString s_kglobalAccelService = QStringLiteral("org.kde.kglobalaccel");
static const QString s_componentInterface = QStringLiteral("org.kde.kglobalaccel.Component");

static const QString s_networkManagerDestination = QStringLiteral("org.freedesktop.NetworkManager");
static const QString s_networkManagerPath = QStringLiteral("/org/freedesktop/NetworkManager");
static const QString s_networkManagerInterface = QStringLiteral("org.freedesktop.NetworkManager");
static const QString s_bluezDestination = QStringLiteral("org.bluez");
static const QString s_bluezHci0Path = QStringLiteral("/org/bluez/hci0");
static const QString s_bluezAdapter1Interface = QStringLiteral("org.bluez.Adapter1");
static const QString s_propertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");
static const QString s_getMethod = QStringLiteral("Get");
static const QString s_setMethod = QStringLiteral("Set");
static const QString s_wirelessEnabledProperty = QStringLiteral("WirelessEnabled");
static const QString s_poweredProperty = QStringLiteral("Powered");
static const QString s_ofonoDestination = QStringLiteral("org.ofono");
static const QString s_ofonoPath = QStringLiteral("/ril_0");
static const QString s_ofonoModemInterface = QStringLiteral("org.ofono.Modem");
static const QString s_getPropertiesMethod = QStringLiteral("GetProperties");
static const QString s_setPropertyMethod = QStringLiteral("SetProperty");

Q_DECLARE_METATYPE(QList<KGlobalShortcutInfo>)

/**
 * Whitelist of the components which are allowed to get global shortcuts.
 * The DBus path of the component is the key for the whitelist.
 * The value for each key contains a regular expression matching unique shortcut names which are allowed.
 * This allows to not only restrict on component, but also restrict on the shortcuts.
 * E.g. plasmashell might accept media shortcuts, but not shortcuts for switching the activity.
 **/
static const QMap<QString, QRegularExpression> s_shortcutWhitelist{
    {QStringLiteral("/component/mediacontrol"), QRegularExpression(
        QStringLiteral("stopmedia|nextmedia|previousmedia|playpausemedia")
    )},
    {QStringLiteral("/component/kmix"), QRegularExpression(
        QStringLiteral("mute|decrease_volume|increase_volume")
    )},
    {QStringLiteral("/component/org_kde_powerdevil"), QRegularExpression(
        QStringLiteral("Increase Screen Brightness|Decrease Screen Brightness|Increase Keyboard Brightness|Decrease Keyboard Brightness")
    )},
    {QStringLiteral("/component/KDE_Keyboard_Layout_Switcher"), QRegularExpression(
        QStringLiteral("Switch to Next Keyboard Layout|Switch keyboard layout to .*")
    )},
    {QStringLiteral("/component/kcm_touchpad"), QRegularExpression(
        QStringLiteral("Toggle Touchpad|Enable Touchpad|Disable Touchpad")
    )}
};

static uint g_keyModMaskXAccel = 0;
static uint g_keyModMaskXOnOrOff = 0;

static void calculateGrabMasks()
{
    g_keyModMaskXAccel = KKeyServer::accelModMaskX();
    g_keyModMaskXOnOrOff =
            KKeyServer::modXLock() |
            KKeyServer::modXNumLock() |
            KKeyServer::modXScrollLock() |
            KKeyServer::modXModeSwitch();
}

GlobalAccel::GlobalAccel(QObject *parent)
    : QObject(parent)
{
}

void GlobalAccel::prepare()
{
    qDBusRegisterMetaType<QList<KGlobalShortcutInfo>>();
    qDBusRegisterMetaType<KGlobalShortcutInfo>();

    // recursion check
    if (m_updatingInformation) {
        return;
    }
    // first ensure that we don't have some left over
    release();

    if (QX11Info::isPlatformX11()) {
        m_keySymbols = xcb_key_symbols_alloc(QX11Info::connection());
        calculateGrabMasks();
    }

    // fetch all compnents from KGlobalAccel
    m_updatingInformation++;
    auto message = QDBusMessage::createMethodCall(s_kglobalAccelService,
                                                  QStringLiteral("/kglobalaccel"),
                                                  QStringLiteral("org.kde.KGlobalAccel"),
                                                  QStringLiteral("allComponents"));
    QDBusPendingReply<QList<QDBusObjectPath>> async = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(async, this);
    connect(callWatcher, &QDBusPendingCallWatcher::finished, this, &GlobalAccel::components);
}

void GlobalAccel::components(QDBusPendingCallWatcher *self)
{
    QDBusPendingReply<QList<QDBusObjectPath>> reply = *self;
    self->deleteLater();
    if (!reply.isValid()) {
        m_updatingInformation--;
        return;
    }
    // go through all components, check whether they are in our whitelist
    // if they are whitelisted we check whether they are active
    for (const auto &path : reply.value()) {
        const QString objectPath = path.path();
        bool whitelisted = false;
        for (auto it = s_shortcutWhitelist.begin(); it != s_shortcutWhitelist.end(); ++it) {
            if (objectPath == it.key()) {
                whitelisted = true;
                break;
            }
        }
        if (!whitelisted) {
            continue;
        }
        auto message = QDBusMessage::createMethodCall(s_kglobalAccelService,
                                                      objectPath,
                                                      s_componentInterface,
                                                      QStringLiteral("isActive"));
        QDBusPendingReply<bool> async = QDBusConnection::sessionBus().asyncCall(message);
        QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(async, this);
        m_updatingInformation++;
        connect(callWatcher, &QDBusPendingCallWatcher::finished, this,
            [this, objectPath] (QDBusPendingCallWatcher *self) {
                QDBusPendingReply<bool> reply = *self;
                self->deleteLater();
                // filter out inactive components
                if (!reply.isValid() || !reply.value()) {
                    m_updatingInformation--;
                    return;
                }

                // active, whitelisted component: get all shortcuts
                auto message = QDBusMessage::createMethodCall(s_kglobalAccelService,
                                                              objectPath,
                                                              s_componentInterface,
                                                              QStringLiteral("allShortcutInfos"));
                QDBusPendingReply<QList<KGlobalShortcutInfo>> async = QDBusConnection::sessionBus().asyncCall(message);
                QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(async, this);
                connect(callWatcher, &QDBusPendingCallWatcher::finished, this,
                    [this, objectPath] (QDBusPendingCallWatcher *self) {
                        m_updatingInformation--;
                        QDBusPendingReply<QList<KGlobalShortcutInfo>> reply = *self;
                        self->deleteLater();
                        if (!reply.isValid()) {
                            return;
                        }
                        // restrict to whitelist
                        QList<KGlobalShortcutInfo> infos;
                        auto whitelist = s_shortcutWhitelist.constFind(objectPath);
                        if (whitelist == s_shortcutWhitelist.constEnd()) {
                            // this should not happen, just for safety
                            return;
                        }
                        const auto s = reply.value();
                        for (auto it = s.begin(); it != s.end(); ++it) {
                            auto matches = whitelist.value().match((*it).uniqueName());
                            if (matches.hasMatch()) {
                                infos.append(*it);
                            }
                        }
                        m_shortcuts.insert(objectPath, infos);
                    }
                );
            }
        );
    }
    m_updatingInformation--;
}

void GlobalAccel::release()
{
    m_shortcuts.clear();
    if (m_keySymbols) {
        xcb_key_symbols_free(m_keySymbols);
        m_keySymbols = nullptr;
    }
}

bool GlobalAccel::keyEvent(QKeyEvent *event)
{
    const int keyCodeQt = event->key();
    Qt::KeyboardModifiers keyModQt = event->modifiers();

    if (keyModQt & Qt::SHIFT && !(keyModQt & Qt::META ||KKeyServer::isShiftAsModifierAllowed(keyCodeQt))) {
        keyModQt &= ~Qt::SHIFT;
    }

    if ((keyModQt == 0 || keyModQt == Qt::SHIFT) && (keyCodeQt >= Qt::Key_Space && keyCodeQt <= Qt::Key_AsciiTilde)) {
        // security check: we don't allow shortcuts without modifier for "normal" keys
        // this is to prevent a malicious application to grab shortcuts for all keys
        // and by that being able to read out the keyboard
        return false;
    }

    const QKeySequence seq(keyCodeQt | keyModQt);
    // let's check whether we have a mapping shortcut
    for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
        for (const auto &info : it.value()) {
            if (info.keys().contains(seq)) {
                auto signal = QDBusMessage::createMethodCall(s_kglobalAccelService, it.key(), s_componentInterface, QStringLiteral("invokeShortcut"));
                signal.setArguments(QList<QVariant>{QVariant(info.uniqueName())});
                QDBusConnection::sessionBus().asyncCall(signal);
                return true;
            }
        }
    }
    return false;
}

bool GlobalAccel::checkKeyPress(xcb_key_press_event_t *event)
{
    if (!m_keySymbols) {
        return false;
    }

    int keyQt = 0;
    KKeyServer::xcbKeyPressEventToQt(m_keySymbols,event,&keyQt);

    if ((keyQt == 0 || keyQt == Qt::SHIFT) && (keyQt >= Qt::Key_Space && keyQt <= Qt::Key_AsciiTilde)) {
        // security check: we don't allow shortcuts without modifier for "normal" keys
        // this is to prevent a malicious application to grab shortcuts for all keys
        // and by that being able to read out the keyboard
        return false;
    }

    const QKeySequence seq(keyQt);
    // let's check whether we have a mapping shortcut
    for (auto it = m_shortcuts.begin(); it != m_shortcuts.end(); ++it) {
        for (const auto &info : it.value()) {
            bool match=false;
            if (info.keys().count() == 1) {
                int shortcutKeySymX;
                KKeyServer::keyQtToSymX(info.keys()[0][0], &shortcutKeySymX);
                match = (shortcutKeySymX == keyQt);
            }
            if (match || info.keys().contains(seq)) {
                auto signal = QDBusMessage::createMethodCall(s_kglobalAccelService, it.key(), s_componentInterface, QStringLiteral("invokeShortcut"));
                signal.setArguments(QList<QVariant>{QVariant(info.uniqueName())});
                QDBusConnection::sessionBus().asyncCall(signal);
                return true;
            }
        }
    }
    return false;
}

void GlobalAccel::handleFunctionKeys(xcb_key_press_event_t *event) {
    if (!m_keySymbols) {
        return;
    }
    bool ctrl = event->state & KKeyServer::modXCtrl();
    xcb_keysym_t keySymX = xcb_key_press_lookup_keysym(m_keySymbols, event, 0);

    qInfo() << keySymX;

    switch (keySymX) {
        case XKB_KEY_t:
            if (ctrl) {
                qDebug() << "C-T pressed\n";
//                ToggleTorchState();
                return;
            }
            break;
        case XKB_KEY_c:
            if (ctrl) {
                qDebug() << "C-C pressed\n";
                ToggleCellularState();
                return;
            }
            break;
        case XKB_KEY_b:
            if (ctrl) {
                qDebug() << "C-B pressed\n";
                ToggleBluetoothState();
                return;
            }
            break;
        case XKB_KEY_w:
            if (ctrl) {
                qDebug() << "C-W pressed\n";
                ToggleWirelessState();
                return;
            }
            break;
    }
}

void GlobalAccel::ToggleWirelessState() {
    //dbus-send --system --print-reply --dest=org.freedesktop.NetworkManager /org/freedesktop/NetworkManager org.freedesktop.DBus.Properties.Get string:"org.freedesktop.NetworkManager" string:"WirelessEnabled"
    //dbus-send --system --print-reply --dest=org.freedesktop.NetworkManager /org/freedesktop/NetworkManager org.freedesktop.DBus.Properties.Set string:"org.freedesktop.NetworkManager" string:"WirelessEnabled" variant:boolean:false

    QDBusMessage message = QDBusMessage::createMethodCall(s_networkManagerDestination,
                                                          s_networkManagerPath,
                                                          s_propertiesInterface,
                                                          s_getMethod);
    message << s_networkManagerInterface;
    message << s_wirelessEnabledProperty;
    QDBusPendingReply<QVariant> async = QDBusConnection::systemBus().asyncCall(message);
    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(async, this);
    connect(callWatcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *self) {
                QDBusPendingReply<QVariant> reply = *self;
                self->deleteLater();
                if (!reply.isValid()) {
                    if (reply.isError()) {
                        qInfo() << reply.error().message();
                    }
                    return;
                }
                QDBusMessage m = QDBusMessage::createMethodCall(s_networkManagerDestination,
                                                                s_networkManagerPath,
                                                                s_propertiesInterface,
                                                                s_setMethod);
                m << s_networkManagerInterface;
                m << s_wirelessEnabledProperty;
                m << QVariant::fromValue(QDBusVariant(!reply.value().toBool()));
                QDBusConnection::systemBus().asyncCall(m);
            }
    );
}

void GlobalAccel::ToggleBluetoothState() {
    //dbus-send --system --dest=org.bluez --print-reply /org/bluez/hci0 org.freedesktop.DBus.Properties.Get string:org.bluez.Adapter1 string:Powered
    //dbus-send --system --dest=org.bluez --print-reply /org/bluez/hci0 org.freedesktop.DBus.Properties.Set string:org.bluez.Adapter1 string:Powered variant:boolean:false

    QDBusMessage message = QDBusMessage::createMethodCall(s_bluezDestination,
                                                          s_bluezHci0Path,
                                                          s_propertiesInterface,
                                                          s_getMethod);
    message << s_bluezAdapter1Interface;
    message << s_poweredProperty;
    QDBusPendingReply<QVariant> async = QDBusConnection::systemBus().asyncCall(message);
    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(async, this);
    connect(callWatcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *self) {
                QDBusPendingReply<QVariant> reply = *self;
                self->deleteLater();
                if (!reply.isValid()) {
                    if (reply.isError()) {
                        qInfo() << reply.error().message();
                    }
                    return;
                }
                QDBusMessage m = QDBusMessage::createMethodCall(s_bluezDestination,
                                                                s_bluezHci0Path,
                                                                s_propertiesInterface,
                                                                s_setMethod);
                m << s_bluezAdapter1Interface;
                m << s_poweredProperty;
                m << QVariant::fromValue(QDBusVariant(!reply.value().toBool()));
                QDBusConnection::systemBus().asyncCall(m);
            }
    );
}

void GlobalAccel::ToggleCellularState() {
    //Get current state
    //dbus-send --system --dest=org.ofono --print-reply /ril_0 org.ofono.Modem.GetProperties
    //Set
    //dbus-send --system --dest=org.ofono --print-reply /ril_0 org.ofono.Modem.SetProperty string:Powered variant:boolean:true

    QDBusMessage message = QDBusMessage::createMethodCall(s_ofonoDestination,
                                                          s_ofonoPath,
                                                          s_ofonoModemInterface,
                                                          s_getPropertiesMethod);

    QDBusPendingReply<QVariantMap> async = QDBusConnection::systemBus().asyncCall(message);
    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(async, this);
    connect(callWatcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *self) {
                QDBusPendingReply<QVariantMap> reply = *self;  // QVariantMap = a{sv}
                self->deleteLater();
                if (!reply.isValid()) {
                    if (reply.isError()) {
                        qInfo() << reply.error().message();
                    }
                    return;
                }
                auto map = reply.value().toStdMap();
                if (map[s_poweredProperty].isValid()) {
                    QVariant powered = map[s_poweredProperty];
                    QDBusMessage m = QDBusMessage::createMethodCall(s_ofonoDestination,
                                                                    s_ofonoPath,
                                                                    s_ofonoModemInterface,
                                                                    s_setPropertyMethod);
                    m << s_poweredProperty;
                    m << QVariant::fromValue(QDBusVariant(!powered.toBool()));
                    QDBusConnection::systemBus().asyncCall(m);
                }
            }
    );

}
/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*********************************************************************/
#ifndef KSCREENLOCKER_WALLPAPER_INTEGRATION_H
#define KSCREENLOCKER_WALLPAPER_INTEGRATION_H

#include <KPackage/Package>
#include <KSharedConfig>

class KConfigLoader;

namespace KDeclarative
{
class ConfigPropertyMap;
}

namespace ScreenLocker
{

class WallpaperIntegration : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString pluginName READ pluginName NOTIFY packageChanged)
    Q_PROPERTY(KDeclarative::ConfigPropertyMap *configuration READ configuration NOTIFY configurationChanged)

public:
    explicit WallpaperIntegration(QObject *parent);
    ~WallpaperIntegration() override;

    void init();

    void setConfig(const KSharedConfig::Ptr &config) {
        m_config = config;
    }
    QString pluginName() const {
        return m_pluginName;
    }
    void setPluginName(const QString &name);

    KPackage::Package package() const {
        return m_package;
    }

    KDeclarative::ConfigPropertyMap *configuration() const {
        return m_configuration;
    }

    KConfigLoader *configScheme();

Q_SIGNALS:
    void packageChanged();
    void configurationChanged();

private:
    QString m_pluginName;
    KPackage::Package m_package;
    KSharedConfig::Ptr m_config;
    KConfigLoader *m_configLoader = nullptr;
    KDeclarative::ConfigPropertyMap *m_configuration = nullptr;
};

}

#endif

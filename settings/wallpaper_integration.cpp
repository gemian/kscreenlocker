/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*********************************************************************/
#include "wallpaper_integration.h"

#include <KConfig>
#include <KConfigGroup>
#include <KConfigLoader>
#include <KPackage/PackageLoader>
#include <KDeclarative/ConfigPropertyMap>

#include <QFile>

namespace ScreenLocker
{

WallpaperIntegration::WallpaperIntegration(QObject *parent)
    : QObject(parent)
    , m_package(KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Wallpaper")))
{
    qRegisterMetaType<KDeclarative::ConfigPropertyMap*>();
}

WallpaperIntegration::~WallpaperIntegration() = default;

void WallpaperIntegration::init()
{
    if (!m_package.isValid()) {
        return;
    }
    if (auto config = configScheme()) {
        m_configuration = new KDeclarative::ConfigPropertyMap(config, this);
        m_configuration->setAutosave(false);
        // potd (picture of the day) is using a kded to monitor changes and
        // cache data for the lockscreen. Let's notify it.
        m_configuration->setNotify(true);
    }
}

void WallpaperIntegration::setPluginName(const QString &name)
{
    if (m_pluginName == name) {
        return;
    }
    m_pluginName = name;
    m_package.setPath(name);
    emit packageChanged();
}

KConfigLoader *WallpaperIntegration::configScheme()
{
    if (!m_configLoader) {
        const QString xmlPath = m_package.filePath(QByteArrayLiteral("config"), QStringLiteral("main.xml"));

        const KConfigGroup cfg = m_config->group("Greeter").group("Wallpaper").group(m_pluginName);

        if (xmlPath.isEmpty()) {
            m_configLoader = new KConfigLoader(cfg, nullptr, this);
        } else {
            QFile file(xmlPath);
            m_configLoader = new KConfigLoader(cfg, &file, this);
        }
    }
    return m_configLoader;
}

}

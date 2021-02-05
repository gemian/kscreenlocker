/********************************************************************
 This file is part of the KDE project.

SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>
SPDX-FileCopyrightText: 2019 Kevin Ottens <kevin.ottens@enioka.com>
SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

SPDX-License-Identifier: GPL-2.0-or-later
*********************************************************************/
#ifndef KCM_H
#define KCM_H

#include <KPackage/Package>
#include <KQuickAddons/ManagedConfigModule>

#include "kscreensaversettings.h"

class ScreenLockerKcmForm;
class AppearanceSettings;
namespace ScreenLocker
{
class WallpaperIntegration;
class LnFIntegration;
}

namespace KDeclarative
{
class ConfigPropertyMap;
}


class ScreenLockerKcm : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
public:
    explicit ScreenLockerKcm(QObject *parent = nullptr, const QVariantList& args = QVariantList());

    Q_PROPERTY(KScreenSaverSettings *settings READ settings CONSTANT)
    Q_PROPERTY(KDeclarative::ConfigPropertyMap *wallpaperConfiguration READ wallpaperConfiguration NOTIFY currentWallpaperChanged)
    Q_PROPERTY(KDeclarative::ConfigPropertyMap *lnfConfiguration READ lnfConfiguration CONSTANT)
    Q_PROPERTY(QUrl lnfConfigFile READ lnfConfigFile CONSTANT)
    Q_PROPERTY(QUrl wallpaperConfigFile READ wallpaperConfigFile NOTIFY currentWallpaperChanged)
    Q_PROPERTY(ScreenLocker::WallpaperIntegration *wallpaperIntegration READ wallpaperIntegration NOTIFY currentWallpaperChanged)
    Q_PROPERTY(QString currentWallpaper READ currentWallpaper NOTIFY currentWallpaperChanged)
    Q_PROPERTY(bool isDefaultsAppearance READ isDefaultsAppearance NOTIFY isDefaultsAppearanceChanged)

    Q_INVOKABLE QVector<WallpaperInfo> availableWallpaperPlugins() {
        return KScreenSaverSettings::getInstance().availableWallpaperPlugins();
    }

    KScreenSaverSettings *settings() const;
    QUrl lnfConfigFile() const;
    QUrl wallpaperConfigFile() const;
    ScreenLocker::WallpaperIntegration *wallpaperIntegration() const;
    QString currentWallpaper() const;
    bool isDefaultsAppearance() const;

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;
    void updateState();

Q_SIGNALS:
    void currentWallpaperChanged();
    void isDefaultsAppearanceChanged();

private:
    bool isSaveNeeded() const override;
    bool isDefaults() const override;

    KDeclarative::ConfigPropertyMap *wallpaperConfiguration() const;
    KDeclarative::ConfigPropertyMap *lnfConfiguration() const;

    AppearanceSettings *m_appearanceSettings;
    QString m_currentWallpaper;
};

#endif

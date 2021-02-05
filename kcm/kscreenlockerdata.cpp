/********************************************************************
 This file is part of the KDE project.

SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>

SPDX-License-Identifier: GPL-2.0-or-later
*********************************************************************/

#include "kscreenlockerdata.h"
#include "kscreensaversettings.h"
#include "appearancesettings.h"

KScreenLockerData::KScreenLockerData(QObject *parent, const QVariantList &args)
    : KCModuleData(parent, args)
    , m_appearanceSettings(new AppearanceSettings(this))
{
    m_appearanceSettings->load();
}

bool KScreenLockerData::isDefaults() const
{
    return KScreenSaverSettings::getInstance().isDefaults() && m_appearanceSettings->isDefaults();
}

#include "kscreenlockerdata.moc"

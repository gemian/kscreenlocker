/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

Copyright 2021 Gemian

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
#include "gemianlockinterface.h"
#include "ksldapp.h"
#include "gemianlockadaptor.h"
// KDE
#include <KAuthorized>
#include <KIdleTime>
#include <KRandom>
// Qt
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusServiceWatcher>

namespace ScreenLocker
{
const uint ChangeScreenSettings = 4;

    GemianLockInterface::GemianLockInterface(KSldApp *parent)
    : QObject(parent)
    , m_daemon(parent)
{
    (void) new GemianLockAdaptor( this );
    QDBusConnection::systemBus().registerService(QStringLiteral("org.thinkglobally.Gemian.Lock")) ;
    QDBusConnection::systemBus().registerObject(QStringLiteral("/org/thinkglobally/Gemian/Lock"), this);

    connect(m_daemon, &KSldApp::locked, this, &GemianLockInterface::slotLocked);
    connect(m_daemon, &KSldApp::unlocked, this, &GemianLockInterface::slotUnlocked);
}

GemianLockInterface::~GemianLockInterface()
{
}

void GemianLockInterface::slotLocked()
{
    emit Active();
}

void GemianLockInterface::slotUnlocked()
{
    emit Inactive();
}


} // namespace


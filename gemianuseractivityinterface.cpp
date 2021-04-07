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
#include "gemianuseractivityinterface.h"
#include "gemianuseractivityadaptor.h"
#include <syslog.h>
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

GemianUserActivityInterface::GemianUserActivityInterface(AbstractLocker *parent)
    : QObject(parent)
    , m_daemon(parent)
    , event_period{500}
{
    (void) new GemianUserActivityAdaptor( this );
    QDBusConnection::systemBus().registerService(QStringLiteral("org.thinkglobally.Gemian.UserActivity")) ;
    QDBusConnection::systemBus().registerObject(QStringLiteral("/org/thinkglobally/Gemian/UserActivity"), this);

    connect(m_daemon, &AbstractLocker::userActivityPress, this, &GemianUserActivityInterface::ActivityPress);
    connect(m_daemon, &AbstractLocker::userActivityRelease, this, &GemianUserActivityInterface::ActivityRelease);

}

GemianUserActivityInterface::~GemianUserActivityInterface()
{
}

void GemianUserActivityInterface::ActivityPress()
{
    if (std::chrono::steady_clock::now() >= last_activity_changing_power_state_event_time + event_period) {
        emit UserActivityChange();
        syslog(LOG_DEBUG, "emit UserActivityChange");
        last_activity_changing_power_state_event_time = std::chrono::steady_clock::now();
    }
}

void GemianUserActivityInterface::ActivityRelease()
{
    if (std::chrono::steady_clock::now() >= last_activity_extending_power_state_event_time + event_period) {
        emit UserActivityExtend();
        last_activity_extending_power_state_event_time = std::chrono::steady_clock::now();
    }
}

} // namespace


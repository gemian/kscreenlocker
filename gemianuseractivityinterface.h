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
#ifndef GEMIAN_USER_ACTIVITY_INTERFACE_H
#define GEMIAN_USER_ACTIVITY_INTERFACE_H

#include "abstractlocker.h"

#include <chrono>

#include <QObject>
#include <QDBusContext>
#include <QDBusMessage>

class QDBusServiceWatcher;

namespace ScreenLocker
{
class GemianUserActivityInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.thinkglobaly.Gemian.UserActivity")
public:
    explicit GemianUserActivityInterface(AbstractLocker *parent = nullptr);
    ~GemianUserActivityInterface() override;

Q_SIGNALS:
    // DBus signals
    void UserActivityChange();
    void UserActivityExtend();

private Q_SLOTS:
    void ActivityPress();
    void ActivityRelease();

private:
    AbstractLocker *m_daemon;
    std::chrono::milliseconds event_period;
    std::chrono::steady_clock::time_point last_activity_changing_power_state_event_time;
    std::chrono::steady_clock::time_point last_activity_extending_power_state_event_time;
};
}

#endif // GEMIAN_USER_ACTIVITY_INTERFACE_H

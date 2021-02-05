/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

 SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*********************************************************************/
#ifndef SCREENLOCKER_WAYLANDSERVER_H
#define SCREENLOCKER_WAYLANDSERVER_H

#include <QObject>

struct wl_client;
struct wl_global;
struct wl_resource;

namespace KWayland
{
namespace Server
{
class ClientConnection;
class Display;
}
}

namespace ScreenLocker
{

class WaylandServer : public QObject
{
    Q_OBJECT
public:
    explicit WaylandServer(QObject *parent = nullptr);
    ~WaylandServer() override;
    int start();
    void stop();

Q_SIGNALS:
    void x11WindowAdded(quint32 window);

private Q_SLOTS:
    void osdProgress(const QString &icon, int percent, const QString &additionalText);
    void osdText(const QString &icon, const QString &additionalText);

private:
    static void bind(wl_client *client, void *data, uint32_t version, uint32_t id);
    static void unbind(wl_resource *resource);
    static void x11WindowCallback(wl_client *client, wl_resource *resource, uint32_t id);
    static void suspendSystemCallback(wl_client *client, wl_resource *resource);
    static void hibernateSystemCallback(wl_client *client, wl_resource *resource);
    void addResource(wl_resource *r);
    void removeResource(wl_resource *r);
    void sendCanSuspend();
    void sendCanHibernate();
    QScopedPointer<KWayland::Server::Display> m_display;
    KWayland::Server::ClientConnection *m_allowedClient = nullptr;
    wl_global *m_interface = nullptr;
    QList<wl_resource*> m_resources;
};

}

#endif

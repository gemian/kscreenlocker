/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*********************************************************************/
#include "noaccessnetworkaccessmanagerfactory.h"

#include <QNetworkAccessManager>

namespace ScreenLocker
{

QNetworkAccessManager *NoAccessNetworkAccessManagerFactory::create(QObject *parent)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(parent);
    manager->setNetworkAccessible(QNetworkAccessManager::NotAccessible);
    return manager;
}

}

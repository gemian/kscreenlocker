/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

SPDX-FileCopyrightText: 2017 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*********************************************************************/
#include "../greeter/authenticator.h"
#include <QGuiApplication>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCommandLineParser parser;
    QCommandLineOption delayedOption(QStringLiteral("delayed"),
                                     QStringLiteral("KCheckpass is created at startup, the authentication is delayed"));
    QCommandLineOption directOption(QStringLiteral("direct"),
                                    QStringLiteral("A new KCheckpass gets created when trying to authenticate"));
    parser.addOption(directOption);
    parser.addOption(delayedOption);
    parser.addHelpOption();
    parser.process(app);
    AuthenticationMode mode = AuthenticationMode::Delayed;
    if (parser.isSet(directOption)) {
        mode = AuthenticationMode::Direct;
    }
    if (parser.isSet(directOption) && parser.isSet(delayedOption)) {
        parser.showHelp(0);
    }
    Authenticator authenticator(mode);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("authenticator"), &authenticator);
    engine.load(QUrl::fromLocalFile(QStringLiteral(QML_FILE)));
    return app.exec();
}

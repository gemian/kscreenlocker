/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

SPDX-FileCopyrightText: 2017 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*********************************************************************/
import QtQuick 2.1
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

ApplicationWindow {
    visible: true
    ColumnLayout {
        anchors.fill: parent
        Label {
            id: message
            Connections {
                target: authenticator
                onFailed: {
                    message.text = "Authentication failed";
                }
                onSucceeded: {
                    message.text = "Authentication succeeded";
                }
                onGraceLockedChanged: {
                    message.text = ""
                }
            }
        }
        TextField {
            id: password
            enabled: !authenticator.graceLocked
            echoMode: TextInput.Password
        }
        Button {
            text: "Authenticate"
            enabled: !authenticator.graceLocked
            onClicked: {
                authenticator.tryUnlock(password.text)
            }
        }
    }
}

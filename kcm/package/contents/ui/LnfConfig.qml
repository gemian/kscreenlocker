/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>
SPDX-FileCopyrightText: 2019 Kevin Ottens <kevin.ottens@enioka.com>

SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*********************************************************************/
import QtQuick 2.0
import QtQuick.Controls 2.0 as QtControls
import QtQuick.Layouts 1.1

QtControls.StackView {
    id: main
    signal configurationChanged
    property string sourceFile
    implicitHeight: currentItem && currentItem.implicitHeight || 0
    Layout.fillWidth: true
    onSourceFileChanged: {
        pop()
        if (sourceFile) {
            var props = {}
            var lnfConfiguration = configDialog.lnfConfiguration
            for (var key in lnfConfiguration) {
                props["cfg_" + key] = lnfConfiguration[key]
            }

            var newItem = push(sourceFile, props, QtControls.StackView.ReplaceTransition)

            lnfConfiguration.valueChanged.connect(function(key, value) {
                if (newItem["cfg_" + key] !== undefined) {
                    newItem["cfg_" + key] = value
                }
            })

            var createSignalHandler = function(key) {
                return function() {
                    configDialog.lnfConfiguration[key] = newItem["cfg_" + key]
                    configurationChanged()
                }
            }

            for (var key in lnfConfiguration) {
                var changedSignal = newItem["cfg_" + key + "Changed"]
                if (changedSignal) {
                    changedSignal.connect(createSignalHandler(key))
                }
            }
        } else {
            push(empty)
        }
    }
    Component {
        id: empty
        Item {}
    }
}

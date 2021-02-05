/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>
SPDX-FileCopyrightText: 2019 Kevin Ottens <kevin.ottens@enioka.com>

SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*********************************************************************/
import QtQuick 2.0
import QtQuick.Controls 2.15 as QtControls
import QtQuick.Layouts 1.1
QtControls.StackView {
    id: main
    Layout.fillHeight: true
    implicitHeight: 490
    Layout.fillWidth: true
    property string sourceFile
    signal configurationChanged
    onSourceFileChanged: {
        if (sourceFile) {
            var props = {}
            var wallpaperConfig = configDialog.wallpaperConfiguration
            for (var key in wallpaperConfig) {
                props["cfg_" + key] = wallpaperConfig[key]
            }

            var newItem = replace(sourceFile, props, QtControls.StackView.ReplaceTransition)

            wallpaperConfig.valueChanged.connect(function(key, value) {
                if (newItem["cfg_" + key] !== undefined) {
                    newItem["cfg_" + key] = value
                }
            })

            var createSignalHandler = function(key) {
                return function() {
                    configDialog.wallpaperConfiguration[key] = newItem["cfg_" + key]
                    configurationChanged()
                }
            }

            for (var key in wallpaperConfig) {
                var changedSignal = newItem["cfg_" + key + "Changed"]
                if (changedSignal) {
                    changedSignal.connect(createSignalHandler(key))
                }
            }
        }
        else {
            replace(empty)
        }
    }
    Component {
        id: empty
        Item {}
    }
}

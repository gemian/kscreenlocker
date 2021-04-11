/*
 *   Copyright 2016 Gemian
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.bluezqt 1.0 as BluezQt
import org.kde.plasma.networkmanagement 0.2 as PlasmaNM
import org.kde.kquickcontrolsaddons 2.0
import MeeGo.QOfono 0.2

Rectangle {
    id: connectivity

    color: "white"

    height: wifiSwitchButton.height
    width: connectivityButtonRow.width

    property QtObject btManager : BluezQt.Manager

    OfonoManager {
        id: ofonoManager
    }

    OfonoModem {
        id: ofonoModem1
        modemPath: ofonoManager.defaultModem
    }


    PlasmaNM.Handler {
        id: handler
    }

    PlasmaNM.AvailableDevices {
        id: availableDevices
    }

    PlasmaNM.EnabledConnections {
        id: enabledConnections

        onWirelessEnabledChanged: {
            wifiSwitchButton.checked = wifiSwitchButton.enabled && enabled
        }

        onWirelessHwEnabledChanged: {
            wifiSwitchButton.enabled = enabled && availableDevices.wirelessDeviceAvailable
        }

    }

    RowLayout {
        id: connectivityButtonRow

        spacing: units.smallSpacing * 4

        Rectangle {
            color: "transparent"
            width: units.smallSpacing
            height: wifiSwitchButton.height
        }

        SwitchButton {
            id: wifiSwitchButton

            checked: enabled && enabledConnections.wirelessEnabled
            enabled: enabledConnections.wirelessHwEnabled && availableDevices.wirelessDeviceAvailable
            icon: "network-wireless"
            visible: availableDevices.wirelessDeviceAvailable

            onClicked: {
                handler.enableWireless(checked);
            }
        }

        SwitchButton {
            id: cellularSwitchButton

            checked: enabled && ofonoModem1.powered
            enabled: ofonoManager.available
            icon: "phone"
            visible: ofonoManager.available

            onClicked: {
                ofonoModem1.powered = checked;
            }
        }

        SwitchButton {
            id: bluetoothSwitchButton

            checked: btManager.bluetoothOperational
            enabled: btManager.bluetoothBlocked || btManager.adapters.length
            icon: "preferences-system-bluetooth"

            onClicked: {
                enableBluetooth(checked);
            }
        }

        Rectangle {
            color: "transparent"
            width: units.smallSpacing
            height: wifiSwitchButton.height
        }
    }

    function enableBluetooth(enable)
    {
        btManager.bluetoothBlocked = !enable;

        for (var i = 0; i < btManager.adapters.length; ++i) {
            var adapter = btManager.adapters[i];
            adapter.powered = enable;
        }
    }

}
/*
    Virtual MIDI Piano Keyboard for N9
    Copyright (C) 2008-2011, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 1.1
import com.nokia.meego 1.0

Item {
    height: 98
    width: parent.width
    property string itemTitle: ""
    property string itemValue: ""
    property color sampleColor: "transparent"
    signal itemSelected

    BorderImage {
        id: background
        anchors.fill: parent
        anchors.leftMargin: -parent.anchors.leftMargin
        anchors.rightMargin: -parent.anchors.rightMargin
        visible: mouseArea.pressed
        source: "image://theme/meegotouch-list-background-pressed-center"
    }

    Item {
        anchors {
            left: parent.left
            right: colorSampleRect.left
            top: parent.top
            bottom: parent.bottom
        }
        Column {
            anchors.verticalCenter: parent.verticalCenter
            Label {
                id: mainText
                text: itemTitle
                font.pixelSize: 26
                font.bold: true;
                color: (theme.inverted ? "#d6d6d6" : "#282828")
            }
            Label {
                id: subText
                text: itemValue
                font.pixelSize: 22
                color: (theme.inverted ? "#b0b0b0" : "#505050")
            }
        }
    }

    Rectangle {
        id: colorSampleRect
        anchors.right: imageIndicator.left
        anchors.rightMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        width: 60
        height: 60
        color: sampleColor
    }

    Image {
        id: imageIndicator
        source: "image://theme/meegotouch-combobox-indicator" +
                (theme.inverted ? "-inverted" : "") +
                (enabled ? "" : "-disabled") +
                (mouseArea.pressed ? "-pressed" : "")
        anchors.right: parent.right;
        anchors.verticalCenter: parent.verticalCenter
    }

    MouseArea {
        id: mouseArea
        anchors.fill: background
        onClicked: itemSelected()
    }
}

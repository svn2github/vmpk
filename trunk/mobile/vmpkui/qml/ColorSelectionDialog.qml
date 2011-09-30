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

Dialog {
    id: colorPicker
    property Style platformStyle: SelectionDialogStyle {}
    property color fillColor: colors[8]
    property variant colors: [
        "maroon",  "green",     "navy",   "grey",
        "red",     "darkgreen", "blue",   "dimgrey",
        "purple",  "olive",     "teal",   "darkorange",
        "magenta", "limegreen", "#4591ff","gold"
    ]
    title: Item {
        id: header
        height: colorPicker.platformStyle.titleBarHeight
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        Item {
            id: labelField
            anchors.fill:  parent
            Item {
                id: labelWrapper
                anchors.left: labelField.left
                anchors.right: closeButton.left
                anchors.bottom:  parent.bottom
                anchors.bottomMargin: colorPicker.platformStyle.titleBarLineMargin
                height: titleLabel.height
                Label {
                    id: titleLabel
                    x: colorPicker.platformStyle.titleBarIndent
                    width: parent.width - closeButton.width
                    font: colorPicker.platformStyle.titleBarFont
                    color: colorPicker.platformStyle.commonLabelColor
                    elide: colorPicker.platformStyle.titleElideMode
                    text: qsTr("Color Selection")
                }
            }
            Image {
                id: closeButton
                anchors.bottom:  parent.bottom
                anchors.bottomMargin: colorPicker.platformStyle.titleBarLineMargin-6
                anchors.right: labelField.right
                opacity: closeButtonArea.pressed ? 0.5 : 1.0
                source: "image://theme/icon-m-common-dialog-close"
                MouseArea {
                    id: closeButtonArea
                    anchors.fill: parent
                    onClicked:  {
                        colorPicker.reject()
                    }
                }
            }
        }
        Rectangle {
            id: headerLine
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom:  header.bottom
            height: 1
            color: "#4D4D4D"
        }
    }
    content: Grid {
        id: colorGrid
        columns: 4
        spacing: 10
        anchors.horizontalCenter: parent.horizontalCenter
        Repeater {
            model: 16
            Rectangle {
                width: 80
                height: 80
                color: chooserArea.pressed ? Qt.darker(colorPicker.colors[index]) : colorPicker.colors[index]
                radius: 5
                MouseArea {
                    id: chooserArea
                    anchors.fill: parent
                    onClicked: {
                        colorPicker.fillColor = colorPicker.colors[index]
                        colorPicker.accept()
                    }
                }
            }
        }
    }
}

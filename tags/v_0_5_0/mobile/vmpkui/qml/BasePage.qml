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

Page {
    id: container
    property string title: ""
    property alias client: clientArea

    Rectangle {
        id: titleArea
        width: parent.width
        height: 18*2+26
        x: 0
        y: 0
        z: 1
        color: "purple"
        Label {
            id: titletext
            text: container.title
            anchors.verticalCenter: parent.verticalCenter
            x: 2 * 18
            font.pixelSize: 30
            font.bold: true
            color: "white"
            visible: text != ""
        }
    }

    Item {
        id: clientArea
        anchors.top: titleArea.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
    }

    tools: ToolBarLayout {
        id: pageToolbar
        ToolIcon {
            iconId: "toolbar-back";
            onClicked: pageStack.pop();
        }
    }
}

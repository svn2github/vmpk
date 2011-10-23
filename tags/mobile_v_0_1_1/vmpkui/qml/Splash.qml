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
    id: splashPage
    anchors.margins: 32
    orientationLock: PageOrientation.LockPortrait
    property int fadetime: 2000;

    Timer {
        property int tick: 0
        id: timer1
        interval: 500; running: true; repeat: true
        onTriggered: {
            tick++;
            if (tick == 1)
                image1.opacity = image2.opacity = 1
            if (tick == 4) {
                var component = Qt.createComponent("MainPage.qml")
                if (component.status == Component.Ready) {
                    appWindow.pageStack.clear();
                    appWindow.pageStack.push(component);
                } else {
                    console.log("Error loading component:", component.errorString());
                }
                running = false
            }
        }
    }

    Column {
        id: col
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.bottomMargin: bottom.height + 50
        Image {
            id: image1
            opacity: 0
            source: "vmpk.png"
            smooth: true
            width: 300
            height: 300
            anchors.horizontalCenter: parent.horizontalCenter
            Behavior on opacity { NumberAnimation { duration: fadetime; easing.type: Easing.OutQuad } }
        }
        Item {
            id: space
            width: 32; height: 32
        }
        Text {
            id: text1
            text: qsTr("VMPK & FluidSynth")
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 48
            font.bold: true
            color: "black"
            Behavior on opacity { NumberAnimation { duration: fadetime } }
        }
        Item {
            id: space2
            width: 128; height: 32
        }
        Image {
            id: image2
            opacity: 0
            source: "fluidsynth.png"
            smooth: true
            width: 300
            anchors.horizontalCenter: parent.horizontalCenter
            Behavior on opacity { NumberAnimation { duration: fadetime; easing.type: Easing.OutQuad } }
        }
    }

    Row {
        id: bottom
        anchors.bottomMargin: 32
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        Behavior on opacity { NumberAnimation { duration: fadetime } }
        Text {
            id: text2
            anchors.verticalCenter: parent.verticalCenter
            text: "http://vmpk.sourceforge.net<br>http://www.fluidsynth.org"
            font.pixelSize: 32
            verticalAlignment: Text.AlignBottom
        }
    }
}

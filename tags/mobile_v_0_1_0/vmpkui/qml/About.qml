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

BasePage {
    id: aboutPage
    title: qsTr("About")

    Flickable {
        anchors.fill: client
        contentWidth: client.width
        contentHeight: col.height
        flickableDirection : Flickable.VerticalFlick

        Column {
            id: col
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width

            Item {
                width: 1; height: 32
            }
            Rectangle {
                 width: 320
                 height: 320
                 color: "#d6d6d6"
                 radius: 20
                 anchors.horizontalCenter: parent.horizontalCenter
                Image {
                    id: image1
                    anchors.centerIn: parent
                    source: "vmpk.png"
                    smooth: true
                    width: 300
                    height: 300

                }
            }
            Item {
                width: 1; height: 32
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("VMPK & FluidSynth")
                font.pixelSize: 48
                font.bold: true
                color: (theme.inverted ? "#d6d6d6" : "#282828")
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("v%1").arg(synthEngine.version())
                font.pixelSize: 24
                font.bold: true
                color: (theme.inverted ? "#d6d6d6" : "#282828")
            }
            Item {
                width: 1; height: 32
            }
            Rectangle {
                 width: 320
                 height: 160
                 color: "#d6d6d6"
                 radius: 20
                 anchors.horizontalCenter: parent.horizontalCenter
                 Image {
                     id: image2
                     anchors.centerIn: parent
                     source: "fluidsynth.png"
                     smooth: true
                     width: 300
                 }
             }
            Item {
                width: 1; height: 32
            }
            Text {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 18
                anchors.rightMargin: 18
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
                font.pixelSize: 24
                color: (theme.inverted ? "#d6d6d6" : "#282828")
                onLinkActivated: Qt.openUrlExternally(link)
                text: qsTr("<p><a href='http://vmpk.sf.net'>VMPK (Virtual MIDI Piano Keyboard)</a>.<br>Copyright © 2008-2011, <a href='mailto:plcl@users.sf.net?subject=VMPK'> Pedro Lopez-Cabanillas &lt;plcl@users.sf.net&gt;</a></p>" +
                      "<p>This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.</p>" +
                      "<p>This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License  along with this program. If not, see <a href='http://www.gnu.org/licenses/'>http://www.gnu.org/licenses/</a>.</p>" +
                      "<p>The following copyright notice is also applicable:</p>" +
                      "<p><a href='http://www.fluidsynth.org'>FluidSynth</a>, A Software Synthesizer. Copyright © 2003-2011 Peter Hanappe and others.</p>")
            }
        }
    }
}

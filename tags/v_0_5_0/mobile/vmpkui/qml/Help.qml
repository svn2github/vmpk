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
import QtWebKit 1.0
import com.nokia.meego 1.0

BasePage {
    id: helpPage
    title: qsTr("Help")
    property string document:""

    Flickable {
        id: flickable
        anchors.fill: client
        contentWidth: Math.max(client.width,browser.width)
        contentHeight: Math.max(client.height,browser.height)
        flickableDirection : Flickable.VerticalFlick

        WebView {
            id: browser
            url : "http://vmpk.sourceforge.net/n9/" + document
            transformOrigin: Item.TopLeft
            preferredWidth: flickable.width
            preferredHeight: flickable.height
            settings.standardFontFamily: "Nokia Pure"
            settings.defaultFontSize: 32
            settings.javascriptEnabled: true
            onAlert: console.log(message)
            onLoadFailed: {
                console.log("document " + document + " load failed");
            }
        }
    }

    Component.onCompleted: {
        if (synthEngine.language != "en" && synthEngine.language != "" ) {
            document = "index." + synthEngine.language + ".html"
        }
    }

}

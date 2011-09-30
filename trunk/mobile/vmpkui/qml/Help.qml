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

    Flickable {
        id: flickable
        anchors.fill: client
        contentWidth: Math.max(client.width,browser.width)
        contentHeight: Math.max(client.height,browser.height)
        flickableDirection : Flickable.HorizontalAndVerticalFlick

        WebView {
            id: browser
            transformOrigin: Item.TopLeft
            url: "http://vmpk.sourceforge.net/n9/"
            preferredWidth: flickable.width
            preferredHeight: flickable.height
            settings.defaultFontSize: 32
            onAlert: console.log(message)
        }
    }

}

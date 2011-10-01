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
import com.nokia.extras 1.0

BasePage {
    id: preferences
    title: qsTr("Preferences")

    Flickable {
        anchors.fill: client
        contentWidth: client.width
        contentHeight: col.height
        flickableDirection : Flickable.VerticalFlick

        Column {
            id: column
            width: parent.width - 36
            anchors.horizontalCenter: parent.horizontalCenter

            PreferencesItem {
                id: octavesItem
                itemTitle: qsTr("Number of Octaves")
                itemValue: (octaveSelection.selectedIndex>=0 ? octaveSelection.selectedIndex + 1 : 2)
                onItemSelected: octaveSelection.open()
            }

            PreferencesItem {
                id: colorItem
                itemTitle: qsTr("Key Pressed Color")
                itemValue: colorPicker.fillColor
                sampleColor: colorPicker.fillColor
                onItemSelected: colorPicker.open()
            }

            PreferencesItem {
                id: soundFontItem
                itemTitle: qsTr("SoundFont File")
                itemValue: synthEngine.soundFont
                onItemSelected: soundfontSelection.open()
            }

            PreferencesItem {
                id: languageItem
                itemTitle: qsTr("Language")
                itemValue: "English"
                onItemSelected: languageSelection.open()
            }

            Item {
                id: invertItem
                height: 98
                width: parent.width
                Label {
                    id: mainText
                    text: qsTr("Inverted Theme")
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 26
                    font.bold: true;
                    color: (theme.inverted ? "#d6d6d6" : "#282828")
                }
                Switch {
                    id: cbInvertedTheme
                    anchors.right: parent.right;
                    anchors.verticalCenter: parent.verticalCenter
                    checked: theme.inverted
                    onCheckedChanged: {
                        theme.inverted = cbInvertedTheme.checked
                        synthEngine.invertedTheme = cbInvertedTheme.checked
                    }
                }
            }
        }
    }

    ColorSelectionDialog {
        id: colorPicker
        fillColor: synthEngine.highlightColor
        onAccepted: {
            synthEngine.highlightColor = fillColor
        }
    }

    SelectionDialog {
        id: octaveSelection
        titleText: qsTr("Number of Octaves")
        selectedIndex: synthEngine.numberOfOctaves - 1
        model: ListModel { }
        onAccepted: {
            if (selectedIndex >= 0) {
                synthEngine.numberOfOctaves = selectedIndex + 1
            }
        }
    }

    SelectionDialog {
        id: soundfontSelection
        titleText: qsTr("SoundFont File")
        model: ListModel {
            id: soundfontModel
        }
        onAccepted: {
            if (selectedIndex >= 0) {
                synthEngine.soundFont = model.get(selectedIndex).value
            }
        }
    }

    SelectionDialog {
        id: languageSelection
        titleText: qsTr("Language")
        model: ListModel {
            ListElement { name: "English"; value: "en" }
            ListElement { name: "Español"; value: "es" }
        }
        onAccepted: {
            synthEngine.language = model.get(selectedIndex).value
            languageItem.itemValue = model.get(selectedIndex).name
            langMessage.open()
        }
    }

    QueryDialog {
        id: langMessage
        //icon: "image://theme/icon-l-help"
        titleText: qsTr("Language changed")
        message: qsTr("The new language will take effect after restarting the program")
        acceptButtonText: qsTr("Accept")
    }

    Component.onCompleted: {
        for (var i=1; i<10; ++i)
            octaveSelection.model.append({name: qsTr("%1 Octaves").arg(i)})
        for (var j=0; j<languageSelection.model.count; ++j) {
            if (languageSelection.model.get(j).value == synthEngine.language) {
                languageSelection.selectedIndex = j;
                languageItem.itemValue = languageSelection.model.get(j).name
                break
            }
        }
        synthEngine.scanSoundFonts()
    }

    Connections {
        target: synthEngine
        onSoundFontsChanged: {
            soundfontModel.clear()
        }
        onSoundFontAdded: {
            var shortName = name.substr(name.lastIndexOf("/")+1)
            soundfontModel.append( { name: shortName, value: name } )
        }
    }

}

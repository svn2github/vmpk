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
import QtMobility.sensors 1.1
import PianoKeyboard 0.1

Page {
    id: mainPage
    tools: commonTools
    orientationLock: PageOrientation.LockLandscape
    anchors.margins: 5

    function openFile(file) {
        var component = Qt.createComponent(file)
        if (component.status == Component.Ready) {
            pageStack.push(component);
        } else {
            console.log("error: " + component.errorString())
        }
    }

    SelectionDialog {
        id: instrumentSelection
        titleText: qsTr("Instrument Selection")
        model: ListModel { }
        onAccepted: {
            if (selectedIndex >= 0) {
                synthEngine.setInstrument(selectedIndex);
            }
        }
        onStatusChanged: {
            pianoKbd.enabled = (status == DialogStatus.Closed);
        }
    }

    SelectionDialog {
        id: channelSelection
        titleText: qsTr("Channel Selection")
        selectedIndex: synthEngine.channel
        model: ListModel { }
        onAccepted: {
            if (selectedIndex >= 0) {
                synthEngine.channel = selectedIndex
                channelButton.text = model.get(selectedIndex).name;
            }
        }
        onStatusChanged: {
            pianoKbd.enabled = (status == DialogStatus.Closed);
        }
    }

    SelectionDialog {
        id: baseOctaveSelection
        titleText: qsTr("Base Octave Selection")
        selectedIndex: pianoKbd.baseOctave
        model: ListModel { }
        onAccepted: {
            if (selectedIndex >= 0) {
                pianoKbd.baseOctave = selectedIndex
                octaveButton.text = model.get(selectedIndex).name;
            }
        }
        onStatusChanged: {
            pianoKbd.enabled = (status == DialogStatus.Closed);
        }
    }

    SelectionDialog {
        id: controllerSelection
        titleText: qsTr("Controller Selection")
        selectedIndex: synthEngine.currentController
        model: ListModel { }
        onAccepted: {
            if (selectedIndex >= 0) {
                synthEngine.currentController = selectedIndex
            }
        }
        onStatusChanged: {
            pianoKbd.enabled = (status == DialogStatus.Closed);
        }
    }

    Column {
        id: topPanel
        height: 130
        width: parent.width
        anchors.left: parent.left
        anchors.top: parent.top
        spacing: 5

        ButtonRow {
            Button { id:btNote; text: qsTr("Common") }
            Button { id:btVel;  text: qsTr("Velocity") }
            Button { id:btInst; text: qsTr("Instrument") }
            Button { id:btCtl;  text: qsTr("Controller") }
            Button { id:btBend; text: qsTr("Bender") }
        }

        Row {
            id: panelNote
            visible: btNote.checked
            spacing: 5
            TumblerButton {
                id: channelButton
                anchors.verticalCenter: parent.verticalCenter
                onClicked: {
                    channelSelection.open()
                }
            }
            TumblerButton {
                id: octaveButton
                anchors.verticalCenter: parent.verticalCenter
                onClicked: {
                    baseOctaveSelection.open()
                }
            }
            Label {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Transpose:")
                font.pixelSize: 24
            }
            CountBubble {
                width: 40
                anchors.verticalCenter: parent.verticalCenter
                value: transposeSlider.value
                largeSized: true
            }
            Slider {
                id: transposeSlider
                width: 200
                anchors.verticalCenter: parent.verticalCenter
                minimumValue: -12
                maximumValue: 12
                stepSize: 1
                valueIndicatorVisible: true
                value: 0
            }
        }

        Row {
            id: panelVel
            visible: btVel.checked
            spacing: 5
            Label {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Velocity:")
                font.pixelSize: 24
            }
            CountBubble {
                width: 50
                anchors.verticalCenter: parent.verticalCenter
                largeSized: true
                value: velocitySlider.value
            }
            Slider {
                id: velocitySlider
                anchors.verticalCenter: parent.verticalCenter
                width: 500
                minimumValue: 0
                maximumValue: 127
                stepSize: 1
                valueIndicatorVisible: true
                value: 100
            }
            Label {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Turn:")
                font.pixelSize: 24
            }
            Switch {
                id: velocityTurn
                anchors.verticalCenter: parent.verticalCenter
                checked: false
                onCheckedChanged: {
                    velocitySlider.value = 64
                }
            }
        }

        Row {
            id: panelIns
            visible: btInst.checked
            spacing: 5
            TumblerButton {
                width: 500
                anchors.verticalCenter: parent.verticalCenter
                text: synthEngine.currentInstrumentName
                onClicked: {
                    instrumentSelection.open();
                }
            }
            Button {
                text: qsTr("Panic!")
                iconSource: "image://theme/icon-m-toolbar-volume-off" + (theme.inverted ? "" : "-white")
                platformStyle: ButtonStyle { inverted: !theme.inverted }
                onClicked: {
                    synthEngine.panic();
                }
            }
        }

        Row {
            id: panelCtl
            visible: btCtl.checked
            spacing: 5
            TumblerButton {
                width: 250;
                anchors.verticalCenter: parent.verticalCenter
                text: synthEngine.currentControllerName
                onClicked: {
                    controllerSelection.open();
                }
            }
            CountBubble {
                width: 50
                anchors.verticalCenter: parent.verticalCenter
                value: controlSlider.value
                largeSized: true
            }
            Slider {
                id: controlSlider
                width: 400;
                anchors.verticalCenter: parent.verticalCenter
                minimumValue: 0
                maximumValue: 127
                stepSize: 1
                valueIndicatorVisible: true
                value: synthEngine.currentControllerValue
                onValueChanged: {
                    synthEngine.controlChange(value)
                }
            }
            Label {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Turn:")
                font.pixelSize: 24
            }
            Switch {
                id: ctlTurn
                anchors.verticalCenter: parent.verticalCenter
                checked: false
                onCheckedChanged: {
                    controlSlider.value = 0
                }
            }
        }

        Row {
            id: panelBend
            visible: btBend.checked
            spacing: 5
            Label {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Bender:")
                font.pixelSize: 24
            }
            CountBubble {
                width: 80
                anchors.verticalCenter: parent.verticalCenter
                value: benderSlider.value
                largeSized: true
            }
            Slider {
                id: benderSlider
                anchors.verticalCenter: parent.verticalCenter
                width: 500
                minimumValue: -8192
                maximumValue: 8191
                stepSize: 1
                valueIndicatorVisible: true
                value: 0
                onPressedChanged: {
                    if (!pressed) value = 0;
                }
                onValueChanged: {
                    synthEngine.bender(value)
                }
            }
            Label {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Turn:")
                font.pixelSize: 24
            }
            Switch {
                id: benderTurn
                anchors.verticalCenter: parent.verticalCenter
                checked: false
                onCheckedChanged: {
                    benderSlider.value = 0
                }
            }
        }
    }

    PianoKeyboard {
        id: pianoKbd
        anchors {
            left: parent.left
            right: parent.right
            top: topPanel.bottom
            bottom: parent.bottom
            margins: 5
        }
        showLabels: noteNames.checked
        numOctaves: synthEngine.numberOfOctaves
        keyPressedColor: synthEngine.highlightColor
        velocity: Math.round(velocitySlider.value)
        transpose: Math.round(transposeSlider.value)
        onNoteOn: {
            synthEngine.noteOn(n, v)
        }
        onNoteOff: {
            synthEngine.noteOff(n, v)
        }
    }

    RotationSensor {
        id: rotationSensor
        onReadingChanged: {
            if(velocityTurn.checked)
                velocitySlider.value = 64 - (reading.x * 0.71)
            if (ctlTurn.checked)
                controlSlider.value = -reading.x * 1.41
            if (benderTurn.checked)
                benderSlider.value = -reading.x * 91
        }
    }

    InfoBanner {
        id: volumeInfo
        iconSource: "image://theme/icon-m-toolbar-volume-white"
        anchors.centerIn: parent
        timerEnabled: true
        timerShowTime: 2000
    }

    Component.onCompleted: {
        for (var i=1; i<17; ++i)
            channelSelection.model.append({name: qsTr("Channel %1").arg(i)})
        channelButton.text = channelSelection.model.get(0).name;
        for (var j=-1; j<9; ++j)
            baseOctaveSelection.model.append({name: qsTr("Octave %1").arg(j)})
        octaveButton.text = baseOctaveSelection.model.get(5).name;
        synthEngine.initialize();
        theme.inverted = synthEngine.invertedTheme;
        rotationSensor.start()
    }

    Connections {
        target: synthEngine
        onInstrumentsChanged: {
            instrumentSelection.model.clear()
        }
        onControllersChanged: {
            controllerSelection.model.clear()
        }
        onInstrumentAdded: {
            instrumentSelection.model.append({name: name})
        }
        onControllerAdded: {
            controllerSelection.model.append({name: name})
        }
        onVolumeChanged: {
            volumeInfo.text = qsTr("Volume: %1%").arg(vol)
            volumeInfo.show()
        }
    }

    ToolBarLayout {
        id: commonTools
        visible: true

        Text {
            text: qsTr("VMPK & FluidSynth 0.0.2 (beta)")
            anchors.centerIn: parent
            font.pixelSize: 24
        }

        ToolIcon {
            platformIconId: "toolbar-view-menu";
            anchors.right: parent===undefined ? undefined : parent.right
            onClicked: (mainMenu.status == DialogStatus.Closed) ? mainMenu.open() : mainMenu.close()
        }
    }

    Menu {
        id: mainMenu
        MenuLayout {
            MenuItem {
                text: qsTr("Show Note Names")
                CheckBox {
                    id: noteNames
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                }
                onClicked: {
                    noteNames.checked = !noteNames.checked
                }
            }
            MenuItem {
                text: qsTr("Help")
                MoreIndicator {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                }
                onClicked: mainPage.openFile("Help.qml")
            }
            MenuItem {
                text: qsTr("Preferences")
                MoreIndicator {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                }
                onClicked: mainPage.openFile("Preferences.qml")
            }
            MenuItem {
                text: qsTr("About")
                MoreIndicator {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                }
                onClicked: mainPage.openFile("About.qml")
            }
        }
        onStatusChanged: {
            pianoKbd.enabled = (status == DialogStatus.Closed)
        }
    }
}

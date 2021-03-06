/*
    MIDI Virtual Piano Keyboard 
    Copyright (C) 2008, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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


#include "midisetup.h"

MidiSetup::MidiSetup()
{
    ui.setupUi(this);
    connect(ui.chkEnableInput, SIGNAL(toggled(bool)), SLOT(toggledInput(bool)));
}

void MidiSetup::toggledInput(bool state)
{
    if (!state) ui.comboInput->setCurrentIndex(-1);
}

void MidiSetup::inputNotAvailable()
{
    setInputEnabled(false);
    ui.chkEnableInput->setEnabled(false);
    ui.comboInput->setCurrentIndex(-1);
}

bool MidiSetup::inputIsEnabled()
{
    return ui.chkEnableInput->isChecked();
}

void MidiSetup::setInputEnabled(const bool state)
{
    ui.chkEnableInput->setChecked(state);
}

void MidiSetup::clearCombos()
{
    ui.comboInput->clear();
    ui.comboOutput->clear();
}

void MidiSetup::addInputPortName(const QString& input, int index)
{
    ui.comboInput->addItem(input, index);
}

void MidiSetup::setCurrentInput(int index)
{
    if (index < 0)
        ui.comboInput->setCurrentIndex(index);
    else {
        int i; 
        for (i = 0; i < ui.comboInput->count(); ++i) {
            if (index == ui.comboInput->itemData(i).toInt()) {
                ui.comboInput->setCurrentIndex(i);
                return;
            }
        }
    }
}

void MidiSetup::setCurrentOutput(int index)
{
    if (index < 0)
        ui.comboOutput->setCurrentIndex(index);
    else {
        int i; 
        for (i = 0; i < ui.comboOutput->count(); ++i) {
            if (index == ui.comboOutput->itemData(i).toInt()) {
                ui.comboOutput->setCurrentIndex(i);
                return;
            }
        }
    }
}

void MidiSetup::addOutputPortName(const QString& output, int index)
{
    ui.comboOutput->addItem(output, index);
}

int MidiSetup::selectedInput()
{
    int idx = ui.comboInput->currentIndex();
    if (idx >= 0)
        return ui.comboInput->itemData(idx).toInt();
    else 
        return -1;    
}

int MidiSetup::selectedOutput()
{
    int idx = ui.comboOutput->currentIndex();    
    if (idx >= 0)
        return ui.comboOutput->itemData(idx).toInt();
    else 
        return -1;    
}


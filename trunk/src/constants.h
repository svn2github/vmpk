/*
    MIDI Virtual Piano Keyboard
    Copyright (C) 2008-2009, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <QApplication>
#include <QString>
#include "vmpk_version.h"

/* Don't translate any string defined in this header */

//replaced with the generated header "vmpk_version.h"
//const QString PGM_VERSION("0.2.4cvs");

const QString BLD_DATE(__DATE__);
const QString BLD_TIME(__TIME__);
const QString CMP_VERSION(__VERSION__);

const QString QSTR_APPNAME("Virtual MIDI Piano Keyboard");
const QString QSTR_DOMAIN("vmpk.sourceforge.net");
const QString QSTR_VMPKPX("vmpk_");
const QString QSTR_QTPX("qt_");

const QString QSTR_WINDOW("Window");
const QString QSTR_GEOMETRY("Geometry");
const QString QSTR_STATE("State");
const QString QSTR_PREFERENCES("Preferences");
const QString QSTR_CHANNEL("Channel");
const QString QSTR_VELOCITY("Velocity");
const QString QSTR_BASEOCTAVE("BaseOctave");
const QString QSTR_TRANSPOSE("Transpose");
const QString QSTR_NUMOCTAVES("NumOctaves");
const QString QSTR_INSTRUMENTSDEFINITION("InstrumentsDefinition");
const QString QSTR_INSTRUMENTNAME("InstrumentName");
const QString QSTR_KEYPRESSEDCOLOR("KeyPressedColor");
const QString QSTR_CONNECTIONS("Connections");
const QString QSTR_INENABLED("InEnabled");
const QString QSTR_THRUENABLED("ThruEnabled");
const QString QSTR_INPORT("InPort");
const QString QSTR_OUTPORT("OutPort");
const QString QSTR_KEYBOARD("Keyboard");
const QString QSTR_MAPFILE("MapFile");
const QString QSTR_RAWMAPFILE("RawMapFile");
const QString QSTR_DEFAULT("default");
const QString QSTR_CONTROLLERS("Controllers");
const QString QSTR_INSTRUMENT("Instrument");
const QString QSTR_BANK("Bank");
const QString QSTR_PROGRAM("Program");
const QString QSTR_CONTROLLER("Controller");
const QString QSTR_GRABKB("GrabKeyboard");
const QString QSTR_STYLEDKNOBS("StyledKnobs");
const QString QSTR_ALWAYSONTOP("AlwaysOnTop");
const QString QSTR_SHOWNOTENAMES("ShowNoteNames");
const QString QSTR_RAWKEYBOARDMODE("RawKeyboardMode");

const QString QSTR_VMPK("VMPK");
const QString QSTR_VMPKINPUT("VMPK Input");
const QString QSTR_VMPKOUTPUT("VMPK Output");

const QString QSTR_VMPKURL("http://vmpk.sourceforge.net");

#endif /*CONSTANTS_H_*/

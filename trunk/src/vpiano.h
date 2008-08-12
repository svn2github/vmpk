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

#ifndef VPIANO_H
#define VPIANO_H

#include <QMainWindow>
#include <QEvent>

#include "ui_vpiano.h"
#include "about.h"
#include "preferences.h"
#include "midisetup.h"
#include "kmapdialog.h"
#include "mididefs.h"

class QComboBox;
class QSpinBox;
class QSlider;
class QStyle;
class Knob;
class Instrument;
class RtMidiIn;
class RtMidiOut;

const QEvent::Type NoteOnEventType = QEvent::Type( QEvent::User + 0x90 );  
const QEvent::Type NoteOffEventType = QEvent::Type( QEvent::User + 0x80 );  

class NoteEvent : public QEvent
{
public:    
    NoteEvent(unsigned char note, QEvent::Type type) 
        : QEvent(type),
        m_note(note) { }
    unsigned char getNote() const { return m_note; }
private:
    unsigned char m_note;
};

class NoteOnEvent : public NoteEvent
{
public:    
    NoteOnEvent(unsigned char note) 
        : NoteEvent(note, NoteOnEventType)
    { }
};

class NoteOffEvent : public NoteEvent
{
public:    
    NoteOffEvent(unsigned char note) 
        : NoteEvent(note, NoteOffEventType)
    { }
};

class VPiano : public QMainWindow
{
    Q_OBJECT
public:
    VPiano( QWidget * parent = 0, Qt::WindowFlags flags = 0 );
    virtual ~VPiano();
    int getInputChannel();
    
protected:
    void closeEvent ( QCloseEvent *event );
    void customEvent ( QEvent *event );  
    void showEvent ( QShowEvent *event ); 
    void hideEvent( QHideEvent *event );
    
public slots:
    void slotAbout();
    void slotAboutQt();
    void slotConnections();
    void slotPreferences();
    void slotLoadKeyboardMap();
    void slotSaveKeyboardMap();
    void slotEditKeyboardMap();
    void slotNoteOn(int midiNote);
    void slotNoteOff(int midiNote);
    void slotPanic();
    void slotResetAllControllers();
    void slotResetBender();
    void slotController();
    void slotBender();
    void slotBankChanged(int index);
    void slotProgChanged(int index);
    void slotBaseOctave(int octave);
    
private:
    void initialization();
    void initMidi();
    void readSettings();
    void writeSettings();
    void applyPreferences();
    void applyConnections();
    void refreshConnections();
    void initToolBars();
    void sendController(int controller, int value);
    void resetAllControllers();
    void allNotesOff();
    void bankChange(int bank);
    void programChange(int program);
    void bender(int value);

    RtMidiOut* m_midiout;
    RtMidiIn* m_midiin;
    int m_currentOut;
    int m_currentIn;
    
    Ui::VPiano ui;
    About dlgAbout;
    Preferences dlgPreferences;
    MidiSetup dlgMidiSetup;
    KMapDialog dlgKeyMap;

    QSpinBox* m_sboxChannel;
    QSpinBox* m_sboxOctave;
    Knob* m_Velocity;
    QComboBox* m_comboControl;
    Knob* m_Control;
    QSlider* m_bender;
    QComboBox* m_comboBank;
    QComboBox* m_comboProg;
    QStyle* m_dialStyle;
    Instrument* m_ins;
};

#endif // VPIANO_H

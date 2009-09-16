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

const QEvent::Type NoteOnEventType = QEvent::Type( QEvent::registerEventType( QEvent::User + STATUS_NOTEON ));
const QEvent::Type NoteOffEventType = QEvent::Type( QEvent::registerEventType( QEvent::User + STATUS_NOTEOFF ));
const QEvent::Type ControllerEventType = QEvent::Type( QEvent::registerEventType( QEvent::User + STATUS_CONTROLLER ));

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

class ControllerEvent : public QEvent
{
public:
    ControllerEvent(unsigned char ctl, unsigned char val)
        : QEvent(ControllerEventType),
        m_ctl(ctl),
        m_value(val)
        { }
    unsigned char getController() const { return m_ctl; }
    unsigned char getValue() const { return m_value; }
private:
    unsigned char m_ctl;
    unsigned char m_value;
};

class VPiano : public QMainWindow, public PianoHandler
{
    Q_OBJECT
public:
    VPiano( QWidget * parent = 0, Qt::WindowFlags flags = 0 );
    virtual ~VPiano();
    int getInputChannel();
    void midiThru(std::vector<unsigned char> *message) const;
    bool isInitialized() const { return m_initialized; }

    // PianoHandler methods
    void noteOn(const int midiNote);
    void noteOff(const int midiNote);

    // static methods
    static QString dataDirectory();

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
    void slotEditKeyboardMap();
    void slotPanic();
    void slotResetAllControllers();
    void slotResetBender();
    void slotController();
    void slotController(const int value);
    void slotControlToggled(const bool value);
    void slotBender();
    void slotBenderReleased();
    void slotBankChanged(const int index);
    void slotProgChanged(const int index);
    void slotBaseOctave(const int octave);
    void slotTranspose(const int transpose);
    void slotOutChannel(const int channel);
    void slotCtlChanged(const int index);
    void slotHelpContents();
    void slotOpenWebSite();
    void setVelocity(int value) { m_velocity = value; }
    void slotImportSF();
    void slotEditExtraControls();
    //void slotEditPrograms();
    //void slotDebugDestroyed(QObject *obj);

private:
    void initialization();
    bool initMidi();
    void initControllers();
    void readSettings();
    void writeSettings();
    void applyPreferences();
    void applyConnections();
    void applyInitialSettings();
    void refreshConnections();
    void initToolBars();
    void clearExtraControllers();
    void initExtraControllers();
    void sendController(const int controller, const int value);
    void resetAllControllers();
    void allNotesOff();
    void bankChange(const int bank);
    void programChange(const int program);
    void bender(const int value);
    void messageWrapper(std::vector<unsigned char> *message) const;
    void updateController(int ctl, int val);
    void grabKb() const;
    void releaseKb() const;
    void updateKnobs();

    RtMidiOut* m_midiout;
    RtMidiIn* m_midiin;
    int m_currentOut;
    int m_currentIn;
    bool m_inputActive;
    bool m_midiThru;
    bool m_initialized;

    Ui::VPiano ui;
    About dlgAbout;
    Preferences dlgPreferences;
    MidiSetup dlgMidiSetup;
    KMapDialog dlgKeyMap;

    QSpinBox* m_sboxChannel;
    QSpinBox* m_sboxOctave;
    QSpinBox* m_sboxTranspose;
    Knob* m_Velocity;
    QComboBox* m_comboControl;
    Knob* m_Control;
    QSlider* m_bender;
    QComboBox* m_comboBank;
    QComboBox* m_comboProg;
    QStyle* m_dialStyle;
    Instrument* m_ins;
    QMap<int,int> m_ctlState, m_ctlSettings;
    QStringList m_extraControls;
    int m_lastBank;
    int m_lastProg;
    int m_lastCtl;
    int m_channel;
    int m_velocity;
    int m_baseOctave;
    int m_transpose;
};

#endif // VPIANO_H

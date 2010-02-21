/*
    MIDI Virtual Piano Keyboard
    Copyright (C) 2008-2010, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
#include "mididefs.h"

class QComboBox;
class QSpinBox;
class QSlider;
class QStyle;
class Knob;
class Instrument;
class RtMidiIn;
class RtMidiOut;
class About;
class Preferences;
class MidiSetup;
class KMapDialog;
class DialogExtraControls;
class RiffImportDlg;

const QEvent::Type NoteOnEventType = QEvent::Type( QEvent::registerEventType( QEvent::User + STATUS_NOTEON ));
const QEvent::Type NoteOffEventType = QEvent::Type( QEvent::registerEventType( QEvent::User + STATUS_NOTEOFF ));
const QEvent::Type ControllerEventType = QEvent::Type( QEvent::registerEventType( QEvent::User + STATUS_CONTROLLER ));
const QEvent::Type BenderEventType = QEvent::Type( QEvent::registerEventType( QEvent::User + STATUS_BENDER));

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

class BenderEvent : public QEvent
{
public:
    BenderEvent(int value) : QEvent(BenderEventType), m_value(value)
    { }
    int getValue() const { return m_value; }
private:
    int m_value;
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
    void slotController(const int value);
    void slotExtraController(const int value);
    void slotControlClicked(const bool value);
    void slotBender(const int pos);
    void slotBenderReleased();
    void slotBankChanged(const int index);
    void slotProgChanged(const int index);
    void slotBaseOctave(const int octave);
    void slotTranspose(const int transpose);
    void slotChannelChanged(const int channel);
    void slotCtlChanged(const int index);
    void slotHelpContents();
    void slotOpenWebSite();
    void setVelocity(int value);
    void slotImportSF();
    void slotEditExtraControls();
    void slotShowNoteNames();
    //void slotEditPrograms();
    //void slotDebugDestroyed(QObject *obj);

private:
    void initialization();
    bool initMidi();
    void readSettings();
    void writeSettings();
    void applyPreferences();
    void applyConnections();
    void applyInitialSettings();
    void populateInstruments();
    void populateControllers();
    void refreshConnections();
    void initToolBars();
    void clearExtraControllers();
    void initExtraControllers();
    void initControllers(int channel);
    void sendController(const int controller, const int value);
    void resetAllControllers();
    void allNotesOff();
    void bankChange(const int bank);
    void programChange(const int program);
    void bender(const int value);
    void messageWrapper(std::vector<unsigned char> *message) const;
    void updateController(int ctl, int val);
    void updateExtraController(int ctl, int val);
    void grabKb();
    void releaseKb();
    void updateStyles();
    void updateNoteNames(bool drums);
    void setWidgetTip(QWidget* w, int val);
    void sendSysex(const QByteArray& data);
    QByteArray readSysexDataFile(const QString& fileName);

    About *dlgAbout();
    Preferences *dlgPreferences();
    MidiSetup *dlgMidiSetup();
    KMapDialog *dlgKeyMap();
    DialogExtraControls *dlgExtra();
    RiffImportDlg *dlgRiffImport();

    RtMidiOut* m_midiout;
    RtMidiIn* m_midiin;
    int m_currentOut;
    int m_currentIn;
    bool m_inputActive;
    bool m_midiThru;
    bool m_initialized;

    About *m_dlgAbout;
    Preferences *m_dlgPreferences;
    MidiSetup *m_dlgMidiSetup;
    KMapDialog *m_dlgKeyMap;
    DialogExtraControls *m_dlgExtra;
    RiffImportDlg *m_dlgRiffImport;

    Ui::VPiano ui;

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
    QStringList m_extraControls;
    QMap<int,QMap<int,int> > m_ctlState, m_ctlSettings;
    QMap<int,int> m_lastBank;
    QMap<int,int> m_lastProg;
    QMap<int,int> m_lastCtl;
    int m_channel;
    int m_velocity;
    int m_baseOctave;
    int m_transpose;
};

#endif // VPIANO_H

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

#include <QString>
#include <QSettings>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QSlider>
#include <QSpinBox>
#include <QDial>

#include "vpiano.h"
#include "instrument.h"
#include "mididefs.h"
#include "knob.h"
#include "classicstyle.h"
#include "RtMidi.h"
#include "constants.h"
#include "riffimportdlg.h"

VPiano::VPiano( QWidget * parent, Qt::WindowFlags flags )
    : QMainWindow(parent, flags),
    m_midiout(0),
    m_midiin(0),
    m_currentOut(-1),
    m_currentIn(-1),
    m_inputActive(false),
    m_midiThru(false)
{
    ui.setupUi(this);
    ui.actionStatusBar->setChecked(false);
    connect(ui.actionAbout, SIGNAL(triggered()), SLOT(slotAbout()));
    connect(ui.actionAbout_Qt, SIGNAL(triggered()), SLOT(slotAboutQt()));
    connect(ui.actionConnections, SIGNAL(triggered()), SLOT(slotConnections()));
    connect(ui.actionPreferences, SIGNAL(triggered()), SLOT(slotPreferences()));
    connect(ui.actionLoadKM, SIGNAL(triggered()), SLOT(slotLoadKeyboardMap()));
    connect(ui.actionSaveKM, SIGNAL(triggered()), SLOT(slotSaveKeyboardMap()));
    connect(ui.actionEditKM, SIGNAL(triggered()), SLOT(slotEditKeyboardMap()));
    connect(ui.actionContents, SIGNAL(triggered()), SLOT(slotHelpContents()));
    connect(ui.actionWebSite, SIGNAL(triggered()), SLOT(slotOpenWebSite()));
    connect(ui.actionImportSoundFont, SIGNAL(triggered()), SLOT(slotImportSF()));
    ui.pianokeybd->setPianoHandler(this);
    initialization();
}

VPiano::~VPiano()
{
    try {
        if (m_midiout != NULL) {
            m_midiout->closePort();
            delete m_midiout;
        }
        if (m_midiin != NULL) {
            if (m_inputActive) {
                m_midiin->cancelCallback();
                m_inputActive = false;
            }
            if (m_currentIn > -1)
                m_midiin->closePort();
            delete m_midiin;
        }
    } catch (RtError& err) {
        qWarning() << QString::fromStdString(err.getMessage());
    }
}

void VPiano::initialization()
{
    if (initMidi()) {
        refreshConnections();
        readSettings();
        initToolBars();
        applyPreferences();
        applyConnections();
        applyInitialSettings();
    } else close();
}

int VPiano::getInputChannel()
{
    return m_channel;
}

void midiCallback( double /*deltatime*/,
                   std::vector< unsigned char > *message,
                   void *userData )
{
    QEvent* ev = NULL;
    VPiano* instance = static_cast<VPiano*>(userData);
    unsigned char status = message->at(0) & MASK_STATUS;
    if ((status == STATUS_NOTEON) || (status == STATUS_NOTEOFF)) {
        unsigned char channel = message->at(0) & MASK_CHANNEL;
        unsigned char channelFilter = instance->getInputChannel();
        if (channel == channelFilter) {
            unsigned char midi_note = message->at(1);
            unsigned char vel = message->at(2);
            if ((status == STATUS_NOTEOFF) || (vel == 0))
                ev = new NoteOffEvent(midi_note);
            else
                ev = new NoteOnEvent(midi_note);
        }
    } else if (status == STATUS_CONTROLLER) {
        unsigned char channel = message->at(0) & MASK_CHANNEL;
        unsigned char channelFilter = instance->getInputChannel();
        if (channel == channelFilter) {
            unsigned char ctl = message->at(1);
            unsigned char val = message->at(2);
            ev = new ControllerEvent(ctl, val);
        }
    }
    instance->midiThru(message);
    if (ev != NULL)
        QApplication::postEvent(instance, ev);
}

bool VPiano::initMidi()
{
    try {
        m_midiout = new RtMidiOut(QSTR_VMPKOUTPUT.toStdString());
        m_midiin = new RtMidiIn(QSTR_VMPKINPUT.toStdString());
#if !defined(__LINUX_ALSASEQ__) && !defined(__MACOSX_CORE__)
        int nOutPorts = m_midiout->getPortCount();
        if (nOutPorts == 0) {
            delete m_midiout;
            m_midiout = 0;
            QMessageBox::critical(this, tr("Error"),
                                  tr("No MIDI output ports available. Aborting"));
            return false;
        }
        int nInPorts = m_midiin->getPortCount();
        if (nInPorts == 0) {
            delete m_midiin;
            m_midiin = NULL;
        }
#endif
#if defined(__LINUX_ALSASEQ__) || defined(__MACOSX_CORE__)
        if (m_midiout != NULL)
            m_midiout->openVirtualPort(QSTR_VMPKOUTPUT.toStdString());
        if (m_midiin != NULL)
            m_midiin->openVirtualPort(QSTR_VMPKINPUT.toStdString());
#else //if defined(__WINDOWS_MM__) || defined(__IRIX_MD__)
        m_midiout->openPort( m_currentOut = 0 );
#endif
        if (m_midiin != NULL) {
            m_midiin->ignoreTypes(true,true,true); //ignore SYX, clock and active sense
            m_midiin->setCallback( &midiCallback, this );
            m_inputActive = true;
        }
    } catch (RtError& err) {
        QMessageBox::critical( this, tr("Error. Aborting"),
                               QString::fromStdString(err.getMessage()));
        return false;
    }
    return true;
}

void VPiano::initToolBars()
{
    m_dialStyle = new ClassicStyle();
    m_dialStyle->setParent(this);
    // Notes tool bar
    ui.toolBarNotes->addWidget(new QLabel(tr(" Channel: "), this));
    m_sboxChannel = new QSpinBox(this);
    m_sboxChannel->setMinimum(1);
    m_sboxChannel->setMaximum(16);
    m_sboxChannel->setValue(m_channel + 1);
    ui.toolBarNotes->addWidget(m_sboxChannel);
    ui.toolBarNotes->addWidget(new QLabel(tr(" Base Octave: "), this));
    m_sboxOctave = new QSpinBox(this);
    m_sboxOctave->setMinimum(0);
    m_sboxOctave->setMaximum(9);
    m_sboxOctave->setValue(m_baseOctave);
    ui.toolBarNotes->addWidget(m_sboxOctave);
    ui.toolBarNotes->addWidget(new QLabel(tr(" Transpose: "), this));
    m_sboxTransport = new QSpinBox(this);
    m_sboxTransport->setMinimum(-11);
    m_sboxTransport->setMaximum(11);
    m_sboxTransport->setValue(m_transport);
    ui.toolBarNotes->addWidget(m_sboxTransport);
    ui.toolBarNotes->addWidget(new QLabel(tr(" Velocity: "), this));
    m_Velocity = new Knob(this);
    m_Velocity->setFixedSize(32, 32);
    m_Velocity->setStyle(dlgPreferences.getStyledKnobs()? m_dialStyle : NULL);
    m_Velocity->setMinimum(0);
    m_Velocity->setMaximum(127);
    m_Velocity->setDefaultValue(100);
    m_Velocity->setDialMode(Knob::LinearMode);
    m_Velocity->setValue(m_velocity);
    ui.toolBarNotes->addWidget(m_Velocity);
    connect( m_sboxChannel, SIGNAL(valueChanged(int)),
             this, SLOT(slotOutChannel(int)) );
    connect( m_sboxOctave, SIGNAL(valueChanged(int)),
             this, SLOT(slotBaseOctave(int)) );
    connect( m_sboxTransport, SIGNAL(valueChanged(int)),
             this, SLOT(slotTransport(int)) );
    connect( m_Velocity, SIGNAL(valueChanged(int)),
             this, SLOT(setVelocity(int)) );
    // Controllers tool bar
    ui.toolBarControllers->addWidget(new QLabel(tr(" Control: "), this));
    m_comboControl = new QComboBox(this);
    m_comboControl->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    ui.toolBarControllers->addWidget(m_comboControl);
    ui.toolBarControllers->addWidget(new QLabel(tr(" Value: "), this));
    m_Control= new Knob(this);
    m_Control->setFixedSize(32, 32);
    m_Control->setStyle(dlgPreferences.getStyledKnobs()? m_dialStyle : NULL);
    m_Control->setMinimum(0);
    m_Control->setMaximum(127);
    m_Control->setValue(0);
    m_Control->setDefaultValue(0);
    m_Control->setDialMode(Knob::LinearMode);
    ui.toolBarControllers->addWidget(m_Control);
    connect( m_comboControl, SIGNAL(currentIndexChanged(int)), SLOT(slotCtlChanged(int)) );
    connect( m_Control, SIGNAL(valueChanged(int)), SLOT(slotController()) );
    // Pitch bender tool bar
    ui.toolBarBender->addWidget(new QLabel(tr(" Bender: "), this));
    m_bender = new QSlider(this);
    m_bender->setOrientation(Qt::Horizontal);
    m_bender->setMaximumWidth(200);
    m_bender->setMinimum(BENDER_MIN);
    m_bender->setMaximum(BENDER_MAX);
    m_bender->setValue(0);
    ui.toolBarBender->addWidget(m_bender);
    connect( m_bender, SIGNAL(valueChanged(int)), SLOT(slotBender()) );
    connect( m_bender, SIGNAL(sliderReleased()), SLOT(slotBenderReleased()) );
    // Programs tool bar
    ui.toolBarPrograms->addWidget(new QLabel(tr(" Bank: "), this));
    m_comboBank = new QComboBox(this);
    m_comboBank->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    ui.toolBarPrograms->addWidget(m_comboBank);
    ui.toolBarPrograms->addWidget(new QLabel(tr(" Program: "), this));
    m_comboProg = new QComboBox(this);
    m_comboProg->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    ui.toolBarPrograms->addWidget(m_comboProg);
    connect( m_comboBank, SIGNAL(currentIndexChanged(int)), SLOT(slotBankChanged(int)) );
    connect( m_comboProg, SIGNAL(currentIndexChanged(int)), SLOT(slotProgChanged(int)) );
    // Toolbars actions: toggle view
    connect(ui.toolBarNotes->toggleViewAction(), SIGNAL(toggled(bool)),
            ui.actionNotes, SLOT(setChecked(bool)));
    connect(ui.toolBarControllers->toggleViewAction(), SIGNAL(toggled(bool)),
            ui.actionControllers, SLOT(setChecked(bool)));
    connect(ui.toolBarBender->toggleViewAction(), SIGNAL(toggled(bool)),
            ui.actionBender, SLOT(setChecked(bool)));
    connect(ui.toolBarPrograms->toggleViewAction(), SIGNAL(toggled(bool)),
            ui.actionPrograms, SLOT(setChecked(bool)));
    // Toolbars actions: buttons
    connect(ui.actionPanic, SIGNAL(triggered()), SLOT(slotPanic()));
    connect(ui.actionResetAll, SIGNAL(triggered()), SLOT(slotResetAllControllers()));
    connect(ui.actionReset, SIGNAL(triggered()), SLOT(slotResetBender()));
}

void VPiano::readSettings()
{
    QSettings settings;

    settings.beginGroup(QSTR_WINDOW);
    restoreGeometry(settings.value(QSTR_GEOMETRY).toByteArray());
    restoreState(settings.value(QSTR_STATE).toByteArray());
    settings.endGroup();

    settings.beginGroup(QSTR_PREFERENCES);
    m_channel = settings.value(QSTR_CHANNEL, 0).toInt();
    m_velocity = settings.value(QSTR_VELOCITY, 100).toInt();
    m_baseOctave = settings.value(QSTR_BASEOCTAVE, 3).toInt();
    m_transport = settings.value(QSTR_TRANSPORT, 0).toInt();
    int num_octaves = settings.value(QSTR_NUMOCTAVES, 5).toInt();
    QString insFileName = settings.value(QSTR_INSTRUMENTSDEFINITION).toString();
    QString insName = settings.value(QSTR_INSTRUMENTNAME).toString();
    QColor defColor = QApplication::palette().highlight().color();
    QColor keyColor = settings.value(QSTR_KEYPRESSEDCOLOR, defColor).value<QColor>();
    bool grabKb = settings.value(QSTR_GRABKB, false).toBool();
    bool styledKnobs = settings.value(QSTR_STYLEDKNOBS, true).toBool();
    bool alwaysOnTop = settings.value(QSTR_ALWAYSONTOP, false).toBool();
    bool showNames = settings.value(QSTR_SHOWNOTENAMES, false).toBool();
    settings.endGroup();

    dlgPreferences.setNumOctaves(num_octaves);
    dlgPreferences.setKeyPressedColor(keyColor);
    dlgPreferences.setGrabKeyboard(grabKb);
    dlgPreferences.setStyledKnobs(styledKnobs);
    dlgPreferences.setAlwaysOnTop(alwaysOnTop);
    dlgPreferences.setShowNames(showNames);
    if (!insFileName.isEmpty()) {
        dlgPreferences.setInstrumentsFileName(insFileName);
        if (!insName.isEmpty()) {
            dlgPreferences.setInstrumentName(insName);
        }
    }

    settings.beginGroup(QSTR_CONNECTIONS);
    bool inEnabled = settings.value(QSTR_INENABLED, true).toBool();
    bool thruEnabled = settings.value(QSTR_THRUENABLED, false).toBool();
    QString in_port = settings.value(QSTR_INPORT).toString();
    QString out_port = settings.value(QSTR_OUTPORT).toString();
    settings.endGroup();
#if defined(__LINUX_ALSASEQ__) || defined(__MACOSX_CORE__)
    inEnabled = true;
#endif

    if (m_midiin == NULL) {
        dlgMidiSetup.inputNotAvailable();
    } else {
        dlgMidiSetup.setInputEnabled(inEnabled);
        dlgMidiSetup.setThruEnabled(thruEnabled);
        dlgMidiSetup.setCurrentInput(in_port);
    }
    dlgMidiSetup.setCurrentOutput(out_port);

    settings.beginGroup(QSTR_KEYBOARD);
    QString mapFile = settings.value(QSTR_MAPFILE, QSTR_DEFAULT).toString();
    settings.endGroup();

    settings.beginGroup(QSTR_INSTRUMENT);
    m_lastBank = settings.value(QSTR_BANK, -1).toInt();
    m_lastProg = settings.value(QSTR_PROGRAM, 0).toInt();
    m_lastCtl = settings.value(QSTR_CONTROLLER, 1).toInt();
    settings.endGroup();

    settings.beginGroup(QSTR_CONTROLLERS);
    QStringList keys = settings.allKeys();
    QStringList::const_iterator it;
    for(it = keys.constBegin(); it != keys.constEnd(); ++it) {
        int ctl = (*it).toInt();
        int val = settings.value(*it, 0).toInt();
        m_ctlSettings[ctl] = val;
    }
    settings.endGroup();

    if (!mapFile.isEmpty() && (mapFile != QSTR_DEFAULT)) {
        ui.pianokeybd->getKeyboardMap()->loadFromXMLFile(mapFile);
    }
}

void VPiano::writeSettings()
{
    QSettings settings;

    settings.beginGroup(QSTR_WINDOW);
    settings.setValue(QSTR_GEOMETRY, saveGeometry());
    settings.setValue(QSTR_STATE, saveState());
    settings.endGroup();

    settings.beginGroup(QSTR_PREFERENCES);
    settings.setValue(QSTR_CHANNEL, m_channel);
    settings.setValue(QSTR_VELOCITY, m_velocity);
    settings.setValue(QSTR_BASEOCTAVE, m_baseOctave);
    settings.setValue(QSTR_TRANSPORT, m_transport);
    settings.setValue(QSTR_NUMOCTAVES, dlgPreferences.getNumOctaves());
    settings.setValue(QSTR_INSTRUMENTSDEFINITION, dlgPreferences.getInstrumentsFileName());
    settings.setValue(QSTR_INSTRUMENTNAME, dlgPreferences.getInstrumentName());
    settings.setValue(QSTR_KEYPRESSEDCOLOR, dlgPreferences.getKeyPressedColor());
    settings.setValue(QSTR_GRABKB, dlgPreferences.getGrabKeyboard());
    settings.setValue(QSTR_STYLEDKNOBS, dlgPreferences.getStyledKnobs());
    settings.setValue(QSTR_ALWAYSONTOP, dlgPreferences.getAlwaysOnTop());
    settings.setValue(QSTR_SHOWNOTENAMES, dlgPreferences.getShowNames());
    settings.endGroup();

    settings.beginGroup(QSTR_CONNECTIONS);
    settings.setValue(QSTR_INENABLED, dlgMidiSetup.inputIsEnabled());
    settings.setValue(QSTR_THRUENABLED, dlgMidiSetup.thruIsEnabled());
    settings.setValue(QSTR_INPORT,  dlgMidiSetup.selectedInputName());
    settings.setValue(QSTR_OUTPORT, dlgMidiSetup.selectedOutputName());
    settings.endGroup();

    settings.beginGroup(QSTR_KEYBOARD);
    settings.setValue(QSTR_MAPFILE, ui.pianokeybd->getKeyboardMap()->getFileName());
    settings.endGroup();

    settings.beginGroup(QSTR_CONTROLLERS);
    QMap<int,int>::const_iterator it;
    for(it = m_ctlState.constBegin(); it != m_ctlState.constEnd(); ++it) {
        settings.setValue(QString::number(it.key()), it.value());
    }
    settings.endGroup();

    settings.beginGroup(QSTR_INSTRUMENT);
    settings.setValue(QSTR_BANK, m_comboBank->itemData(m_comboBank->currentIndex()).toInt());
    settings.setValue(QSTR_PROGRAM, m_comboProg->itemData(m_comboProg->currentIndex()).toInt());
    settings.setValue(QSTR_CONTROLLER, m_comboControl->itemData(m_comboControl->currentIndex()).toInt());
    settings.endGroup();
}

void VPiano::closeEvent( QCloseEvent *event )
{
    writeSettings();
    event->accept();
}

void VPiano::customEvent ( QEvent *event )
{
    if (event->type() == NoteOnEventType ) {
        NoteOnEvent *ev = static_cast<NoteOnEvent*>(event);
        ui.pianokeybd->showNoteOn(ev->getNote());
        event->accept();
    } else if (event->type() == NoteOffEventType ) {
        NoteOffEvent *ev = static_cast<NoteOffEvent*>(event);
        ui.pianokeybd->showNoteOff(ev->getNote());
        event->accept();
    } else if (event->type() == ControllerEventType ) {
        ControllerEvent *ev = static_cast<ControllerEvent*>(event);
        updateController(ev->getController(), ev->getValue());
        event->accept();
    }
}

void VPiano::showEvent ( QShowEvent *event )
{
    QMainWindow::showEvent(event);
    ui.pianokeybd->setFocus();
    grabKb();
}

void VPiano::hideEvent( QHideEvent *event )
{
    releaseKb();
    QMainWindow::hideEvent(event);
}

void VPiano::midiThru(std::vector<unsigned char> *message) const
{
    if (m_midiThru) {
        try {
            m_midiout->sendMessage( message );
        } catch (RtError& err) {
            qWarning() << QString::fromStdString(err.getMessage());
        }
    }
}

void VPiano::messageWrapper(std::vector<unsigned char> *message) const
{
    try {
        m_midiout->sendMessage( message );
    } catch (RtError& err) {
        ui.statusBar->showMessage(QString::fromStdString(err.getMessage()));
    }
}

void VPiano::noteOn(const int midiNote)
{
    std::vector<unsigned char> message;
    if ((midiNote & MASK_SAFETY) == midiNote) {
        unsigned char chan = static_cast<unsigned char>(m_channel);
        unsigned char vel = static_cast<unsigned char>(m_velocity);
        // Note On: 0x90 + channel, note, vel
        message.push_back(STATUS_NOTEON + (chan & MASK_CHANNEL));
        message.push_back(midiNote & MASK_SAFETY);
        message.push_back(vel & MASK_SAFETY);
        messageWrapper( &message );
    }
}

void VPiano::noteOff(const int midiNote)
{
    std::vector<unsigned char> message;
    if ((midiNote & MASK_SAFETY) == midiNote) {
        unsigned char chan = static_cast<unsigned char>(m_channel);
        unsigned char vel = static_cast<unsigned char>(m_velocity);
        // Note Off: 0x80 + channel, note, vel
        message.push_back(STATUS_NOTEOFF + (chan & MASK_CHANNEL));
        message.push_back(midiNote & MASK_SAFETY);
        message.push_back(vel & MASK_SAFETY);
        messageWrapper( &message );
    }
}

void VPiano::sendController(const int controller, const int value)
{
    std::vector<unsigned char> message;
    unsigned char chan = static_cast<unsigned char>(m_channel);
    unsigned char ctl  = static_cast<unsigned char>(controller);
    unsigned char val  = static_cast<unsigned char>(value);
    // Controller: 0xB0 + channel, ctl, val
    message.push_back(STATUS_CONTROLLER + (chan & MASK_CHANNEL));
    message.push_back(ctl & MASK_SAFETY);
    message.push_back(val & MASK_SAFETY);
    messageWrapper( &message );
}

void VPiano::resetAllControllers()
{
    sendController(CTL_RESET_ALL_CTL, 0);
    int index = m_comboControl->currentIndex();
    int ctl = m_comboControl->itemData(index).toInt();
    initControllers();
    m_comboControl->setCurrentIndex(index);
    m_Control->setValue(m_ctlState[ctl]);
}

void VPiano::allNotesOff()
{
    sendController(CTL_ALL_NOTES_OFF, 0);
    ui.pianokeybd->allKeysOff();
}

void VPiano::programChange(const int program)
{
    std::vector<unsigned char> message;
    unsigned char chan = static_cast<unsigned char>(m_channel);
    unsigned char pgm  = static_cast<unsigned char>(program);
    // Program: 0xC0 + channel, pgm
    message.push_back(STATUS_PROGRAM + (chan & MASK_CHANNEL));
    message.push_back(pgm & MASK_SAFETY);
    messageWrapper( &message );
}

void VPiano::bankChange(const int bank)
{
    int method = (m_ins != NULL) ? m_ins->bankSelMethod() : 0;
    int lsb, msb;
    switch (method) {
    case 0:
        lsb = CALC_LSB(bank);
        msb = CALC_MSB(bank);
        sendController(CTL_MSB, msb);
        sendController(CTL_LSB, lsb);
        break;
    case 1:
        sendController(CTL_MSB, bank);
        break;
    case 2:
        sendController(CTL_LSB, bank);
        break;
    default: /* if method is 3 or above, do nothing */
        break;
    }
}

void VPiano::bender(const int value)
{
    std::vector<unsigned char> message;
    int v = value + BENDER_MID; // v >= 0, v <= 16384
    unsigned char chan = static_cast<unsigned char>(m_channel);
    unsigned char lsb  = static_cast<unsigned char>(CALC_LSB(v));
    unsigned char msb  = static_cast<unsigned char>(CALC_MSB(v));
    // Program: 0xE0 + channel, lsb, msb
    message.push_back(STATUS_BENDER + (chan & MASK_CHANNEL));
    message.push_back(lsb);
    message.push_back(msb);
    messageWrapper( &message );
}

void VPiano::slotPanic()
{
    allNotesOff();
}

void VPiano::slotResetAllControllers()
{
    resetAllControllers();
}

void VPiano::slotResetBender()
{
    m_bender->setValue(0);
}

void VPiano::slotController()
{
    int index = m_comboControl->currentIndex();
    int controller = m_comboControl->itemData(index).toInt();
    int value = m_Control->value();
    sendController( controller, value );
    m_ctlState[ controller ] = value;
}

void VPiano::slotBender()
{
    bender(m_bender->value());
}

void VPiano::slotBenderReleased()
{
    m_bender->setValue(0);
}

void VPiano::slotAbout()
{
    releaseKb();
    dlgAbout.exec();
    grabKb();
}

void VPiano::slotAboutQt()
{
    releaseKb();
    qApp->aboutQt();
    grabKb();
}

void VPiano::refreshConnections()
{
    int i = 0, nInPorts = 0, nOutPorts = 0;
    try {
        dlgMidiSetup.clearCombos();
        // inputs
        if (m_midiin == NULL) {
            dlgMidiSetup.inputNotAvailable();
            dlgMidiSetup.setInputEnabled(false);
        } else {
#if !defined(__LINUX_ALSASEQ__) && !defined(__MACOSX_CORE__)
            dlgMidiSetup.setInputEnabled(m_currentIn != -1);
#endif
            dlgMidiSetup.addInputPortName(QString::null, -1);
            nInPorts = m_midiin->getPortCount();
            for ( i = 0; i < nInPorts; i++ ) {
                QString name = QString::fromStdString(m_midiin->getPortName(i));
                if (!name.startsWith(QSTR_VMPK))
                    dlgMidiSetup.addInputPortName(name, i);
            }
        }
        // outputs
        nOutPorts = m_midiout->getPortCount();
        for ( i = 0; i < nOutPorts; i++ ) {
            QString name = QString::fromStdString(m_midiout->getPortName(i));
            if (!name.startsWith(QSTR_VMPK))
                dlgMidiSetup.addOutputPortName(name, i);
        }
    } catch (RtError& err) {
        ui.statusBar->showMessage(QString::fromStdString(err.getMessage()));
    }
}

void VPiano::slotConnections()
{
    refreshConnections();
    dlgMidiSetup.setCurrentInput(m_currentIn);
    dlgMidiSetup.setCurrentOutput(m_currentOut);
    releaseKb();
    if (dlgMidiSetup.exec() == QDialog::Accepted) {
        applyConnections();
    }
    grabKb();
}

void VPiano::applyConnections()
{
    int i, nInPorts = 0, nOutPorts = 0;
    try {
        nOutPorts = m_midiout->getPortCount();
        i = dlgMidiSetup.selectedOutput();
        if ((i >= 0) && (i < nOutPorts) && (i != m_currentOut)) {
            m_midiout->closePort();
            m_midiout->openPort(i);
        }
        m_currentOut = i;
        if (m_midiin != NULL) {
            nInPorts = m_midiin->getPortCount();
            i = dlgMidiSetup.selectedInput();
            if (m_inputActive && (i != m_currentIn)) {
                m_midiin->cancelCallback();
                m_inputActive = false;
                if (m_currentIn > -1)
                    m_midiin->closePort();
            }
            if ((i >= 0) && (i < nInPorts) && (i != m_currentIn) &&
                dlgMidiSetup.inputIsEnabled()) {
                m_midiin->openPort(i);
                m_midiin->setCallback( &midiCallback, this );
                m_inputActive = true;
            }
            m_currentIn = i;
            m_midiThru = dlgMidiSetup.thruIsEnabled();
        }
    } catch (RtError& err) {
        ui.statusBar->showMessage(QString::fromStdString(err.getMessage()));
    }
}

void VPiano::initControllers()
{
    m_comboControl->blockSignals(true);
    m_comboControl->clear();
    if (m_ins != NULL) {
        InstrumentData controls = m_ins->control();
        InstrumentData::ConstIterator i;
        for( i=controls.begin(); i!=controls.end(); ++i ) {
            m_comboControl->addItem(i.value(), i.key());
            //qDebug() << "Control[" << i.key() << "]=" << i.value();
            // initialize controller values
            switch (i.key()) {
            case CTL_VOLUME:
                m_ctlState[CTL_VOLUME] = 100;
                break;
            case CTL_PAN:
                m_ctlState[CTL_PAN] = 64;
                break;
            case CTL_EXPRESSION:
                m_ctlState[CTL_EXPRESSION] = 127;
                break;
            default:
                m_ctlState[i.key()] = 0;
            }
        }
    }
    m_comboControl->blockSignals(false);
}

void VPiano::applyPreferences()
{
    ui.pianokeybd->allKeysOff();

    if (ui.pianokeybd->baseOctave() != m_baseOctave) {
        ui.pianokeybd->setBaseOctave(m_baseOctave);
    }
    if (ui.pianokeybd->numOctaves() != dlgPreferences.getNumOctaves()) {
        ui.pianokeybd->setNumOctaves(dlgPreferences.getNumOctaves());
    }
    ui.pianokeybd->setKeyPressedColor(dlgPreferences.getKeyPressedColor());
    ui.pianokeybd->setShowLabels(dlgPreferences.getShowNames());

    m_ins = NULL;
    m_comboBank->clear();
    m_comboProg->clear();

    if (!dlgPreferences.getInstrumentsFileName().isEmpty() &&
        ((m_ins = dlgPreferences.getInstrument()) != NULL)) {
        //qDebug() << "Instrument Name:" << m_ins->instrumentName();
        //qDebug() << "Bank Selection method: " << m_ins->bankSelMethod();
        m_ctlState.clear();
        initControllers();
        InstrumentPatches patches = m_ins->patches();
        InstrumentPatches::ConstIterator j;
        for( j=patches.begin(); j!=patches.end(); ++j ) {
            //if (j.key() < 0) continue;
            InstrumentData patch = j.value();
            m_comboBank->addItem(patch.name(), j.key());
            //qDebug() << "---- Bank[" << j.key() << "]=" << patch.name();
        }
    }

    QPoint wpos = pos();
    Qt::WindowFlags flags = windowFlags();
    if (dlgPreferences.getAlwaysOnTop())
        flags |= Qt::WindowStaysOnTopHint;
    else
        flags &= ~Qt::WindowStaysOnTopHint;
    setWindowFlags( flags );
    move(wpos);

    updateKnobs();
    show();
}

void VPiano::applyInitialSettings()
{
    int idx;
    QMap<int,int>::Iterator i, j;
    for(i = m_ctlSettings.begin(); i != m_ctlSettings.end(); ++i) {
        j = m_ctlState.find(i.key());
        if (j != m_ctlState.end())
            m_ctlState[i.key()] = i.value();
    }

    for(idx = 0; idx < m_comboControl->count(); ++idx) {
        int ctl = m_comboControl->itemData(idx).toInt();
        if (ctl == m_lastCtl) {
            m_comboControl->setCurrentIndex(idx);
            break;
        }
    }

    for(idx = 0; idx < m_comboBank->count(); ++idx) {
        int bank = m_comboBank->itemData(idx).toInt();
        if (bank == m_lastBank) {
            m_comboBank->setCurrentIndex(idx);
            break;
        }
    }

    for(idx = 0; idx < m_comboProg->count(); ++idx) {
        int pgm = m_comboProg->itemData(idx).toInt();
        if (pgm == m_lastProg) {
            m_comboProg->setCurrentIndex(idx);
            break;
        }
    }
}

void VPiano::slotPreferences()
{
    releaseKb();
    if (dlgPreferences.exec() == QDialog::Accepted) {
        applyPreferences();
    }
    grabKb();
}

QString VPiano::dataDirectory()
{
#ifdef Q_OS_WIN32
    return QApplication::applicationDirPath() + "/";
#endif
#ifdef Q_OS_LINUX
    return QApplication::applicationDirPath() + "/../share/vmpk/";
#endif
#ifdef Q_OS_DARWIN
    return QApplication::applicationDirPath() + "/../Resources/";
#endif
    return QString();
}

void VPiano::slotLoadKeyboardMap()
{

    releaseKb();
    QString fileName = QFileDialog::getOpenFileName(0,
                                tr("Open keyboard map definition"),
                                VPiano::dataDirectory(),
                                tr("Keyboard map (*.xml)"));
    if (!fileName.isEmpty()) {
        ui.pianokeybd->getKeyboardMap()->loadFromXMLFile(fileName);
    }
    grabKb();
}

void VPiano::slotSaveKeyboardMap()
{
    releaseKb();
    QString fileName = QFileDialog::getSaveFileName(this,
                                tr("Save keyboard map definition"),
                                VPiano::dataDirectory(),
                                tr("Keyboard map (*.xml)"));
    if (!fileName.isEmpty()) {
        ui.pianokeybd->getKeyboardMap()->saveToXMLFile(fileName);
    }
    grabKb();
}

void VPiano::slotEditKeyboardMap()
{
    dlgKeyMap.displayMap(ui.pianokeybd->getKeyboardMap());
    releaseKb();
    if (dlgKeyMap.exec() == QDialog::Accepted) {
        ui.pianokeybd->setKeyboardMap(dlgKeyMap.getMap());
    }
    grabKb();
}

void VPiano::slotBankChanged(const int index)
{
    m_comboProg->clear();
    if (index < 0) return;
    int bank = m_comboBank->itemData(index).toInt();
    InstrumentData patch = m_ins->patch(bank);
    InstrumentData::ConstIterator k;
    for( k = patch.begin(); k != patch.end(); ++k ) {
        //qDebug() << "patch[" << k.key() << "]=" << k.value();
        m_comboProg->addItem(k.value(), k.key());
    }
}

void VPiano::slotProgChanged(const int index)
{
    if (index < 0) return;
    int bankIdx = m_comboBank->currentIndex();
    int bank = m_comboBank->itemData(bankIdx).toInt();
    if (bank >= 0)
        bankChange(bank);
    int pgm = m_comboProg->itemData(index).toInt();
    if (pgm >= 0)
        programChange(pgm);
}

void VPiano::slotBaseOctave(const int octave)
{
    if (octave != m_baseOctave) {
        ui.pianokeybd->allKeysOff();
        ui.pianokeybd->setBaseOctave(octave);
        m_baseOctave = octave;
    }
}

void VPiano::slotTransport(const int transport)
{
	if (transport != m_transport) {
		ui.pianokeybd->setTransport(transport);
		m_transport = transport;
	}
}

void VPiano::slotOutChannel(const int channel)
{
    int c = channel - 1;
    if (c != m_channel) {
        m_channel = c;
    }
}

void VPiano::updateController(int ctl, int val)
{
    int index = m_comboControl->currentIndex();
    int controller = m_comboControl->itemData(index).toInt();
    if (controller == ctl) {
        m_Control->setValue(val);
    }
    m_ctlState[ctl] = val;
}

void VPiano::slotCtlChanged(const int index)
{
    int ctl = m_comboControl->itemData(index).toInt();
    m_Control->setValue(m_ctlState[ctl]);
}

void VPiano::grabKb() const
{
    if (dlgPreferences.getGrabKeyboard()) {
        ui.pianokeybd->grabKeyboard();
    }
}

void VPiano::releaseKb() const
{
    if (dlgPreferences.getGrabKeyboard()) {
        ui.pianokeybd->releaseKeyboard();
    }
}

void VPiano::slotHelpContents()
{
    QStringList hlps;
    QLocale loc = QLocale::system();
    QStringList lc = loc.name().split("_");
    hlps += QString("help_%1.html").arg(loc.name());
    if (lc.count() > 1)
        hlps += QString("help_%1.html").arg(lc[0]);
    hlps += "help.html";
    foreach(QString hlp_name, hlps) {
        QString fullName = VPiano::dataDirectory() + hlp_name;
        if (QFile::exists(fullName)) {
            QUrl url = QUrl::fromLocalFile(fullName);
            QDesktopServices::openUrl(url);
            return;
        }
    }
    QMessageBox::critical(this, tr("Error"), tr("No help file found"));
}

void VPiano::slotOpenWebSite()
{
    QUrl url(QSTR_VMPKURL);
    QDesktopServices::openUrl(url);
}

void VPiano::updateKnobs()
{
    QList<Knob *> allKnobs = findChildren<Knob *> ();
    foreach(Knob* knob, allKnobs) {
        knob->setStyle(dlgPreferences.getStyledKnobs() ? m_dialStyle : NULL);
    }
}

void VPiano::slotImportSF()
{
    RiffImportDlg dlg;
    releaseKb();
    if ((dlg.exec() == QDialog::Accepted) && !dlg.getOutput().isEmpty()) {
        dlg.save();
        dlgPreferences.setInstrumentsFileName(dlg.getOutput());
        dlgPreferences.setInstrumentName(dlg.getName());
        applyPreferences();
    }
    grabKb();
}

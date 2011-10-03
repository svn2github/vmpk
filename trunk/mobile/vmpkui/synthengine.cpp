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

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QDebug>
#include <QDesktopServices>
#include <QApplication>

#include "synthengine.h"

const QString QSTR_PREFERENCES("Preferences");
const QString QSTR_NUMOCTAVES("NumOctaves");
const QString QSTR_INSTRUMENTSDEFINITION("InstrumentsDefinition");
const QString QSTR_KEYPRESSEDCOLOR("KeyPressedColor");
const QString QSTR_DATADIR("/opt/vmpkn9/sf2/");
const QString QSTR_SOUNDFONT("VintageDreamsWaves-v2.sf2");
const QString QSTR_INVERTHEME("InvertedTheme");
const QString QSTR_LANGUAGE("Language");
const QString QSTR_DOWNLOADS("/MyDocs/Downloads");

SynthEngine::SynthEngine(QObject *parent)
    : QObject(parent), m_channel(0), m_invertedTheme(false)
{
    keys = new MeeGo::QmKeys(this);
    connect( keys, SIGNAL(keyEvent(MeeGo::QmKeys::Key, MeeGo::QmKeys::State)),
             SLOT(keyEvent(MeeGo::QmKeys::Key, MeeGo::QmKeys::State)));
    resourceSet = new ResourcePolicy::ResourceSet("player", this);
    resourceSet->addResourceObject(new ResourcePolicy::ScaleButtonResource);
    resourceSet->acquire();
    qApp->installEventFilter(this);
    initializeSynth();
    readSettings();
}

SynthEngine::~SynthEngine()
{
    saveSettings();
    ::delete_fluid_audio_driver(m_driver);
    ::delete_fluid_synth(m_synth);
    ::delete_fluid_settings(m_settings);
    resourceSet->release();
}

void SynthEngine::keyEvent(MeeGo::QmKeys::Key key, MeeGo::QmKeys::State state)
{
    if ((MeeGo::QmKeys::KeyDown == state) &&
        (MeeGo::QmKeys::VolumeDown == key ||
         MeeGo::QmKeys::VolumeUp == key)) {
        float g = ::fluid_synth_get_gain(m_synth) * 100.0f;
        g += (key == MeeGo::QmKeys::VolumeUp ? 2.0f : -2.0f);
        if (g < 0) g = 0;
        if (g > 100.0f) g = 100.0f;
        ::fluid_synth_set_gain(m_synth, g / 100.0f);
        emit volumeChanged(g);
    }
}

bool SynthEngine::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::ApplicationDeactivated:
        resourceSet->release();
        break;
    case QEvent::ApplicationActivated:
        resourceSet->acquire();
        break;
    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}

void SynthEngine::initializeSynth()
{
    m_settings = ::new_fluid_settings();
    ::fluid_settings_setstr(m_settings, "audio.driver", "pulseaudio");
    ::fluid_settings_setint(m_settings, "audio.period-size", 1024);
    ::fluid_settings_setint(m_settings, "audio.periods", 2);
    ::fluid_settings_setnum(m_settings, "synth.sample-rate", 12000.0);
    ::fluid_settings_setint(m_settings, "synth.chorus.active", 0);
    ::fluid_settings_setint(m_settings, "synth.reverb.active", 0);
    ::fluid_settings_setnum(m_settings, "synth.gain", 0.4);
    ::fluid_settings_setint(m_settings, "synth.polyphony", 16);
    m_synth = ::new_fluid_synth(m_settings);
    m_driver = ::new_fluid_audio_driver(m_settings, m_synth);
    ::fluid_synth_set_chorus_on(m_synth, 0);
    ::fluid_synth_set_reverb_on(m_synth, 0);
    ::fluid_synth_set_interp_method(m_synth, -1, FLUID_INTERP_LINEAR); // FLUID_INTERP_NONE);
    for (int i=0; i<16; ++i) {
        m_controller[i]=0;
    }
}

void SynthEngine::setChannel(int chan)
{
    if (chan != m_channel) {
        m_channel = chan;
        emit channelChanged();
        emit currentInstrumentChanged();
        emit currentControllerChanged();
    }
}

void SynthEngine::setInstrument(int i)
{
    Preset *bp = dynamic_cast<Preset*>(m_instruments.at(i));
    if (bp != 0) {
        //qDebug() << i << bp->name() << bp->bank() << bp->program();
        ::fluid_synth_bank_select(m_synth, m_channel, bp->bank());
        ::fluid_synth_program_change(m_synth, m_channel, bp->program());
        emit currentInstrumentChanged();
    }
}

void SynthEngine::noteOn(int midiNote, int velocity)
{
    //qDebug() << "NoteOn " << midiNote << " vel: " << velocity; // << " time: " << m_time.elapsed();
    ::fluid_synth_noteon(m_synth, m_channel, midiNote, velocity);
}

void SynthEngine::noteOff(int midiNote, int /*velocity*/)
{
    //qDebug() << "NoteOff " << midiNote << " vel: " << velocity; // << " time: " << m_time.elapsed();
    ::fluid_synth_noteoff(m_synth, m_channel, midiNote);
}

void SynthEngine::loadSoundFont()
{
    fluid_preset_t preset;
    m_instruments.clear();
    emit instrumentsChanged();
    if (m_sfid != -1) {
        ::fluid_synth_sfunload(m_synth, unsigned(m_sfid), 1);
    }
    m_sfid = ::fluid_synth_sfload(m_synth, m_soundFont.toLocal8Bit(), 1);
    if (m_sfid != -1) {
        fluid_sfont_t *pSoundFont = ::fluid_synth_get_sfont_by_id(m_synth, unsigned(m_sfid));
        if (pSoundFont) {
            pSoundFont->iteration_start(pSoundFont);
            while (pSoundFont->iteration_next(pSoundFont, &preset)) {
                int iBank = preset.get_banknum(&preset);
                int iProg = preset.get_num(&preset);
                QString sName = preset.get_name(&preset);
                //qDebug() << iBank << iProg << sName;
                Preset *bp = new Preset(iBank, iProg, sName, this);
                m_instruments.append(bp);
                emit instrumentAdded(sName);
            }
        }
        emit currentInstrumentChanged();
    }
}

void SynthEngine::loadControllers()
{
    m_controllers.clear();
    emit controllersChanged();
    m_controllers << new Controller( 1,   0, tr("1:Modulation"), this);
    m_controllers << new Controller( 7, 100, tr("7:Volume"), this);
    m_controllers << new Controller(10,  64, tr("10:Pan"), this);
    m_controllers << new Controller(64,   0, tr("64:Sustain"), this);
//  m_controllers << new Controller(91,   0, tr("91:Reverb"), this);
//  m_controllers << new Controller(93,   0, tr("93:Chorus"), this);
    foreach(Controller *ctl, m_controllers) {
        emit controllerAdded(ctl->name());
    }
    emit currentControllerChanged();
}

void SynthEngine::initialize()
{
    loadSoundFont();
    loadControllers();
}

QString SynthEngine::currentInstrumentName()
{
    unsigned int sfid, bank, preset;
    ::fluid_synth_get_program(m_synth, m_channel, &sfid, &bank, &preset);
    foreach(Preset *bp, m_instruments) {
        if (bp->bank() == bank && bp->program() == preset)
            return bp->name();
    }
    return QLatin1String("Select...");
}

void SynthEngine::panic()
{
    ::fluid_synth_system_reset(m_synth);
    emit currentInstrumentChanged();
}

void SynthEngine::controlChange(const int value)
{
    int idx = m_controller[m_channel];
    if (idx >= 0 && idx < m_controllers.length())
    {
        Controller *ctl = m_controllers[idx];
        ::fluid_synth_cc(m_synth, m_channel, ctl->control(), value);
        ctl->setValue(m_channel, value);
    }
}

void SynthEngine::bender(const int value)
{
    ::fluid_synth_pitch_bend(m_synth, m_channel, value + 8192);
}

int SynthEngine::currentController()
{
    return m_controller[m_channel];
}

void SynthEngine::setCurrentController(int ctl)
{
    m_controller[m_channel] = ctl;
    emit currentControllerChanged();
}

QString SynthEngine::currentControllerName()
{
    int idx = m_controller[m_channel];
    if (idx >= 0 && idx < m_controllers.length())  {
        return m_controllers[idx]->name();
    }
    return QLatin1String("Select...");
}

int SynthEngine::currentControllerValue()
{
    int idx = m_controller[m_channel];
    if (idx >= 0 && idx < m_controllers.length())  {
        return m_controllers[idx]->value(m_channel);
    }
    return 0;
}

void SynthEngine::setNumberOfOctaves(const int value)
{
    //qDebug() << Q_FUNC_INFO << value;
    if (value != m_numOctaves) {
        m_numOctaves = value;
        emit numberOfOctavesChanged();
    }
}

void SynthEngine::setHighlightColor(const QColor &value)
{
    //qDebug() << Q_FUNC_INFO << value;
    if (value != m_highlightColor) {
        m_highlightColor = value;
        emit highlightColorChanged();
    }
}

void SynthEngine::setSoundFont(const QString &value)
{
    //qDebug() << Q_FUNC_INFO << value;
    if (value != m_soundFont) {
        m_soundFont = value;
        loadSoundFont();
        emit soundFontChanged();
    }
}

void SynthEngine::scanSoundFonts(const QDir &initialDir)
{
    //qDebug() << Q_FUNC_INFO << initialDir.absolutePath();
    QDir dir(initialDir);
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::Name);
    QStringList filters;
    filters << "*.sf2" << "*.SF2";
    QFileInfoList entries= dir.entryInfoList(filters);
    foreach(const QFileInfo &info, entries) {
        QString name = info.absoluteFilePath();
        m_soundFontsList << name;
        emit soundFontAdded(name);
    }
}

void SynthEngine::scanSoundFonts()
{
    m_soundFontsList.clear();
    emit soundFontsChanged();
    QDir globaldir(QSTR_DATADIR);
    scanSoundFonts(globaldir);
    QDir homedir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + QSTR_DOWNLOADS);
    scanSoundFonts(homedir);
}

void SynthEngine::setInvertedTheme(const bool value)
{
    if (value != m_invertedTheme) {
        m_invertedTheme = value;
        emit invertedThemeChanged();
    }
}

void SynthEngine::setLanguage(const QString value)
{
    //qDebug() << Q_FUNC_INFO << value;
    if (value != m_language) {
        m_language = value;
        emit languageChanged();
    }
}

void SynthEngine::initLanguage(const QString value)
{
    //qDebug() << Q_FUNC_INFO << value;
    if (!value.isEmpty() && m_language.isEmpty()) {
        m_language = value;
        emit languageChanged();
    }
}

void SynthEngine::readSettings()
{
    //qDebug() << Q_FUNC_INFO;
    QDir dir(QSTR_DATADIR);
    QFileInfo sf2(dir, QSTR_SOUNDFONT);
    QString defSoundFont;
    if (sf2.exists()) {
        defSoundFont = sf2.absoluteFilePath();
    }
    QColor defColor("purple");
    m_sfid = -1;

    QSettings settings;
    settings.beginGroup(QSTR_PREFERENCES);
    m_numOctaves = settings.value(QSTR_NUMOCTAVES, 2).toInt();
    m_soundFont = settings.value(QSTR_INSTRUMENTSDEFINITION, defSoundFont).toString();
    m_highlightColor = settings.value(QSTR_KEYPRESSEDCOLOR, defColor).value<QColor>();
    m_invertedTheme = settings.value(QSTR_INVERTHEME, false).toBool();
    m_language = settings.value(QSTR_LANGUAGE, QString()).toString();
    settings.endGroup();
}

void SynthEngine::saveSettings()
{
    QSettings settings;
    settings.beginGroup(QSTR_PREFERENCES);
    settings.setValue(QSTR_NUMOCTAVES, m_numOctaves);
    settings.setValue(QSTR_INSTRUMENTSDEFINITION, m_soundFont);
    settings.setValue(QSTR_KEYPRESSEDCOLOR, m_highlightColor);
    settings.setValue(QSTR_INVERTHEME, m_invertedTheme);
    settings.setValue(QSTR_LANGUAGE, m_language);
    settings.endGroup();
    settings.sync();
}

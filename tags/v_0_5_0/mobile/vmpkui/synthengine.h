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

#ifndef SynthEngine_H
#define SynthEngine_H

#include <QObject>
#include <QString>
#include <QList>
#include <QColor>
#include <QDir>
#include <qmkeys.h>
#include <policy/resource-set.h>

#include "fluidsynth.h"
#include "preset.h"
#include "ctl.h"

#define str(s) #s
#define stringify(s) str(s)

class SynthEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int channel READ channel WRITE setChannel NOTIFY channelChanged)
    Q_PROPERTY(QString currentInstrumentName READ currentInstrumentName NOTIFY currentInstrumentChanged)
    Q_PROPERTY(int currentController READ currentController WRITE setCurrentController NOTIFY currentControllerChanged)
    Q_PROPERTY(int currentControllerValue READ currentControllerValue NOTIFY currentControllerChanged)
    Q_PROPERTY(QString currentControllerName READ currentControllerName NOTIFY currentControllerChanged)
    Q_PROPERTY(int numberOfOctaves READ numberOfOctaves WRITE setNumberOfOctaves NOTIFY numberOfOctavesChanged)
    Q_PROPERTY(QColor highlightColor READ highlightColor WRITE setHighlightColor NOTIFY highlightColorChanged)
    Q_PROPERTY(QString soundFont READ soundFont WRITE setSoundFont NOTIFY soundFontChanged)
    Q_PROPERTY(bool invertedTheme READ invertedTheme WRITE setInvertedTheme NOTIFY invertedThemeChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)

public:
    SynthEngine(QObject *parent = 0);
    virtual ~SynthEngine();

    int channel() const { return m_channel; }
    void setChannel(const int chan);

    QString currentInstrumentName();

    int currentController();
    void setCurrentController(int ctl);
    int currentControllerValue();
    QString currentControllerName();

    int numberOfOctaves() const { return m_numOctaves; }
    void setNumberOfOctaves(const int value);

    QColor highlightColor() const { return m_highlightColor; }
    void setHighlightColor(const QColor &value);

    QString soundFont() const { return m_soundFont; }
    void setSoundFont(const QString &value);

    bool invertedTheme() const { return m_invertedTheme; }
    void setInvertedTheme(const bool value);

    QString language() const { return m_language; }
    void setLanguage(const QString value);
    void initLanguage(const QString value);

    Q_INVOKABLE void initialize();
    Q_INVOKABLE void readSettings();
    Q_INVOKABLE void saveSettings();
    Q_INVOKABLE void scanSoundFonts();
    Q_INVOKABLE void panic();
    Q_INVOKABLE void setInstrument(int i);
    Q_INVOKABLE void noteOn(const int midiNote, const int velocity);
    Q_INVOKABLE void noteOff(const int midiNote, const int velocity);
    Q_INVOKABLE void controlChange(const int value);
    Q_INVOKABLE void bender(const int value);
    Q_INVOKABLE QString version() const { return stringify(VERSION); }

public slots:
    void keyEvent(MeeGo::QmKeys::Key key, MeeGo::QmKeys::State state);

signals:
    void instrumentAdded(const QString &name);
    void instrumentsChanged();
    void currentInstrumentChanged();

    void controllerAdded(const QString &name);
    void controllersChanged();
    void currentControllerChanged();

    void soundFontAdded(const QString &name);
    void soundFontsChanged();
    void soundFontChanged();

    void channelChanged();
    void numberOfOctavesChanged();
    void highlightColorChanged();
    void invertedThemeChanged();
    void languageChanged();

    void volumeChanged(float vol);

private:
    virtual bool eventFilter(QObject *watched, QEvent *event);
    void scanSoundFonts(const QDir &dir);
    void initializeSynth();
    void loadSoundFont();
    void loadControllers();

    int m_channel;
    int m_controller[16];
    int m_numOctaves;
    int m_sfid;
    bool m_invertedTheme;
    QColor m_highlightColor;
    QString m_soundFont;
    QString m_defSoundFont;
    fluid_settings_t* m_settings;
    fluid_synth_t* m_synth;
    fluid_audio_driver_t* m_driver;
    QList<Preset*> m_instruments;
    QList<Controller*> m_controllers;
    QStringList m_soundFontsList;
    QString m_language;
    MeeGo::QmKeys *keys;
    ResourcePolicy::ResourceSet *resourceSet;
};

#endif // SynthEngine_H


/*
    MIDI Virtual Piano Keyboard
    Copyright (C) 2008, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>
#include "ui_preferences.h"
#include "instrument.h"

class Preferences : public QDialog
{
    Q_OBJECT

public:
    Preferences(QWidget *parent = 0);
    int getNumOctaves() const { return m_numOctaves; }
    QColor getKeyPressedColor() const { return m_keyPressedColor; }
    bool getGrabKeyboard() const { return m_grabKb; }
    bool getStyledKnobs() const { return m_styledKnobs; }
    void setInstrumentsFileName( const QString fileName );
    QString getInstrumentsFileName();
    void setInstrumentName( const QString name );
    QString getInstrumentName();
    void apply();
    Instrument* getInstrument();

public slots:
    void setNumOctaves(int value) { m_numOctaves = value; }
    void setGrabKeyboard(bool value) { m_grabKb = value; }
    void setStyledKnobs(bool value) { m_styledKnobs = value; }
    void setKeyPressedColor(QColor value);
    void slotButtonClicked(QAbstractButton *button);
    void slotOpenInstrumentFile();
    void slotSelectColor();
    void accept();

protected:
    void showEvent ( QShowEvent *event );

private:
    Ui::PreferencesClass ui;
    QString m_insFileName;
    InstrumentList m_ins;
    int m_numOctaves;
    bool m_grabKb;
    bool m_styledKnobs;
    QColor m_keyPressedColor;
};

#endif // PREFERENCES_H

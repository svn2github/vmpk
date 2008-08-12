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
    int getInChannel() const { return m_inChannel; }
    int getOutChannel() const { return m_outChannel; }
    int getVelocity() const { return m_velocity; }
    int getBaseOctave() const { return m_baseOctave; }
    int getNumOctaves() const { return m_numOctaves; }
    void setInstrumentsFileName( const QString fileName );
    QString getInstrumentsFileName();
    void setInstrumentName( const QString name );
    QString getInstrumentName();
    void apply();
    Instrument* getInstrument();

public slots:
    void setInChannel(int value) { m_inChannel = value; }
    void setOutChannel(int value) { m_outChannel = value; }
    void setVelocity(int value) { m_velocity = value; }
    void setBaseOctave(int value) { m_baseOctave = value; }
    void setNumOctaves(int value) { m_numOctaves = value; }
    void slotButtonClicked(QAbstractButton *button);
    void slotOpenInstrumentFile();
    void accept();

protected:
    void showEvent ( QShowEvent *event ); 

private:
    Ui::PreferencesClass ui;
    QString m_insFileName;
    InstrumentList m_ins;
    int m_inChannel;
    int m_outChannel;
    int m_velocity;
    int m_baseOctave;
    int m_numOctaves;
};

#endif // PREFERENCES_H

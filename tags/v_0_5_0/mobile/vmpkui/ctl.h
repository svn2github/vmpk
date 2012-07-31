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

#ifndef CTL_H
#define CTL_H

#include <QObject>

class Controller : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int control READ control)
    Q_PROPERTY(int defaultValue READ defaultValue)
    Q_PROPERTY(QString name READ name)

public:
    Controller(const int ctl, const int value, const QString & name, QObject *parent) :
        QObject(parent), m_ctl(ctl), m_defaultValue(value), m_name(name)
    {
        for (int i=0; i<16; ++i)
            m_value[i] = m_defaultValue;
    }

    int control() const { return m_ctl; }
    int defaultValue() const { return m_defaultValue; }
    QString name() const { return m_name; }

    int value(const int channel) const
    {
        if (channel >= 0 && channel < 16)
            return m_value[channel];
        return -1;
    }

    void setValue(const int channel, const int value)
    {
        if (channel >= 0 && channel < 16)
            m_value[channel] = value;
    }

private:
    int m_ctl;
    int m_defaultValue;
    QString m_name;
    int m_value[16];
};

#endif // CTL_H

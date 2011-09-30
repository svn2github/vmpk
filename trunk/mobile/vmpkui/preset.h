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

#ifndef PRESET_H
#define PRESET_H

#include <QObject>

class Preset : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(uint bank READ bank)
    Q_PROPERTY(uint program READ program)

public:
    explicit Preset(int b, int p, QString s, QObject *parent = 0):
        QObject(parent), m_name(s), m_bank(b), m_prog(p)
        { }

    QString name() const { return m_name; }
    uint bank() const    { return m_bank; }
    uint program() const { return m_prog; }

private:
    QString m_name;
    uint m_bank;
    uint m_prog;
};


#endif // PRESET_H

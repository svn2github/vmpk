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

#ifndef KEYBOARDMAP_H
#define KEYBOARDMAP_H

#include <QHash>
#include <QIODevice>
#include "constants.h"

class KeyboardMap : public QHash<int, int>  
{
public:
    KeyboardMap() : QHash<int, int>(), m_fileName(QSTR_DEFAULT) {}
    QString loadFromXMLFile(const QString fileName);
    void saveToXMLFile(const QString fileName);
    QString initializeFromXML(QIODevice *dev);
	void serializeToXML(QIODevice *dev);
	const QString& getFileName() const { return m_fileName; }
	
private:
    QString m_fileName;
};

#endif /* KEYBOARDMAP_H */

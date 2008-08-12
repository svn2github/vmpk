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

#include "keyboardmap.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

QString KeyboardMap::loadFromXMLFile(const QString fileName)
{
    QFile f(m_fileName = fileName);
    f.open(QFile::ReadOnly | QFile::Text);
    QString res = initializeFromXML(&f);
    f.close();
    return res;
}

void KeyboardMap::saveToXMLFile(const QString fileName)
{
    QFile f(m_fileName = fileName);
    f.open(QFile::WriteOnly | QFile::Text);
    serializeToXML(&f);
    f.close();
}

QString KeyboardMap::initializeFromXML(QIODevice *dev)
{
    QXmlStreamReader reader(dev);
    clear();
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            if (reader.name() == "keyboardmap") {
                reader.readNext();
                while (reader.isWhitespace()) 
                    reader.readNext();
                while (reader.isStartElement()) {
                    if (reader.name() == "mapping") {
                        QString key = reader.attributes().value("key").toString();
                        QString sn = reader.attributes().value("note").toString();
                        bool ok = false;
                        int note = sn.toInt(&ok);
                        if (ok) {
                            insert(key, note);
                        }
                    }
                    reader.readNext();
                    while (reader.isWhitespace() || reader.isEndElement()) 
                        reader.readNext();
                }
            } else {
                reader.readNext();
            }
        }
    }
    if (reader.hasError()) {
        return reader.errorString();
    }
    return QString();
}

void KeyboardMap::serializeToXML(QIODevice *dev)
{
    QXmlStreamWriter writer(dev);
    writer.setAutoFormatting(true);
    //writer.setCodec("UTF-8");
    writer.writeStartDocument();
    writer.writeDTD("<!DOCTYPE keyboardmap>");
    writer.writeStartElement("keyboardmap");
    writer.writeAttribute("version", "1.0");
    foreach(QKeySequence key, keys()) {
        writer.writeEmptyElement("mapping");
        writer.writeAttribute("key", key.toString(QKeySequence::PortableText));
        writer.writeAttribute("note", QString::number(value(key))); 
    }
    writer.writeEndElement();
    writer.writeEndDocument();
}

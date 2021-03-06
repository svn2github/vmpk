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

#ifndef PIANOKEY_H_
#define PIANOKEY_H_

#include <QGraphicsRectItem>
#include <QBrush>

class PianoKey : public QGraphicsRectItem
{
public:
    PianoKey(QGraphicsItem * parent = 0 ) : QGraphicsRectItem(parent) { }
    PianoKey(const QRectF &rect, const QBrush &brush, const int note); 
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    int getNote() const { return m_note; }

private:
    QBrush m_brush;
    int m_note;
};

#endif /*PIANOKEY_H_*/

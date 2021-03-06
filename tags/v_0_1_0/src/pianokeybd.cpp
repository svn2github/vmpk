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

#include "pianokeybd.h"
#include "pianoscene.h"
#include <QApplication>

PianoKeybd::PianoKeybd(QWidget *parent) 
    : QGraphicsView(parent), m_rotation(0)
{
    initialize();
    initScene(3, 5);
}

PianoKeybd::PianoKeybd(const int baseOctave, const int numOctaves, QWidget *parent) 
    : QGraphicsView(parent), m_rotation(0)
{
    initialize();
    initScene(baseOctave, numOctaves);
}

void PianoKeybd::initScene(int base, int num)
{
    m_scene = new PianoScene(base, num, this);
    m_scene->setKeyboardMap(&m_defaultMap);
    connect(m_scene, SIGNAL(noteOn(int)), SIGNAL(noteOn(int)));
    connect(m_scene, SIGNAL(noteOff(int)), SIGNAL(noteOff(int)));
    setScene(m_scene);
}

void PianoKeybd::initialize()
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(FullViewportUpdate);
    setRenderHints(QPainter::Antialiasing);
    setBackgroundBrush(QApplication::palette().brush(QPalette::Background));
    initDefaultMap();
}

void PianoKeybd::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
}

void PianoKeybd::showNoteOn(int midiNote)
{
    m_scene->showNoteOn(midiNote);
}

void PianoKeybd::showNoteOff(int midiNote)
{
    m_scene->showNoteOff(midiNote);
}

void PianoKeybd::initDefaultMap()
{
    m_defaultMap.insert(QKeySequence(Qt::Key_A), 8);
    m_defaultMap.insert(QKeySequence(Qt::Key_Z), 9);
    m_defaultMap.insert(QKeySequence(Qt::Key_S), 10);
    m_defaultMap.insert(QKeySequence(Qt::Key_X), 11);
    m_defaultMap.insert(QKeySequence(Qt::Key_C), 12);
    m_defaultMap.insert(QKeySequence(Qt::Key_F), 13);
    m_defaultMap.insert(QKeySequence(Qt::Key_V), 14);
    m_defaultMap.insert(QKeySequence(Qt::Key_G), 15);
    m_defaultMap.insert(QKeySequence(Qt::Key_B), 16);
    m_defaultMap.insert(QKeySequence(Qt::Key_N), 17);
    m_defaultMap.insert(QKeySequence(Qt::Key_J), 18);
    m_defaultMap.insert(QKeySequence(Qt::Key_M), 19);
    m_defaultMap.insert(QKeySequence(Qt::Key_K), 20);
    m_defaultMap.insert(QKeySequence(Qt::Key_Comma), 21);
    m_defaultMap.insert(QKeySequence(Qt::Key_L), 22);
    m_defaultMap.insert(QKeySequence(Qt::Key_Period), 23);
    m_defaultMap.insert(QKeySequence(Qt::Key_Slash), 24);
    m_defaultMap.insert(QKeySequence(Qt::Key_Apostrophe), 25);
    m_defaultMap.insert(QKeySequence(Qt::Key_Backslash), 26);
}

void PianoKeybd::setNumOctaves(const int numOctaves)
{
    if (numOctaves != m_scene->numOctaves()) {
        int baseOctave = m_scene->baseOctave();
        delete m_scene;
        initScene(baseOctave, numOctaves);
        fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    }
}

void PianoKeybd::setRotation(int r)
{
    if (r != m_rotation) {
        m_rotation = r;
        resetTransform();
        rotate(m_rotation);
        fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    }
}

QSize PianoKeybd::sizeHint() const 
{ 
    return mapFromScene(sceneRect()).boundingRect().size();
}

void PianoKeybd::allKeysOff()
{
    m_scene->allKeysOff();
}

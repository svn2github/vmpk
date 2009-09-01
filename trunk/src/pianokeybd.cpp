/*
    MIDI Virtual Piano Keyboard
    Copyright (C) 2008-2009, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
#include <QDebug>

#if defined(Q_WS_X11)
#include <QX11Info>
#include <X11/Xlib.h>
#endif

PianoKeybd::PianoKeybd(QWidget *parent)
    : QGraphicsView(parent), m_rawkbd(true), m_rotation(0)
{
    initialize();
    initScene(3, 5);
}

PianoKeybd::PianoKeybd(const int baseOctave, const int numOctaves, QWidget *parent)
    : QGraphicsView(parent), m_rawkbd(true), m_rotation(0)
{
    initialize();
    initScene(baseOctave, numOctaves);
}

void PianoKeybd::initScene(int base, int num, const QColor& c)
{
    m_scene = new PianoScene(base, num, c, this);
    m_scene->setKeyboardMap(&m_defaultMap);
    connect(m_scene, SIGNAL(noteOn(int)), SIGNAL(noteOn(int)));
    connect(m_scene, SIGNAL(noteOff(int)), SIGNAL(noteOff(int)));
    setScene(m_scene);
}

void PianoKeybd::initialize()
{
    setAttribute(Qt::WA_InputMethodEnabled, false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(MinimalViewportUpdate);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setOptimizationFlag(DontClipPainter, true);
    setOptimizationFlag(DontSavePainterState, true);
    //setOptimizationFlag(DontAdjustForAntialiasing, true);
    setBackgroundBrush(QApplication::palette().background());
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
    // Default translated Keyboard Map
    m_defaultMap.insert(Qt::Key_Z, 12);
    m_defaultMap.insert(Qt::Key_S, 13);
    m_defaultMap.insert(Qt::Key_X, 14);
    m_defaultMap.insert(Qt::Key_D, 15);
    m_defaultMap.insert(Qt::Key_C, 16);
    m_defaultMap.insert(Qt::Key_V, 17);
    m_defaultMap.insert(Qt::Key_G, 18);
    m_defaultMap.insert(Qt::Key_B, 19);
    m_defaultMap.insert(Qt::Key_H, 20);
    m_defaultMap.insert(Qt::Key_N, 21);
    m_defaultMap.insert(Qt::Key_J, 22);
    m_defaultMap.insert(Qt::Key_M, 23);
    m_defaultMap.insert(Qt::Key_Q, 24);
    m_defaultMap.insert(Qt::Key_2, 25);
    m_defaultMap.insert(Qt::Key_W, 26);
    m_defaultMap.insert(Qt::Key_3, 27);
    m_defaultMap.insert(Qt::Key_E, 28);
    m_defaultMap.insert(Qt::Key_R, 29);
    m_defaultMap.insert(Qt::Key_5, 30);
    m_defaultMap.insert(Qt::Key_T, 31);
    m_defaultMap.insert(Qt::Key_6, 32);
    m_defaultMap.insert(Qt::Key_Y, 33);
    m_defaultMap.insert(Qt::Key_7, 34);
    m_defaultMap.insert(Qt::Key_U, 35);
    m_defaultMap.insert(Qt::Key_I, 36);
    m_defaultMap.insert(Qt::Key_9, 37);
    m_defaultMap.insert(Qt::Key_O, 38);
    m_defaultMap.insert(Qt::Key_0, 39);
    m_defaultMap.insert(Qt::Key_P, 40);

    // Default Raw Keyboard Map
#if defined(Q_WS_X11)
    m_rawKbMap.insert(94, 11);
    m_rawKbMap.insert(52, 12);
    m_rawKbMap.insert(39, 13);
    m_rawKbMap.insert(53, 14);
    m_rawKbMap.insert(40, 15);
    m_rawKbMap.insert(54, 16);
    m_rawKbMap.insert(55, 17);
    m_rawKbMap.insert(42, 18);
    m_rawKbMap.insert(56, 19);
    m_rawKbMap.insert(43, 20);
    m_rawKbMap.insert(57, 21);
    m_rawKbMap.insert(44, 22);
    m_rawKbMap.insert(58, 23);
    m_rawKbMap.insert(59, 24);
    m_rawKbMap.insert(46, 25);
    m_rawKbMap.insert(60, 26);
    m_rawKbMap.insert(47, 27);
    m_rawKbMap.insert(61, 28);

    m_rawKbMap.insert(24, 24);
    m_rawKbMap.insert(11, 25);
    m_rawKbMap.insert(25, 26);
    m_rawKbMap.insert(12, 27);
    m_rawKbMap.insert(26, 28);
    m_rawKbMap.insert(27, 29);
    m_rawKbMap.insert(14, 30);
    m_rawKbMap.insert(28, 31);
    m_rawKbMap.insert(15, 32);
    m_rawKbMap.insert(29, 33);
    m_rawKbMap.insert(16, 34);
    m_rawKbMap.insert(30, 35);
    m_rawKbMap.insert(31, 36);
    m_rawKbMap.insert(18, 37);
    m_rawKbMap.insert(32, 38);
    m_rawKbMap.insert(19, 39);
    m_rawKbMap.insert(33, 40);
    m_rawKbMap.insert(34, 41);
    m_rawKbMap.insert(21, 42);
    m_rawKbMap.insert(35, 43);
#endif
}

void PianoKeybd::setNumOctaves(const int numOctaves)
{
    if (numOctaves != m_scene->numOctaves()) {
        int baseOctave = m_scene->baseOctave();
        QColor color = m_scene->getKeyPressedColor();
        PianoHandler* handler = m_scene->getPianoHandler();
        KeyboardMap* keyMap = m_scene->getKeyboardMap();
        delete m_scene;
        initScene(baseOctave, numOctaves, color);
        m_scene->setPianoHandler(handler);
        m_scene->setKeyboardMap(keyMap);
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

void PianoKeybd::setRawKeyboardMode(bool b)
{
    if (m_rawkbd != b) {
        m_rawkbd = b;
    }
}

#if defined(Q_WS_X11)

struct qt_auto_repeat_data
{
    // match the window and keycode with timestamp delta of 10 ms
    Window window;
    KeyCode keycode;
    Time timestamp;
    // queue scanner state
    bool release;
    bool error;
};

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static Bool qt_keypress_scanner(Display *, XEvent *event, XPointer arg)
{
    if (event->type != KeyPress && event->type != KeyRelease)
        return false;
    qt_auto_repeat_data *data = (qt_auto_repeat_data *) arg;
    if (data->error)
        return false;
    if (event->xkey.window  != data->window ||
        event->xkey.keycode != data->keycode) {
        // deal breakers: key events in a different window or an event
        // with a different key code
        data->error = true;
        return false;
    }
    if (event->type == KeyPress) {
        data->error = (! data->release || event->xkey.time - data->timestamp > 10);
        return (! data->error);
    }
    // must be XKeyRelease event
    if (data->release) {
        // found a second release
        data->error = true;
        return false;
    }
    // found a single release
    data->release = true;
    data->timestamp = event->xkey.time;
    return false;
}

#if defined(Q_C_CALLBACKS)
}
#endif

bool PianoKeybd::x11Event ( XEvent * event )
{
    if ( m_rawkbd && (event->type == KeyPress || event->type == KeyRelease )) {
        bool autorepeat = false;
        Display *dpy =  QX11Info::display();
        // was this the last auto-repeater?
        qt_auto_repeat_data auto_repeat_data;
        auto_repeat_data.window = event->xkey.window;
        auto_repeat_data.keycode = event->xkey.keycode;
        auto_repeat_data.timestamp = event->xkey.time;
        static uint curr_autorep = 0;
        if (event->type == KeyPress) {
            if (curr_autorep == event->xkey.keycode) {
                autorepeat = true;
                curr_autorep = 0;
            }
        } else {
            // look ahead for auto-repeat
            XEvent nextpress;
            auto_repeat_data.release = true;
            auto_repeat_data.error = false;
            autorepeat = XCheckIfEvent( dpy, &nextpress, &qt_keypress_scanner,
                                        (XPointer) &auto_repeat_data );
            curr_autorep = autorepeat ? event->xkey.keycode : 0;
        }
        if (!autorepeat && m_rawKbMap.contains(event->xkey.keycode)) {
            int note = m_rawKbMap.value(event->xkey.keycode);
            if (event->type == KeyPress)
                m_scene->keyOn(note);
            else
                m_scene->keyOff(note);
        }
        return true;
    }
    return false;
}

#endif

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

#include <QApplication>
#include <QPalette>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QColor>
#include <QDebug>

#include "pianokeyboard.h"

static const int KEYWIDTH = 18;
static const int KEYHEIGHT = 72;

// Default translated Keyboard Map
KeyboardMap *defaultKeyboardMap()
{
    KeyboardMap *tmp = new KeyboardMap;
    tmp->insert(Qt::Key_Z, 0);
    tmp->insert(Qt::Key_S, 1);
    tmp->insert(Qt::Key_X, 2);
    tmp->insert(Qt::Key_D, 3);
    tmp->insert(Qt::Key_C, 4);
    tmp->insert(Qt::Key_V, 5);
    tmp->insert(Qt::Key_G, 6);
    tmp->insert(Qt::Key_B, 7);
    tmp->insert(Qt::Key_H, 8);
    tmp->insert(Qt::Key_N, 9);
    tmp->insert(Qt::Key_J, 10);
    tmp->insert(Qt::Key_M, 11);
    tmp->insert(Qt::Key_Q, 12);
    tmp->insert(Qt::Key_2, 13);
    tmp->insert(Qt::Key_W, 14);
    tmp->insert(Qt::Key_3, 15);
    tmp->insert(Qt::Key_E, 16);
    tmp->insert(Qt::Key_R, 17);
    tmp->insert(Qt::Key_5, 18);
    tmp->insert(Qt::Key_T, 19);
    tmp->insert(Qt::Key_6, 20);
    tmp->insert(Qt::Key_Y, 21);
    tmp->insert(Qt::Key_7, 22);
    tmp->insert(Qt::Key_U, 23);
    tmp->insert(Qt::Key_I, 24);
    tmp->insert(Qt::Key_9, 25);
    tmp->insert(Qt::Key_O, 26);
    tmp->insert(Qt::Key_0, 27);
    tmp->insert(Qt::Key_P, 28);
    return tmp;
}

PianoKeyboard::PianoKeyboard(QDeclarativeItem *parent)
    : QDeclarativeItem( parent ),
      m_baseOctave( 5 ),
      m_numOctaves( 2 ),
      m_minNote( 0 ),
      m_maxNote( 127 ),
      m_transpose( 0 ),
      m_showLabels( false ),
      m_useFlats( false ),
      m_rawkbd( false ),
      m_keyPressedColor( QColor() ),
      m_mousePressed( false ),
      m_velocity( 100 ),
      m_handler( 0 ),
      m_keybdMap( defaultKeyboardMap() )
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);
    setFlag(QGraphicsItem::ItemAcceptsInputMethod, false);
    //setFlag(QGraphicsItem::ItemIsFocusable, true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::MiddleButton | Qt::RightButton);
    setAcceptTouchEvents(true);
    //setFocus(Qt::OtherFocusReason);
    setTransformOrigin(QDeclarativeItem::TopLeft);
    setSmooth(true);
    initialize(m_numOctaves);
}

void PianoKeyboard::clear()
{
    foreach(PianoKey* k, m_keys) {
        delete k;
    }
    m_keys.clear();
    m_labels.clear();
    childItems().clear();
}

void PianoKeyboard::initialize(int octaves)
{
    int i, numkeys = octaves * 12;
    QBrush hilightBrush(m_keyPressedColor.isValid() ? m_keyPressedColor : QApplication::palette().highlight());
    m_numOctaves = octaves;
    for(i = 0; i < numkeys; ++i)
    {
        float x = 0;
        PianoKey* key = 0;
        KeyLabel* lbl = 0;
        int octave = i / 12 * 7;
        int j = i % 12;
        if (j >= 5) j++;
        if ((j % 2) == 0) {
            x = (octave + j / 2) * KEYWIDTH;
            key = new PianoKey( QRectF(x, 0, KEYWIDTH, KEYHEIGHT), false, i, this );
            lbl = new KeyLabel(key);
            lbl->setScale(0.8);
            lbl->setDefaultTextColor(Qt::black);
            lbl->setPos(x - 1, KEYHEIGHT);
        } else {
            x = (octave + j / 2) * KEYWIDTH + KEYWIDTH * 6/10 + 1;
            key = new PianoKey( QRectF( x, 0, KEYWIDTH * 8/10 - 1, KEYHEIGHT * 6/10 ), true, i, this );
            key->setZValue( 1 );
            lbl = new KeyLabel(key);
            lbl->setScale(0.8);
            lbl->setDefaultTextColor(Qt::white);
            lbl->setPos(x - 3, KEYHEIGHT * 6/10 - 3);
        }
        if (m_keyPressedColor.isValid())
            key->setPressedBrush(hilightBrush);
        m_keys.insert(i, key);
        m_labels.insert(i, lbl);
    }
    hideOrShowKeys();
    retranslate();
}

void PianoKeyboard::setNumOctaves(const int octaves)
{
    clear();
    initialize(octaves);
    rescale(boundingRect());
}

QSize PianoKeyboard::sizeHint() const
{
    return QSize(KEYWIDTH * m_numOctaves * 7, KEYHEIGHT);
}

void PianoKeyboard::showKeyOn( PianoKey* key, int vel )
{
    if (vel >= 0 && m_keyPressedColor.isValid() ) {
        QBrush hilightBrush(m_keyPressedColor.lighter(200 - vel));
        key->setPressedBrush(hilightBrush);
    }
    key->setPressed(true);
}

void PianoKeyboard::showKeyOff( PianoKey* key, int )
{
    key->setPressed(false);
}

void PianoKeyboard::showNoteOn( const int note, int vel )
{
    int n = note - m_baseOctave*12 - m_transpose;
    if ((note >= m_minNote) && (note <= m_maxNote) &&
        (n >= 0) && (n < m_keys.size()))
        showKeyOn(m_keys[n], vel);
}

void PianoKeyboard::showNoteOff( const int note, int vel )
{
    int n = note - m_baseOctave*12 - m_transpose;
    if ((note >= m_minNote) && (note <= m_maxNote) &&
        (n >= 0) && (n < m_keys.size()))
        showKeyOff(m_keys[n], vel);
}

void PianoKeyboard::triggerNoteOn( const int note, const int vel )
{
    int n = m_baseOctave*12 + note + m_transpose;
    if ((n >= m_minNote) && (n <= m_maxNote)) {
        if (m_handler != 0) {
            m_handler->noteOn(n, vel);
        } else {
            emit noteOn(n, vel);
        }
    }
}

void PianoKeyboard::triggerNoteOff( const int note, const int vel )
{
    int n = m_baseOctave*12 + note + m_transpose;
    if ((n >= m_minNote) && (n <= m_maxNote)) {
        if (m_handler != 0) {
            m_handler->noteOff(n, vel);
        } else {
            emit noteOff(n, vel);
        }
    }
}

void PianoKeyboard::keyOn( PianoKey* key )
{
    triggerNoteOn(key->getNote(), m_velocity);
    showKeyOn(key, m_velocity);
}

void PianoKeyboard::keyOff( PianoKey* key )
{
    triggerNoteOff(key->getNote(), 0);
    showKeyOff(key, 0);
}

void PianoKeyboard::keyOn( PianoKey* key, qreal pressure )
{
    int vel = m_velocity * pressure;
    triggerNoteOn(key->getNote(), vel);
    showKeyOn(key, vel);
}

void PianoKeyboard::keyOff( PianoKey* key, qreal pressure )
{
    int vel = m_velocity * pressure;
    triggerNoteOff(key->getNote(), vel);
    showKeyOff(key, vel);
}

void PianoKeyboard::keyOn(const int note)
{
    if (note >=0 && note < m_keys.size())
        keyOn(m_keys[note]);
    else
        triggerNoteOn(note, m_velocity);
}

void PianoKeyboard::keyOff(const int note)
{
    if (note >=0 && note < m_keys.size())
        keyOff(m_keys[note]);
    else
        triggerNoteOff(note, m_velocity);
}

PianoKey* PianoKeyboard::getKeyForPos( const QPointF& p ) const
{
    PianoKey* key = 0;
    QList<QGraphicsItem *> items = scene()->items(p, Qt::IntersectsItemShape, Qt::DescendingOrder);
    foreach(QGraphicsItem *itm, items) {
        key = dynamic_cast<PianoKey*>(itm);
        if (key != 0)
            break;
    }
    return key;
}

void PianoKeyboard::mouseMoveEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    //qDebug() << "mouseMove";
    if (m_mousePressed) {
        PianoKey* key = getKeyForPos(mouseEvent->scenePos());
        PianoKey* lastkey = getKeyForPos(mouseEvent->lastScenePos());
        if ((lastkey != 0) && (lastkey != key) && lastkey->isPressed()) {
            keyOff(lastkey);
        }
        if ((key != 0) && !key->isPressed()) {
            keyOn(key);
        }
        mouseEvent->accept();
        return;
    }
    mouseEvent->ignore();
}

void PianoKeyboard::mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    //qDebug() << "mousePress";
    PianoKey* key = getKeyForPos(mouseEvent->scenePos());
    if (key != 0 && !key->isPressed()) {
        keyOn(key);
        m_mousePressed = true;
        mouseEvent->accept();
        return;
    }
    mouseEvent->ignore();
}

void PianoKeyboard::mouseReleaseEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    //qDebug() << "mouseRelease";
    m_mousePressed = false; 
    PianoKey* key = getKeyForPos(mouseEvent->scenePos());
    if (key != 0 && key->isPressed()) {
        keyOff(key);
        mouseEvent->accept();
        return;
    }
    mouseEvent->ignore();
}

int PianoKeyboard::getNoteFromKey( const int key ) const
{
    if (m_keybdMap != 0) {
        KeyboardMap::ConstIterator it = m_keybdMap->constFind(key);
        if ((it != m_keybdMap->constEnd()) && (it.key() == key)) {
            int note = it.value();
            return note;
        }
    }
    return -1;
}

PianoKey* PianoKeyboard::getPianoKey( const int key ) const
{
    int note = getNoteFromKey(key);
    if ((note >= 0) && (note < m_keys.size()))
        return m_keys[note];
    return 0;
}

void PianoKeyboard::keyPressEvent ( QKeyEvent * keyEvent )
{
    if ( !m_rawkbd && !keyEvent->isAutoRepeat() ) { // ignore auto-repeats
        int note = getNoteFromKey(keyEvent->key());
        if (note > -1)
            keyOn(note);
    }
    keyEvent->accept();
}

void PianoKeyboard::keyReleaseEvent ( QKeyEvent * keyEvent )
{
    if ( !m_rawkbd && !keyEvent->isAutoRepeat() ) { // ignore auto-repeats
        int note = getNoteFromKey(keyEvent->key());
        if (note > -1)
            keyOff(note);
    }   
    keyEvent->accept();
}

bool PianoKeyboard::sceneEvent(QEvent *event)
{
    switch(event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchEnd:
    case QEvent::TouchUpdate: {
        QTouchEvent *touchEvent = static_cast<QTouchEvent*>(event);
        QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
        foreach(const QTouchEvent::TouchPoint& touchPoint, touchPoints) {
            switch (touchPoint.state()) {
            case Qt::TouchPointReleased: {
                //qDebug() << "touchpoint released";
                PianoKey* key = getKeyForPos(touchPoint.scenePos());
                if (key != 0 && key->isPressed()) {
                    keyOff(key, touchPoint.pressure());
                }
                break;
            }
            case Qt::TouchPointPressed: {
                //qDebug() << "touchpoint pressed";
                PianoKey* key = getKeyForPos(touchPoint.scenePos());
                if (key != 0 && !key->isPressed()) {
                    keyOn(key, touchPoint.pressure());
                }
                break;
            }
            case Qt::TouchPointMoved: {
                if (touchPoint.pos() != touchPoint.lastPos()) {
                    //qDebug() << "touchpoint moved";
                    PianoKey* key = getKeyForPos(touchPoint.scenePos());
                    PianoKey* lastkey = getKeyForPos(touchPoint.lastScenePos());
                    if ((lastkey != 0) && (lastkey != key) && lastkey->isPressed()) {
                        keyOff(lastkey, touchPoint.pressure());
                    }
                    if ((key != 0) && !key->isPressed()) {
                        keyOn(key, touchPoint.pressure());
                    }
                }
                break;
            }
            default:
                break;
            }
        }
        break;
    }
    default:
        //qDebug() << Q_FUNC_INFO << event;
        return QDeclarativeItem::sceneEvent(event);
    }
    event->accept();
    return true;
}

void PianoKeyboard::allKeysOff()
{
    QList<PianoKey*>::ConstIterator it; 
    for(it = m_keys.constBegin(); it != m_keys.constEnd(); ++it) {
        (*it)->setPressed(false);
    }
}

void PianoKeyboard::setKeyPressedColor(const QColor& color)
{
    if (color.isValid() && (color != m_keyPressedColor)) {
        m_keyPressedColor = color;
        QBrush hilightBrush(m_keyPressedColor);
        foreach(PianoKey* key, m_keys) {
            key->setPressedBrush(hilightBrush);
        }
    }
}

void PianoKeyboard::hideOrShowKeys()
{
    QListIterator<PianoKey*> it(m_keys);
    while(it.hasNext()) {
        PianoKey* key = it.next();
        int n = m_baseOctave*12 + key->getNote() + m_transpose;
        bool b = !(n > m_maxNote) && !(n < m_minNote);
        key->setVisible(b);
    }
}

void PianoKeyboard::setMinNote(const int note)
{
    if (m_minNote != note) {
        m_minNote = note;
        hideOrShowKeys();
    }
}

void PianoKeyboard::setMaxNote(const int note)
{
    if (m_maxNote != note) {
        m_maxNote = note;
        hideOrShowKeys();
    }
}

void PianoKeyboard::setBaseOctave(const int base)
{ 
    if (m_baseOctave != base) {
        m_baseOctave = base;
        hideOrShowKeys();
        refreshLabels();
    }
}

QString PianoKeyboard::noteName(const int note)
{
    int num = (note + m_transpose + 12) % 12;
    int oct = m_baseOctave + ((note + m_transpose) / 12) - 1;
    if (m_noteNames.isEmpty()) {
        QString name;
        if (!m_names_f.isEmpty() && !m_names_s.isEmpty())
            name = m_useFlats ? m_names_f.value(num) : m_names_s.value(num);
        return QString("%1<span style='vertical-align:sub;'>%2</span>").arg(name).arg(oct);
    } else {
        int noteIndex = note + m_transpose + 12*m_baseOctave;
        return QString("<span style='font-size:5pt;'>%1</span>").arg(m_noteNames.value(noteIndex));
    }
}

void PianoKeyboard::refreshLabels()
{
    QListIterator<KeyLabel*> it(m_labels);
    while(it.hasNext()) {
        KeyLabel* lbl = it.next();
        PianoKey* key = dynamic_cast<PianoKey*>(lbl->parentItem());
        if(key != 0) {
            lbl->setHtml(noteName(key->getNote()));
            lbl->setVisible(m_showLabels);
        }
    }
}

void PianoKeyboard::setShowLabels(bool show)
{
    if (m_showLabels != show) {
        m_showLabels = show;
        refreshLabels();
    }
}

void PianoKeyboard::setUseFlats(bool use)
{
    if (m_useFlats != use) {
        m_useFlats = use;
        refreshLabels();
    }
}

void PianoKeyboard::setTranspose(const int transpose)
{
    if (m_transpose != transpose && transpose > -12 && transpose < 12) {
        m_transpose = transpose;
        hideOrShowKeys();
        refreshLabels();
    }
}

/*
void PianoKeyboard::setRawKeyboardMode(bool b)
{
    if (m_rawkbd != b) {
        m_rawkbd = b;
//        RawKeybdApp* rapp = dynamic_cast<RawKeybdApp*>(qApp);
//        if (rapp != 0) rapp->setRawKbdEnable(m_rawkbd);
    }
}
*/

void PianoKeyboard::useCustomNoteNames(const QStringList& names)
{
    m_noteNames = names;
    refreshLabels();
}

void PianoKeyboard::useStandardNoteNames()
{
    m_noteNames.clear();
    refreshLabels();
}

void PianoKeyboard::retranslate()
{
    m_names_s.clear();
    m_names_f.clear();
    m_names_s << trUtf8("C")
              << trUtf8("C\xe2\x99\xaf")
              << trUtf8("D")
              << trUtf8("D\xe2\x99\xaf")
              << trUtf8("E")
              << trUtf8("F")
              << trUtf8("F\xe2\x99\xaf")
              << trUtf8("G")
              << trUtf8("G\xe2\x99\xaf")
              << trUtf8("A")
              << trUtf8("A\xe2\x99\xaf")
              << trUtf8("B");
    m_names_f << trUtf8("C")
              << trUtf8("D\xe2\x99\xad")
              << trUtf8("D")
              << trUtf8("E\xe2\x99\xad")
              << trUtf8("E")
              << trUtf8("F")
              << trUtf8("G\xe2\x99\xad")
              << trUtf8("G")
              << trUtf8("A\xe2\x99\xad")
              << trUtf8("A")
              << trUtf8("B\xe2\x99\xad")
              << trUtf8("B");
    refreshLabels();
}

void PianoKeyboard::rescale(const QRectF &realState)
{
    qreal s1 = realState.width() / (7.0 * KEYWIDTH * m_numOctaves);
    qreal s2 = realState.height() / KEYHEIGHT;
    setScale(qMin(s1, s2));
}

void PianoKeyboard::geometryChanged ( const QRectF & newGeometry,
                                      const QRectF & oldGeometry )
{
    if (newGeometry != oldGeometry &&
        newGeometry.width() > 0 && newGeometry.height() > 0) {
        rescale(newGeometry);
    }
    QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);
}

void PianoKeyboard::setEnabled(bool enabled)
{
    QGraphicsItem::setEnabled(enabled);
}

bool PianoKeyboard::isEnabled()
{
    return QGraphicsItem::isEnabled();
}

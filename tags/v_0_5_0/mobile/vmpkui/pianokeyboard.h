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

#ifndef PIANOSCENE_H_
#define PIANOSCENE_H_

#include "pianokey.h"
#include "keylabel.h"
#include "keyboardmap.h"
#include <QtGui/QGraphicsScene>
#include <QtCore/QHash>
#include <QDeclarativeItem>

class PianoHandler
{
public:
    virtual ~PianoHandler() {}
    virtual void noteOn( const int note, const int vel ) = 0;
    virtual void noteOff( const int note, const int vel ) = 0;
};

class Q_DECLARATIVE_EXPORT PianoKeyboard : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY( int baseOctave READ baseOctave WRITE setBaseOctave )
    Q_PROPERTY( int numOctaves READ numOctaves WRITE setNumOctaves )
    Q_PROPERTY( QColor keyPressedColor READ getKeyPressedColor WRITE setKeyPressedColor )
    Q_PROPERTY( bool showLabels READ showLabels WRITE setShowLabels )
    Q_PROPERTY( bool useFlats READ useFlats WRITE setUseFlats )
    Q_PROPERTY( int transpose READ getTranspose WRITE setTranspose )
    Q_PROPERTY( int velocity READ getVelocity WRITE setVelocity )
    Q_PROPERTY( bool enabled READ isEnabled WRITE setEnabled )

public:
    PianoKeyboard(QDeclarativeItem* parent = 0);
    QSize sizeHint() const;
    void setKeyboardMap( KeyboardMap* map ) { m_keybdMap = map; }
    KeyboardMap* getKeyboardMap() const { return m_keybdMap; }
    PianoHandler* getPianoHandler() const { return m_handler; }
    void setPianoHandler(PianoHandler* handler) { m_handler = handler; }
    QColor getKeyPressedColor() const { return m_keyPressedColor; }
    void setKeyPressedColor(const QColor& color);
    int getMinNote() const { return m_minNote; } 
    void setMinNote(const int note);
    int getMaxNote() const { return m_maxNote; }
    void setMaxNote(const int note);
    int getTranspose() const { return m_transpose; }
    void setTranspose(const int transpose);
    bool showLabels() const { return m_showLabels; }
    void setShowLabels(const bool show);
    bool useFlats() const { return m_useFlats; }
    void setUseFlats(const bool use);
    int baseOctave() const { return m_baseOctave; }
    void setBaseOctave( const int base );
    int numOctaves() const { return m_numOctaves; }
    void setNumOctaves(const int octaves);
    int getVelocity() { return m_velocity; }
    void setVelocity(const int velocity) { m_velocity = velocity; }
    void retranslate();
    void useCustomNoteNames(const QStringList& names);
    void useStandardNoteNames();
    //bool getRawKeyboardMode() const { return m_rawkbd; }
    //void setRawKeyboardMode(const bool b);
    void clear();
    int type() const { return UserType + 100; }

    void showNoteOn( const int note, int vel = -1 );
    void showNoteOff( const int note, int vel = -1 );
    void allKeysOff();
    void keyOn( const int note );
    void keyOff( const int note );

    bool isEnabled();
    void setEnabled ( bool enabled );

signals:
    void noteOn(int n, int v);
    void noteOff(int n, int v);

protected:
    void showKeyOn( PianoKey* key, int vel );
    void showKeyOff( PianoKey* key, int vel );
    void keyOn( PianoKey* key );
    void keyOff( PianoKey* key );
    void keyOn( PianoKey* key, qreal pressure );
    void keyOff( PianoKey* key, qreal pressure );
    PianoKey* getKeyForPos( const QPointF& p ) const;
    PianoKey* getPianoKey( const int key ) const;
    QString noteName(const int note);

    void mouseMoveEvent ( QGraphicsSceneMouseEvent * mouseEvent );
    void mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent );
    void mouseReleaseEvent ( QGraphicsSceneMouseEvent * mouseEvent );
    void keyPressEvent ( QKeyEvent * keyEvent );
    void keyReleaseEvent ( QKeyEvent * keyEvent );
    bool sceneEvent(QEvent *event);
    void geometryChanged ( const QRectF & newGeometry, const QRectF & oldGeometry );

private:
    void initialize(int octaves);
    void rescale(const QRectF &realState);
    void hideOrShowKeys();
    void refreshLabels();
    void triggerNoteOn( const int note, const int vel );
    void triggerNoteOff( const int note, const int vel );
    int getNoteFromKey( const int key ) const;
    
    int m_baseOctave;
    int m_numOctaves;
    int m_minNote;
    int m_maxNote;
    int m_transpose;
    bool m_showLabels;
    bool m_useFlats;
    bool m_rawkbd;
    bool m_enabled;
    QColor m_keyPressedColor;
    bool m_mousePressed;
    int m_velocity;
    PianoHandler* m_handler;
    KeyboardMap* m_keybdMap;
    QList<PianoKey*> m_keys;
    QList<KeyLabel*> m_labels;
    QStringList m_noteNames;
    QStringList m_names_s;
    QStringList m_names_f;
};

#endif /*PIANOSCENE_H_*/

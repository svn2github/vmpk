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

#include "preferences.h"
#include <QPushButton>
#include <QShowEvent>
#include <QFileDialog>

Preferences::Preferences(QWidget *parent)
    : QDialog(parent),
    m_inChannel(0),
    m_outChannel(0),
    m_velocity(100),
    m_baseOctave(4),
    m_numOctaves(5)
{
    ui.setupUi( this );
    connect(ui.buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(slotButtonClicked(QAbstractButton*)));
    connect(ui.btnInstrument, SIGNAL(clicked()), SLOT(slotOpenInstrumentFile()));
}

void Preferences::slotButtonClicked(QAbstractButton *button)
{
    if (dynamic_cast<QPushButton *>(button) == ui.buttonBox->button(QDialogButtonBox::Apply)) {
        apply();
    }
}

void Preferences::showEvent ( QShowEvent *event )
{
    if (event->type() == QEvent::Show) {
        ui.spinInChan->setValue( m_inChannel );
        ui.spinOutChan->setValue( m_outChannel );
        ui.spinVelocity->setValue( m_velocity );
        ui.spinBaseOctave->setValue( m_baseOctave );
        ui.spinNumOctaves->setValue( m_numOctaves );
    }
}

void Preferences::apply()
{
    m_inChannel = ui.spinInChan->value();
    m_outChannel = ui.spinOutChan->value();
    m_velocity = ui.spinVelocity->value();
    m_baseOctave = ui.spinBaseOctave->value();
    m_numOctaves = ui.spinNumOctaves->value();
}

void Preferences::accept()
{
    apply();
    QDialog::accept();
}

void Preferences::slotOpenInstrumentFile() 
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                tr("Open instruments definition"),
                                "", tr("Instrument definitions (*.ins)"));
    if (!fileName.isEmpty()) {
        setInstrumentsFileName(fileName);
    }
}

Instrument* Preferences::getInstrument()
{
    QString key = ui.cboInstrument->currentText();
    if (key.isEmpty())
        return NULL;
    return &m_ins[key];
}

void Preferences::setInstrumentsFileName( const QString fileName )
{
    QFileInfo f(fileName);
    m_ins.clear();
    m_ins.load(fileName);
    ui.txtFileInstrument->setText(f.fileName());
    ui.cboInstrument->clear();
    InstrumentList::ConstIterator it; 
    for(it = m_ins.begin(); it != m_ins.end(); ++it) {
        ui.cboInstrument->addItem(it.key());
    }
    ui.cboInstrument->setCurrentIndex(-1);
    m_insFileName = fileName;
}

QString Preferences::getInstrumentsFileName()
{
    return m_insFileName;
}

void Preferences::setInstrumentName( const QString name )
{
    int index = ui.cboInstrument->findText( name );
    ui.cboInstrument->setCurrentIndex( index );    
}

QString Preferences::getInstrumentName()
{
    return ui.cboInstrument->currentText();    
}

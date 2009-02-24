/*
    MIDI Virtual Piano Keyboard
    Copyright (C) 2008-2009, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
#include "constants.h"
#include "vpiano.h"

#include <QPushButton>
#include <QShowEvent>
#include <QFileDialog>
#include <QColorDialog>

Preferences::Preferences(QWidget *parent)
    : QDialog(parent),
    m_numOctaves(5),
    m_grabKb(false),
    m_styledKnobs(true)
{
    ui.setupUi( this );
    connect(ui.buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(slotButtonClicked(QAbstractButton*)));
    connect(ui.btnInstrument, SIGNAL(clicked()), SLOT(slotOpenInstrumentFile()));
    connect(ui.btnColor, SIGNAL(clicked()), SLOT(slotSelectColor()));
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
        ui.spinNumOctaves->setValue( m_numOctaves );
        ui.chkGrabKb->setChecked( m_grabKb );
        ui.chkStyledKnobs->setChecked( m_styledKnobs );
    }
}

void Preferences::apply()
{
    m_numOctaves = ui.spinNumOctaves->value();
    m_keyPressedColor = QColor(ui.lblColorName->text());
    m_grabKb = ui.chkGrabKb->isChecked();
    m_styledKnobs = ui.chkStyledKnobs->isChecked();
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
                                VPiano::dataDirectory(),
                                tr("Instrument definitions (*.ins)"));
    if (!fileName.isEmpty()) {
        setInstrumentsFileName(fileName);
    }
}

void Preferences::slotSelectColor()
{
    QColor color = QColorDialog::getColor(m_keyPressedColor, this);
    if (color.isValid()) {
        ui.lblColorName->setText(color.name());
        ui.lblColorName->setPalette(QPalette(color));
        ui.lblColorName->setAutoFillBackground(true);
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
    if (f.isReadable()) {
        m_ins.clear();
        ui.cboInstrument->clear();
        if (m_ins.load(fileName)) {
            ui.txtFileInstrument->setText(f.fileName());
            InstrumentList::ConstIterator it;
            for(it = m_ins.begin(); it != m_ins.end(); ++it) {
                ui.cboInstrument->addItem(it.key());
            }
            ui.cboInstrument->setCurrentIndex(-1);
            m_insFileName = fileName;
        } else {
            m_insFileName.clear();
            ui.txtFileInstrument->setText(m_insFileName);
        }
    }
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

void Preferences::setKeyPressedColor(QColor value)
{
    if (m_keyPressedColor != value) {
        m_keyPressedColor = value;
        ui.lblColorName->setText(value.name());
        ui.lblColorName->setPalette(QPalette(value));
        ui.lblColorName->setAutoFillBackground(true);
    }
}

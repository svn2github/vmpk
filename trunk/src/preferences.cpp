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
#include "constants.h"

#include <QPushButton>
#include <QShowEvent>
#include <QFileDialog>
#include <QColorDialog>

Preferences::Preferences(QWidget *parent)
    : QDialog(parent),
    m_inChannel(0),
    m_outChannel(0),
    m_velocity(100),
    m_baseOctave(4),
    m_numOctaves(5),
    m_grabKb(false)
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
        ui.spinInChan->setValue( m_inChannel + 1);
        ui.spinOutChan->setValue( m_outChannel + 1);
        ui.spinVelocity->setValue( m_velocity );
        ui.spinBaseOctave->setValue( m_baseOctave );
        ui.spinNumOctaves->setValue( m_numOctaves );
        ui.chkGrabKb->setChecked( m_grabKb );
    }
}

void Preferences::apply()
{
    m_inChannel = ui.spinInChan->value() - 1;
    m_outChannel = ui.spinOutChan->value() - 1;
    m_velocity = ui.spinVelocity->value();
    m_baseOctave = ui.spinBaseOctave->value();
    m_numOctaves = ui.spinNumOctaves->value();
    m_keyPressedColor = QColor(ui.lblColorName->text());
    m_grabKb = ui.chkGrabKb->isChecked();
}

void Preferences::accept()
{
    apply();
    QDialog::accept();
}

void Preferences::slotOpenInstrumentFile()
{
    QString dataDirectory =
#ifdef Q_OS_WIN32
        QApplication::applicationDirPath();
#endif
#ifdef Q_OS_LINUX
        QApplication::applicationDirPath() + "/../share/vmpk/";
#endif
#ifdef Q_OS_DARWIN
        QApplication::applicationDirPath() + "/../Resources/";
#endif

    QString fileName = QFileDialog::getOpenFileName(this,
                                tr("Open instruments definition"),
                                dataDirectory,
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

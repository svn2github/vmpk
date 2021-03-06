/*
    MIDI Virtual Piano Keyboard
    Copyright (C) 2008-2012, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

#include <QtGui/QPushButton>
#include <QtGui/QShowEvent>
#include <QtGui/QFileDialog>
#include <QtGui/QColorDialog>
#include <QtCore/QDebug>

#ifdef NETWORK_MIDI
#include <QNetworkInterface>
#endif

Preferences::Preferences(QWidget *parent)
    : QDialog(parent),
    m_numOctaves(DEFAULTNUMBEROFOCTAVES),
    m_drumsChannel(MIDIGMDRUMSCHANNEL),
    m_networkPort(NETWORKPORTNUMBER),
    m_grabKb(false),
    m_styledKnobs(true),
    m_alwaysOnTop(false),
    m_rawKeyboard(false),
    m_velocityColor(true)
{
    ui.setupUi( this );
    ui.txtFileInstrument->setText(QSTR_DEFAULT);
    ui.txtFileKmap->setText(QSTR_DEFAULT);
    ui.txtFileRawKmap->setText(QSTR_DEFAULT);
    m_keymap.setRawMode(false);
    m_rawmap.setRawMode(true);
    restoreDefaults();
    connect(ui.btnInstrument, SIGNAL(clicked()), SLOT(slotOpenInstrumentFile()));
    connect(ui.btnColor, SIGNAL(clicked()), SLOT(slotSelectColor()));
    connect(ui.btnSharpColor, SIGNAL(clicked()), SLOT(slotSelectSharpColor()));
    connect(ui.btnKmap, SIGNAL(clicked()), SLOT(slotOpenKeymapFile()));
    connect(ui.btnRawKmap, SIGNAL(clicked()), SLOT(slotOpenRawKeymapFile()));
    QPushButton *btnDefaults = ui.buttonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(btnDefaults, SIGNAL(clicked()), SLOT(slotRestoreDefaults()));
#if !defined(RAWKBD_SUPPORT)
    ui.chkRawKeyboard->setVisible(false);
    ui.lblRawKmap->setVisible(false);
    ui.txtFileRawKmap->setVisible(false);
    ui.btnRawKmap->setVisible(false);
#endif
#if !defined(NETWORK_MIDI)
    ui.lblNetworkPort->setVisible(false);
    ui.txtNetworkPort->setVisible(false);
    ui.lblNetworkIface->setVisible(false);
    ui.cboNetworkIface->setVisible(false);
#else
    ui.cboNetworkIface->clear();
    ui.cboNetworkIface->addItem(QString());
    foreach ( const QNetworkInterface& iface,
              QNetworkInterface::allInterfaces()) {
        if ((iface.flags() & QNetworkInterface::CanMulticast) != 0)
            ui.cboNetworkIface->addItem(iface.name());
    }
#endif
#if defined(SMALL_SCREEN)
    ui.chkRawKeyboard->setVisible(false);
    ui.lblRawKmap->setVisible(false);
    ui.txtFileRawKmap->setVisible(false);
    ui.btnRawKmap->setVisible(false);
    ui.lblKmap->setVisible(false);
    ui.txtFileKmap->setVisible(false);
    ui.btnKmap->setVisible(false);
    ui.chkAlwaysOnTop->setVisible(false);
    ui.chkGrabKb->setVisible(false);
    setWindowState(Qt::WindowActive | Qt::WindowMaximized);
#else
    setMinimumSize(480,320);
    adjustSize();
#endif
}

void Preferences::showEvent ( QShowEvent *event )
{
    if (event->type() == QEvent::Show) {
        ui.spinNumOctaves->setValue( m_numOctaves );
        ui.cboDrumsChannel->setCurrentIndex(m_drumsChannel+1);
        ui.chkGrabKb->setChecked( m_grabKb );
        ui.chkStyledKnobs->setChecked( m_styledKnobs );
        ui.chkAlwaysOnTop->setChecked( m_alwaysOnTop );
        ui.chkRawKeyboard->setChecked( m_rawKeyboard );
        ui.chkVelocityColor->setChecked( m_velocityColor );
        ui.txtNetworkPort->setText( QString::number( m_networkPort ));
        if (!m_keyPressedColor.isValid()) {
            setKeyPressedColor(QApplication::palette().highlight().color());
        }
        if (!m_keyPressedSharpColor.isValid()) {
            setKeyPressedSharpColor(Qt::green);
        }
    }
}

void Preferences::apply()
{
    m_numOctaves = ui.spinNumOctaves->value();
    m_keyPressedColor = QColor(ui.lblColorName->text());
    m_keyPressedSharpColor = QColor(ui.lblSharpColorName->text());
    m_grabKb = ui.chkGrabKb->isChecked();
    m_styledKnobs = ui.chkStyledKnobs->isChecked();
    m_alwaysOnTop = ui.chkAlwaysOnTop->isChecked();
    m_rawKeyboard = ui.chkRawKeyboard->isChecked();
    m_velocityColor = ui.chkVelocityColor->isChecked();
    if ( ui.txtFileRawKmap->text().isEmpty() ||
         ui.txtFileRawKmap->text() == QSTR_DEFAULT)
        m_rawmap.setFileName(QSTR_DEFAULT);
    if ( ui.txtFileKmap->text().isEmpty() ||
         ui.txtFileKmap->text() == QSTR_DEFAULT)
        m_keymap.setFileName(QSTR_DEFAULT);
    if ( ui.txtFileInstrument->text().isEmpty() ||
         ui.txtFileInstrument->text() == QSTR_DEFAULT )
        m_insFileName = QSTR_DEFAULT;
    m_drumsChannel = ui.cboDrumsChannel->currentIndex() - 1;
    m_networkPort = ui.txtNetworkPort->text().toInt();
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
    setKeyPressedColor(color);
}

void Preferences::slotSelectSharpColor()
{
    QColor color = QColorDialog::getColor(m_keyPressedSharpColor, this);
    setKeyPressedSharpColor(color);
}

Instrument* Preferences::getInstrument()
{
    QString key = ui.cboInstrument->currentText();
    if (key.isEmpty())
        return NULL;
    if (!m_ins.contains(key))
        return NULL;
    return &m_ins[key];
}

Instrument* Preferences::getDrumsInstrument()
{
    QString key = ui.cboInstrument->currentText();
    if (key.isEmpty())
        return NULL;
    key.append(" Drums");
    if (!m_ins.contains(key))
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
            for(it = m_ins.constBegin(); it != m_ins.constEnd(); ++it) {
                if(!it.key().endsWith(QLatin1String("Drums"), Qt::CaseInsensitive))
                    ui.cboInstrument->addItem(it.key());
            }
            ui.cboInstrument->setCurrentIndex(-1);
            m_insFileName = fileName;
        } else {
            m_insFileName.clear();
            ui.txtFileInstrument->setText(m_insFileName);
        }
    } else {
        qDebug() << "file" << fileName << "not readable.";
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

void Preferences::setNetworkIface(const QString iface)
{
    int index = ui.cboNetworkIface->findText( iface );
    ui.cboNetworkIface->setCurrentIndex( index );
}

QString Preferences::getNetworkInterface()
{
    return ui.cboNetworkIface->currentText();
}

void Preferences::setKeyPressedColor(QColor value)
{
    if (m_keyPressedColor != value && value.isValid()) {
        m_keyPressedColor = value;
        ui.lblColorName->setText(value.name());
        ui.lblColorName->setPalette(QPalette(value));
        ui.lblColorName->setAutoFillBackground(true);
    }
}

void Preferences::setKeyPressedSharpColor(QColor value)
{
    if (m_keyPressedSharpColor != value && value.isValid()) {
        m_keyPressedSharpColor = value;
        ui.lblSharpColorName->setText(value.name());
        ui.lblSharpColorName->setPalette(QPalette(value));
        ui.lblSharpColorName->setAutoFillBackground(true);
    }
}

void Preferences::slotOpenKeymapFile()
{
    QString fileName = QFileDialog::getOpenFileName(0,
                                tr("Open keyboard map definition"),
                                VPiano::dataDirectory(),
                                tr("Keyboard map (*.xml)"));
    if (!fileName.isEmpty()) {
        setKeyMapFileName(fileName);
    }
}

void Preferences::slotOpenRawKeymapFile()
{
    QString fileName = QFileDialog::getOpenFileName(0,
                                tr("Open keyboard map definition"),
                                VPiano::dataDirectory(),
                                tr("Keyboard map (*.xml)"));
    if (!fileName.isEmpty()) {
        setRawKeyMapFileName(fileName);
    }
}

void Preferences::setRawKeyMapFileName( const QString fileName )
{
    QFileInfo f(fileName);
    if (f.isReadable()) {
        m_rawmap.loadFromXMLFile(fileName);
        ui.txtFileRawKmap->setText(f.fileName());
    } else {
        m_rawmap.clear();
        m_rawmap.setFileName(QSTR_DEFAULT);
    }
}

void Preferences::setKeyMapFileName( const QString fileName )
{
    QFileInfo f(fileName);
    if (f.isReadable()) {
        m_keymap.loadFromXMLFile(fileName);
        ui.txtFileKmap->setText(f.fileName());
    } else {
        m_keymap.clear();
        m_keymap.setFileName(QSTR_DEFAULT);
    }
}

void Preferences::restoreDefaults()
{
    ui.chkAlwaysOnTop->setChecked(false);
    ui.chkGrabKb->setChecked(false);
    ui.chkRawKeyboard->setChecked(false);
    ui.chkStyledKnobs->setChecked(true);
    ui.spinNumOctaves->setValue(DEFAULTNUMBEROFOCTAVES);
    ui.txtFileKmap->setText(QSTR_DEFAULT);
    ui.txtFileRawKmap->setText(QSTR_DEFAULT);
    ui.chkVelocityColor->setChecked(true);
    setInstrumentsFileName(VPiano::dataDirectory() + QSTR_DEFAULTINS);
    ui.cboInstrument->setCurrentIndex(0);
    ui.txtNetworkPort->setText(QString::number(NETWORKPORTNUMBER));
}

void Preferences::slotRestoreDefaults()
{
    restoreDefaults();
    setKeyPressedColor(QApplication::palette().highlight().color());
    setKeyPressedSharpColor(Qt::green);
}

void Preferences::retranslateUi()
{
    ui.retranslateUi(this);
}

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

#include "kmapdialog.h"
#include "vpiano.h"
#include <QFileInfo>
#include <QHeaderView>
#include <QPushButton>
#include <QKeySequence>
#include <QFileDialog>

KMapDialog::KMapDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	QPushButton* btnOpen = ui.buttonBox->addButton(tr("Open..."), QDialogButtonBox::ActionRole);
    QPushButton* btnSave = ui.buttonBox->addButton(tr("Save As..."), QDialogButtonBox::ActionRole);
    btnOpen->setIcon(style()->standardIcon(QStyle::StandardPixmap(QStyle::SP_DialogOpenButton)));
    btnSave->setIcon(style()->standardIcon(QStyle::StandardPixmap(QStyle::SP_DialogSaveButton)));
	connect(btnOpen, SIGNAL(clicked()), SLOT(slotOpen()));
    connect(btnSave, SIGNAL(clicked()), SLOT(slotSave()));
}

void KMapDialog::displayMap(const KeyboardMap* map)
{
    int row;
    if (map != NULL) m_map.copyFrom(map);
    setWindowTitle(m_map.getRawMode() ? tr("Raw Key Map Editor") : tr("Key Map Editor"));
    ui.tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(m_map.getRawMode() ? tr("Key Code") : tr("Key")));
    QFileInfo f(m_map.getFileName());
    ui.labelMapName->setText(f.fileName());
    KeyboardMap::ConstIterator it;
    for(it = m_map.begin(); it != m_map.end(); ++it) {
        row = it.value();
        if (m_map.getRawMode()) {
            ui.tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(it.key())));
        } else {
            QKeySequence ks(it.key());
            ui.tableWidget->setItem(row, 0, new QTableWidgetItem(ks.toString()));
        }
    }
}

void KMapDialog::updateMap()
{
    bool ok;
    m_map.clear();
    QTableWidgetItem* item;
    for( int i=0; i<128; ++i) {
        item = ui.tableWidget->item(i, 0);
        if ((item != NULL) && !item->text().isEmpty()) {
            if (m_map.getRawMode()) {
                int keycode = item->text().toInt(&ok);
                if (ok) m_map.insert(keycode, i);
            } else {
                QKeySequence ks(item->text());
                m_map.insert(ks[0], i);
            }
        }
    }
}

void KMapDialog::getMap(KeyboardMap* map)
{
    updateMap();
    map->copyFrom(&m_map);
}

void KMapDialog::slotOpen()
{
    QString fileName = QFileDialog::getOpenFileName(0,
                                tr("Open keyboard map definition"),
                                VPiano::dataDirectory(),
                                tr("Keyboard map (*.xml)"));
    if (!fileName.isEmpty()) {
        m_map.clear();
        m_map.loadFromXMLFile(fileName);
        displayMap(NULL);
    }
}

void KMapDialog::slotSave()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                tr("Save keyboard map definition"),
                                VPiano::dataDirectory(),
                                tr("Keyboard map (*.xml)"));
    if (!fileName.isEmpty()) {
        updateMap();
        m_map.saveToXMLFile(fileName);
    }
}

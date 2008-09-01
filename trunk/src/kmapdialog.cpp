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

#include "kmapdialog.h"
#include <QFileInfo>
#include <QHeaderView>
#include <QPushButton>
#include <QKeySequence>

KMapDialog::KMapDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
}

void KMapDialog::displayMap(const KeyboardMap* map)
{
    int row;
    QFileInfo f(map->getFileName());
    ui.labelMapName->setText(f.fileName());
    KeyboardMap::ConstIterator it;
    for(it = map->begin(); it != map->end(); ++it) {
        row = it.value();
        QKeySequence ks(it.key());
        ui.tableWidget->setItem(row, 0, new QTableWidgetItem(ks.toString()));
    }
}

KeyboardMap* KMapDialog::getMap()
{
    m_map.clear();
    QTableWidgetItem* item;
    for( int i=0; i<128; ++i) {
        item = ui.tableWidget->item(i, 0);
        if ((item != NULL) && !item->text().isEmpty()) {
            QKeySequence ks(item->text());
            m_map.insert(ks[0], i);
        }
    }
    return &m_map;
}

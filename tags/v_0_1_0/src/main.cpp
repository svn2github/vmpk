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

#include "vpiano.h"

#include <QtGui>
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("vmpk.sourceforge.net");
    QCoreApplication::setOrganizationDomain("vmpk.sourceforge.net");
    QCoreApplication::setApplicationName("Virtual MIDI Piano Keyboard");    
    QApplication a(argc, argv);
    VPiano w;
    w.show();
    return a.exec();
}

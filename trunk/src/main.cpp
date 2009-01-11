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

#include "constants.h"
#include "vpiano.h"

#include <QtGui>
#include <QApplication>
#include <QLibraryInfo>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(QSTR_DOMAIN);
    QCoreApplication::setOrganizationDomain(QSTR_DOMAIN);
    QCoreApplication::setApplicationName(QSTR_APPNAME);
    QApplication a(argc, argv);

    QString localeDirectory =
#ifdef Q_OS_WIN32
        QApplication::applicationDirPath();
#endif
#ifdef Q_OS_LINUX
        QApplication::applicationDirPath() + "/../share/locale/";
#endif
#ifdef Q_OS_DARWIN
        QApplication::applicationDirPath() + "/../Resources/";
#endif
    qDebug() << localeDirectory;
    QTranslator tr_q, tr_p;
    QString loc_q = QSTR_QTPX + QLocale::system().name();
    QString loc_p = QSTR_VMPKPX + QLocale::system().name();
    tr_q.load(loc_q, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    tr_p.load(loc_p, localeDirectory);
    a.installTranslator(&tr_q);
    a.installTranslator(&tr_p);

    VPiano w;
    w.show();
    return a.exec();
}

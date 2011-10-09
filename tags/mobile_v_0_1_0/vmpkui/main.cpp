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

#include <QtGui/QApplication>
#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>
#include <QtCore/QTranslator>
#include <MDeclarativeCache>
#include <MLocale>

#include "pianokeyboard.h"
#include "synthengine.h"

const QString QSTR_DOMAIN("vmpk.sourceforge.net");
const QString QSTR_APPNAME("VMPK-N9");

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(QSTR_DOMAIN);
    QCoreApplication::setOrganizationDomain(QSTR_DOMAIN);
    QCoreApplication::setApplicationName(QSTR_APPNAME);
    QScopedPointer<QApplication> app(MDeclarativeCache::qApplication(argc, argv));
    QScopedPointer<QDeclarativeView> view(MDeclarativeCache::qDeclarativeView());
    qmlRegisterType<SynthEngine>();
    qmlRegisterType<PianoKeyboard>("PianoKeyboard", 0, 1, "PianoKeyboard");
    SynthEngine synthEngine;
    QTranslator appTranslator;
    MLocale loc;
    synthEngine.initLanguage(loc.language());
    if (appTranslator.load("vmpkn9_" + synthEngine.language(), ":/i18n/"))
        app->installTranslator(&appTranslator);
    view->setSource(QUrl("qrc:/qml/main.qml"));
    QObject::connect((QObject*) view->engine(), SIGNAL(quit()), view.data(), SLOT(close()));
    view->engine()->rootContext()->setContextProperty("synthEngine", &synthEngine);
    view->showFullScreen();
    return app->exec();
}

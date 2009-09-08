# Virtual MIDI Piano Keyboard
# Copyright (C) 2008 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along 
# with this program; If not, see <http://www.gnu.org/licenses/>.

TEMPLATE = app
TARGET = vmpk
VERSION = 0.2.7cvs
DESTDIR = build
OBJECTS_DIR = build
UI_DIR = build
MOC_DIR = build
RCC_DIR = build
QT += core gui xml svg
CONFIG += release
CONFIG -= debug_and_release
CONFIG -= debug
VERSIONH = $$sprintf(const QString PGM_VERSION(\"%1\");,$$VERSION)
win32 {
    DEFINES += __WINDOWS_MM__
    LIBS += -lwinmm
    RC_FILE = src/vpianoico.rc
    system(echo $$VERSIONH > $$DESTDIR/vmpk_version.h)
}
linux* { 
    DEFINES += __LINUX_ALSASEQ__
    DEFINES += AVOID_TIMESTAMPING
    CONFIG += link_pkgconfig
    PKGCONFIG += alsa
    system(echo \'$$VERSIONH\' > $$DESTDIR/vmpk_version.h)
}
macx {
    CONFIG += x86 ppc
    ICON = data/vmpk.icns
    DEFINES += __MACOSX_CORE__
    BUNDLE_RES.files = data/help.html \
	  data/help_es.html \
	  data/help_tr.html \
        data/gmgsxg.ins \
        data/spanish.xml \
        data/german.xml \
        data/azerty.xml \
        data/it-qwerty.xml \
        data/vkeybd-default.xml \
        translations/vmpk_es.qm \
        translations/vmpk_tr.qm \
        translations/vmpk_de.qm
    BUNDLE_RES.path = Contents/Resources
    QMAKE_BUNDLE_DATA += BUNDLE_RES
    LIBS += -framework CoreMidi -framework CoreAudio -framework CoreFoundation
    system(echo \'$$VERSIONH\' > $$DESTDIR/vmpk_version.h)
}
irix* { 
    DEFINES += __IRIX_MD__
    LIBS += -laudio -lpthread
    system(echo \'$$VERSIONH\' > $$DESTDIR/vmpk_version.h)
}
debug:DEFINES += __RTMIDI_DEBUG__
INCLUDEPATH += src
# Input
FORMS += src/kmapdialog.ui \
    src/midisetup.ui \
    src/vpiano.ui \
    src/about.ui \
    src/preferences.ui \
    src/riffimportdlg.ui
HEADERS += src/kmapdialog.h \
    src/keyboardmap.h \
    src/mididefs.h \
    src/instrument.h \
    src/midisetup.h \
    src/RtError.h \
    src/RtMidi.h \
    src/knob.h \
    src/keylabel.h \
    src/pianodefs.h \
    src/pianokey.h \
    src/pianokeybd.h \
    src/pianoscene.h \
    src/classicstyle.h \
    src/vpiano.h \
    src/about.h \
    src/preferences.h \
    src/rawkeybdapp.h \
    src/riff.h \
    src/riffimportdlg.h \
    src/instrument.h \
    src/constants.h
SOURCES += src/kmapdialog.cpp \
    src/keyboardmap.cpp \
    src/midisetup.cpp \
    src/RtMidi.cpp \
    src/knob.cpp \
    src/keylabel.cpp \
    src/pianokey.cpp \
    src/pianokeybd.cpp \
    src/pianoscene.cpp \
    src/rawkeybdapp.cpp \
    src/riff.cpp \
    src/riffimportdlg.cpp \
    src/classicstyle.cpp \
    src/vpiano.cpp \
    src/about.cpp \
    src/preferences.cpp \
    src/instrument.cpp \
    src/main.cpp
RESOURCES += data/vmpk.qrc
TRANSLATIONS += translations/vmpk_es.ts \
    translations/vmpk_tr.ts \ 
    translations/vmpk_de.ts \
    translations/vmpk_ca.ts

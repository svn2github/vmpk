TEMPLATE = app
TARGET = vmpk
QT += core \
    gui \
    xml
CONFIG -= release
CONFIG -= debug_and_release
CONFIG += debug
win32 { 
    DEFINES += __WINDOWS_MM__
    LIBS += -lwinmm
    RC_FILE = src/vpianoico.rc
    CONFIG += console
}
linux-g++ { 
    DEFINES += __LINUX_ALSASEQ__
    LIBS += -lasound \
        -lpthread
}
macx-g++ { 
    DEFINES += __MACOSX_CORE__
    LIBS += -framework \
        CoreMidi \
        -framework \
        CoreAudio \
        -framework \
        CoreFoundation
}
irix-g++ { 
    DEFINES += __IRIX_MD__
    LIBS += -laudio \
        -lpthread
}
debug:DEFINES += __RTMIDI_DEBUG__
INCLUDEPATH = src/

# Input
FORMS += src/kmapdialog.ui \
    src/midisetup.ui \
    src/vpiano.ui \
    src/about.ui \
    src/preferences.ui
HEADERS += src/kmapdialog.h \
    src/keyboardmap.h \
    src/mididefs.h \
    src/instrument.h \
    src/midisetup.h \
    src/rterror.h \
    src/rtmidi.h \
    src/knob.h \
    src/pianokey.h \
    src/pianokeybd.h \
    src/pianoscene.h \
    src/classicstyle.h \
    src/vpiano.h \
    src/about.h \
    src/preferences.h \
    src/instrument.h
SOURCES += src/kmapdialog.cpp \
    src/keyboardmap.cpp \
    src/midisetup.cpp \
    src/rtmidi.cpp \
    src/knob.cpp \
    src/pianokey.cpp \
    src/pianokeybd.cpp \
    src/pianoscene.cpp \
    src/classicstyle.cpp \
    src/vpiano.cpp \
    src/about.cpp \
    src/preferences.cpp \
    src/instrument.cpp \
    src/main.cpp
RESOURCES += data/vmpk.qrc

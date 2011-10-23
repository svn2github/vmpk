TEMPLATE = app
TARGET = vmpkn9
VERSION = 0.1.1
LIBS += -L../fluidsynth -lfluidsynth

DEFINES += VERSION=$$VERSION

# Additional import path used to resolve QML modules in Creator's code model
# QML_IMPORT_PATH =

sf2folder.source = sf2
DEPLOYMENTFOLDERS += sf2folder

QT += declarative

CONFIG += link_pkgconfig \
    qdeclarative-boostable \
    qmsystem2

PKGCONFIG += libpulse-simple \
    meegotouchcore \
    libresourceqt1

INCLUDEPATH+=../fluidsynth

SOURCES += main.cpp \
    pianokey.cpp \
    pianokeyboard.cpp \
    synthengine.cpp

HEADERS += keyboardmap.h  \
    keylabel.h  \
    pianokey.h \
    pianokeyboard.h \
    synthengine.h \
    preset.h \
    ctl.h

RESOURCES += pianokeybd.qrc \
    res.qrc

OTHER_FILES += \
    qml/main.qml \
    qml/About.qml \
    qml/BasePage.qml \
    qml/MainPage.qml \
    qml/Preferences.qml \
    qml/Splash.qml \
    qml/PreferencesItem.qml \
    qml/ColorSelectionDialog.qml \
    qml/Help.qml

# Required for deployment.
include(deployment.pri)
qtcAddDeployment()

# enable booster
CONFIG += qdeclarative-boostable
QMAKE_CXXFLAGS += -fPIC -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_LFLAGS += -pie -rdynamic

# meego harmattan
unix:!symbian:!maemo5 {
    target.path = /opt/vmpkn9/bin
    INSTALLS += target
}

# Virtual MIDI Piano Keyboard
# Copyright (C) 2008-2010 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

INCLUDE_DIRECTORIES ( 
    ${CMAKE_CURRENT_BINARY_DIR}
)

LINK_DIRECTORIES (
    ${CMAKE_CURRENT_BINARY_DIR}
)

IF (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/vmpk_version.h)
    FILE ( WRITE ${CMAKE_CURRENT_BINARY_DIR}/vmpk_version.h
           "const QString PGM_VERSION(\"${VERSION}\");\n" )
ENDIF (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/vmpk_version.h)

SET(vmpk_forms_SRCS
    about.ui
    extracontrols.ui
    kmapdialog.ui
    midisetup.ui
    preferences.ui
    riffimportdlg.ui
    vpiano.ui
    shortcutdialog.ui
)

QT4_WRAP_UI(vmpk_ui_SRCS ${vmpk_forms_SRCS})

SET(vmpk_SRCS
    about.cpp
    about.h
    classicstyle.cpp
    classicstyle.h
    constants.h
    extracontrols.cpp
    extracontrols.h
    instrument.cpp
    instrument.h
    keyboardmap.cpp
    keyboardmap.h
    keylabel.cpp
    keylabel.h
    kmapdialog.cpp
    kmapdialog.h
    knob.cpp
    knob.h
    main.cpp
    mididefs.h
    midisetup.cpp
    midisetup.h
    pianodefs.h
    pianokeybd.cpp
    pianokeybd.h
    pianokey.cpp
    pianokey.h
    pianoscene.cpp
    pianoscene.h
    preferences.cpp
    preferences.h
    qticonloader.cpp
    qticonloader.h
    rawkeybdapp.cpp
    rawkeybdapp.h
    riff.cpp
    riff.h
    riffimportdlg.cpp
    riffimportdlg.h
    RtError.h
    RtMidi.cpp
    RtMidi.h
    vpiano.cpp
    vpiano.h
    shortcutdialog.cpp
    shortcutdialog.h
)

QT4_ADD_RESOURCES(vmpk_SRCS ../data/vmpk.qrc)

SET(vmpk_qtobject_SRCS
    about.h
    extracontrols.h
    kmapdialog.h
    knob.h
    midisetup.h
    pianokeybd.h
    pianoscene.h
    preferences.h
    riff.h
    riffimportdlg.h
    vpiano.h
    shortcutdialog.h
)

IF(ENABLE_DBUS)
    QT4_ADD_DBUS_ADAPTOR( vmpk_SRCS 
                          net.sourceforge.vmpk.xml 
                          vpiano.h 
                          VPiano
                          vmpk_adaptor
    )
ENDIF(ENABLE_DBUS)

QT4_WRAP_CPP(vmpk_moc_SRCS ${vmpk_qtobject_SRCS})

IF (UNIX AND NOT APPLE)
    ADD_EXECUTABLE ( vmpk
        ${vmpk_ui_SRCS}
        ${vmpk_moc_SRCS}
        ${vmpk_SRCS} )
    INSTALL ( TARGETS vmpk
              RUNTIME DESTINATION bin )
ENDIF (UNIX AND NOT APPLE)

IF (WIN32)
    SET ( vmpk_RESOURCES )
    CONFIGURE_FILE ( vmpk.rc.in ${CMAKE_CURRENT_BINARY_DIR}/vmpk.rc 
                     IMMEDIATE @ONLY )
    IF ( MINGW )
        ADD_WINDRES_OBJS( vmpk_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/vmpk.rc )
    ENDIF ( MINGW )
    IF ( MSVC )
        SET ( vmpk_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/vmpk.rc )
    ENDIF ( MSVC )
    ADD_EXECUTABLE ( vmpk WIN32
        ${vmpk_ui_SRCS}
        ${vmpk_moc_SRCS}
        ${vmpk_SRCS} 
        ${vmpk_RESOURCES} )
    INSTALL ( TARGETS vmpk
              RUNTIME DESTINATION . )
ENDIF (WIN32)

IF (APPLE)
    SET(MACOSX_BUNDLE_INFO_STRING "Virtual MIDI Piano Keyboard" )
    SET(MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION} )
    SET(MACOSX_BUNDLE_ICON_FILE "vmpk.icns" )
    SET(MACOSX_BUNDLE_GUI_IDENTIFIER "net.sourceforge" )
    SET(MACOSX_BUNDLE_BUNDLE_NAME "vmpk" )
    SET(vmpk_RSC ../data/vmpk.icns
             ../data/help.html
             ../data/help_de.html
             ../data/help_es.html
             ../data/help_nl.html
             ../data/help_ru.html
             ../data/help_tr.html
             ../data/txt2ins.awk
             ../data/gmgsxg.ins
             ../data/spanish.xml
             ../data/german.xml
             ../data/azerty.xml
             ../data/it-qwerty.xml
             ../data/vkeybd-default.xml
             ../data/pc102mac.xml )
    SET_SOURCE_FILES_PROPERTIES(${vmpk_RSC} PROPERTIES
                                MACOSX_PACKAGE_LOCATION Resources)
    ADD_EXECUTABLE ( vmpk MACOSX_BUNDLE
        ${vmpk_ui_SRCS}
        ${vmpk_moc_SRCS}
        ${vmpk_SRCS}
        ${vmpk_RSC} )
ENDIF (APPLE)

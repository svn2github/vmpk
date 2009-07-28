# Virtual MIDI Piano Keyboard
# Copyright (C) 2008-2009 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/vmpk_version.h
     "const QString PGM_VERSION(\"${VERSION}\");\n")

SET(vmpk_forms_SRCS
    about.ui
    kmapdialog.ui
    midisetup.ui
    preferences.ui
    riffimportdlg.ui
    vpiano.ui
)

QT4_WRAP_UI(vmpk_ui_SRCS ${vmpk_forms_SRCS})

SET(vmpk_SRCS
    about.cpp
    about.h
    classicstyle.cpp
    classicstyle.h
    constants.h
    instrument.cpp
    instrument.h
    keyboardmap.cpp
    keyboardmap.h
    kmapdialog.cpp
    kmapdialog.h
    knob.cpp
    knob.h
    main.cpp
    mididefs.h
    midisetup.cpp
    midisetup.h
    pianokeybd.cpp
    pianokeybd.h
    pianokey.cpp
    pianokey.h
    pianoscene.cpp
    pianoscene.h
    preferences.cpp
    preferences.h
    riff.cpp
    riff.h
    riffimportdlg.cpp
    riffimportdlg.h
    RtError.h
    RtMidi.cpp
    RtMidi.h
    vpiano.cpp
    vpiano.h
)

QT4_ADD_RESOURCES(vmpk_SRCS ../data/vmpk.qrc)

SET(vmpk_qtobject_SRCS
    about.h
    kmapdialog.h
    knob.h
    midisetup.h
    pianokeybd.h
    pianoscene.h
    preferences.h
    riff.h
    riffimportdlg.h
    vpiano.h
)

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
    ADD_WINDRES_OBJS(vmpk_SRCS vpianoico.rc)
    ADD_EXECUTABLE ( vmpk WIN32
        ${vmpk_ui_SRCS}
        ${vmpk_moc_SRCS}
        ${vmpk_SRCS} )
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
             ../data/gmgsxg.ins
             ../data/spanish.xml
             ../data/german.xml
             ../data/azerty.xml
             ../data/it-qwerty.xml
             ../data/vkeybd-default.xml)
    SET_SOURCE_FILES_PROPERTIES(${vmpk_RSC} PROPERTIES
                                MACOSX_PACKAGE_LOCATION Resources)
    ADD_EXECUTABLE ( vmpk MACOSX_BUNDLE
        ${vmpk_ui_SRCS}
        ${vmpk_moc_SRCS}
        ${vmpk_SRCS}
        ${vmpk_RSC} )
ENDIF (APPLE)

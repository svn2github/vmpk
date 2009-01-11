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
    rterror.h
    rtmidi.cpp
    rtmidi.h
    vpiano.cpp
    vpiano.h
)

QT4_ADD_RESOURCES(vmpk_SRCS ../data/vmpk.qrc)

IF(WIN32)
    ADD_WINDRES_OBJS(vmpk_SRCS vpianoico.rc)
ENDIF(WIN32)

IF(APPLE)
   SET(MACOSX_BUNDLE_INFO_STRING "Virtual MIDI Piano Keyboard" )
   SET(MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION} )
   SET(MACOSX_BUNDLE_ICON_FILE "vmpk.icns" )
   SET(MACOSX_BUNDLE_GUI_IDENTIFIER "net.sourceforge" )
   SET(MACOSX_BUNDLE_BUNDLE_NAME "vmpk" )
ENDIF(APPLE)

SET(vmpk_qtobject_SRCS
    about.h
    kmapdialog.h
    knob.h
    midisetup.h
    pianokeybd.h
    pianoscene.h
    preferences.h
    vpiano.h
)

QT4_WRAP_CPP(vmpk_moc_SRCS ${vmpk_qtobject_SRCS})

SET(vmpk_RESOURCES
    ../data/vmpk.icns
    ../data/azerty.xml
    ../data/german.xml
    ../data/spanish.xml
    ../data/it-qwerty.xml
    ../data/vkeybd-default.xml
    ../data/gmgsxg.ins
)

ADD_EXECUTABLE ( vmpk WIN32 MACOSX_BUNDLE
    ${vmpk_ui_SRCS}
    ${vmpk_moc_SRCS}
    ${vmpk_SRCS}
    ${vmpk_RESOURCES}
)

IF(APPLE)
    SET_TARGET_PROPERTIES ( vmpk PROPERTIES RESOURCE ${vmpk_RESOURCES} )
ENDIF(APPLE)

INSTALL ( TARGETS vmpk
          RUNTIME DESTINATION bin 
	    BUNDLE DESTINATION bin
          RESOURCE DESTINATION share/vmpk )

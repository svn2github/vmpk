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

IF(WIN32)
    ADD_WINDRES_OBJS(vmpk_SRCS vpianoico.rc)
ENDIF(WIN32)

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

ADD_EXECUTABLE(vmpk
    ${vmpk_ui_SRCS}
    ${vmpk_moc_SRCS}
    ${vmpk_SRCS}
)

# we should install some files...
INSTALL(TARGETS vmpk
    RUNTIME DESTINATION bin)

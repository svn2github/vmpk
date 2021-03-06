# Virtual MIDI Piano Keyboard
# Copyright (C) 2008-2011 Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
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

include_directories (${CMAKE_CURRENT_BINARY_DIR})
link_directories (${CMAKE_CURRENT_BINARY_DIR})

if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/vmpk_version.h)
    file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/vmpk_version.h
          "const QString PGM_VERSION(\"${VERSION}\");\n")
endif (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/vmpk_version.h)

set (vmpk_forms_SRCS
    about.ui
    extracontrols.ui
    kmapdialog.ui
    midisetup.ui
    preferences.ui
    riffimportdlg.ui
    vpiano.ui
    shortcutdialog.ui)

QT4_WRAP_UI (vmpk_ui_SRCS ${vmpk_forms_SRCS})

set (vmpk_SRCS
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
    shortcutdialog.h )

if (HAVE_ALSA)
set (vmpk_SRCS
    ${vmpk_SRCS}
    RtMidiAlsa.cpp
    RtMidiAlsa.h )
endif ()
if (HAVE_JACK)
set (vmpk_SRCS
    ${vmpk_SRCS}
    RtMidiJack.cpp
    RtMidiJack.h )
endif ()
if (HAVE_COREMIDI)
set (vmpk_SRCS
    ${vmpk_SRCS}
    RtMidiCoreMidi.cpp
    RtMidiCoreMidi.h )
endif ()
if (HAVE_IRIX_MD)
set (vmpk_SRCS
    ${vmpk_SRCS}
    RtMidiIrix.cpp
    RtMidiIrix.h )
endif ()
if (HAVE_WINMM)
set (vmpk_SRCS
    ${vmpk_SRCS}
    RtMidiWinMM.cpp
    RtMidiWinMM.h )
endif ()
if (HAVE_NETWORK_MIDI)
set (vmpk_SRCS
    ${vmpk_SRCS}
    RtMidiNet.cpp
    RtMidiNet.h )
endif ()

QT4_ADD_RESOURCES (vmpk_SRCS ../data/vmpk.qrc)

set (vmpk_qtobject_SRCS
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
    shortcutdialog.h)

if (ENABLE_DBUS)
    QT4_ADD_DBUS_ADAPTOR (vmpk_SRCS 
                          net.sourceforge.vmpk.xml 
                          vpiano.h 
                          VPiano
                          vmpk_adaptor)
endif ()

QT4_WRAP_CPP (vmpk_moc_SRCS ${vmpk_qtobject_SRCS})

if (UNIX AND NOT APPLE)
    add_executable (vmpk
					${vmpk_ui_SRCS}
					${vmpk_moc_SRCS}
					${vmpk_SRCS})
    install (TARGETS vmpk
             RUNTIME DESTINATION bin)
endif ()

if (WIN32)
    set (vmpk_RESOURCES)
    configure_file (vmpk.rc.in ${CMAKE_CURRENT_BINARY_DIR}/vmpk.rc 
                    IMMEDIATE @ONLY)
    if (MINGW)
        ADD_WINDRES_OBJS (vmpk_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/vmpk.rc)
    endif ()
    if ( MSVC )
        set (vmpk_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/vmpk.rc)
    endif ()
    add_executable (vmpk WIN32
					${vmpk_ui_SRCS}
					${vmpk_moc_SRCS}
					${vmpk_SRCS} 
					${vmpk_RESOURCES})
    install (TARGETS vmpk
			 RUNTIME DESTINATION .)
endif ()

if (APPLE)
    set (MACOSX_BUNDLE_INFO_STRING "Virtual MIDI Piano Keyboard" )
    set (MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION} )
    set (MACOSX_BUNDLE_ICON_FILE "vmpk.icns" )
    set (MACOSX_BUNDLE_GUI_IDENTIFIER "net.sourceforge" )
    set (MACOSX_BUNDLE_BUNDLE_NAME "vmpk" )
    set (vmpk_RSC ../data/vmpk.icns
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
    set_source_files_properties (${vmpk_RSC} PROPERTIES
                                 MACOSX_PACKAGE_LOCATION Resources)
    add_executable (vmpk MACOSX_BUNDLE
					${vmpk_ui_SRCS}
					${vmpk_moc_SRCS}
					${vmpk_SRCS}
					${vmpk_RSC})
endif ()

set_target_properties (vmpk PROPERTIES OUTPUT_NAME ${PROGRAM_NAME})

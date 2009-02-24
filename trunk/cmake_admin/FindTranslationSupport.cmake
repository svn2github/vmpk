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

IF(LUPDATE_EXECUTABLE)
    SET(LUPDATE_FOUND TRUE)
ELSE(LUPDATE_EXECUTABLE)
    FIND_PROGRAM(LUPDATE_EXECUTABLE
                 NAMES lupdate-qt4 lupdate) 
    IF(LUPDATE_EXECUTABLE)
        SET(LUPDATE_FOUND TRUE)
    ELSE(LUPDATE_EXECUTABLE)
        MESSAGE(FATAL_ERROR "lupdate program couldn't be found")
    ENDIF(LUPDATE_EXECUTABLE)
ENDIF(LUPDATE_EXECUTABLE)

IF(LRELEASE_EXECUTABLE)
    SET(LRELEASE_FOUND TRUE)
ELSE(LRELEASE_EXECUTABLE)
    FIND_PROGRAM(LRELEASE_EXECUTABLE
                 NAMES lrelease-qt4 lrelease) 
    IF(LRELEASE_EXECUTABLE)
        SET(LRELEASE_FOUND TRUE)
    ELSE(LRELEASE_EXECUTABLE)
        MESSAGE(FATAL_ERROR "lrelease program couldn't be found")
    ENDIF(LRELEASE_EXECUTABLE)
ENDIF(LRELEASE_EXECUTABLE)

MARK_AS_ADVANCED( LUPDATE_EXECUTABLE LUPDATE_FOUND 
                  LRELEASE_EXECUTABLE LRELEASE_FOUND )

MACRO(ADD_TRANSLATIONS)
    SET(_outputs)
    FOREACH (_it ${ARGN})
        GET_FILENAME_COMPONENT(_outfile ${_it} NAME_WE)
        GET_FILENAME_COMPONENT(_infile ${_it} ABSOLUTE)
        SET(_outfile ${CMAKE_CURRENT_BINARY_DIR}/${_outfile}.qm)
        ADD_CUSTOM_COMMAND( OUTPUT ${_outfile}
            COMMAND ${LRELEASE_EXECUTABLE}
            ARGS    ${_infile} 
                    -qm ${_outfile}
            MAIN_DEPENDENCY ${_infile}
        )
        SET(_outputs ${_outputs} ${_outfile})
    ENDFOREACH (_it)
    INSTALL(FILES ${_outputs}
            DESTINATION share/locale)
    ADD_CUSTOM_TARGET(translations ALL DEPENDS ${_outputs})
ENDMACRO(ADD_TRANSLATIONS)

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
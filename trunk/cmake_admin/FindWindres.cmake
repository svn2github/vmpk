IF(WINDRES_EXECUTABLE)
    SET(WINDRES_FOUND TRUE)
ELSE(WINDRES_EXECUTABLE)
    FIND_PROGRAM(WINDRES_EXECUTABLE
    NAMES windres mingw32-windres i686-mingw32-windres) 
    IF(WINDRES_EXECUTABLE)
        SET(WINDRES_FOUND TRUE)
    ELSE(WINDRES_EXECUTABLE)
    IF(NOT WINDRES_FIND_QUIETLY)
        IF(WINDRES_FIND_REQUIRED)
            MESSAGE(FATAL_ERROR "windres program couldn't be found")
        ENDIF(WINDRES_FIND_REQUIRED)
    ENDIF(NOT WINDRES_FIND_QUIETLY)
    ENDIF(WINDRES_EXECUTABLE)
ENDIF (WINDRES_EXECUTABLE)

# ADD_WINDRES_OBJS(outfiles inputfile ... )

MACRO(ADD_WINDRES_OBJS outfiles)
    FOREACH (it ${ARGN})
        GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)
        GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
        SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfile}.obj)
        ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
            COMMAND ${WINDRES_EXECUTABLE} 
            ARGS -I${CMAKE_CURRENT_SOURCE_DIR}
                 -i ${infile}
                 -o ${outfile}
            MAIN_DEPENDENCY ${infile})
        SET(${outfiles} ${${outfiles}} ${outfile})
    ENDFOREACH (it)
ENDMACRO(ADD_WINDRES_OBJS)

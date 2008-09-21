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

PROJECT(VMPK)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.4)

# Show full commands and messages. 
# SET(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL)

# Seems that relative paths break the link scripts. Don't set it ON
SET(CMAKE_USE_RELATIVE_PATHS OFF)

# Assorted settings
SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_INCLUDE_CURRENT_DIR TRUE)
SET(CMAKE_SKIP_RULE_DEPENDENCY FALSE)

# Additional CMake modules for 3rd party library checks reside here
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_admin")

# VMPK version string
SET (VERSION_MAJOR "0")
SET (VERSION_MINOR "1")
SET (VERSION_PATCH "2")
#SET (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
SET (VERSION_SUFFIX "CVS")
SET (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX})
ADD_DEFINITIONS(-D'VERSION="${VERSION}"')

MESSAGE(STATUS "vmpk ${VERSION} prefix: ${CMAKE_INSTALL_PREFIX}")

# User options
MESSAGE(STATUS "Build configuration: ${CMAKE_BUILD_TYPE}")

# Check for Qt
FIND_PACKAGE(Qt4 REQUIRED QUIET)
IF(QT4_FOUND)
    ADD_DEFINITIONS(${QT_DEFINITIONS})
    ADD_DEFINITIONS("-DQT_GUI_LIB -DQT_CORE_LIB -DQT_XML_LIB")
    IF (NOT QT_LIBRARIES)
        SET(QT_LIBRARIES ${QT_QTCORE_LIBRARY}
                         ${QT_QTGUI_LIBRARY}
                         ${QT_QTXML_LIBRARY}
                         ${QT_CORE_LIB_DEPENDENCIES}
                         ${QT_GUI_LIB_DEPENDENCIES}
                         ${QT_XML_LIB_DEPENDENCIES})
    ENDIF (NOT QT_LIBRARIES)
    MESSAGE(STATUS "Found Qt4.${QT_EDITION} version = ${QTVERSION}")
    #MESSAGE("QT_DEFINITIONS: ${QT_DEFINITIONS}")
    #MESSAGE("QT_LIBRARIES: ${QT_LIBRARIES}")
    #MESSAGE("QT_QTCORE_LIBRARY: ${QT_QTCORE_LIBRARY}")
    #MESSAGE("QT_QTGUI_LIBRARY: ${QT_QTGUI_LIBRARY}")
    #MESSAGE("QT_INCLUDES: ${QT_INCLUDES}")
    #MESSAGE("QT_WRAP_CPP: ${QT_WRAP_CPP}")
    #MESSAGE("QT_MOC_EXECUTABLE: ${QT_MOC_EXECUTABLE}")
    #MESSAGE("QT_UIC_EXECUTABLE: ${QT_UIC_EXECUTABLE}")
    #MESSAGE("QT_LIBRARY_DIR: ${QT_LIBRARY_DIR}")
    #MESSAGE("QT_INCLUDE_DIR: ${QT_INCLUDE_DIR}")
ELSE(QT4_FOUND)
    MESSAGE(FATAL_ERROR "Qt4 not found")
ENDIF(QT4_FOUND)

INCLUDE_DIRECTORIES(${QT_INCLUDES}) 
LINK_DIRECTORIES(${QT_LIBRARY_DIR})
LINK_LIBRARIES(${QT_LIBRARIES})

IF(${CMAKE_SYSTEM} MATCHES "Linux")
# Check for PKG-CONFIG
    FIND_PACKAGE(PkgConfig REQUIRED)
    IF(PKG_CONFIG_FOUND)
        MESSAGE(STATUS "Program pkg-config found (${PKG_CONFIG_EXECUTABLE})")
    ELSE(PKG_CONFIG_FOUND)
        MESSAGE(FATAL_ERROR "Program pkg-config not found")
    ENDIF(PKG_CONFIG_FOUND)
# Initialize some variables
    SET(HAVE_ALSA FALSE)
# Check for ALSA 1.0
    PKG_CHECK_MODULES(ALSA REQUIRED alsa>=1.0)
    IF(ALSA_FOUND)
        SET(HAVE_ALSA TRUE)
        ADD_DEFINITIONS(-DHAVE_ALSA)
        SET(ALSA_LIBS ${ALSA_LIBRARIES})
        #MESSAGE("ALSA_LIBS: ${ALSA_LIBS}")
        LIST(APPEND ALSA_LIB_DIR ${ALSA_LIBRARY_DIRS} ${ALSA_LIBDIR})
        #MESSAGE("ALSA_LIB_DIR: ${ALSA_LIB_DIR}")
        LIST(APPEND ALSA_INC_DIR ${ALSA_INCLUDE_DIRS} ${ALSA_INCLUDEDIR})
        #MESSAGE("ALSA_INC_DIR: ${ALSA_INC_DIR}")
    ELSE(ALSA_FOUND)
        MESSAGE(FATAL_ERROR "Please install the required package. Aborting")
    ENDIF(ALSA_FOUND)
    LINK_DIRECTORIES(${ALSA_LIB_DIR})
    INCLUDE_DIRECTORIES(${ALSA_INC_DIR})
    ADD_DEFINITIONS(-D__LINUX_ALSASEQ__)
    LINK_LIBRARIES(${ALSA_LIBS})
ENDIF(${CMAKE_SYSTEM} MATCHES "Linux")

IF(${CMAKE_SYSTEM} MATCHES "Darwin")
    ADD_DEFINITIONS(-D__MACOSX_CORE__)
    LINK_LIBRARIES("-framework CoreMidi -framework CoreAudio -framework CoreFoundation")
ENDIF(${CMAKE_SYSTEM} MATCHES "Darwin")

IF(${CMAKE_SYSTEM} MATCHES "IRIX")
    ADD_DEFINITIONS(-D__IRIX_MD__)
    LINK_LIBRARIES(audio)
    LINK_LIBRARIES(pthread)
ENDIF(${CMAKE_SYSTEM} MATCHES "IRIX")

IF(${CMAKE_SYSTEM} MATCHES "Windows")
    FIND_PACKAGE(WINDRES REQUIRED)
    ADD_DEFINITIONS(-D__WINDOWS_MM__)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
    LINK_LIBRARIES(winmm)
ENDIF(${CMAKE_SYSTEM} MATCHES "Windows")

FIND_PACKAGE(TranslationSupport)

ADD_SUBDIRECTORY(src build)
ADD_SUBDIRECTORY(translations build/translations)

CONFIGURE_FILE(vmpk.spec.in vmpk.spec IMMEDIATE @ONLY)

# uninstall custom target
CONFIGURE_FILE(
    "${CMAKE_SOURCE_DIR}/cmake_admin/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET ( uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# Custom targets
IF (UNIX)
    ADD_CUSTOM_TARGET ( distclean
        COMMAND rm -f  CMakeCache.txt
        COMMAND rm -f  cmake_install.cmake
        COMMAND rm -f  cmake_uninstall.cmake
        COMMAND rm -f  CPackConfig.cmake
        COMMAND rm -f  CPackSourceConfig.cmake
        COMMAND rm -f  install_manifest.txt
        COMMAND rm -f  vmpk.spec
        COMMAND rm -f  progress.make
        COMMAND rm -f  Makefile
        COMMAND rm -rf CMakeFiles
        COMMAND rm -rf _CPack_Packages
        COMMAND rm -rf build/*
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    ADD_DEPENDENCIES(distclean clean)
    
    ADD_CUSTOM_TARGET ( tarball
        COMMAND mkdir -p vmpk-${VERSION}
        COMMAND cp -r cmake_admin vmpk-${VERSION}
        COMMAND cp -r data vmpk-${VERSION}
        COMMAND cp -r src vmpk-${VERSION}
        COMMAND cp -r translations vmpk-${VERSION}
        COMMAND cp AUTHORS ChangeLog CMakeLists.txt COPYING gpl.rtf NEWS README setup.nsi TODO vmpk.* vmpk-${VERSION}
        COMMAND tar -cj --exclude CVS --exclude .cvsignore -f vmpk-${VERSION}.tar.bz2 vmpk-${VERSION}
        COMMAND tar -cz --exclude CVS --exclude .cvsignore -f vmpk-${VERSION}.tar.gz vmpk-${VERSION}
        COMMAND rm -rf vmpk-${VERSION}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
ENDIF (UNIX)

INSTALL ( FILES
    AUTHORS
    ChangeLog
    COPYING
    NEWS
    README
    TODO
DESTINATION 
    share/doc/packages/vmpk
)

INSTALL ( FILES
    data/azerty.xml
    data/vkeybd-default.xml
    data/it-qwerty.xml
    data/german.xml
    data/gmgsxg.ins
    data/spanish.xml
DESTINATION 
    share/vmpk 
)

INSTALL ( FILES 
    data/vmpk_16x16.png 
    data/vmpk_32x32.png 
    data/vmpk_48x48.png 
    data/vmpk_64x64.png 
DESTINATION 
    share/pixmaps 
)

INSTALL ( FILES 
    vmpk.desktop
DESTINATION 
    share/applications
)

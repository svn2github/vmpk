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

PROJECT(VMPK)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

OPTION(ENABLE_DBUS "Enable experimental VMPK DBus interface" ON)

# Show full commands and messages. 
# SET(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL)

# Seems that relative paths break the link scripts. Don't set it ON
SET(CMAKE_USE_RELATIVE_PATHS OFF)

# Assorted settings
SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_INCLUDE_CURRENT_DIR TRUE)
SET(CMAKE_SKIP_RULE_DEPENDENCY FALSE)
SET(CMAKE_CXX_FLAGS_DEBUG "-g -DQT_DEBUG -DDEBUG -D__RTMIDI_DEBUG__")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2 -DQT_NO_DEBUG -DNDEBUG")

# Additional CMake modules for 3rd party library checks reside here
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_admin")

# VMPK version string
SET (VERSION_MAJOR "0")
SET (VERSION_MINOR "3")
SET (VERSION_PATCH "3")
SET (VERSION_SUFFIX "")
SET (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX})

MESSAGE(STATUS "vmpk ${VERSION} prefix: ${CMAKE_INSTALL_PREFIX}")

# User options
MESSAGE(STATUS "Build configuration: ${CMAKE_BUILD_TYPE}")

# Check for Qt
SET(QT_MIN_VERSION "4.4.0")
FIND_PACKAGE(Qt4 REQUIRED QUIET)
IF(QT4_FOUND)
    MESSAGE(STATUS "Found Qt4.${QT_EDITION} version = ${QTVERSION}")
ELSE(QT4_FOUND)
    MESSAGE(FATAL_ERROR "Qt4 >= 4.4 not found")
ENDIF(QT4_FOUND)

SET(QT_USE_QTXML TRUE)
SET(QT_USE_QTSVG TRUE)
IF(ENABLE_DBUS)
    SET(QT_USE_QTDBUS TRUE)
    ADD_DEFINITIONS(-DENABLE_DBUS)
ENDIF(ENABLE_DBUS)
INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(${QT_INCLUDES}) 
LINK_DIRECTORIES(${QT_LIBRARY_DIR})
LINK_LIBRARIES(${QT_LIBRARIES})
ADD_DEFINITIONS(-DQT_STRICT_ITERATORS)

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
    ADD_DEFINITIONS(-DAVOID_TIMESTAMPING)
    LINK_LIBRARIES(${ALSA_LIBS} ${QT_X11_X11_LIBRARY})
ENDIF(${CMAKE_SYSTEM} MATCHES "Linux")

IF(${CMAKE_SYSTEM} MATCHES "Darwin")
    ADD_DEFINITIONS(-D__MACOSX_CORE__)
    LINK_LIBRARIES("-framework CoreMidi -framework CoreAudio -framework CoreFoundation -framework Carbon")
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

ADD_SUBDIRECTORY (src)
ADD_SUBDIRECTORY (translations)
CONFIGURE_FILE (vmpk.spec.in vmpk.spec IMMEDIATE @ONLY)

# uninstall custom target
CONFIGURE_FILE(
    "${CMAKE_SOURCE_DIR}/cmake_admin/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET ( uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

IF (UNIX)
    ADD_CUSTOM_TARGET ( distclean
        COMMAND rm -rf build/*
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    ADD_DEPENDENCIES (distclean clean)
    
    ADD_CUSTOM_TARGET ( tarball
        COMMAND mkdir -p vmpk-${VERSION}/translations/
        COMMAND cp -r cmake_admin vmpk-${VERSION}
        COMMAND cp -r data vmpk-${VERSION}
        COMMAND cp -r src vmpk-${VERSION}
        COMMAND cp -r man vmpk-${VERSION}
        COMMAND cp -r dbus vmpk-${VERSION}
        COMMAND cp translations/vmpk_{cs,de,es,fr,ru,zh_CN}.ts vmpk-${VERSION}/translations/
        COMMAND cp translations/CMakeLists.txt vmpk-${VERSION}/translations/
        COMMAND cp AUTHORS ChangeLog CMakeLists.txt COPYING gpl.rtf NEWS README setup.nsi TODO vmpk.desktop vmpk.pro updateqm.pri vmpk.spec.in qt.conf vmpk-${VERSION}
        COMMAND tar -cj --exclude=.* -f vmpk-${VERSION}.tar.bz2 vmpk-${VERSION}
        COMMAND tar -cz --exclude=.* -f vmpk-${VERSION}.tar.gz vmpk-${VERSION}
        COMMAND rm -rf vmpk-${VERSION}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
ENDIF (UNIX)

IF (UNIX AND NOT APPLE)

    ADD_SUBDIRECTORY (man)

# unmaintained translations:
# data/help_tr.html
    INSTALL ( FILES
        data/azerty.xml
        data/vkeybd-default.xml
        data/it-qwerty.xml
        data/german.xml
        data/gmgsxg.ins
        data/spanish.xml
        data/pc102x11.xml
        data/txt2ins.awk
        data/help.html
        data/help_de.html
        data/help_es.html
        data/help_ru.html
    DESTINATION 
        share/vmpk 
    )

    INSTALL(FILES data/vmpk_16x16.png 
        DESTINATION share/icons/hicolor/16x16/apps   
        RENAME vmpk.png)
    
    INSTALL(FILES data/vmpk_32x32.png 
        DESTINATION share/icons/hicolor/32x32/apps   
        RENAME vmpk.png)
    
    INSTALL(FILES data/vmpk_48x48.png 
        DESTINATION share/icons/hicolor/48x48/apps   
        RENAME vmpk.png)
    
    INSTALL(FILES data/vmpk_64x64.png 
        DESTINATION share/icons/hicolor/64x64/apps   
        RENAME vmpk.png)

    INSTALL(FILES data/vmpk_128x128.png 
        DESTINATION share/icons/hicolor/128x128/apps   
        RENAME vmpk.png)

    INSTALL(FILES data/vmpk.svgz
        DESTINATION share/icons/hicolor/scalable/apps )   
    
    INSTALL ( FILES 
        vmpk.desktop
    DESTINATION 
        share/applications
    )

ENDIF (UNIX AND NOT APPLE)

IF (WIN32)
    INCLUDE (InstallRequiredSystemLibraries)
    SET ( QTLIBLIST QtCore QtGui QtXml QtSvg )
    GET_FILENAME_COMPONENT(QT_DLL_PATH_tmp ${QT_QMAKE_EXECUTABLE} PATH)
    FOREACH(qtlib ${QTLIBLIST})
        INSTALL ( FILES ${QT_DLL_PATH_tmp}/${qtlib}4.dll
                  DESTINATION . )
    ENDFOREACH(qtlib)
    INSTALL ( FILES ${QT_DLL_PATH_tmp}/mingwm10.dll 
                    ${QT_DLL_PATH_tmp}/libgcc_s_dw2-1.dll
              DESTINATION . )

# unmaintained translations:
# data/help_tr.html
    INSTALL ( FILES
        data/azerty.xml
        data/vkeybd-default.xml
        data/it-qwerty.xml
        data/german.xml
        data/gmgsxg.ins
        data/spanish.xml
        data/pc102win.xml
        data/help.html
        data/help_de.html
        data/help_es.html
        data/help_ru.html
    DESTINATION . )
ENDIF (WIN32)

IF (APPLE)
    SET(target "vmpk")
    CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/cmake_admin/CompleteBundle.cmake.in"
      "${CMAKE_CURRENT_BINARY_DIR}/build/CompleteBundle.cmake" @ONLY IMMEDIATE)
    CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/cmake_admin/CreateBundle.sh.in"
      "${CMAKE_CURRENT_BINARY_DIR}/build/CreateBundle.sh" @ONLY IMMEDIATE)
    INSTALL(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/build/CompleteBundle.cmake")
ENDIF (APPLE)

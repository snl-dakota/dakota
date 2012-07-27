# - Try to find a version of Scilab and headers/library required by the 
#   used compiler.
#
# An optional intput definition
#  SCILAB_USER_PATH: if Scilab is installed in a custom directory, SCILAB_USER_PATH points to it.
# This module defines: 
#  SCILAB_LIBRARY: Scilab library
#  SCILAB_BINARY: Scilab binary 
#  SCILAB_ROOT: Scilab installation path
#  SCILAB_BIN_PATH: Scilab binary path
#  SCILAB_LIBRARIES_PATH: Scilab library path
#  SCILAB_INCLUDES_PATH: Scilab include path

# Copyright (c) 2011 Yann Collette <ycollette.nospam@free.fr>
# Copyright (c) 2009 Arnaud Barré <arnaud.barre@gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF (SCILAB_ROOT AND SCILAB_LIBRARIES)
   # in cache already
   SET (Scilab_FIND_QUIETLY TRUE)
ENDIF (SCILAB_ROOT AND SCILAB_LIBRARIES)

IF (WIN32)
  IF (NOT SCILAB_USER_PATH)
    GET_FILENAME_COMPONENT(SCILAB_VER "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Scilab;LASTINSTALL]" NAME)
    GET_FILENAME_COMPONENT(SCILAB_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Scilab\\${SCILAB_VER};SCIPATH]" ABSOLUTE)
    IF (SCILAB_VER STREQUAL "registry")
      GET_FILENAME_COMPONENT(SCILAB_VER "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scilab;LASTINSTALL]" NAME)
      GET_FILENAME_COMPONENT(SCILAB_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Scilab\\${SCILAB_VER};SCIPATH]" ABSOLUTE)
    ENDIF (SCILAB_VER EQUAL "registry")
  ELSE (NOT SCILAB_USER_PATH)
    SET(SCILAB_ROOT ${SCILAB_USER_PATH})
  ENDIF (NOT SCILAB_USER_PATH)

  FIND_PATH(SCILAB_ROOT 
            NAMES "etc/scilab.start" 
            PATHS ${SCILAB_PATH})

  IF (NOT SCILAB_ROOT)
    MESSAGE(SEND_ERROR "Scilab path not found")
  ENDIF (NOT SCILAB_ROOT)

  SET (SCILAB_BIN_PATH ${SCILAB_ROOT}/bin)
  SET (SCILAB_LIBRARIES_PATH ${SCILAB_ROOT}/bin)
  SET (SCILAB_INCLUDES_PATH ${SCILAB_ROOT}/modules)

  SET(LIBSCILAB "LibScilab")
  SET(SCILAB_BIN "Scilex")
ELSEIF(APPLE)
  IF (NOT SCILAB_USER_PATH)
    FILE(GLOB SCILAB_PATHS "/Applications/Scilab*")
  ELSE (NOT SCILAB_USER_PATH)
    SET(SCILAB_ROOT ${SCILAB_USER_PATH})
  ENDIF (NOT SCILAB_USER_PATH)

  FIND_PATH(SCILAB_ROOT 
            NAMES "Contents/MacOS/share/scilab/etc/scilab.start" 
            PATHS ${SCILAB_PATHS})

  IF (NOT SCILAB_ROOT)
    MESSAGE(SEND_ERROR "Scilab path not found")
  ENDIF (NOT SCILAB_ROOT)

  SET (SCILAB_BIN_PATH ${SCILAB_ROOT}/Contents/MacOS/bin)
  SET (SCILAB_LIBRARIES_PATH ${SCILAB_ROOT}/Contents/MacOS/lib/scilab)
  SET (SCILAB_INCLUDES_PATH ${SCILAB_ROOT}/Contents/MacOS/share/scilab/modules)

  SET(LIBSCILAB "scilab")
  SET(SCILAB_BIN "scilab")
ELSE(WIN32)

  IF (NOT SCILAB_USER_PATH)
    FILE(GLOB TEST_OPT_SCILAB /opt/scilab*)
    LIST(SORT TEST_OPT_SCILAB)
    LIST(GET TEST_OPT_SCILAB -1 TEST_OPT_SCILAB)

    SET(SCILAB_PATH /usr
                    /usr/local
                    ${TEST_OPT_SCILAB}
                    /opt/local)
  ELSE (NOT SCILAB_USER_PATH)
    SET(SCILAB_ROOT ${SCILAB_USER_PATH})
  ENDIF (NOT SCILAB_USER_PATH)

  FIND_PATH(SCILAB_ROOT 
            NAMES "share/scilab/etc/scilab.start" 
            PATHS ${SCILAB_PATH})

  IF (NOT SCILAB_ROOT)
    MESSAGE(SEND_ERROR "Scilab path not found")
  ENDIF (NOT SCILAB_ROOT)

  SET (SCILAB_BIN_PATH ${SCILAB_ROOT}/bin)
  SET (SCILAB_LIBRARIES_PATH ${SCILAB_ROOT}/lib/scilab)
  SET (SCILAB_INCLUDES_PATH ${SCILAB_ROOT}/include/scilab)

  SET(LIBSCILAB scilab)
  SET(SCILAB_BIN scilab)
ENDIF(WIN32)

FIND_PROGRAM(SCILAB_BINARY
             NAMES ${SCILAB_BIN}
             PATHS ${SCILAB_BIN_PATH} NO_DEFAULT_PATH)
  
FIND_LIBRARY(SCILAB_LIBRARY
             NAMES ${LIBSCILAB}
             PATHS ${SCILAB_LIBRARIES_PATH} NO_DEFAULT_PATH)

IF(SCILAB_ROOT)
  INCLUDE(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(Scilab DEFAULT_MSG SCILAB_ROOT 
                                                       SCILAB_BINARY 
						       SCILAB_LIBRARY
						       SCILAB_BIN_PATH
						       SCILAB_LIBRARIES_PATH
						       SCILAB_INCLUDES_PATH)
ENDIF(SCILAB_ROOT)

MARK_AS_ADVANCED(
  SCILAB_LIBRARY
  SCILAB_BINARY
  SCILAB_ROOT
  SCILAB_BIN_PATH
  SCILAB_LIBRARIES_PATH
  SCILAB_INCLUDES_PATH
)

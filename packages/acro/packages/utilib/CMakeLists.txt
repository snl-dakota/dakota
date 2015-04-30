# UTILIB: CMake Script

# Warning - does not include the CXX-TEST tools at this time
# Revision 0.1 - dbhart - 20100701


cmake_minimum_required(VERSION 2.6)
project(UTILIB)
set(ExportTarget ${CMAKE_PROJECT_NAME}-targets CACHE STRING 
  "Name for the export target for ${CMAKE_PROJECT_NAME}")

#developer controlled options that change the way utilib is built
option(UTILIB_VALIDATING "Turn on code validation tests?" OFF)
if(UTILIB_VALIDATING)
  add_definitions(-DUTILIB_VALIDATING=1)
endif()
option(UTILIB_DISABLE_COMMONIO "Should CommonIO be disabled in utilib?" OFF)
if(UTILIB_DISABLE_COMMONIO)
  add_definitions(-DUTILIB_DISABLE_COMMONIO=1)
endif()
option(UTILIB_YES_DEBUGPR "Should DEBUGPR be included in utilib?" ON)
if(UTILIB_YES_DEBUGPR)
  add_definitions(-DUTILIB_YES_DEBUGPR=1)
endif()
option(UTILIB_YES_LINPACK "Should LINPACK be included in utilib?" OFF)
if(UTILIB_YES_LINPACK)
  add_definitions(-DUTILIB_YES_LINPACK=1)
endif()
option(UTILIB_YES_MEMDEBUG "Should MEMDEBUG be included in utilib?" OFF)
if(UTILIB_YES_MEMDEBUG)
  add_definitions(-DUTILIB_YES_MEMDEBUG=1)
endif()
option(UTILIB_HAVE_EXPAT "Enable expat in utilib" OFF)
if (UTILIB_HAVE_EXPAT)
  find_package(EXPAT)
  if (EXPAT_FOUND)
    # Use the same definition as autotools build
    add_definitions("-DUTILIB_HAVE_EXPAT_H")
    # TODO: add include path (EXPAT_INCLUDE_DIRS) in relevant subdirs?
  else()
    message(WARNING "expat requested, but not found")
  endif()
endif()

#inspect the system so that we can generate utilib_config.h 
include(CheckFunctionExists)
CHECK_FUNCTION_EXISTS(clock UTILIB_HAVE_CLOCK)
CHECK_FUNCTION_EXISTS(ftime UTILIB_HAVE_FTIME)
CHECK_FUNCTION_EXISTS(getrusage UTILIB_HAVE_GETRUSAGE)
CHECK_FUNCTION_EXISTS(gettimeofday UTILIB_HAVE_GETTIMEOFDAY)
CHECK_FUNCTION_EXISTS(localtime UTILIB_HAVE_LOCALTIME)
CHECK_FUNCTION_EXISTS(nrand48 UTILIB_HAVE_NRAND48)
CHECK_FUNCTION_EXISTS(sysconf UTILIB_HAVE_SYSCONF)
CHECK_FUNCTION_EXISTS(times UTILIB_HAVE_TIMES)

include(CheckIncludeFiles)
CHECK_INCLUDE_FILES(cxxabi.h UTILIB_HAVE_CXXABI_H)
CHECK_INCLUDE_FILES("windows.h;dbghelp.h" UTILIB_HAVE_DBGHELP_H)
CHECK_INCLUDE_FILES(err.h UTILIB_HAVE_ERR_H)
CHECK_INCLUDE_FILES(execinfo.h UTILIB_HAVE_EXECINFO_H)
CHECK_INCLUDE_FILES(float.h UTILIB_HAVE_FLOAT_H)
CHECK_INCLUDE_FILES(limits.h UTILIB_HAVE_LIMITS_H)
CHECK_INCLUDE_FILES(strings.h UTILIB_HAVE_STRINGS_H)
CHECK_INCLUDE_FILES(sys/resource.h UTILIB_HAVE_SYS_RESOURCE_H)
CHECK_INCLUDE_FILES(sys/time.h UTILIB_HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILES(unistd.h UTILIB_HAVE_UNISTD_H)
CHECK_INCLUDE_FILES(values.h UTILIB_HAVE_VALUES_H)
CHECK_INCLUDE_FILES(windows.h UTILIB_HAVE_WINDOWS_H)


# Use common HAVE_BOOST to indicate usage of Boost across packages
# Ultimately need to #define UTILIB_HAVE_BOOST if used 
set(HAVE_BOOST OFF CACHE BOOL "Should Utilib use BOOST?")
set(UTILIB_HAVE_BOOST 0)
if(HAVE_BOOST)

  if(Boost_DIR)
    message( "in ${CMAKE_CURRENT_BINARY_DIR} Boost_DIR already set to ${Boost_DIR}" )
    set(UTILIB_HAVE_BOOST 1) 
    include_directories(${Boost_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
  else()
    find_package(Boost COMPONENTS signals)
    if(Boost_FOUND)
      set(UTILIB_HAVE_BOOST 1)
      include_directories(${Boost_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
    endif()
  endif(Boost_DIR)

endif(HAVE_BOOST)

option(UTILIB_USE_MPI "Should UTILIB be built using MPI?" OFF)
  if(UTILIB_USE_MPI)
  find_package(MPI)
  if(MPI_FOUND)
    set(UTILIB_HAVE_MPI 1)
    include_directories(${MPI_INCLUDE_PATH})
  endif()
endif()

#copy settings from acro if utilib is being built as a subproject
if(ACRO_HAVE_MPI)
  set(UTILIB_USE_MPI ON CACHE BOOL "Should UTILIB be built using MPI?" FORCE)
  include_directories(${MPI_INCLUDE_PATH})
endif()


# Use common HAVE_TINYXML to indicate usage of TinyXML across packages
# Ultimately need to #define UTILIB_HAVE_TINYXML if used 
set(HAVE_TINYXML OFF CACHE BOOL "Should Utilib use TinyXML?")
set(UTILIB_HAVE_TINYXML 0)
if(HAVE_TINYXML)
  find_path(TINYXML_DIR tinyxml.h DOC "Path to directory containing tinyxml.h")
  if(NOT TINYXML_DIR)
    message(SEND_ERROR "HAVE_TINYXML is ON, but TINYXML_DIR is not found.  Please set TINYXML_DIR to the location of tinyxml/tinyxml.h")
  endif()
  include_directories(${TINYXML_DIR})
  set(UTILIB_HAVE_TINYXML 1)
endif()

find_library(M_LIBRARY m)
if(M_LIBRARY)
  set(previous_required_libs ${CMAKE_REQUIRED_LIBRARIES})
  set(CMAKE_REQUIRED_LIBRARIES ${M_LIBRARY})
  CHECK_FUNCTION_EXISTS(lround UTILIB_HAVE_LROUND)
  set(CMAKE_REQUIRED_LIBRARIES ${previous_required_libs})
  if(UTILIB_HAVE_LROUND)
    # Subsequent commented-out add_definitions() calls should be
    # uncommented once we blank utilib_config.h.in
    #add_definitions(-DUTILIB_HAVE_LROUND)
  endif()
endif()

set(includes_to_check
  "sys/types.h"
  "sys/time.h"
  "time.h"
)
CHECK_INCLUDE_FILES("${includes_to_check}" UTILIB_TIME_WITH_SYS_TIME)
if(UTILIB_HAVE_TIME_WITH_SYS_TIME)
  #add_definitions(-DUTILIB_HAVE_TIME_WITH_SYS_TIME)
endif()

# Assume that any modern compiler supports these features
set(UTILIB_HAVE_EXCEPTIONS 1)
set(UTILIB_HAVE_MEMBER_TEMPLATES 1)
set(UTILIB_HAVE_NAMESPACES 1)
set(UTILIB_HAVE_SSTREAM 1)
set(UTILIB_HAVE_STD 1)
set(UTILIB_STDC_HEADERS 1)
#add_definitions(-DUTILIB_HAVE_EXCEPTIONS)
#add_definitions(-DUTILIB_HAVE_MEMBER_TEMPLATES)
#add_definitions(-DUTILIB_HAVE_NAMESPACES)
#add_definitions(-DUTILIB_HAVE_SSTREAM)
#add_definitions(-DUTILIB_HAVE_STD)
#add_definitions(-DUTILIB_STDC_HEADERS)

if(${CMAKE_SYSTEM_NAME} MATCHES "AIX")
  set(UTILIB_AIX_CC 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "OSF")
  set(UTILIB_OSF_CC 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "SGI")
  set(UTILIB_SGI_CC 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
  set(UTILIB_SOLARIS_CC 1)
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/utilib_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/utilib_config.h
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/utilib_config.h DESTINATION include)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)
#add_subdirectory(doc)
add_subdirectory(test/unit)


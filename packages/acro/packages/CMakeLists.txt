cmake_minimum_required(VERSION 2.6)

project(Acro-Packages)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
 
#flag so subprojects can tell that they're being built by acro
set(BUILT_BY_ACRO TRUE)

# generate acro_config.h
function(FindOptionalLibrary library)
  option(HAVE_${library} "Should Acro use ${library}?" OFF)
  if(${library}_LIB OR ${library}_LIBS)
    set(HAVE_${library} ON CACHE BOOL "Should Acro use ${library}?" FORCE)
  endif()
  if(HAVE_${library})
    string(TOLOWER ${library} library_lower)
    find_library(${library}_LIB ${library_lower})
    if(${library}_LIB)
      set(HAVE_${library} TRUE)
      add_definitions(-DHAVE_${library})
    endif()
  endif()
endfunction()

FindOptionalLibrary(CPLEX)

#FindOptionalLibrary(BLAS)
include(FindBLAS)
set( HAVE_BLAS ${BLAS_FOUND} 
     CACHE BOOL "Enable interfaces to system BLAS" )

include(CheckIncludeFiles)
# appears no acro sources depend on this define any longer
#CHECK_INCLUDE_FILES("sys/types.h" HAVE_SYS_TYPES_H)
#if(HAVE_SYS_TYPES_H)
#  add_definitions(-DHAVE_SYS_TYPES_H)
#endif()
# some pico sources (and possibly utilib) MIGHT be using this
CHECK_INCLUDE_FILES("values.h" HAVE_VALUES_H)
if(HAVE_VALUES_H)
  add_definitions(-DHAVE_VALUES_H)
endif()


# replicate TPL defs for now
if(HAVE_AMPL)
  add_definitions(-DACRO_USING_AMPL)
endif()

if(HAVE_3PO)
  add_definitions(-DACRO_USING_THREEPO)
endif()

if(HAVE_TINYXML)
  add_definitions(-DACRO_USING_TINYXML)
endif()

if(HAVE_APPSPACK)
  add_definitions(-DACRO_USING_APPSPACK)
endif()

if(HAVE_GLPK)
  add_definitions(-DACRO_USING_GLPK)
endif()

if(HAVE_SOPLEX)
  add_definitions(-DACRO_USING_SOPLEX)
endif()

set(packages 
  "${CMAKE_CURRENT_SOURCE_DIR}/colin"
  "${CMAKE_CURRENT_SOURCE_DIR}/interfaces"
  "${CMAKE_CURRENT_SOURCE_DIR}/jega"
  "${CMAKE_CURRENT_SOURCE_DIR}/optpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/pebbl"
  "${CMAKE_CURRENT_SOURCE_DIR}/pico"
  "${CMAKE_CURRENT_SOURCE_DIR}/scolib"
  "${CMAKE_CURRENT_SOURCE_DIR}/sundry"
  "${CMAKE_CURRENT_SOURCE_DIR}/utilib")

foreach(package ${packages})
  AcroCheckDirExists("${package}")
endforeach()

option(HAVE_BOOST "Should Acro use BOOST?" ON)
option(ACRO_HAVE_MPI "Should Acro use MPI?" OFF)
option(VALIDATING "Turn on code validation tests?" OFF)

configure_file(
  ${Acro_SOURCE_DIR}/cmake/acro_config.h.cmake.in
  ${Acro-Packages_BINARY_DIR}/acro_config.h
)
include_directories(${Acro-Packages_BINARY_DIR})
install(FILES ${Acro-Packages_BINARY_DIR}/acro_config.h DESTINATION include)

if(HAVE_BOOST)
  add_definitions(-DHAVE_BOOST)

  if(Boost_DIR)
    #message( "in ${CMAKE_CURRENT_BINARY_DIR} Boost_DIR already set to ${Boost_DIR}" )
    include_directories(${Boost_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
  else()
    find_package(Boost COMPONENTS signals)
    if(Boost_FOUND)
      include_directories(${Boost_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
    endif()
  endif(Boost_DIR)
endif(HAVE_BOOST)

option(HAVE_MOMHLIB "Build with MOMHLib enabled?" OFF)
if(HAVE_MOMHLIB)
  find_path(MOMHLIB_DIR libmomh-config.h DOC "Path to directory containing libmomh-config.h")
  find_path(SIGCPP_DIR sigc++/sigc++.h DOC "Path to directory containing sigc++.h")
  find_path(SIGCPPCONFIG_DIR sigc++config.h DOC "Path to directory containing sigc++config.h")
  if(NOT MOMHLIB_DIR)
    message(SEND_ERROR "HAVE_MOMHLIB is ON, but MOMHLIB_DIR is not found.  Please set MOMHLIB_DIR to the location of libmomh-config.h")
  endif()
  if(NOT SIGCPP_DIR)
    message(SEND_ERROR "HAVE_MOMHLIB is ON, but SIGCPP_DIR is not found.  Please set SIGCPP_DIR to the location of sigc++/sigc++.h")
  endif()
  if(NOT SIGCPPCONFIG_DIR)
    message(SEND_ERROR "HAVE_MOMHLIB is ON, but SIGCPPCONFIG_DIR is not found.  Please set SIGCPPCONFIG_DIR to the location of sigc++config.h")
  endif()
  set(MOMHLIB_INCLUDE_DIRS ${MOMHLIB_DIR} ${SIGCPP_DIR} ${SIGCPPCONFIG_DIR})
  add_definitions(-DACRO_USING_MOMHLIB)
endif()

if(ACRO_HAVE_MPI)
  find_package(MPI)
  add_definitions(-DHAVE_MPI)
endif()


if(HAVE_TINYXML)
  find_path(TINYXML_DIR tinyxml.h DOC "Path to directory containing tinyxml.h")
  if(NOT TINYXML_DIR)
    message(SEND_ERROR "HAVE_TINYXML is ON, but TINYXML_DIR is not found.  Please set TINYXML_DIR to the location of tinyxml/tinyxml.h")
  endif()
endif()

# In Acro, presence of a local tecuhos/optpp means Acro needs it,
# but it might be provided externally by the user, e.g., in DAKOTA.
# Take care in case HAVE_DAKOTA set by a parent instead of locally.
if(HAVE_OPTPP AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/optpp)
  add_definitions(-DACRO_USING_OPTPP)
  add_subdirectory(optpp)
  # manage Acro dependence separately from DAKOTA's for now
  set(ACRO_HAVE_OPTPP ON CACHE BOOL "Acro building with OPT++")
endif()

if(HAVE_SUNDRY)
  add_subdirectory(sundry)
endif()

if(HAVE_UTILIB)
  add_subdirectory(utilib)
endif()

if(HAVE_AMPL AND NOT AMPL_INCLUDE_DIRS)
  set(AMPL_INCLUDE_DIRS ${amplsolver_SOURCE_DIR} ${amplsolver_BINARY_DIR})
endif()

if(HAVE_COLIN)
  if(NOT HAVE_UTILIB)
    message(SEND_ERROR " HAVE_COLIN requires HAVE_UTILIB to be turned ON")
  endif()
  if(NOT HAVE_BOOST)
    message(SEND_ERROR " HAVE_COLIN requires HAVE_BOOST to be turned ON")
  endif()
  if (NOT HAVE_TINYXML)
    message(SEND_ERROR " HAVE_COLIN requires HAVE_TINYXML to be turned ON")
  endif()
  add_subdirectory(colin)
endif()

if(HAVE_PEBBL)
  if(NOT HAVE_UTILIB)
    message(ERROR " HAVE_PEBBL requires HAVE_UTILIB to be turned ON")
  endif()
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utilib/src
                      ${CMAKE_CURRENT_BINARY_DIR}/utilib)
  add_definitions(-DACRO_USING_PEBBL)
  add_subdirectory(pebbl)
endif()

if(HAVE_SCOLIB)
  if(NOT HAVE_COLIN)
      message(ERROR " HAVE_SCOLIB requires HAVE_COLIN to be turned ON")
  endif()
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/pebbl/src
                      ${CMAKE_CURRENT_SOURCE_DIR}/pebbl/src/bb
                      ${CMAKE_CURRENT_SOURCE_DIR}/pebbl/src/pbb
                      ${CMAKE_CURRENT_SOURCE_DIR}/pebbl/src/example
                      ${CMAKE_CURRENT_SOURCE_DIR}/pebbl/src/misc
                      ${CMAKE_CURRENT_BINARY_DIR}/pebbl)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utilib/src
                      ${CMAKE_CURRENT_BINARY_DIR}/utilib)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/colin/src
                      ${CMAKE_CURRENT_BINARY_DIR}/colin)
  add_definitions(-DACRO_USING_SCOLIB)
  add_subdirectory(scolib)
endif()

if(HAVE_INTERFACES)
  # Interfaces can USE OPT++ and SCOLib, but they are not required
  if(NOT HAVE_COLIN)
    message(SEND_ERROR " HAVE_INTERFACES requires HAVE_COLIN to be turned ON")
  endif()
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/scolib/src
    ${CMAKE_CURRENT_SOURCE_DIR}/utilib/src
    ${CMAKE_CURRENT_BINARY_DIR}/utilib
  )
  if (ACRO_HAVE_OPTPP)
    include_directories(
      ${CMAKE_CURRENT_SOURCE_DIR}/optpp/include
      ${CMAKE_CURRENT_BINARY_DIR}/optpp
      )     
  endif()
  if(HAVE_3PO)
    add_definitions(-DACRO_USING_THREEPO)

    find_path(COBYLA_DIR cobyla.h DOC "Path to directory containing cobyla.h")
    if(COBYLA_DIR)
      add_definitions(-DACRO_USING_COBYLA)
    else()
      message(SEND_ERROR "HAVE_3PO is ON, but COBYLA_DIR is not found.  Please set COBYLA_DIR to the location of cobyla2c/cobyla.h")
    endif()

  endif()
  add_subdirectory(interfaces)
endif()

if(HAVE_PICO)
  if(NOT HAVE_PEBBL)
    message(SEND_ERROR " HAVE_PICO requires HAVE_PEBBL to be turned ON")
  endif()
  add_subdirectory(pico)
endif()

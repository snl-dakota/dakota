# - Searches for GNU Scientific Libraries
# See
# http://www.gnu.org/software/gsl/  and
# http://gnuwin32.sourceforge.net/packages/gsl.htm
#
# Adopted from the following blog post:
# http://www.miscdebris.net/blog/2010/06/21/cmake-module-to-find-gnu-scientific-library-findgsl-cmake/
# Which was in turn based on a script of Felix Woelk and Jan Woetzel
# (www.mip.informatik.uni-kiel.de)
#
# It defines the following variables:
#  GSL_FOUND - system has GSL lib
#  GSL_INCLUDE_DIRS - where to find headers
#  GSL_LIBRARIES - full path to the libraries
#  GSL_LIBRARY_DIRS, the directory where the gsl library is found.
#  GSL_CFLAGS  = Unix compiler flags for GSL, essentially "`gsl-config --cflags`"
# 
#  gsl-config is used to define these variables on systems where it can expect
#  to be found.  CMake first searches for gsl-config using the following
#  environment variables:
#  $ENV{GSL_CONFIG}
#  $ENV{GSL_HOME}/bin
#
#  Otherwise, if gsl-config is not installed in a default location, the user
#  is expected to set the GSL_CONFIG variable to point to this executable.
#  On systems where gsl-config is not supported (non-Cygwin, non-MSYS Windows)
#  the user will need to manually specify GSL_INCLUDE_DIR and GSL_LIBRARY 
#  if CMake cannot find them in any default installation location.
#
#  If a user sets the GSL_WITHOUT_CBLAS variable it will cause the cblas
#  library to be excluded from the GSL_LIBRARIES list.
#
#=============================================================================
# Copyright 2001-2012 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

set( GSL_FOUND OFF)
 
# Windows, but not for Cygwin and MSys where gsl-config is available
if( WIN32 AND NOT CYGWIN AND NOT MSYS )

  # look for headers
  find_path( GSL_INCLUDE_DIR
    NAMES gsl/gsl_cdf.h gsl/gsl_randist.h
    )
    
  # look for gsl library
  find_library( GSL_LIBRARY
    NAMES gsl
  )
  if( GSL_LIBRARY )
    set( GSL_INCLUDE_DIRS ${GSL_INCLUDE_DIR} )
    get_filename_component( GSL_LIBRARY_DIRS ${GSL_LIBRARY} PATH )
  endif( GSL_LIBRARY )

  # look for gsl cblas library (unless the user told us not to)
  if( NOT GSL_WITHOUT_CBLAS )
    find_library( GSL_CBLAS_LIBRARY
        NAMES gslcblas
      )
  endif( NOT GSL_WITHOUT_CBLAS )
 
  if( GSL_LIBRARY AND GSL_INCLUDE_DIR AND
      (GSL_CBLAS_LIBRARY OR GSL_WITHOUT_CBLAS) )
    set( GSL_LIBRARIES ${GSL_LIBRARY} ${GSL_CBLAS_LIBRARY} )
    set( GSL_FOUND ON )
  endif()
  
  if( GSL_FOUND )
    mark_as_advanced(
      GSL_INCLUDE_DIR
      GSL_LIBRARY
      GSL_CBLAS_LIBRARY
    )
  endif( GSL_FOUND )

else()
  # Respect relevant environment variables while searching for gsl-config
  #message("Dbg GSL_CONFIG=$ENV{GSL_CONFIG}")
  if(DEFINED ENV{GSL_CONFIG})
    set(GSL_CONFIG_EXECUTABLE "$ENV{GSL_CONFIG}")
  else()
    if(DEFINED ENV{GSL_HOME})
      set(GSL_CONFIG_PREFER_PATH "$ENV{GSL_HOME}/bin")
    endif()
    find_program(GSL_CONFIG_EXECUTABLE gsl-config
      HINTS
        ${GSL_CONFIG_PREFER_PATH}
        /usr/local/bin)
  endif()

  if( GSL_CONFIG_EXECUTABLE )
    set( GSL_FOUND ON )

    # run the gsl-config program to get cflags
    execute_process(
      COMMAND sh "${GSL_CONFIG_EXECUTABLE}" --cflags
      OUTPUT_VARIABLE GSL_CFLAGS
      RESULT_VARIABLE RET
      ERROR_QUIET
      )
    if( RET EQUAL 0 )
      string( STRIP "${GSL_CFLAGS}" GSL_CFLAGS )
      separate_arguments( GSL_CFLAGS )

      # parse definitions from cflags; drop -D* from CFLAGS
      string( REGEX MATCHALL "-D[^;]+"
        GSL_DEFINITIONS  "${GSL_CFLAGS}" )
      string( REGEX REPLACE "-D[^;]+;" ""
        GSL_CFLAGS "${GSL_CFLAGS}" )

      # parse include dirs from cflags; drop -I prefix
      string( REGEX MATCHALL "-I[^;]+"
        GSL_INCLUDE_DIRS "${GSL_CFLAGS}" )
      string( REPLACE "-I" ""
        GSL_INCLUDE_DIRS "${GSL_INCLUDE_DIRS}")
      string( REGEX REPLACE "-I[^;]+;" ""
        GSL_CFLAGS "${GSL_CFLAGS}")
    else( RET EQUAL 0 )
      set( GSL_FOUND FALSE )
    endif( RET EQUAL 0 )

    # run the gsl-config program to get the libs
    execute_process(
      COMMAND sh "${GSL_CONFIG_EXECUTABLE}" --libs
      OUTPUT_VARIABLE GSL_LIB_OUTPUT
      RESULT_VARIABLE RET
      ERROR_QUIET
      )
    if( RET EQUAL 0 )
      string(STRIP "${GSL_LIB_OUTPUT}" GSL_LIB_OUTPUT )
      separate_arguments( GSL_LIB_OUTPUT )

      # extract linkdirs (-L) for rpath (i.e., LINK_DIRECTORIES)
      string( REGEX MATCHALL "-L[^;]+"
        GSL_LIBRARY_DIRS "${GSL_LIB_OUTPUT}" )
      string( REPLACE "-L" ""
        GSL_LIBRARY_DIRS "${GSL_LIBRARY_DIRS}" )

      # `gsl-config --libs` returns a -L value followed by a number of -l values
      # Parse this information to construct absolute paths to the libraries.
      set(GSL_LIBRARIES)
      foreach(lib_arg ${GSL_LIB_OUTPUT})
        string(SUBSTRING "${lib_arg}" 0 2 lib_arg_begin) 
        if("${lib_arg_begin}" STREQUAL "-l")
          string(REPLACE "-l" "" lib_name "${lib_arg}")
          find_library(GSL_LIB "${lib_name}" HINTS "${GSL_LIBRARY_DIRS}")
          list(APPEND GSL_LIBRARIES "${GSL_LIB}")
          unset(GSL_LIB CACHE)
        endif()
      endforeach()

    else( RET EQUAL 0 )
      set( GSL_FOUND FALSE )
    endif( RET EQUAL 0 )

    MARK_AS_ADVANCED(
      GSL_CFLAGS
    )
  else( GSL_CONFIG_EXECUTABLE )
    message( STATUS "FindGSL: gsl-config not found.")
  endif( GSL_CONFIG_EXECUTABLE )
endif( WIN32 AND NOT CYGWIN AND NOT MSYS )

if( GSL_FOUND )
  if( NOT GSL_FIND_QUIETLY )
    message( STATUS "FindGSL: Found GSL headers and library" )
  endif( NOT GSL_FIND_QUIETLY )
else( GSL_FOUND )
  if( GSL_FIND_REQUIRED )
    message( FATAL_ERROR "FindGSL: Could not find GSL headers and/or library" )
  endif( GSL_FIND_REQUIRED )
endif( GSL_FOUND )
 

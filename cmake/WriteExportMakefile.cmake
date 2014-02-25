# Much simplified version of Trilinos Makefile export capability,
# using its naming conventions

# --------------------------------
# BEGIN two functions from TriBITS
# --------------------------------

# ************************************************************************
#
#            TriBITS: Tribial Build, Integrate, and Test System
#                    Copyright 2013 Sandia Corporation
#
# Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
# the U.S. Government retains certain rights in this software.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the Corporation nor the names of the
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ************************************************************************
#
#  This function will take a list and turn it into a space separated string
#  adding the prefix to the front of every entry.
#
FUNCTION(TRIBITS_LIST_TO_STRING LIST PREFIX OUTPUT_STRING)
  SET(LIST_STRING "")

  FOREACH(ITEM ${LIST})
    SET(LIST_STRING "${LIST_STRING} ${PREFIX}${ITEM}")
  ENDFOREACH()

  SET(${OUTPUT_STRING} ${LIST_STRING} PARENT_SCOPE)
ENDFUNCTION()

#
#  This function will take a list of libraries and turn it into a space
#  separated string. In this case though the prefix is not always added
#  to the front of each entry as libraries can be specified either as a
#  name of a library to find or the absolute path to the library file
#  with any decorations the system uses. When an absolute path is given
#  the entry is used verbatim.
#
FUNCTION(TRIBITS_LIBRARY_LIST_TO_STRING LIST PREFIX OUTPUT_STRING)
  SET(LIST_STRING "")

  FOREACH(ITEM ${LIST})
    STRING(SUBSTRING ${ITEM} 0 1 OPTION_FLAG)
    IF(EXISTS ${ITEM} OR OPTION_FLAG STREQUAL "-")
      SET(LIST_STRING "${LIST_STRING} ${ITEM}")
    ELSE()
      SET(LIST_STRING "${LIST_STRING} ${PREFIX}${ITEM}")
    ENDIF()
  ENDFOREACH()

  SET(${OUTPUT_STRING} ${LIST_STRING} PARENT_SCOPE)
ENDFUNCTION()

# --------------------------------
# END two functions from TriBITS
# --------------------------------


# Simplified version of CMake logic to choose opt or debug library
# from list of link libraries (will not work for non-Makefile
# generators).
function(LibsRemoveInactiveConfigs lib_list_in lib_list_out)
  
  set(pruned_list)

  # TODO: determine if this is a debug configuration
  #list(FIND ${DEBUG_CONFIGURATIONS} "${CMAKE_BUILD_TYPE}" found_index)
  string(TOLOWER "${CMAKE_BUILD_TYPE}" lcbt)
  set(debug_flag FALSE)
  if ("${lcbt}" STREQUAL "debug")
    set(debug_flag TRUE)
  endif()
  
  set(libtype "general")
  foreach(item ${lib_list_in})
    if ( ("${item}" STREQUAL "debug") OR ("${item}" STREQUAL "optimized") 
	 OR ("${item}" STREQUAL "general") )
      # assign type
      set(libtype ${item})
    else()
      # process the library name and reset type
      if( ("${libtype}" STREQUAL "debug" AND debug_flag) OR 
	  ("${libtype}" STREQUAL "optimized" AND NOT debug_flag) OR
	  ("${libtype}" STREQUAL "general")
	  )
      	list(APPEND pruned_list "${item}")
      endif()
      set(libtype "general")
    endif()

  endforeach()
  set(${lib_list_out} ${pruned_list} PARENT_SCOPE)
endfunction()


# Set MAKEFILE_* variables and write export Makefiles to the build tree
function(WriteExportMakefile)

  tribits_list_to_string("${Dakota_DEFINES}" "-D" 
    MAKEFILE_DEFINES)
  tribits_list_to_string("${Dakota_INCLUDE_DIRS}" "-I" 
    MAKEFILE_INCLUDE_DIRS)
  tribits_list_to_string("${Dakota_LIBRARY_DIRS}" "${CMAKE_LIBRARY_PATH_FLAG}"
    MAKEFILE_LIBRARY_DIRS)
  tribits_library_list_to_string("${Dakota_LIBRARIES}" 
    "${CMAKE_LINK_LIBRARY_FLAG}" MAKEFILE_LIBRARIES)

  tribits_list_to_string("${Dakota_TPL_INCLUDE_DIRS}" "-I" 
    MAKEFILE_TPL_INCLUDE_DIRS)
  tribits_list_to_string("${Dakota_TPL_LIBRARY_DIRS}" 
    "${CMAKE_LIBRARY_PATH_FLAG}" MAKEFILE_TPL_LIBRARY_DIRS)
  LibsRemoveInactiveConfigs("${Dakota_TPL_LIBRARIES}" pruned_tpl_libs)
  tribits_library_list_to_string("${pruned_tpl_libs}"
    "${CMAKE_LINK_LIBRARY_FLAG}" MAKEFILE_TPL_LIBRARIES)

  set(EXPORT_FILE_VAR_PREFIX Dakota)

  # Configure the build tree Makefile.export
  configure_file(${Dakota_SOURCE_DIR}/cmake/Makefile.export.Dakota.in 
    ${Dakota_BINARY_DIR}/src/Makefile.export.Dakota)

  # Configure the install tree Makefile.export
  set(MAKEFILE_INCLUDE_DIRS "-I${CMAKE_INSTALL_PREFIX}/include")
  set(MAKEFILE_LIBRARY_DIRS 
    "${CMAKE_LIBRARY_PATH_FLAG}${CMAKE_INSTALL_PREFIX}/lib")
  configure_file(${Dakota_SOURCE_DIR}/cmake/Makefile.export.Dakota.in 
    ${Dakota_BINARY_DIR}/src/Makefile.export.Dakota.install)

endfunction()

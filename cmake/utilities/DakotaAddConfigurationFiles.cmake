# This function checks for the existence of a configuration file and
# adds it to CTEST_CONFIGURE_COMMAND.
#
# set_configure_command _filename
#  [SUBDIR <subdirectory>]
#  [REQUIRED] )
#
# This function will look for _filename in its configuration directories:
#   $DAKOTA_CONFIG_DIR/[SUBDIR]
#   $DAKOTA_LOCAL_CONFIG_DIR/[SUBDIR]
# If the file exists, it adds the file to the CTEST_CONFIGURE_COMMAND.
# If the file is required and does not exist, CMake exits with an error.

function( set_configure_command _filename )
   
  set( _option_args REQUIRED )
  set( _one_value_keyword_args SUBDIR  )
  set( _multi_value_keyword_args "" )

  cmake_parse_arguments( 
    _addnl_args
    "${_option_args}"
    "${_one_value_keyword_args}"
    "${_multi_value_keyword_args}"
    ${ARGN} )

  # Set up path for filename
  #message ("filename: [${_filename}]")
  #message("_addnl_args_SUBDIR: [${_addnl_args_SUBDIR}]")
  #message("_addnl_args_REQUIRED: [${_addnl_args_REQUIRED}]")
  if ( NOT _addnl_args_SUBDIR )
    #message("no subdir needed")
    set( _addnl_args_SUBDIR "" )
    set( localDir ${DAKOTA_LOCAL_CONFIG_DIR} )
    set( publicDir ${DAKOTA_CONFIG_DIR} )
  else()
    #message("setting subdir ${_addnl_args_SUBDIR}") 
    set( localDir "${DAKOTA_LOCAL_CONFIG_DIR}/${_addnl_args_SUBDIR}" )
    set( publicDir "${DAKOTA_CONFIG_DIR}/${_addnl_args_SUBDIR}" )
  else()

  endif()

  # look for filename in the local directory first
  #message("looking for ${localDir}/${_filename}" )
  if ( EXISTS ${localDir}/${_filename} )
    set( _filename "${localDir}/${_filename}" )
    set ( fileFound TRUE )
    #message("file ${_filename} found")
  else()
    # then in the public directory
    #message("looking for ${publicDir}/${_filename}" )
    if ( EXISTS ${publicDir}/${_filename} )
      set( _filename "${publicDir}/${_filename}" )
      set ( fileFound TRUE )
      #message("file ${_filename} found")
    endif()
  endif()    

  if ( fileFound )
    #message("setting configure command")
    set( CTEST_CONFIGURE_COMMAND
    	 "${CTEST_CONFIGURE_COMMAND} \"-C${_filename}\"" PARENT_SCOPE ) 
    #message ("CTEST_CONFIGURE_COMMAND:[${CTEST_CONFIGURE_COMMAND}]")

  # if still not found, and it is required, exit with an error message
  else()
    if ( _addnl_args_REQUIRED )
      message( FATAL_ERROR "ERROR: ${_filename} does not exist" )
    endif()

  endif()

endfunction()


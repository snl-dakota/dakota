# Main option
option(DAKOTA_APPLE_FIX_BOOSTLIBS
  "Fix up Apple Boost libraries lacking absolute install names" FALSE)

# Two alternate sub-options considered
## option(DAKOTA_APPLE_FIX_RPATH "Adjust embedded @rpath on libraries/executables" FALSE)
## option(DAKOTA_APPLE_BOOST_ABSPATH "Copy Boost libs to build; embed absolute paths" TRUE)

# Copy any Boost_LIBRARIES identified by FindBoost.cmake to specified
# dest_dir (will not account for secondary libs not passed as
# COMPONENTS to FindBoost, e.g., filesystem needs system
## function(dakota_copy_boost_libs dest_dir)
##   message(STATUS "Copying Boost libraries ${Boost_LIBRARIES} to ${dest_dir}")
##   if(NOT EXISTS "${dest_dir}")
##     file(MAKE_DIRECTORY "${dest_dir}")
##   endif()
##   foreach(boost_lib ${Boost_LIBRARIES})
##     file(COPY ${boost_lib} DESTINATION "${dest_dir}")
##   endforeach()
## endfunction()

# Copy specified boost_libs, e.g., regex, serialization, from src_dir
# to dest_dir when we haven't yet found Boost
function(dakota_copy_specific_boost_dylibs src_dir boost_libs dest_dir)
  message(STATUS "Copying libboost_{${boost_libs}}.dylib libraries from ${src_dir} to ${dest_dir}")
  if(NOT EXISTS "${dest_dir}")
    file(MAKE_DIRECTORY "${dest_dir}")
  endif()
  foreach(boost_lib ${boost_libs})
    set(_src_file "${src_dir}/libboost_${boost_lib}.dylib")
    if(EXISTS "${_src_file}")
      file(COPY "${_src_file}" DESTINATION "${dest_dir}")
    else()
      message(WARNING "Could not find ${_src_file} to copy")
    endif()
  endforeach()
endfunction()


# Modify the copied Boost libs so filesystem knows to find system
# (There are currently no other broken ones)
# Could instead consider a dakota-env copy of SEMS Boost with an @rpath change.
## function(dakota_fix_libboost_filesystem lib_dir)
##   message(STATUS "Embedding @rpath/libboost_system.dylib in ${lib_dir}/libboost_filesystem.dylib")
##   execute_process(COMMAND install_name_tool -change
##     libboost_system.dylib @rpath/libboost_system.dylib
##     "${lib_dir}/libboost_filesystem.dylib"
##     )
## endfunction()

# Embed all absolute install_names in the specified libraries in lib_dir
function(dakota_boost_abs_install_names lib_dir boost_libs)
  message(STATUS "Embedding absolute install names in Boost libs ${boost_libs} in ${lib_dir}")
  foreach(boost_lib ${boost_libs})
    execute_process(COMMAND ${Dakota_SOURCE_DIR}/cmake/fix_boost_libs.sh
      ${lib_dir} libboost_${boost_lib}.dylib
      )
  endforeach()
endfunction()


# Prepend @rpath/ to the specified full, bare, library name in the
# specified target's embedded library info
## function(dakota_prepend_rpath target linked_lib)
##   add_custom_command(TARGET ${target} POST_BUILD
##     COMMAND install_name_tool -change "${linked_lib}" "@rpath/${linked_lib}"
##       "$<TARGET_FILE:${target}>"
##     )
## endfunction()


# Prepend @rpath/ to each of the bare Boost library names in the
# specified executable
## function(dakota_prepend_boost_rpath target)
##   if(NOT DAKOTA_APPLE_FIX_RPATH)
##     return()
##   endif()
##   message(STATUS "Prepending @rpath/ to Boost libraries in target ${target}")
##   foreach(boost_lib ${Boost_LIBRARIES})
##     get_filename_component(lib_name ${boost_lib} NAME)
##     dakota_prepend_rpath(${target} ${lib_name})
##   endforeach()
## endfunction()

# Copy boost dylibs and fix-up to have aboslute install names
#
# Publishes variables _dakota_boost_{root,libdir,includedir} that can
# be used by caller
macro(dakota_fix_boost_dylibs requested_boost_libs dest_boost_libdir)

  # Trying not to reproduce all of FindBoost.cmake, just current use cases

  # Parse out include and lib from root if needed
  if(BOOST_ROOT)
    set(_dakota_boost_root "${BOOST_ROOT}")
  elseif(BOOSTROOT)
    set(_dakota_boost_root "${BOOSTROOT}")
  elseif(DEFINED ENV{BOOST_ROOT})
    set(_dakota_boost_root "$ENV{BOOST_ROOT}")
  endif()

  if(BOOST_LIBRARYDIR)
    set(_dakota_boost_libdir "${BOOST_LIBRARYDIR}")
  elseif(DEFINED ENV{BOOST_LIBRARYDIR})
    set(_dakota_boost_libdir "$ENV{BOOST_LIBRARYDIR}")
  elseif(_dakota_boost_root AND IS_DIRECTORY "${_dakota_boost_root}/lib")
    set(_dakota_boost_libdir "${_dakota_boost_root}/lib")
    message("Assuming Boost libraries in ${_dakota_boost_libdir}")
  else()
    message(FATAL_ERROR "Cannot find Boost library dir to fix up dylibs; consider setting BOOST_LIBRARYDIR")
  endif()

  if(BOOST_INCLUDEDIR)
    set(_dakota_boost_includedir "${BOOST_INCLUDEDIR}")
  elseif(DEFINED ENV{BOOST_INCLUDEDIR})
    set(_dakota_boost_includedir "$ENV{BOOST_INCLUDEDIR}")
  elseif(_dakota_boost_root AND IS_DIRECTORY "${_dakota_boost_root}/boost")
    set(_dakota_boost_includedir "${_dakota_boost_root}")
    message("Assuming Boost headers in ${_dakota_boost_includedir}")
  elseif(_dakota_boost_root AND IS_DIRECTORY "${_dakota_boost_root}/include/boost")
    set(_dakota_boost_includedir "${_dakota_boost_root}/include")
    message("Assuming Boost headers in ${_dakota_boost_includedir}")
  else()
    message(FATAL_ERROR "Cannot find Boost include dir when fixing up dylibs; consider setting BOOST_INCLUDEDIR")
  endif()

  dakota_copy_specific_boost_dylibs("${_dakota_boost_libdir}"
    "${requested_boost_libs}" "${dest_boost_libdir}")

  dakota_boost_abs_install_names("${dest_boost_libdir}"
    "${requested_boost_libs}")

endmacro()

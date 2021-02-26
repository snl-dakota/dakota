# Copy any Boost_LIBRARIES identified by FindBoost.cmake to specified
# dest_dir (will not account for secondary libs not passed as
# COMPONENTS to FindBoost, e.g., filesystem needs system
function(dakota_copy_boost_libs dest_dir)
  message(STATUS "Copying Boost libraries ${Boost_LIBRARIES} to ${dest_dir}")
  if(NOT EXISTS "${dest_dir}")
    file(MAKE_DIRECTORY "${dest_dir}")
  endif()
  foreach(boost_lib ${Boost_LIBRARIES})
    file(COPY ${boost_lib} DESTINATION "${dest_dir}")
  endforeach()
endfunction()

# Modify the copied Boost libs so filesystem knows to find system
# (There are currently no other broken ones)
# Could instead consider a dakota-env copy of SEMS Boost with an @rpath change.
function(dakota_fix_libboost_filesystem lib_dir)
  message(STATUS "Embedding @rpath/libboost_system.dylib in ${lib_dir}/libboost_filesystem.dylib")
  execute_process(COMMAND install_name_tool -change
    libboost_system.dylib @rpath/libboost_system.dylib
    "${lib_dir}/libboost_filesystem.dylib"
    )
endfunction()

# Prepend @rpath/ to the specified full, bare, library name in the
# specified target's embedded library info
function(dakota_prepend_rpath target linked_lib)
  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND install_name_tool -change "${linked_lib}" "@rpath/${linked_lib}"
      "$<TARGET_FILE:${target}>"
    )
endfunction()


# Prepend @rpath/ to each of the bare Boost library names in the
# specified executable
function(dakota_prepend_boost_rpath target)
  message(STATUS "Prepending @rpath/ to Boost libraries in target ${target}")
  foreach(boost_lib ${Boost_LIBRARIES})
    get_filename_component(lib_name ${boost_lib} NAME)
    dakota_prepend_rpath(${target} lib_name)
  endforeach()
endfunction()

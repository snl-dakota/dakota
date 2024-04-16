

if ( DEFINED ENV{WORKSPACE} )
  # By convention, all Dakota, jenkins-driven build jobs use a 'build'
  # subdir for clear separation of source and build trees in the WORKSPACE
  set( _build_dir $ENV{WORKSPACE}/build )
else() # assume make package was run in the build folder
  set( _build_dir $ENV{PWD} )
endif()

message("Installing TPLs and updating RPATHs and names")
set(_script_path ${_build_dir}/install_macos_libs.py)
if(NOT EXISTS ${_script_path})
  message("Could not locate installation script at ${_script_path}; skipping TPL installation and updates; portability of the package/install may be reduced")
else()
  execute_process(
      COMMAND python3 ${_build_dir}/install_macos_libs.py ${CMAKE_INSTALL_PREFIX}
      RESULT_VARIABLE _install_result
  )
  if(NOT _install_result EQUAL 0)
    message("TPL installation and update failed; portability of the package/install may be reduced")
  endif()
endif()


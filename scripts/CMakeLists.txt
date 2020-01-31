install(PROGRAMS "dprepro.perl" DESTINATION "bin")
# dakota.bat is useful in both native and Cygwin due to command prompt usage
add_subdirectory("pyprepro")
if (MSVC OR CYGWIN)
  install(PROGRAMS "dakota.bat" DESTINATION "bin")
endif()
# dakota.sh is useful for all except native windows builds
if(NOT MSVC)
  install(PROGRAMS "dakota.sh" DESTINATION "bin")
endif()


# Install dyanamic deps, omitting standard system libs
# CMake variables aren't passed to the script, so we have to invoke
# separate scripts for each platform
#
# NOTE: This code is in this directory to force these install scripts
# to run after other directories have completed their install steps.
#
if (DAKOTA_INSTALL_DYNAMIC_DEPS)
  if (CYGWIN)
    install(SCRIPT "${Dakota_SOURCE_DIR}/cmake/InstallCygwinDLLs.cmake")
  elseif (APPLE)
    install(SCRIPT "${Dakota_SOURCE_DIR}/cmake/InstallDarwinDylibs.cmake")
  elseif (UNIX)
#    include(InstallRequiredSystemLibraries)
    install(SCRIPT "${Dakota_SOURCE_DIR}/cmake/InstallUnixSOs.cmake")
  elseif (WIN32)
    include(InstallWinDLLs)
  endif()
endif()

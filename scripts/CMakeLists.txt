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


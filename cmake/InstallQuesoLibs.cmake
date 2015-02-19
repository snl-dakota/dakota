# Find the QUESO libs and install to
# ${CMAKE_INSTALL_PREFIX}/lib

# NOTE: This script will only work for make install from top of build tree
# TODO: Review string quoting conventions and test with spaces in filename

message( "CMAKE_CURRENT_BINARY_DIR (1): ${CMAKE_CURRENT_BINARY_DIR}" ) 
if ( DAKOTA_JENKINS_BUILD OR DEFINED ENV{WORKSPACE} )
  # By convention, all Dakota, jenkins-driven build jobs use a 'build'
  # subdir for clear separation of source and build trees in the WORKSPACE
  set( CMAKE_CURRENT_BINARY_DIR $ENV{WORKSPACE}/build )
elseif ( NOT CMAKE_CURRENT_BINARY_DIR )
  set( CMAKE_CURRENT_BINARY_DIR $ENV{PWD} )
endif()
message( "CMAKE_CURRENT_BINARY_DIR (2): ${CMAKE_CURRENT_BINARY_DIR}" ) 

# On some platforms, these libraries get installed to $prefix/lib64
file(GLOB queso_libs "${CMAKE_CURRENT_BINARY_DIR}/packages/queso_ext/lib/*")
file(GLOB queso64_libs "${CMAKE_CURRENT_BINARY_DIR}/packages/queso_ext/lib64/*")

# Process each library and install
foreach(dakota_lib ${queso_libs})
  file(INSTALL "${dakota_lib}" DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
endforeach()

foreach(dakota_lib ${queso64_libs})
  file(INSTALL "${dakota_lib}" DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64")
endforeach()

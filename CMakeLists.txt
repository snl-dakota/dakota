# Dakota itself requires this CMake version; some TPLs may require even newer
cmake_minimum_required(VERSION 3.1)

set(CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/semsCMake
  ${CMAKE_MODULE_PATH}
  )

# We delay enabling languages until developer compiler flags are set,
# even though we no longer force the compiler to the MPI wrapper
project("Dakota" NONE)

message(STATUS "Configuring Dakota with CMake ${CMAKE_VERSION}")

# Dakota requires C++11
include(DakotaCxxOptions)
dakota_cxx_standard()

# Publish all top-level and Dakota package options
include(DakotaOptions)

# DAKOTA versions for packaging
set(Dakota_VERSION_MAJOR 6)
set(Dakota_VERSION_MINOR 11)
set(Dakota_VERSION_PATCH 0)
set(Dakota_VERSION_PAIR
  "${Dakota_VERSION_MAJOR}.${Dakota_VERSION_MINOR}")
set(Dakota_VERSION_TRIPLE
  "${Dakota_VERSION_MAJOR}.${Dakota_VERSION_MINOR}.${Dakota_VERSION_PATCH}")

# --- Release time toggle 
# Toggle this to append to the version number for stable
#set(Dakota_VERSION_APPEND_STABLE "")   # Versioned release
set(Dakota_VERSION_APPEND_STABLE "+")  # Stable releases

# --- Release time toggle 
# If empty, the build date will be used as the release date (could consider 
# using the Git rev date); use format Mmm dd yyyy, as in asctime()
#set(Dakota_RELEASE_DATE "Nov 15 2019")    # Versioned release date Mmm dd yyyy
set(Dakota_RELEASE_DATE "")               # Stable release: empty -> build date

# Dakota version to use for naming archives

set(Dakota_VERSION_STRING "${Dakota_VERSION_MAJOR}.${Dakota_VERSION_MINOR}.${Dakota_VERSION_PATCH}${Dakota_VERSION_APPEND_STABLE}")

# Dakota version to use in source/docs (could use for tarball names?)
# For now, we don't use .0 for new minor releases, use 5.4 not 5.4.0
if (${Dakota_VERSION_PATCH} GREATER 0) 
  set(Dakota_VERSION_SRC 
    "${Dakota_VERSION_TRIPLE}${Dakota_VERSION_APPEND_STABLE}")
else()
  set(Dakota_VERSION_SRC 
    "${Dakota_VERSION_PAIR}${Dakota_VERSION_APPEND_STABLE}")
endif()


# Installation destination options
# (relative to ${CMAKE_INSTALL_PREFIX}, unless absolute)
set(DAKOTA_EXAMPLES_INSTALL "share/dakota" CACHE STRING
  "Installation destination for DAKOTA examples/ dir")
set(DAKOTA_TEST_INSTALL "share/dakota" CACHE STRING
  "Installation destination for DAKOTA test/ dir")
set(DAKOTA_TOPFILES_INSTALL "share/dakota" CACHE STRING
  "Installation destination for DAKOTA top-level files")


# Location for generated files
execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory
  "${Dakota_BINARY_DIR}/generated")
# TODO: Consistent generated file locations and directory creation.  For now:
execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory
  "${Dakota_BINARY_DIR}/generated/src")
execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory
  "${Dakota_BINARY_DIR}/generated/VERSION")

# Buiding in a Git repo or a source package?
set(DAKOTA_VERSION_file_path)
set(DAKOTA_GIT_checkout FALSE)
if(EXISTS ${Dakota_SOURCE_DIR}/.git)
  set(DAKOTA_GIT_checkout TRUE)
  # GIT checkout, determine revision level

  find_package(Git)

  if(GIT_FOUND)
    # Workarounds for Trilinos, where CMAKE_PROJECT_NAME != Dakota
    # and the Dakota/ directory may be symlinked.
    get_filename_component(abs_source_dir ${Dakota_SOURCE_DIR} REALPATH)
    
    # Get the abbreviated SHA1 of the most recent commit
    execute_process(COMMAND ${GIT_EXECUTABLE} log --pretty=format:%h -1 
      WORKING_DIRECTORY ${abs_source_dir}
      OUTPUT_VARIABLE Dakota_GIT_ABBREV_SHA1)
    # Get the date and time of the most recent commit
    execute_process(COMMAND ${GIT_EXECUTABLE} log --pretty=format:%ci -1 
      WORKING_DIRECTORY ${abs_source_dir}
      OUTPUT_VARIABLE Dakota_GIT_DATETIME)
    # Extract the date
    string(SUBSTRING "${Dakota_GIT_DATETIME}" 0 10 Dakota_GIT_DATE)
    # Build revision string
    ##string(CONFIGURE "@Dakota_GIT_ABBREV_SHA1@ (@Dakota_GIT_DATE@)" Dakota_GIT_REV)
    set(Dakota_GIT_REV "${Dakota_GIT_ABBREV_SHA1} (${Dakota_GIT_DATE})")
  endif(GIT_FOUND) # GIT_FOUND

  # Create VERSION file
  file( WRITE ${Dakota_BINARY_DIR}/generated/VERSION/VERSION
        "DakotaVersion ${Dakota_VERSION_SRC}
Built from GIT revision ${Dakota_GIT_REV}
" )
  set(DAKOTA_VERSION_file_path "${Dakota_BINARY_DIR}/generated/VERSION/")
  install(FILES ${Dakota_BINARY_DIR}/generated/VERSION/VERSION DESTINATION
	  ${DAKOTA_TOPFILES_INSTALL})

  message(STATUS "Dakota release version is: ${Dakota_VERSION_SRC}")
  message(STATUS "Dakota git revision is: ${Dakota_GIT_REV}")

else() # source tree NOT a Git checkout
  message(STATUS "Dakota release version is: ${Dakota_VERSION_SRC}")
  message(STATUS "Appear to building from source package.")
endif() # .git directory conditionals

# More widely distributed packages exclude more content
# So smaller number implies less content
set(DAKOTA_InternalFull 3)       # Complete checkout
set(DAKOTA_InternalSupervised 2) # Less some of local/
set(DAKOTA_InternalWeb 1)        # Also less DOT
set(DAKOTA_ExternalWeb 0)        # Also less NPSOL, NLPQL

# The permission level of the distribution (to be made an option)
# TODO: change default to DAKOTA_ExternalWed?
set(DAKOTA_DISTRO ${DAKOTA_InternalFull} CACHE STRING
  "Dakota Distribution Type")

# Initial CPack Settings.  Done here as affects configuration in packages/
# (DOT, NPSOL, NLPQL)
set(CPACK_PACKAGE_VENDOR "Sandia National Laboratories")

set(local_arch "${CMAKE_SYSTEM_NAME}.${CMAKE_SYSTEM_PROCESSOR}")
if(WIN32)
  # We always build 32-bit Windows binaries and don't want to confuse users
  # with the processor name from a 64-bit build host
  set(local_arch "${CMAKE_SYSTEM_NAME}.x86")
endif()

# TODO: insert distribution type into package names, i.e., instead of 
# ${Dakota_VERSION_STRING} use ${Dakota_VERSION_STRING}.int_web, based on DAKOTA_DISTRO
set(CPACK_PACKAGE_FILE_NAME "dakota-${Dakota_VERSION_STRING}.${local_arch}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "dakota-${Dakota_VERSION_STRING}.src")
set(CPACK_PACKAGE_VERSION_MAJOR "${Dakota_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Dakota_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${Dakota_VERSION_PATCH}")
set(CPACK_DAKOTA_BINARY_DIR "${Dakota_BINARY_DIR}")

# Always omit .svn directories, .git directories, duplicate TPLs, and GPL packages
# FFTW is GPL, CDDLIB is GPL
set(CPACK_SOURCE_IGNORE_FILES ".*\\\\.git/" ".*\\\\.git.*"
  "^${Dakota_SOURCE_DIR}/sync.local-git.sh"
  "^${Dakota_SOURCE_DIR}/packages/external/fftw/"
  "^${Dakota_SOURCE_DIR}/packages/external/hopspack/src/src-citizens/citizen-gss/cddlib/"
  )

# Only internal full (developer version) has these things
if(DAKOTA_DISTRO LESS ${DAKOTA_InternalFull})
  list(APPEND CPACK_SOURCE_IGNORE_FILES
    "^${Dakota_SOURCE_DIR}/local/cmake/"
    "^${Dakota_SOURCE_DIR}/local/examples/"
    "^${Dakota_SOURCE_DIR}/local/scripts/"
    "^${Dakota_SOURCE_DIR}/local/snl/"
  )
endif()

if(DAKOTA_DISTRO LESS ${DAKOTA_InternalSupervised})
  message(STATUS "Removing DOT for less than InternalSupervised build")
  # When building InternalWeb, don't want DOT in the binaries
  # Be aggressive and remove from source tree
  file(REMOVE_RECURSE ${Dakota_SOURCE_DIR}/local/packages/DOT/)
endif()

if(DAKOTA_DISTRO LESS ${DAKOTA_InternalWeb})
  # Be aggressive and don't distribute any localfiles outside SNL
  # External integrators may need to change this
  list(APPEND CPACK_SOURCE_IGNORE_FILES "^${Dakota_SOURCE_DIR}/local")
  message(STATUS "Removing NPSOL, NLPQL for less than InternalWeb build")
  # When building InternalWeb, don't want NLPQL, NPSOL in the binaries
  # Be aggressive and remove from source tree
  file(REMOVE_RECURSE "${Dakota_SOURCE_DIR}/local/packages/NLPQL"
    "${Dakota_SOURCE_DIR}/local/packages/NPSOL")
endif()

# Default RPATH settings
#
# These settings must take into consideration the presence of shared
# libraries in both bin/ and /lib, and applications in bin/ (formerly
# test/, which is now managed in share/dakota/test; see override in
# test/CMakeLists.txt).
#
#
# $ORIGIN (@executable_path on Mac) is replaced at runtime by the path
# of the executable (application or .so).
#
# Add:
#   $ORIGIN so executables can find shared libs in their own folder
#   $ORIGIN/../lib so that executables in bin and test can find shared libs in the lib folder
#   $ORIGIN/../bin so that exectuables in lib and test can find shared libs in the bin folder.
if(APPLE)
  set(CMAKE_MACOSX_RPATH TRUE CACHE BOOL "Add @rpath to library install names")
  set(CMAKE_INSTALL_RPATH "@executable_path;@executable_path/../lib;@executable_path/../bin"
    CACHE STRING "Set the RPATH in Dakota executables and libraries")
elseif(UNIX)
   set(CMAKE_INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../lib:\$ORIGIN/../bin"
     CACHE STRING "Set the RPATH in Dakota executables and libraries")
endif()


# Manage developer convenience options (experimental)
if(DevDebug)
  include(DakotaDebug)
  include(DakotaDev)
elseif (DevDistro)
  include(DakotaDistro)
  include(DakotaDev)
endif()


# Now check languages after compiler flags and options are set
enable_language(C)
enable_language(CXX)
dakota_check_cxx11()
enable_language(Fortran)
if(MSVC_VERSION EQUAL 1400 OR MSVC_VERSION GREATER 1400 OR MSVC10)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE 
    -D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_SCL_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
endif()

# Perl is required for examples, docs, and system tests
find_package(Perl REQUIRED)

# Python is optionally required by Dakota, Teuchos, and Acro; probe
# for the interpreter here at top-level:
include(FindPythonInterp)

# Conditionally find Java JDK needed for input spec, docs, Java API
include(DakotaFindJava)
dakota_find_java()

# Locate MPI compiler and library settings (must be after languages enabled)
# The DakotaFindMPI() function checks whether the option is enabled
include(DakotaMPI)
DakotaFindMPI()


include(FortranCInterface)
include(CheckFunctionExists)
include(CheckIncludeFile)

if( CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE )
  message(FATAL_ERROR "In-source builds are not allowed.
Please create a directory and run cmake from there, passing the path
to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory `CMakeFiles'.
Please delete them.")
endif()

add_definitions("-DHAVE_CONFIG_H")

# This no longer works as Trilinos CMake rejects custom CMAKE_BUILD_TYPE
# Add CMAKE_BUILD_TYPE for DISTRIBUTION to match historical tests / releases
#set(CMAKE_C_FLAGS_DISTRIBUTION "-O2")
#set(CMAKE_CXX_FLAGS_DISTRIBUTION "-O2")
#set(CMAKE_Fortran_FLAGS_DISTRIBUTION "-O2")

# Note: May need this in some Cygwin builds as well
if(CMAKE_SYSTEM_NAME MATCHES Darwin)
  set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS
      "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -undefined dynamic_lookup")
endif()


include(DakotaFindSystemTPLs)
# Unconditionally find BLAS/LAPACK or equivalent
dakota_find_linalg()
# Conditionally find GSL; needed by both packages/ (QUESO) and src/
dakota_find_gsl()
# Conditionally find HDF5; needed by both src/ and test/
dakota_find_hdf5()
# Unconditionally find Boost
dakota_find_boost()


include(CTest)


# Set the export name for install targets; parent packages likely want
# to override this to the name of their target
set(ExportTarget DakotaTargets CACHE STRING 
  "Export set name for ${CMAKE_PROJECT_NAME}")

# Unconditionally find Trilinos/Teuchos, optionally ROL
include(DakotaFindTrilinos)
dakota_find_trilinos()

# Conditionally find X-Windows graphics deps
include(DakotaXGraphics)
dakota_find_x_graphics()

if(DAKOTA_F90)
  FortranCInterface_HEADER(dak_f90_config.h MACRO_NAMESPACE DAK_F90_)
  install(FILES ${Dakota_BINARY_DIR}/dak_f90_config.h DESTINATION include)
endif(DAKOTA_F90)


add_subdirectory(packages)

add_subdirectory(src)
# Build the executables in the test directory even if BUILD_TESTING is OFF
if(DAKOTA_ENABLE_TESTS)
  add_subdirectory(test)
endif()

if(ENABLE_DAKOTA_DOCS)
  add_subdirectory(docs)
endif()

add_subdirectory(examples)

add_subdirectory(interfaces)

# Top-level install rules from source (binary rules are in subdirs)
install(FILES INSTALL LICENSE COPYRIGHT README
	${DAKOTA_VERSION_file_path}VERSION
  DESTINATION ${DAKOTA_TOPFILES_INSTALL})

install(DIRECTORY examples DESTINATION ${DAKOTA_EXAMPLES_INSTALL}
  USE_SOURCE_PERMISSIONS
  PATTERN "CMakeLists.txt" EXCLUDE
  PATTERN "*.c" EXCLUDE 
  PATTERN "*.f" EXCLUDE 
  )

install(DIRECTORY test DESTINATION ${DAKOTA_TEST_INSTALL}
  PATTERN "CMakeLists.txt" EXCLUDE
  PATTERN "*.c" EXCLUDE 
  PATTERN "*.cpp" EXCLUDE 
  PATTERN "*.f" EXCLUDE 
  )

# Install helper scripts to bin/:
# NOTE: This directory manages dynamic library manipulation at install time,
# as the install script must be called after src/ and test/ are installed.
add_subdirectory("scripts")


# Final CPack settings

# Create Add generated files to source package
if(DAKOTA_GIT_checkout)
  SET(CPACK_SOURCE_INSTALLED_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR};/") 
  LIST(APPEND CPACK_SOURCE_INSTALLED_DIRECTORIES
	"${Dakota_BINARY_DIR}/generated/src/;/src")
  LIST(APPEND CPACK_SOURCE_INSTALLED_DIRECTORIES
	"${Dakota_BINARY_DIR}/generated/VERSION/;/")
endif()

if(WIN32)
  set(CPACK_GENERATOR ZIP)
  set(CPACK_SOURCE_GENERATOR ZIP)
elseif(CYGWIN)
  # The Cygwin-specific generators require additional information to
  # create packages that work with Cygwin's package manager (we suspect)
  #set(CPACK_GENERATOR CygwinBinary)
  #set(CPACK_SOURCE_GENERATOR CygwinSource)
  set(CPACK_GENERATOR ZIP)
  set(CPACK_SOURCE_GENERATOR ZIP)
else()
  # All unix-like systems EXCEPT cygwin (WJB - ToDo:  MacOSX-specific packages)
  set(CPACK_GENERATOR TGZ)
  set(CPACK_SOURCE_GENERATOR TGZ)
endif()


include(CPack)

## Dakota requires CMake 2.8.12 or newer; some TPLs may require even newer
cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/semsCMake
  ${CMAKE_MODULE_PATH}
  )

# We delay enabling languages until developer compiler flags are set,
# even though we no longer force the compiler to the MPI wrapper
project("Dakota" NONE)

# DAKOTA versions for packaging
set(Dakota_VERSION_MAJOR 6)
set(Dakota_VERSION_MINOR 6)
set(Dakota_VERSION_PATCH 0)
# TODO?: set(Dakota_VERSION_TWEAK SVNREV)
set(Dakota_VERSION_PAIR
  "${Dakota_VERSION_MAJOR}.${Dakota_VERSION_MINOR}")
set(Dakota_VERSION_TRIPLE
  "${Dakota_VERSION_MAJOR}.${Dakota_VERSION_MINOR}.${Dakota_VERSION_PATCH}")

# --- Release time toggle 
# Toggle this to append to the version number for stable
set(Dakota_VERSION_APPEND_STABLE "")   # Versioned release
#set(Dakota_VERSION_APPEND_STABLE "+")  # Stable releases

# --- Release time toggle 
# If empty, the build date will be used as the release date (could consider 
# using the SVN rev date); use format Mmm dd yyyy, as in asctime()
set(Dakota_RELEASE_DATE "May 15 2016")    # Versioned release date Mmm dd yyyy
#set(Dakota_RELEASE_DATE "")               # Stable release: empty -> build date 

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

# Location for generated files
execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory
  "${Dakota_BINARY_DIR}/generated")
# TODO: Consistent generated file locations and directory creation.  For now:
execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory
  "${Dakota_BINARY_DIR}/generated/src")
execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory
  "${Dakota_BINARY_DIR}/generated/VERSION")

# Buiding in an SVN checkout or a source package? 
set(DAKOTA_VERSION_file_path)
if(EXISTS ${Dakota_SOURCE_DIR}/.svn)
  set(DAKOTA_SVN_checkout TRUE)
  # SVN checkout, determine revision level

  if((APPLE AND DEFINED ENV{BUILD_URL}) OR (CYGWIN AND DEFINED ENV{BUILD_URL}))
    # Workarounds for Jenkins slave shell where svn is different version
    # than the svn client that performed the 'update' step

    file(STRINGS ${Dakota_SOURCE_DIR}/.svn/entries
      list_dakota_rev LIMIT_COUNT 4)

    # Get the trailing (or 4th) entry (SVN rev of Dakota)
    # Would also want to silence CMP0007, or remove blanks
    list(GET list_dakota_rev -1 Dakota_SVN_REV)

  else()
    # NOT CYGWIN/jenkins so "assert" that the same svn client is used for
    # 'update' and 'configure' steps
    find_package(Subversion)

    if(Subversion_FOUND)
      # Workarounds for Trilinos, where CMAKE_PROJECT_NAME != Dakota
      # and the Dakota/ directory may be symlinked.
      get_filename_component(abs_source_dir ${Dakota_SOURCE_DIR} REALPATH)
      Subversion_WC_INFO(${abs_source_dir} Dakota)

      set(Dakota_SVN_REV "${Dakota_WC_REVISION}")
    endif(Subversion_FOUND) # Subversion_FOUND
  endif()

  # Create VERSION file
  file( WRITE ${Dakota_BINARY_DIR}/generated/VERSION/VERSION
        "DAKOTA Version ${Dakota_VERSION_SRC}
Built from SVN revision ${Dakota_SVN_REV}
" )
  set(DAKOTA_VERSION_file_path "${Dakota_BINARY_DIR}/generated/VERSION/")
  install(FILES ${Dakota_BINARY_DIR}/generated/VERSION/VERSION DESTINATION
	  .)

  message(STATUS "Dakota release version is: ${Dakota_VERSION_SRC}")
  message(STATUS "Dakota subversion revision is: ${Dakota_SVN_REV}")

elseif(EXISTS ${Dakota_SOURCE_DIR}/.git)
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
    string(CONFIGURE "@Dakota_GIT_ABBREV_SHA1@ (@Dakota_GIT_DATE@)" Dakota_GIT_REV)
  endif(GIT_FOUND) # GIT_FOUND

  # Create VERSION file
  file( WRITE ${Dakota_BINARY_DIR}/generated/VERSION/VERSION
        "DakotaVersion ${Dakota_VERSION_SRC}
Built from GIT revision ${Dakota_GIT_REV}
" )
  set(DAKOTA_VERSION_file_path "${Dakota_BINARY_DIR}/generated/VERSION/")
  install(FILES ${Dakota_BINARY_DIR}/generated/VERSION/VERSION DESTINATION
	  .)

  message(STATUS "Dakota release version is: ${Dakota_VERSION_SRC}")
  message(STATUS "Dakota git revision is: ${Dakota_GIT_REV}")

else() # source tree NOT an SVN or GIT checkout
  set(DAKOTA_SVN_checkout FALSE)
  message(STATUS "Dakota release version is: ${Dakota_VERSION_SRC}")
  message(STATUS "Appear to building from source package.")
endif() # .svn .git directory conditionals

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
set(CPACK_SOURCE_IGNORE_FILES ".*\\\\.svn/" ".*\\\\.git/"
  "^${Dakota_SOURCE_DIR}/packages/fftw/"
  "^${Dakota_SOURCE_DIR}/packages/hopspack/src/src-citizens/citizen-gss/cddlib/"
  "^${Dakota_SOURCE_DIR}/packages/pecos/packages/teuchos/"
  "^${Dakota_SOURCE_DIR}/packages/pecos/packages/fftw/"
  "^${Dakota_SOURCE_DIR}/packages/surfpack/packages/CONMIN/"
  "^${Dakota_SOURCE_DIR}/packages/surfpack/packages/NCSUOpt/"
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

# RPATH settings 
if(APPLE)
  set(CMAKE_MACOSX_RPATH TRUE CACHE BOOL "Add @rpath to library install names")
  set(CMAKE_INSTALL_RPATH "@executable_path;@executable_path/../lib" 
    CACHE STRING "Set the RPATH in Dakota executables and libraries")
elseif(UNIX)
  set(CMAKE_INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../lib")
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

# Locate MPI compiler and library settings (must be after lanugages enabled)
include(DakotaMPI)
option(DAKOTA_HAVE_MPI "Enable MPI in DAKOTA" OFF)
if(DAKOTA_HAVE_MPI)
  DakotaFindMPI()
endif()


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

option(BUILD_SHARED_LIBS "Build shared libraries?" ON)

# Note: May need this in some Cygwin builds as well
if(CMAKE_SYSTEM_NAME MATCHES Darwin)
  set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS
      "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -undefined dynamic_lookup")
endif()

# TODO: Can't this be integrated into the following logic?
if(MSVC)
  find_package(LAPACK REQUIRED NO_MODULE)
  set(BLAS_LIBS blas)
  set(LAPACK_LIBS lapack)
endif()
# first check for a system blas and lapack
if(NOT DEFINED BLAS_LIBS OR NOT DEFINED LAPACK_LIBS)
  if(NOT DEFINED BLAS_LIBS)
    find_library(BLAS_LIBS blas)
  endif()
  if(NOT DEFINED LAPACK_LIBS)
    find_library(LAPACK_LIBS lapack)
  endif()
  if(NOT BLAS_LIBS OR NOT LAPACK_LIBS)
    # if not a system blas and lapack, then look for a cmake built LAPACK
    # with find_package
    find_package(LAPACK REQUIRED NO_MODULE)
    set(BLAS_LIBS blas)
    set(LAPACK_LIBS lapack)
  endif()
endif()


# GSL check at top-level due to need for settings in BOTH packages
# and src subdirectories
option(DAKOTA_HAVE_GSL "Toggle GSL support, default is disabled" OFF)
if(DAKOTA_HAVE_GSL)
  # One may want to provide an alternate CBLAS library via
  # DAKOTA_CBLAS_LIBS to use in place of GSL's CBLAS.  On many
  # platforms, linking the GSL CBLAS induces numerical DIFFs in other
  # parts of Dakota. (Might prefer to probe for a library containing
  # cblas_ symbols, then conditionally set this option automatically.)
  if(DAKOTA_CBLAS_LIBS)
    set(GSL_WITHOUT_CBLAS TRUE CACHE BOOL "Omit GSL CBLAS libraries")
  endif()
  find_package(GSL REQUIRED)
  message(STATUS "Found GSL libraries: ${GSL_LIBRARIES}")
endif()

# HDF5 check at top-level due to need for settings in BOTH src
# and test subdirectories
option(DAKOTA_HAVE_HDF5 "Toggle HDF5 support, default is disabled" OFF)
if(DAKOTA_HAVE_HDF5)
  find_package(HDF5 REQUIRED COMPONENTS "C;HL")

  if(NOT HDF5_FOUND)
    message( SEND_ERROR "Dakota cannot write BINARY ResultsDB without HDF5 -
             ${HDF5_hdf5_FOUND}" )
  endif() # HDF5_FOUND

  #message( "Using pre-installed HDF5 in ${FIND_PACKAGE_MESSAGE_DETAILS_HDF5}" )

  #find_package(ZLIB REQUIRED)
  #message( "Using pre-installed ZLIB in ${ZLIB_LIBRARIES}" )

endif() # DAKOTA_HAVE_HDF5

# Mandate a system or user-provided Boost, including some libraries
set(dakota_boost_components
  "filesystem;program_options;regex;serialization;system")

# Acro requires the signals library in addition
option(HAVE_ACRO "Build the ACRO package." ON)
if(HAVE_ACRO)
  list(APPEND dakota_boost_components "signals")
  # Acro uses Boost.Signals (deprecated) instead of Boost.Signals2
  add_definitions("-DBOOST_SIGNALS_NO_DEPRECATION_WARNING")
endif()

# Queso requires the program_options library in addition
option(HAVE_QUESO "Build the QUESO package." OFF)
option(HAVE_QUESO_GPMSA "Use the QUESO GPMSA package when QUESO enabled." ON)
option(HAVE_QUESO_GRVY "Use optional QUESO GRVY package; not recommended." OFF)
option(DAKOTA_QUESO_CMAKE "Experimental: Dakota building QUESO with CMake" ON)


option(HAVE_APPROXNN "Build the Approximate Nearest Neighbor package." ON)

# Options for adaptive sampling that affect both packages and src
option(HAVE_ADAPTIVE_SAMPLING "Enable Morse-Smale-related sampling" ON)
option(HAVE_MORSE_SMALE 
  "Use Morse-Smale; requires APPROXNN and Dionysus libraries" OFF)
if(HAVE_ADAPTIVE_SAMPLING AND HAVE_MORSE_SMALE)
  if (NOT HAVE_APPROXNN)
    set(HAVE_APPROXNN TRUE CACHE BOOL 
      "HAVE_APPROXNN enabled for Morse-Smale Adaptive Sampling")
  endif()
endif()

# Options that affect both src/ and test/
option(DAKOTA_MODELCENTER "Toggle ModelCenter support, default is disabled" OFF)

if(WIN32)
  set(Boost_USE_STATIC_LIBS TRUE)
endif()
# Dakota requires Boost 1.49 or newer; enforce for all libs in the build
find_package(Boost 1.49 REQUIRED COMPONENTS "${dakota_boost_components}")


include(CTest)


# Installation destination options 
# (relative to ${CMAKE_INSTALL_PREFIX}, unless absolute)
set(DAKOTA_EXAMPLES_INSTALL . CACHE FILEPATH 
  "Installation destination for DAKOTA examples/ dir")
set(DAKOTA_TEST_INSTALL . CACHE FILEPATH 
  "Installation destination for DAKOTA test/ dir")
set(DAKOTA_TOPFILES_INSTALL . CACHE FILEPATH 
  "Installation destination for DAKOTA top-level files")


# Set the export name for install targets; parent packages likely want
# to override this to the name of their target
set(ExportTarget DakotaTargets CACHE STRING 
  "Export set name for ${CMAKE_PROJECT_NAME}")

# TODO: Have etphipp remove Trilinos-specific code and instead set in TriKota
# when building inside Trilinos, the path to Teuchos will already be set
if (NOT BUILD_IN_TRILINOS)

  # first probe for system-installed Trilinos
  # this will respect Trilinos_DIR if already set
  find_package(Trilinos QUIET)

  if ( NOT Trilinos_DIR )

    # if no one has configured a local src Teuchos, do so
    # this will respect Teuchos_DIR if already set
    if( NOT Teuchos_DIR )
      if( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/packages/external/teuchos/cmake/tribits )
        set( Teuchos_DIR
          ${CMAKE_CURRENT_BINARY_DIR}/packages/external/teuchos/packages/teuchos )
        set( Trilinos_ENABLE_Teuchos ON CACHE BOOL
          "ENABLE TriBITS build of teuchos" FORCE )

        # Map key Dakota variables to TriBITS variables
        set( TPL_BLAS_LIBRARIES ${BLAS_LIBS} )
        set( TPL_LAPACK_LIBRARIES ${LAPACK_LIBS} )
	# Dakota doesn't use any Teuchos MPI features; may want to force off
        #set( TPL_ENABLE_MPI ${DAKOTA_HAVE_MPI} )


# This doesn't do as name implies; setting OFF doesn't generate Config.cmake 
# at all; doesn't just control whether installed!  Want Config.cmake in build
#        set(Trilinos_ENABLE_INSTALL_CMAKE_CONFIG_FILES OFF CACHE BOOL
#	  "Dakota is the top-level package; don't write Trilinos config files")

      else()
        # Use "historical" directory hierarchy, before TriBITS
        set( Teuchos_DIR ${CMAKE_CURRENT_BINARY_DIR}/packages/teuchos )
      endif() # tribits source directory EXISTS

      message(STATUS "Dakota setting Teuchos_DIR to ${Teuchos_DIR}" )
      set(DAKOTA_BUILDING_TEUCHOS TRUE CACHE BOOL
        "Dakota is building Teuchos in its build tree" FORCE)
      add_subdirectory(packages/external/teuchos)

    else()
      message(STATUS
	"In ${CMAKE_CURRENT_BINARY_DIR} using Teuchos_DIR: ${Teuchos_DIR}")
    endif() # NOT Teuchos_DIR

    # Additional setting to prevent multiple targets with the same name
    set(Trilinos_TARGETS_IMPORTED 1)

    find_package( Teuchos NO_MODULE REQUIRED )

  else()
    message( "Using system trilinos in ${Trilinos_DIR}" )
  endif() # NOT Trilinos_DIR

endif() # NOT BUILD_IN_TRILINOS


# Manage option for graphics (default ON if possible)
include(DakotaXGraphics)
option(HAVE_X_GRAPHICS "Build the Dakota/Motif Graphics package." ON)
if(HAVE_X_GRAPHICS)
  if(WIN32)
    message(STATUS "HAVE_X_GRAPHICS requested, but not supported on Windows; "
      "disabling.")
    set(HAVE_X_GRAPHICS OFF CACHE BOOL 
      "X graphics not supported on Windows; disabling" FORCE)
  else()
    dakota_x_graphics()
    # TODO: option to force even when deps not found?
    if(DAKOTA_X_DEPS_FOUND)
      message(STATUS "All Dakota X_GRAPHICS dependencies found; 2D graphics "
	"will be enabled as requested.")
    else()
      message(WARNING "HAVE_X_GRAPHICS requested, but X11 dependencies not "
	"found; disabling. (Dakota graphics require X11, Xmu, Xpm, and "
	"Motif/Lesstif, including development headers and libraries, as well as "
	"pthreads.)"
	)
      set(HAVE_X_GRAPHICS OFF CACHE BOOL 
	"X graphics dependencies not found; disabling" FORCE)
    endif()
  endif()  # WIN32
endif()  # HAVE_X_GRAPHICS

# Manage option for f90 (default ON)
option(DAKOTA_F90 "Build the f90, bvls_wrapper." ON)
if(DAKOTA_F90)
  FortranCInterface_HEADER(dak_f90_config.h MACRO_NAMESPACE DAK_F90_)
  install(FILES ${Dakota_BINARY_DIR}/dak_f90_config.h DESTINATION include)
endif(DAKOTA_F90)

option(DAKOTA_ENABLE_TESTS "Enable Dakota-specific tests?" ON)
# Option to turn off key DAKOTA TPL tests, default OFF
# Needs to go before adding the packages subdirectory
option(DAKOTA_ENABLE_TPL_TESTS "Enable DAKOTA TPL tests?" OFF)
if(DAKOTA_ENABLE_TPL_TESTS)
  set(DDACE_ENABLE_TESTS    ON CACHE BOOL "Enable DDACE tests.")
  set(HOPSPACK_ENABLE_TESTS ON CACHE BOOL "Enable HOPSPACK tests.")
  set(LHS_ENABLE_TESTS      ON CACHE BOOL "Enable LHS tests.")
  set(OPTPP_ENABLE_TESTS    ON CACHE BOOL "Enable OPT++ tests.")
  set(PECOS_ENABLE_TESTS    ON CACHE BOOL "Enable PECOS tests.")
else()
  set(DDACE_ENABLE_TESTS    OFF CACHE BOOL "Enable DDACE tests.")
  set(HOPSPACK_ENABLE_TESTS OFF CACHE BOOL "Enable HOPSPACK tests.")
  set(LHS_ENABLE_TESTS      OFF CACHE BOOL "Enable LHS tests.")
  set(OPTPP_ENABLE_TESTS    OFF CACHE BOOL "Enable OPT++ tests.")
  set(PECOS_ENABLE_TESTS    OFF CACHE BOOL "Enable PECOS tests.")
endif()

add_subdirectory(packages)

# The Dakota DLL API should build on all platforms, though the
# ModelCenter portion will not
option(DAKOTA_DLL_API "Enable DAKOTA DLL API." ON)

option(ENABLE_SPEC_MAINT "Enable DAKOTA specification maintenance mode." OFF)
if(ENABLE_SPEC_MAINT AND NOT UNIX)
  message(FATAL_ERROR 
    "DAKOTA specification maintenance mode only available on UNIX platforms")
endif()

option(ENABLE_DAKOTA_DOCS "Enable DAKOTA documentation build." OFF)
if(ENABLE_DAKOTA_DOCS AND NOT UNIX)
  message(FATAL_ERROR 
    "DAKOTA documentation build only available on UNIX platforms")
endif()

option(DAKOTA_API_JAVA "Enable Dakota library Java API" OFF)

if (ENABLE_SPEC_MAINT OR ENABLE_DAKOTA_DOCS OR DAKOTA_API_JAVA)
  # Each of these requires compiling Java code with Java 1.6 or newer 
  find_package(Java 1.6 REQUIRED)
  # BMA: Not sure why Java_FOUND doesn't work here:
  if (NOT Java_JAVA_EXECUTABLE OR NOT Java_JAVAC_EXECUTABLE OR 
      NOT Java_JAR_EXECUTABLE)
    message(SEND_ERROR "Dakota spec maint, docs, and Java API require JDK.")
  endif()
  include(UseJava)
endif()

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
  PATTERN "Makefile.am" EXCLUDE
  PATTERN ".svn" EXCLUDE
  PATTERN ".git" EXCLUDE
  PATTERN "*.c" EXCLUDE 
  PATTERN "*.f" EXCLUDE 
  )

install(DIRECTORY test DESTINATION ${DAKOTA_TEST_INSTALL}
  PATTERN "CMakeLists.txt" EXCLUDE
  PATTERN "Makefile.*" EXCLUDE
  PATTERN ".svn" EXCLUDE
  PATTERN ".git" EXCLUDE
  PATTERN "*.c" EXCLUDE 
  PATTERN "*.cpp" EXCLUDE 
  PATTERN "*.f" EXCLUDE 
  )

# Install helper scripts to bin/:
install(PROGRAMS "scripts/dprepro" DESTINATION "bin")
# dakota.bat is useful in both native and Cygwin due to command prompt usage
if (MSVC OR CYGWIN)
  install(PROGRAMS "scripts/dakota.bat" DESTINATION "bin")
endif()
# dakota.sh is useful for all except native windows builds
if(NOT MSVC)
  install(PROGRAMS "scripts/dakota.sh" DESTINATION "bin")
endif()




# Final CPack settings

# Create Add generated files to source package
if(DAKOTA_SVN_checkout OR DAKOTA_GIT_checkout)
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


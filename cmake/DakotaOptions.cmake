# Key options controlling Dakota TPLs, features, and behavior
#
# BMA TODO: Options dictionary and verbose printing of all options


# -----
# CMake built-in defaults
# -----

option(BUILD_SHARED_LIBS "Build shared libraries?" ON)


# -----
# Dakota core
# -----

option(DAKOTA_HAVE_MPI "Enable MPI in DAKOTA" OFF)
option(DAKOTA_F90 "Build Dakota's Fortran90 bvls_wrapper." ON)

# Testing options
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

# The Dakota DLL API should build on all platforms, though the
# ModelCenter portion will not
option(DAKOTA_DLL_API "Enable DAKOTA DLL API." ON)
# Options that affect both src/ and test/
option(DAKOTA_MODELCENTER "Toggle ModelCenter support, default is disabled" OFF)

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

# Option to build an unsupported Java wrapper for Dakota's library mode that 
# has a Java callback for performing function evaluations.  This is a non-
# functioning capability that should only be enabled by developers.  It does 
# not relate to compiling or using Dakota's graphical user interface.
option(DAKOTA_API_JAVA "Unsupported: Enable Dakota library Java API" OFF)
mark_as_advanced(DAKOTA_API_JAVA)


# -----
# System TPLs
# -----

option(DAKOTA_HAVE_GSL "Toggle GSL support, default is disabled" OFF)
option(DAKOTA_HAVE_HDF5 "Toggle HDF5 support, default is disabled" OFF)
option(HAVE_X_GRAPHICS "Build the Dakota/Motif Graphics package." OFF)


# -----
# Dakota packages/ TPLs
# -----

# BMA NOTE: NonDBayes and MorseSmale unconditionally depend on ANN
option(HAVE_APPROXNN "Build the Approximate Nearest Neighbor package." ON)

# Adaptive Sampling: Affect both packages and src
option(HAVE_ADAPTIVE_SAMPLING "Enable Morse-Smale-related sampling" ON)
option(HAVE_MORSE_SMALE 
  "Use Morse-Smale; requires APPROXNN and Dionysus libraries" OFF)
# BMA TODO: Remove this once we remove AdaptiveSampling
if(HAVE_ADAPTIVE_SAMPLING AND HAVE_MORSE_SMALE)
  if (NOT HAVE_APPROXNN)
    set(HAVE_APPROXNN TRUE CACHE BOOL 
      "HAVE_APPROXNN enabled for Morse-Smale Adaptive Sampling")
  endif()
endif()

# QUESO: Affect both packages and src
option(HAVE_QUESO "Build the QUESO package." OFF)
option(HAVE_QUESO_GPMSA "Use the QUESO GPMSA package when QUESO enabled." ON)
option(HAVE_QUESO_GRVY "Use optional QUESO GRVY package; not recommended." OFF)
# BMA TODO: Remove this option as no longer conditional
option(DAKOTA_QUESO_CMAKE "Experimental: Dakota building QUESO with CMake" ON)

option(HAVE_ROL "Build the Trilinos/ROL package." ON)


# -----
# Dakota Installation
# -----

option(DAKOTA_INSTALL_DYNAMIC_DEPS
  "Install Dakota's dynamic dependencies to CMAKE_INSTALL_PREFIX/bin/" ON)

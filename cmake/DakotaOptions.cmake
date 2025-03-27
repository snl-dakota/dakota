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

# BMA NOTE: May not want an option as this is required
option(DAKOTA_MODULE_UTIL "Enable Dakota module util" ON)
option(DAKOTA_MODULE_SURROGATES "Enable Dakota module surrogates" ON)
# BMA NOTE: This isn't a great module name, but seems more intentional
# that "rest of Dakota"
option(DAKOTA_MODULE_DAKOTA "Enable Dakota module for traditional all Dakota"
  ON)

if(BUILD_IN_TRILINOS)
  set(DAKOTA_MODULE_SURROGATES OFF CACHE BOOL
    "Dakota surrogates module is disabled for in-Trilinos builds" FORCE)
endif()

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


option(DAKOTA_PLUGIN_DEMOS "Build Dakota plugin demos in src/plugins." OFF)


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


## Python options

# NOTES/RATIONALE:
#  * Dakota build should only require a Python interpreter by default
#  * We do not default-enable components that have library
#    dependencies due to distribution challenges. These include:
#     - Python libs: DAKOTA_PYTHON_DIRECT_INTERFACE, DAKOTA_PYTHON_SURROGATES
#     - Python numpy: DAKOTA_PYTHON_DIRECT_INTERFACE_NUMPY
#  * When h5py is present, unit tests using it will automatically be enabled

# Scripts and tests based on interpreter
# (Formerly, DAKOTA_PYTHON controlled linked/direct Python interface only)
option(DAKOTA_PYTHON "Dakota Python scripts (Interpreter); default ON" ON)

# Direct interface defaults OFF to avoid Python library dependencies
option(DAKOTA_PYTHON_DIRECT_INTERFACE
  "Dakota Python direct interface (Development); default OFF" OFF)
# Direct interface numpy is a dependent option; will only be enabled
# if the direct interface is. (Formerly called DAKOTA_PYTHON_NUMPY)
option(DAKOTA_PYTHON_DIRECT_INTERFACE_NUMPY
  "Dakota Python direct interface uses NumPy (only has effect when DAKOTA_PYTHON_DIRECT_INTERFACE is ON)" 
  ON
  )

# Top-level dakota.environment Python wrapper defaults off to avoid
# Python library deps
option(DAKOTA_PYTHON_WRAPPER
  "Top-level Dakota Python interface (dakota.environment wrapper)" OFF)

if(DAKOTA_PYTHON_DIRECT_INTERFACE OR DAKOTA_PYTHON_WRAPPER)
  set(DAKOTA_PYBIND11 ON CACHE BOOL
    "Dakota using Pybind11 for interfaces.")
endif()

# Requires Pybind11; default OFF to avoid default library dependencies
option(DAKOTA_PYTHON_SURROGATES
  "Dakota Python interface to surrogates module; default OFF" OFF)

if(DAKOTA_PYTHON_SURROGATES)
  if(DAKOTA_MODULE_SURROGATES)
    set(DAKOTA_PYBIND11 ON CACHE BOOL
      "Dakota using Pybind11 for surrogate wrappers.")
  else()
    message(STATUS "Disabling DAKOTA_PYTHON_SURROGATES as "
      "DAKOTA_MODULE_SURROGATES=${DAKOTA_MODULE_SURROGATES}")
    set(DAKOTA_PYTHON_SURROGATES OFF CACHE BOOL
      "Python surrogate wrappers disabled as surrogate module is off." FORCE)
  endif()
endif()

option(DAKOTA_JAVA_SURROGATES
  "Dakota Java interface to surrogate module; default OFF" OFF)

if(DAKOTA_JAVA_SURROGATES)
  if(NOT DAKOTA_MODULE_SURROGATES)
    message(STATUS "Disabling DAKOTA_JAVA_SURROGATES as "
      "DAKOTA_MODULE_SURROGATES=${DAKOTA_MODULE_SURROGATES}")
    set(DAKOTA_JAVA_SURROGATES OFF CACHE BOOL
      "Java surrogate wrappers disabled as surrogate module is off.")
  endif()
endif()

# Option to build an unsupported Java wrapper for Dakota's library mode that 
# has a Java callback for performing function evaluations.  This is a non-
# functioning capability that should only be enabled by developers.  It does 
# not relate to compiling or using Dakota's graphical user interface.
option(DAKOTA_API_JAVA "Unsupported: Enable Dakota library Java API" OFF)
mark_as_advanced(DAKOTA_API_JAVA)


option(ENABLE_DAKOTA_DOCS "Enable Dakota documentation build." OFF)
option(DAKOTA_DOCS_LEGACY "Enable pre-2022 Dakota docs build." OFF)
# TODO: Can probably relax this for Sphinx and Doxygen docs...
if(ENABLE_DAKOTA_DOCS AND NOT UNIX)
  message(FATAL_ERROR
    "Dakota documentation build only available on UNIX platforms")
endif()
if(ENABLE_DAKOTA_DOCS AND NOT DAKOTA_PYTHON)
  message(FATAL_ERROR
    "Dakota documentation build only available with DAKOTA_PYTHON=ON")
endif()

option(DAKOTA_GCOV "GNU gcov for Dakota core" OFF)

option(DAKOTA_DISABLE_FPE_TRAPS
  "Globally disable traps that raise SIGFPE; use with caution" OFF)


# -----
# System TPLs
# -----

option(DAKOTA_HAVE_GSL "Toggle GSL support, default is disabled" OFF)
option(DAKOTA_HAVE_HDF5 "Toggle HDF5 support, default is disabled" OFF)
option(HAVE_X_GRAPHICS "Build the Dakota/Motif Graphics package." OFF)


# -----
# Dakota packages/ TPLs
# -----

option(HAVE_ACRO "Build the ACRO package." ON)

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

option(HAVE_AMPL "Build the AMPL package." ON)
option(HAVE_C3 "Build the Compressed Continuous Computation package." OFF)
option(HAVE_CONMIN "Build the CONMIN package." ON)
option(HAVE_DDACE "Build the DDACE package." ON)
option(HAVE_DEMO_TPL "Build the Demo_TPL package." OFF)
option(HAVE_DOT "Build the DOT package." ON)
option(HAVE_DREAM "Build the DREAM package." ON)

# Formerly sub-packages of Pecos
option(HAVE_FFT "Use either the fftw or dfftpack packages" ON)
option(HAVE_DFFTPACK "Build the dfftpack package.  This OPTION has no effect if
  HAVE_FFT is OFF" ON)
option(HAVE_FFTW "Find and use an installed fftw package. This OPTION has no
  effect if HAVE_FFT is OFF" OFF)

option(HAVE_FSUDACE "Build the FSUDACE package." ON)
option(HAVE_HOPSPACK "Build the hopspack package." ON)
option(HAVE_JEGA "Build the JEGA package." ON)

# Formerly sub-package of Pecos
option(HAVE_LHS "Build the LHS package." ON)

option(HAVE_NCSUOPT "Build the NCSUOPT package." ON)
option(HAVE_NL2SOL "Build the NL2SOL package." ON)
option(HAVE_NLPQL "Build the NLPQL package." ON)
option(HAVE_NOMAD "Build the NOMAD package." ON)
option(HAVE_MUQ "Build the MIT MUQ package (experimental)." OFF)
option(HAVE_NOWPAC "Build the SNOWPAC package (experimental)." OFF)
option(HAVE_NPSOL "Build the NPSOL package." ON)
option(HAVE_OPTPP "Build the OPTPP package." ON)
option(HAVE_PECOS "Build the Pecos package." ON)
option(HAVE_PSUADE "Build the PSUADE package." ON)

# QUESO: Affect both packages and src
option(HAVE_QUESO "Build the QUESO package." OFF)
option(HAVE_QUESO_GPMSA "Use the QUESO GPMSA package when QUESO enabled." ON)
option(HAVE_QUESO_GRVY "Use optional QUESO GRVY package; not recommended." OFF)
# BMA TODO: Remove this option as no longer conditional
option(DAKOTA_QUESO_CMAKE "Experimental: Dakota building QUESO with CMake" ON)

option(HAVE_ROL "Build the Trilinos/ROL package." ON)

# Formerly sub-package of Pecos
option(HAVE_SPARSE_GRID "Build the VPISparseGrid package." ON)

option(HAVE_SURFPACK "Build the Surfpack package." ON)

# -----
# Dakota Installation
# -----

option(DAKOTA_INSTALL_DYNAMIC_DEPS
  "Install Dakota's dynamic dependencies to CMAKE_INSTALL_PREFIX/bin/" ON)

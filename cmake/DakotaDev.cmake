# CMake options for DAKOTA developer builds

# Developer options
set(DAKOTA_HAVE_MPI TRUE CACHE BOOL "Enable MPI in DAKOTA?")
set(ENABLE_DAKOTA_DOCS TRUE CACHE BOOL "Enable DAKOTA documentation build")
set(ENABLE_SPEC_MAINT TRUE CACHE BOOL 
  "Enable DAKOTA specification maintenance mode?")
set(PECOS_ENABLE_TESTS TRUE CACHE BOOL "Enable Pecos-specific tests?")

set(DAKOTA_TEST_EXAMPLES_REPO TRUE CACHE BOOL
  "Enable Dakota Examples Library tests")

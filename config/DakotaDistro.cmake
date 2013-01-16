
# CMake options for DAKOTA testing builds

# This no longer works as Trilinos CMake rejects custom CMAKE_BUILD_TYPE
# Use distribution build options (-O2)
#set(CMAKE_BUILD_TYPE "DISTRIBUTION" CACHE STRING "Type of build")

# CMake build flags to match historical baselines and releases
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Dakota Dev Release")
set(CMAKE_C_FLAGS_RELEASE "-O2" CACHE STRING "Dakota Dev Release Flags")
set(CMAKE_CXX_FLAGS_RELEASE "-O2" CACHE STRING "Dakota Dev Release Flags")
set(CMAKE_Fortran_FLAGS_RELEASE "-O2" CACHE STRING "Dakota Dev Release Flags")

# New Trilinos seems to want to override those above.  Try to force.
set(CMAKE_C_FLAGS_RELEASE_OVERRIDE "-O2" CACHE STRING "Dakota Dev Release Flags")
set(CMAKE_CXX_FLAGS_RELEASE_OVERRIDE "-O2" CACHE STRING "Dakota Dev Release Flags")
set(CMAKE_Fortran_FLAGS_RELEASE_OVERRIDE "-O2" CACHE STRING "Dakota Dev Release Flags")

# Use shared libraries and don't use static
set(BUILD_STATIC_LIBS FALSE CACHE BOOL "Build static libs?")
set(BUILD_SHARED_LIBS TRUE  CACHE BOOL "Build shared libs?")

# No bounds checking in Teuchos
set(Teuchos_ENABLE_ABC FALSE CACHE BOOL "Enable bounds checking?")

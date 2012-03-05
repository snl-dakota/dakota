
# CMake options for DAKOTA testing builds

# Use distribution build options (-O2)
set(CMAKE_BUILD_TYPE "DISTRIBUTION" CACHE STRING "Type of build")

# Use shared libraries and don't use static
set(BUILD_STATIC_LIBS FALSE CACHE BOOL "Build static libs?")
set(BUILD_SHARED_LIBS TRUE  CACHE BOOL "Build shared libs?")

# No bounds checking in Teuchos
set(Teuchos_ENABLE_ABC FALSE CACHE BOOL "Enable bounds checking?")

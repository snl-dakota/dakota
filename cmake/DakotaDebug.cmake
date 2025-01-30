# CMake options for DAKOTA debug builds

# Use debug build options (-g)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Type of build" FORCE)

# Use static libraries and don't use shared
if(DevDebugShared)
  set(BUILD_SHARED_LIBS TRUE CACHE BOOL "Build shared libs?" FORCE)
else()
  # DevDebugStatic, DevDebug
  set(BUILD_SHARED_LIBS FALSE CACHE BOOL "Build shared libs?" FORCE)
endif()

# Force bounds checking in Teuchos
set(Teuchos_ENABLE_ABC FALSE CACHE BOOL "Enable bounds checking?")

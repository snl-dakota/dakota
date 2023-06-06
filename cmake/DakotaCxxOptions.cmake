# Specify Dakota's C++ language requirements
#
# TODO: Would prefer to check for specific C++ features needed and let
# CMake deduce the compiler requirements
macro(dakota_cxx_standard)

  # Require C++11 at a minimum, but allow newer standards
  # Do this prior to other flag settings and enabling the C++ language
  if (NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD EQUAL 98)
    set(CMAKE_CXX_STANDARD 17 CACHE STRING
      "Dakota strictly requires C++17 or better")
  endif()
  if (NOT DEFINED CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS FALSE CACHE BOOL
      "Dakota prefers not to use compiler-specific C++ extensions")
  endif()
  set(CMAKE_CXX_STANDARD_REQUIRED TRUE CACHE BOOL
    "Dakota strictly requires C++11 or better")

  # clumsy setting of flag to prevent issues in boost::serialization for c++17.
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-fno-new-ttp-matching ${CMAKE_CXX_FLAGS}" CACHE STRING "fix boost::serialization" FORCE)
  endif()

endmacro()


# When compiling with GCC or Clang, enable stricter warnings on a per-target basis
# Intel supports GCC -W options, but doesn't document which, so not enabling for now
macro(dakota_strict_warnings target_name)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
     CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${target_name} PRIVATE "-Wall" "-Wextra" "-Wpedantic")
  endif()
endmacro()

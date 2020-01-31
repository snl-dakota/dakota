# Specify Dakota's C++ language requirements
#
# TODO: Would prefer to check for specific C++ features needed and let
# CMake deduce the compiler requirements
macro(dakota_cxx_standard)

  # Require C++11 at a minimum, but allow newer standards
  # Do this prior to other flag settings and enabling the C++ language
  if (NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD EQUAL 98)
    set(CMAKE_CXX_STANDARD 11 CACHE STRING
      "Dakota strictly requires C++11 or better")
  endif()
  if (NOT DEFINED CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS FALSE CACHE BOOL
      "Dakota prefers not to use compiler-specific C++ extensions")
  endif()
  set(CMAKE_CXX_STANDARD_REQUIRED TRUE CACHE BOOL
    "Dakota strictly requires C++11 or better")

endmacro()


# Warn if we know the CMake version in use doesn't know how to add
# compiler flags for C++11
macro(dakota_check_cxx11)

  # Based on CMake versions suppporting language feature, this is our
  # best guess at support...
  # CMake 3.1           Clang, GNU
  # CMake 3.2           AppleClang, Clang, GNU, SunPro
  # CMake 3.3 -- 3.5    AppleClang, Clang, GNU, MSVC, SunPro
  # CMake 3.6 -- 3.10   AppleClang, Clang, GNU, Intel, MSVC, SunPro

  # Compilers for which compiler features are supported by CMake 3.1
  set(known_cxx11_compilers Clang GNU)
  # CMake 3.1 doesn't have VERSION_GREATER_EQUAL, so we do NOT VERSION_LESS
  if(NOT CMAKE_VERSION VERSION_LESS 3.2)
    # Additional compilers supported by 3.2
    list(APPEND known_cxx11_compilers AppleClang SunPro)
  endif()
  if(NOT CMAKE_VERSION VERSION_LESS 3.3)
    # Additional compilers supported by 3.3
    list(APPEND known_cxx11_compilers MSVC)
  endif()
  if(NOT CMAKE_VERSION VERSION_LESS 3.6)
    # Additional compilers supported by 3.6
    list(APPEND known_cxx11_compilers Intel)
  endif()

  list(FIND known_cxx11_compilers ${CMAKE_CXX_COMPILER_ID} cxx11_compiler_found)
  if(cxx11_compiler_found EQUAL -1)
    message(WARNING "CMake version ${CMAKE_VERSION} may not set "
      "${CMAKE_CXX_COMPILER_ID} compiler flags for C++11.  It may be necessary "
      "to add appropriate CMAKE_CXX_FLAGS, such as -std=c++11.")
  endif()

endmacro()

include(ExternalProject)

# Forward Dakota Boost entries to the plugins (for Python plugins)
set(boost_cache_entries "")
foreach(boost_var BOOST_ROOT BOOSTROOT BOOST_INCLUDEDIR BOOST_LIBRARYDIR
  Boost_INCLUDE_DIR Boost_LIBRARY_DIR Boost_NO_SYSTEM_PATHS Boost_NO_BOOST_CMAKE
  Boost_VERSION)
  if (DEFINED ${boost_var})
    list(APPEND boost_cache_entries "-D${boost_var}:STRING=${${boost_var}}")
  endif()
endforeach()

message(STATUS "Building plugin demos in src/plugins with Boost settings:
   ${boost_cache_entries}")

# This doesn't work as download deletes the files
#file(MAKE_DIRECTORY "${Dakota_BINARY_DIR}/src/plugins/src/DakotaPluginDemos")
#file(CREATE_LINK "${Dakota_SOURCE_DIR}/packages/external/pybind11"
# "${Dakota_BINARY_DIR}/src/plugins/src/DakotaPluginDemos/pybind11" SYMBOLIC)

file(CREATE_LINK "${Dakota_SOURCE_DIR}/packages/external/pybind11"
 "${Dakota_SOURCE_DIR}/src/plugins/pybind11" SYMBOLIC)

ExternalProject_Add(DakotaPluginDemos
  PREFIX "${Dakota_BINARY_DIR}/src/plugins"
  URL "${Dakota_SOURCE_DIR}/src/plugins"
  BINARY_DIR "${Dakota_BINARY_DIR}/src/plugins/build"
  INSTALL_DIR "${Dakota_BINARY_DIR}/src/plugins/install"
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  CMAKE_CACHE_ARGS "${boost_cache_entries}"
  )

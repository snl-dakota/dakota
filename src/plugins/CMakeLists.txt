#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2025
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

cmake_minimum_required(VERSION 3.23.0)
project(DakotaPlugins CXX)

find_package(Python COMPONENTS Interpreter Development)
add_subdirectory(pybind11)

add_library(generic_python_plugin SHARED DakotaPythonPlugin.cpp)
set_target_properties(generic_python_plugin PROPERTIES CXX_STANDARD 17)
set_target_properties(generic_python_plugin PROPERTIES CXX_VISIBILITY_PRESET hidden)

if(Boost_VERSION VERSION_LESS 1.70.0)
    #message(STATUS "Boost version is older than 1.70.0")
    # Python plugins require Boost DLL headers (to load libpython.so)
    option(DAKOTA_PLUGINS_USE_BOOST "Whether to use Boost to build plugins" ON)
    if(DAKOTA_PLUGINS_USE_BOOST)
      find_package(Boost 1.69 REQUIRED COMPONENTS filesystem)
    endif()

    target_link_libraries(generic_python_plugin
      PUBLIC Boost::boost Boost::filesystem
      PRIVATE pybind11::embed Python::Python
    )
else()
    #message(STATUS "Boost version is 1.70.0 or newer")
    find_package(Boost 1.70 REQUIRED COMPONENTS system)
    # handle special case for gcc filesystem support
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0) OR DAKOTA_LINK_STDCPPFS)
      target_link_libraries(generic_python_plugin
        PUBLIC Boost::system stdc++fs
        PRIVATE pybind11::embed Python::Python
      )
    else()
        target_link_libraries(generic_python_plugin
          PUBLIC Boost::system
          PRIVATE pybind11::embed Python::Python
        )
    endif()
endif()


#target_link_options(plugin_python PUBLIC -rdynamic)
#-Wl,--no-gc-sections -Wl,--export-dynamic)

# Workaround to be able to import Python modules in user space after the Python
# plugin is runtime loaded.
get_target_property(_python_lib Python::Python LOCATION)
target_compile_definitions(generic_python_plugin PRIVATE
  PLUGIN_PYTHON_LIB=\"${_python_lib}\")

# old stuff (non-python)
# TODO: function(add_plugin PLUGIN_NAME SOURCES) spanning the following
add_library(identity_map SHARED PluginIdentityMap.cpp)

set_target_properties(identity_map PROPERTIES
                      CXX_STANDARD 17
                      CXX_STANDARD_REQUIRED TRUE
                      CXX_VISIBILITY_PRESET hidden)

if(Boost_VERSION VERSION_LESS 1.70.0)
    if(DAKOTA_PLUGINS_USE_BOOST)
      target_compile_definitions(identity_map PRIVATE DAKOTA_PLUGINS_USE_BOOST=1)
      target_link_libraries(identity_map Boost::boost)
    endif()
endif()
# Only install the plugins Dakota will rely on at runtime
install(TARGETS generic_python_plugin DESTINATION lib)

#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2020
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

cmake_minimum_required(VERSION 3.12)
project(DakotaPlugins CXX)

set(BOOST_ROOT "/projects/sems/install/rhel7-x86_64/sems/tpl/boost/1.69.0/gcc/4.8.5/base")

find_package(Boost 1.69 REQUIRED)

# conflict here? - this is from C++ only version
option(DAKOTA_PLUGINS_USE_BOOST "Whether to use Boost to build plugins" OFF)
if(DAKOTA_PLUGINS_USE_BOOST)
  find_package(Boost REQUIRED)
endif()

find_package(Python COMPONENTS Interpreter Development)
add_subdirectory(pybind11)

add_library(generic_python_plugin SHARED DakotaPythonPlugin.cpp)
set_target_properties(generic_python_plugin PROPERTIES CXX_STANDARD 11)
set_target_properties(generic_python_plugin PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_link_libraries(generic_python_plugin
  PUBLIC Boost::boost
  PRIVATE pybind11::embed Python::Python
)

#add_library(textbook_python_plugin SHARED TextbookPythonPlugin.cpp)
#set_target_properties(textbook_python_plugin PROPERTIES CXX_STANDARD 11)
#set_target_properties(textbook_python_plugin PROPERTIES CXX_VISIBILITY_PRESET hidden)
#target_link_libraries(textbook_python_plugin
#  PUBLIC Boost::boost
#  PRIVATE pybind11::embed Python::Python
#)

#add_library(alt_textbook_python_plugin SHARED AltTextbookPythonPlugin.cpp)
#set_target_properties(alt_textbook_python_plugin PROPERTIES CXX_STANDARD 11)
#set_target_properties(alt_textbook_python_plugin PROPERTIES CXX_VISIBILITY_PRESET hidden)
#target_link_libraries(alt_textbook_python_plugin
#  PUBLIC Boost::boost
#  PRIVATE pybind11::embed Python::Python
#)

#target_link_options(plugin_python PUBLIC -rdynamic)
#-Wl,--no-gc-sections -Wl,--export-dynamic)

# Workaround to be able to import Python modules in user space after the Python
# plugin is runtime loaded.
get_target_property(_python_lib Python::Python LOCATION)
target_compile_definitions(generic_python_plugin PRIVATE
  PLUGIN_PYTHON_LIB=\"${_python_lib}\")
#target_compile_definitions(textbook_python_plugin PRIVATE
#  PLUGIN_PYTHON_LIB=\"${_python_lib}\")
#target_compile_definitions(alt_textbook_python_plugin PRIVATE
#  PLUGIN_PYTHON_LIB=\"${_python_lib}\")

# old stuff (non-python)
# TODO: function(add_plugin PLUGIN_NAME SOURCES) spanning the following
add_library(identity_map SHARED PluginIdentityMap.cpp)

set_target_properties(identity_map PROPERTIES
                      CXX_STANDARD 11
                      CXX_STANDARD_REQUIRED TRUE
                      CXX_VISIBILITY_PRESET hidden)

if(DAKOTA_PLUGINS_USE_BOOST)
  target_compile_definitions(identity_map PRIVATE DAKOTA_PLUGINS_USE_BOOST=1)
  target_link_libraries(identity_map Boost::boost)
endif()


# TODO: install whole plugin source dir to share/dakota/ for end users
#install(...)

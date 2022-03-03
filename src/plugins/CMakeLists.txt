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


option(DAKOTA_PLUGINS_USE_BOOST "Whether to use Boost to build plugins" OFF)
if(DAKOTA_PLUGINS_USE_BOOST)
  find_package(Boost REQUIRED)
endif()


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

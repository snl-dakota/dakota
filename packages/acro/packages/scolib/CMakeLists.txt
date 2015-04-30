cmake_minimum_required(VERSION 2.6)
project(scolib)
set(ExportTarget ${CMAKE_PROJECT_NAME}-targets CACHE STRING
  "Name for the export target for ${CMAKE_PROJECT_NAME}")

if(NOT TINYXML_DIR)
  message(ERROR " Cannot build colin without tinyxml.  Please set TINYXML_DIR")
endif()
include_directories(${TINYXML_DIR})
add_definitions(-DUTILIB_HAVE_TINYXML)

if(NOT Boost_FOUND)
  message(ERROR " Cannot build colin without Boost.  Please enable USE_BOOST and set BOOST_INCLUDE_DIR")
endif()

if(USE_MPI)
  include_directories(${MPI_INCLUDE_PATH})
endif()

add_subdirectory(src)
#enable_testing()
#add_subdirectory(test)

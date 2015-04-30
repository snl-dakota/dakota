cmake_minimum_required(VERSION 2.6)
project(colin)
set(ExportTarget ${CMAKE_PROJECT_NAME}-targets CACHE STRING
  "Name for the export target for ${CMAKE_PROJECT_NAME}")
  
if(NOT TINYXML_DIR)
  message(FATAL_ERROR " Cannot build colin without tinyxml.  Please set TINYXML_DIR")
endif()

if(NOT Boost_FOUND)
  message(FATAL_ERROR " Cannot build colin without Boost.  Please enable USE_BOOST and set Boost_INCLUDE_DIR")
endif()

include_directories(
  ${Acro-Packages_SOURCE_DIR}/utilib/src
  ${Acro-Packages_BINARY_DIR}/utilib
  ${AMPL_INCLUDE_DIRS}
  ${colin_SOURCE_DIR}/src
  ${Boost_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${TINYXML_DIR}
)

if(USE_MPI)
  include_directories(${MPI_INCLUDE_PATH})
endif()

add_subdirectory(src)
add_subdirectory(test/problems)
add_subdirectory(test/unit)
#add_subdirectory(doc)

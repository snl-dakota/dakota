cmake_minimum_required(VERSION 2.6)
project(interfaces)
set(ExportTarget ${CMAKE_PROJECT_NAME}-targets CACHE STRING
  "Name for the export target for ${CMAKE_PROJECT_NAME}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src
                    ${Acro-Packages_SOURCE_DIR}/colin/src
                    ${TINYXML_DIR})
if(MPI_FOUND)
  include_directories(${MPI_INCLUDE_PATH})
endif()

add_subdirectory(src)
#add_subdirectory(test)
#add_subdirectory(doc)

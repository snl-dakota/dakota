cmake_minimum_required(VERSION 2.6)
project(pebbl)
set(ExportTarget ${CMAKE_PROJECT_NAME}-targets CACHE STRING
  "Name for the export target for ${CMAKE_PROJECT_NAME}")

add_subdirectory(src)
#add_subdirectory(doc)
#enable_testing()
#add_subdirectory(test)


cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)

include(AddApplicationTest)

include(CTest)

# Add the tests to be run in unique directories.
add_application_test(
  run_app_file1
  APPLICATION COMMAND "\"${CMAKE_COMMAND}\"" -P file1.in
  FILE_DEPENDENCIES common.in file1.in
  UNIQUE_DIRECTORY)

add_application_test(
  run_app_file2
  APPLICATION COMMAND "\"${CMAKE_COMMAND}\"" -P file2.in
  FILE_DEPENDENCIES common.in file2.in
  UNIQUE_DIRECTORY)

add_application_test(
  run_app_file3
  APPLICATION COMMAND "\"${CMAKE_COMMAND}\"" -P file3.in
  FILE_DEPENDENCIES common.in file3.in
  UNIQUE_DIRECTORY)

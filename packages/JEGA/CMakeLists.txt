cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project(JEGA)

# Set the export name for install targets; parent packages likely want
# to override this to the name of their target
set(ExportTarget ${CMAKE_PROJECT_NAME}-targets CACHE STRING 
  "Name for the export target for ${CMAKE_PROJECT_NAME}")

#inspect the system so that we can generate jega_config.h and eddy_config.h
include(CheckTypeSize)
CHECK_TYPE_SIZE("int8_t" JEGA_SIZEOF_INT8_T)
set(JEGA_HAVE_INT8_T ${HAVE_JEGA_SIZEOF_INT8_T})
CHECK_TYPE_SIZE("int16_t" JEGA_SIZEOF_INT16_T)
set(JEGA_HAVE_INT16_T ${HAVE_JEGA_SIZEOF_INT16_T})
CHECK_TYPE_SIZE("int32_t" JEGA_SIZEOF_INT32_T)
set(JEGA_HAVE_INT32_T ${HAVE_JEGA_SIZEOF_INT32_T})
CHECK_TYPE_SIZE("int64_t" JEGA_SIZEOF_INT64_T)
set(JEGA_HAVE_INT64_T ${HAVE_JEGA_SIZEOF_INT64_T})
CHECK_TYPE_SIZE("uint8_t" JEGA_SIZEOF_UINT8_T)
set(JEGA_HAVE_UINT8_T ${HAVE_JEGA_SIZEOF_UINT8_T})
CHECK_TYPE_SIZE("uint16_t" JEGA_SIZEOF_UINT16_T)
set(JEGA_HAVE_UINT16_T ${HAVE_JEGA_SIZEOF_UINT16_T})
CHECK_TYPE_SIZE("uint32_t" JEGA_SIZEOF_UINT32_T)
set(JEGA_HAVE_UINT32_T ${HAVE_JEGA_SIZEOF_UINT32_T})
CHECK_TYPE_SIZE("uint64_t" JEGA_SIZEOF_UINT64_T)
set(JEGA_HAVE_UINT64_T ${HAVE_JEGA_SIZEOF_UINT64_T})

try_compile(JEGA__HAVE_TEMPLATE_KEYWORD_QUALIFIER
  ${JEGA_BINARY_DIR}/template_keyword_qualifier
  ${JEGA_SOURCE_DIR}/template_keyword_qualifier.cxx
)

option(JEGA_USE_BOOST "Should JEGA be built using Boost?" OFF)
  if(JEGA_USE_BOOST)
  find_package(Boost)
  if(Boost_FOUND)
    set(JEGA_HAVE_BOOST 1)
    include_directories(${Boost_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
  endif()
endif()

#configure those two files
configure_file(${JEGA_SOURCE_DIR}/jega_config.h.in
               ${JEGA_BINARY_DIR}/jega_config.h
)

#don't rewrite eddy_config.h unless something changed
configure_file(${JEGA_SOURCE_DIR}/eddy_config.h.in
               ${JEGA_BINARY_DIR}/eddy_config.h.0
)

file(READ ${JEGA_BINARY_DIR}/eddy_config.h.0 config_contents_0)
string(REPLACE "JEGA" "EDDY" config_contents_0 "${config_contents_0}")
string(REPLACE "jega" "eddy" config_contents_0 "${config_contents_0}")

if(NOT EXISTS ${JEGA_BINARY_DIR}/eddy_config.h)
  file(WRITE ${JEGA_BINARY_DIR}/eddy_config.h "${config_contents_0}")
else()
  file(READ ${JEGA_BINARY_DIR}/eddy_config.h config_contents)
  if(NOT config_contents STREQUAL config_contents_0)
    file(WRITE ${JEGA_BINARY_DIR}/eddy_config.h "${config_contents_0}")
  endif()
endif()

include_directories(
  ${JEGA_SOURCE_DIR} ${JEGA_BINARY_DIR} ${JEGA_SOURCE_DIR}/eddy
  ${JEGA_SOURCE_DIR}/include
)

add_definitions(-DHAVE_CONFIG_H)
add_subdirectory(MOGA)
add_subdirectory(SOGA)
add_subdirectory(Utilities)
add_subdirectory(FrontEnd)
add_subdirectory(eddy)
add_subdirectory(src)

file(GLOB_RECURSE all_jega_headers *.hpp *.hpp.inl)
install(FILES ${all_jega_headers} DESTINATION include)

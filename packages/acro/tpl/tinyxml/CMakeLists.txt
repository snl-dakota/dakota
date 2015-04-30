project(tinyxml)
set(ExportTarget ${CMAKE_PROJECT_NAME}-targets CACHE STRING
  "Name for the export target for ${CMAKE_PROJECT_NAME}")

add_library(tinyxml
 tinyxml/tinyxml.cpp
 tinyxml/tinyxmlerror.cpp
 tinyxml/tinyxmlparser.cpp
)
# While target_include_directories was added in 2.8.11, it is buggy (and
#   broke the DAKOTA build).  We will require at least 2.8.12.
if( ${CMAKE_VERSION} VERSION_GREATER 2.8.12 )
  target_include_directories(tinyxml INTERFACE 
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
      $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include> )
endif()

install(TARGETS tinyxml EXPORT ${ExportTarget} DESTINATION lib)
install(FILES tinyxml/tinyxml.h tinyxml/tinystr.h DESTINATION include)

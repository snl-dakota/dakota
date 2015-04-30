add_definitions(-DHAVE_CONFIG_H)
include_directories(
  ${Acro_SOURCE_DIR}/tpl/appspack/src
  ${Acro_SOURCE_DIR}/tpl/3po
  ${Teuchos_INCLUDE_DIRS}
  ${OPTPP_INCLUDE_DIRS}
  ${Acro_BINARY_DIR}/tpl/appspack
  ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
  ${MOMHLIB_INCLUDE_DIRS}
)

set(interfaces_srcs 
  libs/APPS.cpp
  libs/Cobyla.cpp
  libs/OPTpp.cpp
  libs/InterfacesRegistrations.cpp
)

if(HAVE_MOMHLIB)
  list(APPEND interfaces_srcs libs/MOMHLib.cpp)
endif()

if(HAVE_3PO)
  list(APPEND interfaces_srcs libs/Hooke.cpp)
endif()

add_library(interfaces ${interfaces_srcs})

if(HAVE_3PO)
  target_link_libraries(interfaces 3po) 
endif()
if(HAVE_APPSPACK)
  target_link_libraries(interfaces appspack)
endif()

# Only link if there's an Acro-local OPTPP
if(ACRO_HAVE_OPTPP)
  target_link_libraries(interfaces optpp teuchos)
endif()

install(TARGETS interfaces EXPORT ${ExportTarget} DESTINATION lib)

if(HAVE_BOOST)
  if(HAVE_AMPL)
    include_directories(${AMPL_INCLUDE_DIRS})
    add_executable(coliny exec/coliny.cpp exec/funcadd.c)
  else()
    add_executable(coliny exec/coliny.cpp)
  endif()
  target_link_libraries(coliny interfaces colin)
  if(HAVE_SCOLIB)
    target_link_libraries(coliny scolib)
  endif()
  install(TARGETS coliny DESTINATION bin)
endif()

install(DIRECTORY interfaces DESTINATION include PATTERN .svn EXCLUDE)

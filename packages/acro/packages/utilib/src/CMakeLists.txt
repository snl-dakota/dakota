# Build the UTILIB libraries
# Warning - does not include the CXX-TEST tools at this time

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(libs)


# Following autotools pattern for now and building binaries from here
#add_subdirectory(exec)
add_executable(memmon       exec/memmon.c)
if(NOT MSVC)# This will need a huge port to work on win32 native
  add_executable(timer      exec/timer.c)
endif()
add_executable(complete_doe exec/doe/complete_doe.cpp)
add_executable(xu           exec/doe/xu.c)
add_executable(xu_doe       exec/doe/xu_doe.cpp)

target_link_libraries(complete_doe utilib ${Boost_LIBRARIES})
target_link_libraries(xu utilib ${Boost_LIBRARIES})
# Force CXX linker since depends on libutilib, but is a C source
set_target_properties(xu PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(xu_doe utilib ${Boost_LIBRARIES})

if(NOT MSVC)# This will need a huge port to work on win32 native
  install(TARGETS timer  DESTINATION bin)
endif()
install(TARGETS memmon DESTINATION bin)
# These testers will be built, but not installed:
#install(TARGETS complete_doe xu xu_doe DESTINATION bin)

install(DIRECTORY utilib DESTINATION include PATTERN .svn EXCLUDE)

include(FindPerl)
add_subdirectory(cantilever)
add_subdirectory(methods)
add_subdirectory(tutorial)

install(PROGRAMS script_interfaces/generic/dprepro DESTINATION bin)
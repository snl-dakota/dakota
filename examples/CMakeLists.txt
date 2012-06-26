include(FindPerl)
add_subdirectory(tutorial)
add_subdirectory(methods)

install(PROGRAMS script_interfaces/generic/dprepro DESTINATION bin)
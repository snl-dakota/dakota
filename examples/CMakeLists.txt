include(FindPerl)
add_subdirectory(cantilever)
add_subdirectory(users)

install(PROGRAMS script_interfaces/generic/dprepro DESTINATION bin)
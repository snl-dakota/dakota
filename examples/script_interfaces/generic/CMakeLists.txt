# Install dprepro and rosenbrock_bb to both bin/ and examples/ so generic 
# example will run out of the box
install(PROGRAMS ${Dakota_SOURCE_DIR}/scripts/dprepro
  DESTINATION "${DAKOTA_EXAMPLES_INSTALL}/examples/script_interfaces/generic")

add_executable(rosenbrock_bb rosenbrock_bb.f)
install(TARGETS rosenbrock_bb DESTINATION bin)
install(TARGETS rosenbrock_bb
  DESTINATION "${DAKOTA_EXAMPLES_INSTALL}/examples/script_interfaces/generic")

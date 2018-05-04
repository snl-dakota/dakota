# Install dprepro and rosenbrock_bb to both bin/ and examples/ so generic 
# example will run out of the box
include(ModifyTargetRpath)

INSTALL(PROGRAMS "${Dakota_SOURCE_DIR}/scripts/pyprepro/pyprepro"
      DESTINATION "${DAKOTA_EXAMPLES_INSTALL}/examples/script_interfaces/generic"
      RENAME "dprepro")

add_executable(rosenbrock_bb rosenbrock_bb.f)
#install(TARGETS rosenbrock_bb DESTINATION bin)
install(TARGETS rosenbrock_bb
  DESTINATION "${DAKOTA_EXAMPLES_INSTALL}/examples/script_interfaces/generic")
ModifyTargetRpath( rosenbrock_bb 5 )

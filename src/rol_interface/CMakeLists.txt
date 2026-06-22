# ROL Interface - Integrated Build
#
# The ROL interface sources are now built directly into dakota_src library
# to eliminate circular dependencies.
#
# Source files from rol_interface/src/ are added to dakota_src in the main
# CMakeLists.txt (around line 803-809)
#
# Include files from rol_interface/include/ are added to DAKOTA_INCDIRS in
# the main CMakeLists.txt (around line 812)
#
# TESTS: ROL interface tests are located in src/unit/dakota_rol/ and are
# built through the standard Dakota unit test framework (src/unit/CMakeLists.txt)

message(STATUS "ROL Interface: Sources integrated into dakota_src (no separate library)")

# Tests are handled by src/unit/dakota_rol/ - no test subdirectory here

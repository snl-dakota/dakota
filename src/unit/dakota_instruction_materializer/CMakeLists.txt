include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_instruction_materializer
  SOURCES instruction_materializer_tests.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS )

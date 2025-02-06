include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_lhs_constants
  SOURCES lhs_constants.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS )

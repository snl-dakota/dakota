include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_ego_state_variable
  SOURCES ego_state_variable_test.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS )

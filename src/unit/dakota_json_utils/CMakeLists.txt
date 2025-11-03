include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_json_utils
  SOURCES json_utils.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS )

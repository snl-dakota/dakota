include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_workdir_utils
  SOURCES test_workdir_utils.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

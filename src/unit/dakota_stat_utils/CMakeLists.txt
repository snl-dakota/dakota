include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_stat_utils
  SOURCES stat_utils.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_opt_api_traits
  SOURCES opt_api_traits.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

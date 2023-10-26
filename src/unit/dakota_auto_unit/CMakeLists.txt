include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_auto_unit
  SOURCES auto_unit_test.cpp
  LINK_DAKOTA_LIBS
  # need team buy-in for Boost.Test library deps
  #LINK_LIBS boost_unit_test_framework
  LINK_LIBS Boost::boost)

include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_opt_tpl_api
  SOURCES opt_tpl_api.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS GTest::gtest_main)

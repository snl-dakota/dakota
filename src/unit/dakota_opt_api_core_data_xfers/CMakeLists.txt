include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_opt_api_core_data_xfers
  SOURCES opt_api_core_data_xfers.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS GTest::gtest_main)

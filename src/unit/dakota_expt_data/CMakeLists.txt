include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_expt_data
  SOURCES expt_data.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS GTest::gtest_main)

include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_data_conversions
  SOURCES data_conversions.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS GTest::gtest_main)

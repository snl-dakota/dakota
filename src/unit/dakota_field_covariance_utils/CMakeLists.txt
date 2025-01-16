include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_field_covariance_utils
  SOURCES test_field_covariance_utils.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS GTest::gtest_main)

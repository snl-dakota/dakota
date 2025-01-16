include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_covariance_reader
  SOURCES covariance_reader.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS GTest::gtest_main)

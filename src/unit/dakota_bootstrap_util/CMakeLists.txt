include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_bootstrap_util
  SOURCES bootstrap_util_test.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS GTest::gtest_main)

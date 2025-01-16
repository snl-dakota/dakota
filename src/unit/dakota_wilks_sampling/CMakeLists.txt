include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_wilks_sampling
  SOURCES wilks_sampling.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS GTest::gtest_main)

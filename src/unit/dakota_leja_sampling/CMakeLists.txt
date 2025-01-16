include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_leja_sampling
  SOURCES leja_sampling.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS GTest::gtest_main)

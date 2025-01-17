include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_surr_gauss_proc
  SOURCES gauss_proc_test.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS dakota_surrogates GTest::gtest_main)

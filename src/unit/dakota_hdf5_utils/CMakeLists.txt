include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_hdf5_utils
  SOURCES hdf5_utils.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS GTest::gtest_main)

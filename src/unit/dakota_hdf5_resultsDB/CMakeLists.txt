include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_hdf5_resultsDB
  SOURCES ResultsDBHDF5_Test.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS GTest::gtest_main)

include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_nond_low_discrepancy_sampling_test
  SOURCES nond_low_discrepancy_sampling.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

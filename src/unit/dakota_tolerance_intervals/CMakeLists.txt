include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_tolerance_intervals
  SOURCES test_tolerance_intervals.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS )

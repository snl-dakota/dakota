include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_low_discrepancy_driver
  SOURCES low_discrepancy_driver.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS )

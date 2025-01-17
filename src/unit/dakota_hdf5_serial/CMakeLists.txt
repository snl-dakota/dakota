include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_hdf5_serial
  SOURCES hdf5_test_serial.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS )

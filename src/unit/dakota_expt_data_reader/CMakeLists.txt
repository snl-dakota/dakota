include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_expt_data_reader
  SOURCES expt_data_reader.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

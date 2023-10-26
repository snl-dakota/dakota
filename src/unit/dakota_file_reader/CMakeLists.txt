include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_file_reader
  SOURCES file_reader.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

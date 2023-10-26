include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_response_io
  SOURCES response_io.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

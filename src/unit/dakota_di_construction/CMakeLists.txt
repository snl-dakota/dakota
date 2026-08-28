include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_di_construction
  SOURCES di_construction.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS )

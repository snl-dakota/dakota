include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_rank_1_lattice_test
  SOURCES rank_1_lattice.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

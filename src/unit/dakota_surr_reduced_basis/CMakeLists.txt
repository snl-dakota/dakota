include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_surr_reduced_basis
  SOURCES reduced_basis.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS dakota_surrogates )

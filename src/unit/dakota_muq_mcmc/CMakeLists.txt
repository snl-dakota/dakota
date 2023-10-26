include(DakotaUnitTest)

  dakota_add_unit_test(NAME dakota_muq_mcmc
    SOURCES muq_mcmc.cpp
    LINK_DAKOTA_LIBS
    LINK_LIBS Boost::boost)

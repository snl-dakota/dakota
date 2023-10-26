include(DakotaUnitTest)

  dakota_add_unit_test(NAME dakota_preproc_tests
    SOURCES test_preproc.cpp
    LINK_DAKOTA_LIBS
    LINK_LIBS Boost::filesystem Boost::boost)

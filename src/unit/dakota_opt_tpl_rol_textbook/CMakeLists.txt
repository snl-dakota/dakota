include(DakotaUnitTest)

  dakota_add_unit_test(NAME dakota_opt_tpl_rol_textbook
    SOURCES opt_tpl_rol_test_textbook.cpp
    LINK_DAKOTA_LIBS
    LINK_LIBS Boost::boost)

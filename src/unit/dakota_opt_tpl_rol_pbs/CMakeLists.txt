include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_opt_tpl_rol_pbs
  SOURCES opt_tpl_rol_test_rol_pbs.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS )

include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_opt_tpl_adapters
  SOURCES opt_tpl_adapters.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS )

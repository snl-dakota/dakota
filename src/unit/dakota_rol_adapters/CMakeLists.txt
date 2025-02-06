include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_rol_adapters
  SOURCES rol_adapters.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS )

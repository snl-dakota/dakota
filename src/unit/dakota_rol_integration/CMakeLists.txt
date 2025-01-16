include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_rol_integration
  SOURCES rol_integration.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS GTest::gtest_main)

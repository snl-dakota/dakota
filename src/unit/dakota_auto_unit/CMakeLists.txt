include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_auto_unit
  SOURCES auto_unit_test.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS GTest::gtest_main)

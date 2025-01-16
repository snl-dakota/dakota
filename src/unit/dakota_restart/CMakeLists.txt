include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_restart
  SOURCES restart_test.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS GTest::gtest_main)

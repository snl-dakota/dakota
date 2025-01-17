include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_min_unit
  SOURCES min_unit_test.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS GTest::gtest_main)

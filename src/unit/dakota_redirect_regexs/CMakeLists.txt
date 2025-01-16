include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_redirect_regexs
  SOURCES redirect_regexs.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS GTest::gtest_main)

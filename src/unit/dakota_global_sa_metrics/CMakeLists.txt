include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_global_sa_metrics
  SOURCES global_sa_metrics.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS GTest::gtest_main)

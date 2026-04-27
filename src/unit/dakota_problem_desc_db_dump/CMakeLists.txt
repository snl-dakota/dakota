include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_problem_desc_db_dump
  SOURCES problem_desc_db_dump.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS )

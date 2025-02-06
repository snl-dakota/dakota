include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_json_results_parser
  SOURCES json_results_parser.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS )

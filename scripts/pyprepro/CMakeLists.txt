

if(PYTHONINTERP_FOUND AND PYTHON_VERSION_STRING VERSION_GREATER 2.6)
  add_test(NAME pyprepro_test 
	  COMMAND ${PYTHON_EXECUTABLE} -B run_tests.py
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  set_property(TEST pyprepro_test PROPERTY LABELS UnitTest)
  set_property(TEST pyprepro_test 
	  PROPERTY ENVIRONMENT PYTHONPATH "$ENV{PYTHONPATH}:${CMAKE_SOURCE_DIR}/interfaces/Python")
endif()

install(PROGRAMS "pyprepro" DESTINATION "bin/")
install(PROGRAMS "pyprepro" DESTINATION "bin/" RENAME "dprepro")


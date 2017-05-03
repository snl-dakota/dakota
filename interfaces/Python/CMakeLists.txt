

if(PYTHONINTERP_FOUND AND PYTHON_VERSION_STRING VERSION_GREATER 2.7)
  add_test(NAME dakota_interfacing_test 
	  COMMAND ${PYTHON_EXECUTABLE} -B dakota_interfacing_test.py
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  set_property(TEST dakota_interfacing_test PROPERTY LABELS UnitTest)
endif()

install(DIRECTORY "dakota" DESTINATION "share/dakota/Python/")
install(FILES "mpitile" DESTINATION "bin/")


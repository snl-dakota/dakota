

if(DAKOTA_ENABLE_TESTS AND Python_Interpreter_FOUND AND
    Python_VERSION VERSION_GREATER 2.6)
  add_test(NAME dakota_interfacing_test 
	  COMMAND ${Python_EXECUTABLE} -B dakota_interfacing_test.py
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  set_property(TEST dakota_interfacing_test PROPERTY LABELS UnitTest)
endif()

install(DIRECTORY "dakota" DESTINATION "share/dakota/Python/")
install(PROGRAMS "mpitile" DESTINATION "bin/")


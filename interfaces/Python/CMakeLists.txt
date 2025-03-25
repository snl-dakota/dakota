

if(DAKOTA_ENABLE_TESTS AND Python_Interpreter_FOUND AND
    Python_VERSION VERSION_GREATER 2.6)
  add_test(NAME dakota_interfacing_test 
	  COMMAND ${Python3_EXECUTABLE} -B dakota_interfacing_test.py
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  set_property(TEST dakota_interfacing_test PROPERTY LABELS Unit Python)
endif()

install(DIRECTORY "dakota" DESTINATION "share/dakota/Python/"
  PATTERN "__pycache__" EXCLUDE
  PATTERN "*.py[cdo]" EXCLUDE
  )
install(PROGRAMS "mpitile" DESTINATION "bin/")


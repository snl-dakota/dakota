

if(DAKOTA_ENABLE_TESTS AND Python_Interpreter_FOUND AND
    Python_VERSION VERSION_GREATER 2.6)
  add_test(NAME pyprepro_test 
	  COMMAND ${Python_EXECUTABLE} -B run_tests.py
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  set_property(TEST pyprepro_test PROPERTY LABELS Unit Python)
  set_property(TEST pyprepro_test 
	  PROPERTY ENVIRONMENT PYTHONPATH "$ENV{PYTHONPATH}:${CMAKE_SOURCE_DIR}/interfaces/Python")
endif()

# Install both tool (for backward comat) and tool.py (for Windows
# extension-based lookup). 
install(PROGRAMS "pyprepro" DESTINATION "bin/")
install(PROGRAMS "pyprepro.py" DESTINATION "bin/")
install(PROGRAMS "dprepro" DESTINATION "bin/")
install(PROGRAMS "dprepro3" DESTINATION "bin/")
install(PROGRAMS "dprepro.py" DESTINATION "bin/")



if(PYTHONINTERP_FOUND AND PYTHON_VERSION_STRING VERSION_GREATER 2.5)
  add_test(NAME dipy_test 
	  COMMAND ${PYTHON_EXECUTABLE} -B dipy_test.py
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  set_property(TEST dipy_test PROPERTY LABELS UnitTest)
  install(FILES "dipy.py" DESTINATION "share/dakota/interfaces/Python/")
endif()




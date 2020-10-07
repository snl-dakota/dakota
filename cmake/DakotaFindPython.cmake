# Conditionally find various Python components, depending on Dakota config
macro(dakota_find_python)

  if(DAKOTA_PYTHON)
    message(STATUS "Dakota enabling Python (Interpreter)")
    set(dakota_python_components Interpreter)

    if(DAKOTA_PYTHON_DIRECT_INTERFACE)
      message(STATUS "Dakota enabling Python direct interface (Development)")
      list(APPEND dakota_python_components Development)

      if (DAKOTA_PYTHON_DIRECT_INTERFACE_NUMPY)
	message(STATUS "Dakota enabling Python direct interface with NumPy")
	if (CMAKE_VERSION VERSION_LESS 3.14)
	  message(WARNING "Enabling Dakota's NumPy interface requires CMake "
	    "3.14 or newer;\nsee option DAKOTA_PYTHON_DIRECT_INTERFACE_NUMPY")
	endif()

	list(APPEND dakota_python_components NumPy)
      endif()

    endif()

    find_package(Python REQUIRED ${dakota_python_components})

    # pybind11, C3, Acro, etc., use older CMake FindPythonInterp, so we
    # coerce it to use same as Dakota; more complex situations may
    # require setting other variables
    if(NOT PYTHON_EXECUTABLE)
      set(PYTHON_EXECUTABLE "${Python_EXECUTABLE}" CACHE FILEPATH
	"Dakota set legacy PYTHON_EXECUTABLE to match Python_EXECUTABLE")
    endif()
    
    # TODO: fine-grained error messages
    # if(DAKOTA_PYTHON_DIRECT_INTERFACE and NOT Python_Development_FOUND)

    if(DAKOTA_PYTHON_SURROGATES)
      # Find Pybind11 (TODO on branch)
    endif()

  else()

    # Disable some components that default ON
    message(STATUS
      "DAKOTA_PYTHON = ${DAKOTA_PYTHON}; disabling all Python components")
    set(DAKOTA_PYTHON_DIRECT_INTERFACE OFF CACHE BOOL
      "Direct interface disabled based on DAKOTA_PYTHON=${DAKOTA_PYTHON}")
    set(DAKOTA_PYTHON_SURROGATES OFF CACHE BOOL
      "Direct interface disabled based on DAKOTA_PYTHON=${DAKOTA_PYTHON}")

  endif()

endmacro()

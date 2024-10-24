# Conditionally find various Python components, depending on Dakota config
macro(dakota_find_python)

  if(DAKOTA_PYTHON)
    message(STATUS "Dakota enabling Python (Interpreter)")
    set(dakota_python_components Interpreter)

    if(DAKOTA_PYTHON_DIRECT_INTERFACE OR DAKOTA_PYTHON_SURROGATES OR
	DAKOTA_PYTHON_WRAPPER OR DAKOTA_PYBIND11)
      message(STATUS "Dakota enabling Python (Development) for direct or surrogate interface")
      list(APPEND dakota_python_components Development)
      
      if (DAKOTA_PYTHON_DIRECT_INTERFACE_LEGACY)
        if (DAKOTA_PYTHON_DIRECT_INTERFACE)
		message(STATUS "Dakota enabling legacy Python direct interface")
	else()
		message(WARNING "Request to enable Dakota's legacy Python direct interface ignored; set DAKOTA_PYTHON_DIRECT_INTERFACE to ON to use it")
        endif()
      endif()

      if (DAKOTA_PYTHON_DIRECT_INTERFACE_NUMPY)
	if (DAKOTA_PYTHON_DIRECT_INTERFACE)
          message(STATUS "Dakota enabling Python direct interface with NumPy")
	  list(APPEND dakota_python_components NumPy)
	else()
		message(WARNING "Request to enable NumPy in Dakota's Python direct interface ignored; set DAKOTA_PYTHON_DIRECT_INTERFACE to ON to use it")
	endif()
      endif()

    endif()

    find_package(Python REQUIRED ${dakota_python_components})

    if (DAKOTA_PYTHON_DIRECT_INTERFACE_NUMPY)
	    message(STATUS "NumPy version ${Python_NumPy_VERSION} found at ${Python_NumPy_INCLUDE_DIRS}")
        if (DAKOTA_PYTHON_DIRECT_INTERFACE_LEGACY)
          string(FIND ${Python_NumPy_VERSION} "." first_dot)
	  if(first_dot GREATER -1)
	    string(SUBSTRING ${Python_NumPy_VERSION} 0 ${first_dot} MAJOR_VERSION)
	    if(MAJOR_VERSION GREATER 1)
              message(FATAL_ERROR "Dakota legacy Python direct interface incompatible with NumPy version 2+; set DAKOTA_PYTHON_DIRECT_INTERFACE_LEGACY to OFF")
	    endif()
	  endif()
        endif()
    endif()


    # pybind11, C3, Acro, etc., use older CMake FindPythonInterp, so we
    # coerce it to use same as Dakota; more complex situations may
    # require setting other variables
    if(NOT PYTHON_EXECUTABLE)
      set(PYTHON_EXECUTABLE "${Python_EXECUTABLE}" CACHE FILEPATH
	"Dakota set legacy PYTHON_EXECUTABLE to match Python_EXECUTABLE")
    endif()
    
    # TODO: fine-grained error messages
    # if(DAKOTA_PYTHON_DIRECT_INTERFACE and NOT Python_Development_FOUND)

    if(DAKOTA_PYBIND11)
      # This add_subdirectory must be done at top-level so pybind11's
      # CMake functions are pulled in for src/ and below
      add_subdirectory(packages/external/pybind11)
    endif()

    # If building with HDF5 support was requested, check for h5py available.
    if(DAKOTA_HAVE_HDF5)
      execute_process(COMMAND ${Python_EXECUTABLE} -c "import h5py"
      RESULT_VARIABLE missing_h5py OUTPUT_QUIET ERROR_QUIET)
      if(NOT missing_h5py)
        set(DAKOTA_H5PY_FOUND ON CACHE BOOL "h5py probe successful")
        message(STATUS "Python h5py module found")
      else()
        set(DAKOTA_H5PY_FOUND OFF CACHE BOOL "h5py probe failed")
        message(WARNING "HDF5 requested, but Python h5py module not found. HDF5 tests that require h5py will be disabled.")
      endif()
    endif()

  else()

    # Disable some components that definitely won't work
    message(STATUS
      "DAKOTA_PYTHON = ${DAKOTA_PYTHON}; disabling all Python components")
    set(DAKOTA_PYTHON_DIRECT_INTERFACE OFF CACHE BOOL
      "Python direct interface disabled based on DAKOTA_PYTHON=${DAKOTA_PYTHON}")
    set(DAKOTA_PYTHON_SURROGATES OFF CACHE BOOL
      "Python surrogates interface disabled based on DAKOTA_PYTHON=${DAKOTA_PYTHON}")
    set(DAKOTA_PYTHON_WRAPPER OFF CACHE BOOL
      "Python Dakota wrapper disabled based on DAKOTA_PYTHON=${DAKOTA_PYTHON}")

  endif()

endmacro()

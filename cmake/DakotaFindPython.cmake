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
      "Direct interface disabled based on DAKOTA_PYTHON=${DAKOTA_PYTHON}")
    set(DAKOTA_PYTHON_SURROGATES OFF CACHE BOOL
      "Direct interface disabled based on DAKOTA_PYTHON=${DAKOTA_PYTHON}")

  endif()

endmacro()

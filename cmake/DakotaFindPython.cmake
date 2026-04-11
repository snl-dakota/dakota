# Conditionally find various Python3 components, depending on Dakota config
macro(dakota_find_python)

  if(DAKOTA_PYTHON)
    message(STATUS "Dakota enabling Python3 (Interpreter)")
    set(dakota_python_components Interpreter)

    if(DAKOTA_PYTHON_DIRECT_INTERFACE OR DAKOTA_PYTHON_SURROGATES OR
	DAKOTA_PYTHON_WRAPPER OR DAKOTA_PYBIND11)
      message(STATUS
        "Dakota enabling Python3 Development.Module and Development.Embed "
        "for Python modules and embedded interpreter support")
      list(APPEND dakota_python_components Development.Module Development.Embed)
      
      if (DAKOTA_PYTHON_DIRECT_INTERFACE_NUMPY)
	if (DAKOTA_PYTHON_DIRECT_INTERFACE)
          message(STATUS "Dakota enabling Python3 direct interface with NumPy")
	  list(APPEND dakota_python_components NumPy)
	else()
          message(WARNING "Request to enable NumPy in Dakota's Python3 direct interface forced OFF; set DAKOTA_PYTHON_DIRECT_INTERFACE to ON to use it")
          set(DAKOTA_PYTHON_DIRECT_INTERFACE_NUMPY OFF CACHE BOOL "Force OFF because direct interface not enabled" FORCE) 
	endif()
      endif()

    endif()

    find_package(Python3 REQUIRED ${dakota_python_components})

    foreach(_dakota_python_target Python3::Module Python3::Python Python3::Interpreter)
      if(TARGET ${_dakota_python_target})
        get_target_property(_dakota_imported_location ${_dakota_python_target} IMPORTED_LOCATION)
        get_target_property(_dakota_imported_location_release ${_dakota_python_target} IMPORTED_LOCATION_RELEASE)
        get_target_property(_dakota_interface_link_libraries ${_dakota_python_target} INTERFACE_LINK_LIBRARIES)
        get_target_property(_dakota_interface_link_options ${_dakota_python_target} INTERFACE_LINK_OPTIONS)
        message(STATUS "${_dakota_python_target} IMPORTED_LOCATION = ${_dakota_imported_location}")
        message(STATUS "${_dakota_python_target} IMPORTED_LOCATION_RELEASE = ${_dakota_imported_location_release}")
        message(STATUS "${_dakota_python_target} INTERFACE_LINK_LIBRARIES = ${_dakota_interface_link_libraries}")
        message(STATUS "${_dakota_python_target} INTERFACE_LINK_OPTIONS = ${_dakota_interface_link_options}")
      else()
        message(STATUS "${_dakota_python_target} target not defined")
      endif()
    endforeach()

    if (DAKOTA_PYTHON_DIRECT_INTERFACE_NUMPY)
	message(STATUS "NumPy version ${Python_NumPy_VERSION} found at ${Python_NumPy_INCLUDE_DIRS}")
    endif()


    # Keep legacy consumers aligned on the selected interpreter.
    if(NOT PYTHON_EXECUTABLE)
      set(PYTHON_EXECUTABLE "${Python3_EXECUTABLE}" CACHE FILEPATH
	"Dakota set PYTHON_EXECUTABLE to match Python3_EXECUTABLE")
    endif()
    
    # TODO: fine-grained error messages
    # if(DAKOTA_PYTHON_DIRECT_INTERFACE and NOT Python_Development_FOUND)

    if(DAKOTA_PYBIND11)
      set(PYBIND11_FINDPYTHON ON CACHE BOOL
        "Force pybind11 to use CMake FindPython targets" FORCE)
      # This add_subdirectory must be done at top-level so pybind11's
      # CMake functions are pulled in for src/ and below
      add_subdirectory(packages/external/pybind11)
    endif()

    # Check for default and user-requested python module support
    set(dakota_python_modules h5py numpy scipy)

    if(DEFINED PROBE_PYTHON_MODULES)
      list(APPEND dakota_python_modules ${PROBE_PYTHON_MODULES})
    endif()

    foreach(mod ${dakota_python_modules})
      string(TOUPPER "${mod}" upmod)
      execute_process(COMMAND ${Python3_EXECUTABLE} -c "import ${mod}"
      RESULT_VARIABLE missing_mod OUTPUT_QUIET ERROR_QUIET)
      if(NOT missing_mod)
        set(DAKOTA_PYTHON_${upmod}_FOUND ON CACHE BOOL "${mod} probe successful")
        message(STATUS "Python3 ${mod} module found")
        add_definitions("-DDAKOTA_PYTHON_${upmod}_FOUND")
      else()
        set(DAKOTA_PYTHON_${upmod}_FOUND OFF CACHE BOOL "${mod} probe failed")
        message(STATUS "Python3 ${mod} module NOT found. Tests that require DAKOTA_PYTHON_${upmod}_FOUND will be disabled.")
      endif()
    endforeach()

  else()

    # Disable some components that definitely won't work
    message(STATUS
      "DAKOTA_PYTHON = ${DAKOTA_PYTHON}; disabling all Python3 components")
    set(DAKOTA_PYTHON_DIRECT_INTERFACE OFF CACHE BOOL
      "Python3 direct interface disabled based on DAKOTA_PYTHON=${DAKOTA_PYTHON}")
    set(DAKOTA_PYTHON_SURROGATES OFF CACHE BOOL
      "Python3 surrogates interface disabled based on DAKOTA_PYTHON=${DAKOTA_PYTHON}")
    set(DAKOTA_PYTHON_WRAPPER OFF CACHE BOOL
      "Python3 Dakota wrapper disabled based on DAKOTA_PYTHON=${DAKOTA_PYTHON}")

  endif()

endmacro()

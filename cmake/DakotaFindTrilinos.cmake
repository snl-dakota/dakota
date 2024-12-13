# Dakota-specific Trilinos probe to allow building under
# Trilinos/TriKota, with external Trilinos, or using Dakota's
# packages/
macro(dakota_find_trilinos)

  # TODO: Have etphipp remove Trilinos-specific code and instead set in TriKota
  # when building inside Trilinos, the path to Teuchos will already be set
  if (NOT BUILD_IN_TRILINOS)

    # Workaround to skip finding system Trilinos until this probe is
    # simplified and follows find_package semantics. Double negative to
    # preserve historical behavior without overcomplicating things.
    if(NOT DAKOTA_NO_FIND_TRILINOS)
      # First probe for system-installed Trilinos, respecting Trilinos_DIR if set
      find_package(Trilinos QUIET)
    endif()

    if(Trilinos_FOUND)
      if(Trilinos_DIR)
	message(STATUS
	  "Dakota using previously specified Trilinos in ${Trilinos_DIR}")
      else()
	message(STATUS "Dakota using external Trilinos")
      endif()
      # TODO: Make this a hard error and check for version during find_package
      if(DEFINED Trilinos_VERSION AND Trilinos_VERSION VERSION_LESS 12.16)
	message(WARNING
	  "Dakota requires Trilinos 12.16 or newer; found ${Trilinos_VERSION}"
	  "Hint: Set DAKOTA_NO_FIND_TRILINOS:BOOL=TRUE to use Dakota's copy"
	  )
      endif()
    else()

      # If no parent project configured Teuchos, do so, using Teuchos_DIR if set
      if(Teuchos_DIR)
	message(STATUS
	  "Dakota using previously specified Teuchos in ${Teuchos_DIR}")
      else()
	# Directory containing TeuchosConfig.cmake at build time
	set(Teuchos_DIR
          ${CMAKE_CURRENT_BINARY_DIR}/packages/external/trilinos/cmake_packages/Teuchos)
	set(Trilinos_ENABLE_Teuchos ON CACHE BOOL
          "Dakota enabling Trilinos Teuchos" FORCE)
        #set(Trilinos_VERBOSE_CONFIGURE OFF CACHE BOOL
        #  "Dakota enabling Trilinos VERBOSE Configure" FORCE)
	# Map key Dakota variables to TriBITS variables
	set( TPL_BLAS_LIBRARIES ${BLAS_LIBS} )
	set( TPL_LAPACK_LIBRARIES ${LAPACK_LIBS} )
	# Newer versions of Trilinos automatically turn off Fortran name-mangling
	# on Windows
	if(WIN32)
          set(Trilinos_ENABLE_Fortran ON CACHE BOOL
	    "Dakota enabling Trilinos Fortran")
	endif()

	# Dakota doesn't use any Teuchos MPI features; may want to force off
	#set( TPL_ENABLE_MPI ${DAKOTA_HAVE_MPI} )
	# This doesn't do as name implies; setting OFF doesn't generate Config.cmake 
	# at all; doesn't just control whether installed!  Want Config.cmake in build
	#        set(Trilinos_ENABLE_INSTALL_CMAKE_CONFIG_FILES OFF CACHE BOOL
	#	  "Dakota is the top-level package; don't write Trilinos config files")

	message(STATUS "Dakota setting Teuchos_DIR to ${Teuchos_DIR}")
	set(DAKOTA_BUILDING_TEUCHOS TRUE CACHE BOOL
          "Dakota is building Teuchos in its build tree" FORCE)

	# Partial support for ROL TPL; some logic paths are uncovered in
	# the case of specified Teuchos_DIR.  This only works since ROL
	# is header-only and won't build/install any libraries.
	if(HAVE_ROL)
	  # Enabling so we generate ROL_config.h
	  set(Trilinos_ENABLE_ROL ON CACHE BOOL "Dakota enabling Trilinos ROL")
	  # Directory containing ROLConfig.cmake at build time (not used for now)
	  ##set(ROL_DIR
          ##  ${CMAKE_CURRENT_BINARY_DIR}/packages/external/trilinos/packages/rol)
	  ##message(STATUS "Dakota setting ROL_DIR to ${ROL_DIR}")
	else()
	  set(Trilinos_ENABLE_ROL OFF CACHE BOOL "Dakota disabling Trilinos ROL")
	endif()

	# This mirrors the Trilinos release process setting, so Dakota
	# can work with Trilinos master, when needed.  Specifically it
	# will suppress errors about missing packages.
	set(Trilinos_ENABLE_DEVELOPMENT_MODE OFF CACHE BOOL
	  "Dakota disabling Trilinos development mode")
	# This would be lighter weight, but might result in strong warnings
	##set(Trilinos_ASSERT_MISSING_PACKAGES OFF CACHE BOOL "Dakota being lazy")

	set(Trilinos_GENERATE_REPO_VERSION_FILE OFF CACHE BOOL
          "Dakota disabling generation of TrilinosRepoVersion.txt")

	# BMA: Since Pecos surrogates not using complex yet, disabled this
	##set(Trilinos_ENABLE_COMPLEX_DOUBLE TRUE CACHE BOOL
	##  "Pecos requires some complex blas wrappers")

	set(Teuchos_HIDE_DEPRECATED_CODE TRUE CACHE BOOL
          "Teuchos deprecated code clashes with operator<< in Dakota")

	add_subdirectory(packages/external/trilinos)

      endif() # Teuchos_DIR

      # Additional setting to prevent multiple targets with the same name
      set(Trilinos_TARGETS_IMPORTED 1)

    endif() # Trilinos_DIR

  endif() # NOT BUILD_IN_TRILINOS
endmacro()

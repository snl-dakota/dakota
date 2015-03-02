# TODO: Use cases to review
#  - When testing, try case of no wrapper on PATH (fail)
#  - New probe needs to support
#  - Trilinos setting the compiler wrappers, or not using them
#  - default of use include/libs
#  - option to set and use compiler wrapper for CXX
#  - TODO: If you just specify MPI_CXX_COMPILER to an alternate you find
#    the wrong MPIEXEC if PATH isn't set.


# Function DakotaFindMPI wraps the CMake MPI probe
#
# When DAKOTA_HAVE_MPI is on:
#
#  * Default: probe using CMake probe, use defines, headers, libs found;
#    will fallback on CMAKE_CXX_COMPILER as last resort
#
#  * User can guide with MPI_CXX_COMPILER to push us to the right MPI
#
#  * User can specify MPI_CXX_INCLUDE_PATH and MPI_CXX_LIBRARIES to
#    circumvent autodetection
#
#  * User can specify CMAKE_CXX_COMPILER=mpicxx and
#    MPI_CXX_COMPILER=mpicxx to make sure right MPI is found.  We will
#    still append found MPI LIBRARIES, but could autodetect and not...
#
#  * DAKOTA_MPI_WRAPPER_ONLY will use CMAKE_CXX_COMPILER (which must
#    be set to MPI wrapper) only and omit libs/headers; in this case
#    could skip probe entirely...
function(DakotaFindMPI)

  if (DAKOTA_HAVE_MPI)

    message(STATUS "DAKOTA MPI: requested.")

    if (DAKOTA_MPI_WRAPPER_ONLY AND CMAKE_CXX_COMPILER AND NOT MPI_CXX_COMPILER)
      # Help MPI find the compiler In this case?
      # May be better to just require users to set MPI_CXX_COMPILER as
      # well to preclude the probe finding a different
      # MPI_CXX_COMPILER than is set for CMAKE_CXX_COMPILER
      set(MPI_CXX_COMPILER ${CMAKE_CXX_COMPILER} CACHE FILEPATH 
	"MPI CXX compiler overridden with CXX compiler")
    endif()

    #if (NOT DAKOTA_MPI_WRAPPER_ONLY)
    # Do not use REQUIRED option since we only need MPI_CXX; instead perform 
    # our own error check below.
    find_package(MPI)
    #endif()

    message(STATUS "Dakota MPI C++ configuration:")
    foreach(output 
	FOUND COMPILER COMPILE_FLAGS INCLUDE_PATH LINK_FLAGS LIBRARIES)
      message(STATUS "  MPI_CXX_${output}: ${MPI_CXX_${output}}")
    endforeach()
    message(STATUS "  MPIEXEC: ${MPIEXEC}")

    # cmake-2.8.5 and newer discriminate among compiler types; we only need CXX
    # is that true, or do we only need C functions?
    if(NOT MPI_CXX_FOUND)
      message(FATAL_ERROR "DAKOTA MPI requested, but MPI for C++ not found. "
        " Please build a serial configuration instead")
    endif()

  else()
    message(STATUS "DAKOTA MPI: not requested.")
  endif()

endfunction()


# Apply the MPI compile and link flags to the specified target; this
# function should be used on any target that should use the MPI
# compile or link flags.
function(DakotaApplyMPISettings mpi_enabled_target)

  # If using wrapper only, don't apply detected MPI settings
  if (DAKOTA_HAVE_MPI AND NOT DAKOTA_MPI_WRAPPER_ONLY)
  
    # This must be done once per directory src/ and test/, and NOT at top-level:
    #if (MPI_CXX_INCLUDE_PATH)
    #  include_directories(${MPI_CXX_INCLUDE_PATH})
    #endif()

    # Omit this since MPI libraries are added to a global link list in
    # src/ and test/
    #if (MPI_CXX_LIBRARIES) 
    #  target_link_libraries(${mpi_enabled_target} ${MPI_CXX_LIBRARIES})
    #endif()

    if(MPI_CXX_COMPILE_FLAGS)
      set_target_properties(${mpi_enabled_target} PROPERTIES 
	COMPILE_FLAGS "${MPI_CXX_COMPILE_FLAGS}")
    endif()

    if(MPI_CXX_LINK_FLAGS)
      set_target_properties(${mpi_enabled_target} PROPERTIES
	LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
    endif()

  endif()

endfunction()

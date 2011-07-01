# Check for a MPI-enabled build.  
#
# Recommended practice is to set DAKOTA_HAVE_MPI and set CMAKE_CXX_COMPILER 
# to a compiler wrapper.
#
# However, we try to do introspection as well; alternates:
# * set MPI_CXX_COMPILER in cache to a specific MPI C++ wrapper (Trilinos)
# * set PATH in environment to include MPI C++ wrapper
# * set MPI_LIBRARY and MPI_INCLUDE_PATH in cache
# * set MPI_BASE_DIR in cache (Trilinos)
# * set MPIHOME in environment
#
# When testing, try case of no wrapper on PATH (fail)
function(DakotaHaveMPI)

  if (DAKOTA_HAVE_MPI)
  
    message(STATUS "DAKOTA MPI: requested.")
  
    # For now we coerce CMake's FindMPI to do the right thing
  
    # Model loosely after Trilinos PackageArchSetupMPI.cmake. Consider
    # bringing in their macros, since will get all wrappers right,
    # option for include/lib, and MPI runtime executables, but right now
    # pulling that thread means keeping at least 12 files in sync
  
    # DAKOTA only requires the C++ compiler wrapper
    if (CMAKE_CXX_COMPILER)
  
      message(STATUS 
        "DAKOTA MPI: CMAKE_CXX_COMPILER already set to\n"
        "               ${CMAKE_CXX_COMPILER}.\n"
        "               Will use its path to try to find MPI.")
  
      # This doesn't work; MPI_COMPILER gets overwritten in FindMPI
      #set(MPI_COMPILER ${CMAKE_CXX_COMPILER}
      #  CACHE FILEPATH "MPI compiler set to CXX compiler.")
  
    else(CMAKE_CXX_COMPILER)
  
      # Try to find only the C++ compiler wrappers 
      # (FindMPI.cmake doesn't discriminate)
      # autotools names: mpic++ mpicxx mpiCC hcp mpxlC_r mpxlC mpCC cmpic++
      # Trilinos names: mpicxx mpic++ mpiCC
      set(possible_wrappers 
        ${MPI_CXX_COMPILER} mpicxx mpic++ mpiCC hcp mpxlC_r mpxlC mpCC cmpic++)
  
      # Could consider using name MPI_CXX_COMPILER and skipping check if given
      find_program(DAKOTA_MPI_CXX_COMPILER
        NAMES ${possible_wrappers} 
        HINTS ${MPI_BASE_DIR} ENV MPIHOME 
        PATH_SUFFIXES bin
        DOC "DAKOTA local MPI compiler. Used to set MPI location."
        )
      mark_as_advanced(DAKOTA_MPI_CXX_COMPILER)
  
      if(DAKOTA_MPI_CXX_COMPILER)
        message(STATUS 
  	"DAKOTA MPI: found MPI compiler wrapper:\n" 
  	"               ${DAKOTA_MPI_CXX_COMPILER}.\n"
  	"               Setting CMAKE_CXX_COMPILER.")
        set(CMAKE_CXX_COMPILER ${DAKOTA_MPI_CXX_COMPILER} 
  	CACHE FILEPATH "CXX compiler overridden with MPI C++ wrapper")
      else()
        message(STATUS
  	"DAKOTA MPI: could not find compiler wrapper. Consider setting:\n"
  	"               CMAKE_CXX_COMPILER (cache) or\n"
  	"               MPIHOME (environment) or\n"
  	"               MPI_BASE_DIR (cache) or\n"
  	"               (MPI_LIBRARY and MPI_INCLUDE_PATH in cache)")
        message(STATUS "DAKOTA MPI: assuming user management of MPI.")
      endif()
  
    endif(CMAKE_CXX_COMPILER)
  
    # Get the PATH of the CXX compiler and add to search path if it's a path
    if (CMAKE_CXX_COMPILER) # won't be set if using include/lib
      get_filename_component(wrapper_bindir ${CMAKE_CXX_COMPILER} PATH)
      if (wrapper_bindir)
        list(APPEND CMAKE_PROGRAM_PATH ${wrapper_bindir})
        message(STATUS 
  	"DAKOTA_MPI: added ${wrapper_bindir} to CMAKE_PROGRAM_PATH.")
      endif()
    endif()
  
    # Use the native CMake MPI probe
    find_package(MPI REQUIRED)
  
    if(NOT MPI_FOUND)
      message(FATAL_ERROR "DAKOTA MPI requested, but not found. "
        " Please build a serial configuration instead")
    endif(MPI_FOUND)
    # Some TPLs manage the setting with USE_MPI rather than HAVE_MPI
    set(USE_MPI "ON")
  
  endif()

endfunction()

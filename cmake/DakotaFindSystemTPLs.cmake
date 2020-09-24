# Dakota CMake helpers to find system TPLs
# (These are initially macros in case of unexpected dependencies on variables they set.)

macro(dakota_find_boost)
  if(WIN32)
    # BMA TODO: Relax this and document
    set(Boost_USE_STATIC_LIBS TRUE)
  endif()
  # Dakota requires the specified compiled Boost library components
  # Dakota requires Boost 1.58 or newer (1.69 recommended);
  # enforce for all libs in the build
  find_package(Boost 1.58 REQUIRED
    COMPONENTS "filesystem;program_options;regex;serialization;system")
  set(DAKOTA_BOOST_TARGETS Boost::boost Boost::filesystem Boost::program_options
    Boost::regex Boost::serialization Boost::system)

  # BMA TODO: relax this and verify Modern CMake behavior
  # This cache variable is used to package the Boost liraries on Darwin (see
  # cmake/InstallDarwinDylibs.cmake). The variable Boost_LIBRARY_DIRS is set
  # in this scope by the CMake Boost probe (at least for CMake 3.5.2), but
  # this changes from version to version of CMake.
  if(CMAKE_SYSTEM_NAME MATCHES Darwin)
    if("${Boost_LIBRARY_DIRS}" STREQUAL "")
      message(WARNING "The Boost probe for this version of CMake (${CMAKE_VERSION}) \
        does not set Boost_LIBRARY_DIRS. Please notify the Dakota development team.")
    endif()
    set(DAKOTA_Boost_LIB_DIR "${Boost_LIBRARY_DIRS}" CACHE PATH "Dakota-added Boost lib path")
  endif()
endmacro()

# Dakota custom find BLAS/LAPACK or equivalent Fortran linear algebra
# BMA TODO: Update to use CMake BLAS/LAPACK probes and properly use their output
macro(dakota_find_linalg)
  if(NOT DEFINED BLAS_LIBS OR NOT DEFINED LAPACK_LIBS)
    # Historically on MSVC, tried to use CMake config file approach first.  
    # Could probably just use the Unix logic below instead...
    if(MSVC)
      find_package(LAPACK REQUIRED NO_MODULE)
      set(BLAS_LIBS blas)
      set(LAPACK_LIBS lapack)
    else()
      # first check for a system blas and lapack
      if(NOT DEFINED BLAS_LIBS)
	find_library(BLAS_LIBS blas)
      endif()
      if(NOT DEFINED LAPACK_LIBS)
	find_library(LAPACK_LIBS lapack)
      endif()
      if(NOT BLAS_LIBS OR NOT LAPACK_LIBS)
	# if not a system blas and lapack, then look for a cmake built LAPACK
	# with find_package
	find_package(LAPACK REQUIRED NO_MODULE)
	set(BLAS_LIBS blas)
	set(LAPACK_LIBS lapack)
      endif()
    endif()
  endif()
endmacro()


# Conditionally find GSL using Dakota copy of CMake probe
# BMA TODO: Once require CMake 3.2 or newer, use CMake standard probe
# BMA TODO: Document DAKOTA_CBLAS_LIBS
macro(dakota_find_gsl)
  if(DAKOTA_HAVE_GSL)
    # One may want to provide an alternate CBLAS library via
    # DAKOTA_CBLAS_LIBS to use in place of GSL's CBLAS.  On many
    # platforms, linking the GSL CBLAS induces numerical DIFFs in other
    # parts of Dakota. (Might prefer to probe for a library containing
    # cblas_ symbols, then conditionally set this option automatically.)
    if(DAKOTA_CBLAS_LIBS)
      set(GSL_WITHOUT_CBLAS TRUE CACHE BOOL "Omit GSL CBLAS libraries")
    endif()
    find_package(GSL REQUIRED)
    message(STATUS "Found GSL libraries: ${GSL_LIBRARIES}")
  endif()
endmacro()

# Conditionally find HDF5 using standard CMake probe
macro(dakota_find_hdf5)
  if(DAKOTA_HAVE_HDF5)
    find_package(HDF5 REQUIRED COMPONENTS "C;HL;CXX")
    # BMA TODO: Possibly tell find_packages the min version?
    if(HDF5_VERSION VERSION_LESS "1.10.4")
      message( SEND_ERROR "Found HDF5 version ${HDF5_VERSION}. Minimum required is 1.10.4")
    endif()
  endif() # DAKOTA_HAVE_HDF5
endmacro()

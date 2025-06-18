# Dakota CMake helpers to find system TPLs
# (These are initially macros in case of unexpected dependencies on variables they set.)

include(DakotaAppleBoostLibs)

macro(dakota_find_boost)

  if(WIN32)
    # BMA TODO: Relax this and document
    set(Boost_USE_STATIC_LIBS TRUE)
  endif()

  # Dakota requires the specified compiled Boost library components
  # Dakota requires Boost 1.69 or newer (this can possibly be relaxed to 1.58
  # if not building unit tests); enforce for all libs in the build
  set(dakota_boost_libs filesystem program_options regex serialization system)
  # Remove filesystem after moving to Boost 1.70+
  #set(dakota_boost_libs program_options regex serialization system)

  if(DAKOTA_APPLE_FIX_BOOSTLIBS)
    # This approach requires separate include and lib dirs
    # Macro publishes _dakota_boost_includedir
    dakota_fix_boost_dylibs("${dakota_boost_libs}"
      "${CMAKE_CURRENT_BINARY_DIR}/boost_libs")
    set(BOOST_ROOT)
    set(BOOST_INCLUDEDIR "${_dakota_boost_includedir}")
    set(BOOST_LIBRARYDIR "${CMAKE_CURRENT_BINARY_DIR}/boost_libs")
  endif()

  # std::unary_function and std::binary_function were removed from the C++17 standard library
  # but are used by default in Boost until version 1.80.0
  if(CMAKE_CXX_STANDARD GREATER_EQUAL 17)
    # Removes use of std::unary_function in boost/container_hash/hash.hpp      
    add_compile_definitions(BOOST_NO_CXX98_FUNCTION_BASE)

    # Removes use of std::unary_function and std::binary_function in boost/functional.hpp
    add_compile_definitions(_HAS_AUTO_PTR_ETC=0)
  endif()


  find_package(Boost 1.69 REQUIRED COMPONENTS ${dakota_boost_libs})

  # Remove filesystem after moving to Boost 1.70+
  #set(DAKOTA_BOOST_TARGETS Boost::boost Boost::program_options
  set(DAKOTA_BOOST_TARGETS Boost::boost Boost::filesystem Boost::program_options
    Boost::regex Boost::serialization Boost::system)

  set(dakota_boost_libs "${dakota_boost_libs}" CACHE STRING "")
  set(dakota_boost_version "${Boost_VERSION_MAJOR}.${Boost_VERSION_MINOR}.${Boost_VERSION_PATCH}" CACHE STRING "")

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

macro(dakota_find_googletest)
  # For now, always build googletest in packages/external
  if(WIN32)
  # from the googletest docs
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  endif()
  set(INSTALL_GTEST OFF CACHE BOOL "No need to install googletest")
  add_subdirectory(packages/external/googletest)
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


# Unconditionally find Eigen3, first on system, falling back on Dakota's
# Can override with Eigen3_DIR and/or Eigen3_ROOT as needed
#
# NOTE: If this search order doesn't suffice, can consider additional
# CMake variables to prefer Dakota's over system Eigen
macro(dakota_find_eigen3)
  find_package(Eigen3 3.3 QUIET)
  if(NOT Eigen3_FOUND)
    find_package(Eigen3 3.3 REQUIRED
      HINTS "${Dakota_SOURCE_DIR}/packages/external/eigen3/share/eigen3/cmake")
    # Only install if using our Eigen
    install(DIRECTORY
      "${Dakota_SOURCE_DIR}/packages/external/eigen3/include/eigen3"
      DESTINATION include
      )
    install(DIRECTORY
      "${Dakota_SOURCE_DIR}/packages/external/eigen3/share/eigen3"
      DESTINATION share
      )
  endif()
  get_target_property(DAKOTA_EIGEN3_INCLUDE_DIR Eigen3::Eigen
    INTERFACE_INCLUDE_DIRECTORIES)
  message(STATUS "Dakota using Eigen3::Eigen target with include directories\n"
    "   ${DAKOTA_EIGEN3_INCLUDE_DIR}")
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
    find_package(GSL REQUIRED)
    if(DAKOTA_CBLAS_LIBS)
      message(STATUS "Using GSL library: ${GSL_LIBRARY} with\n"
	"DAKOTA_CBLAS_LIBS = ${DAKOTA_CBLAS_LIBS}")
    else()
      message(STATUS "Using GSL libraries: ${GSL_LIBRARIES}")
    endif()
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

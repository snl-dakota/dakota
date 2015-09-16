# CMakeLists.txt for Dakota and TPLs it depends on
cmake_minimum_required(VERSION 2.8.11)

project("SuperBuildTPLs" C CXX Fortran)

include(ExternalProject)

set( ExtSourceRoot
    ${CMAKE_CURRENT_SOURCE_DIR}/.. )

set( EXT_COMMON_CONFIG_ARGS
    -D CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -D CMAKE_C_COMPILER=${CMAKE_C_COMPILER} 
    -D CMAKE_Fortran_COMPILER=${CMAKE_Fortran_COMPILER} 
    -D BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
    -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  )

set( ProjNameForExtTrilinos TrilinosExt )

set( TrilinosVersionNum "12.2.1" CACHE STRING
  "Override default Trilinos version number used to build Teuchos from distro" )

set( TrilinosPkgFormat "zip" CACHE STRING
  "Override default Trilinos source package file format" )

set( TrilinosPkgName trilinos-${TrilinosVersionNum}-Source.${TrilinosPkgFormat} )

set( TRILINOS_TPL_INSTALL_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/${ProjNameForExtTrilinos}-prefix/install )

externalproject_add(
  ${ProjNameForExtTrilinos}
  URL ${ExtSourceRoot}/packages/${TrilinosPkgName}
  URL_MD5 a0cf5edee8940a1279f751139e2e181f
  CMAKE_ARGS ${EXT_COMMON_CONFIG_ARGS}
    -D Trilinos_SKIP_FORTRANCINTERFACE_VERIFY_TEST=ON
    -D Trilinos_ENABLE_Teuchos=ON
    -D Trilinos_ENABLE_Kokkos=OFF
    -D Trilinos_ENABLE_TeuchosKokkosCompat=OFF
    -D Trilinos_ENABLE_TeuchosKokkosComm=OFF
    -D Trilinos_ENABLE_CXX11=OFF
    -D CMAKE_INSTALL_PREFIX=${TRILINOS_TPL_INSTALL_DIR}
  )


set( ProjNameForExtDak DakotaExt )
option( TPL_HAVE_MPI   "HAVE MPI?"   OFF )
option( TPL_HAVE_GSL   "HAVE GSL?"   OFF )
option( TPL_HAVE_QUESO "HAVE QUESO?" OFF )

set( DAKOTA_TPL_INSTALL_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/${ProjNameForExtDak}-prefix/install )

#    -D MPI_CXX_COMPILER=$MPIBIN
externalproject_add(
  ${ProjNameForExtDak}
  SOURCE_DIR ${ExtSourceRoot}
  DEPENDS ${ProjNameForExtTrilinos}
  CMAKE_ARGS ${EXT_COMMON_CONFIG_ARGS}
    -D DAKOTA_HAVE_MPI=${TPL_HAVE_MPI}
    -D BOOST_INCLUDEDIR=${BOOST_INCLUDEDIR}
    -D BOOST_LIBRARYDIR=${BOOST_LIBRARYDIR}
    -D Boost_NO_SYSTEM_PATHS=ON
    -D BUILD_IN_TRILINOS=OFF
    -D CMAKE_PREFIX_PATH=${TRILINOS_TPL_INSTALL_DIR}
    -D Trilinos_DIR=${TRILINOS_TPL_INSTALL_DIR}
    -D DAKOTA_HAVE_GSL=${TPL_HAVE_GSL}
    -D HAVE_QUESO=${TPL_HAVE_QUESO}
    -D CMAKE_INSTALL_PREFIX=${DAKOTA_TPL_INSTALL_DIR}
  )


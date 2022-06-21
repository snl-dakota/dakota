# CMakeLists.txt for Dakota and TPLs it depends on
cmake_minimum_required(VERSION 2.8.11)

project("SuperBuildTPLs" C CXX Fortran)

include(ExternalProject)

set( ExtSourceRoot
    ${CMAKE_CURRENT_SOURCE_DIR}/.. )

configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/SuperBuildTemplate.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/SuperBuild.cmake" @ONLY
  )

# Not sure this variable makes sense, given the current strategy
# of setting common_build vars in a file, pulled-in via '-C <file>'
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

# bz2 might be something to consider for the "final product" (but verify 1st)
# Eventually, allow cmake option to override the means of obtaining Trilinos distro
set( TrilinosPkgFormat "zip" CACHE STRING
  "Override default Trilinos source package file format" )

set( TrilinosPkgName trilinos-${TrilinosVersionNum}-Source.${TrilinosPkgFormat} )

set( TrilinosTplInstallDir
    ${CMAKE_CURRENT_BINARY_DIR}/${ProjNameForExtTrilinos}-prefix/install )

# WJB:  might prefer to allow the warning for the package verification step
# (the URL_MD5 specified will only work with trilinos-12.2.1-Source.zip)
externalproject_add(
  ${ProjNameForExtTrilinos}
  URL ${ExtSourceRoot}/packages/${TrilinosPkgName}
  URL_MD5 a0cf5edee8940a1279f751139e2e181f
  CMAKE_ARGS ${EXT_COMMON_CONFIG_ARGS}
    -C ${CMAKE_CURRENT_BINARY_DIR}/SuperBuild.cmake
    -D Trilinos_SKIP_FORTRANCINTERFACE_VERIFY_TEST=ON
    -D Trilinos_ENABLE_Teuchos=ON
    -D Trilinos_ENABLE_Kokkos=OFF
    -D Trilinos_ENABLE_TeuchosKokkosCompat=OFF
    -D Trilinos_ENABLE_TeuchosKokkosComm=OFF
    -D Trilinos_ENABLE_CXX11=OFF
    -D CMAKE_INSTALL_PREFIX=${TrilinosTplInstallDir}
  )


set( ProjNameForExtDak DakotaExt )

externalproject_add(
  ${ProjNameForExtDak}
  SOURCE_DIR ${ExtSourceRoot}
  DEPENDS ${ProjNameForExtTrilinos}
  CMAKE_ARGS ${EXT_COMMON_CONFIG_ARGS}
    -D BUILD_IN_TRILINOS=OFF
    -D CMAKE_PREFIX_PATH=${TrilinosTplInstallDir}
    -D Trilinos_DIR=${TrilinosTplInstallDir}
    -C ${CMAKE_CURRENT_BINARY_DIR}/SuperBuild.cmake
  )


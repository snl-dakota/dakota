# CMakeLists.txt for Dakota src

include_directories(
  ${Dakota_BINARY_DIR} ${Dakota_BINARY_DIR}/src
  ${Dakota_SOURCE_DIR}/src ${Dakota_SOURCE_DIR}/packages
  ${Boost_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${Teuchos_INCLUDE_DIRS}
  )

add_definitions("-DHAVE_CONFIG_H")
add_definitions("-DDISABLE_DAKOTA_CONFIG_H")
add_definitions("-DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION")

if(${CMAKE_CXX_COMPILER_ID} MATCHES "XL" OR
   ${CMAKE_CXX_COMPILER_ID} MATCHES "VisualAge")
  add_definitions("-DBOOST_MPL_CFG_ASSERT_USE_RELATION_NAMES")
endif()

# DAKOTA-specific processing of specification maintenance options
include(DakotaEnableSpecMaint)
DakotaEnableSpecMaint()

set(TPL_LIBS nidr)
set(EXTRA_TPL_LIBS "")
# WJB - ToDo:  convince myself that dakota_src_fortran should have a
#              better name and be specified in the initial EXPORTS_TARGETS set
set(EXPORT_TARGETS dakota_src nidr)

link_directories(${Teuchos_LIBRARY_DIRS})
list(APPEND TPL_LIBS ${Teuchos_LIBRARIES})
list(APPEND EXPORT_TARGETS ${Teuchos_LIBRARIES})

option(DAKOTA_PLUGIN "Toggle Plugin support, default is PluginEnabled" ON)
if(DAKOTA_PLUGIN)
  add_definitions("-DDAKOTA_PLUGIN")
endif(DAKOTA_PLUGIN)

option(BOOST_DISABLE_ASSERTS "Toggle BoostAssert, default is AssertDisabled" ON)
if (BOOST_DISABLE_ASSERTS)
  add_definitions("-DBOOST_DISABLE_ASSERTS")
endif (BOOST_DISABLE_ASSERTS)

option(DAKOTA_HAVE_BOOST_FS "Toggle FS support, default is BoostFS enabled" ON)
if (DAKOTA_HAVE_BOOST_FS)
  add_definitions("-DDAKOTA_HAVE_BOOST_FS")

  # Boost Version info (to determine default Boost.Filesystem Version support)
  file(STRINGS ${Boost_INCLUDE_DIR}/boost/version.hpp BOOST_VERSIONSTR
       REGEX "#define[ ]+BOOST_VERSION[ ]+[0-9]+")

  if(BOOST_VERSIONSTR)
    math(EXPR BOOST_VN_MAJOR "${BOOST_VERSIONSTR} / 100000")
    math(EXPR BOOST_VN_MINOR "${BOOST_VERSIONSTR} / 100 % 1000")
    math(EXPR BOOST_VN_SUBMINOR "${BOOST_VERSIONSTR} % 100")
    set(DAK_BOOST_VN "${BOOST_VN_MAJOR}.${BOOST_VN_MINOR}.${BOOST_VN_SUBMINOR}")
    message(STATUS "Boost version: ${DAK_BOOST_VN}")

    if(NOT ${BOOST_VN_MINOR} VERSION_LESS "46")
      #add_definitions("-DBOOST_FILESYSTEM_VERSION=3")
      #set(DAKOTA_HAVE_BOOST_FS OFF)
      message(STATUS "DAKOTA using Boost.FilesystemV3")
    endif()

  else()
    message(FATAL_ERROR 
      "Problem with Boost version from ${Boost_INCLUDE_DIR}/boost/version.hpp")
  endif(BOOST_VERSIONSTR)

endif (DAKOTA_HAVE_BOOST_FS)


include(CheckIncludeFile)

check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
  add_definitions("-DHAVE_UNISTD_H")
endif(HAVE_UNISTD_H)

check_function_exists(system HAVE_SYSTEM)
if(HAVE_SYSTEM)
  add_definitions("-DHAVE_SYSTEM")
endif(HAVE_SYSTEM)

check_function_exists(fork HAVE_FORK)
if(HAVE_FORK)
  set(HAVE_WORKING_FORK ${HAVE_FORK})
  add_definitions("-DHAVE_WORKING_FORK")
  check_function_exists(vfork HAVE_VFORK)
  if(HAVE_VFORK)
    set(HAVE_WORKING_VFORK ${HAVE_VFORK})
    add_definitions("-DHAVE_WORKING_VFORK")
  endif(HAVE_VFORK)
endif(HAVE_FORK)

check_function_exists(wait HAVE_SYS_WAIT_H)
if(HAVE_SYS_WAIT_H)
  add_definitions("-DHAVE_SYS_WAIT_H")
endif(HAVE_SYS_WAIT_H)

check_function_exists(usleep HAVE_USLEEP)
if(HAVE_USLEEP)
  add_definitions("-DHAVE_USLEEP")
endif(HAVE_USLEEP)

check_include_file(pdb.h HAVE_PDB_H)
if(HAVE_PDB_H)
  add_definitions("-DHAVE_PDB_H")
endif(HAVE_PDB_H)

if(UNIX)
  #check_include_files("rpc/types.h" HAVE_RPC_TYPES_H)
  check_include_file("rpc/xdr.h" HAVE_RPC_XDR_H)
  find_program(RPCGEN rpcgen)

  if(HAVE_RPC_XDR_H)
    #message("  Found XDR header files.")
    set(HAVE_RPC_XDR_H ON)
    add_definitions("-DHAVE_RPC_XDR_H -DHAVE_RPC_TYPES_H")
  else()
    message(WARNING "** XDR bindings NOT properly enabled; continuing anyway..")
    add_definitions("-DNO_XDR")
  endif()
endif(UNIX)


if(DAKOTA_F90)
  # A future version of CMake should set the Fortran FIXED_FLAG automagically
  if(${CMAKE_Fortran_COMPILER_ID} MATCHES "Intel")
    set(FIXED_FLAG "-fixed")
  elseif(${CMAKE_Fortran_COMPILER_ID} MATCHES "GNU")
    set(FIXED_FLAG "-ffixed-form")
  elseif(${CMAKE_Fortran_COMPILER_ID} MATCHES "PGI")
    set(FIXED_FLAG "-Mfixed")
  elseif(${CMAKE_Fortran_COMPILER_ID} MATCHES "XL" OR
         ${CMAKE_Fortran_COMPILER_ID} MATCHES "VisualAge")
    set(FIXED_FLAG "-qfixed")
  elseif(${CMAKE_Fortran_COMPILER_ID} MATCHES "SunPro")
    set(FIXED_FLAG "-fixed")
  endif()

  add_definitions("-DDAKOTA_F90")
  include_directories(${Dakota_BINARY_DIR})

  #list(APPEND CMAKE_Fortran_FLAGS ${FIXED_FLAG})
  set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} ${FIXED_FLAG}")
endif(DAKOTA_F90)


# WJB - ToDo: Improve logic to support WinDLL case
option(DAKOTA_DL_SOLVER
  "Toggle DAKOTA DL Solvers, default is disabled." OFF
  )
if(DAKOTA_DL_SOLVER)
  check_include_file(dlfcn.h DAKOTA_HAVE_DLFCN_H)

  if(DAKOTA_HAVE_DLFCN_H)
    # WJB - ToDo:  Test/debug on cygwin ("lgdi32" vs "" ?)
    message(STATUS "DAKOTA DL Solver enabled, LibDL = ${CMAKE_DL_LIBS}")
    add_definitions("-DDAKOTA_DL_SOLVER")
    add_definitions("-DDAKOTA_HAVE_DLFCN_H")
    list(APPEND EXTRA_TPL_LIBS ${CMAKE_DL_LIBS})
  else()
    message(WARNING "DL solver requested, but dlfcn.h not found; disabling.")
    add_definitions("-DNO_NIDR_DYNLIB")
    set(DAKOTA_DL_SOLVER OFF CACHE BOOL 
      "DAKOTA DL solver is disabled due to no dlfcn.h" FORCE)
  endif(DAKOTA_HAVE_DLFCN_H)
endif(DAKOTA_DL_SOLVER)

option(DAKOTA_USAGE_TRACKING
  "Toggle DAKOTA usage tracking, default is disabled." OFF
  )
set(TRACKING_LOCATION_AND_PROXY "" CACHE STRING 
  "<URL;proxy> for usage tracking"
  )
if(DAKOTA_USAGE_TRACKING)
  find_package(CURL)
  if(CURL_FOUND)
    message(STATUS "DAKOTA usage tracking enabled with <URL;proxy> = <${TRACKING_LOCATION_AND_PROXY}>")
    add_definitions("-DDAKOTA_USAGE_TRACKING=\"${TRACKING_LOCATION_AND_PROXY}\"")
    list(APPEND EXTRA_TPL_LIBS ${CURL_LIBRARIES})
  else()
    message(WARNING "Tracking requested, but curl not found; disabling.")
    set(DAKOTA_USAGE_TRACKING OFF CACHE BOOL 
      "Toggle DAKOTA usage tracking, default is disabled." FORCE)
  endif(CURL_FOUND)
endif(DAKOTA_USAGE_TRACKING)

option(DAKOTA_MATLAB "Toggle Matlab interface, default is disabled" OFF)
if(DAKOTA_MATLAB)
  include(FindMatlab)

  if(MATLAB_FOUND)
    message(STATUS "Using MATLAB headers in: ${MATLAB_INCLUDE_DIR}")
    message(STATUS "Using MATLAB libraries in: ${MATLAB_LIBRARIES}")
    include_directories(${MATLAB_INCLUDE_DIR})
    add_definitions("-DDAKOTA_MATLAB")
    list(APPEND EXTRA_TPL_LIBS ${MATLAB_LIBRARIES})
  else() # MATLAB NOT FOUND
    message(SEND_ERROR "MATLAB requested, but not found in ${MATLAB_ROOT}; consider setting MATLAB_DIR")
  endif(MATLAB_FOUND)
endif(DAKOTA_MATLAB)

# TODO: consider rework of Scilab options more like original patch,
# separating whether to build from whether we have?
# SCILAB provides its own BLAS/LAPACK.  Should probably use those instead.
option(DAKOTA_SCILAB "Toggle Scilab interface, default is disabled" OFF)
if(DAKOTA_SCILAB)
  find_package(Scilab)
  if (SCILAB_ROOT)

    # Scilab compilation options
    add_definitions("-DDAKOTA_SCILAB")
    include_directories(${SCILAB_INCLUDES_PATH})
      
    # Scilab linking options 
    # TODO: Don't appear to get used anywhere!
    set(SCI_LINK_FLAGS "${SCI_LINK_FLAGS} -Wl,-rpath,${SCILAB_ROOT}/lib/scilab")
    set(SCI_LINK_FLAGS "${SCI_LINK_FLAGS} -Wl,-rpath-link,${SCILAB_ROOT}/lib/scilab")
    set(SCI_LINK_FLAGS "${SCI_LINK_FLAGS} -Wl,-rpath,${SCILAB_ROOT}/lib/thirdparty")
    set(SCI_LINK_FLAGS "${SCI_LINK_FLAGS} -Wl,-rpath-link,${SCILAB_ROOT}/lib/thirdparty")
      
    if (EXISTS ${SCILAB_ROOT}/thirdparty/java/lib/amd64)
      set(SCI_ARCH "amd64")
    else()
      set(SCI_ARCH "i386")
    endif()
      
    set(SCI_LINK_FLAGS
      "${SCI_LINK_FLAGS} -Wl,-rpath,${SCILAB_ROOT}/thirdparty/java/lib/${SCI_ARCH}")
    set(SCI_LINK_FLAGS 
      "${SCI_LINK_FLAGS} -Wl,-rpath-link,${SCILAB_ROOT}/thirdparty/java/lib/${SCI_ARCH}")
    set(SCI_LINK_FLAGS 
      "${SCI_LINK_FLAGS} -Wl,-rpath,${SCILAB_ROOT}/thirdparty/java/lib/${SCI_ARCH}/server")
    set(SCI_LINK_FLAGS 
      "${SCI_LINK_FLAGS} -Wl,-rpath-link,${SCILAB_ROOT}/thirdparty/java/lib/${SCI_ARCH}/")

    list(APPEND EXTRA_TPL_LIBS ${SCILAB_LIBRARY})

  else() # SCILAB_ROOT
    message(SEND_ERROR 
      "Scilab requested, but not found; consider setting SCILAB_USER_PATH")
  endif() # SCILAB_ROOT
endif() #DAKOTA_SCILAB

option(DAKOTA_PYTHON "Toggle Python interface, default is disabled" OFF)
option(DAKOTA_PYTHON_NUMPY
  "Use Python NumPy for data structures, default is disabled" OFF
  )
if(DAKOTA_PYTHON_NUMPY AND NOT DAKOTA_PYTHON)
  message(FATAL_ERROR "DAKOTA_PYTHON_NUMPY option requires DAKOTA_PYTHON")
endif()
if(DAKOTA_PYTHON)
  include(FindPythonInterp)
  include(FindPythonLibs)

  if(PYTHONLIBS_FOUND)
    # Have a valid Python.h and libraries
    message(STATUS "Using Python headers in: ${PYTHON_INCLUDE_DIRS}")
    message(STATUS "Using Python libraries in: ${PYTHON_LIBRARIES}")
    include_directories(${PYTHON_INCLUDE_DIRS})
    add_definitions("-DDAKOTA_PYTHON")
    list(APPEND EXTRA_TPL_LIBS ${PYTHON_LIBRARIES})

    if(DAKOTA_PYTHON_NUMPY)
      include(FindNUMARRAY)
      if(PYTHON_NUMARRAY_FOUND)
	# Have a valid arrayobject.h
	message(STATUS "Using NumPy headers in: ${PYTHON_NUMARRAY_INCLUDE_DIR}")
	include_directories(${PYTHON_NUMARRAY_INCLUDE_DIR})
	add_definitions("-DDAKOTA_PYTHON_NUMPY")
      else()
	message(SEND_ERROR
	  "Python NumPy requested, but headers/libaries not found")
	set(DAKOTA_PYTHON_NUMPY OFF)
      endif(PYTHON_NUMARRAY_FOUND)
    endif()
  else()
    message(SEND_ERROR "Python requested, but Python headers/libraries associated with Python installed in ${PYTHON_EXECUTABLE} not found")
  endif(PYTHONLIBS_FOUND)
endif(DAKOTA_PYTHON)

option(DAKOTA_MODELCENTER "Toggle ModelCenter support, default is disabled" OFF)
if(DAKOTA_MODELCENTER)
  add_definitions("-DDAKOTA_MODELCENTER")
endif(DAKOTA_MODELCENTER)

if(DAKOTA_HAVE_GSL)
  # Ensure only INDIRECT dependency of Dakota/src on GSL
  #add_definitions("-DDAKOTA_HAVE_GSL")

  #message(STATUS "Enable GSL support -- headers: ${GSL_INCLUDE_DIRS}")
  include_directories("${GSL_INCLUDE_DIRS}")

  list(APPEND link_directories "${GSL_LIBRARY_DIRS}")
  list(APPEND EXTRA_TPL_LIBS gsl m)
endif(DAKOTA_HAVE_GSL)

if(DAKOTA_HAVE_MPI)
  add_definitions("-DDAKOTA_HAVE_MPI")
  include_directories(${MPI_INCLUDE_PATH})
  list(APPEND EXTRA_TPL_LIBS ${MPI_LIBRARY} ${MPI_LIBRARIES})
endif(DAKOTA_HAVE_MPI)


# Couple with "external" packages based on user preference

function(CheckPackage package)
  if(HAVE_${package})
    add_definitions("-DHAVE_${package}")
    string(TOLOWER ${package} package_lower)
    set(TPL_LIBS ${TPL_LIBS} ${package_lower} PARENT_SCOPE)
    list(APPEND EXPORT_TARGETS ${package_lower})
  endif()
endfunction(CheckPackage)


CheckPackage(PECOS)
list(APPEND TPL_LIBS ${Pecos_LINK_LIBRARIES})
list(APPEND EXPORT_TARGETS ${Pecos_LINK_LIBRARIES})
if(HAVE_PECOS)
  include_directories(${Pecos_BINARY_DIR} ${Pecos_SOURCE_DIR}/src
                      ${VPISparseGrid_SOURCE_DIR}/src)
else()
  message(FATAL_ERROR
    "DAKOTA cannot currently be configured without PECOS: ${Pecos_SOURCE_DIR}")
endif(HAVE_PECOS)


## DAKOTA library sources.
set(Dakota_src DakotaBuildInfo.cpp)

## UTIL sources.
set(util_src ParallelLibrary.cpp MPIPackBuffer.cpp
    dakota_data_util.cpp dakota_tabular_io.cpp dakota_global_defs.cpp CommandLineHandler.cpp DakotaGraphics.cpp
    DakotaString.cpp DakotaBinStream.cpp SensAnalysisGlobal.cpp WorkdirHelper.cpp
    ResultsManager.cpp ResultsDBAny.cpp
    )

if(HAVE_RPC_XDR_H)
  list(APPEND util_src dak_xdr_interface.c)
endif(HAVE_RPC_XDR_H)

if(DAKOTA_DL_SOLVER)
  list(APPEND util_src DLSolver.cpp)
endif(DAKOTA_DL_SOLVER)

if(DAKOTA_USAGE_TRACKING)
  list(APPEND util_src TrackerHTTP.cpp)
endif(DAKOTA_USAGE_TRACKING)

## EVALDATA sources.
set(evaldata_src DakotaVariables.cpp MixedVariables.cpp RelaxedVariables.cpp
    SharedVariablesData.cpp DakotaActiveSet.cpp DakotaResponse.cpp ParamResponsePair.cpp)

## DB sources.
set(db_src ProblemDescDB.cpp NIDRProblemDescDB.cpp DataStrategy.cpp DataMethod.cpp
    DataModel.cpp DataVariables.cpp DataInterface.cpp DataResponses.cpp 
    dakota_filesystem_utils.cpp)

## Model sources.
set(model_src DakotaModel.cpp SingleModel.cpp RecastModel.cpp NestedModel.cpp
    SurrogateModel.cpp DataFitSurrModel.cpp HierarchSurrModel.cpp
    DiscrepancyCorrection.cpp)

## Constraint sources.
set(constraint_src DakotaConstraints.cpp MixedVarConstraints.cpp
    RelaxedVarConstraints.cpp)

## Interface sources.
set(interface_src DakotaInterface.cpp ApproximationInterface.cpp
    DakotaApproximation.cpp TaylorApproximation.cpp TANA3Approximation.cpp
    GaussProcApproximation.cpp PecosApproximation.cpp ApplicationInterface.cpp
    SysCallApplicInterface.cpp DirectApplicInterface.cpp TestDriverInterface.cpp
    AnalysisCode.cpp SysCallAnalysisCode.cpp CommandShell.cpp)
if(NOT MSVC)
  list(APPEND interface_src ForkApplicInterface.cpp ForkAnalysisCode.cpp)
endif()

CheckPackage(SURFPACK)
if(HAVE_SURFPACK)
  list(APPEND interface_src SurfpackApproximation.cpp)
  include_directories(${Surfpack_BINARY_DIR} ${Surfpack_SOURCE_DIR}/src)
  list(APPEND TPL_LIBS surfpack surfpack_fortran)
endif(HAVE_SURFPACK)

if(DAKOTA_MATLAB)
  list(APPEND interface_src MatlabInterface.cpp)
endif()

if(DAKOTA_PYTHON)
  list(APPEND interface_src PythonInterface.cpp)
endif()

if(DAKOTA_PLUGIN)
  set(interface_src ${interface_src} PluginSerialDirectApplicInterface.cpp
      PluginParallelDirectApplicInterface.cpp)
endif(DAKOTA_PLUGIN)

if(DAKOTA_SCILAB)
  list(APPEND interface_src ScilabInterface.cpp)
endif()

## Strategy sources.
set(strategy_src DakotaStrategy.cpp SingleMethodStrategy.cpp HybridStrategy.cpp
    SequentialHybridStrategy.cpp EmbeddedHybridStrategy.cpp
    CollaborativeHybridStrategy.cpp ConcurrentStrategy.cpp)

## Iterator sources.
set(iterator_src DakotaIterator.cpp DakotaAnalyzer.cpp DakotaPStudyDACE.cpp
    ParamStudy.cpp DakotaVerification.cpp RichExtrapVerification.cpp DakotaNonD.cpp
    NonDReliability.cpp NonDLocalReliability.cpp NonDGlobalReliability.cpp
    NonDInterval.cpp NonDLHSInterval.cpp NonDLHSEvidence.cpp NonDLHSSingleInterval.cpp
    NonDLocalInterval.cpp NonDLocalEvidence.cpp NonDLocalSingleInterval.cpp
    NonDGlobalInterval.cpp NonDGlobalEvidence.cpp NonDGlobalSingleInterval.cpp
    NonDIntegration.cpp NonDCubature.cpp NonDQuadrature.cpp NonDSparseGrid.cpp
    NonDExpansion.cpp NonDPolynomialChaos.cpp NonDStochCollocation.cpp
    NonDCalibration.cpp NonDBayesCalibration.cpp NonDSampling.cpp NonDLHSSampling.cpp
    NonDAdaptImpSampling.cpp NonDGPImpSampling.cpp NonDIncremLHSSampling.cpp
    DakotaMinimizer.cpp DakotaOptimizer.cpp DakotaLeastSq.cpp NonlinearCGOptimizer.cpp 
    SurrBasedMinimizer.cpp SurrBasedLocalMinimizer.cpp SurrBasedGlobalMinimizer.cpp
    EffGlobalMinimizer.cpp
)
set(Dakota_src_fortran lawson_hanson1.f)
if(DAKOTA_F90)
  list(APPEND Dakota_src_fortran lawson_hanson2.f90)
endif(DAKOTA_F90)

if(HAVE_ACRO)
  add_definitions("-DDAKOTA_COLINY")
  add_definitions("-DDAKOTA_UTILIB")

  include_directories(${Acro_BINARY_DIR}/packages
                      ${Acro_BINARY_DIR}/packages/utilib
                      ${Acro_SOURCE_DIR}/packages/interfaces/src
                      ${Acro_SOURCE_DIR}/packages/scolib/src
                      ${Acro_SOURCE_DIR}/packages/utilib/src
                      ${Acro_SOURCE_DIR}/packages/colin/src
                      ${Acro_SOURCE_DIR}/tpl/tinyxml)

  list(APPEND interface_src COLINApplication.cpp)
  list(APPEND iterator_src COLINOptimizer.cpp)

  foreach(acro_dep utilib colin interfaces scolib 3po pebbl tinyxml)
    string(TOUPPER "${acro_dep}" ACRODEP)
    if (HAVE_${ACRODEP})
      list(APPEND TPL_LIBS ${acro_dep})
      list(APPEND EXPORT_TARGETS ${acro_dep})
    endif()
  endforeach()
endif(HAVE_ACRO)

if(HAVE_ADAPTIVE_SAMPLING)
  add_definitions("-DHAVE_ADAPTIVE_SAMPLING")
  list(APPEND iterator_src NonDAdaptiveSampling.cpp)
  if(HAVE_MORSE_SMALE)
    add_definitions("-DHAVE_MORSE_SMALE")
    list(APPEND iterator_src MorseSmaleComplex.cpp)

    # ANN does approximate k-nearest neighbor search
    include_directories(
      ${Dakota_SOURCE_DIR}/packages/ann/include)
    list(APPEND TPL_LIBS ANN)
    list(APPEND EXPORT_TARGETS ANN)

    # Dionysus provides persistence diagram and bottleneck distance
    # The MS_Complex class requires this unconditionally
    add_definitions("-DHAVE_DIONYSUS")
    include_directories(${Dakota_BINARY_DIR}/packages/dionysus_ext-prefix/src/dionysus_ext/include)
  endif()
endif()

option(HAVE_ESM "Whether to enable efficient subspace method" OFF)
if(HAVE_ESM)
  add_definitions("-DHAVE_ESM")
  list(APPEND iterator_src EfficientSubspaceMethod.cpp)
endif()

CheckPackage(QUESO)
if(HAVE_QUESO)
  list(APPEND iterator_src NonDQUESOBayesCalibration.cpp)
  add_definitions("-DDAKOTA_QUESO")

  #include_directories("$ENV{QUESO_PREFIX}/include" "$ENV{GRVY_PREFIX}/include"
  include_directories(${Dakota_BINARY_DIR}/packages/queso_ext/include
                      ${Dakota_BINARY_DIR}/packages/grvy_ext/include
                      ${GSL_INCLUDE_DIRS})

  #list(APPEND link_directories "$ENV{QUESO_PREFIX}/lib" "$ENV{GRVY_PREFIX}/lib"
  list(APPEND link_directories ${Dakota_BINARY_DIR}/packages/queso_ext/lib
       ${Dakota_BINARY_DIR}/packages/grvy_ext/lib)

  list(APPEND TPL_LIBS grvy)
  list(APPEND EXPORT_TARGETS queso grvy)
endif(HAVE_QUESO)


# TODO: conditional on Acro?  
if(Boost_FOUND)
  list(APPEND link_directories ${Boost_LIBRARY_DIRS})
  list(APPEND EXTRA_TPL_LIBS ${Boost_LIBRARIES})
endif()

CheckPackage(CONMIN)
if(HAVE_CONMIN)
  link_directories ( ${link_directories} ${CONMIN_LIB_DIRS} )
  set(iterator_src ${iterator_src} CONMINOptimizer.cpp)
endif(HAVE_CONMIN)

if(HAVE_DDACE)
  add_definitions("-DDAKOTA_DDACE")
  include_directories(${ddace_BINARY_DIR} ${ddace_SOURCE_DIR}/include)
  set(iterator_src ${iterator_src} DDACEDesignCompExp.cpp)
  #link_directories(${Dakota_BINARY_DIR}/methods/DDACE)
  set(TPL_LIBS ${TPL_LIBS} dace analyzer random sampling bose)
  list(APPEND EXPORT_TARGETS dace analyzer random sampling bose)
endif(HAVE_DDACE)

CheckPackage(DOT)
if(HAVE_DOT)
  set(iterator_src ${iterator_src} DOTOptimizer.cpp)
endif(HAVE_DOT)

CheckPackage(FSUDACE)
if(HAVE_FSUDACE)
  include_directories(${FSUDace_BINARY_DIR} ${FSUDace_SOURCE_DIR})
  set(iterator_src ${iterator_src} FSUDesignCompExp.cpp)
endif(HAVE_FSUDACE)

if(HAVE_GPMSA)
  add_definitions("-DDAKOTA_GPMSA")
  add_dependencies(queso_ext grvy_ext)
  list(APPEND iterator_src NonDGPMSABayesCalibration.cpp)
  #include_directories("$ENV{QUESO_PREFIX}/include" "$ENV{GRVY_PREFIX}/include"
  include_directories(${Dakota_BINARY_DIR}/packages/queso_ext/include
                      ${Dakota_BINARY_DIR}/packages/grvy_ext/include
                      ~/dakota/packages/queso_tarballs/code/libs/gpmsa/inc
                      ${GSL_INCLUDE_DIRS})

  #list(APPEND link_directories "$ENV{QUESO_PREFIX}/lib" "$ENV{GRVY_PREFIX}/lib"
  list(APPEND link_directories ${Dakota_BINARY_DIR}/packages/queso_ext/lib
       ${Dakota_BINARY_DIR}/packages/grvy_ext/lib
       ~/dakota/packages/queso_tarballs/code/libs/gpmsa/lib)
  #link_directories(${Dakota_BINARY_DIR}/packages/gpmsa)
  list(APPEND TPL_LIBS queso grvy gpmsa_any)
  list(APPEND EXPORT_TARGETS queso grvy gpmsa_any)
endif(HAVE_GPMSA)

if(HAVE_HOPSPACK)
  #add_definitions("-DHAVE_HOPSPACK") # WJB: use convention?
  add_definitions("-DDAKOTA_HOPS")
  include_directories(${HOPSPACK_BINARY_DIR}/src/src-shared
                      ${HOPSPACK_SOURCE_DIR}/src/src-shared
                      ${HOPSPACK_SOURCE_DIR}/src/src-citizens
                      ${HOPSPACK_SOURCE_DIR}/src/src-evaluator
                      ${HOPSPACK_SOURCE_DIR}/src/src-executor
                      ${HOPSPACK_SOURCE_DIR}/src/src-framework
                      ${HOPSPACK_SOURCE_DIR}/src/src-main)
  list(APPEND iterator_src APPSOptimizer.cpp APPSEvalMgr.cpp)
  #link_directories(${Dakota_BINARY_DIR}/packages/hopspack)
  list(APPEND TPL_LIBS hopspack)
  list(APPEND EXPORT_TARGETS hopspack)
endif(HAVE_HOPSPACK)

if(HAVE_JEGA)
  add_definitions("-DHAVE_JEGA")
  include_directories(${JEGA_BINARY_DIR} ${JEGA_BINARY_DIR}/eddy
                      ${JEGA_SOURCE_DIR}/eddy ${JEGA_SOURCE_DIR}/include)
  list(APPEND iterator_src JEGAOptimizer.cpp)
  list(APPEND TPL_LIBS jega jega_fe moga soga eutils utilities)
  list(APPEND EXPORT_TARGETS jega jega_fe moga soga eutils utilities)
endif(HAVE_JEGA)

if(HAVE_NCSUOPT)
  add_definitions("-DHAVE_NCSU")
  set(iterator_src ${iterator_src} NCSUOptimizer.cpp)
  #link_directories(${Dakota_BINARY_DIR}/packages/NCSUOpt)
  list(APPEND TPL_LIBS ncsuopt)
  list(APPEND EXPORT_TARGETS ncsuopt)
endif(HAVE_NCSUOPT)

CheckPackage(NLPQL)
if(HAVE_NLPQL)
  set(iterator_src ${iterator_src} NLPQLPOptimizer.cpp)
endif(HAVE_NLPQL)

if(HAVE_NL2SOL)
  add_definitions("-DHAVE_NL2SOL")
  set(iterator_src ${iterator_src} NL2SOLLeastSq.cpp)
  include_directories(${Dakota_BINARY_DIR}/packages/NP2SOL)
  #link_directories(${Dakota_BINARY_DIR}/packages/NP2SOL)
  list(APPEND TPL_LIBS cport)
  list(APPEND EXPORT_TARGETS cport)
endif(HAVE_NL2SOL)

CheckPackage(NPSOL)
if(HAVE_NPSOL)
  list(APPEND iterator_src SOLBase.cpp NPSOLOptimizer.cpp NLSSOLLeastSq.cpp)

  if(NOT APPLE)
    list(APPEND Dakota_src_fortran npoptn_wrapper.f)
  else()
    # ToDo: Debug unusual Mac build error when npoptn_wrapper.f is appended
    #       to the Dakota_src_fortran list rather than the iterator_src list
    list(APPEND iterator_src npoptn_wrapper.f)
  endif()
endif(HAVE_NPSOL)

if(HAVE_OPTPP)
  include_directories(${optpp_BINARY_DIR}/include ${optpp_SOURCE_DIR}/include)

  add_definitions("-DHAVE_OPTPP")
  # WJB:  OPT++ macro naming "convention" differs from DAKOTA.  Even worse,
  #       Behavior is conditional outside of the Opt++ package due to do inline,
  #       VIRTUAL function definitions which can be enable/disabled "globally".
  # Therefore, define as many build macros as necessary to get a desirable exec.
  add_definitions("-DDAKOTA_OPTPP")

  set(iterator_src ${iterator_src} SNLLBase.cpp SNLLOptimizer.cpp SNLLLeastSq.cpp)
  set(TPL_LIBS ${TPL_LIBS} optpp)
  list(APPEND EXPORT_TARGETS optpp)
endif(HAVE_OPTPP)

CheckPackage(PSUADE)
if(HAVE_PSUADE)
  list(APPEND interface_src PSUADEDesignCompExp.cpp)
  include_directories(${PSUADE_SOURCE_DIR})
endif(HAVE_PSUADE)

if(HAVE_X_GRAPHICS)
  add_definitions("-DHAVE_X_GRAPHICS")
  include_directories(${DakotaMotif_SOURCE_DIR}/include ${X11_INCLUDE_DIR} 
    ${MOTIF_INCLUDE_DIR})
  #set(iterator_src ${iterator_src} PSUADEDesignCompExp.cpp)
  link_directories(${DakotaMotif_BINARY_DIR})
  list(APPEND TPL_LIBS DGraphics)
  list(APPEND EXPORT_TARGETS DGraphics)

  list(APPEND EXTRA_TPL_LIBS ${X11_LIBRARIES} ${MOTIF_LIBRARIES})
  if (X11_Xpm_FOUND)
    list(APPEND EXTRA_TPL_LIBS ${X11_Xpm_LIB})
  endif()
  # cmake-2.8.4 doesn't set a helper variable X11_Xmu_LIB
  if (LIBXMU)
    list(APPEND EXTRA_TPL_LIBS ${LIBXMU})
  endif()
  if (X11_Xt_FOUND)
    list(APPEND EXTRA_TPL_LIBS ${X11_Xt_LIB})
  endif()
  if (Threads_FOUND)
    list(APPEND EXTRA_TPL_LIBS ${CMAKE_THREAD_LIBS_INIT})
  endif()
endif(HAVE_X_GRAPHICS)

## AGGREGATED source code spec for full-up libdakota.
set(Dakota_src ${Dakota_src} ${util_src} ${evaldata_src} ${db_src}
    ${model_src} ${constraint_src} ${interface_src} ${strategy_src}
    ${iterator_src})

if(MSVC)
  list(APPEND Dakota_src dirent.h dirent.c)
endif()

add_library(dakota_src_fortran ${Dakota_src_fortran})
# the intel fortran compiler can  not handle all the -I stuff
# that is coming from the C++ files, but it does not need any of it
# so clear out the includes for this target
set_target_properties(dakota_src_fortran PROPERTIES INCLUDE_DIRECTORIES "")
add_library(dakota_src ${Dakota_src})
target_link_libraries(dakota_src dakota_src_fortran)
list(APPEND EXPORT_TARGETS dakota_src_fortran)


if(HAVE_ACRO)
  target_link_libraries(dakota_src utilib colin interfaces scolib)
endif()

if(ENABLE_SPEC_MAINT)
  # force dependency on generated files in source tree
  add_dependencies(dakota_src dakota-spec-files)
endif()

## MORE TPLs to append to the link stream, conditionally, based on user prefs

if(HAVE_QUESO)
  # Enforce GRVY/QUESO config header creation prior to compiling dakota sources
  add_dependencies(dakota_src queso_ext grvy_ext)
endif()

if(HAVE_AMPL)
  add_definitions("-DHAVE_AMPL")

  # Enforce AMPL config header creation prior to compiling dakota sources
  add_dependencies(dakota_src amplsolver)

  #include_directories(${Ampl_SOURCE_DIR}/ampl)
  include_directories(${Dakota_BINARY_DIR}/packages/ampl)

  #link_directories(${Dakota_BINARY_DIR}/packages/ampl)
  set(TPL_LIBS ${TPL_LIBS} amplsolver)
  list(APPEND EXPORT_TARGETS amplsolver)
endif(HAVE_AMPL)

#if(DAKOTA_RESULTS_DB)
#  set(HAVE_YAML-CPP ON CACHE BOOL 
#      "Results DB requires yaml-cpp" FORCE)
#  CheckPackage(YAML-CPP)
#  include_directories(${YAML_CPP_SOURCE_DIR}/include)
#endif()

if(EXISTS ${Dakota_SOURCE_DIR}/.svn)

  # SVN checkout, determine revision level

  if((APPLE AND DEFINED ENV{BUILD_URL}) OR (CYGWIN AND DEFINED ENV{BUILD_URL}))
    # Workarounds for Jenkins slave shell where svn is different version
    # than the svn client that performed the 'update' step

    file(STRINGS ${Dakota_SOURCE_DIR}/.svn/entries
      list_dakota_rev LIMIT_COUNT 4)

    # Get the trailing (or 4th) entry (SVN rev of Dakota)
    # Would also want to silence CMP0007, or remove blanks
    list(GET list_dakota_rev -1 Dakota_SVN_REV)

  else()
    # NOT CYGWIN/jenkins so "assert" that the same svn client is used for
    # 'update' and 'configure' steps
    find_package(Subversion)

    if(Subversion_FOUND)
      # Workarounds for Trilinos, where CMAKE_PROJECT_NAME != Dakota
      # and the Dakota/ directory may be symlinked.
      get_filename_component(abs_source_dir ${Dakota_SOURCE_DIR} REALPATH)
      Subversion_WC_INFO(${abs_source_dir} Dakota)

      set(Dakota_SVN_REV "${Dakota_WC_REVISION}")
    endif(Subversion_FOUND) # Subversion_FOUND
  endif()

  message("Trunk Dakota_SVN_REV is currently at: ${Dakota_SVN_REV}")

  file( WRITE ${Dakota_BINARY_DIR}/src/DakotaBuildInfo.cpp
        "// -- AUTO GENERATED FILE; DO NOT EDIT --
        #include \"DakotaBuildInfo.hpp\"
        std::string Dakota::DakotaBuildInfo::rev=\"${Dakota_SVN_REV}\"; 
        // ToDo: Gather SVN rev for key TPLs, e.g. pecos, surfpack, etc.
        //       Lower priority - a post-5.3 release task
        " )

  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/DakotaBuildInfo.cpp)
    message(STATUS
      "Found a generated file: ${CMAKE_CURRENT_SOURCE_DIR}/DakotaBuildInfo.cpp in the source tree; please remove!")
  endif() # incorrect location DakotaBuildInfo.cpp

else() # source tree NOT an SVN checkout

  # Higher-level Jenkins/PACKAGING job COPIES DakotaBuildInfo.cpp into source tree
  message(STATUS "EXPECTED - DakotaBuildInfo.cpp found in the src tree.")

endif() # .svn directory conditional


if(MSVC)
  # Create "Native Windows dirent" source files in the binary tree so they
  # will only be seen by Native Windows builds.

  configure_file(${Dakota_SOURCE_DIR}/cmake/win_dirent.c.in
                 ${Dakota_BINARY_DIR}/src/dirent.c COPYONLY)

  configure_file(${Dakota_SOURCE_DIR}/cmake/win_dirent.h.in
                 ${Dakota_BINARY_DIR}/src/dirent.h COPYONLY)
endif()

## DAKOTA executables.
add_executable(dakota main.cpp)
add_executable(dakota_restart_util restart_util.cpp)
add_executable(dakota_library_mode library_mode.cpp)
  
list(APPEND EXTRA_TPL_LIBS ${LAPACK_LIBS} ${BLAS_LIBS})

if(DAKOTA_SCILAB)
  set_target_properties(dakota PROPERTIES LINK_FLAGS "${SCI_LINK_FLAGS}")
  set_target_properties(dakota_restart_util PROPERTIES LINK_FLAGS "${SCI_LINK_FLAGS}")
  set_target_properties(dakota_library_mode PROPERTIES LINK_FLAGS "${SCI_LINK_FLAGS}")
endif()

target_link_libraries(dakota dakota_src ${TPL_LIBS} ${EXTRA_TPL_LIBS})
target_link_libraries(dakota_restart_util dakota_src ${TPL_LIBS} ${EXTRA_TPL_LIBS})
target_link_libraries(dakota_library_mode dakota_src ${TPL_LIBS} ${EXTRA_TPL_LIBS})

#include(InstallRequiredSystemLibraries)
#include(GetPrerequisites)
#list_prerequisites(${CMAKE_CURRENT_BINARY_DIR}/dakota)

install(TARGETS dakota_src_fortran EXPORT ${ExportTarget} DESTINATION lib)
install(TARGETS dakota_src EXPORT ${ExportTarget} DESTINATION lib)
install(TARGETS dakota dakota_restart_util EXPORT DESTINATION bin)
install(TARGETS dakota_library_mode DESTINATION bin)
file(GLOB Dakota_hdr *.hpp *.h)
install(FILES ${Dakota_hdr} DESTINATION include)
file(GLOB Dakota_spec_files dakota.input.*)
install(FILES ${Dakota_spec_files} DESTINATION bin)

if (DAKOTA_HAVE_MPI)
  add_executable(dakota_library_split library_split.cpp)
  target_link_libraries(dakota_library_split
    dakota_src ${TPL_LIBS} ${EXTRA_TPL_LIBS})
  install(TARGETS dakota_library_split DESTINATION bin)
endif()

if (DAKOTA_DLL_API)
  add_library(dll_api "akota_dll_api.cpp)
  target_link_libraries(dll_api dakota_src ${TPL_LIBS} ${EXTRA_TPL_LIBS})
  install(TARGETS dll_api DESTINATION lib)
endif()

# Install dyanamic deps, omitting standard system libs
# CMake variables aren't passed to the script, so we have to invoke
# separate scripts for each platform
option(DAKOTA_INSTALL_DYNAMIC_DEPS 
  "Install Dakota's dynamic dependencies to CMAKE_INSTALL_PREFIX/bin/" ON)
if (DAKOTA_INSTALL_DYNAMIC_DEPS)
  if (CYGWIN)
    install(SCRIPT "${Dakota_SOURCE_DIR}/cmake/InstallCygwinDLLs.cmake")
  elseif (APPLE)
    install(SCRIPT "${Dakota_SOURCE_DIR}/cmake/InstallDarwinDylibs.cmake")
  elseif (UNIX)
#    include(InstallRequiredSystemLibraries)
    install(SCRIPT "${Dakota_SOURCE_DIR}/cmake/InstallUnixSOs.cmake")
  endif()
endif()
  
if(Teuchos_TPL_LIST MATCHES "LAPACK;BLAS")
  message(STATUS "Building against pre-installed Teuchos WITH own lapack: ${Teuchos_TPL_LIST}")
endif()

message(STATUS "TPL LIBS: ${TPL_LIBS}")
message(STATUS "EXTRA TPL LIBS: ${EXTRA_TPL_LIBS}")

# Pass some data back to Trilinos
if (BUILD_IN_TRILINOS)
  get_directory_property(Dakota_INC_DIRS INCLUDE_DIRECTORIES)
  set(Dakota_INCLUDE_DIRS ${Dakota_INC_DIRS} CACHE INTERNAL 
    "Dakota include directories")
  set(Dakota_LIBS dakota_src dakota_src_fortran ${TPL_LIBS})
  # Only remove the Teuchos libraries from the Dakota_LIBS variable
  list(REMOVE_ITEM Dakota_LIBS ${Teuchos_LIBRARIES})
  set(Dakota_LIBRARIES ${Dakota_LIBS} CACHE INTERNAL 
    "Dakota libraries")
  set(Dakota_EXTRA_LIBRARIES ${EXTRA_TPL_LIBS} CACHE INTERNAL 
    "Dakota extra libraries")
  message(STATUS 
    "Export targets to <export-name> = ${ExportTarget}: ${EXPORT_TARGETS}"
    )
  export(TARGETS ${EXPORT_TARGETS} FILE
    "${CMAKE_CURRENT_BINARY_DIR}/DakotaTargets.cmake")
endif()


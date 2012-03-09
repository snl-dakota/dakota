# CMakeLists.txt for Dakota src

include_directories(
  ${Dakota_BINARY_DIR} ${Dakota_BINARY_DIR}/src
  ${Dakota_SOURCE_DIR}/src ${Dakota_SOURCE_DIR}/packages
  ${Boost_INCLUDE_DIRS} ${Teuchos_INCLUDE_DIRS}
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
      message(WARNING 
        "DAKOTA will NOT support Boost.FilesystemV3 (externally) until Rel5.3")
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


option(HAVE_USAGE_TRACKING "Build the DAKOTA usage tracker module." OFF)
set(TRACKING_LOCATION_AND_PROXY "" CACHE STRING 
    "<URL;proxy> for usage tracking")
if(HAVE_USAGE_TRACKING)
  if(CURL_FOUND)
    add_definitions("-DHAVE_USAGE_TRACKING=\"${TRACKING_LOCATION_AND_PROXY}\"")
  else(CURL_FOUND)
    message(WARNING "Tracking requested, but curl not found; disabling.")
    set(HAVE_USAGE_TRACKING OFF FORCE)
  endif(CURL_FOUND)
endif(HAVE_USAGE_TRACKING)


set(TPL_LIBS teuchos nidr)
set(EXTRA_TPL_LIBS "")
set(EXPORT_TARGETS dakota_src teuchos nidr)

if(DAKOTA_HAVE_MPI)
  add_definitions("-DDAKOTA_HAVE_MPI")
  list(APPEND EXTRA_TPL_LIBS ${MPI_LIBRARIES})
endif(DAKOTA_HAVE_MPI)

link_directories(${Teuchos_LIBRARY_DIRS})


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
endif(HAVE_PECOS)


## DAKOTA library sources.
set(Dakota_src DakotaBuildInfo.C)

## UTIL sources.
set(util_src ParallelLibrary.C MPIPackBuffer.C
    data_util.C tabular_io.C global_defs.C CommandLineHandler.C DakotaGraphics.C
    DakotaString.C DakotaBinStream.C SensAnalysisGlobal.C WorkdirHelper.C)

if(HAVE_RPC_XDR_H)
  list(APPEND util_src dak_xdr_interface.c)
endif(HAVE_RPC_XDR_H)

if(HAVE_USAGE_TRACKING)
  list(APPEND util_src TrackerHTTP.C)
endif(HAVE_USAGE_TRACKING)

## EVALDATA sources.
set(evaldata_src DakotaVariables.C MergedVariables.C MixedVariables.C
    SharedVariablesData.C DakotaActiveSet.C DakotaResponse.C ParamResponsePair.C)

## DB sources.
set(db_src ProblemDescDB.C NIDRProblemDescDB.C DataStrategy.C DataMethod.C
    DataModel.C DataVariables.C DataInterface.C DataResponses.C 
    filesystem_utils.C)

## Model sources.
set(model_src DakotaModel.C SingleModel.C RecastModel.C NestedModel.C
    SurrogateModel.C DataFitSurrModel.C HierarchSurrModel.C
    DiscrepancyCorrection.C)

## Constraint sources.
set(constraint_src DakotaConstraints.C MergedConstraints.C MixedConstraints.C)

## Interface sources.
set(interface_src DakotaInterface.C ApproximationInterface.C
    DakotaApproximation.C TaylorApproximation.C TANA3Approximation.C
    GaussProcApproximation.C PecosApproximation.C ApplicationInterface.C
    SysCallApplicInterface.C ForkApplicInterface.C DirectApplicInterface.C
    AnalysisCode.C ForkAnalysisCode.C SysCallAnalysisCode.C CommandShell.C)

CheckPackage(SURFPACK)
if(HAVE_SURFPACK)
  list(APPEND interface_src SurfpackApproximation.C)
  include_directories(${Surfpack_BINARY_DIR} ${Surfpack_SOURCE_DIR}/src)
  list(APPEND TPL_LIBS surfpack)
endif(HAVE_SURFPACK)

if(DAKOTA_PLUGIN)
  set(interface_src ${interface_src} PluginSerialDirectApplicInterface.C
      PluginParallelDirectApplicInterface.C)
endif(DAKOTA_PLUGIN)

## Strategy sources.
set(strategy_src DakotaStrategy.C SingleMethodStrategy.C HybridStrategy.C
    SequentialHybridStrategy.C EmbeddedHybridStrategy.C
    CollaborativeHybridStrategy.C ConcurrentStrategy.C)

## Iterator sources.
set(iterator_src DakotaIterator.C DakotaAnalyzer.C DakotaPStudyDACE.C
    ParamStudy.C DakotaVerification.C RichExtrapVerification.C DakotaNonD.C
    NonDReliability.C NonDLocalReliability.C NonDGlobalReliability.C
    NonDInterval.C NonDLHSInterval.C NonDLHSEvidence.C NonDLHSSingleInterval.C
    NonDLocalInterval.C NonDLocalEvidence.C NonDLocalSingleInterval.C
    NonDGlobalInterval.C NonDGlobalEvidence.C NonDGlobalSingleInterval.C
    NonDIntegration.C NonDCubature.C NonDQuadrature.C NonDSparseGrid.C
    NonDExpansion.C NonDPolynomialChaos.C NonDStochCollocation.C
    NonDCalibration.C NonDBayesCalibration.C NonDSampling.C NonDLHSSampling.C
    NonDAdaptImpSampling.C NonDGPImpSampling.C NonDIncremLHSSampling.C
    DakotaMinimizer.C DakotaOptimizer.C DakotaLeastSq.C NonlinearCGOptimizer.C 
    SurrBasedMinimizer.C SurrBasedLocalMinimizer.C SurrBasedGlobalMinimizer.C
    lawson_hanson1.f
    EffGlobalMinimizer.C
)

if(DAKOTA_F90)
  list(APPEND iterator_src lawson_hanson2.f90)
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

  list(APPEND interface_src COLINApplication.C)
  list(APPEND iterator_src COLINOptimizer.C)

  foreach(acro_dep utilib colin interfaces scolib 3po pebbl tinyxml)
    string(TOUPPER "${acro_dep}" ACRODEP)
    if (HAVE_${ACRODEP})
      list(APPEND TPL_LIBS ${acro_dep})
      list(APPEND EXPORT_TARGETS ${acro_dep})
    endif()
  endforeach()
endif(HAVE_ACRO)


CheckPackage(QUESO)
if(HAVE_QUESO)
  list(APPEND iterator_src NonDQUESOBayesCalibration.C)
  add_definitions("-DDAKOTA_QUESO")

  exec_program("$ENV{GSL_CONFIG}" ARGS --prefix OUTPUT_VARIABLE GSL_PREFIX)
  message(STATUS "GSL prefix: ${GSL_PREFIX}")

  # QUESO depends on additional Trilinos packages (NOT just teuchos)
  include(${Trilinos_DIR}/TrilinosConfig_install.cmake)

  #include_directories("$ENV{QUESO_PREFIX}/include" "$ENV{GRVY_PREFIX}/include"
  include_directories(${Dakota_BINARY_DIR}/packages/queso_ext/include
                      ${Dakota_BINARY_DIR}/packages/grvy_ext/include
                      ${GSL_PREFIX}/include
                      ${Trilinos_INCLUDE_DIRS})

  #list(APPEND link_directories "$ENV{QUESO_PREFIX}/lib" "$ENV{GRVY_PREFIX}/lib"
  list(APPEND link_directories ${Dakota_BINARY_DIR}/packages/queso_ext/lib
       ${Dakota_BINARY_DIR}/packages/grvy_ext/lib
       ${Trilinos_LIBRARY_DIRS})

  list(APPEND TPL_LIBS grvy ${Trilinos_LIBRARIES})
  list(APPEND EXPORT_TARGETS queso grvy ${Trilinos_LIBRARIES})
endif(HAVE_QUESO)


# TODO: conditional on Acro?  
if(Boost_FOUND)
  list(APPEND link_directories ${Boost_LIBRARY_DIRS})
  list(APPEND EXTRA_TPL_LIBS ${Boost_LIBRARIES})
endif()

CheckPackage(CONMIN)
if(HAVE_CONMIN)
  link_directories ( ${link_directories} ${CONMIN_LIB_DIRS} )
  set(iterator_src ${iterator_src} CONMINOptimizer.C)
endif(HAVE_CONMIN)

if(HAVE_DDACE)
  add_definitions("-DDAKOTA_DDACE")
  include_directories(${ddace_BINARY_DIR} ${ddace_SOURCE_DIR}/include)
  set(iterator_src ${iterator_src} DDACEDesignCompExp.C)
  #link_directories(${Dakota_BINARY_DIR}/methods/DDACE)
  set(TPL_LIBS ${TPL_LIBS} dace analyzer random sampling bose)
  list(APPEND EXPORT_TARGETS dace analyzer random sampling bose)
endif(HAVE_DDACE)

CheckPackage(DOT)
if(HAVE_DOT)
  set(iterator_src ${iterator_src} DOTOptimizer.C)
endif(HAVE_DOT)

CheckPackage(FSUDACE)
if(HAVE_FSUDACE)
  include_directories(${FSUDace_BINARY_DIR} ${FSUDace_SOURCE_DIR})
  set(iterator_src ${iterator_src} FSUDesignCompExp.C)
endif(HAVE_FSUDACE)

if(HAVE_GPMSA)
  add_definitions("-DDAKOTA_GPMSA")
  include_directories(${GPMSA_SOURCE_DIR})
  list(APPEND iterator_src NonDGPMSABayesCalibration.C)
  #link_directories(${Dakota_BINARY_DIR}/packages/gpmsa)
  list(APPEND TPL_LIBS gpmsa)
  list(APPEND EXPORT_TARGETS gpmsa)
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
  list(APPEND iterator_src APPSOptimizer.C APPSEvalMgr.C)
  #link_directories(${Dakota_BINARY_DIR}/packages/hopspack)
  list(APPEND TPL_LIBS hopspack)
  list(APPEND EXPORT_TARGETS hopspack)
endif(HAVE_HOPSPACK)

if(HAVE_JEGA)
  add_definitions("-DHAVE_JEGA")
  include_directories(${JEGA_BINARY_DIR} ${JEGA_BINARY_DIR}/eddy
                      ${JEGA_SOURCE_DIR}/eddy ${JEGA_SOURCE_DIR}/include)
  list(APPEND iterator_src JEGAOptimizer.C)
  #link_directories(${Dakota_BINARY_DIR}/packages/JEGA)
  # WJB: "jega_link_libs: looks fishy; confer with Zack about JEGA commit 692
  list(APPEND TPL_LIBS jega jega_fe moga soga eutils utilities)
  list(APPEND EXPORT_TARGETS jega jega_fe moga soga eutils utilities)
endif(HAVE_JEGA)

if(HAVE_NCSUOPT)
  add_definitions("-DHAVE_NCSU")
  set(iterator_src ${iterator_src} NCSUOptimizer.C)
  #link_directories(${Dakota_BINARY_DIR}/packages/NCSUOpt)
  list(APPEND TPL_LIBS ncsuopt)
  list(APPEND EXPORT_TARGETS ncsuopt)
endif(HAVE_NCSUOPT)

CheckPackage(NLPQL)
if(HAVE_NLPQL)
  set(iterator_src ${iterator_src} NLPQLPOptimizer.C)
endif(HAVE_NLPQL)

if(HAVE_NL2SOL)
  add_definitions("-DHAVE_NL2SOL")
  set(iterator_src ${iterator_src} NL2SOLLeastSq.C)
  include_directories(${Dakota_BINARY_DIR}/packages/NP2SOL)
  #link_directories(${Dakota_BINARY_DIR}/packages/NP2SOL)
  list(APPEND TPL_LIBS cport)
  list(APPEND EXPORT_TARGETS cport)
endif(HAVE_NL2SOL)

CheckPackage(NPSOL)
if(HAVE_NPSOL)
  set(iterator_src ${iterator_src} SOLBase.C NPSOLOptimizer.C NLSSOLLeastSq.C
      npoptn_wrapper.f)
endif(HAVE_NPSOL)

if(HAVE_OPTPP)
  # WJB - ToDo: FY11 contract - "pair-program" a FINAL iteration with Zack
  include_directories(${optpp_BINARY_DIR}/include ${optpp_SOURCE_DIR}/include)

  add_definitions("-DHAVE_OPTPP")
  # WJB:  OPT++ macro naming "convention" differs from DAKOTA.  Even worse,
  #       Behavior is conditional outside of the Opt++ package due to do inline,
  #       VIRTUAL function definitions which can be enable/disabled "globally".
  # Therefore, define as many build macros as necessary to get a desirable exec.
  add_definitions("-DDAKOTA_OPTPP")

  set(iterator_src ${iterator_src} SNLLBase.C SNLLOptimizer.C SNLLLeastSq.C)
  set(TPL_LIBS ${TPL_LIBS} optpp)
  list(APPEND EXPORT_TARGETS optpp)
endif(HAVE_OPTPP)

CheckPackage(PSUADE)
if(HAVE_PSUADE)
  list(APPEND interface_src PSUADEDesignCompExp.C)
  include_directories(${PSUADE_SOURCE_DIR})
endif(HAVE_PSUADE)

if(HAVE_X_GRAPHICS)
  add_definitions("-DHAVE_X_GRAPHICS")
  include_directories(${DakotaMotif_SOURCE_DIR}/include ${X11_INCLUDE_DIR} 
    ${MOTIF_INCLUDE_DIR})
  #set(iterator_src ${iterator_src} PSUADEDesignCompExp.C)
  link_directories(${DakotaMotif_BINARY_DIR})
  list(APPEND TPL_LIBS DGraphics)
  list(APPEND EXPORT_TARGETS DGraphics)

  list(APPEND EXTRA_TPL_LIBS ${X11_LIBRARIES} ${MOTIF_LIBRARIES})
  if (X11_Xpm_FOUND)
    list(APPEND EXTRA_TPL_LIBS ${X11_Xpm_LIB})
  endif()
  if (LIBXMU)
    list(APPEND EXTRA_TPL_LIBS -lXmu)
  endif()
  if (Threads_FOUND)
    list(APPEND EXTRA_TPL_LIBS ${CMAKE_THREAD_LIBS_INIT})
  endif()
endif(HAVE_X_GRAPHICS)

## AGGREGATED source code spec for full-up libdakota.
set(Dakota_src ${Dakota_src} ${util_src} ${evaldata_src} ${db_src}
    ${model_src} ${constraint_src} ${interface_src} ${strategy_src}
    ${iterator_src})

add_library(dakota_src ${Dakota_src})

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


file( WRITE ${Dakota_BINARY_DIR}/src/DakotaBuildInfo.C
      "// -- AUTO GENERATED FILE; DO NOT EDIT --
      #include \"DakotaBuildInfo.H\"
      std::string Dakota::DakotaBuildInfo::rev =" )

file(READ ${Dakota_SOURCE_DIR}/config/DakotaBuildInfo.C.in build-info-contents)
string(REGEX MATCH "Dakota\\^([0-9]+)" notused ${build-info-contents})
file( APPEND ${Dakota_BINARY_DIR}/src/DakotaBuildInfo.C
      " \"${CMAKE_MATCH_1}\"; " )

file( APPEND ${Dakota_BINARY_DIR}/src/DakotaBuildInfo.C
"
// -- WJB: CMake version needs to replace sed/awk for .build-info creation
" )


## DAKOTA executables.
add_executable(dakota main.C)
add_executable(dakota_restart_util restart_util.C)
add_executable(dakota_library_mode library_mode.C)
  
## WJB: request macro from Kitware? FindLibDL.cmake (e.g. digidoc++ or casacore)
list(APPEND EXTRA_TPL_LIBS ${LAPACK_LIBS} ${CURL_LIBRARIES} dl)

target_link_libraries(dakota dakota_src ${TPL_LIBS} ${EXTRA_TPL_LIBS})
target_link_libraries(dakota_restart_util dakota_src ${TPL_LIBS} ${EXTRA_TPL_LIBS})
target_link_libraries(dakota_library_mode dakota_src ${TPL_LIBS} ${EXTRA_TPL_LIBS})

install(TARGETS dakota_src EXPORT ${ExportTarget} DESTINATION lib)
install(TARGETS dakota dakota_restart_util EXPORT DESTINATION bin)
install(TARGETS dakota_library_mode DESTINATION bin)
file(GLOB Dakota_hdr *.H *.h)
install(FILES ${Dakota_hdr} DESTINATION include)
file(GLOB Dakota_spec_files dakota.input.*)
install(FILES ${Dakota_spec_files} DESTINATION bin)

if (DAKOTA_HAVE_MPI)
  add_executable(dakota_library_split library_split.C)
  install(TARGETS dakota_library_split DESTINATION bin)
  target_link_libraries(dakota_library_split
    dakota_src ${TPL_LIBS} ${EXTRA_TPL_LIBS})
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
set(Dakota_LIBS dakota_src ${TPL_LIBS})
# Remove teuchos from Dakota_LIBS
list(REMOVE_ITEM Dakota_LIBS teuchos)

set(Dakota_LIBRARIES ${Dakota_LIBS} CACHE INTERNAL 
    "Dakota libraries")
set(Dakota_EXTRA_LIBRARIES ${EXTRA_TPL_LIBS} CACHE INTERNAL 
    "Dakota extra libraries")
message(STATUS "Export targets: ${EXPORT_TARGETS}")
export(TARGETS ${EXPORT_TARGETS} FILE
       "${CMAKE_CURRENT_BINARY_DIR}/DakotaTargets.cmake")
endif()

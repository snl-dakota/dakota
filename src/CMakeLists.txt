include_directories(${Dakota_BINARY_DIR} ${Dakota_SOURCE_DIR}/src
                    ${NIDR_SOURCE_DIR}/..
                    ${Boost_INCLUDE_DIRS}
                    ${Teuchos_INCLUDE_DIRS})

add_definitions("-DHAVE_CONFIG_H")
add_definitions("-DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION")

option(BOOST_DISABLE_ASSERTS "Toggle BoostAssert, default is AssertDisabled" ON)
if (BOOST_DISABLE_ASSERTS)
  add_definitions("-DBOOST_DISABLE_ASSERTS")
endif (BOOST_DISABLE_ASSERTS)


# Check already done in Teuchos.  Setting the macro again leads to redef warning
check_include_files(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
  add_definitions("-DHAVE_UNISTD_H")
endif(HAVE_UNISTD_H)

check_function_exists(system HAVE_SYSTEM)
if(HAVE_SYSTEM)
  add_definitions("-DHAVE_SYSTEM")
endif(HAVE_SYSTEM)

set(TPL_LIBS teuchos nidr)

if(HAVE_MPI)
  list(APPEND TPL_LIBS ${MPI_LIBRARIES})
endif(HAVE_MPI)

link_directories(${Teuchos_LIBRARY_DIRS})


# Couple with "external" packages based on user preference

function(CheckPackage package)
  if(HAVE_${package})
    add_definitions("-DHAVE_${package}")
    string(TOLOWER ${package} package_lower)
    set(TPL_LIBS ${TPL_LIBS} ${package_lower} PARENT_SCOPE)
  endif()
endfunction(CheckPackage)

CheckPackage(PECOS)

if(HAVE_PECOS)
  # WJB - ToDo: Change M4 and source files to new convention, i.e. -DHAVE_PECOS
  add_definitions("-DDAKOTA_PECOS")

  # WJB: really need Pecos_INCLUDE_DIRS for dakota
  include_directories(${Pecos_BINARY_DIR} ${Pecos_SOURCE_DIR}/src
                      ${VPISparseGrid_SOURCE_DIR}/src)
endif(HAVE_PECOS)

## DAKOTA library sources.
set(Dakota_src DakotaBuildInfo.C)

## UTIL sources.
set(util_src ParallelLibrary.C MPIPackBuffer.C
    data_util.C global_defs.C CommandLineHandler.C DakotaGraphics.C
    DakotaString.C DakotaBinStream.C CtelRegExp.C regexp.c
    SensAnalysisGlobal.C)

if(HAVE_XDR)
  list(APPEND util_src dak_xdr_interface.c)
endif(HAVE_XDR)

if(HAVE_TRACKING)
  list(APPEND util_src TrackerHTTP.C)
  #WJB set(util_src ${util_src} TrackerHTTP.C)
endif(HAVE_TRACKING)

## EVALDATA sources.
set(evaldata_src DakotaVariables.C MergedVariables.C MixedVariables.C
    SharedVariablesData.C DakotaActiveSet.C DakotaResponse.C ParamResponsePair.C)

## DB sources.
set(db_src ProblemDescDB.C NIDRProblemDescDB.C DataStrategy.C DataMethod.C
    DataModel.C DataVariables.C DataInterface.C DataResponses.C 
    filesystem_utils.C)

## Model sources.
set(model_src DakotaModel.C SingleModel.C RecastModel.C NestedModel.C
    SurrogateModel.C DataFitSurrModel.C HierarchSurrModel.C)

## Constraint sources.
set(constraint_src DakotaConstraints.C MergedConstraints.C MixedConstraints.C)

## Interface sources.
set(interface_src DakotaInterface.C ApproximationInterface.C
    DakotaApproximation.C TaylorApproximation.C TANA3Approximation.C
    GaussProcApproximation.C PecosApproximation.C ApplicationInterface.C
    SysCallApplicInterface.C ForkApplicInterface.C DirectApplicInterface.C
    AnalysisCode.C ForkAnalysisCode.C SysCallAnalysisCode.C CommandShell.C)

if(HAVE_SURFPACK)
  set(interface_src ${interface_src} SurfpackApproximation.C)
endif(HAVE_SURFPACK)

if(HAVE_PLUGIN)
  set(interface_src ${interface_src} PluginSerialDirectApplicInterface.C
      PluginParallelDirectApplicInterface.C)
endif(HAVE_PLUGIN)

## Strategy sources.
set(strategy_src DakotaStrategy.C SingleMethodStrategy.C HybridStrategy.C
    SequentialHybridStrategy.C EmbeddedHybridStrategy.C
    CollaborativeHybridStrategy.C ConcurrentStrategy.C)

## Iterator sources.
set(iterator_src DakotaIterator.C DakotaAnalyzer.C DakotaPStudyDACE.C
    ParamStudy.C DakotaVerification.C RichExtrapVerification.C DakotaNonD.C
    NonDReliability.C NonDLocalReliability.C NonDGlobalReliability.C
    NonDInterval.C NonDLHSInterval.C NonDLHSEvidence.C
    NonDLHSSingleInterval.C NonDLocalInterval.C NonDLocalEvidence.C
    NonDLocalSingleInterval.C NonDGlobalInterval.C NonDGlobalEvidence.C
    NonDGlobalSingleInterval.C NonDIntegration.C NonDCubature.C
    NonDQuadrature.C NonDSparseGrid.C NonDSampling.C NonDAdaptImpSampling.C
    NonDExpansion.C NonDPolynomialChaos.C NonDStochCollocation.C
    NonDLHSSampling.C NonDIncremLHSSampling.C
    DakotaMinimizer.C DakotaOptimizer.C DakotaLeastSq.C NonlinearCGOptimizer.C 
    SurrBasedMinimizer.C SurrBasedLocalMinimizer.C SurrBasedGlobalMinimizer.C
    lawson_hanson1.f
    EffGlobalMinimizer.C
)

if(HAVE_ACRO)
  add_definitions("-DDAKOTA_COLINY")
  #include_directories(${Acro_BINARY_DIR} ${Acro_SOURCE_DIR}/include
  #                    ${Acro_BINARY_DIR}/src ${Acro_SOURCE_DIR}/src)
  include_directories(${Acro_BINARY_DIR}/include
                      ${Acro_BINARY_DIR}/packages/colin/src
                      ${Acro_BINARY_DIR}/packages/interfaces/src
                      ${Acro_BINARY_DIR}/scolib/interfaces/src)
  list(APPEND interface_src COLINApplication.C COLINEvaluator.C)
  list(APPEND iterator_src COLINOptimizer.C)
  #link_directories(${Dakota_BINARY_DIR}/packages/acro)
  #list(APPEND TPL_LIBS colin tinyxml 3po)
endif(HAVE_ACRO)

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
  set(TPL_LIBS ${TPL_LIBS} dace)
endif(HAVE_DDACE)

CheckPackage(DOT)
if(HAVE_DOT)
  add_definitions("-DDAKOTA_DOT")
  set(iterator_src ${iterator_src} DOTOptimizer.C)
endif(HAVE_DOT)

CheckPackage(FSUDACE)
if(HAVE_FSUDACE)
  add_definitions("-DDAKOTA_FSUDACE")
  include_directories(${FSUDace_BINARY_DIR} ${FSUDace_SOURCE_DIR})
  set(iterator_src ${iterator_src} FSUDesignCompExp.C)
endif(HAVE_FSUDACE)

if(HAVE_GPMSA)
  add_definitions("-DDAKOTA_GPMSA")
  include_directories(${GPMSA_SOURCE_DIR})
  set(iterator_src ${iterator_src} NonDBayesCal.C)
  #link_directories(${Dakota_BINARY_DIR}/packages/GPMSA)
  # WJB: verify no longer HEADER-ONLY! set(TPL_LIBS ${TPL_LIBS} gpmsa)
endif(HAVE_GPMSA)

if(HAVE_JEGA)
  add_definitions("-DDAKOTA_JEGA")
  include_directories(${JEGA_BINARY_DIR} ${JEGA_SOURCE_DIR}/include
                      ${JEGA_SOURCE_DIR}/src ${JEGA_SOURCE_DIR}/eddy
                      ${JEGA_BINARY_DIR}/src ${JEGA_BINARY_DIR}/eddy)
  set(iterator_src ${iterator_src} JEGAOptimizer.C)
  #link_directories(${Dakota_BINARY_DIR}/packages/JEGA)
  # WJB: "jega_link_libs: looks fishy; confer with Zack about JEGA commit 692
  set(TPL_LIBS ${TPL_LIBS} jega jega_fe moga soga eutils)
endif(HAVE_JEGA)

if(HAVE_NCSUOPT)
  add_definitions("-DDAKOTA_NCSU")
  set(iterator_src ${iterator_src} NCSUOptimizer.C)
  #link_directories(${Dakota_BINARY_DIR}/packages/NCSUOpt)
  set(TPL_LIBS ${TPL_LIBS} ncsuopt)
endif(HAVE_NCSUOPT)

if(HAVE_NLPQL)
  add_definitions("-DDAKOTA_NLPQL")
  set(iterator_src ${iterator_src} NLPQLPOptimizer.C)
  #link_directories(${Dakota_BINARY_DIR}/packages/NLPQL)
  set(TPL_LIBS ${TPL_LIBS} nlpql)
endif(HAVE_NLPQL)

if(HAVE_NL2SOL)
  add_definitions("-DDAKOTA_NL2SOL")
  set(iterator_src ${iterator_src} NL2SOLLeastSq.C)
  include_directories(${Dakota_BINARY_DIR}/packages/NP2SOL)
  #link_directories(${Dakota_BINARY_DIR}/packages/NP2SOL)
  set(TPL_LIBS ${TPL_LIBS} cport)
endif(HAVE_NL2SOL)

CheckPackage(NPSOL)
if(HAVE_NPSOL)
  add_definitions("-DDAKOTA_NPSOL")
  set(iterator_src ${iterator_src} SOLBase.C NPSOLOptimizer.C NLSSOLLeastSq.C
      npoptn_wrapper.f)
endif(HAVE_NPSOL)

if(HAVE_OPTPP)
  # WJB - ToDo: FY11 contract - "pair-program" a FINAL iteration with Zack
  include_directories(${optpp_BINARY_DIR}/config ${optpp_SOURCE_DIR}/include
                      ${optpp_SOURCE_DIR}/newmat11)
  #include_directories(${PROJECT_BINARY_DIR}/packages/OPTPP/include
  #                    ${PROJECT_SOURCE_DIR}/packages/OPTPP/include
  #                    ${PROJECT_SOURCE_DIR}/packages/OPTPP/newmat11)

  add_definitions("-DDAKOTA_OPTPP")
  add_definitions("-DDAKOTA_NEWMAT")
  set(iterator_src ${iterator_src} SNLLBase.C SNLLOptimizer.C SNLLLeastSq.C)
  set(TPL_LIBS ${TPL_LIBS} opt newmat)
endif(HAVE_OPTPP)

if(HAVE_PSUADE)
  add_definitions("-DHAVE_PSUADE")
  include_directories(${PSUADE_SOURCE_DIR})
  set(iterator_src ${iterator_src} PSUADEDesignCompExp.C)
  #link_directories(${Dakota_BINARY_DIR}/packages/PSUADE)
  set(TPL_LIBS ${TPL_LIBS} psuade)
endif(HAVE_PSUADE)


CheckPackage(SURFPACK)
if(HAVE_SURFPACK)
  include_directories(${Surfpack_BINARY_DIR} ${Surfpack_SOURCE_DIR}/src
                      ${Surfpack_SOURCE_DIR}/src/surfaces)
  set(TPL_LIBS ${TPL_LIBS} mars surfaces)
endif(HAVE_SURFPACK)


## AGGREGATED source code spec for full-up libdakota.
set(Dakota_src ${Dakota_src} ${util_src} ${evaldata_src} ${db_src}
    ${model_src} ${constraint_src} ${interface_src} ${strategy_src}
    ${iterator_src})


## WJB: can the library AND executable target have the same name? add_library(dakota ${Dakota_src})
add_library(dakota_src SHARED ${Dakota_src})
add_library(dakota_src-static STATIC ${Dakota_src})

## Ensure both the shared and static libraries have the same root name
set_target_properties(dakota_src-static PROPERTIES OUTPUT_NAME "dakota_src")
set_target_properties(dakota_src-static PROPERTIES PREFIX "lib")


## MORE TPLs to append to the link stream, conditionally, based on user prefs
if(HAVE_AMPL)
  add_definitions("-DHAVE_AMPL")

  # WJB - ToDo: ask Zack about whether this is correct??
  # (i.e. enforce AMPL config header creation prior to compiling dakota sources)
  add_dependencies(dakota_src amplsolver)

  #include_directories(${Ampl_SOURCE_DIR}/ampl)
  include_directories(${Dakota_BINARY_DIR}/packages/ampl)

  #link_directories(${Dakota_BINARY_DIR}/packages/ampl)
  set(TPL_LIBS ${TPL_LIBS} amplsolver)
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
#target_link_libraries(dakota dakota_src ${TPL_LIBS} ${CURL_LIBRARIES} ${LAPACK_LIBS} ${BLAS_LIBS} ${FLIBS} ${FCLIBS})

## WJB: request macro from Kitware? FindLibDL.cmake (e.g. digidoc++)
target_link_libraries(dakota dakota_src ${TPL_LIBS} ${LAPACK_LIBS}
                      ${CURL_LIBRARIES} dl)

if(Teuchos_TPL_LIST MATCHES "LAPACK;BLAS")
  message(STATUS "Building against pre-installed Teuchos WITH own lapack: ${Teuchos_TPL_LIST}")
endif()

message(STATUS "TPL LIBS: ${TPL_LIBS}")


# CMakeLists.txt for Dakota src

# --- Initialize variables for Makefile export

# No variable for #defines as no need to separately treat Dakota and TPLs

# All source/build tree include directories (including ExternalProject)
set(DAKOTA_INCDIRS)
# All external TPL include directories
set(EXT_TPL_INCDIRS)

# All source/build build tree library directories
set(DAKOTA_LIBDIRS)
# All external TPL library directories
set(EXT_TPL_LIBDIRS)

# Dakota core libraries 
set(DAKOTA_LIBS) 
# Libraries from CMake-built packages from Dakota or its parent 
set(DAKOTA_PKG_LIBS) 
# Libraries from packages built with ExternalProject_add within Dakota
set(DAKOTA_EXTPROJ_LIBS) 
# External third-party libraries installed on the system
set(EXT_TPL_LIBS)

# QUESO libraries have to be managed at end of link due to conflicts
# with gslcblas (QUESO package built with ExternalProject_add within Dakota)
set(QUESO_EXTPROJ_LIBS) 
# QUESO external LIBS to be linked last
set(QUESO_TPL_LIBS)

list(APPEND DAKOTA_INCDIRS 
  ${Dakota_BINARY_DIR}/generated/src
  # Consider removing these two paths and moving to generated...
  ${Dakota_BINARY_DIR}
  ${Dakota_BINARY_DIR}/src
  ${Dakota_SOURCE_DIR}/src ${Dakota_SOURCE_DIR}/packages
  )

list(APPEND EXT_TPL_INCDIRS
  ${Boost_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})

add_definitions("-DHAVE_CONFIG_H")
add_definitions("-DDISABLE_DAKOTA_CONFIG_H")

# Input specification maintenance options
if (ENABLE_SPEC_MAINT OR ENABLE_DAKOTA_DOCS)
  # Build Java utilities needed for XML (dakota.xml) to NIDR 
  # (dakota.input.nspec) translation and/or reference manual generation.
  add_subdirectory(java_utils)
endif()
include(DakotaSpecMaint)
if(ENABLE_SPEC_MAINT)
  # dakota.xml to dakota.input.nspec is conditional as it requires Java
  DakotaXml2Nspec()
endif()
DakotaNidrgen()

# Use TPL_LIBS for libraries built as part of this project
list(APPEND DAKOTA_PKG_LIBS nidr)

set(EXPORT_TARGETS dakota_src nidr)

# We don't want to export Teuchos libs unless we're building in our
# tree.  However, when building under Trilinos, targets like pecos_src
# and dakota_src need to see these libraries in the exports list at
# this point as Trilinos hasn't exported them yet.
if (BUILD_IN_TRILINOS OR DAKOTA_BUILDING_TEUCHOS)
  list(APPEND DAKOTA_INCDIRS ${Teuchos_INCLUDE_DIRS})
  list(APPEND DAKOTA_LIBDIRS ${Teuchos_LIBRARY_DIRS})
  list(APPEND EXPORT_TARGETS ${Teuchos_LIBRARIES})
  list(APPEND DAKOTA_PKG_LIBS ${Teuchos_LIBRARIES})
  # As of 20141219, Teuchos optionally depends on Kokkos, which may be
  # on if building in Trilinos; try to detect all Teuchos deps.
  if (Teuchos_FULL_ENABLED_DEP_PACKAGES)
    foreach(dep_pkg ${Teuchos_FULL_ENABLED_DEP_PACKAGES})
      # Add the libraries of all non-Teuchos packages
      if(NOT ${dep_pkg} MATCHES "^Teuchos")
	if (${dep_pkg}_LIBRARIES)
	  list(APPEND EXPORT_TARGETS ${${dep_pkg}_LIBRARIES})
	  list(APPEND DAKOTA_PKG_LIBS ${${dep_pkg}_LIBRARIES})
	  message(STATUS 
	    "Adding Teuchos dependency ${dep_pkg}_LIBRARIES to Dakota libraries")
	endif()
      endif()
    endforeach()
  else()
    # conservative fall-through for currently known Teuchos deps
    if(KokkosCore_LIBRARIES)
      list(APPEND EXPORT_TARGETS ${KokkosCore_LIBRARIES})
      list(APPEND DAKOTA_PKG_LIBS ${KokkosCore_LIBRARIES})
      message(STATUS 
	"Adding Teuchos dependency KokkosCore_LIBRARIES to Dakota libraries")
    endif()
    if(KokkosTPL_LIBRARIES)
      list(APPEND EXPORT_TARGETS ${KokkosTPL_LIBRARIES})
      list(APPEND DAKOTA_PKG_LIBS ${KokkosTPL_LIBRARIES})
      message(STATUS
	"Adding Teuchos dependency KokkosTPL_LIBRARIES to Dakota libraries")
    endif()
  endif()
else()
  list(APPEND EXT_TPL_INCDIRS ${Teuchos_INCLUDE_DIRS})
  list(APPEND EXT_TPL_LIBDIRS ${Teuchos_LIBRARY_DIRS})
  list(APPEND EXT_TPL_LIBS ${Teuchos_LIBRARIES})
endif()

#------------------
# Generat DakotaBuildInfo.src if needed

set(DAKOTA_BUILDINFO_PATH)
if(DAKOTA_SVN_checkout OR DAKOTA_GIT_checkout)
  if(DAKOTA_SVN_checkout)
    file( WRITE ${Dakota_BINARY_DIR}/generated/src/DakotaBuildInfo.cpp
      "// -- AUTO GENERATED FILE; DO NOT EDIT --
      #include \"DakotaBuildInfo.hpp\"
      std::string Dakota::DakotaBuildInfo::releaseNum=\"${Dakota_VERSION_SRC}\";
      std::string Dakota::DakotaBuildInfo::revision=\"${Dakota_SVN_REV}\";
      std::string Dakota::DakotaBuildInfo::releaseDate=\"${Dakota_RELEASE_DATE}\";
      // ToDo: Gather SVN rev for key TPLs, e.g. pecos, surfpack, etc.
      //       Lower priority - a post-5.4 release task
      " )
  else()
    file( WRITE ${Dakota_BINARY_DIR}/generated/src/DakotaBuildInfo.cpp
      "// -- AUTO GENERATED FILE; DO NOT EDIT --
      #include \"DakotaBuildInfo.hpp\"
      std::string Dakota::DakotaBuildInfo::releaseNum=\"${Dakota_VERSION_SRC}\";
      std::string Dakota::DakotaBuildInfo::revision=\"${Dakota_GIT_REV}\";
      std::string Dakota::DakotaBuildInfo::releaseDate=\"${Dakota_RELEASE_DATE}\";
      " )
  endif(DAKOTA_SVN_checkout)
  set(DAKOTA_BUILDINFO_PATH "${Dakota_BINARY_DIR}/generated/src/")

  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/DakotaBuildInfo.cpp)
    message(STATUS
      "Found a generated file: ${CMAKE_CURRENT_SOURCE_DIR}/DakotaBuildInfo.cpp in the source tree; please remove!")
  endif() # incorrect location DakotaBuildInfo.cpp

else() # source tree NOT an SVN checkout

  message(STATUS "EXPECTED - DakotaBuildInfo.cpp found in the src tree.")

endif() # .svn directory conditional

# -----------------


# -----------------
# Boost Adjustments

if(${CMAKE_CXX_COMPILER_ID} MATCHES "XL" OR
   ${CMAKE_CXX_COMPILER_ID} MATCHES "VisualAge")
  add_definitions("-DBOOST_MPL_CFG_ASSERT_USE_RELATION_NAMES")
endif()

option(BOOST_DISABLE_ASSERTS "Toggle BoostAssert, default is AssertDisabled" ON)
if (BOOST_DISABLE_ASSERTS)
  add_definitions("-DBOOST_DISABLE_ASSERTS")
endif (BOOST_DISABLE_ASSERTS)

# -----------------


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

check_function_exists(vfork HAVE_VFORK)
if(HAVE_VFORK)
  set(HAVE_WORKING_VFORK ${HAVE_VFORK})
  add_definitions("-DHAVE_WORKING_VFORK")
endif(HAVE_VFORK)

# Override source code defaults for fork vs. vfork.  Currently, vfork
# is the default on all platforms. To override and use fork, set  
# DAKOTA_PREFER_FORK to true. DAKOTA_PREFER_VFORK does nothing and
# is reserved for future functionality.
set(DAKOTA_PREFER_FORK OFF)
set(DAKOTA_PREFER_VFORK OFF)
if(DAKOTA_PREFER_FORK)
  if(DAKOTA_PREFER_VFORK)
    message(FATAL_ERROR
      "Cannot specify both DAKOTA_PREFER_FORK and DAKOTA_PREFER_VFORK")
  endif()
  if (NOT HAVE_FORK)
    message(FATAL_ERROR
      "Explicit request for DAKOTA_PREFER_FORK, but fork not found")
  endif()
  message("Dakota configured to prefer fork over vfork")
  add_definitions("-DDAKOTA_PREFER_FORK")
elseif(DAKOTA_PREFER_VFORK)
  if (NOT HAVE_VFORK)
    message(FATAL_ERROR
      "Explicit request for DAKOTA_PREFER_VFORK, but vfork not found")
  endif()
  message("Dakota configured to prefer vfork over fork")
  add_definitions("-DDAKOTA_PREFER_VFORK")
endif()


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
  #include_directories(${Dakota_BINARY_DIR})

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
    message(STATUS "DAKOTA DL Solver enabled, LibDL = ${CMAKE_DL_LIBS}")
    add_definitions("-DDAKOTA_DL_SOLVER")
    add_definitions("-DDAKOTA_HAVE_DLFCN_H")
    list(APPEND EXT_TPL_LIBS ${CMAKE_DL_LIBS})
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
    message(STATUS "Dakota usage tracking enabled with URL/proxies = <${TRACKING_LOCATION_AND_PROXY}>")
    add_definitions("-DDAKOTA_USAGE_TRACKING=\"${TRACKING_LOCATION_AND_PROXY}\"")
    list(APPEND EXT_TPL_LIBS ${CURL_LIBRARIES})
  else()
    message(WARNING "Tracking requested, but curl not found; disabling.")
    set(DAKOTA_USAGE_TRACKING OFF CACHE BOOL 
      "Toggle Dakota usage tracking, default is disabled." FORCE)
  endif(CURL_FOUND)
endif(DAKOTA_USAGE_TRACKING)

option(DAKOTA_MATLAB "Toggle Matlab interface, default is disabled" OFF)
if(DAKOTA_MATLAB)
  include(FindMatlab)

  if(MATLAB_FOUND)
    message(STATUS "Using MATLAB headers in: ${MATLAB_INCLUDE_DIR}")
    message(STATUS "Using MATLAB libraries in: ${MATLAB_LIBRARIES}")
    list(APPEND EXT_TPL_INCDIRS ${MATLAB_INCLUDE_DIR})
    add_definitions("-DDAKOTA_MATLAB")
    list(APPEND EXT_TPL_LIBS ${MATLAB_LIBRARIES})
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
    list(APPEND EXT_TPL_INCDIRS ${SCILAB_INCLUDES_PATH})
      
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

    list(APPEND EXT_TPL_LIBS ${SCILAB_LIBRARY})

  else() # SCILAB_ROOT
    message(SEND_ERROR 
      "Scilab requested, but not found; consider setting SCILAB_USER_PATH")
  endif() # SCILAB_ROOT
endif() #DAKOTA_SCILAB

option(DAKOTA_PYTHON "Toggle Python interface, default is disabled" OFF)
option(DAKOTA_PYTHON_NUMPY
  "Attempt to link to Python NumPy interface? Default: yes." ON)
if(DAKOTA_PYTHON)
  include(FindPythonLibs)

  if(PYTHONLIBS_FOUND)
    # Have a valid Python.h and libraries
    message(STATUS "Using Python headers in: ${PYTHON_INCLUDE_DIRS}")
    message(STATUS "Using Python libraries in: ${PYTHON_LIBRARIES}")
    list(APPEND EXT_TPL_INCDIRS ${PYTHON_INCLUDE_DIRS})
    add_definitions("-DDAKOTA_PYTHON")
    list(APPEND EXT_TPL_LIBS ${PYTHON_LIBRARIES})

    if(DAKOTA_PYTHON_NUMPY)
      include(FindNUMARRAY)
      if(PYTHON_NUMARRAY_FOUND)
	# Have a valid arrayobject.h
	message(STATUS "Using NumPy headers in: ${PYTHON_NUMARRAY_INCLUDE_DIR}")
	list(APPEND EXT_TPL_INCDIRS ${PYTHON_NUMARRAY_INCLUDE_DIR})
	add_definitions("-DDAKOTA_PYTHON_NUMPY")
      else()
	message("Python NumPy requested, but headers/libaries not found; "
	  "disabling NumPy")
      endif(PYTHON_NUMARRAY_FOUND)
    else()
      message(STATUS "Disabling NumPy interface due to DAKOTA_PYTHON_NUMPY = "
	"${DAKOTA_PYTHON_NUMPY}")
    endif()
  else()
    message(SEND_ERROR 
      "Python requested via DAKOTA_PYTHON, but Python headers/libraries "
      "associated with Python installed with ${PYTHON_EXECUTABLE} not found"
      )
  endif()
endif(DAKOTA_PYTHON)

if(DAKOTA_MODELCENTER)
  add_definitions("-DDAKOTA_MODELCENTER")
  # TODO: generalize the location of this library, possibly install
  # it.  Using TPL as these paths will be needed for include/link if a
  # client is using an installed Dakota.
  list(APPEND EXT_TPL_INCDIRS
    "${Dakota_SOURCE_DIR}/examples/linked_interfaces/ModelCenter/include")
  list(APPEND EXT_TPL_LIBDIRS 
    "${Dakota_SOURCE_DIR}/examples/linked_interfaces/ModelCenter/lib/i686-cygwin-cygwinNT")
  list(APPEND EXT_TPL_LIBS PHXCppApi)
endif(DAKOTA_MODELCENTER)

if(DAKOTA_HAVE_MPI)
  add_definitions("-DDAKOTA_HAVE_MPI")
  list(APPEND EXT_TPL_INCDIRS ${MPI_CXX_INCLUDE_PATH})
  list(APPEND EXT_TPL_LIBS ${MPI_CXX_LIBRARIES})
endif(DAKOTA_HAVE_MPI)

if(DAKOTA_HAVE_HDF5)
  add_definitions("-DDAKOTA_HAVE_HDF5")
  list(APPEND EXT_TPL_INCDIRS ${HDF5_INCLUDE_DIRS})
  list(APPEND EXT_TPL_LIBS ${HDF5_HL_LIBRARY} ${HDF5_LIBRARIES})
endif(DAKOTA_HAVE_HDF5)


# Couple with "external" packages based on user preference

function(CheckPackage package)
  if(HAVE_${package})
    add_definitions("-DHAVE_${package}")
    string(TOLOWER ${package} package_lower)
    set(DAKOTA_PKG_LIBS ${DAKOTA_PKG_LIBS} ${package_lower} PARENT_SCOPE)
    list(APPEND EXPORT_TARGETS ${package_lower})
  endif()
endfunction(CheckPackage)


CheckPackage(PECOS)
list(APPEND DAKOTA_PKG_LIBS ${Pecos_LINK_LIBRARIES})
list(APPEND EXPORT_TARGETS ${Pecos_LINK_LIBRARIES})
if(HAVE_PECOS)
  list(APPEND DAKOTA_INCDIRS ${Pecos_BINARY_DIR} ${Pecos_SOURCE_DIR}/src
    ${VPISparseGrid_SOURCE_DIR}/src)
else()
  message(FATAL_ERROR
    "DAKOTA cannot currently be configured without PECOS: ${Pecos_SOURCE_DIR}")
endif(HAVE_PECOS)


## DAKOTA library sources.  Depend on NIDR_keywds.hpp to force generation
set(Dakota_src ${DAKOTA_BUILDINFO_PATH}DakotaBuildInfo.cpp 
  ${Dakota_BINARY_DIR}/generated/src/NIDR_keywds.hpp)

## UTIL sources.
set(util_src ParallelLibrary.cpp IteratorScheduler.cpp MPIPackBuffer.cpp
    dakota_data_util.cpp dakota_data_io.cpp dakota_global_defs.cpp 
    dakota_linear_algebra.cpp dakota_tabular_io.cpp 
    CommandLineHandler.cpp DakotaGraphics.cpp SensAnalysisGlobal.cpp 
    WorkdirHelper.cpp ResultsManager.cpp ResultsDBAny.cpp
    MPIManager.cpp ProgramOptions.cpp OutputManager.cpp
    ExperimentData.cpp UsageTracker.cpp ExperimentDataUtils.cpp
    ReducedBasis.cpp spectral_diffusion.cpp nested_sampling.cpp
    predator_prey.cpp
    )

if(DAKOTA_HAVE_HDF5)
  #list(APPEND util_src ResultsDBHDF5.cpp)
  list(APPEND util_src BinaryIO_Helper.cpp)
endif()

if(DAKOTA_DL_SOLVER)
  list(APPEND util_src DLSolver.cpp)
endif(DAKOTA_DL_SOLVER)

if(DAKOTA_USAGE_TRACKING)
  list(APPEND util_src TrackerHTTP.cpp)
endif(DAKOTA_USAGE_TRACKING)

## EVALDATA sources.
set(evaldata_src DakotaVariables.cpp MixedVariables.cpp RelaxedVariables.cpp
    SharedVariablesData.cpp DakotaActiveSet.cpp DakotaResponse.cpp
    SimulationResponse.cpp ExperimentResponse.cpp SharedResponseData.cpp
    ParamResponsePair.cpp)

## DB sources.
set(db_src ProblemDescDB.cpp NIDRProblemDescDB.cpp DataEnvironment.cpp
  DataMethod.cpp DataModel.cpp DataVariables.cpp DataInterface.cpp 
  DataResponses.cpp)

## Model sources.
set(model_src DakotaModel.cpp SimulationModel.cpp NestedModel.cpp
  RecastModel.cpp DataTransformModel.cpp ProbabilityTransformModel.cpp
  ScalingModel.cpp SurrogateModel.cpp DataFitSurrModel.cpp HierarchSurrModel.cpp
  ActiveSubspaceModel.cpp RandomFieldModel.cpp DiscrepancyCorrection.cpp
  AdaptedBasisModel.cpp)

## Constraint sources.
set(constraint_src DakotaConstraints.cpp MixedVarConstraints.cpp
    RelaxedVarConstraints.cpp)

## Interface sources.
set(interface_src DakotaInterface.cpp ApproximationInterface.cpp
    DakotaApproximation.cpp TaylorApproximation.cpp TANA3Approximation.cpp
    GaussProcApproximation.cpp VPSApproximation.cpp 
    PecosApproximation.cpp SharedApproxData.cpp
    SharedPecosApproxData.cpp
    ApplicationInterface.cpp ProcessApplicInterface.cpp
    ProcessHandleApplicInterface.cpp SysCallApplicInterface.cpp
    CommandShell.cpp DirectApplicInterface.cpp TestDriverInterface.cpp)
if(HAVE_SYS_WAIT_H AND HAVE_UNISTD_H)
  list(APPEND interface_src ForkApplicInterface.cpp)
elseif(WIN32)
  list(APPEND interface_src SpawnApplicInterface.cpp)
endif()
## Always build the plugin interfaces  
set(interface_src ${interface_src} PluginSerialDirectApplicInterface.cpp
  PluginParallelDirectApplicInterface.cpp)

CheckPackage(SURFPACK)
if(HAVE_SURFPACK)
  list(APPEND interface_src SurfpackApproximation.cpp SharedSurfpackApproxData.cpp)
  list(APPEND DAKOTA_INCDIRS ${Surfpack_BINARY_DIR} 
    ${Surfpack_SOURCE_DIR}/src)
  list(APPEND DAKOTA_PKG_LIBS surfpack surfpack_fortran)
endif(HAVE_SURFPACK)

if(DAKOTA_MATLAB)
  list(APPEND interface_src MatlabInterface.cpp)
endif()

if(DAKOTA_PYTHON)
  list(APPEND interface_src PythonInterface.cpp)
endif()

if(DAKOTA_SCILAB)
  list(APPEND interface_src ScilabInterface.cpp)
endif()

## Environment sources.
set(env_src DakotaEnvironment.cpp ExecutableEnvironment.cpp
    LibraryEnvironment.cpp)

## Iterator sources.
set(iterator_src DakotaIterator.cpp DakotaAnalyzer.cpp DakotaPStudyDACE.cpp
    ParamStudy.cpp DakotaVerification.cpp RichExtrapVerification.cpp 
    DakotaNonD.cpp
    NonDReliability.cpp NonDLocalReliability.cpp NonDGlobalReliability.cpp
    NonDInterval.cpp NonDLHSInterval.cpp NonDLHSEvidence.cpp 
    NonDLHSSingleInterval.cpp
    NonDLocalInterval.cpp NonDLocalEvidence.cpp NonDLocalSingleInterval.cpp
    NonDGlobalInterval.cpp NonDGlobalEvidence.cpp NonDGlobalSingleInterval.cpp
    NonDIntegration.cpp NonDCubature.cpp NonDQuadrature.cpp NonDSparseGrid.cpp
    NonDExpansion.cpp NonDPolynomialChaos.cpp NonDStochCollocation.cpp
    NonDCalibration.cpp NonDBayesCalibration.cpp NonDWASABIBayesCalibration.cpp
    NonDSampling.cpp NonDLHSSampling.cpp
    NonDMultilevelSampling.cpp NonDAdaptImpSampling.cpp NonDGPImpSampling.cpp 
    NonDPOFDarts.cpp NonDRKDDarts.cpp
    DakotaMinimizer.cpp DakotaOptimizer.cpp DakotaLeastSq.cpp
    NonlinearCGOptimizer.cpp 
    SurrBasedMinimizer.cpp SurrBasedLocalMinimizer.cpp SurrBasedLevelData.cpp
    HierarchSurrBasedLocalMinimizer.cpp DataFitSurrBasedLocalMinimizer.cpp
    SurrBasedGlobalMinimizer.cpp EffGlobalMinimizer.cpp OptDartsOptimizer.cpp
    MetaIterator.cpp ConcurrentMetaIterator.cpp SeqHybridMetaIterator.cpp
    EmbedHybridMetaIterator.cpp CollabHybridMetaIterator.cpp
)
set(Dakota_src_fortran lawson_hanson1.f)
if(DAKOTA_F90)
  list(APPEND Dakota_src_fortran lawson_hanson2.f90)
endif(DAKOTA_F90)

if(HAVE_ACRO)
  add_definitions("-DHAVE_ACRO")
  #add_definitions("-DDAKOTA_COLINY")
  add_definitions("-DDAKOTA_UTILIB")

  list(APPEND DAKOTA_INCDIRS ${Acro_BINARY_DIR}/packages
    ${Acro_BINARY_DIR}/packages/utilib
    ${Acro_SOURCE_DIR}/packages/interfaces/src
    ${Acro_SOURCE_DIR}/packages/scolib/src
    ${Acro_SOURCE_DIR}/packages/utilib/src
    ${Acro_SOURCE_DIR}/packages/colin/src
    ${Acro_SOURCE_DIR}/packages/pebbl/src/bb
    ${Acro_SOURCE_DIR}/packages/pebbl/src/comm
    ${Acro_SOURCE_DIR}/packages/pebbl/src/misc
    ${Acro_SOURCE_DIR}/packages/pebbl/src/pbb
    ${Acro_SOURCE_DIR}/packages/pebbl/src/sched
    ${Acro_SOURCE_DIR}/tpl/tinyxml
    )

  list(APPEND interface_src COLINApplication.cpp)
  list(APPEND iterator_src COLINOptimizer.cpp 
  	      		   PEBBLMinimizer.cpp PEBBLBranching.cpp)

  foreach(acro_dep utilib colin interfaces scolib 3po pebbl tinyxml)
    string(TOUPPER "${acro_dep}" ACRODEP)
    if (HAVE_${ACRODEP})
      list(APPEND DAKOTA_PKG_LIBS ${acro_dep})
      list(APPEND EXPORT_TARGETS ${acro_dep})
    endif()
  endforeach()
endif(HAVE_ACRO)

# ANN does approximate k-nearest neighbor search
if(HAVE_APPROXNN)
  add_definitions("-DHAVE_APPROXNN")
  list(APPEND DAKOTA_INCDIRS ${Dakota_SOURCE_DIR}/packages/external/approxnn/include)
  set(DAKOTA_PKG_LIBS ${DAKOTA_PKG_LIBS} approxnn)
  list(APPEND EXPORT_TARGETS approxnn)
endif()

if(HAVE_ADAPTIVE_SAMPLING)
  add_definitions("-DHAVE_ADAPTIVE_SAMPLING")
  list(APPEND iterator_src NonDAdaptiveSampling.cpp)
  if(HAVE_MORSE_SMALE)
    add_definitions("-DHAVE_MORSE_SMALE")
    list(APPEND iterator_src MorseSmaleComplex.cpp)
    # Dionysus provides persistence diagram and bottleneck distance
    # The MS_Complex class requires this unconditionally
    add_definitions("-DHAVE_DIONYSUS")
    # TODO: Using build tree as these don't currently get installed
    list(APPEND DAKOTA_INCDIRS 
      ${Dakota_BINARY_DIR}/packages/dionysus_ext-prefix/src/dionysus_ext/include)
  endif()
endif()

# Can't do CheckPackage because queso isn't built in CMake and
# therefore can't be in EXPORT_TARGETS.
#CheckPackage(QUESO)
if(HAVE_QUESO)

  list(APPEND iterator_src NonDQUESOBayesCalibration.cpp)
  add_definitions("-DHAVE_QUESO")
  if(HAVE_QUESO_GPMSA)
    add_definitions("-DHAVE_QUESO_GPMSA")
    list(APPEND iterator_src NonDGPMSABayesCalibration.cpp)
  endif()

  #list(APPEND DAKOTA_INCDIRS "$ENV{QUESO_PREFIX}/include")
  if(DAKOTA_QUESO_CMAKE)

    list(APPEND DAKOTA_INCDIRS ${Dakota_BINARY_DIR}/packages/external/queso/include/
      ${GSL_INCLUDE_DIRS})
    list(APPEND DAKOTA_PKG_LIBS queso)

  else()

  list(APPEND DAKOTA_INCDIRS 
    ${Dakota_BINARY_DIR}/packages/queso_ext/include ${GSL_INCLUDE_DIRS})

  #list(APPEND DAKOTA_LIBDIRS "$ENV{QUESO_PREFIX}/lib")
  # Would prefer to detect 32 vs. 64-bit, but the location is
  # operating system-specific; should be harmless to add the extra
  # directory, but put 64-bit first
  list(APPEND DAKOTA_LIBDIRS 
    ${Dakota_BINARY_DIR}/packages/queso_ext/lib64)
  list(APPEND DAKOTA_LIBDIRS 
    ${Dakota_BINARY_DIR}/packages/queso_ext/lib)

  # Can't put these in DAKOTA_PKG_LIBS; Trilinos will try to export them
  list(APPEND QUESO_EXTPROJ_LIBS queso)

  # Can't export these as they aren't executables or library targets
  #list(APPEND EXPORT_TARGETS queso_ext)
  #list(APPEND EXPORT_TARGETS queso gpmsa_any)

  list(APPEND DAKOTA_EXTPROJ_LIBS ${QUESO_EXTPROJ_LIBS})

  endif()

endif(HAVE_QUESO)


# Boost is required, but have to protect against empty DIR list
if(Boost_FOUND)
  list(APPEND EXT_TPL_LIBDIRS ${Boost_LIBRARY_DIRS})
  list(APPEND EXT_TPL_LIBS ${Boost_LIBRARIES})
endif()

CheckPackage(CONMIN)
if(HAVE_CONMIN)
  list(APPEND DAKOTA_LIBDIRS ${CONMIN_LIB_DIRS})
  set(iterator_src ${iterator_src} CONMINOptimizer.cpp)
endif(HAVE_CONMIN)

CheckPackage(DDACE)
if(HAVE_DDACE)
  list(APPEND DAKOTA_INCDIRS ${ddace_BINARY_DIR} ${ddace_SOURCE_DIR}/include)
  set(iterator_src ${iterator_src} DDACEDesignCompExp.cpp)
endif(HAVE_DDACE)

CheckPackage(DOT)
if(HAVE_DOT)
  set(iterator_src ${iterator_src} DOTOptimizer.cpp)
endif(HAVE_DOT)

CheckPackage(DREAM)
if(HAVE_DREAM)
  list(APPEND DAKOTA_INCDIRS ${Dream_SOURCE_DIR})
  set(iterator_src ${iterator_src} NonDDREAMBayesCalibration.cpp)
endif(HAVE_DREAM)

CheckPackage(FSUDACE)
if(HAVE_FSUDACE)
  list(APPEND DAKOTA_INCDIRS ${FSUDace_BINARY_DIR} ${FSUDace_SOURCE_DIR})
  set(iterator_src ${iterator_src} FSUDesignCompExp.cpp)
endif(HAVE_FSUDACE)

if(HAVE_HOPSPACK)
  #add_definitions("-DHAVE_HOPSPACK") # WJB: use convention?
  add_definitions("-DDAKOTA_HOPS")
  list(APPEND DAKOTA_INCDIRS ${HOPSPACK_BINARY_DIR}/src/src-shared
    ${HOPSPACK_SOURCE_DIR}/src/src-shared
    ${HOPSPACK_SOURCE_DIR}/src/src-citizens
    ${HOPSPACK_SOURCE_DIR}/src/src-evaluator
    ${HOPSPACK_SOURCE_DIR}/src/src-executor
    ${HOPSPACK_SOURCE_DIR}/src/src-framework
    ${HOPSPACK_SOURCE_DIR}/src/src-main
    )
  list(APPEND iterator_src APPSOptimizer.cpp APPSEvalMgr.cpp)
  #link_directories(${Dakota_BINARY_DIR}/packages/hopspack)
  list(APPEND DAKOTA_PKG_LIBS hopspack)
  list(APPEND EXPORT_TARGETS hopspack)
endif(HAVE_HOPSPACK)

if(HAVE_JEGA)
  add_definitions("-DHAVE_JEGA")
  list(APPEND DAKOTA_INCDIRS ${JEGA_BINARY_DIR} ${JEGA_BINARY_DIR}/eddy
    ${JEGA_SOURCE_DIR}/eddy ${JEGA_SOURCE_DIR}/include)
  list(APPEND iterator_src JEGAOptimizer.cpp)
  list(APPEND DAKOTA_PKG_LIBS jega jega_fe moga soga eutils utilities)
  list(APPEND EXPORT_TARGETS jega jega_fe moga soga eutils utilities)
endif(HAVE_JEGA)

if(HAVE_NCSUOPT)
  add_definitions("-DHAVE_NCSU")
  set(iterator_src ${iterator_src} NCSUOptimizer.cpp)
  list(APPEND DAKOTA_PKG_LIBS ncsuopt)
  list(APPEND EXPORT_TARGETS ncsuopt)
endif(HAVE_NCSUOPT)

CheckPackage(NLPQL)
if(HAVE_NLPQL)
  set(iterator_src ${iterator_src} NLPQLPOptimizer.cpp)
endif(HAVE_NLPQL)

if(HAVE_NL2SOL)
  add_definitions("-DHAVE_NL2SOL")
  set(iterator_src ${iterator_src} NL2SOLLeastSq.cpp)
  #link_directories(${Dakota_BINARY_DIR}/packages/NL2SOL)
  list(APPEND DAKOTA_PKG_LIBS cport)
  list(APPEND EXPORT_TARGETS cport)
endif(HAVE_NL2SOL)

if(HAVE_NOMAD)
  add_definitions("-DHAVE_NOMAD")
  list(APPEND DAKOTA_INCDIRS 
    ${NOMAD_BINARY_DIR}/src ${NOMAD_SOURCE_DIR}/src)
  set(iterator_src ${iterator_src} NomadOptimizer.cpp)
  list(APPEND DAKOTA_PKG_LIBS nomad)
  list(APPEND EXPORT_TARGETS nomad)
endif(HAVE_NOMAD)

if(HAVE_NOWPAC)
  add_definitions("-DHAVE_NOWPAC")
  list(APPEND DAKOTA_INCDIRS ${DAKOTA_NOWPAC_ROOT_DIR}/include ${DAKOTA_NOWPAC_NLOPT_ROOT}/include ${DAKOTA_NOWPAC_EIGEN_ROOT}/include/eigen3)
  link_directories(${DAKOTA_NOWPAC_ROOT_DIR}/lib ${DAKOTA_NOWPAC_NLOPT_ROOT}/lib) # ${DAKOTA_NOWPAC_ROOT_DIR}/include ${DAKOTA_NOWPAC_NLOPT_ROOT}/include
  set(iterator_src ${iterator_src} NOWPACOptimizer.cpp)
  list(APPEND DAKOTA_PKG_LIBS nowpac nlopt)
  #list(APPEND EXPORT_TARGETS nowpac nlopt)
  # package requires C++ flags "-std=c++11"
endif(HAVE_NOWPAC)

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
  list(APPEND DAKOTA_INCDIRS 
    ${optpp_BINARY_DIR}/include ${optpp_SOURCE_DIR}/include)

  add_definitions("-DHAVE_OPTPP")
  # WJB:  OPT++ macro naming "convention" differs from DAKOTA.  Even worse,
  #       Behavior is conditional outside of the Opt++ package due to do inline,
  #       VIRTUAL function definitions which can be enable/disabled "globally".
  # Therefore, define as many build macros as necessary to get a desirable exec.
  add_definitions("-DDAKOTA_OPTPP")

  set(iterator_src ${iterator_src} SNLLBase.cpp SNLLOptimizer.cpp SNLLLeastSq.cpp)
  list(APPEND DAKOTA_PKG_LIBS optpp)
  list(APPEND EXPORT_TARGETS optpp)
endif(HAVE_OPTPP)

CheckPackage(PSUADE)
if(HAVE_PSUADE)
  list(APPEND interface_src PSUADEDesignCompExp.cpp)
  list(APPEND DAKOTA_INCDIRS ${PSUADE_SOURCE_DIR})
endif(HAVE_PSUADE)

if(HAVE_X_GRAPHICS)
  add_definitions("-DHAVE_X_GRAPHICS")
  list(APPEND util_src graphics/Graph2D.cpp graphics/Graphics.cpp graphics/menu.c)
  list(APPEND DAKOTA_INCDIRS ${Dakota_SOURCE_DIR}/src/graphics ${DakotaSciPlot_SOURCE_DIR}
    ${DAKOTA_X_INCLUDE_DIR})
  list(APPEND DAKOTA_LIBDIRS ${DakotaSciPlot_BINARY_DIR})
  list(APPEND DAKOTA_PKG_LIBS dakota_sciplot)
  list(APPEND EXPORT_TARGETS dakota_sciplot)
  list(APPEND EXT_TPL_LIBS ${DAKOTA_X_LIBRARIES})
  # Install headers.
  install(
    FILES graphics/Graph2D.hpp graphics/Graphics.hpp graphics/menu.h
    DESTINATION include
    )
endif(HAVE_X_GRAPHICS)

## AGGREGATED source code spec for full-up libdakota.
set(Dakota_src ${Dakota_src} ${util_src} ${evaldata_src} ${db_src}
    ${model_src} ${constraint_src} ${interface_src} ${env_src}
    ${iterator_src})

# Experiment with moving AMPL here; if cross-platform link errors, revert
if(HAVE_AMPL)
  add_definitions("-DHAVE_AMPL")

  #include_directories(${Ampl_SOURCE_DIR}/ampl)
  list(APPEND DAKOTA_INCDIRS ${Dakota_BINARY_DIR}/packages/external/ampl)

  #link_directories(${Dakota_BINARY_DIR}/packages/ampl)
  list(APPEND DAKOTA_PKG_LIBS amplsolver)
  list(APPEND EXPORT_TARGETS amplsolver)
endif(HAVE_AMPL)

#if(DAKOTA_RESULTS_DB)
#  set(HAVE_YAML-CPP ON CACHE BOOL 
#      "Results DB requires yaml-cpp" FORCE)
#  CheckPackage(YAML-CPP)
#  list(APPEND EXT_TPL_INCDIRS ${YAML_CPP_SOURCE_DIR}/include)
#endif()

# Instead of trying to link GSL and CBLAS last, conditionally link a
# system CBLAS instead of gslcblas
if(DAKOTA_HAVE_GSL)
  list(APPEND EXT_TPL_INCDIRS "${GSL_INCLUDE_DIRS}")
  list(APPEND EXT_TPL_LIBDIRS "${GSL_LIBRARY_DIRS}")
  # GSL_LIBRARIES may or may not include a CBLAS library, depending on
  # top-level option GSL_WITHOUT_CBLAS
  list(APPEND QUESO_TPL_LIBS ${GSL_LIBRARIES})
  if(DAKOTA_CBLAS_LIBS) 
    list(APPEND QUESO_TPL_LIBS ${DAKOTA_CBLAS_LIBS})
  endif()
  list(APPEND EXT_TPL_LIBS ${QUESO_TPL_LIBS})
endif(DAKOTA_HAVE_GSL)

list(APPEND EXT_TPL_LIBS ${LAPACK_LIBS} ${BLAS_LIBS})

# Now that packages and TPLs are processed, set include/link directories
include_directories(${DAKOTA_INCDIRS} ${EXT_TPL_INCDIRS})
link_directories(${DAKOTA_LIBDIRS} ${EXT_TPL_LIBDIRS})


# --- Create the core Dakota libraries and set properties

add_library(dakota_src_fortran ${Dakota_src_fortran})
# the intel fortran compiler can  not handle all the -I stuff
# that is coming from the C++ files, but it does not need any of it
# so clear out the includes for this target
set_target_properties(dakota_src_fortran PROPERTIES INCLUDE_DIRECTORIES "")
list(APPEND EXPORT_TARGETS dakota_src_fortran)

add_library(dakota_src ${Dakota_src})
target_link_libraries(dakota_src dakota_src_fortran)
DakotaApplyMPISettings(dakota_src)
if(HAVE_AMPL)
  # Enforce AMPL config header creation prior to compiling dakota sources
  add_dependencies(dakota_src amplsolver)
endif()

# Enforce library ordering
list (APPEND DAKOTA_LIBS dakota_src dakota_src_fortran)


# Build a list of Dakota link libraries, encouraging order for QUESO, GSL
set(DAKOTA_ALL_LIBS ${DAKOTA_LIBS})
list(APPEND DAKOTA_ALL_LIBS ${DAKOTA_PKG_LIBS})
list(APPEND DAKOTA_ALL_LIBS ${DAKOTA_EXTPROJ_LIBS})
list(APPEND DAKOTA_ALL_LIBS ${EXT_TPL_LIBS})
#list(APPEND DAKOTA_ALL_LIBS ${QUESO_EXTPROJ_LIBS})
#list(APPEND DAKOTA_ALL_LIBS ${QUESO_TPL_LIBS})

# --- Explicit dependencies for Dakota core libraries

if(HAVE_ACRO)
  target_link_libraries(dakota_src utilib colin interfaces scolib)
endif()

if(HAVE_QUESO AND NOT DAKOTA_QUESO_CMAKE)
  # Enforce QUESO config header creation prior to compiling dakota sources
  add_dependencies(dakota_src queso_ext)
endif()

# EXTRA_TPL_LIBS doesn't suffice, due to symbols needed by ParallelLibrary
if (DAKOTA_MODELCENTER)
  target_link_libraries(dakota_src PHXCppApi)
endif()


#--- Add Dakota executables, with custom commands and properties

add_executable(dakota main.cpp)
add_executable(dakota_restart_util restart_util.cpp)
add_executable(dakota_library_mode library_mode.cpp)
if (DAKOTA_HAVE_MPI)
  add_executable(dakota_library_split library_split.cpp)
  target_link_libraries(dakota_library_split ${DAKOTA_ALL_LIBS})
  DakotaApplyMPISettings(dakota_library_split)
  install(TARGETS dakota_library_split DESTINATION bin)
endif()

#add_executable(test_sv test_stringvars.cpp)
#target_link_libraries(test_sv ${DAKOTA_ALL_LIBS})

if(DAKOTA_DLL_API)
  if (DAKOTA_MODELCENTER)
    # setup link dirs before inclusion of dll_api library
    link_directories(
      "${Dakota_SOURCE_DIR}/examples/linked_interfaces/ModelCenter/lib/i686-cygwin-cygwinNT")
  endif()
  add_executable(dll_tester dll_tester.cpp)
  
  target_link_libraries(dll_tester dakota_dll_api)
  if (DAKOTA_MODELCENTER)
    # likely not needed due to CMake dependency management of dakota_dll_api library
    target_link_libraries(dll_tester PHXCppApi)
  endif()
endif()

  
if(DAKOTA_SCILAB)
  set_target_properties(dakota PROPERTIES LINK_FLAGS "${SCI_LINK_FLAGS}")
  set_target_properties(dakota_restart_util PROPERTIES LINK_FLAGS "${SCI_LINK_FLAGS}")
  set_target_properties(dakota_library_mode PROPERTIES LINK_FLAGS "${SCI_LINK_FLAGS}")
endif()


target_link_libraries(dakota ${DAKOTA_ALL_LIBS})
DakotaApplyMPISettings(dakota)
target_link_libraries(dakota_restart_util ${DAKOTA_ALL_LIBS})
DakotaApplyMPISettings(dakota_restart_util)
target_link_libraries(dakota_library_mode ${DAKOTA_ALL_LIBS})
DakotaApplyMPISettings(dakota_library_mode)
if(DAKOTA_DLL_API)
  target_link_libraries(dll_tester ${DAKOTA_ALL_LIBS})
  DakotaApplyMPISettings(dll_tester)
endif()


#include(InstallRequiredSystemLibraries)
#include(GetPrerequisites)
#list_prerequisites(${CMAKE_CURRENT_BINARY_DIR}/dakota)

install(TARGETS dakota_src_fortran EXPORT ${ExportTarget} DESTINATION lib)
install(TARGETS dakota_src EXPORT ${ExportTarget} DESTINATION lib)
install(TARGETS dakota dakota_restart_util EXPORT DESTINATION bin)
install(TARGETS dakota_library_mode DESTINATION bin)
file(GLOB Dakota_hdr *.hpp *.h)
# We don't install NIDR_keywds0.hpp as is not needed by clients
install(FILES ${Dakota_hdr} ${Dakota_BINARY_DIR}/generated/src/NIDR_keywds.hpp
  DESTINATION include)
install(FILES dakota.xsd dakota.xml dakota.input.nspec
  ${Dakota_BINARY_DIR}/generated/src/dakota.input.summary
  DESTINATION bin)

if (DAKOTA_DLL_API)
  add_library(dakota_dll_api dakota_dll_api.cpp)
  target_link_libraries(dakota_dll_api ${DAKOTA_ALL_LIBS})
  install(TARGETS dakota_dll_api DESTINATION lib)
  DakotaApplyMPISettings(dakota_dll_api)
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
  elseif (WIN32)
    #include(InstallWinDLLs)
    # Attempt to migrate to a more standard install using Kitware's helper
    include(DakotaInstallRequiredSystemLibraries)
  endif()
endif()
  

# --- Define variables used for export Makefiles
# TODO: use these variables/names in the Trilinos block below

get_directory_property(Dakota_DEFS COMPILE_DEFINITIONS) 
set(Dakota_DEFINES ${Dakota_DEFS} CACHE INTERNAL 
  "Dakota preprocessor defines") 

# Includes from the source and build tree (will be installed)
set(Dakota_INCLUDE_DIRS ${DAKOTA_INCDIRS} CACHE INTERNAL 
  "Dakota source and build include directories") 
# External includes
set(Dakota_TPL_INCLUDE_DIRS ${EXT_TPL_INCDIRS} CACHE INTERNAL 
  "Dakota external TPL include directories") 

# Paths to libraries in the build tree (will be installed)
# Generate a list of build tree lib dirs; this will duplicate motif,
# teuchos, queso, potentially
##foreach(dpl ${DAKOTA_LIBS} ${DAKOTA_PKG_LIBS})
##  get_target_property(dpl_loc ${dpl} LOCATION)
##  get_filename_component(dpl_path ${dpl_loc} PATH)
##  list(APPEND DAKOTA_LIBDIRS ${dpl_path})
##endforeach()

set(Dakota_LIBRARY_DIRS ${DAKOTA_LIBDIRS} CACHE INTERNAL 
  "Dakota build-time library directories") 
# External libraries
set(Dakota_TPL_LIBRARY_DIRS ${EXT_TPL_LIBDIRS} CACHE INTERNAL 
  "Dakota external TPL library directories") 

# Any library that will be installed with Dakota, including Queso
set(Dakota_LIBRARIES ${DAKOTA_LIBS} ${DAKOTA_PKG_LIBS} ${DAKOTA_EXTPROJ_LIBS} 
#  ${QUESO_EXTPROJ_LIBS} 
  CACHE INTERNAL "Dakota libraries") 

# Libraries that will be on the system
set(Dakota_TPL_LIBRARIES ${EXT_TPL_LIBS}  
#  ${QUESO_TPL_LIBS}
  CACHE INTERNAL "Dakota TPL libraries") 
 
message(STATUS "Dakota_LIBRARIES: ${Dakota_LIBRARIES}")
message(STATUS "Dakota_TPL_LIBRARIES: ${Dakota_TPL_LIBRARIES}")


if (DAKOTA_API_JAVA)
  add_subdirectory(api/java)
endif()

if(DAKOTA_ENABLE_TESTS AND BUILD_TESTING)
  add_subdirectory(unit_test)
endif()


# --- Pass some data back to Trilinos

if (BUILD_IN_TRILINOS)

  # All include directories needed for Dakota
  # TODO: clients use Dakota_INCLUDE_DIRS and DAKOTA_TPL_INCLUDE_DIRS instead...
  get_directory_property(Dakota_INC_DIRS INCLUDE_DIRECTORIES)
  set(Dakota_INCLUDE_DIRS ${Dakota_INC_DIRS} CACHE INTERNAL 
    "Dakota include directories")

  # Link directories needed for queso, GSL
  set(Dakota_LINK_DIRS ${DAKOTA_LIBDIRS} ${EXT_TPL_LIBDIRS}
    CACHE INTERNAL "Dakota link directories")

  # Libraries built in Dakota CMake (can't include EXTPROJ libs here)
  set(Dakota_LIBS ${DAKOTA_LIBS} ${DAKOTA_PKG_LIBS})
  # Remove the Teuchos libraries
  list(REMOVE_ITEM Dakota_LIBS ${Teuchos_LIBRARIES})
  set(Dakota_LIBRARIES ${Dakota_LIBS} CACHE INTERNAL 
    "Dakota libraries")

  # Libraries built outside of Dakota CMake
  set(Dakota_EXTRA_LIBRARIES ${DAKOTA_EXTPROJ_LIBS} ${EXT_TPL_LIBS}
#    ${QUESO_EXTPROJ_LIBS}  ${QUESO_TPL_LIBS} 
    CACHE INTERNAL "Dakota extra libraries")
  message(STATUS 
    "Export targets to <export-name> = ${ExportTarget}: ${EXPORT_TARGETS}"
    )
endif()


# Describes where CMake will look during FindPackage
# http://www.cmake.org/cmake/help/v2.8.9/cmake.html#command:find_package
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR cmake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/cmake/Dakota)
endif()
set(Dakota_INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH
  "Installation directory for Dakota's CMake files")

# Generate export Makefile 
include(WriteExportMakefile) 
WriteExportMakefile() 
install(FILES ${Dakota_BINARY_DIR}/src/Makefile.export.Dakota.install 
  DESTINATION include RENAME Makefile.export.Dakota) 

# Unconditionally export targets to detect problems before Trilinos does
export(TARGETS ${EXPORT_TARGETS} FILE
  "${Dakota_BINARY_DIR}/DakotaTargets.cmake")

# Workaround until we fix Dakota's overall use of exports
if(ExportTarget STREQUAL DakotaTargets)
  message(STATUS 
    "Installing Dakota exported targets to ${Dakota_INSTALL_CMAKE_DIR}")
  install(EXPORT ${ExportTarget} DESTINATION ${Dakota_INSTALL_CMAKE_DIR})
endif()


# http://www.cmake.org/cmake/help/git-master/manual/cmake-packages.7.html#package-configuration-file
# http://www.cmake.org/cmake/help/git-master/module/CMakePackageConfigHelpers.html#module:CMakePackageConfigHelpers
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${Dakota_BINARY_DIR}/DakotaVersion.cmake"
  VERSION "${Dakota_VERSION_TRIPLE}"
  COMPATIBILITY AnyNewerVersion
  )

set(Dakota_INCLUDE_INSTALL_DIR include)
set(Dakota_LIB_INSTALL_DIR lib)

configure_package_config_file(
  ${Dakota_SOURCE_DIR}/cmake/DakotaConfig.cmake.in 
  ${Dakota_BINARY_DIR}/DakotaConfig.cmake
  INSTALL_DESTINATION ${Dakota_INSTALL_CMAKE_DIR}
  PATH_VARS Dakota_INCLUDE_INSTALL_DIR Dakota_LIB_INSTALL_DIR
)

install(
  FILES 
  "${Dakota_BINARY_DIR}/DakotaVersion.cmake" 
  "${Dakota_BINARY_DIR}/DakotaConfig.cmake" 
  DESTINATION "${Dakota_INSTALL_CMAKE_DIR}"
  )

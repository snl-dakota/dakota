
add_library(utilib 
  gencau.c   
  scauchy.c  
  genbet.c   
  genchi.c   
  genexp.c   
  genf.c     
  gengam.c   
  genmn.c    
  gennch.c   
  gennf.c    
  gennor.c   
  genunf.c   
  ignbin.c   
  ignpoi.c   
  sexpo.c    
  sgamma.c   
  snorm.c    
  genprm.c   
  globals.cpp  
  LCG.cpp    
  global_rng.c 
  PM_LCG.cpp 
  ParkMiller.c 
  CharString.cpp   
  BitArray.cpp     
  MixedIntVars.cpp 
  lookupa.cpp      
  hash_fn.cpp      
  Factory.cpp      
  Ereal.cpp        
  median.cpp   
  cov.c        
  cholesky.cpp 
  math_basic.cpp 
  math.c 
  misc.c           
  seconds.cpp      
  memdebug.cpp     
  CommonIO.cpp     
  exception_mngr.cpp 
  comments.cpp     
  mpiUtil.cpp      
  logEvent.cpp     
  nicePrint.cpp    
  OptionParser.cpp    
  TinyXML_helper.cpp    
  Property.cpp 
  PropertyDict.cpp 
  PropertyDict_YamlPrinter.cpp 
  Parameter.cpp 
  ParameterSet.cpp 
  ParameterList.cpp 
  Any.cpp      
  PackBuf.cpp      
  Serialize.cpp    
  SerialStream.cpp 
  BinarySerialStream.cpp 
  fSerialStream.cpp 
  sSerialStream.cpp 
  MPIStream.cpp 
  XMLSerialStream.cpp 
  fXMLSerialStream.cpp 
  TypeManager_old.cpp 
  TypeManager.cpp
)
# While target_include_directories was added in 2.8.11, it is buggy (and
#   broke the DAKOTA build).  We will require at least 2.8.12.
if( ${CMAKE_VERSION} VERSION_GREATER 2.8.12 )
  target_include_directories(utilib INTERFACE 
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
      $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
      $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include> )
endif()

set(libs ${Boost_LIBRARIES})
if(NOT MSVC)
  list(APPEND libs m)
endif()
if(UTILIB_HAVE_TINYXML)
  list(APPEND libs tinyxml)
endif()
if(UTILIB_HAVE_MPI)
  list(APPEND libs ${MPI_LIBRARY})
endif()

if(UTILIB_HAVE_EXPAT AND EXPAT_FOUND)
  include_directories(${EXPAT_INCLUDE_DIRS})
  list(APPEND libs ${EXPAT_LIBRARIES})
endif()
if(MSVC)
  list(APPEND libs dbghelp)
endif()


message("linking utilib against ${libs}")
target_link_libraries(utilib ${libs})
install(TARGETS utilib EXPORT ${ExportTarget} DESTINATION lib)

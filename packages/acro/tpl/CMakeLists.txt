set(TPLs 
  "${CMAKE_CURRENT_SOURCE_DIR}/3po"
  "${CMAKE_CURRENT_SOURCE_DIR}/ampl"
  "${CMAKE_CURRENT_SOURCE_DIR}/appspack"
  "${CMAKE_CURRENT_SOURCE_DIR}/glpk"
  "${CMAKE_CURRENT_SOURCE_DIR}/soplex"
  "${CMAKE_CURRENT_SOURCE_DIR}/tinyxml")

foreach(TPL ${TPLs})
  AcroCheckDirExists("${TPL}")
endforeach()

if(HAVE_AMPL AND NOT AMPL_INCLUDE_DIRS)
  add_subdirectory(ampl)
  add_definitions(-DACRO_USING_AMPL)
endif()

if(HAVE_3PO)
  add_subdirectory(3po)
  add_definitions(-DACRO_USING_THREEPO)
endif()

if(HAVE_TINYXML)
  add_subdirectory(tinyxml)
  add_definitions(-DACRO_USING_TINYXML)
endif()

if(HAVE_APPSPACK)
  add_subdirectory(appspack)
  add_definitions(-DACRO_USING_APPSPACK)
endif()

if(HAVE_GLPK)
  add_subdirectory(glpk)
  add_definitions(-DACRO_USING_GLPK)
endif()

if(HAVE_SOPLEX)
  add_subdirectory(soplex)
  add_definitions(-DACRO_USING_SOPLEX)
endif()

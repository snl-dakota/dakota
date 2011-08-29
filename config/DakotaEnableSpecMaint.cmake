# Helper function to process DAKOTA specification maintenance options:
# * whether to build NIDR-generated files into the source tree
# * whether to build options JAGUAR-related files like nidrgen
function(DakotaEnableSpecMaint)

  if (ENABLE_SPEC_MAINT)
  
    # generate dakota.input.summary using nidrgen
    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.summary"
      DEPENDS nidrgen 
              ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec
      COMMAND ${NIDR_BINARY_DIR}/nidrgen
      ARGS    -efp ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec > ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.summary
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      )
  
    # generate dakota.input.desc (depends on dakota.tags.desc)
    find_package(Perl REQUIRED)
    file(GLOB ref_dox_files ${Dakota_SOURCE_DIR}/docs Ref_*.dox)
    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.desc"
      DEPENDS nidrgen 
              ${CMAKE_CURRENT_SOURCE_DIR}/generate_desc.pl
  	    ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec
  	    ${ref_dox_files}
      COMMAND ${NIDR_BINARY_DIR}/nidrgen
      ARGS    -T ${CMAKE_CURRENT_SOURCE_DIR}/dakota.tags.desc ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec
      COMMAND ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/generate_desc.pl
      COMMAND ${CMAKE_COMMAND}
      ARGS    -E remove ${CMAKE_CURRENT_SOURCE_DIR}/dakota.tags.desc
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      )
  
    # generate NIDR_guikeywds.h: could move to jaguar-files target once
    # not checked in
    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/NIDR_guikeywds.h"
      DEPENDS nidrgen 
              ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec
  	    ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.desc
      COMMAND ${NIDR_BINARY_DIR}/nidrgen
      ARGS    -egG ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.desc > ${CMAKE_CURRENT_SOURCE_DIR}/NIDR_guikeywds.h
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      )
  
    # generate NIDR_keywds.H
    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/NIDR_keywds.H"
      DEPENDS nidrgen 
              ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec
  	    ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.desc
  	    ${CMAKE_CURRENT_SOURCE_DIR}/NIDR_initdefs.h
      COMMAND ${NIDR_BINARY_DIR}/nidrgen
      ARGS    dakota.input.nspec dakota.input.desc NIDR_initdefs.h NIDR_keywds.H
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      )
  
    # create a special target and add to "all" target
    add_custom_target(dakota-spec-files 
      ALL
      DEPENDS nidrgen
              ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.summary
  	    ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.desc
  	    ${CMAKE_CURRENT_SOURCE_DIR}/NIDR_guikeywds.h
  	    ${CMAKE_CURRENT_SOURCE_DIR}/NIDR_keywds.H
      VERBATIM
      )
  
    # Target jaguar-files builds targets needed for jaguar that are not
    # built as part of the all target
    # TODO: move guikeywds to this target as not needed in repo
    add_custom_target(jaguar-files DEPENDS dakreorder dakreord VERBATIM)
  
    # generate NIDR_keywds0.h (only needed by dakreorder.c) in two steps
    # (1) generate NIDR_keywds0.h.tmp
    # nidrgen generates the initial version of the file
    # Dt -8 generateds YYYMMDD for the newest file
    set(dakreorder_date_files 
      ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec 
      ${CMAKE_CURRENT_SOURCE_DIR}/dakreorder.c 
      ${NIDR_SOURCE_DIR}/nidr.c 
      ${NIDR_SOURCE_DIR}/nidr-parser.c 
      ${NIDR_SOURCE_DIR}/nidr-scanner.c
      ${NIDR_SOURCE_DIR}/avltree.c 
      ${CMAKE_CURRENT_BINARY_DIR}/NIDR_keywds0.h.tmp 
      )
    ADD_CUSTOM_COMMAND(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/NIDR_keywds0.h.tmp"
             date.txt
      DEPENDS nidrgen Dt ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec
      COMMAND ${NIDR_BINARY_DIR}/nidrgen
      ARGS -ftn- ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec > NIDR_keywds0.h.tmp
      COMMAND ${NIDR_BINARY_DIR}/Dt
      ARGS -8 ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec ${dakreorder_date_files} > date.txt
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      )
    # (2) generate NIDR_keywds0.h
    ADD_CUSTOM_COMMAND(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/NIDR_keywds0.h"
      DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/NIDR_keywds0.h.tmp" date.txt
      COMMAND ${CMAKE_COMMAND}
      ARGS
        -Dinfile:FILEPATH=${CMAKE_CURRENT_BINARY_DIR}/NIDR_keywds0.h.tmp
        -Ddatefile:FILEPATH=${CMAKE_CURRENT_BINARY_DIR}/date.txt
        -Doutfile:FILEPATH=${CMAKE_CURRENT_BINARY_DIR}/NIDR_keywds0.h
        -P ${Dakota_SOURCE_DIR}/config/finish_keywds0.cmake
      VERBATIM)
  
    # TODO: make -ldl optional if possible
    set(dakreorder_libs nidr)
    find_library(libdl dl) 
    if(libdl)
      list(APPEND dakreorder_libs ${libdl})
    endif()
  
    add_executable(dakreorder EXCLUDE_FROM_ALL dakreorder.c NIDR_keywds0.h)
    target_link_libraries(dakreorder ${dakreorder_libs})
  
    add_executable(dakreord EXCLUDE_FROM_ALL dakreorder.c)
    set_target_properties(dakreord PROPERTIES
      COMPILE_DEFINITIONS NO_NIDR_keywds0)
    target_link_libraries(dakreord ${dakreorder_libs})
  
    # TODO: Ensure DAKOTA sources depend on NIDR_keywds.H
  
  endif()

endfunction()

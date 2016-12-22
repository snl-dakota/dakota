# Helper functions to process Dakota specification maintenance options:

# Translate dakota.xml to dakota.input.nspec
# (We still generate the .nspec to the source tree and commit to repo
# since the process requires Java.)
function(DakotaXml2Nspec)

  get_target_property(xml2nidr_jar xml2nidr JAR_FILE)

  # xml2nidr: dakota.xml --> dakota.input.nspec
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/dakota.xml xml2nidr
    COMMAND ${Java_JAVA_EXECUTABLE} -classpath ${xml2nidr_jar}
            gov.sandia.dart.dakota.XMLToNIDRTranslator
	    ${CMAKE_CURRENT_SOURCE_DIR}/dakota.xml
	    ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec
    )

endfunction()


# nidrgen: dakota.input.nspec --> dakota.input.summary
function(DakotaNidrgen)

  # create a special target since the following two commands both
  # depend on the generated .nspec
  add_custom_target(dakota-spec-files
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec
    )

  # nidrgen: dakota.input.nspec --> dakota.input.summary
  add_custom_command(
    OUTPUT ${Dakota_BINARY_DIR}/generated/src/dakota.input.summary
    DEPENDS nidrgen dakota-spec-files
            ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec
    COMMAND ${CMAKE_COMMAND}
    ARGS -E make_directory ${Dakota_BINARY_DIR}/generated/src/
    COMMAND $<TARGET_FILE:nidrgen>
    ARGS    -efp ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec >
            ${Dakota_BINARY_DIR}/generated/src/dakota.input.summary
    )

  # nidrgen: dakota.input.nspec --> NIDR_keywds.hpp
  add_custom_command(
    OUTPUT ${Dakota_BINARY_DIR}/generated/src/NIDR_keywds.hpp
    DEPENDS nidrgen dakota-spec-files
            ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec
            # Artifical dependence to force generation
	    ${Dakota_BINARY_DIR}/generated/src/dakota.input.summary
    COMMAND $<TARGET_FILE:nidrgen>
    ARGS    ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec >
            ${Dakota_BINARY_DIR}/generated/src/NIDR_keywds.hpp
    )


  # -----
  # Simplified process to build/install dakreorder (dakota_order_input)
  # -----

  set(rel_date "${Dakota_RELEASE_DATE}")
  if (NOT rel_date)
    string(TIMESTAMP rel_date "%Y-%m-%d")
  endif()

  add_custom_command(
    OUTPUT "${Dakota_BINARY_DIR}/generated/src/NIDR_keywds0.h"
    DEPENDS nidrgen dakota-spec-files
            ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec
    # generate initial version
    COMMAND $<TARGET_FILE:nidrgen>
    ARGS -ftn- "${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec" >
         "${Dakota_BINARY_DIR}/generated/src/NIDR_keywds0.h"
    # append the version information
    COMMAND ${CMAKE_COMMAND}
    ARGS
      -Dappend_file:FILEPATH="${Dakota_BINARY_DIR}/generated/src/NIDR_keywds0.h"
      -Ddakota_version:STRING="${Dakota_VERSION_SRC}"
      -Drelease_date:STRING="${rel_date}"
  # This doesn't work due to escaping of spaces
  #    -Dnspec_date:STRING="${Dakota_VERSION_SRC} released ${rel_date}."
      -P "${Dakota_SOURCE_DIR}/cmake/append_nspec_date.cmake"
    )

  # TODO: make -ldl optional if possible
  set(dakreorder_libs nidr)
  find_library(libdl dl)
  if(libdl)
    list(APPEND dakreorder_libs ${libdl})
  endif()

  include_directories("${Dakota_BINARY_DIR}/generated/src")
  add_executable(dakota_order_input "${NIDR_SOURCE_DIR}/dakreorder.c"
    "${Dakota_BINARY_DIR}/generated/src/NIDR_keywds0.h")
  add_dependencies(dakota_order_input dakota-spec-files)
  # This shouldn't be necessary, but the line above doesn't seem to suffice
  set_source_files_properties("${NIDR_SOURCE_DIR}/dakreorder.c" PROPERTIES
    OBJECT_DEPENDS "${Dakota_BINARY_DIR}/generated/src/NIDR_keywds0.h")
  target_link_libraries(dakota_order_input ${dakreorder_libs})
  install(TARGETS dakota_order_input DESTINATION bin)

endfunction()


# -----
# Historical GUI-related command/targets
# -----

## DISABLED GUI METADATA
##    # generate dakota.input.desc (depends on dakota.tags.desc)
##    find_package(Perl REQUIRED)
##    file(GLOB ref_dox_files ${Dakota_SOURCE_DIR}/docs/oldref Ref_*.dox)
##    add_custom_command(
##      OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.desc"
##      DEPENDS nidrgen
##              ${CMAKE_CURRENT_SOURCE_DIR}/generate_desc.pl
##  	    ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec
##  	    ${ref_dox_files}
##      COMMAND ${NIDR_BINARY_DIR}/nidrgen
##      ARGS    -T ${CMAKE_CURRENT_SOURCE_DIR}/dakota.tags.desc ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec
##      COMMAND ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/generate_desc.pl
##      COMMAND ${CMAKE_COMMAND}
##      ARGS    -E remove ${CMAKE_CURRENT_SOURCE_DIR}/dakota.tags.desc
##      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
##      )
## DISABLED GUI METADATA

## DISABLED GUI METADATA
##    # generate NIDR_guikeywds.h: could move to jaguar-files target once
##    # not checked in
##    add_custom_command(
##      OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/NIDR_guikeywds.h"
##      DEPENDS nidrgen
##              ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec
##  	    ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.desc
##      COMMAND ${NIDR_BINARY_DIR}/nidrgen
##      ARGS    -egG ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.desc > ${CMAKE_CURRENT_SOURCE_DIR}/NIDR_guikeywds.h
##      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
##      )
## DISABLED GUI METADATA

## DISABLED GUI METADATA
##    # Target jaguar-files builds targets needed for jaguar that are not
##    # built as part of the all target
##    # TODO: move guikeywds to this target as not needed in repo
##    add_custom_target(jaguar-files DEPENDS dakreorder dakreord VERBATIM)
## DISABLED GUI METADATA

## DISABLED GUI METADATA
##    # generate NIDR_keywds0.h (only needed by dakreorder.c) in two steps
##    # (1) generate NIDR_keywds0.h.tmp
##    # nidrgen generates the initial version of the file
##    # Dt -8 generateds YYYMMDD for the newest file
##    set(dakreorder_date_files
##      ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec
##      ${CMAKE_CURRENT_SOURCE_DIR}/dakreorder.c
##      ${NIDR_SOURCE_DIR}/nidr.c
##      ${NIDR_SOURCE_DIR}/nidr-parser.c
##      ${NIDR_SOURCE_DIR}/nidr-scanner.c
##      ${NIDR_SOURCE_DIR}/avltree.c
##      ${CMAKE_CURRENT_BINARY_DIR}/NIDR_keywds0.h.tmp
##      )
##    ADD_CUSTOM_COMMAND(
##      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/NIDR_keywds0.h.tmp"
##             date.txt
##      DEPENDS nidrgen Dt ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec
##      COMMAND ${NIDR_BINARY_DIR}/nidrgen
##      ARGS -ftn- ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec > NIDR_keywds0.h.tmp
##      COMMAND ${NIDR_BINARY_DIR}/Dt
##      ARGS -8 ${CMAKE_CURRENT_SOURCE_DIR}/dakota.input.nspec ${dakreorder_date_files} > date.txt
##      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
##      )
##    # (2) generate NIDR_keywds0.h
##    ADD_CUSTOM_COMMAND(
##      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/NIDR_keywds0.h"
##      DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/NIDR_keywds0.h.tmp" date.txt
##      COMMAND ${CMAKE_COMMAND}
##      ARGS
##        -Dinfile:FILEPATH=${CMAKE_CURRENT_BINARY_DIR}/NIDR_keywds0.h.tmp
##        -Ddatefile:FILEPATH=${CMAKE_CURRENT_BINARY_DIR}/date.txt
##        -Doutfile:FILEPATH=${CMAKE_CURRENT_BINARY_DIR}/NIDR_keywds0.h
##        -P ${Dakota_SOURCE_DIR}/cmake/finish_keywds0.cmake
##      VERBATIM)
## DISABLED GUI METADATA


## DISABLED GUI METADATA
## add_executable(dakreord EXCLUDE_FROM_ALL dakreorder.c)
## set_target_properties(dakreord PROPERTIES
##   COMPILE_DEFINITIONS NO_NIDR_keywds0)
## target_link_libraries(dakreord ${dakreorder_libs})
## DISABLED GUI METADATA


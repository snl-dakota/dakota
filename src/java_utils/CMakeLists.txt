# Build Java tools that work with Dakota XML: NIDR translator and
# reference manual generator.  Building these tools requires java
# development kit, probed for at top-level.

if (${CMAKE_VERSION} VERSION_EQUAL 3.0 OR ${CMAKE_VERSION} VERSION_GREATER 3.0)
  # CMake 3.0 and newer choke on the absolute path
  set(DAKOTA_JAVA_UTIL_DIR "gov/sandia/dart/dakota")
else()
  # CMake 2.8.9 seems to require it
  set(DAKOTA_JAVA_UTIL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/gov/sandia/dart/dakota")
endif()

# Translates dakota.xml to dakota.input.nspec
if (ENABLE_SPEC_MAINT)
  add_jar(xml2nidr
    ${DAKOTA_JAVA_UTIL_DIR}/XMLToNIDRTranslator.java
    ${DAKOTA_JAVA_UTIL_DIR}/Indenter.java
    )
endif()

# The genrefman Java tool takes dakota.xml together with reference
# metadata files to create reference manual doxygen.
if (ENABLE_DAKOTA_DOCS)
  set(CMAKE_JAVA_INCLUDE_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/antlr-runtime-4.1.jar")
  set(NIDRANTLR_DIR "${DAKOTA_JAVA_UTIL_DIR}/nidr/antlr")
  add_jar(genrefman
    ${DAKOTA_JAVA_UTIL_DIR}/RefManGenDoxygen.java
    ${DAKOTA_JAVA_UTIL_DIR}/XMLToRefManSpec.java
    # don't currently neeed .nspec to .dox translation, but need to compile
    ${DAKOTA_JAVA_UTIL_DIR}/NIDRToRefManSpec.java   
    ${DAKOTA_JAVA_UTIL_DIR}/RefManMetaData.java
    ${DAKOTA_JAVA_UTIL_DIR}/RefManInputSpec.java
    ${DAKOTA_JAVA_UTIL_DIR}/RefManTopicTree.java
    # Would prefer to put these all on classpath, can't figure out how
    ${NIDRANTLR_DIR}/NIDRBaseListener.java
    ${NIDRANTLR_DIR}/NIDRBaseVisitor.java
    ${NIDRANTLR_DIR}/NIDRLexer.java
    #${NIDRANTLR_DIR}/NIDRLexer.tokens
    ${NIDRANTLR_DIR}/NIDRListener.java
    ${NIDRANTLR_DIR}/NIDRParser.java
    ${NIDRANTLR_DIR}/NIDR.tokens
    ${NIDRANTLR_DIR}/NIDRVisitor.java
    )
endif()

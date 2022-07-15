#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2022
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

project(DakotaDocs LANGUAGES NONE)
cmake_minimum_required(VERSION 3.17)

find_package(Java 11 REQUIRED)
if (NOT Java_FOUND)
  message(SEND_ERROR "Dakota docs-sphinx requires JDK 11.")
endif()
include(UseJava)

# TODO:
#find_executable(_dakota_sphinx_exe sphinx-build REQUIRED)

# Duplicate build of java utils
#set(Dakota_SOURCE_DIR "..")
#file(REAL_PATH ".." Dakota_SOURCE_DIR)
get_filename_component(Dakota_SOURCE_DIR ".." ABSOLUTE) 

set(DAKOTA_JAVA_UTIL_DIR
  "${Dakota_SOURCE_DIR}/src/java_utils/src/gov/sandia/dart/dakota")

set(CMAKE_JAVA_INCLUDE_PATH 
  "${Dakota_SOURCE_DIR}/src/java_utils/antlr-runtime-4.1.jar")
set(NIDRANTLR_DIR "${DAKOTA_JAVA_UTIL_DIR}/nidr/antlr")
add_jar(genrefman
  ${DAKOTA_JAVA_UTIL_DIR}/refman/RefManGenerator.java
  # don't currently neeed .nspec to .dox translation, but need to compile
  ${DAKOTA_JAVA_UTIL_DIR}/nidr/NIDRToRefManSpec.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/RefManMetaData.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/RefManInputSpec.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/metadata/InputSpecKeywordMetaData.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/metadata/RefManKeywordMetaData.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/print/DoxygenToRSTConverter.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/print/KeywordPageRSTPrinter.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/print/KeywordPageRSTTablePrinter.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/print/KeywordPrinter.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/print/TableKeyword.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/print/rst/table/CellFormatter.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/print/rst/table/CellPayload.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/print/rst/table/CellUtil.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/print/rst/table/GenericCell.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/print/rst/table/GenericRow.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/print/rst/table/GenericTable.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/print/rst/table/RstDividerPrinter.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/print/rst/table/RstRowPrinter.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/print/rst/table/RstTablePrinter.java
  ${DAKOTA_JAVA_UTIL_DIR}/refman/print/rst/table/SpanHoldCell.java
  ${DAKOTA_JAVA_UTIL_DIR}/xml/XMLToRefManSpec.java
  # Would prefer to put these all on classpath, can't figure out how
  ${NIDRANTLR_DIR}/NIDRBaseListener.java
  ${NIDRANTLR_DIR}/NIDRBaseVisitor.java  # may not need
  ${NIDRANTLR_DIR}/NIDRLexer.java
#  ${NIDRANTLR_DIR}/NIDRLexer.tokens      # may not need
  ${NIDRANTLR_DIR}/NIDRListener.java
  ${NIDRANTLR_DIR}/NIDRParser.java
#  ${NIDRANTLR_DIR}/NIDR.tokens           # may not need
  ${NIDRANTLR_DIR}/NIDRVisitor.java
  )

# Keyword reference dependencies
# BMA: Changed this to an absolute glob as relative was causing
# DakotaKeywords.dox to depend on src/environment target
file(GLOB ref_kw_md "${Dakota_SOURCE_DIR}/docs/KeywordMetadata/*")
##list_filter_emacs_backups(ref_kw_md)

# TODO: Generate all reference manual to build tree

# Generate the ultimate RST files for ref man keywords, specifying
# only required dependencies.  The genrefman Java tool takes
# dakota.xml together with reference metadata files to create
# reference manual keyword pages.
get_target_property(genrefman_jar genrefman JAR_FILE)
add_custom_command( 
  OUTPUT ${Dakota_SOURCE_DIR}/docs-sphinx/source/usingdakota/reference/environment.rst
  DEPENDS genrefman
          ${Dakota_SOURCE_DIR}/src/dakota.xml
	  ${ref_kw_md}
  COMMAND ${CMAKE_COMMAND}
  ARGS -E make_directory ${Dakota_SOURCE_DIR}/docs-sphinx/source/usingdakota/reference
  COMMAND ${Java_JAVA_EXECUTABLE} 
  ARGS -classpath ${genrefman_jar} 
    gov.sandia.dart.dakota.refman.RefManGenerator
    ${Dakota_SOURCE_DIR}/src/dakota.xml
    ${Dakota_SOURCE_DIR}/docs/KeywordMetadata
    ${Dakota_SOURCE_DIR}/docs-sphinx/source/usingdakota/reference
  COMMENT "Generating keyword reference RST files to ${Dakota_SOURCE_DIR}/docs-sphinx/source/usingdakota/reference/"
  )

add_custom_target(docs-keywords
  DEPENDS ${Dakota_SOURCE_DIR}/docs-sphinx/source/usingdakota/reference/environment.rst
)

# This assumes sphinx-build is available on the PATH
add_custom_target(docs-sphinx
  COMMAND sphinx-build -b html -j auto "${Dakota_SOURCE_DIR}/docs-sphinx/source" "${CMAKE_CURRENT_BINARY_DIR}/sphinx-html"
  COMMENT "Generating Sphinx docs"
)
add_dependencies(docs-sphinx docs-keywords) 

add_custom_target(docs-sphinx-full
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_CURRENT_BINARY_DIR}/sphinx-html"
  COMMAND sphinx-build -a -E -b html -j auto "${Dakota_SOURCE_DIR}/docs-sphinx/source" "${CMAKE_CURRENT_BINARY_DIR}/sphinx-html"
  COMMENT "Generating Sphinx docs (full rebuild)"
)
add_dependencies(docs-sphinx-full docs-keywords) 

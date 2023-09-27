#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2023
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

# Build Java tools that work with Dakota XML: NIDR translator and
# reference manual generator.  Building these tools requires java
# development kit, probed for at top-level.

set(DAKOTA_JAVA_UTIL_DIR "src/gov/sandia/dart/dakota")

# Translates dakota.xml to dakota.input.nspec
if (ENABLE_SPEC_MAINT)
  add_jar(xml2nidr
    ${DAKOTA_JAVA_UTIL_DIR}/xml/XMLToNIDRTranslator.java
    ${DAKOTA_JAVA_UTIL_DIR}/Indenter.java
    )
endif()

# The genrefman Java tool takes dakota.xml together with reference
# metadata files in KeywordMetadata to generate reference manual RST files.
if (ENABLE_DAKOTA_DOCS)
  set(CMAKE_JAVA_INCLUDE_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/antlr-runtime-4.1.jar")
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
    ${NIDRANTLR_DIR}/NIDRLexer.tokens      # may not need
    ${NIDRANTLR_DIR}/NIDRListener.java
    ${NIDRANTLR_DIR}/NIDRParser.java
    ${NIDRANTLR_DIR}/NIDR.tokens           # may not need
    ${NIDRANTLR_DIR}/NIDRVisitor.java
    )
endif()

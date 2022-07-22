#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2022
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

# TODO: make documentation build cross-platform

include(ListFilterEmacsBackups)

find_package(Doxygen REQUIRED)

# Required for generated included input files
find_package(Perl REQUIRED)

# TODO: Reduce configured headers

# create Doxygen files with substituted PATH and version information
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake.in
	       ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

# create Latex header with Dakota version information
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DakotaDefs.tex.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/DakotaDefs.tex)

# Keyword reference dependencies
# BMA: Changed this to an absolute glob as relative was causing
# DakotaKeywords.dox to depend on src/environment target
file(GLOB ref_kw_md "KeywordMetadata/*")
list_filter_emacs_backups(ref_kw_md)

# Doxygen developer dependencies
file(GLOB dox_dev_dox RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "Dev_*.dox")
set(dox_dev_deps ${dox_dev_dox} 
  ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile header-dev.html header-dev.tex)


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

add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/html-dev/index.html
          ${CMAKE_CURRENT_BINARY_DIR}/latex-dev/refman.tex
  DEPENDS ${dox_dev_deps}
  COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
  COMMENT "Doxygen: Developer's Manual"
)

# Target to perform only doxygen generation phase
add_custom_target(docs-doxygen 
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html-dev/index.html)

# build PDFs of dev/ref (not portable due to tex modifications)

# Create (or modify) texmf.cnf to prevent buffer overrun problem 
# Afterward, strip the trailing newline
##execute_process(COMMAND kpsewhich texmf.cnf OUTPUT_VARIABLE texmf_cnf_src)
##string(REGEX REPLACE "\n$" "" texmf_cnf_src ${texmf_cnf_src})

# If save_size exists in texmf.cnf, multiply it by 10x and replace. Otherwise, append 
# save_size = 150000 to the end.
##add_custom_command(
##  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/texmf.cnf
##  DEPENDS ${texmf_cnf_src}
##  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/modify_texmf.sh ${texmf_cnf_src} ${CMAKE_CURRENT_BINARY_DIR}
##)
##
##add_custom_target(docs-texmf-cnf DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/texmf.cnf)

# Build Developer's Manual into PDF
add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/latex-dev/refman.pdf
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/latex-dev/refman.tex
##${CMAKE_CURRENT_BINARY_DIR}/texmf.cnf
  COMMAND ${CMAKE_COMMAND} -E echo_append "Building Developers Manual PDF..."
##  COMMAND ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/latex_modify.perl
##   --developers
##  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR}/latex-dev sh -c 'export TEXMFCNF=${CMAKE_CURRENT_BINARY_DIR}:$$TEXMFCNF && make'
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR}/latex-dev sh -c 'make'
  COMMAND ${CMAKE_COMMAND} -E echo "Done."
  COMMENT "LaTeX: Developers to PDF"
)

add_custom_target(docs-pdf-dev 
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/latex-dev/refman.pdf)

# Generate HTML Dev/Ref, PDF Theory/Users
add_custom_target(docs)
add_dependencies(docs docs-doxygen)

# Generate PDF Dev/Ref/Theory/Users
add_custom_target(docs-pdf)
add_dependencies(docs-pdf docs-pdf-dev)


# Package docs for nightly builds (HTML and PDF for dev, ref; PDF for
# theory, users)
# TODO: Rework this into custom command(s) and bind into target
add_custom_target(package_docs
  COMMAND ${CMAKE_COMMAND} -E
    tar czf html-dev-${Dakota_VERSION_STRING}.tar.gz html-dev
  COMMAND ${CMAKE_COMMAND} -E copy
    latex-dev/refman.pdf Developers-${Dakota_VERSION_STRING}.pdf
  COMMAND ${CMAKE_COMMAND} -E copy
    latex-user/Users_Main.pdf Users-${Dakota_VERSION_STRING}.pdf
  COMMAND ${CMAKE_COMMAND} -E copy
    latex-theory/Theory_Main.pdf Theory-${Dakota_VERSION_STRING}.pdf
  COMMAND ${CMAKE_COMMAND} -E
    tar czf dakota-${Dakota_FINAL_PACKAGE_LABEL}-docs.tar.gz
      html-dev html-dev-${Dakota_VERSION_STRING}.tar.gz
      Developers-${Dakota_VERSION_STRING}.pdf
      Users-${Dakota_VERSION_STRING}.pdf
      Theory-${Dakota_VERSION_STRING}.pdf
  )
add_dependencies(package_docs docs-pdf docs-doxygen)

# Remove Doxygen-generated directories on make clean
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES 
  "html-dev;latex-dev;man-dev")

if(DAKOTA_DOCS_LEGACY)
  add_subdirectory(inactive)
endif()

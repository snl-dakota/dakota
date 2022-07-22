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

# Required for generated included input files
find_package(Perl REQUIRED)

# TODO: Reduce configured headers

# create Latex header with Dakota version information
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DakotaDefs.tex.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/DakotaDefs.tex)

# Keyword reference dependencies
# BMA: Changed this to an absolute glob as relative was causing
# DakotaKeywords.dox to depend on src/environment target
file(GLOB ref_kw_md "KeywordMetadata/*")
list_filter_emacs_backups(ref_kw_md)

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

add_subdirectory(dev)

# Generate HTML Dev
# TODO: depend on sphinx users
add_custom_target(docs)
add_dependencies(docs docs-dev-html)

# Generate PDF Dev
# TODO: depend on sphinx users
add_custom_target(docs-pdf)
add_dependencies(docs-pdf docs-dev-pdf)


# Package docs for nightly builds (HTML and PDF for dev)
# TODO: Rework this into custom command(s) and bind into target
# TODO: Better handling of html-dev instead of symlink
add_custom_target(package_docs
  COMMAND ${CMAKE_COMMAND} -E create_symlink dev/html-dev html-dev
  COMMAND ${CMAKE_COMMAND} -E
    tar czf html-dev-${Dakota_VERSION_STRING}.tar.gz "html-dev/*"
  COMMAND ${CMAKE_COMMAND} -E copy
    dev/latex-dev/refman.pdf Developers-${Dakota_VERSION_STRING}.pdf
  COMMAND ${CMAKE_COMMAND} -E
    tar czf dakota-${Dakota_FINAL_PACKAGE_LABEL}-docs.tar.gz
      "html-dev/*" html-dev-${Dakota_VERSION_STRING}.tar.gz
      Developers-${Dakota_VERSION_STRING}.pdf
  COMMENT "Packaging docs"
  )
add_dependencies(package_docs docs-pdf docs)


if(DAKOTA_DOCS_LEGACY)
  add_subdirectory(inactive)
endif()

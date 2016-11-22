# TODO: make documentation build cross-platform
# TODO: dep for latex_modify

include(ListFilterEmacsBackups)

find_package(Doxygen REQUIRED)
find_package(LATEX REQUIRED)
# Only required for PDFs of Doxygen, but unconditional currently
find_package(Perl REQUIRED)

# TODO: Reduce configured headers

# create Doxygen files with substituted PATH and version information
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake.in
	       ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile-ref.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile-ref)

# create Latex header with Dakota version information
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DakotaDefs.tex.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/DakotaDefs.tex)

# Doxygen reference dependencies
file(GLOB ref_kw_md RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "KeywordMetadata/*")
list_filter_emacs_backups(ref_kw_md)
file(GLOB ref_topic_md RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "TopicMetadata/*")
list_filter_emacs_backups(ref_topic_md)
file(GLOB ref_dox RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "Ref_*.dox")
file(GLOB ref_images RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "images/*")
list_filter_emacs_backups(ref_images)
set(dox_ref_deps ${ref_kw_md} ${ref_topic_md} ${ref_dox} Doxyfile-ref 
  ${ref_images} header-ref.html header-ref.tex Dakota.bib)

# Doxygen developer dependencies
file(GLOB dox_dev_dox RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "Dev_*.dox")
set(dox_dev_deps 
  ${dox_dev_dox} Doxyfile header-dev.html header-dev.tex)


# Generate the doxygen files for ref man keywords and topics,
# specifying only required dependencies.  The genrefman Java tool
# takes dakota.xml together with reference metadata files to create
# reference manual doxygen.
get_target_property(genrefman_jar genrefman JAR_FILE)
add_custom_command( 
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/DakotaKeywords.dox 
          ${CMAKE_CURRENT_BINARY_DIR}/DakotaTopics.dox 
  DEPENDS genrefman
          ${Dakota_SOURCE_DIR}/src/dakota.xml
	  ${ref_kw_md}
	  ${ref_topic_md}
	  ${Dakota_SOURCE_DIR}/docs/Ref_TopicTree.txt
	  ${Dakota_SOURCE_DIR}/docs/Ref_Topics_Intro.dox
  COMMAND ${Java_JAVA_EXECUTABLE} 
  ARGS -classpath ${genrefman_jar} 
    gov.sandia.dart.dakota.RefManGenDoxygen
    ${Dakota_SOURCE_DIR}/src/dakota.xml
    ${Dakota_SOURCE_DIR}/docs/KeywordMetadata
    ${Dakota_SOURCE_DIR}/docs/Ref_TopicTree.txt
    ${Dakota_SOURCE_DIR}/docs/TopicMetadata
    ${Dakota_SOURCE_DIR}/docs/Ref_Topics_Intro.dox
    ${CMAKE_CURRENT_BINARY_DIR} 
)
 
# Generate the master DakotaReferenceManual.dox
add_custom_command(
  DEPENDS  ${CMAKE_CURRENT_SOURCE_DIR}/ref_concat_dox.sh 
  ${Dakota_BINARY_DIR}/generated/src/dakota.input.summary
    ${CMAKE_CURRENT_BINARY_DIR}/DakotaKeywords.dox 
    ${CMAKE_CURRENT_BINARY_DIR}/DakotaTopics.dox 
    ${ref_dox}
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/ref_concat_dox.sh 
  ARGS ${CMAKE_CURRENT_SOURCE_DIR} 
  ${Dakota_BINARY_DIR}/generated/src/dakota.input.summary
    ${CMAKE_CURRENT_BINARY_DIR}/DakotaReferenceManual.dox 
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/DakotaReferenceManual.dox
    ${CMAKE_CURRENT_BINARY_DIR}/DakotaInputSpec.dox 
) 


# BMA: I simplified this as currently reference does not depend on developers

# Generate .dox into doxygen tag files
add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/dakota-developers.tag
  DEPENDS ${dox_dev_deps}
  COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
  COMMENT "Doxygen: Developer's Manual (first pass: tag generation)"
)

# BMA: Added an OUTPUT here for index.html since 2 passes don't work
# with new manual
add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/dakota-reference.tag
          ${CMAKE_CURRENT_BINARY_DIR}/html-ref/index.html
          ${CMAKE_CURRENT_BINARY_DIR}/latex-ref/refman.tex
  DEPENDS ${dox_ref_deps}
          ${CMAKE_CURRENT_BINARY_DIR}/DakotaReferenceManual.dox
  COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile-ref
  COMMENT "Doxygen: Reference Manual (first pass: tag generation)"
)

# Generate .tag files into final doxygen-generated docs
# Overstate deps to force the tag phase for both to go first
add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/html-dev/index.html
          ${CMAKE_CURRENT_BINARY_DIR}/latex-dev/refman.tex
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dakota-developers.tag
          ${CMAKE_CURRENT_BINARY_DIR}/dakota-reference.tag
  COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
  COMMENT "Doxygen: Developer's Manual (final pass)"
)

# BMA: Running doxygen twice seems to break on the new reference manual
# I commented this and instead made the above command generate the final index.html dep.
#add_custom_command(
#  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/html-ref/index.html
#  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dakota-developers.tag
#          ${CMAKE_CURRENT_BINARY_DIR}/dakota-reference.tag
#  COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile-ref
#  COMMENT "Doxygen: Reference Manual (final pass)"
#)

# Target to perform only doxygen generation phase
add_custom_target(docs-doxygen 
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html-dev/index.html 
          ${CMAKE_CURRENT_BINARY_DIR}/html-ref/index.html
)

# build PDFs of dev/ref (not portable due to tex modifications)

# Create (or modify) texmf.cnf to prevent buffer overrun problem 
# Afterward, strip the trailing newline
execute_process(COMMAND kpsewhich texmf.cnf OUTPUT_VARIABLE texmf_cnf_src)
string(REGEX REPLACE "\n$" "" texmf_cnf_src ${texmf_cnf_src})

# If save_size exists in texmf.cnf, multiply it by 10x and replace. Otherwise, append 
# save_size = 150000 to the end.
add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/texmf.cnf
  DEPENDS ${texmf_cnf_src}
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/modify_texmf.sh ${texmf_cnf_src} ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(docs-texmf-cnf DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/texmf.cnf)

# Build Developer's Manual into PDF
add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/latex-dev/refman.pdf
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/latex-dev/refman.tex ${CMAKE_CURRENT_BINARY_DIR}/texmf.cnf
  COMMAND ${CMAKE_COMMAND} -E echo_append "Building Developers Manual PDF..."
  COMMAND ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/latex_modify.perl
   --developers
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR}/latex-dev sh -c 'export TEXMFCNF=${CMAKE_CURRENT_BINARY_DIR}:$$TEXMFCNF && make'
  COMMAND ${CMAKE_COMMAND} -E echo "Done."
  COMMENT "LaTeX: Developers to PDF"
)

add_custom_target(docs-pdf-dev 
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/latex-dev/refman.pdf)

# Build Reference manual into PDF
add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/latex-ref/refman.pdf
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/latex-ref/refman.tex
  COMMAND ${CMAKE_COMMAND} -E echo_append "Building Reference Manual PDF..."
  COMMAND ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/latex_modify.perl
  --reference
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR}/latex-ref make
  COMMAND ${CMAKE_COMMAND} -E echo "Done."
  COMMENT "LaTeX: Reference to PDF"
)

add_custom_target(docs-pdf-ref 
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/latex-ref/refman.pdf)


# These needed for the SAND shells that follow
add_subdirectory(latex-theory)
if(DAKOTA_ENABLE_TESTS)
  add_subdirectory(latex-user)
else()
  message(WARNING 
    "Users Manual will not be built; requires DAKOTA_ENABLE_TESTS")
endif()


#              ${CMAKE_CURRENT_BINARY_DIR}/latex-${manual}/${sourcePDFname}.pdf

# Helper function to build a SAND manual
function(pdf2sand manual sourcePDFname)

  set(file_tex SAND-shell-${manual})
  add_custom_command(
    OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.pdf
    DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/${file_tex}.tex 
    COMMAND   sh -c 'export TEXINPUTS=${CMAKE_CURRENT_SOURCE_DIR}:${CMAKE_CURRENT_SOURCE_DIR}/SANDreport:$$TEXINPUTS && ${PDFLATEX_COMPILER} -interaction=batchmode ${CMAKE_CURRENT_SOURCE_DIR}/${file_tex}'
  )
  add_custom_target(docs-sand-${manual} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.pdf)
  add_dependencies(docs-sand-${manual} docs-pdf-${manual})

endfunction(pdf2sand)


# Build SAND versions of manuals
pdf2sand(dev refman)
pdf2sand(ref refman)
pdf2sand(theory Theory_Main)
pdf2sand(user Users_Main)


# Generate HTML Dev/Ref, PDF Theory/Users
add_custom_target(docs)
add_dependencies(docs docs-doxygen docs-pdf-user docs-pdf-theory)

# Generate PDF Dev/Ref/Theory/Users
add_custom_target(docs-pdf)
add_dependencies(docs-pdf docs-pdf-dev docs-pdf-ref docs-pdf-theory docs-pdf-user)

# Generate SAND versions of all
add_custom_target(docs-sand)
add_dependencies(docs-sand docs-sand-dev docs-sand-ref docs-sand-theory docs-sand-user)

# Package docs for nightly builds (HTML and PDF for dev, ref; PDF for
# theory, users)
# TODO: Rework this into custom command(s) and bind into target
add_custom_target(package_docs
  COMMAND ${CMAKE_COMMAND} -E 
    tar czf html-ref-${Dakota_VERSION_STRING}.tar.gz html-ref
  COMMAND ${CMAKE_COMMAND} -E
    tar czf html-dev-${Dakota_VERSION_STRING}.tar.gz html-dev
  COMMAND ${CMAKE_COMMAND} -E copy
    latex-dev/refman.pdf Developers-${Dakota_VERSION_STRING}.pdf
  COMMAND ${CMAKE_COMMAND} -E copy
    latex-ref/refman.pdf Reference-${Dakota_VERSION_STRING}.pdf
  COMMAND ${CMAKE_COMMAND} -E copy
    latex-user/Users_Main.pdf Users-${Dakota_VERSION_STRING}.pdf
  COMMAND ${CMAKE_COMMAND} -E copy
    latex-theory/Theory_Main.pdf Theory-${Dakota_VERSION_STRING}.pdf
  COMMAND ${CMAKE_COMMAND} -E
    tar czf dakota-${Dakota_VERSION_STRING}.docs.tar.gz
      html-ref html-ref-${Dakota_VERSION_STRING}.tar.gz
      html-dev html-dev-${Dakota_VERSION_STRING}.tar.gz
      Developers-${Dakota_VERSION_STRING}.pdf
      Reference-${Dakota_VERSION_STRING}.pdf
      Users-${Dakota_VERSION_STRING}.pdf
      Theory-${Dakota_VERSION_STRING}.pdf
  )
add_dependencies(package_docs docs-pdf docs-doxygen)

# Remove Doxygen-generated directories on make clean
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES 
  "html-dev;html-ref;latex-dev;latex-ref;man-dev;man-ref;SAND-shell-dev.aux;SAND-shell-dev.log;SAND-shell-ref.aux;SAND-shell-ref.log;SAND-shell-theory.aux;SAND-shell-theory.log;SAND-shell-user.aux;SAND-shell-user.log"
  )

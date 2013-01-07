# TODO: make documentation build cross-platform
# TODO: dep for latex_modify

find_package(Doxygen REQUIRED)
find_package(LATEX REQUIRED)
# Only required for PDFs of Doxygen, but unconditional currently
find_package(Perl REQUIRED)

# TODO: Reduce configured headers

# create Doxygen files with substituted PATH information
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake.in
	       ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile-ref.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile-ref)

# create DAKOTA headers
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DakotaDefs.tex
               ${CMAKE_CURRENT_BINARY_DIR}/DakotaDefs.tex COPYONLY)


# Doxygen reference dependencies
file(GLOB dox_ref_dox RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "Ref_*.dox")
file(GLOB dox_ref_images RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "images/*")
set(dox_ref_deps 
  ${dox_ref_dox} Doxyfile-ref ${dox_ref_images} header-ref.html header-ref.tex)

# Doxygen developer dependencies
file(GLOB dox_dev_dox RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "Dev_*.dox")
set(dox_dev_deps 
  ${dox_dev_dox} Doxyfile header-dev.html header-dev.tex)


# Generate .dox into doxygen tag files
add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/dakota-developers.tag
  DEPENDS ${dox_dev_deps}
  COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
  COMMENT "Doxygen: Developer's Manual (first pass: tag generation)"
)

add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/dakota-reference.tag
  DEPENDS ${dox_ref_deps}
  COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile-ref
  COMMENT "Doxygen: Reference Manual (first pass: tag generation)"
)

# Generate .tag files into final doxygen-generated docs
# Overstate deps to force the tag phase for both to go first
add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/html-dev/index.html
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dakota-developers.tag
          ${CMAKE_CURRENT_BINARY_DIR}/dakota-reference.tag
  COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
  COMMENT "Doxygen: Developer's Manual (final pass)"
)

add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/html-ref/index.html
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dakota-developers.tag
          ${CMAKE_CURRENT_BINARY_DIR}/dakota-reference.tag
  COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile-ref
  COMMENT "Doxygen: Reference Manual (final pass)"
)

# Target to perform only doxygen generation phase
add_custom_target(docs-doxygen 
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html-dev/index.html 
          ${CMAKE_CURRENT_BINARY_DIR}/html-ref/index.html
)

# Use index.html as fake dep; use Perl script to "generate" refman.tex
add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/latex-dev/refman.tex
          ${CMAKE_CURRENT_BINARY_DIR}/latex-ref/refman.tex
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html-dev/index.html 
          ${CMAKE_CURRENT_BINARY_DIR}/html-ref/index.html
  COMMAND ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/latex_modify.perl
  COMMENT "Perl: Modifying Developer's and Reference Manuals for LaTeX"
)


# build PDFs of dev/ref (not portable due to tex modifications)

# Create texmf.cnf to prevent buffer overrun problem 
# TODO: use `kpsewhich texmf.cnf` to get file location
set(texmf_cnf_src /usr/share/texmf/web2c/texmf.cnf)
add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/texmf.cnf
  DEPENDS ${texmf_cnf_src}
  COMMAND sh -c 
    'curr_save_size=`grep -o \"save_size = [0-9]*\" ${texmf_cnf_src} | cut -f2 -d'='` && 
     new_save_size=`expr 10 \\* $$curr_save_size` &&
     sed -e \"s/save_size = [0-9]\\+/save_size = $$new_save_size/g\" ${texmf_cnf_src} > ${CMAKE_CURRENT_BINARY_DIR}/texmf.cnf'
)
add_custom_target(docs-texmf-cnf DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/texmf.cnf)

# Build Developer's Manual into PDF
add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/latex-dev/refman.pdf
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/latex-dev/refman.tex ${CMAKE_CURRENT_BINARY_DIR}/texmf.cnf
  COMMAND ${CMAKE_COMMAND} -E echo_append "Building Developers Manual PDF..."
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
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR}/latex-ref make
  COMMAND ${CMAKE_COMMAND} -E echo "Done."
  COMMENT "LaTeX: Reference to PDF"
)

add_custom_target(docs-pdf-ref 
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/latex-ref/refman.pdf)


# These needed for the SAND shells that follow
add_subdirectory(latex-theory)
add_subdirectory(latex-user)


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
add_dependencies(docs docs-doxygen docs-latex-user docs-latex-theory)

# Generate PDF Dev/Ref/Theory/Users
add_custom_target(docs-pdf)
add_dependencies(docs-pdf docs-pdf-dev docs-pdf-ref docs-pdf-theory docs-pdf-user)

# Generate SAND versions of all
add_custom_target(docs-sand)
add_dependencies(docs-sand docs-sand-dev docs-sand-ref docs-sand-theory docs-sand-user)


# Remove Doxygen-generated directories on make clean
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES 
  "html-dev;html-ref;latex-dev;latex-ref;man-dev;man-ref;SAND-shell-dev.aux;SAND-shell-dev.log;SAND-shell-ref.aux;SAND-shell-ref.log;SAND-shell-theory.aux;SAND-shell-theory.log;SAND-shell-user.aux;SAND-shell-user.log"
  )
# _______________________________________________________________________
#
#    Dakota: Explore and predict with confidence.
#    Copyright 2014-2025
#    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#    This software is distributed under the GNU Lesser General Public License.
#    For more information, see the README file in the top Dakota directory.
# _______________________________________________________________________

find_package(Doxygen 1.8.20 REQUIRED)
#find_package(LATEX REQUIRED)

# create Doxygen files with substituted PATH and version information
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake.in
	       ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

# Doxygen developer dependencies
file(GLOB dox_dev_dox RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "Dev_*.dox")
set(dox_dev_deps ${dox_dev_dox} 
  ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile header-dev.html header-dev.tex)

add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/html-dev/index.html
          ${CMAKE_CURRENT_BINARY_DIR}/latex-dev/refman.tex
  DEPENDS ${dox_dev_deps}
  COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
  COMMENT "Doxygen: Developer's Manual"
)

# Target to perform only doxygen generation phase
add_custom_target(docs-dev-html
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
#add_custom_command(
#  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/latex-dev/refman.pdf
#  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/latex-dev/refman.tex
##${CMAKE_CURRENT_BINARY_DIR}/texmf.cnf
#  COMMAND ${CMAKE_COMMAND} -E echo_append "Building Developers Manual PDF..."
##  COMMAND ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/latex_modify.perl
##   --developers
##  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR}/latex-dev sh -c 'export TEXMFCNF=${CMAKE_CURRENT_BINARY_DIR}:$$TEXMFCNF && make'
#  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR}/latex-dev sh -c 'make'
#  COMMAND ${CMAKE_COMMAND} -E echo "Done."
#  COMMENT "LaTeX: Developers to PDF"
#)

#add_custom_target(docs-dev-pdf
#  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/latex-dev/refman.pdf)

# Remove Doxygen-generated directories on make clean
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES 
  "dakota-developers.tag;html-dev;latex-dev;man-dev")

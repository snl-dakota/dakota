#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2023
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

find_package(LATEX REQUIRED)

# Required for PDFs of Doxygen and user examples generation
# Only used in test/ for generating examples for this dir...
#find_package(Perl REQUIRED)

# Helper function to build a SAND manual
function(pdf2sand manual sourcePDFname)

  set(file_tex SAND-shell-${manual})
  add_custom_command(
    OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.pdf
    DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/${file_tex}.tex 
    COMMAND   sh -c 'export TEXINPUTS=${CMAKE_CURRENT_BINARY_DIR}/..:${CMAKE_CURRENT_SOURCE_DIR}:${CMAKE_CURRENT_SOURCE_DIR}/SANDreport:$$TEXINPUTS && ${PDFLATEX_COMPILER} -interaction=batchmode ${CMAKE_CURRENT_SOURCE_DIR}/${file_tex}'
  )
  add_custom_target(docs-sand-${manual} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.pdf)
  add_dependencies(docs-sand-${manual} docs-pdf-${manual})

endfunction(pdf2sand)

add_subdirectory(latex-theory)
pdf2sand(theory Theory_Main)

if(DAKOTA_ENABLE_TESTS)
  add_subdirectory(latex-user)
  pdf2sand(user Users_Main)
else()
  message(WARNING
    "Users Manual will not be built; requires DAKOTA_ENABLE_TESTS")
endif()

# Generate SAND versions of theory, user
add_custom_target(docs-sand)
add_dependencies(docs-sand docs-sand-theory)
if(DAKOTA_ENABLE_TESTS)
  add_dependencies(docs-sand docs-sand-user)
endif()

# Remove Doxygen-generated directories on make clean
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES 
  "SAND-shell-theory.aux;SAND-shell-theory.log;SAND-shell-user.aux;SAND-shell-user.log"
  )

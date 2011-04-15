# TODO
# * Consider using file copy instead of configure_file, when config not needed?
# * Create proper make clean information
# * Generate files with deps to the source tree so you can update there and
#   make in the binary tree

# Copy tex inputs into binary tree for now
file(GLOB users_tex ${CMAKE_CURRENT_SOURCE_DIR}
     "*.tex" "*.bib" "*.sty")
file(COPY ${users_tex} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# for deps
file(GLOB users_images 
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/images
     "*.png" "*.eps"
)

# actually copy the files
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/images 
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Command with artificial dependencies to run latex commands in sequence
# TODO: other deps; recopy files if out of date
set(file_tex Theory_Main)

add_custom_command(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.aux
  DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/${file_tex}.tex 
            ${all_generated_inputs} ${users_images}
  COMMAND   ${PDFLATEX_COMPILER}
  ARGS      -interaction=batchmode ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}
  COMMENT   "Latex (first pass): Theory"
)

add_custom_command(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.bbl
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.aux
  COMMAND   ${BIBTEX_COMPILER}
  ARGS      -terse ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}
  COMMENT   "Bibtex: Theory"
)

add_custom_command(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.dvi
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.bbl
  COMMAND   ${PDFLATEX_COMPILER}
  ARGS      -interaction=batchmode ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}
  COMMENT   "Latex (second pass): Theory"
)

add_custom_command(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.log
            ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.pdf
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.bbl
            ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.dvi
  COMMAND   ${PDFLATEX_COMPILER}
  ARGS      -interaction=batchmode ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}
  COMMENT   "Latex (third pass): Theory"
)

# Eventually trigger the whole process
add_custom_target(docs-pdf-theory echo
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.pdf
)

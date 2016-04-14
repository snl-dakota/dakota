include(AddFileCopyCommand)

# Build a list of absolute paths of latex inputs for use in dependencies
set(theory_inputs_abs "") 

# Copy .tex and related files into binary tree (for now), with dep to src
file(GLOB theory_tex RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
  "*.tex" "*.bib" "*.sty")
foreach(file ${theory_tex}) 
  add_file_copy_command( 
    "${CMAKE_CURRENT_SOURCE_DIR}/${file}" 
    "${CMAKE_CURRENT_BINARY_DIR}/${file}"
    ) 
  list(APPEND theory_inputs_abs "${CMAKE_CURRENT_BINARY_DIR}/${file}")
endforeach() 

# Copy the bib file to make latex versions easier to manage
# Really belongs one dir above, but don't know how to manage the dependency
add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/../Dakota.bib
                      ${CMAKE_CURRENT_BINARY_DIR}/../Dakota.bib)

execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory 
  "${CMAKE_CURRENT_BINARY_DIR}/images")

# Build a list of absolute paths of images for use in dependencies
set(theory_images_abs "") 
file(GLOB_RECURSE theory_images RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/images"
  "*.png" "*.eps" "*.pdf")
foreach(file ${theory_images}) 
  add_file_copy_command( 
    "${CMAKE_CURRENT_SOURCE_DIR}/images/${file}"
    "${CMAKE_CURRENT_BINARY_DIR}/images/${file}"
    ) 
  list(APPEND theory_images_abs "${CMAKE_CURRENT_BINARY_DIR}/images/${file}")
endforeach() 


# Command with artificial dependencies to run latex commands in sequence
set(file_tex Theory_Main)

# TODO: verbosity-based control of latex output
add_custom_command(
  OUTPUT    "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.aux"
  DEPENDS   "${CMAKE_CURRENT_SOURCE_DIR}/${file_tex}.tex" 
            ${theory_images_abs} ${theory_inputs_abs}
	    "${CMAKE_CURRENT_BINARY_DIR}/../DakotaDefs.tex"
	    "${CMAKE_CURRENT_BINARY_DIR}/../Dakota.bib"
  COMMAND   "${PDFLATEX_COMPILER}"
  ARGS      -interaction=batchmode "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}"
  COMMENT   "Latex (first pass): Theory"
)

add_custom_command(
  OUTPUT    "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.bbl"
  DEPENDS   "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.aux"
  COMMAND   "${BIBTEX_COMPILER}"
  ARGS      "${file_tex}"
  COMMENT   "Bibtex: Theory"
)

add_custom_command(
  OUTPUT    "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.dvi"
  DEPENDS   "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.bbl"
  COMMAND   "${PDFLATEX_COMPILER}"
  ARGS      -interaction=batchmode "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}"
  COMMENT   "Latex (second pass): Theory"
)

add_custom_command(
  OUTPUT    "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.log"
            "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.pdf"
  DEPENDS   "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.bbl"
            "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.dvi"
  COMMAND   "${PDFLATEX_COMPILER}"
  ARGS      -interaction=batchmode "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}"
  COMMENT   "Latex (third pass): Theory"
)

# Eventually trigger the whole process
add_custom_target(docs-pdf-theory echo
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.pdf
)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
  "Theory_Main.blg;Theory_Main.brf;Theory_Main.idx;Theory_Main.out;Theory_Main.toc"
  )

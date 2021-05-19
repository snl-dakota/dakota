include(AddFileCopyCommand)

# Build a list of absolute paths of latex inputs for use in dependencies
set(users_inputs_abs "") 

# Copy .tex and related files into binary tree (for now), with dep to src
file(GLOB users_tex RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
  "*.tex" "*.bib" "*.sty")
foreach(file ${users_tex}) 
  add_file_copy_command( 
    "${CMAKE_CURRENT_SOURCE_DIR}/${file}" 
    "${CMAKE_CURRENT_BINARY_DIR}/${file}"
    ) 
  list(APPEND users_inputs_abs "${CMAKE_CURRENT_BINARY_DIR}/${file}")
endforeach() 

# Copy the bib file to make latex versions easier to manage
# Really belongs one dir above, but don't know how to manage the dependency
add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/../Dakota.bib
                      ${CMAKE_CURRENT_BINARY_DIR}/../Dakota.bib)


# "Generic" script interface examples in Users_Interface.tex
# Directories in which inputs are found
set(generic_path "${Dakota_SOURCE_DIR}/dakota-examples/official/drivers/bash")
add_file_copy_command("${generic_path}/dakota_rosenbrock.in" 
  "${CMAKE_CURRENT_BINARY_DIR}/dakota_rosenbrock.in")
add_file_copy_command("${generic_path}/simulator_script.sh"
  "${CMAKE_CURRENT_BINARY_DIR}/simulator_script.sh")
add_file_copy_command("${generic_path}/templatedir/ros.template"
  "${CMAKE_CURRENT_BINARY_DIR}/ros.template")
add_custom_command(
  OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/dprepro_usage"
  DEPENDS "${Dakota_SOURCE_DIR}/scripts/pyprepro/dprepro"
  COMMAND "${Python_EXECUTABLE}"
  ARGS    "${Dakota_SOURCE_DIR}/scripts/pyprepro/dprepro" --help > "${CMAKE_CURRENT_BINARY_DIR}/dprepro_usage"
  )

list(APPEND users_inputs_abs
  "${CMAKE_CURRENT_BINARY_DIR}/dakota_rosenbrock.in"
  "${CMAKE_CURRENT_BINARY_DIR}/simulator_script.sh"
  "${CMAKE_CURRENT_BINARY_DIR}/dprepro_usage"
  "${CMAKE_CURRENT_BINARY_DIR}/ros.template"
  )


execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory 
  "${CMAKE_CURRENT_BINARY_DIR}/images")

# Build a list of absolute paths of images for use in dependencies
set(users_images_abs "") 
file(GLOB_RECURSE users_images RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/images"
  "*.png" "*.eps")
foreach(file ${users_images}) 
  add_file_copy_command( 
    "${CMAKE_CURRENT_SOURCE_DIR}/images/${file}"
    "${CMAKE_CURRENT_BINARY_DIR}/images/${file}"
    ) 
  list(APPEND users_images_abs "${CMAKE_CURRENT_BINARY_DIR}/images/${file}")
endforeach() 


# Command with artificial dependencies to run latex commands in sequence
set(file_tex Users_Main)

# TODO: verbosity-based control of latex output
add_custom_command(
  OUTPUT    "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.aux"
  DEPENDS   "${CMAKE_CURRENT_SOURCE_DIR}/${file_tex}.tex" 
            ${users_images_abs} ${users_inputs_abs}
	    examples-users
	    "${CMAKE_CURRENT_BINARY_DIR}/../DakotaDefs.tex"
	    "${CMAKE_CURRENT_BINARY_DIR}/../Dakota.bib"
  COMMAND   "${PDFLATEX_COMPILER}"
  ARGS      -interaction=batchmode "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}"
  COMMENT   "Latex (first pass): User's"
)

add_custom_command(
  OUTPUT    "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.bbl"
  DEPENDS   "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.aux"
  COMMAND   "${BIBTEX_COMPILER}"
  ARGS      "${file_tex}"
  COMMENT   "Bibtex: User's"
)

add_custom_command(
  OUTPUT    "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.dvi"
  DEPENDS   "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.bbl"
  COMMAND   "${PDFLATEX_COMPILER}"
  ARGS      -interaction=batchmode "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}"
  COMMENT   "Latex (second pass): User's"
)

add_custom_command(
  OUTPUT    "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.log"
            "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.pdf"
  DEPENDS   "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.bbl"
            "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.dvi"
  COMMAND   "${PDFLATEX_COMPILER}"
  ARGS      -interaction=batchmode "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}"
  COMMENT   "Latex (third pass): User's"
)

# Eventually trigger the whole process
add_custom_target(docs-pdf-user echo
  DEPENDS   "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.pdf"
)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
  "Users_Main.blg;Users_Main.brf;Users_Main.idx;Users_Main.out;Users_Main.toc"
  )

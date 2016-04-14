include(AddFileCopyCommand)

# Build a list of absolute paths of latex inputs for use in dependencies
set(users_inputs_abs "") 
set(all_generated_files "")

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

# Most input files come from test/ directory
set(test_path    "${Dakota_SOURCE_DIR}/test")

# Each line is "generated file source file test number"
file(STRINGS "${Dakota_BINARY_DIR}/test/dakota_usersexamples.props"
  users_test_inputs)

# Users_Interface.tex
set(ampl_files dakota_ampl_fma.mod dakota_ampl_fma.nl dakota_ampl_fma.col
  dakota_ampl_fma.row dakota_ampl_tb.mod)
foreach (file ${ampl_files})
  add_file_copy_command(${test_path}/${file}
    ${CMAKE_CURRENT_BINARY_DIR}/${file})
  list(APPEND users_inputs_abs ${CMAKE_CURRENT_BINARY_DIR}/${file})
endforeach()

# Users_Advanced_Interfaces.tex
# Directories in which inputs are found
set(generic_path "${Dakota_SOURCE_DIR}/examples/script_interfaces/generic")
add_file_copy_command("${generic_path}/dakota_rosenbrock.in" 
  "${CMAKE_CURRENT_BINARY_DIR}/dakota_rosenbrock.in")
add_file_copy_command("${generic_path}/simulator_script"
  "${CMAKE_CURRENT_BINARY_DIR}/simulator_script")
add_file_copy_command("${generic_path}/templatedir/ros.template"
  "${CMAKE_CURRENT_BINARY_DIR}/ros.template")
add_custom_command(
  OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/dprepro_usage"
  DEPENDS "${Dakota_SOURCE_DIR}/scripts/dprepro"
  COMMAND "${PERL_EXECUTABLE}"
  ARGS    "${Dakota_SOURCE_DIR}/scripts/dprepro" --help > "${CMAKE_CURRENT_BINARY_DIR}/dprepro_usage"
  )

list(APPEND users_inputs_abs
  "${CMAKE_CURRENT_BINARY_DIR}/dakota_rosenbrock.in"
  "${CMAKE_CURRENT_BINARY_DIR}/simulator_script"
  "${CMAKE_CURRENT_BINARY_DIR}/dprepro_usage"
  "${CMAKE_CURRENT_BINARY_DIR}/ros.template"
  )



# Now iterate over the list, copying to build tree
# Also create all_generated_files to use in dependency management
foreach(generated_input ${users_test_inputs})

  # generated a semicolon-separated list from the arguments so we can
  # treat as a list and extract the sub-elements
  separate_arguments(geninput_as_args UNIX_COMMAND ${generated_input})
  list(GET geninput_as_args 0 test_output)
  list(GET geninput_as_args 1 test_input)
  list(GET geninput_as_args 2 test_num)

  add_custom_command(
    OUTPUT  "${test_output}.in"
    COMMAND "${PERL_EXECUTABLE}"
    ARGS    "${test_path}/dakota_test.perl" --extract "${test_path}/${test_input}.in" ${test_num} --file-extract="${CMAKE_CURRENT_BINARY_DIR}/${test_output}.in"
    )
  list(APPEND all_generated_files 
    "${CMAKE_CURRENT_BINARY_DIR}/${test_output}.in")

endforeach()


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
            ${all_generated_files} ${users_images_abs} ${users_inputs_abs}
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

include(AddFileCopyCommand)

# Build a list of absolute paths of latex inputs for use in dependencies
set(users_inputs_abs "") 
set(all_copied_files "")

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

# Most input files come from test/ directory
set(test_path    "${Dakota_SOURCE_DIR}/test")

# Format is generated file, source file, test number
set(users_test_inputs
  "cantilever_opt_npsol dakota_addtnl_users.in 5"
  "cantilever_uq_global_interval dakota_uq_cantilever_interval.in 0"
  "cantilever_uq_sop_rel dakota_uq_cantilever_sop_rel.in 0"
  "container_opt_npsol dakota_container.in 0"
  "cylhead_opt_npsol dakota_addtnl_users.in 4"
  "herbie_shubert_opt_ego dakota_separable_ego_5D.in 0"
  "logratio_uq_reliability dakota_logratio.in 5"
  "mogatest1 dakota_mogatest.in 0"
  "mogatest1_opt_sbo dakota_su_mogatest1.in 0"
  "mogatest2 dakota_mogatest.in 1"
  "mogatest3 dakota_mogatest.in 2"
  "morris_ps_moat dakota_psuade.in 0"
  "qsf_multistart_strat dakota_multistart.in 0"
  "rosen_grad_opt dakota_rosenbrock_users.in 2"
  "rosen_multidim dakota_rosenbrock_users.in 0"
  "rosen_opt_ea dakota_rosenbrock_users.in 5"
  "rosen_opt_ego dakota_rosenbrock_ego.in 0"
  "rosen_opt_nls dakota_rosenbrock_users.in 8"
  "rosen_opt_patternsearch dakota_rosenbrock_users.in 4"
  "rosen_opt_sbo dakota_sbo_rosen.in 0"
  "rosen_opt_scaled dakota_rosenbrock_users.in 9"
  "rosen_ps_vector dakota_rosenbrock_users.in 1"
  "rosen_sampling dakota_rosenbrock_users.in 6"
  "rosen_syscall dakota_rosenbrock_users.in 7"
  "rosen_uq_pce dakota_uq_rosenbrock_pce.in 1"
  "rosen_uq_sc dakota_uq_rosenbrock_sc.in 36"
  "textbook_hybrid_strat dakota_hybrid.in 0"
  "textbook_nls_datafile dakota_textbook_nips.in 7"
  "textbook_opt_conmin dakota_textbook.in 9"
  "textbook_opt_multiobj1 dakota_multiobj1.in 0"
  "textbook_opt_ouu1 dakota_ouu1_tb.in 0"
  "textbook_pareto_strat dakota_pareto.in 0"
  "textbook_uq_glob_evidence dakota_uq_textbook_dste.in 0"
  "textbook_uq_meanvalue dakota_mv.in 0"
  "textbook_uq_sampling dakota_uq_users.in 0"
  "textbook_uq_surrogate dakota_uq_users.in 4"
  )

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
  DEPENDS "${generic_path}/dprepro"
  COMMAND "${PERL_EXECUTABLE}"
  ARGS    "${generic_path}/dprepro" --help > "${CMAKE_CURRENT_BINARY_DIR}/dprepro_usage"
  )

list(APPEND users_inputs_abs
  "${CMAKE_CURRENT_BINARY_DIR}/dakota_rosenbrock.in"
  "${CMAKE_CURRENT_BINARY_DIR}/simulator_script"
  "${CMAKE_CURRENT_BINARY_DIR}/dprepro_usage"
  "${CMAKE_CURRENT_BINARY_DIR}/ros.template"
  )



# Now iterate over the list, copying to build tree
# Also create all_copied_files to use in dependency management
foreach(copied_input ${users_test_inputs})

  # generated a semicolon-separated list from the arguments so we can
  # treat as a list and extract the sub-elements
  separate_arguments(geninput_as_args UNIX_COMMAND ${copied_input})
  list(GET geninput_as_args 0 test_output)
  list(GET geninput_as_args 1 test_num)
  list(GET geninput_as_args 2 test_input)

  add_file_copy_command(
    ${test_path}/dakota_users_${test_output}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.in
    )
  list(APPEND all_copied_files 
    ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.in)

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
            ${all_copied_files} ${users_images_abs} ${users_inputs_abs}
  COMMAND   "${PDFLATEX_COMPILER}"
  ARGS      -interaction=batchmode "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}"
  COMMENT   "Latex (first pass): User's"
)

add_custom_command(
  OUTPUT    "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.bbl"
  DEPENDS   "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.aux"
  COMMAND   "${BIBTEX_COMPILER}"
  ARGS      -terse "${CMAKE_CURRENT_BINARY_DIR}/${file_tex}"
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

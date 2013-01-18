
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

# Create generated files into binary tree; use a list users_test_inputs with 
#   test_input test_num test_output

# Directories in which inputs are found
set(generic_path "${Dakota_SOURCE_DIR}/examples/script_interfaces/generic")
set(test_path    "${Dakota_SOURCE_DIR}/test")

# Users_Tutorial.tex
set(users_test_inputs
  "${test_path}/dakota_rosenbrock_users.in 0 rosen_multidim.in"
  "${test_path}/dakota_rosenbrock_users.in 1 dakota_rosenbrock_vector.in"
  "${test_path}/dakota_rosenbrock_users.in 2 rosen_grad_opt.in"
  "${test_path}/dakota_textbook.in 9 dakota_textbook.in"
  "${test_path}/dakota_rosenbrock_users.in 8 dakota_rosenbrock_ls.in"
  "${test_path}/dakota_rosenbrock_users.in 4 dakota_rosenbrock_ps_opt.in"
  "${test_path}/dakota_rosenbrock_users.in 5 dakota_rosenbrock_ea_opt.in"
  "${test_path}/dakota_mogatest.in 0 dakota_mogatest1.in"
  "${test_path}/dakota_rosenbrock_users.in 6 rosen_sampling.in"
  "${test_path}/dakota_mv.in 0 dakota_mv.in"
  "${test_path}/dakota_uq_rosenbrock_pce.in 1 dakota_pce.in"
  "${test_path}/dakota_uq_cantilever_interval.in 0 dakota_uq_interval.in"
  "${test_path}/dakota_rosenbrock_users.in 7 rosen_syscall.in"
)

# Users_Method_UQ.tex
set(users_test_inputs ${users_test_inputs}
  "${test_path}/dakota_uq_users.in 0 dakota_uq_sampling.in"
  "${test_path}/dakota_logratio.in 5 dakota_uq_reliability.in"
  "${test_path}/dakota_uq_rosenbrock_sc.in 36 dakota_sc.in"
  "${test_path}/dakota_uq_textbook_dste.in 0 dakota_uq_textbook_dste.in"
)

# Users_Method_Opt.tex
set(users_test_inputs ${users_test_inputs}
  "${test_path}/dakota_multiobj1.in 0 dakota_multiobj1.in"
  "${test_path}/dakota_rosenbrock_users.in 9 dakota_rosenbrock_scaled.in"
)

# Users_Models.tex
set(users_test_inputs ${users_test_inputs}
  "${test_path}/dakota_sbo_rosen.in 0 dakota_sbo_rosen.in"
  "${test_path}/dakota_su_mogatest1.in 0 dakota_su_mogatest1.in"
  "${test_path}/dakota_rosenbrock_ego.in 0 dakota_rosenbrock_ego.in"
)

# Users_Strategy.tex
set(users_test_inputs ${users_test_inputs}
  "${test_path}/dakota_hybrid.in 0 dakota_hybrid.in"
  "${test_path}/dakota_multistart.in 0 dakota_multistart.in"
  "${test_path}/dakota_pareto.in 0 dakota_pareto.in"
  "${test_path}/dakota_uq_cantilever_sop_rel.in 0 dakota_uq_cantilever_sop_rel.in"
  "${test_path}/dakota_ouu1_tb.in 0 dakota_ouu1_tb.in"
)

# Users_Output.tex
set(users_test_inputs ${users_test_inputs}
  "${test_path}/dakota_container.in 0 dakota_container.in"
)

# Users_Advanced_Interfaces.tex
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

# Users_Additional_Examples.tex
set(users_test_inputs ${users_test_inputs}
  "${test_path}/dakota_addtnl_users.in 0 dakota_addtnl_rosen_opt.in"
  "${test_path}/dakota_addtnl_users.in 1 dakota_addtnl_rosen_ls.in"
  "${test_path}/dakota_addtnl_users.in 4 dakota_addtnl_cylhead.in"
  "${test_path}/dakota_addtnl_users.in 5 dakota_addtnl_cantilever.in"
  "${test_path}/dakota_mogatest.in 1 dakota_mogatest2.in"
  "${test_path}/dakota_mogatest.in 2 dakota_mogatest3.in"
)
add_file_copy_command("${test_path}/dakota_separable_ego_5D.in"
  "${CMAKE_CURRENT_BINARY_DIR}/dakota_separable_ego_5D.in")
list(APPEND users_inputs_abs 
  "${CMAKE_CURRENT_BINARY_DIR}/dakota_separable_ego_5D.in")

# Now iterate over the list, parsing and extracting with dakota_test.perl.
# Also create all_generated_inputs to use in dependency management
set(all_generated_inputs)
foreach(generated_input ${users_test_inputs})

  # generated a semicolon-separated list from the arguments so we can
  # treat as a list and extract the sub-elements
  separate_arguments(geninput_as_args UNIX_COMMAND "${generated_input}")
  list(GET geninput_as_args 0 test_input)
  list(GET geninput_as_args 1 test_num)
  list(GET geninput_as_args 2 test_output)
  list(APPEND all_generated_inputs
    "${CMAKE_CURRENT_BINARY_DIR}/${test_output}")

  add_custom_command(
    OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/${test_output}"
    DEPENDS "${test_input}"
    COMMAND "${PERL_EXECUTABLE}"
    ARGS    "${Dakota_SOURCE_DIR}/test/dakota_test.perl" --extract ${test_input} ${test_num} --file-extract="${CMAKE_CURRENT_BINARY_DIR}/${test_output}"
  )

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
            ${all_generated_inputs} ${users_images_abs} ${users_inputs_abs}
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

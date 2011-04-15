# TODO
# * Consider using file copy instead of configure_file, when config not needed?
# * Create proper make clean information
# * Generate files with deps to the source tree so you can update there and
#   make in the binary tree

# Copy tex inputs into binary tree for now
file(GLOB users_tex ${CMAKE_CURRENT_SOURCE_DIR}
     "*.tex" "*.bib" "*.sty")
file(COPY ${users_tex} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


# Create generated files into binary tree; use a list users_test_inputs with 
#   test_input test_num test_output

# Directories in which inputs are found
set(generic_path ${Dakota_SOURCE_DIR}/examples/script_interfaces/generic)
set(test_path    ${Dakota_SOURCE_DIR}/test)

# Users_Tutorial.tex
set(users_test_inputs
  "${test_path}/dakota_rosenbrock_users.in 0 dakota_rosenbrock_2d.in"
  "${test_path}/dakota_rosenbrock_users.in 1 dakota_rosenbrock_vector.in"
  "${test_path}/dakota_rosenbrock_users.in 2 dakota_rosenbrock_grad_opt.in"
  "${test_path}/dakota_textbook.in 9 dakota_textbook.in"
  "${test_path}/dakota_rosenbrock_users.in 8 dakota_rosenbrock_ls.in"
  "${test_path}/dakota_rosenbrock_users.in 4 dakota_rosenbrock_ps_opt.in"
  "${test_path}/dakota_rosenbrock_users.in 5 dakota_rosenbrock_ea_opt.in"
  "${test_path}/dakota_mogatest.in 0 dakota_mogatest1.in"
  "${test_path}/dakota_rosenbrock_users.in 6 dakota_rosenbrock_nond.in"
  "${test_path}/dakota_mv.in 0 dakota_mv.in"
  "${test_path}/dakota_uq_rosenbrock_pce.in 1 dakota_pce.in"
  "${test_path}/dakota_uq_cantilever_interval.in 0 dakota_uq_interval.in"
  "${test_path}/dakota_rosenbrock_users.in 7 dakota_rosenbrock_syscall.in"
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
# TODO: consider file(COPY) instead?
configure_file("${generic_path}/dakota_rosenbrock.in"
	       dakota_rosenbrock.in COPYONLY)
configure_file("${generic_path}/simulator_script"
	       simulator_script COPYONLY)
# TODO: head -47 of dprepro
configure_file("${generic_path}/dprepro"
	       dprepro_head COPYONLY)
configure_file("${generic_path}/templatedir/ros.template"
	       ros.template COPYONLY)

# Users_Additional_Examples.tex
set(users_test_inputs ${users_test_inputs}
  "${test_path}/dakota_addtnl_users.in 0 dakota_addtnl_rosen_opt.in"
  "${test_path}/dakota_addtnl_users.in 1 dakota_addtnl_rosen_ls.in"
  "${test_path}/dakota_addtnl_users.in 4 dakota_addtnl_cylhead.in"
  "${test_path}/dakota_addtnl_users.in 5 dakota_addtnl_cantilever.in"
  "${test_path}/dakota_mogatest.in 1 dakota_mogatest2.in"
  "${test_path}/dakota_mogatest.in 2 dakota_mogatest3.in"
)


# Now iterate over the list, parsing and extracting with dakota_test.perl.
# Also create all_generated_inputs to use in dependency management
set(all_generated_inputs)
foreach(generated_input ${users_test_inputs})

  # generated a semicolon-separated list from the arguments so we can
  # treat as a list and extract the sub-elements
  separate_arguments(geninput_as_args UNIX_COMMAND ${generated_input})
  list(GET geninput_as_args 0 test_input)
  list(GET geninput_as_args 1 test_num)
  list(GET geninput_as_args 2 test_output)
  list(APPEND all_generated_inputs ${test_output})

  # TODO: generate inputs only in build tree!!!
  #       currently pollutes source tree's test
  #       requires changing perl script behavior
  add_custom_command(
    OUTPUT  ${test_output}
    COMMAND ${Dakota_SOURCE_DIR}/test/dakota_test.perl 
    ARGS    extract ${test_input} ${test_num} 
    COMMAND ${CMAKE_COMMAND} 
    ARGS -E copy ${test_input}_ ${CMAKE_CURRENT_BINARY_DIR}/${test_output}
  )

endforeach()

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
set(file_tex Users_Main)

add_custom_command(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.aux
  DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/${file_tex}.tex 
            ${all_generated_inputs} ${users_images}
  COMMAND   ${PDFLATEX_COMPILER}
  ARGS      -interaction=batchmode ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}
  COMMENT   "Latex (first pass): User's"
)

add_custom_command(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.bbl
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.aux
  COMMAND   ${BIBTEX_COMPILER}
  ARGS      -terse ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}
  COMMENT   "Bibtex: User's"
)

add_custom_command(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.dvi
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.bbl
  COMMAND   ${PDFLATEX_COMPILER}
  ARGS      -interaction=batchmode ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}
  COMMENT   "Latex (second pass): User's"
)

add_custom_command(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.log
            ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.pdf
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.bbl
            ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.dvi
  COMMAND   ${PDFLATEX_COMPILER}
  ARGS      -interaction=batchmode ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}
  COMMENT   "Latex (third pass): User's"
)

# Eventually trigger the whole process
add_custom_target(docs-pdf-user echo
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/${file_tex}.pdf
)

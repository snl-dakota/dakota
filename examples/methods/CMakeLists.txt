# Extract files for examples/methods

set(test_path ${Dakota_SOURCE_DIR}/test)

# Users_Method_UQ.tex
set(methods_test_inputs
  "${test_path}/dakota_uq_users.in 0 dakota_uq_sampling.in"
  "${test_path}/dakota_logratio.in 5 dakota_uq_reliability.in"
  "${test_path}/dakota_uq_rosenbrock_sc.in 36 dakota_sc.in"
  "${test_path}/dakota_uq_textbook_dste.in 0 dakota_uq_textbook_dste.in"
)

# Users_Method_Opt.tex
set(methods_test_inputs ${methods_test_inputs}
  "${test_path}/dakota_multiobj1.in 0 dakota_multiobj1.in"
  "${test_path}/dakota_rosenbrock_users.in 9 dakota_rosenbrock_scaled.in"
)

# Users_Models.tex
set(methods_test_inputs ${methods_test_inputs}
  "${test_path}/dakota_sbo_rosen.in 0 dakota_sbo_rosen.in"
  "${test_path}/dakota_su_mogatest1.in 0 dakota_su_mogatest1.in"
  "${test_path}/dakota_rosenbrock_ego.in 0 dakota_rosenbrock_ego.in"
)

# Users_Strategy.tex
set(methods_test_inputs ${methods_test_inputs}
  "${test_path}/dakota_hybrid.in 0 dakota_hybrid.in"
  "${test_path}/dakota_multistart.in 0 dakota_multistart.in"
  "${test_path}/dakota_pareto.in 0 dakota_pareto.in"
  "${test_path}/dakota_uq_cantilever_sop_rel.in 0 dakota_uq_cantilever_sop_rel.in"
  "${test_path}/dakota_ouu1_tb.in 0 dakota_ouu1_tb.in"
)

# Users_Output.tex
set(methods_test_inputs ${methods_test_inputs}
  "${test_path}/dakota_container.in 0 dakota_container.in"
)

# Users_Additional_Examples.tex
set(methods_test_inputs ${methods_test_inputs}
  "${test_path}/dakota_addtnl_users.in 0 dakota_addtnl_rosen_opt.in"
  "${test_path}/dakota_addtnl_users.in 1 dakota_addtnl_rosen_ls.in"
  "${test_path}/dakota_addtnl_users.in 4 dakota_addtnl_cylhead.in"
  "${test_path}/dakota_addtnl_users.in 5 dakota_addtnl_cantilever.in"
  "${test_path}/dakota_mogatest.in 1 dakota_mogatest2.in"
  "${test_path}/dakota_mogatest.in 2 dakota_mogatest3.in"
)

# Not in User's manual
set(methods_test_inputs ${methods_test_inputs}
  "${test_path}/dakota_textbook_nips.in 7 dakota_nls_datafile.in"
  "${test_path}/dakota_uq_users.in 4 dakota_surr_uq.in"
)

# Now iterate over the list, parsing and extracting with dakota_test.perl.
# Also create all_generated_inputs to use in dependency management
set(all_generated_inputs)
foreach(generated_input ${methods_test_inputs})

  # generated a semicolon-separated list from the arguments so we can
  # treat as a list and extract the sub-elements
  separate_arguments(geninput_as_args UNIX_COMMAND ${generated_input})
  list(GET geninput_as_args 0 test_input)
  list(GET geninput_as_args 1 test_num)
  list(GET geninput_as_args 2 test_output)
  list(APPEND all_generated_inputs ${CMAKE_CURRENT_BINARY_DIR}/${test_output})

  add_custom_command(
    OUTPUT  ${test_output}
    COMMAND ${Dakota_SOURCE_DIR}/test/dakota_test.perl 
    ARGS    --extract ${test_input} ${test_num} --file-extract=${CMAKE_CURRENT_BINARY_DIR}/${test_output}
  )

endforeach()

add_custom_target(examples-methods-inputs ALL DEPENDS ${all_generated_inputs})

install(FILES ${all_generated_inputs} 
  ${test_path}/dakota_textbook_nips.lsq.dat 
  ${test_path}/dakota_separable_ego_5D.in
  DESTINATION examples/methods)

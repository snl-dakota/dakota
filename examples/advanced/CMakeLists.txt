# Extract files for examples/advanced
#   dakota_dace.in #0
#   dakota_hybrid.in #0 (Note: replaces dakota_textbook_hybrid.in)
#   dakota_uq_cantilever_ivp_exp.in #4
#   dakota_sbo_hierarchical #0
#   dakota_pcbdo_cantilever.in #4
#   dakota_uq_cantilever_adapt_gsg.in #8

set(test_path ${Dakota_SOURCE_DIR}/test)

set(cantilever_test_inputs
  "${test_path}/dakota_dace.in 0 textbook_dace_parallel.in"
  "${test_path}/dakota_hybrid.in 0 textbook_hybrid_strat.in"
  "${test_path}/dakota_uq_cantilever_ivp_exp.in 4 cantilever_uq_ivp_exp.in"
  "${test_path}/dakota_sbo_hierarchical.in 0 rosenbrock_sbo_hierarchical.in"
  "${test_path}/dakota_pcbdo_cantilever.in 5 cantilever_pcbdo.in"
  "${test_path}/dakota_uq_cantilever_adapt_gsg.in 8 cantilever_uq_adapt_gsg.in"
)

# Now iterate over the list, parsing and extracting with dakota_test.perl.
# Also create all_generated_inputs to use in dependency management
set(all_generated_inputs)
foreach(generated_input ${cantilever_test_inputs})

  # generated a semicolon-separated list from the arguments so we can
  # treat as a list and extract the sub-elements
  separate_arguments(geninput_as_args UNIX_COMMAND ${generated_input})
  list(GET geninput_as_args 0 test_input)
  list(GET geninput_as_args 1 test_num)
  list(GET geninput_as_args 2 test_output)
  list(APPEND all_generated_inputs ${CMAKE_CURRENT_BINARY_DIR}/${test_output})

  add_custom_command(
    DEPENDS "${test_input}"
    OUTPUT  ${test_output}
    COMMAND ${PERL_EXECUTABLE} ${Dakota_SOURCE_DIR}/test/dakota_test.perl 
    ARGS    --extract ${test_input} ${test_num} --file-extract=${CMAKE_CURRENT_BINARY_DIR}/${test_output}
  )

endforeach()

add_custom_target(examples-advanced ALL 
  DEPENDS ${all_generated_inputs})

install(FILES ${all_generated_inputs}
  DESTINATION "${DAKOTA_EXAMPLES_INSTALL}/examples/advanced")

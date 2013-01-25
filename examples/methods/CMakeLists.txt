# Extract files for examples/methods

set(test_path ${Dakota_SOURCE_DIR}/test)

# Users_Method_UQ.tex
set(methods_test_inputs
  "${test_path}/dakota_uq_users.in 0 dakota_uq_sampling"
  "${test_path}/dakota_logratio.in 5 dakota_uq_reliability"
  "${test_path}/dakota_uq_rosenbrock_sc.in 36 dakota_sc"
  "${test_path}/dakota_uq_textbook_dste.in 0 dakota_uq_textbook_dste"
)

# Users_Method_PStudy.tex
set(methods_test_inputs ${methods_test_inputs}
  "${test_path}/dakota_rosenbrock_users.in 1 dakota_rosenbrock_vector"
)

# Users_Method_UQ.tex
set(methods_test_inputs ${methods_test_inputs}
  "${test_path}/dakota_uq_users.in 0 dakota_uq_sampling"
  "${test_path}/dakota_logratio.in 5 dakota_uq_reliability"
  "${test_path}/dakota_uq_rosenbrock_sc.in 36 dakota_sc"
  "${test_path}/dakota_uq_textbook_dste.in 0 dakota_uq_textbook_dste"
  "${test_path}/dakota_mv.in 0 dakota_mv"
  "${test_path}/dakota_uq_rosenbrock_pce.in 1 dakota_pce"
  "${test_path}/dakota_uq_cantilever_interval.in 0 dakota_uq_interval"
)

# Users_Method_Opt.tex
set(methods_test_inputs ${methods_test_inputs}
  "${test_path}/dakota_multiobj1.in 0 dakota_multiobj1"
  "${test_path}/dakota_rosenbrock_users.in 9 dakota_rosenbrock_scaled"
  "${test_path}/dakota_rosenbrock_users.in 4 dakota_rosenbrock_ps_opt"
  "${test_path}/dakota_rosenbrock_users.in 5 dakota_rosenbrock_ea_opt"
)

# Users_Method_SurrBased.tex
set(methods_test_inputs ${methods_test_inputs}
  "${test_path}/dakota_sbo_rosen.in 0 dakota_sbo_rosen"
  "${test_path}/dakota_su_mogatest1.in 0 dakota_su_mogatest1"
  "${test_path}/dakota_rosenbrock_ego.in 0 dakota_rosenbrock_ego"
)

# Users_Output.tex
set(methods_test_inputs ${methods_test_inputs}
  "${test_path}/dakota_container.in 0 dakota_container"
)

# Users_Advanced_Strategies.tex
set(methods_test_inputs ${methods_test_inputs}
  "${test_path}/dakota_hybrid.in 0 dakota_hybrid"
  "${test_path}/dakota_multistart.in 0 dakota_multistart"
  "${test_path}/dakota_pareto.in 0 dakota_pareto"

)

# Users_Advanced_Models.txt
set(methods_test_inputs ${methods_test_inputs}
  "${test_path}/dakota_uq_cantilever_sop_rel.in 0 dakota_uq_cantilever_sop_rel"
  "${test_path}/dakota_ouu1_tb.in 0 dakota_ouu1_tb"
)

# Users_Additional_Examples.tex
set(methods_test_inputs ${methods_test_inputs}
  "${test_path}/dakota_addtnl_users.in 0 dakota_addtnl_rosen_opt"
  "${test_path}/dakota_addtnl_users.in 1 dakota_addtnl_rosen_ls"
  "${test_path}/dakota_separable_ego_5D.in 0 dakota_separable_ego_5D"
  "${test_path}/dakota_addtnl_users.in 4 dakota_addtnl_cylhead"
  "${test_path}/dakota_addtnl_users.in 5 dakota_addtnl_cantilever"
  "${test_path}/dakota_mogatest.in 1 dakota_mogatest2"
  "${test_path}/dakota_mogatest.in 2 dakota_mogatest3"
  "${test_path}/dakota_textbook.in 9 dakota_textbook"
  "${test_path}/dakota_rosenbrock_users.in 8 dakota_rosenbrock_ls"
  "${test_path}/dakota_mogatest.in 0 dakota_mogatest1"
)

# Not in User's manual (likely should be)
set(methods_test_inputs ${methods_test_inputs}
  "${test_path}/dakota_textbook_nips.in 7 dakota_nls_datafile"
  "${test_path}/dakota_uq_users.in 4 dakota_surr_uq"
  "${test_path}/dakota_psuade.in 0 dakota_psuade"
)

# Supplementary data files needed and copied files below
add_file_copy_command(${test_path}/dakota_textbook_nips.lsq.dat 
  ${CMAKE_CURRENT_BINARY_DIR}/dakota_textbook_nips.lsq.dat)

set(all_copied_files ${CMAKE_CURRENT_BINARY_DIR}/dakota_textbook_nips.lsq.dat)

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
  list(APPEND all_generated_inputs ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.in)

  add_custom_command(
    OUTPUT  ${test_output}.in
    COMMAND ${PERL_EXECUTABLE} ${Dakota_SOURCE_DIR}/test/dakota_test.perl 
    ARGS    --extract ${test_input} ${test_num} --file-extract=${CMAKE_CURRENT_BINARY_DIR}/${test_output}.in
  )

  # Copy all .sav files from source to binary tree, avoid glob to get deps
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.out.sav")
    add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.out.sav
      ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.out.sav)
    list(APPEND all_copied_files 
      ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.out.sav)
  endif()
  if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.stdout.sav)
    add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.stdout.sav
      ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.stdout.sav)
    list(APPEND all_copied_files 
      ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.stdout.sav)
  endif()
  if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.dat.sav)
    add_file_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.dat.sav
      ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.dat.sav)
    list(APPEND all_copied_files 
      ${CMAKE_CURRENT_BINARY_DIR}/${test_output}.dat.sav)
  endif()

endforeach()

add_custom_target(examples-methods ALL 
  DEPENDS ${all_generated_inputs} ${all_copied_files})

install(FILES ${all_generated_inputs} ${all_copied_files}
  DESTINATION "${DAKOTA_EXAMPLES_INSTALL}/examples/methods")

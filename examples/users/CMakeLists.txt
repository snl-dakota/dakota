# Extract files for examples/methods

set(test_path ${Dakota_SOURCE_DIR}/test)

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

# Supplementary data files needed and copied files below
add_file_copy_command(${test_path}/dakota_users_textbook_nls_datafile.lsq.dat
  ${CMAKE_CURRENT_BINARY_DIR}/dakota_users_textbook_nls_datafile.lsq.dat)

set(all_copied_files 
  ${CMAKE_CURRENT_BINARY_DIR}/dakota_users_textbook_nls_datafile.lsq.dat)

# Now iterate over the list, copying to build tree
# Also create all_generated_inputs to use in dependency management
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
 
  # Copy all .sav files from source to binary tree, avoid glob to get deps
  if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${test_output}.out.sav)
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

add_custom_target(examples-users ALL DEPENDS ${all_copied_files})

install(FILES ${all_copied_files}
  DESTINATION "${DAKOTA_EXAMPLES_INSTALL}/examples/users")

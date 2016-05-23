# CMakeLists.txt for DAKOTA tests

if( NOT WIN32 )

  set(aux_files)
  set(aux_files " "
    )

  set(extra_files)
  set(extra_files " ")

  set(input_files dakota_qsf_egra.in dakota_qsf_mpp.in dakota_qsf_mv.in dakota_qsf_pce.in dakota_qsf_sampling.in)

  foreach(infile ${input_files})
    add_training_test(${infile} ${aux_files} ${extra_files} FALSE)
  endforeach()

  set(training_exe_deps)
  DakotaCopyTarget(dakota training_exe_deps)
  DakotaCopyTarget(quasi_sine_fcn training_exe_deps)

  # Create a single target to copy all the testing files
  add_custom_target(uncertainty_analysis_training_test_files ALL
    DEPENDS ${input_files} ${training_exe_deps}
    COMMENT
    "Copy DAKOTA training test files, and executables to binary subdirectories"
    )

endif()

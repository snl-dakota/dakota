# CMakeLists.txt for DAKOTA tests

if( NOT WIN32 )

  set(aux_files)
  set(aux_files " "
    )

  set(extra_files)
  set(extra_files " ")

  add_training_test(dakota_opt_quasisine.in ${aux_files} ${extra_files} TRUE)

  set(training_exe_deps)
  DakotaCopyTarget(quasi_sine_fcn training_exe_deps)

  # Create a single target to copy all the testing files
  add_custom_target(optimization_training_test_files ALL
    DEPENDS ${input_files} ${training_exe_deps}
    COMMENT
    "Copy DAKOTA trainingtest files, and executables to binary subdirectories"
    )

endif()

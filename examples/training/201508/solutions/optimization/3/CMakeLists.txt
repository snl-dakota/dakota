# CMakeLists.txt for DAKOTA tests

set(aux_files)
set(aux_files " "
  )

set(extra_files)
set(extra_files " ")

add_training_test(dakota_opt_quasisine.in ${aux_files} ${extra_files})

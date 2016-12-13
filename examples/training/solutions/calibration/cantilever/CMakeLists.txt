# CMakeLists.txt for DAKOTA tests

set(aux_files)
set(aux_files "cantilever cantilever.template driver.sh"
  )

set(extra_files)
set(extra_files "cant_exp_data.5perc.50.dat"
  )

add_training_test(dakota_cantilever_nl2sol.in ${aux_files} ${extra_files} TRUE)

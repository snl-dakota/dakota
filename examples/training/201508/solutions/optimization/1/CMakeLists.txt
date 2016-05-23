# CMakeLists.txt for DAKOTA tests

set(aux_files)
set(aux_files "cantilever cantilever.template"
  )

# Note we use the driver from the solutions subdir and not from exercises 
set(extra_files)
set(extra_files "driver.sh")

add_training_test(dakota_cantilever_nominal.in ${aux_files} ${extra_files} TRUE)


# Thhe grid problem is having issues
#set(aux_files)
#set(aux_files "cantilever cantilever.template driver.sh"
#  )
#
#set(extra_files)
#set(extra_files " ")
#
#add_training_test(dakota_grid_cantilever.in ${aux_files} ${extra_files})

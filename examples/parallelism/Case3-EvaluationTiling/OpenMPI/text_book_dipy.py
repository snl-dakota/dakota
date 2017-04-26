#!/usr/bin/env python
import sys
import shutil
from dipy import parallel, read_parameters_file

## Use the dipy.parallel module to run text_book_simple_par in parallel

params_file=sys.argv[1]
results_file=sys.argv[2]

## Pre-processing
shutil.copy(params_file, "application.in")

## Run
# Construct the command. Because multiple tiles fit on each node, use the mpirun
# option '--bind-to none' to allow tasks to be spread out properly on each node.
# see the mpirun manpage for more information.

command = (2, # Use two tasks
        # command must be tokenized
        ["--bind-to none",
            "text_book_simple_par", 
            "application.in", 
            "application.out"]
        )

# Run using dynamic scheduling
parallel.tile_run_dynamic(commands=[command])

# If static scheduling is desired, add 'local_evaluation_scheduling static' to
# the Dakota input and uncomment:
#parallel.tile_run_static(commands=[command], parameters_file=params_file)


## Post-process
shutil.copy("application.out", results_file)


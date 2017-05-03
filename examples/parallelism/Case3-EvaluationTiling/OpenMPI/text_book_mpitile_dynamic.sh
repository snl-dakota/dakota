#!/bin/bash

# Bash script to run text_book_par in parallel on an available set of
# MPI "tasks" within an allocated job. The total set of tasks in a job is divided
# up into tiles, each containing the number needed to run the user's executable.

# The script assumes a dynamic scheduling model. Under this model, Dakota launches 
# the next available evaluation without regard for the number of the evaluation just
# completed. Unlike the static scheduling case, it is impossible for the analysis driver
# to simply determine which nodes are available. The mpitile wrapper script uses lock 
# files to keep track of which tiles are currently in use. This is largely transparent
# to the user.

# The names of the Dakota parameters and results files are provided as command
# line arguments
params=$1
results=$2

# -------------------------
# INPUT FILE PRE-PROCESSING
# -------------------------

# This demo does not need file pre-processing, but normally (see
# below) APREPRO or DPREPRO is used to "cut-and-paste" data from the
# params.in.# file written by DAKOTA into the template input file for
# the user's simulation code.

# aprepro run6crh_rigid_template.i temp_rigid.new
# grep -vi aprepro temp_rigid.new > run6crh_rigid.i

# dprepro $1 application_input.template application.in 

# For this example we just prepare the application input by copying
# the parameters:
cp $params application.in

# -------------------
# RUN SIMULATION CODE
# -------------------

echo "$0 running text_book_par on 2 processors."

# mpitile the command.
# Because there are multiple tiles per node (as opposed to 1 tile per node or 
# multiple nodes per tile), it may be wise to specify '--bind-to none' to mpirun
# to allow the tasks to float from processor to processor for load balancing.
# Otherwise, multiple MPI processes may be bound to a single core. This behavior
# depends on your OpenMPI version; it is best to consult the documentation.

mpitile -np 2 --bind-to none text_book_par \
    application.in application.out

# mpitile also supports static scheduling. Use the --static option and provide
# either the evaluation number or name of the parameters file.
# mpitile -np 2 --static --params-file $params --bind-to none \
#	text_book_par application.in application.out

# use sleep command if file I/O timing is a problem. This most often
# manifests as Dakota complaining that it can't find a results file.
#sleep 10


# ---------------------------
# OUTPUT FILE POST PROCESSING
# ---------------------------

# Normally any application-specific post-processing to prepare the
# results.out file for Dakota would go here. Here we'll substitute a
# copy command:

cp application.out $results


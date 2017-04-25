#!/bin/bash

# Bash script to run text_book_simple_par in parallel using the Dakota-provided
# mpitile.

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
# TODO: instead of repeating the driver in every directory, could we just have
#       a separate mpi_launch_app.sh in each dir?
# -------------------

echo "$0 running text_book_simple_par on 2 processors."

# For dynamic scheduling:
mpitile -np 2 text_book_simple_par application.in application.out

# For static scheduling, add 'local_evaluation_scheduling static' to the Dakota 
# input and run:
#mpitile --static --params-file=$params -np 2 text_book_simple_par \
#	application.in application.out

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


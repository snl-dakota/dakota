#!/bin/bash

# Bash script to run text_book_par in parallel on an available set of
# MPI "tasks" within an allocated job. The total set of tasks in a job is divided
# up into tiles, each containing the number needed to run the user's executable.

# The script assumes a static scheduling model. Under this model, when
# evaluation number N completes, Dakota launches evaluation N + evaluation_concurrency.
# As such, the tile numbered N % evaluation_concurrency can assumed to be available. 
# Static scheduling of asynchronous evaluations is enabled in Dakota with the keywords
# 'local_evaluation_scheduling static'.

# The names of the Dakota parameters and results files are provided as command
# line arguments
params=$1
results=$2

# Extract the evaluation number from the parameters file name (assumes the file_tag
# keyword is present in the Dakota input file.
num=$(echo $params | awk -F. '{print $NF}')

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


# !!! Requires that APPLIC_PROCS either divide evenly into PPN or be
# !!! an integer multiple of it

# number of concurrent jobs (must agree with Dakota evaluation_concurrency)
CONCURRENCY=8

# number of processes (or tasks) per node (with SLURM could use
# $SLURM_CPUS_ON_NODE or $SLURM_TASKS_PER_NODE if appropriate)
PPN=4

# number of processes (tasks) per application job
APPLIC_PROCS=2

# number of nodes needed per application job ( ceil(APPLIC_PROCS/PPN) )
applic_nodes=$(( ($APPLIC_PROCS+$PPN-1) / $PPN )) 

# relative node on which the job should schedule (floor)
# this is the first of the node block for this job
relative_node=$(( (num - 1) % CONCURRENCY * APPLIC_PROCS / PPN ))

# RESERVE a node for Dakota (may be needed if the analysis_driver is 
# memory intensivs. Assumes Dakota starts on the zeroth node in the 
# allocation (true for SLURM); some queue systems run the job submission
# script (and hence Dakota) on the last node. In this case, you need not 
# do anything special to these calcs, just add a node to the batch request.

# NOTE: it's not trivial to reserve a processor for Dakota in this
# case (due to -N1-1), though easy to reserve a _node_.  (must allow
# one extra node in submission).  It is easy to reserve one CPU for
# Dakota if the analysis requries only one CPU.
##relative_node=$(( (num - 1) % CONCURRENCY * APPLIC_PROCS / PPN + 1 ))

# build a node list to use as the argument to the -host option. This 
# will look something like, "+n0,+n1,+n2,..."
node_list="+n${relative_node}"
for node_increment in `seq 1 $((applic_nodes - 1))`; do
  node_list="$node_list,+n$((relative_node + node_increment))"
done

# mpirun the command with the -host list. 
# Because there are multiple tiles per node (as opposed to 1 tile per node or 
# multiple nodes per tile), it may be wise to specify '--bind-to none' to mpirun
# to allow the tasks to float from processor to processor for load balancing.
# Otherwise, multiple MPI processes may be bound to a single core. This behavior
# depends on your OpenMPI version; it is best to consult the documentation.
mpirun -np $APPLIC_PROCS -host $node_list --bind-to none text_book_par \
    application.in application.out

# TODO: openmpi and/or srun exclusive mode should allow mpiexec-like
# behavior, but haven't gotten working

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


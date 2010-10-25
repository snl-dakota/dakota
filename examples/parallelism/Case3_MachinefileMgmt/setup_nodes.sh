#!/bin/sh

# This script parses the PBS node file and divides the nodes up into
# unique subsets of nodes, each containing enough nodes for a single
# Applic run.  This needs to be done in order to prevent
# mpiexec/mpirun from assigning multiple Applic runs to the same
# processors.

# uses global variables
# APPLIC_PROCS 
# CONCURRENCY

if [ $# -lt 1 ]; then
  echo "usage: $0 NodeFile"
  exit
fi

nodefile=$1

# The following determines the total number of nodes allocated by
# checking the PBS node file generated upon qsub.  Assumes one
# resource per line
if [ -f $nodefile ]; then
   procs_avail=$[0+$(wc -l < $nodefile)]
else
   procs_avail=0
fi

# check that CONCURRENCY * APPLIC_PROCS + 1 <= M
# i.e., the job will fit within the processor allocation
# when static scheduling is used, could modify to 
# CONCURRENCY * APPLIC_PROCS + 1 <= M
procs_needed=`expr $APPLIC_PROCS \* $CONCURRENCY + 1`

if [ $procs_needed -gt $procs_avail ];
then
   echo "Error: Number of processors in PBS allocation must be at least"
   echo "       CONCURRENCY * APPLIC_PROCS + 1 = ${procs_needed},"
   echo "       but only $procs_avail are available."
   exit
fi

# remove any previously created node files.
rm -f dakotaNodeFile applicNodeFile.*

# dakota will run in serial on the current node
dakota_node=`hostname`
# flag indicating whether we've found at least one processor on the dakota_node
dakota_node_done=0  

# initialize some counter variables:
node_set=1    # index to unique node sets
node_count=0  # number of nodes within a set

# name of application node file
applic_nodefile=applicNodeFile.$node_set  

# Read in and process each line of the PBS node file, until empty or enough 
# application node files are created

while [ $node_set -le $CONCURRENCY ] && read LINE
do

  if [ "$LINE" == "$dakota_node" ] && [ $dakota_node_done -eq 0 ]
  then
    # Create dakotaNodeFile for record-keeping, even though unused.
    echo $LINE >> dakotaNodeFile
    dakota_node_done=1
  else 
    # If it is not the current node, add it to the current application node
    # file and update the node counter.
    echo $LINE >> $applic_nodefile
    node_count=`expr $node_count + 1`
  fi

  if [ $node_count -ge $APPLIC_PROCS ]
  then
    # If the number of nodes has reached the number needed for a Applic
    # run, reset the node counter to 0, update the node set counter, and
    # start a new Applic node file.
    node_count=0
    node_set=`expr $node_set + 1`
    applic_nodefile=applicNodeFile.$node_set
  fi

done <$nodefile

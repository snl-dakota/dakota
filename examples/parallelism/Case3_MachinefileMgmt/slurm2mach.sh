#!/bin/sh

## Sample environment we're trying to concert to machines file
# SLURM_NODELIST=glory[99,109,116-119,131,139]
# SLURM_NNODES=8
# SLURM_JOBID=10114
# SLURM_TASKS_PER_NODE=1(x8)
# SLURM_JOB_ID=10114
# SLURM_JOB_NODELIST=glory[99,109,116-119,131,139]
# SLURM_JOB_CPUS_PER_NODE=16(x8)
# SLURM_JOB_NUM_NODES=8


if [ $# -lt 1 ]; then
  echo "Usage: $0 machinefile"
  exit 1
fi

ppn=`echo $SLURM_JOB_CPUS_PER_NODE | cut -f2 -d'=' | cut -f1 -d'('`
#ppn=$NUM_APPLIC_PROCS

# instead of being clever, handle degenerate case here (no brackets)
if [ `echo $SLURM_NODELIST | grep -c '\['` -lt 1 ]; then

   for k in `seq 1 $ppn`; do
     echo "$SLURM_NODELIST" >> $1
   done 

else

  system=`echo $SLURM_NODELIST | cut -f2 -d'=' | cut -f1 -d'['`
  nodes=`echo $SLURM_NODELIST | cut -f2 -d'[' | cut -f1 -d']'`

  # process each comma-separated entry, including any nested - ranges
  for i in `echo $nodes | tr ',' '\n'`; do

    if [ `echo $i | grep -c '-'` -gt 0 ]; then

      start=`echo $i | cut -f1 -d'-'`
      end=`echo $i | cut -f2 -d'-'`
      for j in `seq $start $end`; do
	for k in `seq 1 $ppn`; do
	  echo "$system$j" >> $1
	done
      done

    else
      for k in `seq 1 $ppn`; do
        echo "$system$i" >> $1
      done
    fi

  done

fi

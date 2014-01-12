#!/bin/bash
# RunTank.sh is a driver script for Dakota.
# it interfaces between Dakota and the "application code",
# EvalTank.py
# It is used to set up the arguments for EvalTank.py

# $1 is the input file from Dakota
# $2 is the results file that Dakota expects back

# EvalTank.py is actually executable and can take arguments $1 and $2
# Therefore, it can also be used as a driver script for Dakota.
# So this analysis driver is not necessary UNLESS
# you also want to write out a data file, which is the third
# argument of EvalTank.py, or record the output of the simulation.


dataFileName=$2.data #comment out -> no datafile written
stdOutput=Cylinder.stdout
#stdOutput=/dev/null

EvalTank.py $1 $2 $dataFileName >& $stdOutput

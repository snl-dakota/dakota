#!/bin/bash

# $1 and $2 are special variables in bash that contain the 1st and 2nd 
# command line arguments to the script, which are the names of the
# Dakota parameters and results files, respectively.

params=$1
results=$2

############################################################################### 
##
## Pre-processing Phase -- Generate/configure an input file for your simulation 
##  by substiting in parameter values from the Dakota paramters file.
##
###############################################################################

dprepro $params cantilever.template cantilever.i

############################################################################### 
##
## Execution Phase -- Run your simulation
##
###############################################################################


./cantilever cantilever.i > cantilever.log

############################################################################### 
##
## Post-processing Phase -- Extract (or calculate) quantities of interest
##  from your simulation's output and write them to a properly-formatted
##  Dakota results file.
##
###############################################################################

mass=$(tail -15 cantilever.log | head -1 | awk '{print $1}')
stress=$(tail -11 cantilever.log | head -1 | awk '{print $1}')
displacement=$(tail -7 cantilever.log | head -1 | awk '{print $1}')

echo "$mass mass" > $results
echo "$stress stress" >> $results
echo "$displacement displacement" >> $results


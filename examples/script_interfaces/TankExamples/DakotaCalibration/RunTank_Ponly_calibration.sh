#!/bin/bash
# this is an example Dakota driver script for calibrations
# in the case where the design variables are being selected
# to match data from multiple experiments with different 
# state variables.

# This this case, Dakota cannot specify the state variables,
# these must be controlled within the analyis driver.
# In the example below, Pressure is the state variable,
# and it should not be specified in the Dakota input file.

# Two data points are available, at three different Pressures.

if [ -e "$2" ]
then
  rm $2
fi

# Loop over state variable- pressure, and append results to $2
for Pressure in 29.38 73.45 146.9
do
  cp $1 $1.temp
  echo $Pressure P >> $1.temp
  EvalTank.py $1.temp $1.tempout
  cat $1.tempout $1.tempout >> $2 # write data out twice, to match two experiments
done
rm $1.temp $1.tempout

# Arranging the data is tricky - you must be careful to write out the simulation responses so that they match the datafile.
# In this case, the data file looks like
# Pressure1exp1 responses (4 rows, one per response)
# Pressure1exp2 responses
# Pressure2exp1 responses
# Pressure2exp2 responses
# Pressure3exp1 responses
# Pressure3exp2 responses

# so we loop over three pressures, and write the results twice

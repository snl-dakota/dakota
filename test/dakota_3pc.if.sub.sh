#!/bin/bash

# Make sure that Dakota passes ONLY the parameters file name. If it passes
# the wrong thing, touch FAIL to force the workflow to FAIL

rm -f FAIL
if [ $# -ne 1 ]
then
  touch FAIL
fi
# Stem of file is correct
params=$1
if [ "${params:0:5}" != "tb.in" ]
then
  touch FAIL
fi

# Tag exists
re='^[0-9]+$'
ptag=${params:6}
if ! [[ "$ptag" =~ $re ]] 
then
  touch FAIL
fi



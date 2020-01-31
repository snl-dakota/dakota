#!/bin/bash

# Make sure that Dakota passes ONLY the results file name. If it passes
# the wrong thing, touch FAIL to force the workflow to FAIL


if [ $# -ne 1 ]  # don't know the name of the results file
then
  exit -1
fi

# Stem of file is correct
results=$1
if [ "${results:0:6}" != "tb.out" ]
then
  echo FAIL > $results
  exit -1 
fi

# Tag exists
re='^[0-9]+$'
rtag=${results:7}
if ! [[ "$rtag" =~ $re ]]
then
  echo FAIL > $results
  exit -1
fi


if [ -f FAIL ]
then
  echo FAIL > $results
  exit
fi

mv tempresults.out $results

#!/bin/sh

function checkExcluded () 
{
  NOT_EXCLUDED=1
  for test in $EXCLUDED_TESTS; do
    if [ "x$test" == "x$TESTNAME" ]; then
      NOT_EXCLUDED=0
      return
    fi
  done
}


function main () 
{
  python ${DAKOTA_SRC_DIR}/test/rol_test_scripts/expand_tests.py

  cd Extracted_Tests

  for i in test_*
  do 
    TESTNAME=$i
    checkExcluded
     if [ $NOT_EXCLUDED == 1 ]; then
       ../../../src/dakota $i | tee  "${i/test_/out_}"
       #echo "Running test $TESTNAME"
     fi
  done

  FIRST_OUT_FILE=$(ls -1 out_* | head -1)
  NUM_C_VARS=$(grep continuous_design $FIRST_OUT_FILE |awk {'print $3'})
  #echo "Num Vars : "$NUM_C_VARS

  rm -f results_summary.txt
  touch results_summary.txt

  for i in out_*
  do
    echo "" >> results_summary.txt
    echo "" >> results_summary.txt
    echo "" >> results_summary.txt
    echo "Test #${i/out_/}" >> results_summary.txt
    grep 'Function evaluation summary' $i | grep APPROX_INTERFACE >> results_summary.txt
    grep 'Function evaluation summary' $i | grep TRUE_FN >> results_summary.txt
    grep -A $NUM_C_VARS 'Best parameters' $i >> results_summary.txt
    grep -A 1 'Best objective function' $i >> results_summary.txt
    echo "Finished Test #${i/out_/}"
  done
}

DAKOTA_SRC_DIR=~/Projects/Dakota
EXCLUDED_TESTS=""
#EXCLUDED_TESTS="test_0 test_1 test_2 test_5"

main

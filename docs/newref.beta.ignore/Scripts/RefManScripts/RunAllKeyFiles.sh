#!/bin/bash
# This is run from the working directory


WorkDir=$PWD
ScriptDir=$(readlink -f $(dirname $0) )

cd KeywordContentFiles
files=$(ls)
cd $WorkDir/outputs

counter=0
for f in $files
do #ugly checks to allow for concurent runs
  counter=$(( $counter+1 ))
  if [ ! -e $f.output ]
  then
    sleep .0$[$RANDOM % 100]s
  fi
  if [ ! -e $f.output ]
  then
    sleep .0$[$RANDOM % 100]s
  fi
  if [ ! -e $f.output ]
  then
    touch $f.output
    echo $counter - $f
    sh $ScriptDir/ScriptPasteToTemplate.sh $WorkDir/KeywordContentFiles/$f
    DescTest=$(grep Specification $f.output)
    while [ -z "$DescTest" ] #sometimes the script fails. dont' know why
    do
      sleep .$[$RANDOM % 100]s
      echo $counter - $f    ERROR - retry
      sh $ScriptDir/ScriptPasteToTemplate.sh $WorkDir/KeywordContentFiles/$f
      DescTest=$(grep Specification $f.output)
    done
  fi
done

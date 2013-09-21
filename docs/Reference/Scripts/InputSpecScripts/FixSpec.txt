#!/bin/bash
# Fix the input spec summary
# some things are scriptable, but the groups with duplicate subkeywords are hard to do, so just fix them manually

sed -e "s/\t/        /g" -e "s/  \]/\]/g" -e "s/[ ]*$//g" -e "s/  )/)/g" -e "s/^\([ ]*\)\([a-z]\)/\1( \2/g" -e "s/^\([ ]*\)| /\1|\n\1( /g" < InputSpec_5_4.txt > temp

TestLines=$(grep -n '|' temp | cut -d ':' -f1)
NumLines=$(echo "$TestLines" | wc -l )

for (( idx=1; idx <= NumLines ; idx++ ))
do
  Line=$(echo $TestLines | cut -d ' ' -f$idx)
  Spaces=$(sed -n $Line"p" < temp | cut -d '|' -f1)
  PrevLineIsGood=$(sed -n $(( $Line -1 ))p < temp | grep "$Spaces" )
  if [ -z "$PrevLineIsGood" ] # If same number of spaces, then it's ok
  then
    echo $Line >> BadLines
  fi

done

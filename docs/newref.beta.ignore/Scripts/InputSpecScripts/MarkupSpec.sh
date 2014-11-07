#!/bin/bash -xv
# $1 is a portion of the dakota input spec summary.
# $2 is the output file to write
# This formats it into individual files that describe a single keyword and all keywords ONE LEVEL DOWN.
# some of these lower level keywords also have lower level keywords, those are NOT included here.

sed -e "/^[ ]*$/d" -e "/  /d" < $1 > temp2 #remove all keys that are not at the current level, current level has a descriptor symbol at character 1

numLines=$(wc -l < temp2)
echo '' >> temp2 #need an extra line for comparison in the loop, doesn't do anything

head -n4 temp2 > temp

# identify all the beginning keywords of collections
# Four choices: Required or optional, single or group
for (( idx=5; idx<=$numLines; idx++ )) #First line = hier, 2nd line = name, 3rd = alias, 4th =arguments
do
  Line=$(sed -n $idx"p" < temp2 )
  test0=$(echo $Line | grep "|"  ) #blank unless the line has |, then it should not have a keyword
  test1=$(echo $Line | grep "("  ) #blank unless the line has (  
  test2=$(sed -n $(( idx + 1 ))"p" < temp2 | grep "|"  ) #blank unless next line has |
  test2b=$(sed -n $(( idx - 1 ))"p" < temp2 | grep "|"  ) #blank unless previous line has |

  Line=$(echo $Line | tr -d '[]{}()|' ) #clean up, future - move sed and cut up here
  if [ -n "$test0" ]
  then #not part of a group or keyword, ignore
    echo -n ''
  elif [ -z "$test2$test2b" ]
  then #not part of a group
    #Handle the single keywords
    if [ -z "$test1" ]
    then
      echo $Line | sed "s/^[ ]*/Optional_Keyword::\t/" | cut -d ' ' -f1 >> temp
    else
      echo $Line | sed "s/^[ ]*/Required_Keyword::\t/" | cut -d ' ' -f1 >> temp
    fi
  elif [ -n "$test2b" ] #test2b is not blank
  then # in group, but not the start
    echo $Line | sed "s/^[ ]*/\t/" >> temp
  else # start of a group
    # Handle the groups by marking the first one and no marker on the rest
    if [ -z "$test1" ]
    then
      echo $Line | sed "s/^[ ]*/Optional_Choose_One::\n\t/" | cut -d ' ' -f1 >> temp
    else
      echo $Line | sed "s/^[ ]*/Required_Choose_One::\n\t/" | cut -d ' ' -f1 >> temp
    fi
  fi
done

cp temp $2
#rm -f temp temp2

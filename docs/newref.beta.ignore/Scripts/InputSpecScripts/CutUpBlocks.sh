#!/bin/bash
# $1 is a portion of the dakota input spec.
# This formats it into individual files that describe a single keyword and all keywords ONE LEVEL DOWN.
# some of these lower level keywords also have lower level keywords, those are NOT included here.

# $1 should describe a single block and ALL its keywords
# $1 MUST be in the $PWD

if [ "$( cat $1 | wc -l )" -eq "1" ]
then
  exit
fi

cp $1 temp2
#cat $1 | cut -c 3- > temp2 #remove two leading spaces (unindent)

firstLettersIdx=$( cat temp2 | cut -c1 | grep -n [\[\({] | cut -d ':' -f1)
firstLettersIdx=$(echo $firstLettersIdx $(( 1 + $( cat temp2 | wc -l) )) ) #append a line number to indicate end of file
#echo $firstLettersIdx
numKeywords=$(echo $firstLettersIdx | tr ' ' '\n' | wc -l)

# cut up the file
for (( keyIdx=1; keyIdx<$numKeywords; keyIdx++ ))
do
  KeywordLine=$( echo $firstLettersIdx | cut -d ' ' -f $keyIdx )
  EndLine=$((  $( echo $firstLettersIdx | cut -d ' ' -f $(( $keyIdx + 1 )) ) - 1 )) #end before the next keyword
  KeywordName=$( sed -n $KeywordLine"p" < temp2 | tr -d '[]()|' | sed -e "s:^[ ]*::g" | cut -d ' ' -f1) #take the line, remove markup, remove leading spaces, take the first word
  AliasArg="$( sed -n $KeywordLine"p" < temp2 | tr -d '[]()|' | sed -e "s:^[ ]*::g" -e "s:$KeywordName::" | cut -d ' ' -f2-) " #take the line, remove markup, remove leading spaces, take all but the first Keyword name, add a trailing space
  HasAlias=$(echo $AliasArg | grep ALIAS)
  if [ -n "$HasAlias" ]
  then
    Alias=$( echo $AliasArg | cut -d ' ' -f2)
    Argument=$( echo $AliasArg | cut -d ' ' -f3-)
  else
    Alias=""
    Argument=$( sed -n $KeywordLine"p" < temp2 | tr -d '[]()|' | sed -e "s:^[ ]*::g" -e "s:$KeywordName::" | cut -d ' ' -f2-) #take the line, remove markup, remove leading spaces, take all but the first Keyword name 
  fi
  newName=$1"-"$KeywordName
  keyhier=$(echo $newName | sed "s:spec_::")
  echo "Keyword_Hierarchy::	$keyhier 
Name::	$KeywordName
Alias::	$Alias
Argument::	$Argument" > $newName
  if [ "$KeywordLine" -ne "$EndLine" ]
  then
    sed -n $(( 1 + $KeywordLine )),$EndLine"p" < temp2 | cut -c3- >> $newName
  fi
  echo ' ' >> $newName
done


rm -f temp temp2

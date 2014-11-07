#!/bin/bash
# $1 is the name of the content file, including path

ScriptDir=$(readlink -f $(dirname $0) )
ContentDir=$(readlink -f $(dirname $1) )
OutputFile=$(basename $1).output


KeyHier=$(grep Keyword_Hierarchy $1 | cut -d ':' -f3 | tr -d '\t ')
KeyName=$(grep "Name::" $1 | cut -d ':' -f3 | tr -d "\t ")

AllFieldLineNums=$(grep -n "::" $1 | cut -d ':' -f1 | tr '\n' ' ')
NumFields=$(( ( $(echo $AllFieldLineNums | tr -dc ' ' | wc -c ) ) + 1 ))
AllFieldLineNums="$AllFieldLineNums "$(( $(wc -l < $1) + 1 ))  #add the last line to indicate the end of the last field


###############
## BreadCrumbs
###############
NumCrumbs=$(( ( $(echo $KeyHier | tr -dc '-' | wc -c ) ) + 1 ))

BCtext='
 <ul class="breadcrumb">
  <li>\ref keywords</li> '
for (( idx=1 ; idx <= $NumCrumbs ; idx++ ))
do
  Word=$(echo $KeyHier | cut -d '-' -f1-$idx)
  BCtext=$BCtext'
  <li>\ref '$Word'</li>'
done
BCtext=$BCtext'
 </ul>'

###############
## Blurb
###############

TargetLine=$(grep -n Blurb:: $1 | cut -d ':' -f1)
for (( idx=1 ; idx <= $NumFields ; idx++ ))
do
  LineNum=$(echo $AllFieldLineNums | cut -d ' ' -f$idx)
  if [ "$LineNum" == "$TargetLine" ]
  then
    StartLineNum=$(( $(echo $AllFieldLineNums | cut -d ' ' -f$idx ) + 1 ))
    EndLineNum=$(( $(echo $AllFieldLineNums | cut -d ' ' -f$(( $idx + 1 )) ) - 1 ))
    if [ "$StartLineNum" -le "$EndLineNum" ]
    then
      BlurbText=$(sed -n $StartLineNum,$EndLineNum"p" < $1 | tr -d '\t')
    else
      BlurbText="Need a Blurb"
    fi
  fi
done



###############
## See Also
###############

KeyList=$(grep "See_Also::" $1 | cut -d ':' -f3 | tr -d '\t ')
KeyHier=$(grep Keyword_Hierarchy $1 | cut -d ':' -f3 | tr -d '\t ')
NumKeys=$(( ( $(echo $KeyList | tr -dc ',' | wc -c ) ) + 1 ))

if [ -n "$KeyList" ]
then
  SAtext='<div style="font-size: 150%; font-weight: bold">See Also</div>
These keywords may also be of interest:'
  
  for (( idx=1 ; idx <= $NumKeys ; idx++ ))
  do
    Key=$(echo $KeyList | cut -d ',' -f$idx)
    SAtext=$SAtext"
- \ref $Key  "
  done
fi


###############
## Specification
###############

SpecText='<div style="font-size: 150%; font-weight: bold">Specification</div>'

# Alias and Argument
Alias=$( grep "Alias::" $1 | cut -d ':' -f3 | tr -d '\t' ) # use this method b/c often there is no arg, and no tab. so cutting on tabs will not give good behavior
Argument=$( grep "Argument::" $1 | cut -d ':' -f3 | tr -d '\t' ) # use this method b/c often there is no arg, and no tab. so cutting on tabs will not give good behavior
if [ -z "$Alias" ]
then
  Alias=none
fi
if [ -z "$Argument" ]
then
  Argument=none
fi

SpecText=$SpecText'
<table class="keyword">
 <tr>
  <td class="kwtype">Alias</td>
  <td class="kwname">'$Alias'</td>
 </tr>
 <tr>
  <td class="kwtype">Argument</td>
  <td class="kwname">'$Argument'</td>
 </tr>
</table>'

# Chose One Keywords
KeyLineNums=$( grep -n "Choose_One::" $1 | cut -d ':' -f1 | tr '\n' ' ')
NumGroups=$(( $( echo $KeyLineNums | tr -dc ' ' | wc -c ) + 1 ))

if [ -n "$KeyLineNums" ]
then
  for (( Gidx=1 ; Gidx <= $NumGroups ; Gidx++ ))
  do
    LineNum=$(echo $KeyLineNums | cut -d ' ' -f$Gidx) #Line where the group field is located. Keywords start on next line
    Line=$(sed -n $LineNum"p" < $1 )
    GroupType=$( echo $Line | cut -d ':' -f1 | tr "_" " " ) #either optional or required
    GroupName=$( echo $Line | cut -d ':' -f3 | tr -d "\t" | sed -e "s/^ //" ) #anything after the field is the GroupName
    if [ -z "$GroupName" ]
    then
      GroupName="$KeyName Group $Gidx"
    fi
    NumKeys=0
    KeyList=""
    while true
    do
      LineNum=$(( $LineNum + 1 ))
      Line=$(sed -n $LineNum"p" < $1 )
      if [ -z $( echo $Line | tr -d "\t " ) ] || [ -z "$( echo $Line | grep -v "::" )" ]
      then
        break
      fi
      SubKey=$(echo $Line | cut -d ':' -f3 | tr -d "\t" | sed -e "s/^ //" | cut -d ' ' -f1 )
      KeyList="$KeyList$SubKey,"
      NumKeys=$(( $NumKeys + 1 ))
    done
    SubKey=$(echo $KeyList | cut -d ',' -f1)
    SubKeyBlurbLine=$(( $(grep -n "Blurb::" $ContentDir/$KeyHier-$SubKey | cut -d ':' -f1) + 1 ))
    SubKeyBlurb=$( sed -n $SubKeyBlurbLine"p" < $ContentDir/$KeyHier-$SubKey | tr -d '\t' )
    SpecText=$SpecText'

<table class="keyword">
 <tr>
  <th> </th>
  <th class="kwtype" colspan="2">'$GroupType'</th>
 </tr>
 <tr>
  <th class="kwtype" rowspan="'$(( $NumKeys + 1 ))'">'$GroupName'</th>
  <td class="kwname">\subpage '$KeyHier-$SubKey'</td>
  <td class="emph-italic">'$SubKeyBlurb'</td>
 </tr>'
    for (( Kidx=2 ; Kidx <= $NumKeys ; Kidx++ ))
    do
      SubKey=$(echo $KeyList | cut -d ',' -f$Kidx)
      SubKeyBlurbLine=$(( $(grep -n "Blurb::" $ContentDir/$KeyHier-$SubKey | cut -d ':' -f1) + 1 ))
      SubKeyBlurb=$( sed -n $SubKeyBlurbLine"p" < $ContentDir/$KeyHier-$SubKey | tr -d '\t' )
      SpecText=$SpecText'
 <tr>
  <td class="kwname">\subpage '$KeyHier-$SubKey'</td>
  <td class="emph-italic">'$SubKeyBlurb'</td>
 </tr>'
    done
    SpecText=$SpecText'
</table>'
  done
fi

# Required Keywords
KeyLineNums=$( grep -n "Required_Keyword::" $1 | cut -d ':' -f1 | tr '\n' ' ')

if [ -n "$KeyLineNums" ]
then
  NumKeys=$(( $( echo $KeyLineNums | tr -dc ' ' | wc -c ) + 1 ))
    SpecText=$SpecText'

<table class="keyword">
 <tr>
  <th> </th>
  <th class="kwtype" colspan="2">Required Keywords</th>
 </tr>'
  for (( idx=1 ; idx <= $NumKeys ; idx++ ))
  do
    KeyLineNum=$(echo $KeyLineNums | cut -d ' ' -f$idx)
    SubKey=$(sed -n $KeyLineNum"p" < $1 | cut -d ':' -f3 | tr -d "\t" | sed -e "s/^ //" | cut -d ' ' -f1 )
    SubKeyBlurbLine=$(( $(grep -n "Blurb::" $ContentDir/$KeyHier-$SubKey | cut -d ':' -f1) + 1 ))
    SubKeyBlurb=$( sed -n $SubKeyBlurbLine"p" < $ContentDir/$KeyHier-$SubKey | tr -d '\t' )
    SpecText=$SpecText'
 <tr>
  <td> '$idx'</td>
  <td class="kwname">\subpage '$KeyHier-$SubKey'</td>
  <td class="emph-italic">'$SubKeyBlurb'</td>
 </tr>'
  done
  SpecText=$SpecText'
</table>'
fi


# Optional Keywords
KeyLineNums=$( grep -n "Optional_Keyword::" $1 | cut -d ':' -f1 | tr '\n' ' ')

if [ -n "$KeyLineNums" ]
then
  NumKeys=$(( $( echo $KeyLineNums | tr -dc ' ' | wc -c ) + 1 ))
  SpecText=$SpecText'

<table class="keyword">
 <tr>
  <th> </th>
  <th class="kwtype" colspan="2">Optional Keywords</th>
 </tr>'
  for (( idx=1 ; idx <= $NumKeys ; idx++ ))
  do
    KeyLineNum=$(echo $KeyLineNums | cut -d ' ' -f$idx)
    SubKey=$(sed -n $KeyLineNum"p" < $1 | cut -d ':' -f3 | tr -d "\t" | sed -e "s/^ //" | cut -d ' ' -f1 )
    SubKeyBlurbLine=$(( $(grep -n "Blurb::" $ContentDir/$KeyHier-$SubKey | cut -d ':' -f1) + 1 ))
    SubKeyBlurb=$( sed -n $SubKeyBlurbLine"p" < $ContentDir/$KeyHier-$SubKey | tr -d '\t' )
    SpecText=$SpecText'
 <tr>
  <td> '$idx'</td>
  <td class="kwname">\subpage '$KeyHier-$SubKey'</td>
  <td class="emph-italic">'$SubKeyBlurb'</td>
 </tr>'
  done
  SpecText=$SpecText'
</table>'
fi


###############
## Topics
###############
# Must be run after the Blurb
TargetLine=$(grep Topics:: $1 | cut -d ':' -f3 | tr -d "\t " )

NumTopics=$(( ( $(echo $TargetLine | tr -dc ',' | wc -c ) ) + 1 ))

if [ -n "$TargetLine" ]
then
  TopicText='<div style="font-size: 150%; font-weight: bold">Topics</div>
This keyword is related to the topics: '
  for (( idx=1 ; idx <= $NumTopics ; idx++ ))
  do
    Topic=$(echo $TargetLine | cut -d ',' -f$idx)
    TopicText=$TopicText"
- \ref topic-$Topic"  # may want to switch this to a horizontal layout, would look better.
    if [ -e TopicFile-$Topic ]
    then # !!! the Topic Files should already have been prepped, by parsing the topic tree
      echo "- \ref $KeyHier
 - Full keyword path: $(echo $KeyHier | tr '-' '/')
 - $BlurbText
" >> TopicFile-$Topic 
    fi
  done
fi  


###############
## Description
###############

TargetLine=$(grep -n Description:: $1 | cut -d ':' -f1)
for (( idx=1 ; idx <= $NumFields ; idx++ ))
do
  LineNum=$(echo $AllFieldLineNums | cut -d ' ' -f$idx)
  if [ "$LineNum" == "$TargetLine" ]
  then
    StartLineNum=$(( $(echo $AllFieldLineNums | cut -d ' ' -f$idx ) + 1 ))
    EndLineNum=$(( $(echo $AllFieldLineNums | cut -d ' ' -f$(( $idx + 1 )) ) - 1 ))
    if [ "$StartLineNum" -le "$EndLineNum" ]
    then
      DescriptionText=$(sed -n $StartLineNum,$EndLineNum"p" < $1 | tr -d '\t')
      DescriptionText='<div style="font-size: 150%; font-weight: bold">Description</div>
'"$DescriptionText"
    fi
  fi
done


###############
## Examples
###############

TargetLine=$(grep -n Examples:: $1 | cut -d ':' -f1)
for (( idx=1 ; idx <= $NumFields ; idx++ ))
do
  LineNum=$(echo $AllFieldLineNums | cut -d ' ' -f$idx)
  if [ "$LineNum" == "$TargetLine" ]
  then
    StartLineNum=$(( $(echo $AllFieldLineNums | cut -d ' ' -f$idx ) + 1 ))
    EndLineNum=$(( $(echo $AllFieldLineNums | cut -d ' ' -f$(( $idx + 1 )) ) - 1 ))
    if [ "$StartLineNum" -le "$EndLineNum" ]
    then
      ExamplesText=$(sed -n $StartLineNum,$EndLineNum"p" < $1 | tr -d '\t')
      ExamplesText='<div style="font-size: 150%; font-weight: bold">Examples</div>
'"$ExamplesText"
    fi
  fi
done

###############
## Theory
###############

TargetLine=$(grep -n Theory:: $1 | cut -d ':' -f1)
for (( idx=1 ; idx <= $NumFields ; idx++ ))
do
  LineNum=$(echo $AllFieldLineNums | cut -d ' ' -f$idx)
  if [ "$LineNum" == "$TargetLine" ]
  then
    StartLineNum=$(( $(echo $AllFieldLineNums | cut -d ' ' -f$idx ) + 1 ))
    EndLineNum=$(( $(echo $AllFieldLineNums | cut -d ' ' -f$(( $idx + 1 )) ) - 1 ))
    if [ "$StartLineNum" -le "$EndLineNum" ]
    then
      TheoryText=$(sed -n $StartLineNum,$EndLineNum"p" < $1 | tr -d '\t')
      TheoryText='<div style="font-size: 150%; font-weight: bold">Theory</div>
'"$TheoryText"
    fi
  fi
done


###############
## FAQ
###############

TargetLine=$(grep -n Faq:: $1 | cut -d ':' -f1)
for (( idx=1 ; idx <= $NumFields ; idx++ ))
do
  LineNum=$(echo $AllFieldLineNums | cut -d ' ' -f$idx)
  if [ "$LineNum" == "$TargetLine" ]
  then
    StartLineNum=$(( $(echo $AllFieldLineNums | cut -d ' ' -f$idx ) + 1 ))
    EndLineNum=$(( $(echo $AllFieldLineNums | cut -d ' ' -f$(( $idx + 1 )) ) - 1 ))
    if [ "$StartLineNum" -le "$EndLineNum" ]
    then
      FaqText=$(sed -n $StartLineNum,$EndLineNum"p" < $1 | tr -d '\t')
      FaqText='<div style="font-size: 150%; font-weight: bold">FAQ</div>
'"$FaqText"
    fi
  fi
done


echo '<!----------------------------------------------------------->

\page '$KeyHier' '$KeyName '

'"$BCtext"'

'"$BlurbText"'

'"$TopicText"'

'"$SpecText"'

'"$DescriptionText"'

'"$ExamplesText"'

'"$TheoryText"'

'"$SAtext"'

'"$FaqText" > $OutputFile

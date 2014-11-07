#!/bin/bash
# $1 is the topic tree
# $2 is the path to the directory with the Topic description files

# !!!!!!!!!!!!!!!!!!!!
# Note: this does not handle topics that are listed multiple times in the topic tree
# !!!!!!!!!!!!!!!!!!!!!!

rm -f TopicFile-* 

sed -e "s:[ ]*$::g" -e "s: :+:g" < $1 > tempList.txt  #string starting w/ spaces don't echo well

NumLines=$(more tempList.txt | wc -l)

for (( LineNum=1; LineNum<=$NumLines ; LineNum++ ))
do
  Line=$(sed -n $LineNum"p" < tempList.txt)
  LineLevel=$( echo $Line | tr -dc '+' | wc -c )
  TopicName=$(echo $Line | tr -d '+')
  if [ "$LineLevel" -eq "0" ]
  then
    echo "- \subpage topic-"$TopicName >> TopicFile-Root  #Tracks the highest level of topics, parent node is the root
  fi
#  echo $LineNum $TopicName   - level $LineLevel
  if [ ! -e "TopicFile-$TopicName" ]
  then
    echo "\page topic-"$TopicName" "$( echo $TopicName | tr '_' ' ')'

<div style="font-size: 150%; font-weight: bold">Description</div>
'   > TopicFile-$TopicName

    if [ -e "$2/TopicMetadataFile-$TopicName" ]
    then
      more $2/TopicMetadataFile-$TopicName >> TopicFile-$TopicName
    else
      echo "THIS TOPICS NEEDS A TOPIC METADATA FILE" >> TopicFile-$TopicName
    fi

    echo '
<div style="font-size: 150%; font-weight: bold">Topics related to this topic:</div>
'   >> TopicFile-$TopicName
  fi
  nextLineNum=$LineNum
  nextLevel=999
  HasSubTopics=0
  while [ "$LineLevel" -le "$nextLevel" ]
  do
    nextLineNum=$(( $nextLineNum + 1 ))
    nextLine=$(sed -n $nextLineNum"p" < tempList.txt)
    nextLevel=$( echo $nextLine | tr -dc '+' | wc -c )
    if [ "$nextLevel" -eq "$(( $LineLevel + 1 ))" ]
    then
      nextTopicName=$(echo $nextLine | tr -d '+')
#      echo sub $nextTopicName   - level $nextLevel
      echo "- \subpage topic-"$nextTopicName >> TopicFile-$TopicName
      HasSubTopics=1
    elif [ "$nextLevel" -le "$LineLevel" ]
    then
      break
    fi
    if [ "$nextLineNum" -ge "$NumLines" ]
    then
      break
    fi
  done
#  if [ "$HasSubTopics" -eq 0 ]
#  then # this cannot handle the case where the topic appears multiple times in the tree, and has children in multiple spots...
#    echo "This topic has no relevant subtopics" >> TopicFile-$TopicName
#  fi
done

rm tempList.txt

for f in TopicFile-[!R]*
do
  echo '

<div style="font-size: 150%; font-weight: bold">Keywords related to this topic:</div>

' >> $f
done

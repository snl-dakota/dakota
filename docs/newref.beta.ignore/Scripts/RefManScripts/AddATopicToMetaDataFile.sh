#!/bin/bash

# $1 is the MetaDataFile
# $2 is the topic to add to the Topics:: line
# Need to be careful about deliminter ,

cp $1 temp

LineNum=$(more temp | grep -n Topics'::' | cut -d ':' -f1 )
Line=$(sed -n $LineNum"p" < temp)
LineLen=$(echo $Line | wc -c)
if [ "$LineLen" -le 10 ]
then
  NewLine="Topics::	"$2
else
  NewLine=$(echo "$Line")", "$2
fi
sed -i $LineNum"d" temp
sed -i $LineNum"i""$NewLine" temp
mv temp $1

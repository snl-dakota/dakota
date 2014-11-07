#!/bin/bash
# Run from the working dir
# This will combine all the keyword content,
# Put the keyword information into the topics pages
# and then create output files in doxygen format

# There is some kludged code to allow the slow part to run concurrently. If you run into trouble, try running only one at a time by commenting out:
# sh $SourceDir/Scripts/RefManScripts/RunAllKeyFiles.sh &

# $1 is the path to the Reference manual directory with the source files
# $2 is the concurrency of the keyword prep script. defaults to 1

if [ "$2" -ge 1 ]
then
  NumConcScriptsMinus1=$(( $2 - 1 ))
else
  NumConcScriptsMinus1=0
fi


WorkDir=$PWD
SourceDir=$(readlink -f $1 )

# Paste the Keyword content together
# create dir to store the Keyword Content, must be completed before
# pasting to templates, because that requires access to all keyword content
rm -rf KeywordContentFiles/
mkdir KeywordContentFiles/
ln -s $SourceDir/MetaDataFiles $WorkDir/

cd $WorkDir/KeywordSpecFiles
for f in *  # the KeywordSpecFiles dir does not have the DUPLICATE files
do
  if [ -n "$(grep DUPLICATE- $WorkDir/MetaDataFiles/$f)" ]
  then #deal with duplicate files
    SourceFile=$(sed -n 1p < $WorkDir/MetaDataFiles/$f)
    cat $f $WorkDir/MetaDataFiles/$SourceFile > $WorkDir/KeywordContentFiles/$f
  else #not a duplicate file
    cat $f $WorkDir/MetaDataFiles/$f > $WorkDir/KeywordContentFiles/$f
  fi

done
cd $WorkDir

# create dir to store the finished pages (in doxygen format)
rm -rf outputs
mkdir outputs
mkdir outputs/Keys
mkdir outputs/Topics

cp TopicSpecFiles/TopicFile-* outputs/ #copy Topic template to outputs dir

counter=1
while [ "$counter" -le "$NumConcScriptsMinus1" ]
do
  sh $SourceDir/Scripts/RefManScripts/RunAllKeyFiles.sh &
  counter=$(( $counter + 1 ))
done
sh $SourceDir/Scripts/RefManScripts/RunAllKeyFiles.sh

cd outputs
mv TopicF* Topics/
mv [!KT]* Keys/
cd $WorkDir


#!/bin/bash
# Sets up a working dir for the Dakota Reference Manual
# Run from the empty working dir
# $1 is the path to the Reference manual directory with the directories containing source files

# This is used to create the entire reference manual from scratch
# It should only be run once, or if the input spec has changed
# If the topic metadata files or topic tree have changed, then run the last three scripts
# If the keyword metadata has changed, run the last two

if [ -z "$1" ]
then
  echo 'Must specify the path to the Reference Manual source files'
  exit
fi

WorkDir=$PWD
SourceDir=$(readlink -f $1 )


## 1 Parse Input Spec Summary -> keyword spec files
ln -sf $SourceDir/Scripts/Working_InputSpec.sh ./
echo Phase 1
time sh Working_InputSpec.sh $SourceDir $SourceDir/InputSpecSummary


## 2 Parse Topic tree and topic metadatafiles
ln -sf $SourceDir/Scripts/Working_TopicSpecFiles.sh ./
echo Phase 2
time sh Working_TopicSpecFiles.sh $SourceDir

## 3 Parse Keyword specs, Generate Keyword content, and keyword topics (complete topic content files)
ln -sf $SourceDir/Scripts/Working_KeywordContentFiles.sh ./
echo Phase 2
time sh Working_KeywordContentFiles.sh $SourceDir 1 #2nd arg is the concurrency. can be up to # of cpus.


## 4 Take templates, topic and keyword content files and make a doxygen file
echo Phase 2
time sh $SourceDir/Scripts/RefManScripts/PopulateMdTemplate.sh $SourceDir DakotaReferenceManual.dox

## 5 Build doxgyen files
# requires doxygen 1.8.4 on the path.
TestDox=$(doxygen -help | head -n1 | tr -dc [0-9])
if [ "$TestDox" -ge "184" ] #not robust
then
  rsync -av --exclude=".*" $SourceDir/DoxygenFiles/ ./Doxygen/ #copy without .svn dir
  cd Doxygen
  cp ../DakotaReferenceManual.dox ./
  doxygen Doxyfile 
fi
# the Ref Man should now be accessible on your local machine at
#   $WorkDir/Doxygen/html/index.html

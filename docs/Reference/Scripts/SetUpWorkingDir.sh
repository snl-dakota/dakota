#!/bin/bash
# Sets up a working dir for the Dakota Reference Manual
# Run from the empty working dir
# $1 is the path to the Reference manual directory with the directories containing source files

# This is used to create the entire reference manual from scratch
# It should only be run once, or if the input spec has changed
# If the topic metadata files or topic tree have changed, then run the last three scripts
# If the keyword metadata has changed, run the last two

WorkDir=$PWD
SourceDir=$(readlink -f $1 )

ln -sf $SourceDir/Scripts/Working* ./

sh Working_InputSpec.sh $SourceDir $SourceDir/InputSpec_5_3.modified
sh Working_TopicSpecFiles.sh $SourceDir
sh Working_KeywordContentFiles.sh $SourceDir

sh $SourceDir/Scripts/RefManScripts/PopulateMdTemplate.sh DakotaReferenceManual.dox


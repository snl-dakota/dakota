#!/bin/bash
# Run from the working dir
# This will parse the topic tree and create a topic file for every topic
# This is only a partial topic file, however. It does not have any of the keyword information
# $1 is the path to the Reference manual directory with the source files

WorkDir=$PWD
SourceDir=$(readlink -f $1 )

# Topics
ln -s $SourceDir/TopicMetadataFiles $WorkDir

rm -rf TopicSpecFiles
mkdir TopicSpecFiles
cd TopicSpecFiles

sh $SourceDir/Scripts/RefManScripts/ExpandTopicTree.sh $SourceDir/TopicTree $WorkDir/TopicMetadataFiles/
cd $WorkDir


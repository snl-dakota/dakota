#!/bin/bash
# Run from the working dir
# This will parse the input spec summary and write out a spec file for every keyword
# $1 is the path to the Reference manual directory with the source files
# $2 is the input spec summary file

WorkDir=$PWD
SourceDir=$(readlink -f $1 )

# Input Spec
rm -rf InputSpecFiles
mkdir InputSpecFiles
cd InputSpecFiles

ln -s $SourceDir/Scripts/InputSpecScripts/* ./

sh InputSpec2KeySpecFiles.sh $2

cd $WorkDir
ln -s InputSpecFiles/KeywordSpecFiles/ $WorkDir


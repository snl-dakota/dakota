#!/bin/bash -xv

SourceDir=$1
ScriptDir=$(readlink -f $(dirname $0) )

## CLEAN UP
rm -f outputs/inputspeccontent outputs/topicpages outputs/keywordspages outputs/doxgyenfile 

## INPUT SPEC
echo '\verbatim
'"$(more $SourceDir/InputSpecSummary)"'
\endverbatim' > outputs/inputspeccontent


## TOPICS
mv outputs/Topics/TopicFile-Root outputs/ # process the Root page differently
 
for f in $(ls outputs/Topics/*)
do
  echo "


<!-------------------------------------------->
" >> outputs/topicpages
  more $f >> outputs/topicpages
done


## KEYWORDS
for f in outputs/Keys/*
do
  cat $f >> outputs/keywordspages
done

## PASTE the files together 
# Some are header files, in the Script/RefManScripts directory,
# plus the collected topics and keyword pages, and the input spec summary
cat $ScriptDir/DoxygenFile-main.txt \
  $ScriptDir/DoxygenFile-input_file_examples.txt \
  $ScriptDir/DoxygenFile-input_spec.txt \
  $ScriptDir/DoxygenFile-input_spec_summary.txt outputs/inputspeccontent \
  $ScriptDir/DoxygenFile-topics.txt outputs/TopicFile-Root outputs/topicpages \
  $ScriptDir/DoxygenFile-keywords.txt outputs/keywordspages > outputs/doxgyenfile

# mark the end of doxygen file
echo '*/' >> outputs/doxgyenfile

cp outputs/doxgyenfile $2

mv outputs/TopicFile-Root outputs/Topics/ # move Root file back

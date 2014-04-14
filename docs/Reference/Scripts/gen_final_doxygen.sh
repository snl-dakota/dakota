#!/bin/sh

if [ $# -lt 3 ]; then 
  echo "Usage: $0 ScriptDir input_summary output_dox"
  exit 1;
fi

ScriptDir=${1}
input_summary=${2}
output_dox=${3}

# Generate verbatim spec summary
echo "\\verbatim" > DakotaInputSpec.dox
cat ${input_summary} >> DakotaInputSpec.dox
echo "\\endverbatim" >> DakotaInputSpec.dox

## TOPICS
#mv outputs/Topics/TopicFile-Root outputs/ # process the Root page differently
 
## PASTE the files together 
# Some are header files, in the Script/RefManScripts directory,
# plus the collected topics and keyword pages, and the input spec summary
cat $ScriptDir/DoxygenFile-main.txt \
  $ScriptDir/DoxygenFile-input_file_examples.txt \
  $ScriptDir/DoxygenFile-input_spec.txt \
  $ScriptDir/DoxygenFile-input_spec_summary.txt DakotaInputSpec.dox \
  $ScriptDir/DoxygenFile-topics.txt \
  DakotaTopics.dox \
  $ScriptDir/DoxygenFile-keywords.txt DakotaKeywords.dox > ${output_dox}

# TODO: should we treat separately?
#outputs/TopicFile-Root \

# mark the end of doxygen file
echo '*/' >> ${output_dox}

#mv outputs/TopicFile-Root outputs/Topics/ # move Root file back

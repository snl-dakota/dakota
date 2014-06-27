#!/bin/sh

if [ $# -lt 3 ]; then 
  echo "Usage: $0 dakota/src/docs input_summary output_dox"
  exit 1;
fi

src_docs=${1}
input_summary=${2}
output_dox=${3}

# Generate verbatim spec summary in intermediate file
echo "\\verbatim" > DakotaInputSpec.dox
cat ${input_summary} >> DakotaInputSpec.dox
echo "\\endverbatim" >> DakotaInputSpec.dox

## Concatenate the Doxygen files
# Some are header files, plus the collected topics and keyword pages, 
# and the input spec summary
cat $src_docs/Ref_Main.dox \
  $src_docs/Ref_Sample_Inputs.dox \
  $src_docs/Ref_Test_Problems.dox \
  $src_docs/Ref_Input_Spec.dox \
  $src_docs/Ref_Spec_Summary.dox DakotaInputSpec.dox \
  DakotaTopics.dox \
  $src_docs/Ref_Keywords_Intro.dox DakotaKeywords.dox > ${output_dox}

# Don't need this as the Java incorporates the topics header
#  $src_docs/Ref_TopicsIntro.dox 

# mark the end of doxygen file
echo '*/' >> ${output_dox}

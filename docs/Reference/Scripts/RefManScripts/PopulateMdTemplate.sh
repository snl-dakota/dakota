#!/bin/bash -xv

here=$PWD
ScriptDir=$(readlink -f $(dirname $0) )

cat $ScriptDir/TopOfDoxygenFile.txt outputs/Topics/TopicFile-root > toppart
mv outputs/Topics/TopicFile-root outputs/

for f in $(ls outputs/Topics/*)
do
  cat $f >> topicpages
  echo "


<!-------------------------------------------->
" >> topicpages
done

echo "
<!--------Topics Pages----------------------->  
" >> toppart

#cat the main toppart file and the topic pages file
cat toppart topicpages > addkeywordspages

echo '

<!------Keywords Area------------------------->

\page keywords Keywords Area
This page lists the six blocks. From here, you can navigate to every keyword.

- \subpage strategy
- \subpage method
- \subpage model
- \subpage variables
- \subpage interface
- \subpage responses

<!--------Keywords Pages----------------------->  
'  >> addkeywordspages

for f in outputs/Keys/*
do
  cat $f >> addkeywordspages
done

echo '*/' >> addkeywordspages

cp addkeywordspages $1
mv outputs/TopicFile-root outputs/Topics/
rm toppart addkeywordspages topicpages 

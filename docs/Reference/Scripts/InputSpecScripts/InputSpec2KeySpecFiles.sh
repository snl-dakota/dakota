#!/bin/bash
#1 is the input spec summary file

# clean up, fix the markup
cp $1 temp

sed "/^[ ]*$/d" < temp > temp2  # remove blank lines
sed -e "/^[ ]*\]$/d"  -e "/^[ ]*)$/d" < temp2 > temp  # remove lines w/ closing markers
sed -e "s:^\( [ ]*\)\([a-Z_ ]*\)$:\1( \2:g" < temp > temp2 # add required markers to unmarked
sed -e "s:^\( [ ]*\)| \([a-Z_ ]*\):\1\|\n\1\( \2:g" < temp2 > temp

# find all the blocks
echo KEYWORD >> temp #mark the end of the file
blockLineNums=$( grep -n KEYWORD < temp | cut -d ':' -f1 )
echo $blockLineNums
for (( bNum=1; bNum < $(grep -c KEYWORD < temp) ; bNum++ ))
do

# write each block to a new file
  blockLineNum=$(echo $blockLineNums | cut -d ' ' -f$bNum )
  nextBlockNum=$(( $bNum + 1 ))
  nextBlockLine=$(echo $blockLineNums | cut -d ' ' -f$nextBlockNum )
  blockName=$(sed -n "$blockLineNum"p < temp | cut -d ' ' -f2 )
  fileName=spec_"$blockName"
  echo "Keyword_Hierarchy::	$blockName 
Name::	$blockName
Alias::
Argument::" > $fileName
  sed -n "$(( $blockLineNum + 1 )),$(( $nextBlockLine - 1 ))"p < temp | cut -c9- >> $fileName # Also cut off all leading spaces
done

# now process iteratively - wasteful, but it works.
for f in $( ls spec_* )
do
  echo $f
   sh CutUpBlocks.sh $f
  NewName=$( echo $f | sed -e "s:spec_::g" )
  sh MarkupSpec.sh $f $NewName
done
for f in $( ls spec_*-* )
do
  echo $f
   sh CutUpBlocks.sh $f
  NewName=$( echo $f | sed -e "s:spec_::g" )
  sh MarkupSpec.sh $f $NewName
done
for f in $( ls spec_*-* )
do
  echo $f
   sh CutUpBlocks.sh $f
  NewName=$( echo $f | sed -e "s:spec_::g" )
  sh MarkupSpec.sh $f $NewName
done
for f in $( ls spec_*-*-* )
do
  echo $f
   sh CutUpBlocks.sh $f
  NewName=$( echo $f | sed -e "s:spec_::g" )
  sh MarkupSpec.sh $f $NewName
done
for f in $( ls spec_*-*-*-* )
do
  echo $f
   sh CutUpBlocks.sh $f
  NewName=$( echo $f | sed -e "s:spec_::g" )
  sh MarkupSpec.sh $f $NewName
done
for f in $( ls spec_*-*-*-*-* )
do
  echo $f
   sh CutUpBlocks.sh $f
  NewName=$( echo $f | sed -e "s:spec_::g" )
  sh MarkupSpec.sh $f $NewName
done
for f in $( ls spec_*-*-*-*-*-* )
do
  echo $f
   sh CutUpBlocks.sh $f
  NewName=$( echo $f | sed -e "s:spec_::g" )
  sh MarkupSpec.sh $f $NewName
done
for f in $( ls spec_*-*-*-*-*-*-* )
do
  echo $f
   sh CutUpBlocks.sh $f
  NewName=$( echo $f | sed -e "s:spec_::g" )
  sh MarkupSpec.sh $f $NewName
done
for f in $( ls spec_*-*-*-*-*-*-*-* )
do
  echo $f
   sh CutUpBlocks.sh $f
  NewName=$( echo $f | sed -e "s:spec_::g" )
  sh MarkupSpec.sh $f $NewName
done

mkdir Specs/
mkdir KeywordSpecFiles/
mv spec* Specs/
mv [smivr]* KeywordSpecFiles/

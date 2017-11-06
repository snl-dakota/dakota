#!/bin/sh

function generateMakeLogFile () 
{
  CURDUR=`pwd`
  echo "cd into $DAKOTA_BUILD_DIR from $CURDUR"
  cd $DAKOTA_BUILD_DIR
  $DAKOTA_CMAKE_SCRIPT |tee $DAKOTA_CMAKE_SCRIPT.log
  cd docs
  make clean
  make docs-doxygen >& $DAKOTA_MAKE_LOG_FILE
  cd $CURDUR
}


function copyFiles () 
{
  CURDUR=`pwd`
  cd $DAKOTA_SOURCE_DIR/docs/KeywordMetadata

  for pref in $SOURCE_FILE_PREFIXES; do
    #for f in `ls -1 | egrep "method-polynomial_chaos-[[:alpha:]+_]*_sequence"`
    for f in `ls -1 | egrep "$pref[[:alpha:]+_]*"`
    do
      #echo $f
      for target in $TARGET_FILE_PREFIXES; do

        NEWFILE=$(echo $f | sed "s/$pref/$target/")
        cp $f $NEWFILE
        NEWFILE=$(echo $NEWFILE | sed 's/_sequence//')
        cp $f $NEWFILE
        NEWFILE=$(echo $NEWFILE | sed 's/_sequence//g')
        cp $f $NEWFILE
        if [[ "$f" =~ "_sequence" ]]; then
          NEWFILE=$(echo $f | sed 's/_sequence//g')
          cp $f $NEWFILE
        fi

        ## move the file to same filename, but removing _sequence
        #NEWFILE=$(echo $f | sed 's/_sequence//g')
        #git mv $f $NEWFILE
      done 

    done
  done

  cd $CURDUR
}


function removeExtra () 
{
  for f in $EXTRANEOUS_FILES; do
    rm $f
  done
}


# ------------------------------------------
#              START HERE
# ------------------------------------------


# These files (source and targets) are to facilitate the keyword overhaul
# activity in https://software-sandbox.sandia.gov/jira/browse/DAK-1805
# which reflects a lot of changes brought in from merged topic branches,
# eg multilevel_harden ...

SOURCE_FILE_PREFIXES="method-polynomial_chaos-"

TARGET_FILE_PREFIXES="method-multifidelity_polynomial_chaos-"
TARGET_FILE_PREFIXES+=" method-multilevel_polynomial_chaos-"
TARGET_FILE_PREFIXES+=" method-multifidelity_stoch_collocation-"
for BM in queso dream wasabi; do
  TARGET_FILE_PREFIXES+=" method-bayes_calibration-$BM-emulator-pce-"
  TARGET_FILE_PREFIXES+=" method-bayes_calibration-$BM-emulator-mf_pce-"
  TARGET_FILE_PREFIXES+=" method-bayes_calibration-$BM-emulator-mf_sc-"
  TARGET_FILE_PREFIXES+=" method-bayes_calibration-$BM-emulator-ml_pce-"
  TARGET_FILE_PREFIXES+=" method-bayes_calibration-$BM-emulator-sc-"
  TARGET_FILE_PREFIXES+=" method-bayes_calibration-$BM-emulator-sc_pce-"
done


# User specific settings
DAKOTA_SOURCE_DIR=/home/rhoope/Projects/Dakota
DAKOTA_BUILD_DIR=/home/rhoope/BUILDS/DAKOTA/DAKOTA_DOCS
DAKOTA_CMAKE_SCRIPT=cmake_config
DAKOTA_MAKE_LOG_FILE=make.log

# ------------------------------------------
# These steps were tested on the Dakota devel branch, sha1 56285cf0442bac9
# ------------------------------------------

# Skip this step if you already have a build that includes docs and have made them
# NB: Could do this manually so that can kill at appropriate step instead of waiting for the whole process

# Step 1. Build fresh set of docs
#generateMakeLogFile

# Step 2. Make copies of options for new methods based on existing ones (preserves content instead of using dummy template)
# Create new files based on original method
#copyFiles

# Step 3. Restore existing files that got modified using git to guide
#git status -- . | grep 'modified:' | awk '{print $2;}' | xargs git checkout --

# Step 4. Regenerate docs to determine what options are not used
# NB: Could do this manually so that can kill at appropriate step instead of waiting for the whole process
#generateMakeLogFile
# ... but don't skip this step if building docs manually
#EXTRANEOUS_FILES=`grep 'no such' $DAKOTA_BUILD_DIR/docs/$DAKOTA_MAKE_LOG_FILE | awk '{print $5;}'`
#removeExtra

# Optional
# Might be good to look at what gets deleted by the previous step, ie
#cd $DAKOTA_SOURCE_DIR/docs/KeywordMetadata
#git status -- . | grep 'deleted:' > filesThatWillGetDeleted.txt

# Optional
# Could capture the state of things at this point if desireable
#rm -f filesThatWillGetDeleted.txt
#cd $DAKOTA_SOURCE_DIR/docs/KeywordMetadata
#git add .

# Optional
# Could now use local/scripts/generate_keyword_template.sh to create stubs for remaining new commands
#cd $DAKOTA_SOURCE_DIR/docs/KeywordMetadata
#$DAKOTA_SOURCE_DIR/local/scripts/generate_keyword_template.sh $DAKOTA_BUILD_DIR/docs/$DAKOTA_MAKE_LOG_FILE
#git status -- .
# Look for Untracked files ...



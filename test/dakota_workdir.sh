#!/bin/sh

# Consider reimplementing in Python for cross-platform

# Test work_directory named trydir
workdir_basename="trydir"

mode=""
if [ $# -gt 2 ]; then
  mode=$1
  shift
fi
paramsfile="$1"
resultsfile="$2"

case $mode in

  tag)
    # we should find ourselves in a directory tagged with an integer
    mytag=`pwd | awk -F. '{print $NF}'`
    validtags=`echo $mytag | egrep -c '[0-9]+'`
    if [ "$validtags" -ne 1 ]; then
      echo "Error: Unexpected directory tag!"
      exit 1
    fi
    ;;

  tag_save)
    # we should find ourselves in a directory tagged with an integer
    # and if the eval is 2 or greater, the previous dir should be around	
    mytag=`pwd | awk -F. '{print $NF}'`
    validtags=`echo $mytag | egrep -c '[0-9]+'`
    if [ "$validtags" -ne 1 ]; then
      echo "Error: Unexpected directory tag!"
      exit 1
    fi
    if [ "$mytag" -gt 1 ]; then
      lastdir="${workdir_basename}.$((mytag-1))"
      if [ ! -d "../$lastdir" ]; then
        echo "Error: Failed to find saved directory $lastdir"
        exit 1
      fi
    fi
    ;;


  templatefiles)
    # template files will include dakota_workdir.root.dat
    # and they will be symlinked by default
    if [ ! -h ./dakota_workdir.root.dat ]; then
      if [ -f ./dakota_workdir.root.dat ]; then
	echo "Error: dakota_workdir.root.dat is regular file; should be symlink"
      else
        echo "Error: Failed to find template file dakota_workdir.root.dat"
      fi
      exit 1
    fi
    ;;

  templatefiles_copy)
    # template files will include dakota_workdir.root.dat
    # and they will be copied to pwd
    if [ -h ./dakota_workdir.root.dat ]; then
      echo "Error: dakota_workdir.root.dat is symlink; should be regular file"
      exit 1
    fi
    if [ ! -f ./dakota_workdir.root.dat ]; then
      echo "Error: Failed to find template file dakota_workdir.root.dat"
      exit 1
    fi
    ;;

  templatedir)
    # template files will include dakota_workdir.templatedir.dat
    # and they will be symlinked by default
    if [ ! -h ./dakota_workdir.templatedir.dat ]; then
      if [ -f ./dakota_workdir.templatedir.dat ]; then
	echo "Error: dakota_workdir.templatedir.dat is regular file; should be symlink"
      else
        echo "Error: Failed to find template file dakota_workdir.templatedir.dat"
      fi
      exit 1
    fi
    ;;

  templatedir_copy)
    # template files will include dakota_workdir.templatedir.dat
    # and they will be copied to pwd
    if [ -h ./dakota_workdir.templatedir.dat ]; then
      echo "Error: dakota_workdir.templatedir.dat is symlink; should be regular file"
      exit 1
    fi
    if [ ! -f ./dakota_workdir.templatedir.dat ]; then
      echo "Error: Failed to find template file dakota_workdir.templatedir.dat"
      exit 1
    fi
    ;;
 
  named_params)
    if [ ! -f ./params.in ]; then
      echo "Error: named parameters file params.in does not exist in trydir"
    fi
    ;;

  saved_params)
    # we should find ourselves in a directory tagged with an integer
    # and if the eval is 2 or greater, the previous dir should be around
    # with a saved params file
    mytag=`pwd | awk -F. '{print $NF}'`
    validtags=`echo $mytag | egrep -c '[0-9]+'`
    if [ "$validtags" -ne 1 ]; then
      echo "Error: Unexpected directory tag!"
      exit 1
    fi
    if [ "$mytag" -gt 1 ]; then
      lastdir="${workdir_basename}.$((mytag-1))"
      if [ ! -d "../$lastdir" ]; then
        echo "Error: Failed to find saved directory $lastdir"
        exit 1
      fi
      if [ ! -f ../$lastdir/params.in ]; then
	echo "Error: named parameters file params.in does not exist in last tagged trydir"
      fi
    fi


esac

echo "Running rosenbrock..."
rosenbrock $paramsfile $resultsfile
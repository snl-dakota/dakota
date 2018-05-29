#!/bin/bash

# CL arg should be the directory that contains the pyprepro git repo
pyprepro=$1

if [ -z $pyprepro ]; then
  echo "Must supply the name of the directory that contains the pyprepro clone."  
  exit 2
fi

rsync -rlp --exclude '.git' ${pyprepro}/* .

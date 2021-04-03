#!/bin/bash

abs_path="$1"
target_lib="$2"
abs_target_lib="${abs_path}/${target_lib}"

install_name_tool -id ${abs_path}/${target_lib} ${abs_path}/${target_lib}
for full_lib in $( otool -L ${abs_target_lib} | egrep -e "\s+.*libboost_.*?.dylib" | awk '{print $1}' )
do
  lib=`basename ${full_lib}`
  install_name_tool -change ${full_lib} ${abs_path}/${lib} ${abs_path}/${target_lib}
done

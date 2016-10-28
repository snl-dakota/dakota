#!/bin/bash -e

# Modify the texmf.cnf file before using it to increase the save_size parameter in order to
# prevent buffer underruns. If the save_save parameter exists in the file, multiply it by 10x.
# if it doesn't, append it, settign it to 150000.

texmf_cnf_src=$1
destdir=$2

if save_size_line=$(grep -o "save_size = [0-9]\+" ${texmf_cnf_src} ); then
  curr_save_size=$(cut -f2 -d'=' <(echo $save_size_line))
  new_save_size=$(expr 10 \* $curr_save_size);
  sed -e "s/save_size = [0-9]\+/save_size = $new_save_size/g" ${texmf_cnf_src} > ${destdir}/texmf.cnf 
else
  { cat ${texmf_cnf_src}; echo "save_size = 150000"; } > ${destdir}/texmf.cnf
fi

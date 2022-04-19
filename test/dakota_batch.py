#!/usr/bin/env python
import sys
import os


params_file = sys.argv[1]
results_file = sys.argv[2]
params_tmp_file = params_file + ".tmp"
results_tmp_file = results_file + ".tmp"

to_write = []
with open(params_file,"r") as pif:
    for line in pif:
        to_write.append(line)
        if " metadata" in line:
            with open(params_tmp_file,"w") as pof:
                pof.writelines(to_write)
            os.system("text_book %s %s" % (params_tmp_file, results_tmp_file))
            with open(results_file,"a") as dr:
                with open(results_tmp_file,"r") as tr:
                    dr.write("#\n")
                    dr.writelines(tr.readlines())
            del to_write[:]


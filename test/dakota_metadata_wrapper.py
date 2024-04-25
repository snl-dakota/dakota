#!/usr/bin/env python3

# Wrapper around text_book that appends some metadata at end of file

import os
import re
import sys
import time

def insert_metadata(start, outfile):
    # TODO: generate metadata for each MD request in params file
    elapsed = time.time() - start
    res_out.write("                     {0:20.16e} seconds\n".format(elapsed))


def wrapped_driver(params_name):
    with open(params_name, 'r') as pfile:
        for line in pfile:
            if "AC_1" in line:
                driver_exe = line.strip().split(" ")[0]
                return driver_exe
    raise RuntimeError("{0} requires 1 analysis_component specifying the wrapped driver".format(sys.argv[0]))


if __name__ == "__main__":

    start = time.time()

    params_name = sys.argv[1]
    results_name = sys.argv[2]

    # Leave any file tag to support concurrent evaluations
    tmp_results = results_name + ".tmp"

    driver_exe = wrapped_driver(params_name)

    os.system("{0} {1} {2}".format(driver_exe, params_name, tmp_results))

    with open(tmp_results, 'r') as res_in, open(results_name, 'w') as res_out:
        res_out.write(res_in.read())
        insert_metadata(start, res_out)

    os.remove(tmp_results)

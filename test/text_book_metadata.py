#!/usr/bin/env python

# Wrapper around text_book that inserts some metadata
# (currently between values and gradients)

import os
import sys
import time

def insert_metadata(start, outfile):
    elapsed = time.time() - start
    res_out.write("                     {0:20.16e} seconds\n".format(elapsed))


if __name__ == "__main__":

    start = time.time()

    params_name = sys.argv[1]
    results_name = sys.argv[2]
    tmp_results = "results.tmp"
    
    os.system("text_book {0} {1}".format(params_name, tmp_results))
    
    with open(tmp_results, 'r') as res_in, open(results_name, 'w') as res_out:
        for line in res_in:
            res_out.write(line)
            # This may fail if ASV active...
            if line.strip().endswith("c2"):
                insert_metadata(start, res_out)

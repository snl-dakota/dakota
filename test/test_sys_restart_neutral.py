#!/usr/bin/env python
"""Test Dakota restart translation facilities

Test Dakota restart translation to/from neutral and to tabular formats
to exercise Variables and Response serialization, annotated, and
tabular I/O.

Script is intended to be run by CTest from dakota.build/test directory

TODO: test print and to_tabular functions and use gold standard
baselines throughout
"""
from __future__ import print_function
import filecmp
import glob
import os
import shutil
import subprocess
import sys

if len(sys.argv) != 3:
    raise RuntimeError("Usage:\n  " + sys.argv[0] + 
                       " /path/to/dakota /path/to/dakota_restart_util")
else:
    print("Running with arguments", sys.argv)

dakota_exe = sys.argv[1]
dakota_rst = sys.argv[2]

test_subdir = "sys_restart_neutral"
dakota_input = "dakota_vbd"
error_cnt = 0

# Setup a directory with necessary input, remove any stale output
if os.path.exists(test_subdir):
    if not os.path.isdir(test_subdir):
        raise RuntimeError(test_subdir + " exists, but is not a directory.")
else:
    os.mkdir(test_subdir)
shutil.copy(dakota_input + "/" + dakota_input + ".in", test_subdir)
os.chdir(test_subdir)
for fname in glob.glob("dakota.rst*"):
    os.remove(fname)

# Run initial Dakota study
dakota_cmd = dakota_exe + " -input " + dakota_input + ".in"
print("Running: " + dakota_cmd)
subprocess.call(dakota_cmd, shell=True)

# Restart to tabular; should agree with that output by Dakota run above
rst_tabular_cmd = dakota_rst + " to_tabular dakota.rst dakota.rst.tab"
print("Running: " + rst_tabular_cmd)
subprocess.call(rst_tabular_cmd, shell=True)
if filecmp.cmp("dakota_tabular.dat", "dakota.rst.tab"):
    print("INFO: dakota_tabular.dat and dakota.rst.tab files agree")
else:
    print("ERROR: dakota_tabular.dat and dakota.rst.tab files differ")
    error_cnt += 1

# Restart to neutral
rst_neu_cmd = dakota_rst + " to_neutral dakota.rst dakota.rst.neu"
print("Running: " + rst_neu_cmd)
subprocess.call(rst_neu_cmd, shell=True)

# Neutral back to restart
neu_rst_cmd = dakota_rst + " from_neutral dakota.rst.neu dakota.rst.neu.rst"
print("Running: " + neu_rst_cmd)
subprocess.call(neu_rst_cmd, shell=True)

# Can't assert files same due to missing responseID in neutral format
if filecmp.cmp("dakota.rst", "dakota.rst.neu.rst"):
    print("INFO: restart files same")
else:
    print("WARN: restart files differ")

# Restart back to tabular again as second-hand way to test .neu format
rst_tabular2_cmd = dakota_rst + " to_tabular dakota.rst.neu.rst dakota.rst.neu.rst.tab"
print("Running: " + rst_tabular2_cmd)
subprocess.call(rst_tabular2_cmd, shell=True)

if filecmp.cmp("dakota.rst.tab", "dakota.rst.neu.rst.tab"):
    print("INFO: dakota.rst.tab and dakota.rst.neu.rst.tab files agree")
else:
    print("ERROR: dakota.rst.tab and dakota.rst.neu.rst.tab files differ")
    error_cnt += 1

### Test error cases
# Empty restart file
open("empty.rst","w").close()
err_msg =  "Error reading restart file 'empty.rst' (empty or corrupt file).\n" + \
        "Details (Boost archive exception): invalid signature"
# dakota
dakota_cmd = dakota_exe + " -input " + dakota_input + ".in -read_restart empty.rst" 
print("Running: " + dakota_cmd)
pobj = subprocess.Popen(dakota_cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
stdout, stderr = pobj.communicate()
if err_msg in stderr:
    print("INFO: Dakota reports error and aborts when reading empty restart file")
else:
    print("ERROR: Dakota failed to report an error when reading empty restart file")
    error_cnt += 1

# restart util
empty_rst_cmd = dakota_rst + " to_tabular empty.rst empty.dat"
print("Running: " + empty_rst_cmd)
pobj = subprocess.Popen(empty_rst_cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
stdout, stderr = pobj.communicate()
if err_msg in stderr:
    print("INFO: Restart util reports error and aborts when reading empty restart file")
else:
    print("ERROR: Restart util failed to report an error when reading empty restart file")
    error_cnt += 1

# Non-existent restart file
try:
    os.remove("nonexistent.rst")
except OSError:
    pass
# dakota
err_msg = "Error: could not open restart file 'nonexistent.rst' for reading."
dakota_cmd = dakota_exe + " -input " + dakota_input + ".in -read_restart nonexistent.rst"
print("Running: " + dakota_cmd)
pobj = subprocess.Popen(dakota_cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
stdout, stderr = pobj.communicate()
if err_msg in stderr:
    print("INFO: Dakota reports error and aborts when reading nonexistent restart file")
else:
    print("ERROR: Dakota failed to report an error when reading nonexistent restart file")
    error_cnt += 1

# restart util
non_rst_cmd = dakota_rst + " to_tabular nonexistent.rst empty.dat"
print("Running: " + empty_rst_cmd)
pobj = subprocess.Popen(non_rst_cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
stdout, stderr = pobj.communicate()
if err_msg in stderr:
    print("INFO: Restart util reports error and aborts when reading nonexistent restart file")
else:
    print("ERROR: Restart util failed to report an error when reading nonexistent restart file")
    error_cnt += 1

# Corrupted restart file
# corrupt the file.
with open("dakota.rst","rb") as ifp:
    rst_text = ifp.read()
text_len = len(rst_text)
with open("corrupted.rst","wb") as ofp:
    ofp.write(rst_text[0:int(0.9*text_len)])

# dakota
err_msg = "Error reading restart file 'corrupted.rst'.\n" + \
        "You may be able to recover the first 359 evaluations with -stop_restart 359.\n" + \
        "Details (boost::archive exception): input stream error"
dakota_cmd = dakota_exe + " -input " + dakota_input + ".in -read_restart corrupted.rst"
print( "Running: " + dakota_cmd)
pobj = subprocess.Popen(dakota_cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
stdout, stderr = pobj.communicate()
if err_msg in stderr:
    print( "INFO: Dakota reports error and aborts when reading corrupted restart file")
else:
    print( "ERROR: Dakota failed to report an error when reading corrupted restart file")
    error_cnt += 1

# restart utily
err_msg = "Error reading restart file 'corrupted.rst'.\nDetails (boost::archive exception):      input stream error"
corr_rst_cmd = dakota_rst + " to_tabular corrupted.rst empty.dat"
print( "Running: " + corr_rst_cmd)
pobj = subprocess.Popen(corr_rst_cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
stdout, stderr = pobj.communicate()
if err_msg in stderr:
    print( "INFO: Restart util reports error and aborts when reading corrupted restart file")
else:
    print( "ERROR: Restart util failed to report an error when reading corrupted restart file")
    error_cnt += 1

if error_cnt > 0:
    print( "{:d} errors encountered during test.".format(error_cnt))
    sys.exit(1)

print( "All tests passed.")
sys.exit(0)

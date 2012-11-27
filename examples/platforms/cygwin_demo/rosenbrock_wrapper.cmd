@setlocal enableextensions & python -x %~f0 %* & goto :EOF

# Python script DAKOTA will call to execute rosenbrock binary
# DAKOTA will execute this script as
#   rosenbrock_wrapper.cmd params.in results.out
# so sys.argv[1] will be the parameters file and
#    sys.argv[2] will be the results file to return to DAKOTA

# add python modules (if necessary for pre/post-processing)
import sys
import os
import shutil

# mock pre-processing
shutil.copyfile(sys.argv[1], 'rosen.in')

print "Running Rosenbrock binary"
os.system('rosenbrock rosen.in rosen.out')
os.remove('rosen.in')

# mock post-processing
shutil.move('rosen.out', sys.argv[2])

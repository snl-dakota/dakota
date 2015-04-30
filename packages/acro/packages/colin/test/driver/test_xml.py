#  _________________________________________________________________________
#
#  Acro: A Common Repository for Optimizers
#  Copyright (c) 2008 Sandia Corporation.
#  This software is distributed under the BSD License.
#  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
#  the U.S. Government retains certain rights in this software.
#  For more information, see the README.txt file in the top Acro directory.
#  _________________________________________________________________________
#

#
# Test the COLIN driver commandline
#
import pyutilib.th
import unittest
import glob
import os
import os.path


class Test(pyutilib.th.TestCase): pass

currdir=os.path.dirname(os.path.abspath(__file__))+os.sep
data_dir=os.path.dirname(os.path.abspath(__file__))+os.sep+"data"+os.sep
for file in glob.glob("data/*.xml"):
    bname=os.path.basename(file)
    name=bname.split('.')[0]
    #print "HERE",data_dir+name+".qa"
    Test.add_baseline_test(cmd="cd "+currdir+"; ./colintest data/"+bname, baseline=data_dir+name+".qa", name=name)

    
if __name__ == "__main__":
   unittest.main()



import os
import sys
import pyutilib.th as unittest

currdir = os.path.dirname(os.path.abspath(__file__))+os.sep

class Test(unittest.TestCase): pass

#Test = category('smoke', 'nightly', 'utilib')(Test)

cmd = 'cd '+currdir+'; ./tests %s'

Test.add_baseline_test(cmd=cmd % "array", baseline=currdir+"array.qa", name="array")
Test.add_baseline_test(cmd=cmd % "bitarray", baseline=currdir+"bitarray.qa", name="bitarray")
Test.add_baseline_test(cmd=cmd % "commonio", baseline=currdir+"commonio.qa", name="commonio")
Test.add_baseline_test(cmd=cmd % "commonio_debug", baseline=currdir+"commonio_debug.qa", name="commonio_debug")
Test.add_baseline_test(cmd=cmd % "ereal", baseline=currdir+"ereal.qa", name="ereal")
Test.add_baseline_test(cmd=cmd % "exceptions", baseline=currdir+"exception_test.qa", name="exceptions")
Test.add_baseline_test(cmd=cmd % "factory", baseline=currdir+"factory.qa", name="factory")
Test.add_baseline_test(cmd=cmd % "heap", baseline=currdir+"heap.qa", name="heap")
Test.add_baseline_test(cmd=cmd % "iotest", baseline=currdir+"iotest.qa", name="iotest")
Test.add_baseline_test(cmd=cmd % "iotest_debug", baseline=currdir+"iotest_debug.qa", name="iotest_debug")
Test.add_baseline_test(cmd=cmd % "listtest", baseline=currdir+"listtest.qa", name="listtest")
Test.add_baseline_test(cmd=cmd % "lphash", baseline=currdir+"lphash.qa", name="lphash")
Test.add_baseline_test(cmd=cmd % "malloc", baseline=currdir+"malloc.qa", name="malloc")
Test.add_baseline_test(cmd=cmd % "math", baseline=currdir+"math.qa", name="math")
Test.add_baseline_test(cmd=cmd % "multibitarray", baseline=currdir+"multibitarray.qa", name="multibitarray")
Test.add_baseline_test(cmd=cmd % "olist", baseline=currdir+"olist.qa", name="olist")
Test.add_baseline_test(cmd=cmd % "pvector", baseline=currdir+"pvector.qa", name="pvector")
Test.add_baseline_test(cmd=cmd % "qlist", baseline=currdir+"qlist.qa", name="qlist")
Test.add_baseline_test(cmd=cmd % "smartptr", baseline=currdir+"smartptr.qa", name="smartptr")
Test.add_baseline_test(cmd=cmd % "sort", baseline=currdir+"sort_test.qa", name="sort")
Test.add_baseline_test(cmd=cmd % "splay", baseline=currdir+"splay.qa", name="splay")
Test.add_baseline_test(cmd=cmd % "stl", baseline=currdir+"stltest.qa", name="stl")
Test.add_baseline_test(cmd=cmd % "string", baseline=currdir+"stringtest.qa", name="string")
Test.add_baseline_test(cmd=cmd % "tuple", baseline=currdir+"tuple.qa", name="tuple")
Test.add_baseline_test(cmd=cmd % "typeManager", baseline=currdir+"typeManager.qa", name="typeManager")
Test.add_baseline_test(cmd=cmd % "vector", baseline=currdir+"vectest.qa", name="vector")
Test.add_baseline_test(cmd=cmd % "sregistry", baseline=currdir+"sregistry.qa", name="sregistry")

if __name__ == "__main__":
    unittest.main()


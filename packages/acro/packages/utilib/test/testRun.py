#!/usr/bin/python
#
# Run tests and gather results.
#
# Results can be displayed to the screen, or sent to a Dart server.
#
# Assumptions: The software under test and its tests have been built
#
#  _________________________________________________________________________
#
#  UTILIB: A utility library for developing portable C++ codes.
#  Copyright (c) 2008 Sandia Corporation.
#  This software is distributed under the BSD License.
#  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
#  the U.S. Government retains certain rights in this software.
#  For more information, see the README file in the top UTILIB directory.
#  _________________________________________________________________________
#

import sys
import os 
import re
import xml.parsers.expat

###################################################################
# This section should be modified for each site
###################################################################

# Directories relative to this script that contain cxxunit tests.
# We assume each test suite is decribed in a header file of the form T*.h,
# and that there is a "runner" executable in each directory.

cxxUnitTestDirs = ["."+os.sep+"unit"]

# Directories relative to this script that contain exact studies tests

exacttestDirs = ["."+os.sep+"studies"]

# Directories relative to this script that contain pyUnit tests

pyUnitTestDirs = ["."+os.sep+"unit"]

###################################################################
# Code to read in cxxunit xml output and create a report of results
###################################################################

nl="\n"
infoLine=""
errorLine=""
failure="no"
fname=""

def startElement(name, attrs):
  global nl, infoLine, errorLine, failure
  if (name == "testsuite"):
    infoLine = "Test suite "+attrs["name"]+", tests "+attrs["tests"]+", failures "+attrs["failures"]+nl

  elif (name == "testcase"):
    infoLine = infoLine+"  "+attrs["name"]
  
  elif (name == "failure"):
    infoLine = infoLine+"  FAIL: "+attrs["file"]+", line "+attrs["line"]+"\n"
    failure="yes"

def endElement(name):
  global nl, infoLine, errorLine, failure
  if name == "testsuite":
    infoLine += nl

  elif name == "testcase":
    if failure == "no":
      infoLine = infoLine+" PASS"+nl
    else:
      failure="no"

  elif name == "failure":
    infoLine = infoLine+"    "+errorLine+nl

def charData(data):
  global nl, infoLine, errorLine, failure
  if (len(data) > 1) and (failure == "yes"):
    errorLine += data

def parseCxxtestResults(fname):
  global infoLine
  p = xml.parsers.expat.ParserCreate()
  p.StartElementHandler = startElement
  p.EndElementHandler = endElement
  p.CharacterDataHandler = charData
  infoLine = "" 
  p.ParseFile(fname) 
  return infoLine 

###################################################################
# Find all tests
###################################################################

cxxtestSuiteNames=[]
cxxtestResultFiles=[]

for dir in cxxUnitTestDirs:

  if os.access(dir, os.R_OK) == False:
    print "Problem with cxxtest directory "+dir
    sys.exit(1) 

  entries = os.listdir(dir)

  # Assumption: cxxtest unit tests are defined in T*.h files

  p1 = re.compile("T\w*.h")         

  # Assumption: in that file one or more "class Test* :", a test suite class

  p2 = re.compile("class \s*Test\w*[: ]")

  # Assumption: the test runner is called "runner", and "runner.cpp"
  # contains the text "TEST*.xml", the name of the results file

  p3 = re.compile("TEST\S+\.xml",re.IGNORECASE)

  if not "runner" in entries:
    print "The tests have not been built in "+dir
    # should we run "make check"??, or just run all tests we can find?
    sys.exit(1)

  if not "runner.cpp" in entries:
    print dir+": assumption is runner.cpp is name of test runner source"
    # should we run "make check"??, or just run all tests we can find?
    sys.exit(1)

  testNames = []

  for fname in entries:
    if p1.match(fname) != None:
      tst = open(dir+os.sep+fname, "r") # a T*.h test definition file
      contents = tst.read()
      names = p2.findall(contents)   # the 'class TestBlah' definitions
      for name in names:
        tokens=name.split(" ")
        testNames.append(tokens[1])
      tst.close()

  resultsFileName = "TEST.xml"
  tst = open(dir+os.sep+"runner.cpp")
  contents = tst.read()
  resultsFile = p3.search(contents)
  if (resultsFile != None):
    resultsFileName = resultsFile.group()

  cxxtestSuiteNames.append(testNames)
  cxxtestResultFiles.append(resultsFileName)

print cxxtestSuiteNames
print cxxtestResultFiles




###################################################################
# For testing
###################################################################
#cxxResultFiles=[ "cxxtest/TEST-utilib.xml", "cxxtest/failure.xml"]
#output=[]
#
#for fname in cxxResultFiles:
#  xmlFile = open(fname, "r")
#  testinfo = parseCxxtestResults(xmlFile)
#  output.append(testinfo)
#
#print "Results of C++ Unit Tests"
#print
#for i in range(0,len(output)):
#  print output[i]
#
###################################################################

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
# Typical format to describe tests that are available to run, tests that
# are being requested to be run, and tests that have been run.
#
# <level0  type="directory" select="yes" name="tests/testdir" desc="testdir" >
#   <results>tests/testdir/testResults.txt</results>
#   <level1 type="testsuite" select="yes" name="Tfeature.h" desc="feature suite">
#     <level2 type="test" select="yes" name="testname" desc="testname test">
#        <status>Fail</status>
#        <output></output>
#        <error></error>
#     </level2>
#   </level1>
# </level0>
#
# Number of levels is fixed for a test type class, but can be
# different for different test types.  Names are for the test class,
# descriptions are for the user interface.  "Select" means the user
# can choose that type for execution (all tests in a directory, all
# tests defined in a suite, etc.)  When received back from the
# user, "select" is "yes" if that directory, test, etc has been selected,
# or "no" otherwise.  When tests have been run, the status of
# a test is either "pass", "fail" or "notRun".  If detail is
# requested, that is in the "detail".  This could be test output,
# valgrind output, coverage info, html, etc.
#
# If only one of "name" and "desc" are given, then that is to
# be used both internally and by the user interface.
#
# TODO:
# Given directory name list, maybe we should search recursively
# for test directories.  
#
# TODO:
# Write a signal handler so UI can get progress report while tests
# are running.

import os
import re
import sys
import xml.parsers.expat
from os.path import abspath, dirname

topDir=dirname(dirname(dirname(abspath(__file__))))
sitePyDir=topDir+os.sep+"pyutilib"
sys.path.insert(0,sitePyDir)

#
# run_command(cmd, outfile, cwd)
#  runs command "cmd" in directory "cwd" with stdout/err going to "outfile"
#
from pyutilib.misc.processmngr import run_command
#

from testRunner import testRunner

nl="\n"

## METHODS TO PROCESS XML ##############################################

##
## Process the test definition file, and write out a new one
## that includes pass/fail results from the test run

newXMLContents=""
newXMLIndent=""
newXMLSuite=""
newXMLTest=""
newXMLResults={}
newXMLDirResults={}
newXMLElementRewrite=False

def cxxtestNewXMLInit(results):
  global newXMLContents, newXMLIndent
  global newXMLSuite, newXMLTest
  global newXMLResults, newXMLDirResults
  global newXMLElementRewrite

  newXMLContents=""
  newXMLIndent=""
  newXMLSuite=""
  newXMLTest=""
  newXMLResults=results
  newXMLDirResults={}
  newXMLElementRewrite=False

def cxxtestNewXMLStartElement(name, attrs):
  global newXMLContents, newXMLIndent
  global newXMLSuite, newXMLTest
  global newXMLResults, newXMLDirResults
  global newXMLElementRewrite

  if name == "level0":
    newXMLIndent = ""

  l = newXMLIndent
  nextl = ""

  if name == "status":

    # we may need to rewrite this part of the xml file

    l = l + '<status>'

    if newXMLDirResults != None:
      tname = newXMLSuite + "." + newXMLTest
      tresults = newXMLDirResults[tname]
      if tresults != None:
        newXMLElementRewrite = True
        passFail = tresults[0]
        info = tresults[1]
        l = l + passFail + '</status>' + nl
        if len(info) > 0:
          l = l + newXMLIndent + '<output>' + info + '</output>' + nl

  else: 

    # just copy the line to the XML file
  
    l = l + '<'+name+' '
    for a in attrs:
      l = l + a + '="' + attrs[a] + '" '
    l = l + '>'
  
  
    if name == "level0":
      newXMLIndent = "  "
      newXMLDirResults = newXMLResults[attrs["name"]]
      nextl = "\n"
  
    elif name == "level1":
      newXMLIndent = "    "
      newXMLSuite= attrs["name"]
      nextl = "\n"
  
    elif name == "level2":
      newXMLIndent = "      "
      newXMLTest = attrs["name"]
      nextl = "\n"

  newXMLContents = newXMLContents + l + nextl

def cxxtestNewXMLEndElement(name):
  global newXMLContents, newXMLIndent
  global newXMLElementRewrite

  if newXMLElementRewrite != True:
    newXMLContents = newXMLContents + newXMLIndent + '</'+name+'>'+nl
    newXMLElementRewrite = False

def cxxtestNewXMLCharacterData(data):
  global newXMLContents, newXMLElementRewrite

  if newXMLElementRewrite != True:
    newXMLContents = newXMLContents + data

##
## Process the cxxTest results file, creating a dictionary of results 
##
## Create a dictionary mapping suitename.testname to result

resultsInfo={}
resultsNextTest=""
resultsTestInfo=[]
resultsType=None

def cxxtestResultsInit():
  global resultsInfo, resultsNextTest
  global resultsTestInfo, resultsType
  resultsInfo={}
  resultsNextTest=""
  resultsTestInfo=[]
  resultsType=None

def cxxtestResultsStartElement(name, attrs):
  global resultsNextTest, resultsTestInfo, resultsType

  if name == "testcase":
    resultsNextTest = attrs["classname"]+"."+attrs["name"]
    resultsTestInfo = []

  elif name == "trace":
    resultsType = "pass"

  elif name == "failure":
    resultsType = "fail"

def cxxtestResultsEndElement(name):
  global resultsInfo, resultsNextTest
  global resultsTestInfo, resultsType

  if name == "testcase":
    if len(resultsTestInfo) == 0:
      resultsTestInfo.append("pass")
      resultsTestInfo.append("")
    resultsInfo[resultsNextTest] = resultsTestInfo
    resultsNextTest = ""
    resultsTestInfo = []

  elif name == "trace":
    resultsType = None

  elif name == "failure":
    resultsType = None

def cxxtestResultsCharacterData(data):
  global resultsTestInfo, resultsType

  if resultsType != None:
    if len(resultsTestInfo) == 0:
      resultsTestInfo.append(resultsType)
      resultsTestInfo.append(data)
    else:
      text = resultsTestInfo[1]
      resultsTestInfo = resultsTestInfo[:-1]
      resultsTestInfo.append(text+data)
    

##
## Process the test definition file, running the tests selected 
##

runDirectory=""
runDirectoryDone=False
runSuiteDone=False
runDirectoryList=[]
runNumTests=0

def cxxtestRunInit():
  global runDirectory
  global runDirectoryDone, runSuiteDone
  global runDirectoryList, runNumTests
  runDirectory=""
  runDirectoryDone=False
  runSuiteDone=False
  runDirectoryList=[]
  runNumTests=0

def cxxtestRunStartElement(name, attrs):
  global runDirectory, runSuite
  global runDirectoryDone, runSuiteDone
  global runNumTests
  runOutput="/dev/null"

  if not runDirectoryDone and name in ["level0", "level1", "level2"]:
    if attrs["type"] == "directory":
      runDirectory = attrs["name"]
      if attrs["select"] == "yes":   # run all tests in directory
        run_command("runner", runOutput, runDirectory)
        runDirectoryDone = True
        runNumTests = runNumTests + 1
    elif attrs["type"] == "testsuite" and not runDirectoryDone:
      if attrs["select"] == "yes":   # run all tests in test suite
        run_command("runner "+attrs["name"], runOutput, runDirectory)
        runSuiteDone = True
        runNumTests = runNumTests + 1
    elif attrs["type"] == "test" and not runSuiteDone:
      if attrs["select"] == "yes":   # run one test
        run_command("runner "+attrs["name"], runOutput, runDirectory)
        runNumTests = runNumTests + 1

def cxxtestRunEndElement(name):
  global runDirectoryDone, runSuiteDone, runDirectoryList
  global runNumTests

  if name == "level0":
    if runNumTests > 0:
      runDirectoryList.append(runDirectory)
    runDirectoryDone = False
    runNumTests = 0

  elif name == "level1":
    runSuiteDone = False

##########################################################################

def addOuterElement(xmltext):
  # Problem:  If the outermost element of the xml file 
  #  occurs more than once, expat returns an error upon
  #  parsing the second occurence.  Create a dummy outermost
  #  element.

  newtext = ""
  lines = xmltext.split(nl)

  if "?xml" in lines[0] or "?XML" in lines[0]:
    remaining = nl.join(lines[1:])
    newtext = lines[0] + nl + "<dummy>" + nl 
    newtext = newtext + remaining + nl
    newtext = newtext + "</dummy>" + nl
  else:
    newtext = "<dummy>" + nl + xmltext + nl + "</dummy>"

  return newtext

def runSelectedTests(selectedTests):
  p = xml.parsers.expat.ParserCreate()
  cxxtestRunInit()
  p.StartElementHandler = cxxtestRunStartElement
  p.EndElementHandler = cxxtestRunEndElement

  selectedTests = addOuterElement(selectedTests)

  p.Parse(selectedTests)

  return runDirectoryList


def makeResultsDictionary(results):
  p = xml.parsers.expat.ParserCreate()
  cxxtestResultsInit()
  p.StartElementHandler = cxxtestResultsStartElement
  p.EndElementHandler = cxxtestResultsEndElement
  p.CharacterDataHandler = cxxtestResultsCharacterData

  results = addOuterElement(results)

  p.Parse(results)

  return resultsInfo

def makeXMLResults(selectedTests, resultsDirs, resultsMaps):
  r = {}
  for i in range(0, len(resultsDirs)):
    r[resultsDirs[i]] = resultsMaps[i]
  cxxtestNewXMLInit(r)

  p = xml.parsers.expat.ParserCreate()
  p.StartElementHandler = cxxtestNewXMLStartElement
  p.EndElementHandler = cxxtestNewXMLEndElement
  p.CharacterDataHandler = cxxtestNewXMLCharacterData

  selectedTests = addOuterElement(selectedTests)

  p.Parse(selectedTests)

  return newXMLContents

##########################################################################
## A test runner, specific to cxxTest tests, that is instantiated by some
## user interface.  It can discover the available tests, accept
## a list of selected tests, run the tests, and return the results.
##########################################################################

class cxxtestRunner(testRunner):

  testDirs=[]            # list of directories in which we found tests
  resultFiles=[]         # cxxTest result file in each directory

  def setDirectories(self, dl):
    """ Provide the list of directories that tests may be found in. """

    testRunner.setDirectories(self, dl)

  def parseTestDefinitions(self):
    """ Find all tests in the list of directories.

        Save enough information about them, in a hierarchical
        XML format, so that we can run the tests, find the
        results, and clean up after the tests.
    """

    if len(self.dirs) < 1:
      raise RuntimeError("no test directories named")

    self.testDefs = ""
    self.testDirs = []
    self.resultFiles= []

    # Assumption: cxxTest unit tests are defined in T*.h files

    p1 = re.compile("T\w*.h")

    # Assumption: in that file is one or more "class Test* :", a test suite class

    p2 = re.compile("class \s*Test\w*[: ]")
    p3 = re.compile("Test\w*")

    # Assume the tests within a suite look like "void test*"

    p4 = re.compile("void \s*test\w*")

    # Assumption: the test runner is called "runner", and "runner.cpp"
    # contains the text "TEST*.xml", the name of the results file.
    # There is one runner and results file per directory.

    p5 = re.compile("TEST\S+\.xml",re.IGNORECASE)

    for dir in self.dirs:

      if os.access(dir, os.R_OK | os.W_OK) == False:
        raise RuntimeError("can't access "+dir)

      entries = os.listdir(dir)
  
      if not "runner" in entries:
        continue
  
      if not "runner.cpp" in entries:
        continue

      suiteNames = []
      testNames = []
  
      for fname in entries:
        if p1.match(fname) != None:
          tst = open(dir+os.sep+fname, "r") # a T*.h test definition file
          contents = tst.read()
          tst.close()

          suiteLoc=[]                   # find all test suites in file
          suite = p2.search(contents)
          while suite != None:
            suiteLoc.append(suite.start())
            suite = p2.search(contents[suite.end():])

          for i in range(0, len(suiteLoc)):
            sn = p3.search(contents[suiteLoc[i]:])   # suite name
            suiteNames.append(sn.group())
            if i+1 < len(suiteLoc):
              classText = contents[suiteLoc[i]:suiteLoc[i+1]]
            else:
              classText = contents[suiteLoc[i]:]

            tmpList = []
            tnames = p4.findall(classText)           # test names
            for tn in tnames:
              tokens = tn.split()
              tmpList.append(tokens[1])

            testNames.append(tmpList)
    
      if len(suiteNames) == 0:
        continue

      tst = open(dir+os.sep+"runner.cpp")
      contents = tst.read()
      tst.close()
      rfile = p5.search(contents)
      if (rfile != None):
        self.resultFiles.append(rfile.group())
        self.testDirs.append(dir)
      else:
        raise RuntimeError(dir+": unexpected contents in runner.cpp")

      self.testDefs  = self.testDefs + '<level0 type="directory" '
      self.testDefs  = self.testDefs + 'select="yes" '
      self.testDefs  = self.testDefs + 'name="'+dir+'">'+nl

      self.testDefs  = self.testDefs + '  <results>'+dir+os.sep+rfile.group()
      self.testDefs  = self.testDefs + ' </results>'+nl

      i = 0
      for sn in suiteNames:
        self.testDefs  = self.testDefs + '  <level1 type="testsuite" '
        self.testDefs  = self.testDefs + 'select="yes" '
        self.testDefs  = self.testDefs + 'name="'+sn+'">'+nl

        for tn in testNames[i]:
          self.testDefs  = self.testDefs + '    <level2 type="test" '
          self.testDefs  = self.testDefs + 'select="yes" '
          self.testDefs  = self.testDefs + 'name="'+tn+'">'+nl
          self.testDefs  = self.testDefs + '    <status>NotRun</status>'+nl
          self.testDefs  = self.testDefs + '    </level2 >'+nl

        i = i + 1
        self.testDefs  = self.testDefs + '  </level1>' + nl
      
      self.testDefs  = self.testDefs + '</level0>' + nl

    #next directory


  def getTestDefinitionDepth(self):
    """ How many levels are in the XML hierarchical test description? """

    return 3

  def getTestDefinitions(self):
    """ Return the current list of test definitions. """

    if self.testDefs == "":
      self.parseTestDefinitions()

    return self.testDefs

  def runAllTests(self):
    """ Run all tests found when parsing the test definitions. """

    if self.testDefs == "":
      self.parseTestDefinitions()

    return self.runTests(self.testDefs)

  def runTests(self, selectedTests):
    """ Run the selected tests in the provided list. """

    # Run each selected test

    dirList = runSelectedTests(selectedTests)

    # Gather the results of each test into a dictionary

    resultDirs=[]
    resultsMaps=[]

    for i in range(0, len(self.testDirs)):
      if self.testDirs[i] in dirList:
        rfile = self.testDirs[i]+os.sep+self.resultFiles[i]
        if os.path.exists(rfile):
          tst = open(rfile, "r")
          contents = tst.read()
          tst.close()

          resultsMaps.append(makeResultsDictionary(contents))
          resultDirs.append(self.testDirs[i])

    cxxtestResultsInit()    # clean up memory

    # Use the selected tests and the results to create a new XML
    # file containing results of the selected tests

    testResults = makeXMLResults(selectedTests, resultDirs, resultsMaps)

    r = {}
    cxxtestNewXMLInit(r)    # clean up memory

    return testResults

  def cleanTestResults(self):
    """ Remove all results files and directories. """

    for i in range(0, len(self.testDirs)):
      fname = self.testDirs[i]+os.sep+self.resultFiles[i]
      if os.path.exists(fname):
        os.remove(fname)

  def writeDetailToDartFormatXML(self, selectedTests):
    """ For the tests in the list, write Dart format XML with results.

        Return the name of a directory with the resulting XML files.
        Caller is responsible for deleting this directory after sending
        files to a Dart server.
    """

##
## module test

if __name__ == "__main__":
  runner = cxxtestRunner()
  runner.setDirectories(["unit","unit2"])
  runner.parseTestDefinitions()

  results = runner.runAllTests()

  print "RESULTS"
  print results

  runner.cleanTestResults()

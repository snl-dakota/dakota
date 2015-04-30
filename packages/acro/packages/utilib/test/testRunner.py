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
# A signal handler can reply with the tests results so far while
# tests are being run.  The UI can display progress


class testRunner:

  dirs=["."]
  testDefs=""

  def setDirectories(self, dl):
    """ Provide the list of directories that tests may be found in. """

    self.dirs=dl

  def parseTestDefinitions(self):
    """ Find all tests in the list of directories.

        Save enough information about them, in a hierarchical
        XML format, so that we can run the tests, or find the
        test results.
    """

  def getTestDefinitionDepth(self):
    """ How many levels are in the XML hierarchical test description? """

  def getTestDefinitions(self):
    """ Return the current list of test definitions. """

  def runAllTests(self):
    """ Run all tests found when parsing the test definitions.

        Return the test descriptions with brief results.
    """

  def runTests(self, selectedTests):
    """ Run the selected tests in the provided list.

        Return the test descriptions with brief results.
    """

  def getDetailedResults(self, selectedTests):
    """ If more detailed test results are possible, get these.

        Some test types may have lengthy output, like valgrind results.
    """

  def cleanTestResults(self):
    """ Remove files created by the last run of tests. """

  def writeDetailToDartFormatXML(self, selectedTests):
    """ For the tests in the list, write Dart format XML with results.

        Return the name of a directory with the resulting XML files.
        Caller is responsible for deleting this directory after sending
        files to a Dart server.
    """

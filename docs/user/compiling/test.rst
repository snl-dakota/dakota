.. _test:

""""""""""""""""""""
Running Dakota Tests
""""""""""""""""""""

Dakota's test suite primarily consists of system-level regression tests
supplemented with a growing number of unit and system-level
interfacing and workflow tests.

=================
Basic Smoke Tests
=================

Verify Dakota executes::

  $DAK_BUILD/src/dakota -version

  Dakota version 6.16+ (stable) released Oct 31 2022.
  Repository revision 56f2a6a (2022-10-29) built Oct 31 2022 16:31:22.

Run a basic study as described in :ref:`helloworld-main`.


==========
Unit Tests
==========

Dakota unit tests should pass on most platforms. To run them::

  cd $DAK_BUILD

  ctest -L Unit

  # Or for concurrent test execution:
  # ctest -j 4 -L Unit


================
Regression Tests
================

Most of Dakota regression tests should pass when run in the
recommended RHEL 7 development environment. They commonly produce
numerical differences (reported as DIFF) when run on other platforms.

Recommended minimal test suite::

  cd $DAK_BUILD

  # test suite that should PASS, even for non-development, but supported hosts (e.g. MS Windows)
  ctest -j 4 -L AcceptanceTest

.. attention::

   Dakota's ctest reports a test producing a DIFF as a FAILure. To
   avoid executing tests that are known to sporadically DIFF (e.g. a
   test exercising a stochastic algorithm), ctest's exclude feature
   can be used to avoid the frustration of needing to perform further
   diagnostics to determine whether a test FAIL is indeed genuine.

CTest labels or regular expressions can be used to select subsets of
tests to run. Here are some examples, which overlap each other::

  # "pre-commit" set of fast-running tests that will PASS on typical developer's local host
  ctest -j 4 -L FastTest -LE Diff

  # run all Dakota tests active in this build (should PASS or DIFF)
  # (can also do at top-level $DAK_BUILD to run all unit and regression tests)
  ctest -j 4

  # run all tests excluding those known to DIFF on RHEL 7:
  ctest -j 4 -LE Diff

  # Show all available ctest labels
  ctest  --print-labels

  # Show all relevant tests but do not run; can be used with various labels (-L) or regular expressions (-R)
  ctest -N -R dakota_


Sub-tests: Each Dakota CTest regression test corresponds to a file
``test/dakota_*.in`` and represents a coarse level of testing in that
it may be comprised of multiple variations of a base test. If any of
these variations report DIFF or FAIL, the entire CTest test reports a
FAIL. To obtain more granular test information, a dakota_diffs.out
summary can be generated via::

  # Unix only: generate difference files dakota_diffs.out, dakota_pdiffs.out
  make dakota-diffs

The resulting dakota_[p]diffs.out file can then be inspected to
ascertain which test variation(s) failed, e.g.::

  dakota_experiment_read.in
  PASS test 0
  PASS test 1
  FAIL test 2 (*** new ***)
  FAIL test 3 (*** new ***)

A successful build should include PASS and DIFF, with associated small
numerical differences.  If these files contain DIFF with large
aberrations, or FAIL, there may be an issue with the build.

If any credible test differences are found, a starting point for
determing the cause of failure involves the following steps:

# Determine which test variation(s) is (are) failing by invoking
  ``make dakota-diffs`` from within the test subdirectory.  Examine
  the resulting :file:`dakota_[p]diffs.out` file to see which failures
  are new.

# Change into the subdirectory corresponding to the failing test, e.g.,
  ``cd dakota_experiment_read``.
  
# Extract the test variation that is failing using the
  dakota_test.perl utility, e.g. for test #3 of
  ``dakota_experiment_read``::

    cd $DAK_BUILD/test/dakota_experiment_read
    ../dakota_test.perl --extract --file-extract test3.in dakota_experiment_read.in 3

# The sub-test will be extracted to ``test3.in``. Run this sub-test
and inspect the console output and/or log files for details explaining
why the test did not pass.::

    ../../src/dakota -input test3.in

.. note::

   Note that documentation for using the ``dakota_test.perl`` script can
   be obtained by invoking it with either ``--help`` (for brief help) or ``--man``.

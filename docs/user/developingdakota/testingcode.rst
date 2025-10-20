.. _testingcode-main:

"""""""""""""""""""
Testing Dakota Code
"""""""""""""""""""


==========
Unit Tests
==========

Unit tests are intended for testing specific units, classes and functions
when they can be readily constructed (and/or provided mocks) as needed.
Unit testing also serves as a mechanism for Test Driven Development
(TDD) which represents a best practice for implementing new capability.
Just a few of the benefits of TDD include the following:

 - Enforces (and measures) modularity of code and functionality
 - Encourages incrementally correct development
 - Ensures correct behavior
 - Documents API and software contract
 - Promotes code coverage and other Software Quality Assurance (SQA) metrics

Historically, Dakota has used both Boost.Test and Teuchos Unit Test
features but has recently officially adopted the former.  For a minimal
example for unit testing see:

- :file:`src/unit/min_unit_test.cpp` (Boost.Test unit_test.hpp)
- :file:`src/unit/leja_sampling.cpp` (Boost.Test minimal.hpp)

Some more recent / modern examples include:

- :file:`src/util/unit/MathToolsTest.cpp`
- :file:`src/util/unit/LinearSolverTest.cpp`
- :file:`src/surrogates/unit/PolynomialRegressionTest.cpp`

To add a test with a TDD mindset:

#. Call out the new unit test source file, e.g. ``my_test.cpp``, in
   ``CMakeLists.txt``,e.g. :file:`src/surrogate/unit/CMakeLists.txt`.
   See helper functions: dakota_add_unit_test (adds test, links libraries,
   registers with ctest), dakota_copy_test_file (copies with dependency).
   The build will fail because the file does not yet exist.

#. Add a new file ``my_test.cpp`` with a failing test macro,
   e.g. ``BOOST_CHECK(false)`` to verify it builds but the test fails.
   Name files and associated data directories in a helpful and consistent
   manner.

#. Use Boost utilities/macros to assess individual test conditions PASS /
   FAIL as needed.

#. Compile and run with ``ctest –L UnitTest`` or ``ctest –R my_test``.

#. Iteratively add and refine tests and modify Dakota core source code
   as the capability evolves.


To run all unit tests:

.. code-block::

   cd dakota/build/ 
    
   # Run all unit tests:
   ctest -L (--label-regex) UnitTest
    
   # With detailed output at top-level:
   ctest -L (--label-regex) UnitTest -VV (--extra-verbose)
    
   # To run a single test, via regular expression:
   ctest -L UnitTest -R surrogate_unit_tests


A failing CTest unit test can be diagnosed using the following as a
starting point:

.. code-block::

   cd build/src/unit (or other directory containing the test executable)
    
   # First, manually run the failing test to see what information is provided related to the failure(s):
   ./surrogate_unit_tests

   # To see available Boost Test options:
   ./surrogate_unit_tests --help
    
   # To get detailed debugging info from a unit test:
   ./surrogate_unit_tests --log_level all
    
.. note::

   A google search can also provide current best practices with
   Boost.Test and specifics related to the details of the test
   failure(s)


================
Regression Tests
================

Regression tests compare the output of complete Dakota studies against
baseline behavior to ensure that changes to the code do not cause unexpected
changes to output. Ideally they are fast running and use models 
with known behavior such as polynomials or other canonical problems.

The following are a few key concepts in Dakota's regression test system:

- In the source tree, most important test-related content is located
  in the test/ directory. Test files are named dakota_*.in. Each test file
  has a baseline file named dakota_*.base. Some tests have other
  associated data files and drivers.
- Configuring Dakota causes test files and associated content to be copied
  to subfolders within the test/ folder of the build tree. This is where
  they will be run.
- A single test file can contain multiple numbered serial and parallel subtests. Each
  subtest, after extraction from the test file, is a valid Dakota input file.
- Tests usually should be run using the ``ctest`` commmand. CTest uses the
  script dakota_test.perl, which is located in the test directory, to do most
  of the heavy lifting. This script can be run from the command line, as well.
  Run it with the argument ``--man`` for documentation of its options.
- Subtests can be categorized and described using CTest labels. (use 
  ``ctest --print-labels`` in a build tree to view labels of existing tests).
  One purpose of labels is to state whether an optional component of Dakota
  is needed to run the test.

Running Regression Tests
------------------------

Dakota's full regression test suite contains approximately 300 test files
and more than a thousand subtests. It typically takes between several tens of
minutes to a few hours to complete, depending on available computing resources.
The test system executes Dakota for each subtest, collects the output, and
compares it to a baseline. There are three possible results for a subtest:

- **PASS**: Dakota output matched the baseline to within a numerical tolerance
- **DIFF**: Dakota ran to completion, but its output did not match the baseline
- **FAIL**: Dakota did not run to completion (it failed to run altogether or returned nonzero)

In a Dakota build tree, the ``ctest`` command is the best way to run Dakota
tests, including regression tests. Running the command with no options runs all the tests
sequentially. A few helpful options:

- ``-j N``: Run N tests concurrently. Be aware that some of Dakota's
  regression tests may make use of local or MPI parallelism and may
  use multiple cores.
- ``-L <label>``: Run only those tests whose label matches the regex <label>.
  To run only regression tests (and not, e.g. unit tests), use the label ``Regression``.
- ``-R <name>``: Run only those tests whose name matches the regrex <name>. The

It currently is not possible to run specific subtests; all subtests of a test
selected by label or name will be run.

During configuration, test files, baselines, and auxilliary content are copied
from the source tree to the build tree, where tests will be run. For each test
file, a subdirectory is created in the test/ directory. The subdirectories have the
names of their test files, minus the ``.in`` extension. If Dakota was built with parallel
support, an additional subfolder is created for any parallel subtests in a test file.
Its name is the test name with the letter ``p`` prepended.

The results of each test are located in their subfolders. For serial subtests, the results
are in the file ``dakota_diffs.out``. For parallel subtests, the results file is ``dakota_pdiffs.out``.
These files state whether each subtest PASSed, DIFFed, or FAILed. If the test DIFFed, a diff
of the Dakota console output and baseline is listed.

The make target ``dakota-diffs`` causes all the ``dakota_diffs.out`` files from individual tests
to be concatenated into a one ``dakota_diffs.out`` in the test/ directory, and similarly for the
``dakota_pdiffs.out`` files.

Subsequent runs of ``ctest`` will cause test results to be appended to existing ``dakota_diffs.out``
files. The make target ``dakota-diffs-clean`` freshens the test/ folder.

.. note::
	While Dakota's test system has three possible test results (PASS, DIFF, FAIL), CTest has only
	two (PASS or FAIL) and reports Dakota DIFFs as failures. Quite often tests that CTest reports
	as failing are exhibiting only minor numerical differences from baseline and are no cause for
	concern. Check dakota_diffs.out/dakota_pdiffs.out for "failing" tests before concluding that there's
	a problem with your Dakota build.

The ``ctest`` command uses the script ``dakota_test.perl`` and its helper ``dakota_diff.perl``
to extract subtests, run Dakota to produce test output, and diff the results against
the baseline. It is possible to run ``dakota_test.perl`` from the command line. Use the argument
``--man`` to see its options. (The ``-e`` option to extract a subtest is particularly useful.)

If a regression test fails, steps to diagnose the failure include the
following which are performed in the Dakota build directory:

#. Remove previous test artifacts related to detailed differences and
   failures via ``make dakota-diffs-clean``.

#. By default, ``dakota_test.perl`` overwrites Dakota output after each subtest. Set the
   ``DAKOTA_TEST_SAVE_OUTPUT`` environment variable to 1 to save it.

#. Rerun the failing CTest: ``ctest -R test_name``. (This regex will catch both the serial and parallel
   subtests. Add a carat (``^``) at the beginning of the pattern to exclude the parallel subtests.)

#. Generate details for how the test differs from the corresponding
   baseline: ``make dakota-diffs``.

#. Go into the specific regression test directory and examine the
   ``dakota_diffs.out`` file to see which subtest(s) failed.

#. Compare the ``.tst`` file contents with the ``.base`` file contents
   to determine which values have changed, if there was a catastrophic
   failure of the executable, etc.

Creating a New Regression Test
------------------------------

A complete regression test includes a test file (extension ``*.in``), which can contain multiple
subtests, a baseline (extension ``*.base``), and any auxilliary files (such as data files or
drivers) needed by the subtests.

Writing Subtests
^^^^^^^^^^^^^^^^^^^

Including multiple subtests within a single test file reduces maintenance
burden by allowing related test cases to share Dakota specifications that
they have in common.

A test file is just a Dakota input file that has been annotated to indicate
the lines that belong to each subtest. Subtests are numbered, beginning with 0,
and serial and parallel subtests have independent numbering. The rules for
annotating the lines of a test file are:

- Lines required for all test cases should be left uncommented.
- A line that should only be activated for specific subtests should be commented
  out, and the label ``#sN`` or ``#pN`` for serial and parallel tests, respectively,
  should be added to the end. ``N`` is the integer associated with a subtest.
  When ``dakota_test.perl`` extracts a subtest from the test file, it will keep all
  uncommented lines and also lines that end with a the corresponding subtest tag.
- The tag ``#s0`` has a special meaning. Serial subtest 0 is considered to include
  all uncommented lines in the original test input file. Lines with the ``#s0`` tag should
  not be commented out. They will not be extracted for other subtests. This convention allows
  the 0th serial subtest to be runnable without extraction.
- If a line in the input file is used in multiple subtests (but not all), the tags should
  appear in a comma-separated list. For example, if a line in the input file belongs to
  serial subtests 1 and 2, the line should end in ``#s1,#s2``.

Test Directives
^^^^^^^^^^^^^^^

The test creator can (and in some cases must) provide additional
information to Dakota's test system through the use of directives.
Directives must appear at the top of the file, but can be in any
order. They have the format:

``#@ <subtest specifier> = <directive>``

The subtest specifier indicates which of the subtests the directive applies to.
Its format is similar to the subtest tag:

- ``sN`` or ``pN``, without a pound sign.
- ``N`` can be a subtest number or, if the directive applies to all serial or parallel
  subtests in teh file, ``*``.

.. table:: Regression Test Directives
   :widths: auto

   ======================== ================================================================================= 
   Directive                Meaning
   ======================== ================================================================================= 
   Label=FastTest           A quick-running test.                                                            
   Label=Experimental       Experimental capability. For information only.                                   
   Label=AcceptanceTest     Acceptance tests rarely diff on any platform                                     
   DakotaConfig=FLAG        Only run this test if the CMake variable FLAG is true.                           
   MPIProcs=N               Execute Dakota in parallel with N MPI tasks                                      
   TimeoutDelay=N           Terminate Dakota if console output is unchanged for N seconds (default is 60s)   
   TimeoutAbsolute=N        Terminate Dakota if the subtest takes longer than N seconds (default is 1200 s)  
   CheckOutput='FILENAME'   Dakota output for the test will appear in FILENAME instead of the default        
   Restart=read             Read the restart file written by the last subtest. Implies Restart=write.        
   Restart=write            Write a restart file (usually to be used by a future subtest)
   Restart=none             No restart option                 
   DependsOn=N              This subtest depends on another. Currently for information only.                 
   ExecCmd='CMD'            Instead of 'dakota', run this command.                                           
   ExecArgs='ARGS'          Arguments passed to command                                                      
   InputFile='INPUT'        Specify an input file instead of the extracted one.                                                                
   UserMan='FILENAME'       Extract subtest to FILENAME for use in the User's Manual
   ======================== ================================================================================= 

A few notes on directives:

- Labels can be used to filter tests using the ``-L`` option to ``ctest`` or the ``--label-regex`` option
  to ``dakota_test.perl``.
- ``MPIProcs`` is required for parallel tests.

Including Auxilliary Files
^^^^^^^^^^^^^^^^^^^^^^^^^^

Auxilliary files can be included in a test two ways: 

- By giving them the same base name as the regression test, e.g., if the test file is named
  dakota_newtest.in, auxilliary files matching the regular expression dakota_newtest.* will
  be copied to the test subfolder. 
- By including the directive ``ReqFiles`` in the test file. For example, if auxilliary files 
  are named ``aux1.dat``, ``aux2.dat`` the directive ``ReqFiles=aux1.dat,aux2.dat`` will ensure 
  the files are copied to the test subfolder. 

Creating a New Baseline File
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To create a new baseline dakota_*.base file for serial
regression tests, call 

.. code-block::

	dakota_test.perl --base name_of_new_input_file.in
   
This will create a file with extension .base.new with the same
basename as the input file. Check the results, then change 
the extension to .base to incorporate it into the test suite.

More advanced options for generating baseline
files (e.g., for parallel tests) and more details about creating 
baselines are available in ``dakota_test.perl --man``. 

Example Test File
^^^^^^^^^^^^^^^^^

An example input test file demonstrating a few of these features is below.

.. code-block::

   #@ s*: Label=FastTest
   #@ s0: DakotaConfig=HAVE_QUESO

   method
     bayes_calibration queso #s0
       chain_samples = 100 seed = 100 #s0
   #  sampling #s1,#s2
   #    sample_type lhs #s1
   #   sample_type random #s2
   #    samples = 100 #s1,#s2
   #   seed = 17 #s1,#s2
   
   variables
     uniform_uncertain 2
       lower_bounds -2. -2. 
	   upper_bounds  2.  2.
   
   interface
    analysis_driver = 'rosenbrock'
     direct

   responses
    objective_functions = 1
    no_gradients
    no_hessians

This input file has three test cases: the first (s0) is Bayesian 
calibration using QUESO, the second (s1) is LHS sampling, and the 
third (s2) is random sampling. All the input file lines that are 
shared between the test cases are uncommented. Note that the
lines specific to Subtest s0 that should not appear in the 
input files for Test Cases 1 and 2 have ``#s0`` appended to
them.

The test has the label ``FastTest`` and will only be run when
Dakota is built with the optional QUESO component.


=============================
Unit Test-driven System Tests
=============================

These hybrid tests can be useful when it's difficult to mock up all
the objects needed for testing, e.g., Dakota Model, Variables, Interface,
Responses, and yet finer-grained control over results verification is
desired compared with that of regression tests.  One way to view these
types of unit tests are those that construct most of a complete Dakota
study as a mock and which then do fine-grained testing of selected
functionality from the instantiated objects.  In brief, these tests:

- Are registered as unit tests

- Operate at the level of constructing a Dakota Environment from an
  input file and running a whole study to populate needed class data

- Test criteria that are more fine-grained and controllable than
  regression tests

An illustrative example is described next and in
:file:`src/unit_test/opt_tpl_rol_test_textbook.cpp`.


The following provides a walkthrough for developers who wish to add a
Test-driven System unit test that includes an end-to-end Dakota
analysis. The procedure relies on setting up a problem description
database using a Dakota input string and subsequently executing the
environment. The last step involves extracting the quantities of
interest (results) to be tested using unit test macros.

Test environment definition
---------------------------

The developer defines a testing environment by constructing a problem
description database from a Dakota input string, e.g.

.. code-block::

	// Dakota input string for serial case (cyl_head):
	static const char dakota_input[] = 
	  " method,"
	  "   output silent"
	  "   max_function_evaluations 300"
	  "   mesh_adaptive_search"
	  "     threshold_delta = 1.e-10"
	  " variables,"
	  "   continuous_design = 2"
	  "     initial_point    1.51         0.01"
	  "     upper_bounds     2.164        4.0"
	  "     lower_bounds     1.5          0.0"
	  "     descriptors      'intake_dia' 'flatness'"
	  " interface,"
	  "   direct"
	  "     analysis_driver = 'cyl_head'"
	  " responses,"
	  "   num_objective_functions = 1"
	  "   nonlinear_inequality_constraints = 3"
	  "   no_gradients"
	  "   no_hessians"; 

The input string is then used to create a Dakota environment:

.. code-block:: cpp

	// No input file set --> no parsing:
	Dakota::ProgramOptions opts;
	opts.echo_input(false);

	opts.input_string(dakota_input);

	// delay validation/sync of the Dakota database and iterator
	// construction to allow update after all data is populated
	bool check_bcast_construct = false;

	// set up a Dakota instance
	Dakota::LibraryEnvironment * p_env = new Dakota::LibraryEnvironment(MPI_COMM_WORLD, opts, check_bcast_construct);
	Dakota::LibraryEnvironment & env = *p_env;
	Dakota::ParallelLibrary& parallel_lib = env.parallel_library();

	// configure Dakota to throw a std::runtime_error instead of calling exit
	env.exit_mode("throw");

	// once done with changes: check database, broadcast, and construct iterators
	env.done_modifying_db();


Executing the environment
-------------------------

Once an environment is defined, instantiation of Dakota objects and
population of class data is achieved by executing the study:

.. code-block:: cpp

	// Execute the environment
	env.execute();


Extracting results and test assertions
--------------------------------------

Following execution, the pertinent results are extracted and used to
test correctness criteria. This is performed using the Boost unit test
capabilities, e.g.

.. code-block:: cpp

	// retrieve the final parameter values
	const Variables& vars = env.variables_results();

	// retrieve the final response values
	const Response& resp  = env.response_results();

	// Convergence test: check that first continuous variable
	// has reached optimal value within given tolerance
	double target = 2.1224215765;
	double max_tol = 1.e-5;
	double rel_err = fabs((vars.continuous_variable(0) - target)/target);
	BOOST_CHECK(rel_err < max_tol);

	// Convergence test: check that second continuous variable
	// has reached optimal value within given tolerance
	target = 1.7659069377;
	max_tol = 1.e-2;
	rel_err = fabs((vars.continuous_variable(1) - target)/target);
	BOOST_CHECK(rel_err < max_tol);

	// Convergence test: check that the final response value
	// has reached the corresponding minimum within given tolerance
	target = -2.4614299775;
	max_tol = 1.e-3;
	rel_err = fabs((resp.function_value(0) - target)/target);
	BOOST_CHECK(rel_err < max_tol);

Unit test macros
----------------

There are several unit test macros to support
various comparisons, assertions, exceptions, etc.  See
https://www.boost.org/doc/libs/1_69_0/libs/test/doc/html/boost_test/utf_reference/testing_tool_ref.html
for details and exmaples.

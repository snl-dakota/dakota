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

These tests involve complete Dakota studies albeit typically small,
fast and using models that represent known behavior such as polynomials
or other canonical problems.  In brief, these tests:

- Are primarily located in source/test/dakota_*.in with gold standards
  in dakota_*.base
- Are categorized using CTest labels, e.g. use ``ctest --print-labels``
  to view them
- Are usually wrappers to the native Dakota ``test/dakota_test.perl``
  utility, e.g. use the option ``--man`` for supported options

If a regression test fails, steps to disgnose the failure include the
following which are performed in the Dakota build directory:

#. Remove previous test artifacts related to detailed differences and
   failures via ``make dakota-diffs-clean``.

#. Rerun the failing CTest: ``ctest -R test_name``

#. Generate details for how the test differs from the corresponding
   baseline: ``make dakota-diffs``.

#. Go into the specific regression test directory and examine the
   ``dakota_diffs.out`` file to see which subtest(s) failed.

#. Compare the ``.tst`` file contents with the ``.base`` file contents
   to determine which values have changed, if there was a catastrophic
   failure of the executable, etc.


=============================
Unit Test-driven System Tests
=============================

These hybrid tests can be useful when it's difficult to mock up all
the objects needed for testing, e.g., Dakota Model, Variables, Interface,
Responses, and yet finer-grained control over results verification is
desired compared with that of regression tests.  One way to view these
types of unit tests are those that construct most of a complete Dakota
study as a mock and which then do fine grained testing of selected
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

.. code-block::

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

.. code-block::

	// Execute the environment
	env.execute();


Extracting results and test assertions
--------------------------------------

Following execution, the pertinent results are extracted and used to
test correctness criteria. This is performed using the Boost unit test
capabilities, e.g.

.. code-block::

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

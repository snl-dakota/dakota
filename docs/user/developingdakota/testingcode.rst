.. _testingcode-main:

"""""""""""""""""""
Testing Dakota Code
"""""""""""""""""""


==========
Unit Tests
==========

Intended for testing specific units, classes, functions, when they can
be readily constructed (and/or provided mocks) as needed.

Historically we've used both Boost.Test and Teuchos Unit Test
features. For minimal unit test examples, two ways to do with Boost
and one with teuchos; see:

- :file:`src/unit_test/min_unit_test.cpp` (Boost.Test unit_test.hpp)
- :file:`src/unit_test/leja_sampling.cpp` (Boost.Test minimal.hpp)
- :file:`src/unit_test/demo_teuchos.cpp` (Teuchos)

.. note::

   Prefer Boost test when adding new unit tests, but use Teuchos if
   needed. Overall Teuchos has easier to use numerical computing
   testing macros, but if you use the full Boost testing library, it's
   not really that different. *Rationale: it's better to protect with
   tests than not write them due to not picking...*

Some more recent / modern examples:

- :file:`src/util/unit/MathToolsTest.cpp`
- :file:`src/util/unit/LinearSolverTest.cpp` (Teuchos)
- :file:`src/surrogates/unit/PolynomialRegressionTest.cpp` (Boost)

To add a test with a TDD mindset:

#. Call out the source file ``my_test.cpp`` in ``CMakeLists.txt``,
   e.g., :file:`src/surrogate/unit/CMakeLists.txt` by adding to an
   existing or new test suite. Build will fail.

#. Add a new ``my_test.cpp`` with a failing test macro to verify
   build, but test fails.

#. Iteratively add and refine tests and core source code as the
   capability evolves.

To run all unit tests:

.. code-block::

   cd dakota/build/ 
    
   # Run all unit tests:
   ctest -L (--label-regex) UnitTest
    
   # With detailed output at top-level:
   ctest -L (--label-regex) UnitTest -VV (--extra-verbose)
    
   # To run a single test, via regular expression:
   ctest -L UnitTest -R surrogate_unit_tests


For Teuchos-based unit tests:

.. code-block::

   cd build/src/unit_test (or other directory containing the test executable)
    
   # To see available sub-tests in this suite:
   ./surrogate_unit_tests --no-op
    
   # To get detailed debugging info from a Teuchos-based unit test group (wildcards are also supported):
   ./surrogate_unit_tests --group="reduced_basis"
    
   # Or a particular test by name
   ./surrogate_unit_tests --test-name="*yaml*"

================
Regression Tests
================

- Primary located in source/test/dakota_*.in and gold standards in dakota_*.base
- Documentation mainly in ``test/dakota_test.perl --man``


=============================
Unit Test-driven System Tests
=============================

These hybrid tests can be useful when it's difficult to mock up all
the objects needed for testing, e.g., Dakota Model, Variables,
Interface, Responses, yet want finer-grained control over results
verification than with regression tests (below).

- Registered as unit tests

- Operate at the level of constructing a Dakota Environment from an
  input file and running a whole study

- Test criteria are more fine-grained and controllable

- Key Example: Authoring unit tests for a Dakota input-file driven
  example (see below and
  :file:`src/unit_test/opt_tpl_rol_test_textbook.cpp`.

This section provides a walkthrough for developers who wish to add a
performance-based unit test that includes an end-to-end Dakota
analysis. The procedure relies on setting up a problem description
database using a Dakota input string and subsequently executing the
environment. The last step involves extracting the quantities of
interest (results) and testing an assertion for pass/fail labeling of
the test.

Test environment definition
---------------------------

The developer defines a testing environment by providing a problem
description database using a Dakota input string, e.g.

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

The input string is then used in creating a Dakota environment:

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

Once an evnironment is defined, we proceed with execution:

.. code-block::

	// Execute the environment
	env.execute();


Extracting results and test assertions
--------------------------------------

Following execution, the pertinent results are extracted and used in
testing assertions of interest. This is performed using the Teuchos
unit testing capabilities, e.g.

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
	TEST_COMPARE(rel_err,<, max_tol);

	// Convergence test: check that second continuous variable
	// has reached optimal value within given tolerance
	target = 1.7659069377;
	max_tol = 1.e-2;
	rel_err = fabs((vars.continuous_variable(1) - target)/target);
	TEST_COMPARE(rel_err,<, max_tol);

	// Convergence test: check that the final response value
	// has reached the corresponding minimum within given tolerance
	target = -2.4614299775;
	max_tol = 1.e-3;
	rel_err = fabs((resp.function_value(0) - target)/target);
	TEST_COMPARE(rel_err,<, max_tol);

Teuchos unit test macros
------------------------

The following is a list of Teuchos unit test macros (see `Teuchos Docs
<https://trilinos.org/docs/dev/packages/teuchos/doc/html/group__Teuchos__UnitTestAssertMacros__grp.html#ga0794ff7d415d63cb6a741ecc7829d544>`_
for more):

.. code-block::

	ECHO(statement)                           // Echo the given statement before it is executed.
	TEST_ASSERT(v1)                           // Assert the given statement is true.
	TEST_EQUALITY_CONST(v1, v2)               // Assert the equality of v1 and constant v2.
	TEST_EQUALITY(v1, v2)                     // Assert the equality of v1 and v2.
	TEST_INEQUALITY_CONST(v1, v2)             // Assert the inequality of v1 and constant v2.
	TEST_INEQUALITY(v1, v2)                   // Assert the inequality of v1 and v2.
	TEST_FLOATING_EQUALITY(v1, v2, tol)       // Assert the relative floating-point equality of rel_error(v1,v2) <= tol.
	TEST_ITER_EQUALITY(iter1, iter2)          // Assert that two iterators are equal.
	TEST_ITER_INEQUALITY(iter1, iter2)        // Assert that two iterators are NOT equal.
	TEST_ARRAY_ELE_EQUALITY(a, i, val)        // Assert that a[i] == val.
	TEST_ARRAY_ELE_INEQUALITY(a, i, val)      // Assert that a[i] != val.
	TEST_COMPARE(v1, comp, v2)                // Assert that v1 comp v2 (where comp = '==', '>=", "!=", etc).
	TEST_COMPARE_ARRAYS(a1, a2)               // Assert that a1.size()==a2.size() and a[i]==b[i], i=0....
	TEST_COMPARE_FLOATING_ARRAYS(a1, a2, tol) // Assert that a1.size()==a2.size() and rel_error(a[i],b[i]) <= tol, i=0....
	TEST_THROW(code, ExceptType)              // Assert that the statement 'code' throws the exception 'ExceptType'
											  // (otherwise the test fails).
	TEST_NOTHROW(code)                        // Assert that the statement 'code' does not thrown any excpetions.

Compiling with existing unit tests
----------------------------------

The source code can be compiled and run with the existing ensemble of
unit tests by modifying the CMakeLists.txt file in the
dakota/src/unit_test directory to include the source file containing
the new unit tests. A line with the name of the source file containing
the new unit tests is added to the set(dakota_teuchos_unit_tests ...)
environment. Furthermore, Dakota must be configured and compiled with
the DAKOTA_ENABLE_TEUCHOS_UNIT_TESTS macro enabled. Finally, the tests
can be executed with the command

.. code-block::

	ctest -R unit

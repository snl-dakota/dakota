""""""""
Demo TPL
""""""""

This is a simple Demo which serves as a working example for bringing a new Third-Party Library (TPL) into Dakota. The Demo will serve to show minimal requirements for:

- building and running the Demo
- building a TPL under Dakota using CMake
- exposing TPL functionality to Dakota
- exposing TPL options through Dakota
- transferring data between a TPL and Dakota

Following this Demo, a developer should be able to integrate an optimization TPL/method that:

- is derivative-free
- operates over continuous variables
- supports any of the following types of constraints
  - bound constraints
  - nonlinear inequality constraints
  - nonlinear equality constraints

=========================================
Quickstart: Building and Running the Demo
=========================================

In order to build and run this Demo, it is necessary to build Dakota from source. Complete instructions for doing so can be found at https://dakota.sandia.gov/content/build-compile-source-code. At the point in the instructions where cmake is invoked, append -DHAVE_DEMO_TPL:BOOL=ON to the cmake invocation.

Building Dakota with the Demo TPL enabled will also activate a working example found in $DAKOTA_BUILD/test/dakota_demo_app, where $DAKOTA_BUILD is the root of the Dakota build tree. The test can be run from $DAKOTA_BUILD/test using

.. code-block::

   `ctest -R demo_app`

Summary info will be output to the screen, and test artifacts can be found in $DAKOTA_BUILD/test/dakota_demo_app.

Alternatively the example can be run in the same way a user runs Dakota. In particular, from the $DAKOTA_BUILD/test/dakota_demo_app directory, issue the following command:

.. code-block::

   `/path/to/dakota -i dakota_demo_app.in`

The remainder of this file describes how to integrate a TPL into Dakota using the Demo (found in $DAKOTA_SRC/packages/external/demo_tpl) as an example.

=======================================
Building a TPL under Dakota using CMake
=======================================

This section shows how to include the relevant parts of the Demo TPL as a library that Dakota builds and includes as part of its own native Cmake build.

Assuming the Demo tpl source code has been placed alongside other Dakota TPLs in $DAKOTA_SRC/packages/external/demo_tpl, a simple CMakeLists.txt file can be created at this location to allow Dakota to include it within its own Cmake setup. An minimal example might include:

.. code-block::

    # File $DAKOTA_SRC/packages/external/demo_tpl/CMakeLists.txt
    cmake_minimum_required(VERSION 2.8)
    project("DEMO_TPL" CXX)
    SUBDIRS(src)

In the src subdirectory of demo_tpl would be another CMakeLists.txt file which essentially identifies the relevant source code to be compiled into a library along with defining the library which Daktoa will later include, e.g.

.. code-block::

    # File $DAKOTA_SRC/packages/external/demo_tpl/src/CMakeLists.txt
    set(demo_tpl_HEADERS
        demo_opt.hpp
       )
    set(demo_tpl_SOURCES
        demo_opt.cpp
       )
    # Set the DEMO_TPL library name.
    add_library(demo_tpl ${demo_tpl_SOURCES})
    # Define install targets for "make install"
    install(TARGETS demo_tpl EXPORT ${ExportTarget} DESTINATION lib)

Note that it is possible to use Cmake's glob feature to bring in all source and header files, but care must be taken to avoid introducing main(...) symbols which will collide with Dakota's main at link time.

At this point, Dakota's CMakeLists.txt files will need to be modified to include the Demo TPL. The following modifications can be used to bring in the Demo TPL, conditioned on having -DHAVE_DEMO_TPL:BOOL=ON defined when invoking cmake to configure Dakota:

.. code-block::

    # File $DAKOTA_SRC/packages/CMakeLists.txt
    <... snip ...>
      option(HAVE_DEMO_TPL "Build the Demo_TPL package." OFF)
    <... end snip ...>
    <... snip ...>
      if(HAVE_DEMO_TPL)
        add_subdirectory(external/demo_tpl)
      endif(HAVE_DEMO_TPL)
    <... end snip ...>

This next modification to Dakota will allow the Demo TPL to be used by other Dakota source code by including the necessary include paths, link-time libraries and needed #defines:

.. code-block::

    # File $DAKOTA_SRC/src/CMakeLists.txt
    <... snip ...>
    if(HAVE_DEMO_TPL)
      set(DAKOTA_DEMOTPL_ROOT_DIR "${Dakota_SOURCE_DIR}/packages/external/demo_tpl")
      list(APPEND DAKOTA_INCDIRS 
          ${DAKOTA_DEMOTPL_ROOT_DIR}/dakota_src
          ${DAKOTA_DEMOTPL_ROOT_DIR}/src)
    set(iterator_src ${iterator_src} ${Dakota_SOURCE_DIR}/packages/external/demo_tpl/dakota_src/DemoOptimizer.cpp)
      list(APPEND DAKOTA_PKG_LIBS demo_tpl)
    list(APPEND EXPORT_TARGETS demo_tpl)
      add_definitions("-DHAVE_DEMO_TPL")
    endif(HAVE_DEMO_TPL)
    <... end snip ...>

============================
Test-Driven Code Development
============================

Before making concrete changes, it is often helpful to create a simple Dakota test which will serve to guide the process. This is akin to test-driven development which essentially creates a test which fails until everything has been implemented to allow it to run and pass. A candidate test for the current activity could be the following:

.. code-block::

    # File $DAKOTA_SRC/test/dakota_demo_app.in
        method,
            demo_tpl
            options_file = "demo_tpl.opts"
        variables,
            continuous_design = 3
            initial_point      -1.0    1.5   2.0
            upper_bounds       10.0   10.0  10.0
            lower_bounds       -10.0  -10.0 -10.0
            descriptors         'x1'  'x2'  'x3'
        interface,
            direct
            analysis_driver = 'text_book'
        responses,
            objective_functions = 1
            no_gradients
            no_hessians

For this test to run, we will need to be able to pass parsed options to the Demo TPL and exchange parameters and response values between Dakota and Demo TPL. These details are presented in the following sections.

====================================
Exposing TPL Functionality to Dakota
====================================

Dakota performs some internal checks in order to confirm applicability of a specified method to the problem defined. In order for Dakota to perform those checks for the Demo TPL, the functionality of the method must be communicated to Dakota. That is done via implementation of a traits class. Traits define the types of problems and data formats the Demo TPL supports by overriding the default traits accessors in TraitsBase. By default, nothing is supported, and the TPL integrator must explicitly turn on the traits for any supported features.

.. code-block::

    // File $DAKOTA_SRC/packages/external/demo_tpl/dakota_src/DemoOptimizer.hpp
    class DemoOptTraits: public TraitsBase
    {
    public:
      <... snip ...>
      //
      //- Heading: Constructor and destructor
      //
      DemoOptTraits() { }
      virtual ~DemoOptTraits() { }
      <... end snip ...>
      <... snip ...>
      //
      //- Heading: Virtual member function redefinitions
      //
      bool supports_continuous_variables() override
        { return true; }
      <... end snip ...>
    }; // class DemoOptTraits

A complete list of traits can be found in $DAKOTA_SRC/src/DakotaTraitsBase.hpp. The subset applicable to the Demo TPL can be found in $DAKOTA_SRC/packages/external/demo_tpl/dakota_src/DemoOptimizer.hpp.

===================================
Exposing TPL Options Through Dakota
===================================

The simplest way to pass options to a TPL is via a file. The Demo TPL has the ability to read in a file of method options when given a file name. This file name can be specified in the Dakota input file and retrieved as illustrated below.

.. code-block::

    // File $DAKOTA_SRC/packages/external/demo_tpl/dakota_src/DemoOptimizer.cpp
      <... snip ...>
      // Check for native Demo_Opt input file.  The file name needs to be
      // included in the Dakota input file.
      String adv_opts_file = probDescDB.get_string("method.advanced_options_file");
      if (!adv_opts_file.empty())
      {
        if (!boost::filesystem::exists(adv_opts_file))
        {
          Cerr << "\nError: Demo_Opt options_file '" << adv_opts_file
               << "' specified, but file not found.\n";
          abort_handler(METHOD_ERROR);
        }
      }
      <... end snip ...>

If desired, common stopping criteria can be retrieved from the Dakota input file, rather than passed through a TPL-specific input file, as follows.

.. code-block::

    // File $DAKOTA_SRC/packages/external/demo_tpl/dakota_src/DemoOptimizer.cpp
      <... snip ...>
      get_common_stopping_criteria(max_fn_evals, max_iters, conv_tol,
      min_var_chg, obj_target );
      <... end snip ...>
      
==================================
Exchanging Parameters and Reponses
==================================

Like any TPL, the Demo TPL will need to exchange parameter and obective function values with Dakota. For purposes of demonstration, an example interface between Dakota and the Demo TPL can be seen in $DAKOTA_SRC/packages/external/demo_tpl/dakota_src/DemoOptimizer.hpp (with corresponding .cpp in the same directory). Within these files is a key callback interface used by the Demo TPL to obtain objective function values for given parameter values (3 in the test above), eg:

.. code-block:: cpp

    // File $DAKOTA_SRC/packages/external/demo_tpl/dakota_src/DemoOptimizer.cpp
    Real
    DemoTPLOptimizer::compute_obj(const std::vector<double> & x, bool verbose)
    {
      // Tell Dakota what variable values to use for the function
      // valuation.  x must be (converted to) a std::vector<double> to use
      // this demo with minimal changes.
      set_variables<>(x, iteratedModel, iteratedModel.current_variables());
      // Evaluate the function at the specified x.
      iteratedModel.evaluate();
      // Retrieve the the function value and sign it appropriately based
      // on whether minimize or maximize has been specified in the Dakota
      // input file.
      double f = dataTransferHandler->get_response_value_from_dakota(iteratedModel.current_response());
      return f;
    }

In this instance, the Demo TPL uses std::vector<double> as its native parameter vector data type and is calling back to the example problem (Dakota model) via an interface to Dakota to obtain a single double (aliased to Real in Dakota) obective function value for a given set of parameter values. These data exchanges are facilitated by used of "data adapters" supplied by Dakota with the set_variables<>(...) utility and dataTransferHandler helper class utilized in this case.

For problems involving nonlinear equality and inequality constraints Dakota treats these as additional responses to the objective funtction(s). The Demo TPL supports both types for purposes of showing how these additional responses can be computed by Dakota (via interface to an underlying model) and transferred to the TPL. Similar to the call (by Demo) to compute_obj(...) are two additional methods to compute and transfer nonlinear constraint responses, eg:

.. code-block:: cpp

    // File $DAKOTA_SRC/packages/external/demo_tpl/dakota_src/DemoOptimizer.cpp
    void
    DemoTPLOptimizer::compute_nln_eq(std::vector<Real> &c, const std::vector<Real> &x, bool verbose)
    {
      // Tell Dakota what variable values to use for the nonlinear constraint
      // evaluations.  x must be (converted to) a std::vector<double> to use
      // this demo with minimal changes.
      set_variables<>(x, iteratedModel, iteratedModel.current_variables());
      // Evaluate the function at the specified x.
      iteratedModel.evaluate();
      // Use an adapter to copy data
      dataTransferHandler->get_nonlinear_eq_constraints_from_dakota(iteratedModel.current_response(), c);
    }
    void
    DemoTPLOptimizer::compute_nln_ineq(std::vector<Real> &c, const std::vector<Real> &x, bool verbose)
    {
      set_variables<>(x, iteratedModel, iteratedModel.current_variables());
      iteratedModel.evaluate();
      dataTransferHandler->get_nonlinear_ineq_constraints_from_dakota(iteratedModel.current_response(), c);
    }

Both of these callback methods (to Dakota), compute_nln_eq(...) and compute_nln_ineq(...) follow the same pattern as seen for the objective function callback: 1) set the Dakota model with the current variables (parameters), 2) evaluate the model and 3) transfer the desired response (objective or constraint) back to the TPL. The third step is facilitated by the appropriate call to the dataTransferHandler helper class. It should be noted that even though as many as three separate calls to evaluate the model are made for the same parameter values, Dakota maintains an internal cache of response values for each unique set. The model will be evaluated the first time a new set of parameter values is provided, but the cached values will simply be returned thereafter, thereby avoiding superfluous model evaluations.

Dakota must also provide initial parameter values to the Demo TPL and retrieve final objective function and variable values from the Demo TPL. The initial values for parameters and bound constraints can be obtained from Dakota with the get_variables<>(...) helpers. This example returns the values to a standard vector of doubles (Reals). These values can then be passed to the Demo TPL using whatever API is provided. The API for this last step varies with the particular TPL, and Demo provides a function set_problem_data in this case.

.. code-block:: cpp

    // File $DAKOTA_SRC/packages/external/demo_tpl/dakota_src/DemoOptimizer.cpp
    void DemoTPLOptimizer::initialize_variables_and_constraints()
    {
      // Get the number of variables, the initial values, and the values
      // of bound constraints.  They are returned to standard C++ data
      // types.  This example considers only continuous variables.  Other
      // types of variables and constraints will be added at a later time.
      // Note that double is aliased to Real in Dakota.
      int num_total_vars = numContinuousVars;
      std::vector<Real> init_point(num_total_vars);
      std::vector<Real> lower(num_total_vars),
                        upper(num_total_vars);
      // More on DemoOptTraits can be found in DemoOptimizer.hpp.
      get_variables(iteratedModel, init_point);
      get_variable_bounds_from_dakota<DemoOptTraits>( lower, upper );
      // Replace this line by whatever the TPL being integrated uses to
      // ingest variable values and bounds, including any data type
      // conversion needed.
      // ------------------  TPL_SPECIFIC  ------------------
      demoOpt->set_problem_data(init_point,   //  "Initial Guess"
                                lower     ,   //  "Lower Bounds"
                                upper      ); //  "Upper Bounds"
    }

The TPL should be able to return an optimal objective function value and the corresponding variable (parameter) values via its API. As has been the case throughout, the data should be doubles (aliased to Real in Dakota). The following code takes the values returned by Demo via a call to get_best_f() and sets the Dakota data structures that contain final objective and variable values. It adjusts the sign of the objective based on whether minimize or maximize has been specified in the Dakota input file (minimize is the default). If the problem being optimized involves nonlinear equality and/or inequality constraints, these will also need to be obtained from the TPL and passed to Dakota as part of the array of best function values (responses).

.. code-block:: cpp

    // File $DAKOTA_SRC/packages/external/demo_tpl/dakota_src/DemoOptimizer.cpp
    // in method void DemoTPLOptimizer::core_run()
      // Replace this line with however the TPL being incorporated returns
      // the optimal function value.  To use this demo with minimal
      // changes, the returned value needs to be (converted to) a
      // double.
      double best_f = demoOpt->get_best_f(); // TPL_SPECIFIC
      // If the TPL defaults to doing minimization, no need to do
      // anything with this code.  It manages needed sign changes
      // depending on whether minimize or maximize has been specified in
      // the Dakota input file.
      const BoolDeque& max_sense = iteratedModel.primary_response_fn_sense();
      RealVector best_fns(iteratedModel.response_size()); // includes nonlinear contraints
      // Get best (single) objcetive value respecting max/min expectations
      best_fns[0] = (!max_sense.empty() && max_sense[0]) ?  -best_f : best_f;
      // Get best Nonlinear Equality Constraints from TPL
      if( numNonlinearEqConstraints > 0 )
      {
        auto best_nln_eqs = demoOpt->get_best_nln_eqs(); // TPL_SPECIFIC
        dataTransferHandler->get_best_nonlinear_eq_constraints_from_tpl(
                                            best_nln_eqs,
                                            best_fns);
      }
      // Get best Nonlinear Inequality Constraints from TPL
      if( numNonlinearIneqConstraints > 0 )
      {
        auto best_nln_ineqs = demoOpt->get_best_nln_ineqs(); // TPL_SPECIFIC
        dataTransferHandler->get_best_nonlinear_ineq_constraints_from_tpl(
                                            best_nln_ineqs,
                                            best_fns);
      }
      bestResponseArray.front().function_values(best_fns);
      std::vector<double> best_x = demoOpt->get_best_x(); // TPL_SPECIFIC
      // Set Dakota optimal value data.
      set_variables<>(best_x, iteratedModel, bestVariablesArray.front());

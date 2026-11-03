.. _library-api-tutorial:

Sampling Study Tutorial
=======================

This example builds a Latin hypercube sampling study with two uniform
uncertain variables and the Dakota ``text_book`` analysis driver.  The Python
and C++ versions assemble the same study graph.

Python
------

The Python version uses configuration keyword arguments.  This is usually the
most readable form when translating a small freeform input.

.. code-block:: python

   from dakota.study import Study, StudyConfig

   config = StudyConfig()
   config.output.precision = 12
   config.output.output_file = "di_construction_demo.out"
   config.output.error_file = "di_construction_demo.err"
   config.output.write_restart = "di_construction_demo.rst"
   config.output.results_output = True
   config.output.results_output_file = "di_construction_demo_results"

   study = Study(config)

   variables = study.variables(
       active={"all": True},
       uniform_uncertain={
           "count": 2,
           "descriptors": ["x1", "x2"],
           "lower_bounds": [0.0, 0.0],
           "upper_bounds": [1.0, 1.0],
       },
   )

   response = study.responses(
       variables,
       response_type={"response_functions": {"count": 1}},
       descriptors=["f"],
       gradient_type={"no_gradients": True},
       hessian_type={"no_hessians": True},
   )

   interface = study.interface(
       analysis_drivers={
           "drivers": ["text_book"],
           "interface_type": {
               "fork": {
                   "parameters_file": "params.in",
                   "results_file": "results.out",
                   "file_save": True,
               }
           },
       },
       deactivate={"restart_file": True},
   )

   model = study.model.simulation(variables, interface, response)

   sampling = study.method.sampling(
       model,
       sample_type={"lhs": True},
       samples=10,
       seed=1234,
   )

   study.run(sampling)

The call to ``sampling`` passes its top-level configuration fields as keyword
arguments. Dictionaries are still used for nested keyword groups: for example,
``sample_type={"lhs": True}`` represents the freeform ``sample_type lhs``
choice. The keyword-argument and Pydantic forms for that call are:

.. code-block:: python

   from dakota.spec.method import SamplingConfig

   sampling = study.method.sampling(
       model,
       sample_type={"lhs": True},
       samples=10,
       seed=1234,
   )

   sampling = study.method.sampling(
       model,
       config=SamplingConfig(
           sample_type={"lhs": True},
           samples=10,
           seed=1234,
       ),
   )

Python with inline callback
---------------------------

The same study can be driven by a Python function defined directly in the
script, with no external executable needed.  Decorate the function with
:func:`python_interface` and pass it as the ``callback`` keyword argument to
:meth:`Study.interface`.  Because the callback supplies the driver, no
``analysis_drivers`` configuration is required.

.. code-block:: python
   :emphasize-lines: 4-10, 40

   import dakota.interfacing as di
   from dakota.study import Study, StudyConfig

   @di.python_interface
   def text_book(params, results):
       x1, x2 = params["x1"], params["x2"]
       response = results["f"]
       if response.asv.function:
           response.function = (x1 - 1.0) ** 4 + (x2 - 1.0) ** 4
       return results

   config = StudyConfig()
   config.output.precision = 12
   config.output.output_file = "di_construction_demo_python_callback.out"
   config.output.error_file = "di_construction_demo_python_callback.err"
   config.output.write_restart = "di_construction_demo_python_callback.rst"
   config.output.results_output = True
   config.output.results_output_file = "di_construction_demo_python_callback_results"

   study = Study(config)

   variables = study.variables(
       active={"all": True},
       uniform_uncertain={
           "count": 2,
           "descriptors": ["x1", "x2"],
           "lower_bounds": [0.0, 0.0],
           "upper_bounds": [1.0, 1.0],
       },
   )

   response = study.responses(
       variables,
       response_type={"response_functions": {"count": 1}},
       descriptors=["f"],
       gradient_type={"no_gradients": True},
       hessian_type={"no_hessians": True},
   )

   interface = study.interface(callback=text_book)

   model = study.model.simulation(variables, interface, response)

   sampling = study.method.sampling(
       model,
       sample_type={"lhs": True},
       samples=10,
       seed=1234,
   )

   study.run(sampling)

The :func:`python_interface` decorator adapts the function to the dictionary
protocol that :class:`~dakota.study.Interface` expects.  Inside the decorated
function, ``params`` is a :class:`Parameters` object that provides named
variable access (``params["x1"]``), and ``results`` is a :class:`Results`
object that holds one :class:`Response` entry per response descriptor.  Each
response carries an active-set vector (``response.asv.function``,
``response.asv.gradient``, ``response.asv.hessian``) that the function should
respect before assigning values.

When a callback is provided, interface configuration is not required — Dakota
uses built-in defaults.  A configuration object or keyword arguments can still
be supplied alongside the callback, for example to enable
:ref:`batch evaluation <interfaces:batch>`:

.. code-block:: python

   interface = study.interface(
       concurrency={"batch": {}},
       callback=text_book,
   )

C++
---

The C++ API accepts JSON configuration fragments and typed study-level output
configuration.  Dependencies are passed as C++ objects and smart pointers.

.. literalinclude:: ../../../../src/di_construction_demo.cpp
   :language: cpp
   :start-after: [docs-library-api-start]
   :end-before: [docs-library-api-end]

Both examples keep the ``Study`` alive through execution.  The study owns the
runtime services shared by the variables, interface, model, and iterator.

Running the examples
--------------------

The examples require a Dakota build containing the Study API and the
``text_book`` analysis driver.  Run them from a writable directory because the
configured interface and output settings create files there.  For Python, put
the build-tree or installed Dakota Python package on ``PYTHONPATH``.

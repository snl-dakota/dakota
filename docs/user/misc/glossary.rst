.. _glossary-main:

""""""""
Glossary
""""""""

This page defines terms used throughout the Dakota documentation. Each entry
links to the page that covers the topic in depth.

.. glossary::
   :sorted:

   aleatory uncertain variables
      :term:`Uncertain variables <uncertain variables>` that represent
      inherent randomness, which more information can't reduce. They are
      described by probability distributions, such as normal or uniform.
      See :ref:`variables:uncertain:auv`.

   analysis driver
      The program Dakota runs for each :term:`evaluation` to map parameters
      to responses. It can be a script, such as Python or bash, or a
      workflow built in the Dakota GUI. Set with
      :dakkw:`interface-analysis_drivers`. See
      :ref:`couplingtosimulations-main` and :ref:`ngw-analysisdrivers`.

   black box
   black-box simulation
      A simulation that Dakota treats only as a mapping from
      :term:`variables` to :term:`responses`. Dakota needs nothing else about
      the code, however the data is exchanged. See
      :ref:`couplingtosimulations-main`.

   dakota_parser
      A command-line tool, in the ``bin`` folder of a Dakota installation,
      that converts a :term:`freeform format` input file to the
      :term:`JSON format`: ``dakota_parser <file> --json``. See
      :ref:`jsoninput`.

   design variables
      :term:`Variables <variables>` you control, such as dimensions or
      operating settings, which optimization and parameter studies vary. See
      :ref:`variables:design`.

   direct interface
      An :term:`interface` in which Dakota calls the simulation inside its
      own process, with no :term:`parameters file` or :term:`results file`.
      It covers Dakota's built-in test functions, such as ``rosenbrock``, and
      codes compiled into Dakota. See
      :dakkw:`interface-analysis_drivers-direct` and
      :ref:`interfaces:direct`.

   dprepro
      A template processor included with Dakota. It fills a
      :term:`template file` with the variable values from a
      :term:`parameters file`, either from the command line or through the
      ``dakota.interfacing`` Python module. See
      :ref:`interfaces:dprepro-and-pyprepro`.

   ensemble
      A set of runs of the same simulation with different parameter values.
      Dakota chooses the values, manages the runs, and analyzes the
      responses. See :ref:`aboutdakota-main`.

   environment
      The input-file block for settings that apply to the whole
      :term:`study`, such as :term:`tabular data file` output. It's optional,
      and a study has at most one. See :dakkw:`environment`.

   epistemic uncertain variables
      :term:`Uncertain variables <uncertain variables>` that represent lack
      of knowledge, which more information could reduce. They are often
      described by intervals rather than probability distributions. See
      :ref:`variables:uncertain:euv`.

   evaluation
      One mapping of a set of :term:`variables` values to
      :term:`responses`, for example one run of the :term:`analysis driver`.
      A :term:`method` requests many evaluations during a study. See
      :ref:`interfaces:main`.

   evaluation concurrency
      The (maximum) number of :term:`evaluations <evaluation>` Dakota runs at the same
      time. Set with :dakkw:`interface-asynchronous-evaluation_concurrency`
      under :dakkw:`interface-asynchronous`. See :ref:`parallel`.

   fork interface
   system interface
      :term:`Interfaces <interface>` that run an :term:`analysis driver` as a
      separate process and exchange data through a :term:`parameters file`
      and a :term:`results file`. Use ``fork``; ``system`` is not
      recommended. See :dakkw:`interface-analysis_drivers-fork` and
      :ref:`interfaces:sim`.

   freeform format
      One of the two supported syntaxes for Dakota input files, run with
      ``dakota -i file.in``. It describes the same study structure as the
      :term:`JSON format`. See :ref:`inputfile_formatting_main`.

   interface
      The input-file block that tells Dakota how to perform an
      :term:`evaluation`, for example which :term:`analysis driver` to run
      and how to exchange data with it. A study needs at least one. See
      :dakkw:`interface` and :ref:`interfaces:main`.

   JSON format
      One of the two supported syntaxes for Dakota input files, run with
      ``dakota -json file.json``. It describes the same study structure as
      the :term:`freeform format` as nested JSON objects, which scripts and
      tools can easily generate and validate. See :ref:`jsoninput`.

   library API
      Python and C++ programming interfaces for building and running Dakota
      studies inside another application, instead of running Dakota from the
      command line. See :ref:`library-api`.

   method
      The input-file block that selects the algorithm, such as sampling,
      optimization, or calibration, and its settings. A study needs at least
      one. See :dakkw:`method` and :ref:`studytypes-main`.

   model
      The input-file block that connects a :term:`method` to
      :term:`variables`, an :term:`interface`, and :term:`responses`. It's
      optional in simple studies; other model types build
      :term:`surrogates <surrogate>` or nest studies. See :dakkw:`model` and
      :ref:`models:main`.

   outer-loop analysis
      An analysis that runs a simulation many times, with Dakota choosing the
      inputs for each run and analyzing the results, such as optimization,
      uncertainty quantification, sensitivity analysis or calibration. See
      :ref:`aboutdakota-main`.

   parameters file
      The file Dakota writes for each :term:`evaluation` through a
      :term:`fork interface`. It holds the current :term:`variables` values
      and which response data the method needs. See
      :ref:`couplingtosimulations-paramformat`.

   post-processing
      The step of an :term:`analysis driver` that runs after your code. It
      extracts the :term:`responses` from your code's output and writes them
      to the :term:`results file`.

   pre-processing
      The step of an :term:`analysis driver` that runs before your code. It
      puts the variable values from the :term:`parameters file` into your
      code's input, usually with a :term:`template file` and :term:`dprepro`.

   pyprepro
      A template processor included with Dakota. It fills in a
      :term:`template file` with values you give it, and templates can use
      Python features such as loops and conditionals. :term:`dprepro` uses the
      same template engine, and also reads Dakota's :term:`parameters file`.
      See :ref:`interfaces:dprepro-and-pyprepro`.

   Python direct interface
      An :term:`interface` in which Dakota calls a function in your Python
      module and passes data in memory, with no :term:`parameters file` or
      :term:`results file`. Set with :dakkw:`interface-analysis_drivers-python`;
      available only in Dakota builds that include it. See
      :ref:`advint:existingdirect:python`.

   responses
   quantities of interest
   QoIs
      The outputs Dakota analyzes, such as objective functions, calibration
      terms, or general response functions, and optionally their
      derivatives. Also the input-file block that describes them; a study
      needs at least one. See :dakkw:`responses` and :ref:`responses:main`.

   restart file
      A binary file, ``dakota.rst`` by default, in which Dakota records each
      completed :term:`evaluation`, so an interrupted study can continue
      without repeating work. See :ref:`dakota_restart`.

   results file
      The file the :term:`analysis driver` writes for each
      :term:`evaluation`, containing the :term:`responses` Dakota requested.
      See :ref:`responses:results`.

   state variables
      :term:`Variables <variables>` for fixed model settings, such as solver
      tolerances or mesh resolution, that are neither designed nor treated as
      uncertain. See :ref:`variables:state`.

   study
      One complete Dakota analysis, described by an input file made of
      :term:`environment`, :term:`method`, :term:`model`, :term:`variables`,
      :term:`interface`, and :term:`responses` blocks. See
      :ref:`inputfile-main`.

   surrogate
      An inexpensive approximation of a costly simulation, such as a Gaussian
      process, built from evaluation data and used in place of the
      simulation. Defined in the :term:`model` block. See
      :ref:`models:surrogate`.

   tabular data file
      A text file listing the variables and responses of every
      :term:`evaluation`, one row each. Written when you specify
      :dakkw:`environment-tabular_data`. See :ref:`output:tabular`.

   template file
      A copy of your code's input file with placeholders, such as ``{x1}``,
      where variable values go. :term:`dprepro` fills them in for each
      :term:`evaluation`. See :ref:`interfaces:building`.

   uncertain variables
      :term:`Variables <variables>` whose values aren't known exactly, which
      uncertainty quantification methods propagate to the responses. They are
      either :term:`aleatory <aleatory uncertain variables>` or
      :term:`epistemic <epistemic uncertain variables>`. See
      :ref:`variables:uncertain`.

   variables
      The parameters Dakota varies, with their bounds or distributions. Also
      the input-file block that defines them; a study needs at least one.
      Variables are :term:`design <design variables>`,
      :term:`uncertain <uncertain variables>`, or
      :term:`state <state variables>`, and continuous or discrete. See
      :dakkw:`variables` and :ref:`variables:main`.

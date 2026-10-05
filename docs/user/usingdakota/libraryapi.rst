.. _library-api:

Library APIs
============

Dakota's Python and C++ APIs are an alternative way to construct and run
studies without writing a complete Dakota input file. They use the same
configuration grammar as JSON input, but replace pointer-based relationships
between Dakota blocks with references to constructed objects.

Motivation
----------

Traditionally, setting up a Dakota study has meant authoring a Dakota input
file, running Dakota on the command line, and then post-processing the output
files it produces.  While that workflow remains fully supported, the library
APIs open up a richer set of possibilities.

**User-supplied methods and models.** Although not yet supported in a released
version, the library APIs lay the groundwork for users to supply their own
method and model implementations and plug them directly into Dakota studies.
When that capability becomes available, researchers will no longer need to
rebuild Dakota or maintain a private fork to experiment with new algorithmic
ideas — they will be able to write a Python class or a C++ object, hand it to
Dakota, and run it through the full study infrastructure immediately.

**Flexibility and rapid prototyping.** Because study components are ordinary
programming objects, the full expressive power of Python or C++ is available
for constructing and parameterising them.  Studies can be built conditionally,
driven by data, or composed from reusable helper functions.  This flexibility
makes Dakota more accessible to domain scientists who prefer scripting over
configuration files, and significantly accelerates the prototyping cycle for
algorithm researchers who need to iterate quickly on new ideas.

**Integration into higher-level workflows.** The library APIs make it
straightforward to embed Dakota inside a larger application or automation
pipeline.  A Python orchestration script can construct a study, run it, and
inspect or act on the results all within a single process, without spawning
subprocesses or reading output files.  This lowers the friction of using
Dakota as a component in optimisation frameworks, uncertainty-quantification
pipelines, or digital-twin platforms.

**Agentic AI and automated studies.** Alongside :ref:`JSON input files <jsoninput>`, the Python API is particularly well-suited to agentic AI
workflows.  A language model or autonomous agent can construct a syntactically
correct Dakota study by calling well-typed Python constructors rather than
generating free-form text input, making it straightforward to validate studies
before they run and to iterate on them programmatically.  We anticipate that
the combination of JSON input and the Python API will become a primary path
for AI assistants and agentic systems to perform Dakota studies on behalf of
users.

**Looking ahead.** In the next couple of releases we plan to support plugging
in methods, models, and surrogates from third-party libraries directly into
Dakota studies, and to make the Python API available through PyPI.

The APIs are public and supported, but are still evolving.  The methods and
models available through a particular installation depend on the capabilities
enabled when Dakota was built.

.. toctree::
   :maxdepth: 1

   libraryapi/construction
   libraryapi/tutorial
   libraryapi/python-reference

The generated C++ reference is part of the Dakota Developers Manual.  The
guide and tutorial here describe the public construction model; the C++
reference documents the exact overloads available in a build.

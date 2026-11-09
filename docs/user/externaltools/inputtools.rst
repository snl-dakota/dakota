.. _input-tools:

""""""""""""""""""""""""
Working with Inputs
""""""""""""""""""""""""

Dakota accepts input in two formats: the traditional free-form DSL syntax
(``.in`` files) and JSON (``.json`` files).  Both formats are handled by
the same parser and share the same grammar.  This section describes two
tools — the ``dakota-to-json`` conversion utility and the ``dakota_parser``
standalone executable — that are useful for converting freeform inputs to
JSON and for validating and inspecting Dakota input files.

.. _input-tools-dakota-to-json:

The ``dakota-to-json`` Tool
============================

``dakota-to-json`` converts a freeform Dakota input file to a JSON file
that ``dakota -json`` accepts directly.  It removes the internal and
computed fields that ``dakota_parser`` inserts during validation and that
would cause re-validation to fail, producing output that is accepted by
``dakota -json``.

.. code-block:: shell

    dakota-to-json study.in study.json
    dakota-to-json study.in              # output defaults to study.json
    dakota-to-json --help

**Arguments**

``input``
    Path to the freeform Dakota input file.

``output`` *(optional)*
    Destination path for the JSON output.  Defaults to the input path with
    its extension replaced by ``.json``.

**Options**

``--dakota-parser EXE``
    Path to the ``dakota_parser`` executable.  Defaults to looking up
    ``dakota_parser`` on ``PATH``.

How it works
------------

The conversion runs two steps:

1. ``dakota_parser <input.in> --json`` validates the input and serializes
   it to the expanded internal JSON representation.
2. A stripping pass removes the internal and computed fields that
   ``dakota_parser`` inserts — specifically, inferred variable bounds,
   computed initial points, and internal ``_user_provided`` flags — that
   would cause re-validation to fail.

The output can be used with ``dakota -json``.  Note that it may still
contain some defaults that were expanded during validation and were not
present in the original freeform file.

Requirements
~~~~~~~~~~~~

``dakota_parser`` must be on ``PATH``, or its location must be supplied via
``--dakota-parser``.  Both executables are installed in Dakota's ``bin/``
directory.

Python API
----------

The same conversion is available as a Python function:

.. code-block:: python

    from dakota.tools.freeform_to_json import convert

    convert("study.in", "study.json")

    # Specify an explicit dakota_parser path
    convert("study.in", "study.json", dakota_parser="/opt/dakota/bin/dakota_parser")

.. py:function:: dakota.tools.freeform_to_json.convert(input_path, output_path, *, dakota_parser=None)

   Convert a freeform Dakota input file to clean JSON.

   :param input_path: Path to the freeform Dakota input file.
   :type input_path: str or pathlib.Path
   :param output_path: Destination path for the JSON output.
   :type output_path: str or pathlib.Path
   :param dakota_parser: Path to the ``dakota_parser`` executable.  If
       ``None``, ``dakota_parser`` is looked up on ``PATH``.
   :type dakota_parser: str, pathlib.Path, or None
   :raises FileNotFoundError: If ``dakota_parser`` is not found.
   :raises subprocess.CalledProcessError: If ``dakota_parser`` exits
       non-zero (e.g. the input file contains a syntax error).

.. _input-tools-dakota-parser:

The ``dakota_parser`` Executable
=================================

``dakota_parser`` is the standalone front end to Dakota's input parser.
Dakota uses it internally when reading input files, but it can also be run
directly to validate input and to inspect how a file is parsed.

.. note::

   The JSON output produced by ``dakota_parser --json`` is an internal
   validated representation: it contains computed fields and internal-only
   metadata that will cause validation to fail if fed back to the parser.
   It is not a JSON input file and is not intended for direct use with
   ``dakota -json``.  To convert a freeform input file to a JSON file
   suitable for ``dakota -json``, use :ref:`dakota-to-json
   <input-tools-dakota-to-json>`.

Usage
-----

.. code-block:: shell

    dakota_parser [options] <input_file>

Input mode is auto-detected from the file extension (``.json`` → JSON mode,
anything else → DSL/freeform mode) and can be overridden explicitly.

Input mode options
~~~~~~~~~~~~~~~~~~

``--json-input``
    Force JSON input mode regardless of the file extension.

``--dsl-input``
    Force DSL/freeform input mode even when the file has a ``.json``
    extension.

Output options
~~~~~~~~~~~~~~

``--json``
    Write the parsed and validated input to stdout as the internal JSON
    representation.  Defaults and inferred values are expanded into the
    output.  This flag is implied (and cannot be disabled) when the input
    is JSON.

    Without ``--json``, a freeform input is echoed back in canonical DSL
    form after parsing — useful for checking how the parser interpreted
    the file.

Validation options
~~~~~~~~~~~~~~~~~~

``--no-validate``
    Skip cross-field constraint validation (e.g. required-field checks and
    inter-keyword consistency rules).  Parsing and semantic analysis still
    run.  Useful for partial inputs during development.

``--no-semantic``
    Skip semantic analysis.  Parsing and constraint validation still run.

Diagnostic options
~~~~~~~~~~~~~~~~~~

``--analyze``
    Analyze the parser grammar for ambiguities or issues and print a
    report.  Exits non-zero if issues are found.  Intended for developers
    working on the grammar.

``--trace``
    Enable grammar-level tracing (prints each grammar rule as it is
    matched).  Only available in builds configured with
    ``-DDAKOTA_PARSER_TRACE=ON``.

``--debug``
    Print a step-by-step processing summary: input mode, parse result,
    validation, semantic analysis, default expansion, and a block/keyword
    count on completion.

``-h``, ``--help``
    Print usage and exit.

Processing pipeline
-------------------

For a freeform (``.in``) input file, ``dakota_parser`` runs the following
steps in order:

1. **Parse** — the freeform grammar tokenizes and builds an AST.
2. **Validate** — cross-field constraint rules are checked
   (skipped with ``--no-validate``).
3. **Semantic analysis** — higher-level consistency checks
   (skipped with ``--no-semantic``).
4. **Expand defaults** — schema-defined defaults are filled in.
5. **Output** — the AST is echoed as canonical DSL (default) or serialized
   to the internal JSON representation (``--json``).

For a JSON input file the same steps run, except that step 1 converts the
JSON to the internal AST representation and step 5 always produces JSON
output.

Examples
--------

Validate a freeform input file and echo it in canonical form:

.. code-block:: shell

    dakota_parser study.in

Validate an existing JSON input file:

.. code-block:: shell

    dakota_parser study.json

Validate but skip semantic analysis:

.. code-block:: shell

    dakota_parser study.in --no-semantic

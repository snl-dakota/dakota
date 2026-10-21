"""""""""""""""""""""""""""""""""""""
Modifying Dakota's Input Specification
"""""""""""""""""""""""""""""""""""""

Dakota's canonical input grammar is defined by Pydantic models under
``python/dakota/spec``. The parser under
``packages/dakota_parser/dakota_parser`` consumes JSON Schema generated from
those models and materializes input into the map-based intermediate
representation used by ``ProblemDescDB``.

Adding or changing a keyword
============================

#. Edit the appropriate model in ``python/dakota/spec``. Declare the field
   with ``DakotaField`` and specify its type, default, aliases, constraints,
   and materialization metadata.
#. Map the field to an IR key with ``ir_key``, ``storage_type``, and
   ``ir_value_type``. Reuse an existing field with equivalent behavior as a
   template.
#. Put each IR default in one place: the Pydantic field, an entry in
   ``src/default_overrides_registry.json``, or an entry in
   ``src/default_policy_registry.json``. Code generation rejects collisions.
#. Add cross-field validation or computed fields when required. Matching C++
   implementations and registrations live under
   ``packages/dakota_parser/dakota_validation``.
#. Add parser tests for accepted and rejected syntax and a
   ``ProblemDescDB`` or materializer test that reads the resulting IR value.
   Update keyword documentation and example input files.

Regenerating parser and IR sources
==================================

Configure with ``DAKOTA_PYTHON=ON`` and ``DAKOTA_GENERATE_JSON_SCHEMA=ON``.
From the build directory, regenerate and inspect the schema, parser, and IR
artifacts:

.. code-block:: bash

   cmake --build . --target dakota_json_schema
   cmake --build . --target regenerate_parser
   cmake --build . --target dakota_ir_generated_tables

Do not edit generated parser or IR tables directly. Review the generated JSON
Schema for the expected ``x-materialization``, validation, computed-field, and
default metadata before running the parser, materializer, and input regression
tests.

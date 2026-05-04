JSON Input Reference
====================

.. toctree::
   :maxdepth: 1


This release introduces JSON as an alternative input format for Dakota.
JSON input provides a foundation for more structured, tool-friendly
workflows, including tighter integration with Python-based configuration
and validation. However, The JSON interface is not yet fully documented or
polished, and users should expect some rough edges in this initial release.

JSON input is enabled by `Pydantic`_ models, which have replaced dakota.xml
as the authoratative statement of Dakota grammar. Ordinary users likely will
find the Pydantic models themselves to be of limited use for this relesae. 

.. _Pydantic: https://pydantic.dev/docs/validation/latest/get-started/

Current Documentation Status
----------------------------

For this release, the primary reference for JSON input is automatically
generated documentation of the Pydantic models that define the schema.
These models describe the valid structure, field names, and types accepted
by Dakota's JSON parser.

This approach has an important limitation: the documentation is organized
around model classes, not the familiar Dakota keyword hierarchy.
As a result, it can feel indirect compared to the traditional
:ref:`keyword reference <keyword-reference-area>`.
In particular, users will often need to navigate between models to
understand how nested structures correspond to Dakota concepts.

A more integrated, Dakota-style JSON reference—aligned with the existing
keyword documentation—is planned for the next release.

Relationship to the Traditional Keyword Format
----------------------------------------------

A key design goal for Dakota's JSON format was to mimic the traditional
freeform as closely as possible. However, the mapping is not one-to-one.
Two structural differences are especially important:

"Anchor" keywords for oneOf groups
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In places where the freeform syntax allows mutually exclusive keyword
groups, the JSON representation introduces explicit intermediate objects
(anchors) to represent those choices. These do not appear in the traditional
input format but are required in JSON to make the structure unambiguous.

"Argument" keywords
~~~~~~~~~~~~~~~~~~~

In the freeform format, a keyword may both take a literal argument and
contain child keywords. JSON does not support this pattern directly, so
the literal argument is represented as a separate child field (an "argument"
keyword). As a result, some values that appear inline in the traditional format
become nested fields in JSON.

Because of these differences, users should not expect a mechanical translation
between the two formats without some structural adjustments.

How to Work with the Pydantic Documentation
-------------------------------------------

When using the generated Pydantic documentation:

Think of each model as representing a portion of the Dakota input hierarchy.
Field names within a model correspond to JSON keys.

Nested models represent nested blocks of the input structure.
Required vs optional fields, default values, and allowed types are all defined there.

In practice, constructing a JSON input often involves starting from a top-level
Pydantic model and following field types downward to understand what structures
are expected at each level. This is less direct than the keyword reference, but it
reflects the underlying validation logic used by Pydantic.

Using `dakota_parser`` to Bootstrap JSON Input
----------------------------------------------

To ease the transition, Dakota provides a tool called `dakota_parser` that can
convert an existing freeform input file into JSON.

This can be a practical way to get started:

1. Begin with a working traditional input file.
2. Run `dakota_parser`, located in the `bin` folder of the Dakota install, to 
   generate the corresponding JSON.
3. Use the result as a starting point, modifying it as needed.
4. Refer to the Pydantic documentation to understand and refine specific fields.

This workflow avoids having to construct JSON inputs from scratch while the
documentation is still evolving.

Pydantic Model Documentation
----------------------------


   pydantic_models_reference/base
   pydantic_models_reference/environment
   pydantic_models_reference/interface
   pydantic_models_reference/model
   pydantic_models_reference/responses
   pydantic_models_reference/study
   pydantic_models_reference/variables


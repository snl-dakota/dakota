JSON Input Reference
====================

This release introduces JSON as an alternative input format for Dakota.
JSON input provides a foundation for more structured, tool-friendly
workflows, including tighter integration with Python-based configuration
and validation. However, The JSON interface is not yet fully documented or
polished, and users should expect some rough edges in this initial release.

JSON input is enabled by `Pydantic`_ models, which have replaced dakota.xml
as the authoratative statement of Dakota grammar. Ordinary users likely will
find the Pydantic models themselves to be of limited use for this relesae.

.. _Pydantic: https://pydantic.dev/docs/validation/latest/get-started/

Current Documentation
---------------------

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

The Pydantic model documentation is arranged by module, which
corresponds roughly to top-level Dakota blocks (environment, method,
etc). The `DakotaStudy` model in the Study section defines the overall
organization of the input.

Before consulting the Pydantic model reference, readers are advised
to read the remainder of this page. A JSON reference more integrated 
with the existing keyowrd documenation is planned for the next release.
 
.. toctree::
   :maxdepth: 1

   pydantic_models_reference/study
   pydantic_models_reference/methods
   pydantic_models_reference/environment
   pydantic_models_reference/interface
   pydantic_models_reference/model
   pydantic_models_reference/responses
   pydantic_models_reference/variables

Relationship to the Traditional Keyword Format
----------------------------------------------

A key design goal for Dakota's JSON format was to mimic the traditional
freeform as closely as possible. However, the mapping is not one-to-one.
Several differences are especially important and will be illustrated
using the following example.

.. code-block:: json
    :linenos:

    {
      "environment": {
        "tabular_data": {}
      },
      "method": [
        {
          "optpp_g_newton": {
            "convergence_tolerance": 0.0001,
            "max_iterations": 100
          }
        }
      ],
      "model": [
        {
          "single": {}
        }
      ],
      "interface": [
        {
          "analysis_drivers": {
            "drivers": [
              "rosenbrock"
            ],
            "interface_type": {
              "direct": {}
            }
          }
        }
      ],
      "responses": [
        {
          "gradient_type": {
            "analytic_gradients": true
          },
          "hessian_type": {
            "no_hessians": true
          },
          "response_type": {
            "calibration_terms": {
              "count": 2
            }
          }
        }
      ],
      "variables": [
        {
          "continuous_design": {
            "count": 2,
            "descriptors": [
              "x1",
              "x2"
            ],
            "initial_point": [
              -1.2,
              1.0
            ],
            "lower_bounds": [
              -2.0,
              -2.0
            ],
            "upper_bounds": [
              2.0,
              2.0
            ]
          }
        }
      ]
    }

Top-level Organization
~~~~~~~~~~~~~~~~~~~~~~

A JSON format input file must contain a single object. As specified in the
:ref:`DakotaStudy <pydantic-study>` model, this object has:

* an optional ``environment`` key (line 2), whose value is another object that
  contains configuration information for the environment block.
* a required ``method`` key (line 5), whose value is an array of length 1 or more of 
  method configuration objects.
* an optional ``model`` key (line 13), whose value is an array of length 0
  or more of model configuration objects.
* required ``interface``,``responses``, and ``interface``  keys (lines 18, 30, 45), 
  whose values are arrays of length 1 or more of configuration objects for those
  blocks

The structure of the overall study is specified by the `DakotaStudy` model.

Leaf Keywords
~~~~~~~~~~~~~

Keywords that have no children ("leaf" keywords) receive
a constant ``true`` argument. In the exapmle input in the 
preceeding section, they keys ``analytic_gradients`` (line 33) and
``no_hessians`` (line 35) are instances of this.

Empty Objects
~~~~~~~~~~~~~

Empty objects are used when a keyword can have optional
child keywords, none of which happen to be present in a
particular study. The ``tabular_data`` (line 3) and ``single`` (line 15)
keys are examples.

"Anchor" keywords for groups
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In places where the freeform syntax allows mutually exclusive keyword
groups, the JSON representation introduces explicit intermediate objects
(anchors) to represent those choices. These do not appear in the traditional
input format but are required in JSON to make the structure unambiguous. In
the example, ``gradient_type`` (line 32) and ``hessian_type`` (line 35)
are examples of anchor keywords.

"Argument" keywords
~~~~~~~~~~~~~~~~~~~

In the freeform format, a keyword may both take a literal argument and
contain child keywords. JSON does not support this pattern directly, so
the literal argument is represented as a separate child field (an "argument"
keyword). As a result, some values that appear inline in the traditional format
become nested fields in JSON.

An example of an argument keyword is ``count`` (line 40), which denotes the number of
continuous design variables.

Nonfinite numbers
~~~~~~~~~~~~~~~~~

Nonfinite numbers such as `inf` and `nan` are not natively supported
by JSON. They must be provided as quoted strings ("inf", "nan") in
Dakota input.

How to Work with the Pydantic Documentation
-------------------------------------------

When using the generated Pydantic documentation, think of each model as 
representing a portion of the Dakota input hierarchy. Field names within
a model correspond to JSON keys. Nested models represent nested blocks 
of the input structure. Required vs optional fields, default values, and
allowed types are all defined there.

In practice, constructing a JSON input using the documentation involves
starting from the top-level :ref:`DakotaStudy <pydantic-study>` model and working down
through the nested models.

In the documentation, fields are followed by a type. Some of the more
common ones are:

* a primitive type such as `int` or `str` if the keyword takes an argument
* the hyperlinked name of a model, which indicates that a field has children
* pipe-delimited models, indicating a Union or "choose one" relationship
* Literal[True]. These are leaf keywords and must be set to true.

Some fields have defaults, which follow an equal sign. Fields with defaults
are not required to be set. Others are followed by `| None`, which marks a
field that is optional but has no specified default.

Using ``dakota_parser`` to Bootstrap JSON Input
-----------------------------------------------

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



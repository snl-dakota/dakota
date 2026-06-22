.. _`jsoninput`:

"""""""""""""""""""""""
JSON Format Input Files
"""""""""""""""""""""""

In addition to Dakota's freeform input file format, studies may be configured using
JSON format input files. The JSON format is designed to closely mimic the freeform format
while also providing a more structured and machine-readable and writable alternative.
The JSON schema (that is, specification of the allowed structure and content of JSON input files)
is defined by a set of Pydantic models, which are documented in :ref:`pydantic-docs`.
It is not necessary to understand the models to use the JSON format. Most users will find it
more helpful to refer this page and to Dakota's keyword reference documentation when constructing
JSON input files.

Example JSON Input
------------------

The following example will be used to illustrate some of the important features of Dakota's JSON format,
including how it differs from the freeform format.


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

.. _`jsoninput:groupkeys`:

Group keys
~~~~~~~~~~

In many places where the freeform syntax allows mutually exclusive keyword
groups, the JSON representation introduces explicit intermediate objects
(`group keys` or `anchors`) to represent those choices. These do not appear
in the traditional input format but are required in JSON to make the structure
unambiguous. In the example, ``gradient_type`` (line 32) and ``hessian_type`` (line 35)
are examples of anchor keywords.

In the :ref:`Keyword Reference <keyword-reference-area>`, `JSON Group Keys`
are documented in each keywords child keyword table.

.. _`jsoninput:argumentkeys`:

Argument keys
~~~~~~~~~~~~~

In the freeform format, a keyword may both take a literal argument and
contain child keywords. JSON does not support this pattern directly, so
the literal argument is represented as a separate child field (an "argument"
keyword). As a result, some values that appear inline in the traditional format
become nested fields in JSON.

An example of an argument keyword is ``count`` (line 40), which denotes the number of
continuous design variables. In the :ref:`Keyword Reference <keyword-reference-area>`,
`Argument Keys` appear in the `Specification` section of each keyword's documentation.

Nonfinite numbers
~~~~~~~~~~~~~~~~~

Nonfinite numbers such as `inf` and `nan` are not natively supported
by JSON. They must be provided as quoted strings ("inf", "nan") in
Dakota input.

Aliases, abbreviations, and other features
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Some of the convenience features supported in freeform inputs files do not
work in JSON. These include keyword aliases (e.g. using `nond_sampling` in place of
:dakkw:`method-sampling`), abbreviations (e.g. using `analysis_driver` when the full keyword is
:dakkw:`interface-analysis_drivers`), :ref:`shortcut notations <inputfile_formatting_args>`
such as `N*Value` and `L:S:U` sequences, and input file template pre-processing.

Using ``dakota_parser`` to Bootstrap JSON Input
-----------------------------------------------

While it is possible to write JSON input files from scratch, it may sometimes
be more convenient to start with a working freeform input file and convert it to JSON
using Dakota's `dakota_parser` tool.

1. Begin with a working traditional input file.
2. Run `dakota_parser`, located in the `bin` folder of the Dakota install, to
   generate the corresponding JSON: 
   ``dakota_parser <freeform dakota input> --json``
3. Use the result as a starting point, modifying it as needed.
4. Refer to the Keyword Reference and Pydantic documentation to 
   understand and refine specific fields.



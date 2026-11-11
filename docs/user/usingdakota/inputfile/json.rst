.. _`jsoninput`:

"""""""""""""""""""""""
JSON Format Input Files
"""""""""""""""""""""""

A Dakota input file can be written in JSON format. A JSON input file describes
the same study as a :ref:`freeform input file <inputfile_formatting_main>`:
the same six blocks and the same keywords, written as nested JSON objects.
Because JSON is a standard format, scripts and other tools can easily
generate, read and check Dakota studies written in it. Both formats are fully
supported. For a side-by-side introduction to the two formats, see
:ref:`studyanatomy-formats`.

To run a JSON input file, use the ``-json`` command-line option:

.. code-block:: console

   dakota -json study.json

The JSON schema (that is, the specification of the allowed structure and
content of JSON input files) is defined by a set of Pydantic models, which are
documented in :ref:`pydantic-docs`. You don't need to understand the models
to use the JSON format. Most users will find this page and Dakota's
:ref:`Keyword Reference <keyword-reference-area>` more helpful when writing
JSON input files.

Example JSON Input
------------------

The following example illustrates the main features of Dakota's JSON format.


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
* required ``interface``, ``responses``, and ``variables`` keys (lines 18, 30, 45),
  whose values are arrays of length 1 or more of configuration objects for those
  blocks.

The structure of the overall study is specified by the `DakotaStudy` model.

Leaf Keywords
~~~~~~~~~~~~~

Keywords that have no children ("leaf" keywords) receive
a constant ``true`` argument. In the example input in the
preceding section, the keys ``analytic_gradients`` (line 33) and
``no_hessians`` (line 36) are instances of this.

Empty Objects
~~~~~~~~~~~~~

Empty objects are used when a keyword can have optional
child keywords, none of which happen to be present in a
particular study. The ``tabular_data`` (line 3) and ``single`` (line 15)
keys are examples.

.. _`jsoninput:groupkeys`:

Group keys
~~~~~~~~~~

Where a study must choose one of several mutually exclusive keywords, the
JSON format wraps the choice in an intermediate object, called a `group key`
or `anchor`, that names what is being chosen. Group keys make the structure
unambiguous. In the example, ``gradient_type`` (line 32) and ``hessian_type``
(line 35) are group keys: each holds the one gradient or Hessian option the
study uses. The freeform format has no group keys; there, the chosen keyword
appears directly.

In the :ref:`Keyword Reference <keyword-reference-area>`, `JSON Group Keys`
are documented in each keyword's child keyword table.

.. _`jsoninput:argumentkeys`:

Argument keys
~~~~~~~~~~~~~

Some keywords take a value and also have child keywords. A JSON object can't
have both a value and children, so the keyword's own value goes in a separate
child field, called an `argument key`. For example, ``count`` (line 48) holds
the number of continuous design variables. In the freeform format, the same
value is written directly after the keyword (``continuous_design = 2``).

In the :ref:`Keyword Reference <keyword-reference-area>`,
`Argument Keys` appear in the `Specification` section of each keyword's
documentation.

Nonfinite numbers
~~~~~~~~~~~~~~~~~

Nonfinite numbers such as `inf` and `nan` are not natively supported
by JSON. They must be provided as quoted strings ("inf", "nan") in
Dakota input.

.. _`jsoninput:converting`:

Converting Freeform Input Files
-------------------------------

If you already have freeform input files, Dakota's ``dakota_parser`` tool can
convert them to JSON.

Using ``dakota_parser``
~~~~~~~~~~~~~~~~~~~~~~~

1. Begin with a working freeform input file.
2. Run ``dakota_parser``, located in the ``bin`` folder of the Dakota install,
   to generate the corresponding JSON:
   ``dakota_parser <freeform dakota input> --json``
3. Use the result as a starting point, modifying it as needed. The output
   includes every setting, with default values filled in for the ones your
   freeform file left out. You can delete the defaults you don't need, to
   keep the file short.
4. Refer to the Keyword Reference and Pydantic documentation to
   understand and refine specific fields.

Freeform features that JSON doesn't support
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Some of the convenience features supported in freeform input files do not
work in JSON. These include keyword aliases (e.g. using `nond_sampling` in place of
:dakkw:`method-sampling`), abbreviations (e.g. using `analysis_driver` when the full keyword is
:dakkw:`interface-analysis_drivers`), :ref:`shortcut notations <inputfile_formatting_args>`
such as `N*Value` and `L:S:U` sequences, and input file template pre-processing.

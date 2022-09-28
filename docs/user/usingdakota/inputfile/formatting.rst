.. _`inputfile_formatting_main`:

"""""""""""""""""""""
Input File Formatting
"""""""""""""""""""""
 
------------------------
General Formatting Rules
------------------------

- “Flat” text only.
- Whitespace is ignored.
- Comments begin with # and continue to the end of the line.
- Keyword order is largely unimportant as long as major sections are respected and there is no ambiguity.
- Equal signs are optional.
- Strings can be surrounded by single or double quotes (but not “fancy” quotes).
- Scientific notation is fine.

.. _`inputfile_formatting_args`:

--------------------
Specifying Arguments
--------------------

Some keywords, such as those providing bounds on variables, have an associated list of values or strings, referred to as arguments.

When the same value should be repeated several times in a row, you can use the notation N\*value instead of repeating the value N times.

For example...

.. code-block::

    lower_bounds  -2.0  -2.0  -2.0
    upper_bounds   2.0   2.0   2.0


...could also be written...


.. code-block::

   lower_bounds  3*-2.0
   upper_bounds  3* 2.0

... with optional spaces around the \* .

Another possible abbreviation is for sequences: ``L:S:U`` (with optional spaces around the : ) is expanded to ``L L+S L+2*S ... U``, and ``L:U`` (with no second colon) is treated as ``L:1:U``.

For example, in one of the test examples distributed with Dakota (test case 2 of ``test/dakota_uq_textbook_sop_lhs.in`` )...

.. code-block::

    histogram_point = 2
      abscissas   = 50. 60. 70. 80. 90.
                    30. 40. 50. 60. 70.
      counts      = 10 20 30 20 10
                    10 20 30 20 10


...could also be written...

.. code-block::

    histogram_point = 2
      abscissas   = 50 : 10 : 90
                    30 : 10 : 70
      counts      = 10:10:30 20 10
                    10:10:30 20 10

Count and sequence abbreviations can be used together. For example...

.. code-block::

    response_levels =
        0.0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0
        0.0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0

...can be abbreviated...

.. code-block::

    response_levels =
        2*0.0:0.1:1.0


.. _`inputfile_formatting_mistakes`:

-----------------------------
Common Specification Mistakes
-----------------------------

- Spelling mistakes and omission of required parameters are the most common errors. Some causes of errors are more obscure:

  - Documentation of new capability sometimes lags its availability in source and executables, especially stable releases. When parsing errors occur that the documentation cannot explain, reference to the particular input specification dakota.input.summary used in building the executable, which is installed alongside the executable, will often resolve the errors.
  - In most cases, the Dakota parser provides error messages that help the user isolate errors in input files. Running dakota -input dakota_study.in -check will validate the input file without running the study.
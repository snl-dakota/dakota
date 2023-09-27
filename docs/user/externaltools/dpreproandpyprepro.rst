.. _`interfaces:dprepro-and-pyprepro`:

"""""""""""""""""""""""""""""""""""""""
Preprocessing with dprepro and pyprepro
"""""""""""""""""""""""""""""""""""""""

Dakota is packaged with two template processing tools that are intended
for use in the preprocessing phase of analysis drivers.

The first tool, ``pyprepro``, features simple parameter substitution,
setting of immutable (fixed) variable names, and provides full access
within templates to all of the Python programming language. As such,
templates can contain loops, conditionals, lists, dictionaries, and
other Python language features.

The second tool, ``dprepro``, uses the same template engine as
``pyprepro``, and in addition understands Dakota’s parameter file
formats. In particular, when using ``dprepro`` in an analysis driver,
Dakota variables become available for use within templates. ``dprepro``
is also integrated with the ``dakota.interfacing`` module to provide
direct access to ``Parameters`` and ``Results`` objects within templates
(see Section `1.9.3.8 <#interfaces:params-and-results>`__) and to
provide template processing capability within Python scripts that import
``dakota.interfacing``.

.. _`interfaces:dprepro-changes`:

Changes and Updates to dprepro
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The version of ``dprepro`` described in this section is a replacement
for an earlier version that shipped with Dakota releases prior to 6.8.
Although the new version offers a wide array of new features, it largely
maintains backward compatibility with the old. Users should be aware of
two important differences between the two versions.

-  The earlier version of ``dprepro`` was written in Perl, but the new
   one is written in Python. It is compatible with Python 2 (2.6 and
   greater) and 3. Some users, especially on Windows, may need to modify
   existing analysis drivers to invoke ``dprepro`` using Python instead
   of Perl.

-  Recent versions of Perl ``dprepro`` supported per-field output
   formatting in addition to the global numerical format that could be
   specified on the command line. This was accomplished by adding a
   comma- separated format string to individual substitution expressions
   in templates (e.g. ``{x1,%5.3f}``). Per-field formatting remains a
   feature of the new ``dprepro``, but the syntax has changed.
   Python-style string formatting is used, as explained in
   Section `1.9.5.5 <#interfaces:per-field-output-formatting>`__.
   Existing templates that make use of per-field formatting will need to
   be updated.

Although the old ``dprepro`` has been deprecated as of the 6.8 release
of Dakota, it is still available in Dakota’s ``bin/`` folder under the
name ``dprepro.perl``.

.. _`interfaces:dprepro-usage`:

Usage
~~~~~

Running ``dprepro`` with the ``--help`` option at the command prompt
causes its options and arguments to be listed. These are shown in
:numref:`advint:dprepro_usage`.

``dprepro`` accepts three positional command line arguments. They are:

#. ``include``: The name of a Dakota parameters file (*required*),

#. ``infile``: The name of a template file (or a dash if the template is
   provided on ``stdin``) (*required*), and

#. ``outfile``: The name of the output file, which is the result of
   processing the template. This argument is optional, and output is
   written to ``stdout`` if it is missing.

The remaining options are used to

-  Set custom delimiters for Python code lines (``--code``) and blocks
   (``--code-block``) and for inline statements that print
   (``--inline``). The last of these is equivalent to Perl
   ``dprepro``\ ’s ``--left-delimiter`` and ``--right-delimiter``
   switches, which also have been preserved to maintain backward
   compatibility. They default to ``"{ }"``.

-  Insert additional parameters for substitution, either from a JSON
   file (``--json-include``) or directly on the command line
   (``--var``). Variables that are defined using these options are
   *immutable* (Section `1.9.3.7 <#interfaces:immutable-variables>`__).

-  Silence warnings (``--no-warn``)

-  Set the default numerical output format (``--output-format``).

.. literalinclude:: ../usingdakota/samples/dprepro_usage
   :language: dakota
   :tab-width: 2
   :caption: ``dprepro`` usage
   :name: advint:dprepro_usage

The ``pyprepro`` script accepts largely the same command line options.
The primary differences are that ``pyprepro`` does not require or accept
Dakota-format parameters files, and it has just two positional command
line arguments, the ``infile`` and ``outfile``, both defined as above.
In addition, ``pyprepro`` accepts one or more ``--include`` files. These
may be used to set parameters and execute arbitrary Python scripting
before template processing occurs (See
Section `1.9.3.7 <#interfaces:immutable-variables>`__).

.. _`interfaces:template-expressions`:

Template Expressions
~~~~~~~~~~~~~~~~~~~~

This section describes the expressions that are permitted in templates.
All examples, except where otherwise noted, use the default delimiters
``"{  }"`` for inline printed expressions, ``%`` for single-line Python
statements, and ``"{% %}"`` for Python code blocks.

Expressions can be of three different forms (with defaults)

-  Inline single-line expressions (rendered): ``{expression}``

-  Python code single-line (silent): ``% expression``

-  Python code multi-line blocks (silent):
   ``{% expression (that can span many lines) %}``

Expressions can contain just about any valid Python code. The only
important difference is that indentation is ignored and blocks must end
with ``end``. See the examples below.

.. _`interfaces:inline-expressions`:

Inline Expressions
^^^^^^^^^^^^^^^^^^

Inline expressions are delineated with ``{expression}`` and *always
display*.

Consider:

::

   param1 = {param1 = 10}
   param2 = {param1 + 3}
   param3 = {param3 = param1**2}

Returns:

::

   param1 = 10
   param2 = 13
   param3 = 100

In this example, the first and third line both display a value *and* set
the parameter.

.. _`interfaces:python-single-line-code`:

Python Single Line Code
^^^^^^^^^^^^^^^^^^^^^^^

A ``%`` at the start of a line is used to begin a single-line code
expression. These are non-printing. Consider the following example.

::

   % param1 = pi/4
   The new value is {sin(param1)}

It returns:

::

   The new value is 0.7071067812

Furthermore, single lines can be used for Python logic and loops. This
example demonstrates looping over an array, which is explained in
further detail below. As stated previously, unlike ordinary Python,
indentation is not required and is ignored. Blocks of Python code are
concluded with ``end``.

::

   % angles = [0,pi/4,pi/2,3*pi/4,pi]
   % for angle in angles:
   cos({angle}) = { cos(angle)}
   % end

Returns:

::

   cos(0) = 1
   cos(0.7853981634) = 0.7071067812
   cos(1.570796327) = 6.123233996e-17
   cos(2.35619449) = -0.7071067812
   cos(3.141592654) = -1

.. _`interfaces:code-blocks`:

Code Blocks
^^^^^^^^^^^

Finally, multi-line code blocks may be specified without prepending each
Python statement with ``%``. Instead, the entire block is enclosed in
``{% %}``. (Indentation is ignored within code blocks.)

::

   {%
   # Can have comments too!
   txt = ''
   for ii in range(10):
       txt += ' {}'.format(ii)
   end
   %}
   txt: {txt}

returns:

::

   txt:  0 1 2 3 4 5 6 7 8 9

.. _`interfaces:changing-delimiters`:

Changing Delimiters
^^^^^^^^^^^^^^^^^^^

As noted in the ``--help`` for ``dprepro`` and ``pyprepro``, the
delimiters for single-line Python statements, code blocks, and inline
printed expressions can be changed. This is useful when the defaults are
reserved characters in the output format.

For code blocks (default ``{% %}``), the innermost characters cannot be
any of “``{}[]()``”.

Furthermore every template can individually reset the command delimiter by
specifying them at the top of the file. From the help:

::

    Specify delineators as the the first non-whitespace line. Start with a
    comment '//', '#','%', '$' or '' (nothing), then a command, then '=' or
    space, followed by the new setting. See examples.

    Commands are:

          Commands                Flags
      ---------------------------------------
      | PYPREPRO_CODE        |              |
      | DPREPRO_CODE         |  --code      |
      | PREPRO_CODE          |              |
      | ---------------------|--------------|
      | PYPREPRO_CODE_BLOCK  |              |
      | DPREPRO_CODE_BLOCK   | --code-block |
      | PREPRO_CODE_BLOCK    |              |
      | ---------------------|--------------|
      | PYPREPRO_INLINE      |              |
      | DPREPRO_INLINE       | --inline     |
      | PREPRO_INLINE        |              |
      ---------------------------------------


.. _`interfaces:escaping-delimiters`:

Escaping Delimiters
^^^^^^^^^^^^^^^^^^^

All delimiters can be escaped with a leading ``\``. A double ``\\``
followed by the delimiter will return ``\``. For example:

::

   {A=5}
   \{A=5\}
   \\{A=5\\}

Returns:

::

   5
   {A=5}
   \{A=5\}  

Note that escaping the trailing delimiter (e.g. ``\}``) is optional.

.. _`interfaces:whitespace-control`:

Whitespace Control
^^^^^^^^^^^^^^^^^^

Expressions span the entire line, which can possibly introduce undesired
white space. Ending a line with ``\\`` will prevent the additional
space. Consider the following:

::

   BLOCK \\
   {%
   if True:
       block = 10
   else:
       block = 20
   end
   %}
   {block}

Which renders as:

::

   BLOCK 10

Without the trailing ``\\``, the result would instead be:

::

   BLOCK
   10

This can also be abused to allow spacing. Consider the following:

::

   I want this to \\
   %
   render as \\
   %
   one line

Since the ``%`` symbolize a code block (empty in this case), it will
render

::

   I want this to render as one line

.. _`interfaces:immutable-variables`:

Immutable Variables
^^^^^^^^^^^^^^^^^^^

Variables can be fixed such that they cannot be redefined (without
explicitly allowing it).

In this example, the attempted reassignment of ``param`` to 20 is
ignored,

::

   % param = Immutable(10)
   % param = 20 
   {param}

and the output is

::

   10

because ``param`` is ``Immutable``. To explicitly make a variable
mutable again, call it with ``Mutable()``:

::

   set             : \{ param = Immutable(10) \} : { param = Immutable(10) }           
   try to reset    : \{ param = 20 \}            : { param = 20 }          
   make mutable    : \{ param = Mutable(21) \}   : { param = Mutable(21) } 
   reset           : \{ param = 20 \}            : { param = 20 }         

Returns:

::

   set             : { param = Immutable(10) } : 10
   try to reset    : { param = 20 }            : 10
   make mutable    : { param = Mutable(21) }   : 21
   reset           : { param = 20 }            : 20

Note that any variable set on the command line by any of these three
means:

-  ``--var`` argument

-  ``--include`` file

-  ``--json-include`` file

is immutable. This listing is in order of precedence; variables set by a
``--var`` argument cannot be modified by ``--include`` or
``--json-include`` files. This feature is useful for overriding defaults
set in templates.

Suppose the template file ``MyTemplate.inp`` contains:

::

   param1 = {param1 = 10}
   param2 = {param2 = pi}

Executing ``pyprepro MyTemplate.in`` yields:

::

   param1 = 10
   param2 = 3.141592654

However, for ``pyprepro --var "param1=30" MyTemplate.in``:

::

   param1 = 30
   param2 = 3.141592654

Or, if an optional ``--include`` file that is named ``MyInclude.inp``
and contains the following is added:

::

   {param1 = 32}

Then running ``pyprepro --include MyInclude.inp MyTemplate.inp``
outputs:

::

   param1 = 32
   param2 = 3.141592654

Note that variable definitions set using ``--var`` override definitions
in ``--include`` files.

There is one caveat to variable immutability. While the variable name is
reserved, the value can still be changed if it is a mutable Python
object (“mutable” has different meanings for Python objects than is used
in ``pyprepro`` and ``dprepro`` templates). For example:

::

   % param = Immutable( [1,2,3])
   % param.append(4)   # This will work because it is modifying the object
   % param = ['a','b','c']   # This won't because it is redefining
   {param}

Will output:

::

   [1, 2, 3, 4]

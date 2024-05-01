.. _`interfaces:dakota.interfacing`:

"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
Parameters and Results Using the Python dakota.interfacing module
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

The Python module ``dakota.interfacing`` provides a Python interface
to read and write Dakota parameters and results files, respectively.
Because of this, ``dakota.interfacing`` can greatly simplify
development of black-box interfaces. The benefit may be greatest when
one or more phases of the interface (pre-processing, execution,
post-processing) is written in Python.

The following sections describe the components of
``dakota.interfacing``. These components include:

-  The :class:`Parameters` class. Makes available the variable information
   for a single evaluation
-  The :class:`Results` class. Collects results for a single evaluation and
   writes them to file
-  The :class:`BatchParameters` and :class:`BatchResults` classes. Containers for
   multiple :class:`Parameters` and :class:`Results` objects; used when
   evaluations are performed by Dakota in :ref:`batch mode <interfaces:batch>`
-  The :class:`BatchSplitter', for splitting batch parameters files into the text
   for individual evaluations. Useful when converting an existing workflow that
   expects individual parameters files to work with Dakota's batch interface.
-  The :func:`read_parameters_file` function. Constructs :class:`Parameters`,
   :class:`Results`, :class:`BatchParameters`, and :class:`BatchResults` objects from a
   Dakota parameters file.

API
~~~

.. function:: read_parameters_file(parameters_file=None, \
    results_file=None, ignore_asv=False, batch=False, infer_types=True, \
    types=None)
           
    Creates :class:`Parameters`, :class:`Results`, :class:`BatchParameters`, and :class:`BatchResults` objects from a Dakota parameters file.
           
    :param parameters_file: The names of the parameter file that is to be read.
                            The name can be an absolute or relative filepath, or just a filename.
                            If a parameters file is not provided, it will be obtained from the command line arguments.
                            (The parameters filename is assumed to be the first argument.)
                            Note that if the working directory has changed since script invocation,
                            filenames provided as command line arguments by Dakota’s ``fork`` or ``system``
                            interfaces may be incorrect.           
    :param results_file: The name of the results file that ultimately is to be
                         written. The name can be an absolute or relative filepaths, or just a filename.
                         If a results file is not provided, it will be obtained from the command line arguments.
                         (The results filename is assumed to be the second command line argument.) Note that
                         if the working directory has changed since script invocation, filenames provided
                         as command line arguments by Dakota’s ``fork`` or ``system`` interfaces may be incorrect.
                         If *results_file* is set to the constant ``dakota.interfacing.UNNAMED``, the :class:`Results`
                         or :class:`BatchResults` object is constructed without a results file name. In this case, an
                         output stream must be provided when :meth:`Results.write` or :meth:`BatchResults.write` is
                         called. Unnamed results files are most helpful when no results file will be written,
                         as with a script intended purely for pre-processing.
    :param ignore_asv: By default, the returned :class:`Results` or :class:`BatchResults` object enforces
                       the active set vector (see the :class:`Results` class section). This behavior
                       can be overridden, allowing any property (function, gradient, Hessian)
                       of a response to be set, by setting this field to ``True``. This option can be useful when
                       setting up or debugging a driver.
    :param batch: Must be set to ``True`` when batch evaluation has been requested in the Dakota input file, and ``False`` when not.
    :param infer_types: Controls how types are assigned to parameter values.  The values initially are read
                        as strings from the Dakota parameters file. If ``infer_types`` is ``False`` and ``types`` is ``None``,
                        they remain as type ``str``. If ``infer_types`` is ``True``, an attempt is made to "guess" more
                        convenient types. Conversion first to ``int`` and then to ``float`` are tried. If both fail, the value remains a ``str``.
    :param type: Controls how types are assigned to parameter values.  The values initially are read
                 as strings from the Dakota parameters file. If ``infer_types`` is ``False`` and ``types`` is ``None``,
                 they remain as type ``str``. If ``infer_types`` is ``True``, an attempt is made to "guess" more
                 convenient types. Conversion first to ``int`` and then to ``float`` are tried. If both fail, the value remains a ``str``.
                 
    :return: For single, non-batch evaluation, it returns a tuple that contains (:class:`Parameters`, :class:`Results`). For batch
             evaluations, it instead returns a tuple containing (:class:`BatchParameters`, :class:`BatchResults`).

.. note::
   
   Sometimes automatic type inference does not work as desired; a user
   may have a string-valued variable with the element "5", for example,
   that he does not want converted to an ``int``. Or, a user may wish to
   convert to a custom type, such as ``np.float64`` instead of the
   built-in Python ``float``. The ``types`` argument is useful in these
   cases. It can be set either to a ``list`` of types or a ``dict`` that
   maps variable labels to types. Types communicated using the ``types``
   argument override inferred types. If ``types`` is a list, it must have
   a length equal to the number of variables. A dictionary, on the other
   hand, need not contain types for every variable. This permits
   variable-by-variable control over assignment and inference of types.


.. class:: Parameters

    :class:`Parameters` objects make the variables, analysis components,
    evaluation ID, and evaluation number read from a Dakota parameters file
    available through a combination of key-value access and object
    attributes. Although :class:`Parameters` objects may be constructed directly,
    it is advisable to use the :func:`read_parameters_file` function instead.

    Variable values can be accessed by Dakota descriptor or by index using
    ``[]`` on the object itself. Variables types (integer, real, string) are
    inferred by first attempting to convert to ``int`` and then, if this
    fails, to ``float``.

    Variable values can be accessed by Dakota descriptor or by index using
    ``[]`` on the object itself. Variables types are inferred or set as
    described in the previous section.

    Analysis components are accessible by index only using the :attr:`an_comps`
    attribute. Iterating over a :class:`Parameters` object yields the variable
    descriptors.

   .. attribute:: an_comps
   
      List of the analysis components (strings).

   .. attribute:: eval_id
   
      Evaluation id (string).
      
   .. attribute:: eval_num
     
      Evaluation number (final token in eval_id) (int).
      
   .. attribute:: aprepro_format
   
      Boolean indicating whether the parameters file was in aprepro (True) or Dakota (False) format.
      
   .. attribute:: descriptors
   
      List of the variable descriptors
      
   .. attribute:: num_variables
      
      Number of variables
      
   .. attribute:: num_an_comps
      
      Number of analysis components
      
   .. attribute:: metadata
   
      Names of requested metadata fields (strings)
      
   .. attribute:: num_metadata
   
      Number of requested metadata fields.

   .. method:: items()
   
      Return an iterator that yields tuples of the descriptor and value for each parameter. (:class:`Results` objects also have ``items()``.)
      
   .. method:: values()
   
      Return an iterator that yields the value for each parameter. (:class:`Results` objects have the corresponding method ``responses()``.)

.. class:: Results

    :class:`Results` objects do the following:

    -  communicate response requests from Dakota (active set vector and
       derivative variables)
    -  collect response data (function values, gradients, and Hessians)
    -  write Dakota results files

    :class:`Results` objects are collections of :class:`Response` objects, which are
    documented in the following section. Each :class:`Response` can be accessed
    by name (Dakota descriptor) or by index using ``[]`` on the :class:`Results`
    object itself. Iterating over a :class:`Results` object yields the response
    descriptors. Although :class:`Results` objects may be constructed directly,
    it is advisable to use the :func:`read_parameters_file` function instead.

   .. attribute:: eval_id
   
      Evaluation id (a string).
      
   .. attribute:: eval_num
   
      Evaluation number (final token in eval_id) (int).
      
   .. attribute:: aprepro_format
   
      Boolean indicating whether the parameters file was in aprepro (True) or Dakota (False) format.
      
   .. attribute:: descriptors
   
      List of the response descriptors (strings)
     
   .. attribute:: num_responses
      
      Number of variables (read-only)
      
   .. attribute:: deriv_vars
   
      List of the derivative variables (strings)
      
   .. attribute:: num_deriv_vars
     
      Number of derivative variables (int)

   .. method:: items()
   
      Return an iterator that yields tuples of the descriptor and :class:`Response` object for each response. (:class:`Parameters` objects also have ``items()``.)
      
   .. method:: responses()
   
      Return an iterator that yields the :class:`Response` object for each response. (:class:`Parameters` objects have the corresponding method ``values()``.)
      
   .. method:: fail()
   
      Set the FAIL attribute. When the results file is written, it will contain only the word FAIL, triggering :ref:`Dakota’s failure capturing behavior <failure>`.
   
   .. method:: write (stream=None, ignore_asv=None)
   
      Write the results to the Dakota results file.
      
      :param stream: If *stream* is set, it overrides the results file name provided at construct time. It must be an open file-like object, rather than the name of a file.
      :param ignore_asv: If *ignore_asv* is True, the file will be written even if information requested via the active set vector is missing.

.. note::
      
   Calling ``write()`` on a :class:`Results` object that was generated by reading a batch parameters file will raise a ``BatchWriteError``.
   Instead, ``write()`` should be called on the containing :class:`BatchResults` object.

.. class:: Response

    :class:`Response` objects store response information. They typically are instantiated and accessed through a Results object by index or response
    descriptor using ``[]``.

   .. attribute:: asv
      
      A `named tuple <https://docs.python.org/3/library/collections.html#collections.namedtuple>`_ with three members, *function*, *gradient*, 
      and *hessian*. Each is a boolean indicating whether Dakota requested the associated information for the response.
      
   .. attribute:: namedtuples
   
      These can be accessed by index or by member.
      
   .. attribute:: function
   
      Function value for the response. A ResponseError is raised if Dakota did not request the function value (and ignore_asv is False).
      
   .. attribute:: gradient
   
      Gradient for the response. Gradients must be a 1D iterable of values that can be converted to floats, such as a ``list`` or 1D
      ``numpy array``. A ResponseError is raised if Dakota did not request the gradient (and ignore_asv is False), or if the number of elements
      does not equal the number of derivative variables.

   .. attribute:: hessian
      
      Hessian value for the response. Hessians must be an iterable of iterables (e.g. a 2D ``numpy array`` or list of lists). A ResponseError is
      raised if Dakota did not request the Hessian (and ignore_asv is False), or if the dimension does not correspond correctly with the
      number of derivative variables.

.. class:: BatchParameters

    :class:`BatchParameters` objects are collections of :class:`Parameters` objects. The individual :class:`Parameters` objects can be accessed by index ([]) or
    by iterating the :class:`BatchParameters` object. Although :class:`BatchParameters` objects may be constructed directly, it is advisable
    to use the :func:`read_parameters_file` function instead.

   .. attribute:: batch_id
   
      The "id" of this batch of evaluations, reported by Dakota (string).

.. class:: BatchResults

    :class:`BatchResults` objects are collections of :class:`Results` objects. The individual :class:`Results` objects can be accessed by index ([]) or by
    iterating the :class:`BatchResults` object. Although :class:`BatchResults` objects may be constructed directly, it is advisable to use the
    :func:`read_parameters_file` function instead.

   .. attribute:: batch_id
   
      The "id" of this batch of evaluations, reported by Dakota (string)

   .. method:: write (stream=None, ignore_asv=None) 
   
      :param stream: If *stream* is set, it overrides the results file name
                     provided at construct time. It must be an open file-like object,
                     rather than the name of a file.
      :param ignore_asv: If *ignore_asv* is True, the file will be written even
                         if information requested via the active set vector is missing.

.. class:: BatchSplitter

    A :class:`BatchSplitter` object splits a batch paramters file into the text of individual parameter sets. The parameter sets are 
    accessible as lists of newline terminated strings, in dakota or aprepro format, by 0-based index or by iterating the object. A specified
    parameter set can also be written to file. Calling `len()` on the object returns the number of evaluations in the batch.

   .. attribute:: batch_id
   
      The "id" of this batch of evaluations, reported by Dakota (string)

   .. attribute:: eval_nums

      Evaluation numbers in the batch (list of int).

   .. attribute:: parameters_file

      Name of the batch parameters file (string)

   .. attribute:: format

      Format of the file, "DAKOTA" or "APREPRO" (string)
      
   .. method:: write (index, filename)
   
      Write parameters for one evaluation to a file.
      
      :param index: Index of parameters set (int)
      :param filename: Filepath (string or pathlib.Path)
 
Processing Templates
~~~~~~~~~~~~~~~~~~~~

Dakota is packaged with a sophisticated command-line template processor
called :ref:`dprepro <interfaces:dprepro-and-pyprepro>`. Templates may be
processed within Python analysis drivers without externally invoking
``dprepro`` by calling the ``dprepro`` function:

.. function:: dprepro(template, parameters=None, results=None, include=None, output=None, fmt='%0.10g', \
   code='%', code block='{% %}', inline='{ }', warn=True)

If *template* is a string, it is assumed to contain a template. If it is
a file-like object (that has a ``.read()`` method), the template will be
read from it. (Templates that are already in string form can be passed
in by first wrapping them in a `StringIO <https://docs.python.org/3/library/io.html?highlight=stringio#io.StringIO>`_
object.)

:class:`Parameters` and :class:`Results`

:class:`Parameters` and :class:`Results` objects can be made available to the
template using The *parameters* and *results* keyword arguments, and
additional variable definitions can be provided in a ``dict`` via the
*include* argument.

The *output* keyword is used to specify an output file for the processed
template. *output=None* causes the output to be returned as a string. A
string is interpreted as a file name, and a file-like object (that has a
``.write()`` method) is written to.

The *fmt* keyword sets the global numerical format for template output.

*code*, *code_block*, and *inline* are used to specify custom delimiters
for these three types of expressions within the template.

Finally, the *warn* keyword controls whether warnings are printed by the
template engine.

dakota.interfacing Examples
~~~~~~~~~~~~~~~~~~~~~~~~~~~

In addition to those in this section, the
``dakota/share/dakota/examples/official/drivers/Python/di``
folder contains a runnable
example of a Python analysis driver. This example demonstrates the
``dakota.interfacing`` module.

For most applications, using ``dakota.interfacing`` is straightforward.
The first example, in :numref:`diexample:simple`,
is a mock analysis driver. Two variables with the descriptors ``x1`` and
``x2`` are read from the Dakota parameters file and used to evaluate the
fictitious user function ``applic_module.run()``. The result, stored in
``f``, is assigned to the ``function`` value of the appropriate
response. (A common error is leaving off the ``function`` attribute,
which is needed to distinguish the function value of the response from
its gradient and Hessian.)

.. code-block:: python
   :caption: A simple analysis driver that uses `dakota.interfacing`.
   :name: diexample:simple
   
   import dakota.interfacing as di
   import applic_module # fictitious application 

   params, results = di.read_parameters_file()

   # parameters can be accessed by descriptor, as shown here, or by index
   x1 = params["x1"]
   x2 = params["x2"]

   f = applic_module.run(x1,x2)

   # Responses also can be accessed by descriptor or index
   results["f"].function = f
   results.write()

The :class:`Results` object exposes the active set vector read from the
parameters file. When analytic gradients or Hessians are available for
a response, the ASV should be queried to determine what Dakota has
requested for an evaluation. If an attempt is made to add unrequested
information to a response, a ``dakota.interface.ResponseError`` is
raised. The same exception results if a requested piece of information
is missing when ``Results.write()`` is called. The
``ignore_asv`` option to ``read_parameters_file`` and 
``Results.write()`` overrides ASV checks.

In :numref:`diexample:asv`, ``applic_module.run()``
has been modified to return not only the function value of ``f``, but
also its gradient and Hessian. The ``asv`` attribute is examined to
determine which of these to add to ``results["f"]``.

.. code-block:: python
   :caption: Examining the active set vector
   :name: diexample:asv
   :linenos:

   import dakota.interfacing as di
   import applic_module # fictitious application

   params, results = di.read_parameters_file()

   x1 = params["x1"]
   x2 = params["x2"]

   f, df, df2 = applic_module.run(x1,x2)

   if Results.asv.function:
       results["f"].function = f
   if Results.asv.gradient:
       results["f"].gradient = df
   if Results.asv.hessian:
       results["f"].hessian = df2

   results.write()

As of the 6.16 release, the direct Python interface can interoperate with
``dakota.interfacing`` using a feature of Python known as a decorator.
Instead of receiving parameters from the Dakota parameters file and
writing results to the results file as in :numref:`diexample:asv`,
the decorated Python driver works with the Python dictionary passed from
the direct Python interface.  An example of the decorator syntax and use
of the ``dakota.interfacing`` :class:`Parameters` and :class:`Results`
objects that get created automatically from the direct interface
Python dictionary is shown in :numref:`linkeddiexample:decorator`.  The
complete driver including details of the packing functions can be found in
the ``dakota/share/dakota/examples/official/drivers/Python/linked_di`` folder.

.. code-block:: python
   :caption: Decorated direct Python callback function using
             :class:`Parameters` and :class:`Results` objects
             constructed by the ``dakota.interfacing`` decorator
   :name: linkeddiexample:decorator

   from textbook import textbook_list
   import dakota.interfacing as di
   
   @di.python_interface
   def decorated_driver(params, results):
   
       textbook_input = pack_textbook_parameters(params, results)
       fns, grads, hessians = textbook_list(textbook_input)
       results = pack_dakota_results(fns, grads, hessians, results)
   
       return results


.. _`interfaces:params-and-results`:

DakotaParams and DakotaResults
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If the ``dakota`` :ref:`Python package <interfaces:dakota.interfacing>` is available for
import (e.g. has been added to the ``PYTHONPATH``), then ``dprepro``
generates :class:`Parameters` and :class:`Results` objects from the Dakota
parameters file. These are available for use in templates under the
names ``DakotaParams`` and ``DakotaResults``.

Use of these objects permits convenient access to information such as
the evaluation ID (``DakotaParams.eval_id``) and the active set vector
entries (``DakotaResults[0].asv.function``). Dakota variables also
become available not only directly within the template, but as members
of ``DakotaParams``. That is, if ``x1`` is a Dakota variable, it will be
available within a template both by the name ``x1``, and as
``DakotaParams["x1"]``. In this way, variables that have prohibited
names (explained in the following section) can still be accessed using
their original names.

.. _`interfaces:unicode`:

Unicode Support
^^^^^^^^^^^^^^^

Variables must obey the naming conventions for the version of Python
that is used to run ``d/pyprepro``. For Python 2, only ASCII
alphanumeric characters and the underscore are permitted, and
identifiers must not begin with a number. In Python 3, this requirement
is relaxed considerably, and many Unicode characters are permitted in
identifiers.

Because Dakota itself has few such restrictions on variable names,
``d/pyprepro`` "mangles" noncompliant names in the following ways before
making them available in templates:

-  Variables/parameters that begin with a number are prepended by the
   lowercase letter ’i’.

-  Disallowed characters such as # are replaced by underscores (``_``).

-  In Python 2, non-ASCII letters are normalized to their rough ASCII
   equivalents (e.g. ñ is replaced by n).

As stated in the previous section, when using ``dprepro`` with
``dakota.interfacing``, the original variable names are always available
via the ``DakotaParams`` object.

.. _`interfaces:scripting`:

Scripting
~~~~~~~~~

The language of ``pyprepro`` and ``dprepro`` templates is Python with a
single modification: In normal Python, indentation delineates blocks of
code. However, in ``d/pyprepro`` templates, indentation is ignored and
blocks must end with an ``end`` statement whether they are part of
multi-line code (``{% %}``) or part of single line operation (``%``).

Users unfamiliar with Python, but who do have experience with other
scripting languages such as MATLAB, should find it straightforward to
incorporate simple Python scripts into their templates. A brief guide in
basic Python programming follows. Interested users should consult any of
the many available Python tutorials and guides for more advanced usage.

.. _`interfaces:python-coding-tips`:

Python Coding Tips
^^^^^^^^^^^^^^^^^^

Here are a few characteristics of Python that may be important to note
by users familiar with other languages.

-  Lists (array-like containers) are zero-based

-  Exponentiation is double ``**``. Example: ``x**y`` (“x to the y”)

-  In many languages, blocks of code such as the bodies of loops,
   functions, or conditional statements, are enclosed in symbols such as
   { }. In ordinary Python, statements that initialize new blocks end in
   a colon (``:``), and code within the block is indented,
   conventionally by a single tab or by 4 spaces. In Python in
   ``d/pyprepro`` templates, initializing statements also end in colons,
   but indentation is ignored, and code blocks continue until an ``end``
   statement is encountered.

.. _`interfaces:conditionals`:

Conditionals
^^^^^^^^^^^^

Python has the standard set of conditionals. Conditional block
declaration must end with a ``:``, and the entire block must have an
``end`` statement. Consider the following example:

::

   % param = 10.5
   % if param == 10.0:
   param is 10! See: {param}
   % else:
   param does not equal 10, it is {param}
   % end

   % if 10 <= param <= 11:
   param ({param}) is between 10 and 11
   % else:
   param is out of range
   % end

results in:

::

   param does not equal 10, it is 10.5

   param (10.5) is between 10 and 11

Boolean operations are also possible using simple ``and``, ``or``, and
``not`` syntax

::

   % param = 10.5
   % if param >= 10 and param <= 11:
   param is in [10 11]
   % else:
   param is NOT in [10,11]
   % end

returns:

::

   param is in [10 11]

.. _`interfaces:loops`:

Loops
^^^^^

``for`` loops may be used to iterate over containers that support it. As
with conditionals, the declaration must end with ``:`` and the block
must have an ``end``.

To iterate over an index, from 0 to 4, use the ``range`` command.

::

   % for ii in range(5):
   {ii}
   % end

This returns:

::

   0
   1
   2
   3
   4

This example demonstrates iteration over strings in a list:

::

   % animals = ['cat','mouse','dog','lion']
   % for animal in animals:
   I want a {animal}
   %end

The output is:

::

   I want a cat
   I want a mouse
   I want a dog
   I want a lion

.. _`interfaces:lists`:

Lists
^^^^^

Lists are *zero indexed*. Negative indices are also supported, and are
interpreted as offsets from the last element in the negative direction.
Elements are accessed using square brackets (``[]``).

Consider:

::

   % animals = ['cat','mouse','dog','lion']
   {animals[0]}
   {animals[-1]}

which results in:

::

   cat
   lion

Note that ``d/pyprepro`` tries to nicely format lists for printing. For
certain types of objects, it may not work well.

::

   {theta = [0,45,90,135,180,225,270,315]}

(with ``{ }`` to print input) results in

::

   [0, 45, 90, 135, 180, 225, 270, 315]

.. _`interfaces:math-on-lists`:

Math on lists
^^^^^^^^^^^^^

Unlike some tools (e.g. MATLAB) mathematical operations may not be
performed on lists as a whole. Element-by-element operations can be
compactly written in many cases using *list comprehensions*:

::

   % theta = [0,45,90,135,180,225,270,315] 
   { [ sin(pi*th/180) for th in theta ] }

This results in

::

   [0, 0.7071067812, 1, 0.7071067812, 1.224646799e-16, -0.7071067812, -1, -0.7071067812]

Alternatively, if the NumPy package is available on the host system,
lists can be converted to arrays, which do support MATLAB-style
element-wise operations:

::

   % theta = [0,45,90,135,180,225,270,315]
   % import numpy as np
   % theta = np.array(theta) # Redefine as numpy array
   { np.sin(pi*theta/180) }

Returns:

::

   [0, 0.7071067812, 1, 0.7071067812, 1.224646799e-16, -0.7071067812, -1, -0.7071067812]

.. _`interfaces:strings`:

Strings
^^^^^^^

Python has powerful and extensive string support. Strings can be
initialized in any of the following ways:

::

   {mystring1="""
   multi-line
   string inline
   """}
   {mystring1}
   {% mystring2 = '''
   another multi-line example
   but in a block
   ''' %}
   mystring2: {mystring2}

   Single quotes: {'single'}
   Double quotes: {'double'}

Which returns:

::

   multi-line
   string inline


   multi-line
   string inline

   mystring2:
   another multi-line example
   but in a block


   Single quotes: single
   Double quotes: double

Strings can be enclosed by either single quotes (``'``) or double quotes
(``"``). The choice is a matter of convenience or style.

Strings can be joined by adding them:

::

   {%
   a = 'A'
   b = 'B'
   %}
   {a + ' ' + b}

returns:

::

   A B

.. _`interfaces:custom-functions`:

Custom Functions
^^^^^^^^^^^^^^^^

Arbitrary functions can be defined using either ``def`` or ``lambda``.

Consider the following: (note, we use indentation here for readability
but indentation *is ignored* and the function definition is terminated
with ``end``):

::

   {%
   def myfun1(param):
       return (param + 1) ** 2 + 3
   end

   myfun2 = lambda param: (param + 1) ** 2 + 5
   %}
   {myfun1(1.2)}
   {myfun2(1.2)}
   { [ myfun1(x) for x in [1,2,3,4] ] }

Returns:

::

   7.84
   9.84
   [7, 12, 19, 28]
   
.. _`interfaces:auxiliary-functions`:

Auxiliary Functions
~~~~~~~~~~~~~~~~~~~

Several auxiliary functions that are not part of Python are also
available within templates. The first is the ``include`` function.

.. _`interfaces:include`:

Include
^^^^^^^

Using

::

   % include('path/to/include.txt')

will insert the contents of ``path/to/include.txt``. The inserted file
can contain new variable definitions, and it can access older ones.
Parameters defined in the file are not immutable by default, unlike
those defined in files included from the command line using the
``--include`` option.

..
   TODO: review these claims after talking to Justin

``d/pyprepro`` performs limited searching for included files, first in
the path of the original template, and then in the path where
``pyprepro`` is executed.

.. _`interfaces:immutable-and-mutable`:

Immutable and Mutable
^^^^^^^^^^^^^^^^^^^^^

As explained elsewhere, variables can be defined as ``Immutable(value)``
or ``Mutable(value)``. If a variable is Immutable, it cannot be
reassigned without first explicitly make it Mutable.

.. note::

   Unlike :ref:`variables defined via - -include<interfaces:immutable-variables>`,
   variables from files read in using the ``include()`` function are Mutable by
   default.

.. _`interfaces:print-all-variables`:

Print All Variables
^^^^^^^^^^^^^^^^^^^

``all_vars()`` and ``all_var_names()`` print out all *defined*
variables. Consider the following that also demonstrates setting a
comment string (two ways)

::

   % param1 = 1
   {param2 = 'two'}
   all variables and values: {all_vars()}
   all varables: {all_var_names()}

   {all_var_names(comment='//')}
   // {all_var_names()} <--- Don't do this

Returns:

::

   two
   all variables and values: {'param1': 1, 'param2': u'two'}
   all varables: ['param2', 'param1']

   // ['param2', 'param1']
   // ['param2', 'param1'] <--- Don't do this

Notice the empty ``()`` at the end of ``all_vars`` and
``all_var_names``. If possible, it is better to use ``comment=//``
syntax since the result of these can be multiple lines.

.. _`interfaces:set-global-print-format`:

Set Global Numerical Format
^^^^^^^^^^^^^^^^^^^^^^^^^^^

As discussed elsewhere, the print format can be set on a per item basis
by manually converting to a string. Alternatively, it can be (re)set
globally inside the template (as well as at the command line).

::

   {pi}
   % setfmt('%0.3e')
   {pi}
   % setfmt() # resets
   {pi}

returns:

::

   3.141592654
   3.142e+00
   3.141592654

.. _`interfaces:per-field-output-formatting`:

Per-field Output Formatting
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Use Python string formatting syntax to set the output format of a
particular expression.

::

   {pi}
   { '%0.3f' % pi }

Will output:

::

   3.141592654
   3.142

.. _`interfaces:using-defaults-undefined-parameters`:

Defaults and Undefined Parameters
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Directly calling undefined parameters will result in an error. There is
no *universal* default value. However, there are the following
functions:

-  ``get`` – get param with optional default

-  ``defined`` – determine if the variable is defined

The usage is explained in the following examples:

::

   Defined Parameter:
   % param1 = 'one'
   { get('param1') } <-- one
   { get('param1','ONE') } <-- one

   Undefined Parameter
   { get('param2') } <-- *blank*
   { get('param2',0) } <-- 0

   Check if defined: { defined('param2') }

   % if defined('param2'):
   param2 is defined: {param2}
   % else:
   param2 is undefined
   % end

returns:

::

   Defined Parameter:
   one <-- one
   one <-- one

   Undefined Paremater
    <-- *blank*
   0 <-- 0

   Check if defined: False

   param2 is undefined

But notice if you have the following:

::

   {param3}

you will get the following error:

::

   Error occurred:
       NameError: name 'param3' is not defined

.. _`interfaces:mathematical-functions`:

Mathematical Functions
^^^^^^^^^^^^^^^^^^^^^^

All of the Python ``math`` module in imported with the functions:

::

     acos       degrees     gamma   radians  
     acosh      erf         hypot   sin      
     asin       erfc        isinf   sinh      
     asinh      exp         isnan   sqrt      
     atan       expm1       ldexp   tan       
     atan2      fabs        lgamma  tanh      
     atanh      factorial   log     trunc     
     ceil       floor       log10   
     copysign   fmod        log1p   
     cos        frexp       modf             
     cosh       fsum                               

Also included are the following constants

============================ =============
Name                         value
============================ =============
``pi``,\ ``PI``              3.141592654
``e``,\ ``E``                2.718281828
``tau`` (``2*pi``)           6.283185307
``deg`` (``180/pi``)         57.29577951
``rad`` (``pi/180``)         0.01745329252
``phi`` (``(sqrt(5)+1 )/2``) 1.618033989
============================ =============

Note that all trigonometric functions assume that inputs are in radians.
See `Python’s "math" library <https://docs.Python.org/3/library/math.html>`__ for more
details. To compute based on degrees, convert first:

::

   { tan( radians(45) )}
   { tan( 45*rad)}
   { degrees( atan(1) )}
   { atan(1) * deg }

returns:

::

   1
   1
   45
   45

.. _`interfaces:other-functions`:

Other Functions
^^^^^^^^^^^^^^^

Other functions, modules, and packages that are part of the Python
standard library or that are available for import on the host system can
be used in templates. Use of NumPy to perform element-wise operations on
arrays was demonstrated in a previous section. The following example
illustrates using Python’s ``random`` module to draw a sample from a
uniform distribution:

::

   % from random import random,seed
   % seed(1)
   {A = random()}

Returns (may depend on the system)

::

   0.1343642441
   

.. _`interfaces:mappings`:

""""""""""""""""""""""""""""""""""""""
Parameter to Response Mapping Examples
""""""""""""""""""""""""""""""""""""""

In this section, interface mapping examples are presented through the
discussion of several parameters files and their corresponding results
files. A typical input file for 2 variables (:math:`n=2`) and 3
functions (:math:`m=3`) using :ref:`the standard parameters file format <variables:parameters:standard>`
is as follows:

.. code-block::

                       2 variables
   1.500000000000000e+00 cdv_1
   1.500000000000000e+00 cdv_2
                       3 functions
                       1 ASV_1
                       1 ASV_2
                       1 ASV_3
                       2 derivative_variables
                       1 DVV_1
                       2 DVV_2
                       0 analysis_components

where numerical values are associated with their tags within
value-tag constructs (i.e. the value on the left, the tag on the right,
separated by a space). The number of design variables (:math:`n`)
and the string “``variables``” are followed by the values of the design
variables and their tags, the number of functions (:math:`m`) and the
string “``functions``”, the active set vector (ASV) and its tags, the
number of derivative variables and the string
“``derivative_variables``”, the derivative variables vector (DVV) and
its tags, the number of analysis components and the string
“``analysis_components``”, and the analysis components array and its
tags. The descriptive tags for the variables are always present and they
are either the descriptors in the user’s variables specification, if
given there, or are default descriptors. The length of the active set
vector is equal to the number of functions (:math:`m`). In the case of
an optimization data set with an objective function and two nonlinear
constraints (three response functions total), the first ASV value is
associated with the objective function and the remaining two are
associated with the constraints (in whatever consistent constraint order
has been defined by the user). The DVV defines a subset of the variables
used for computing derivatives. Its identifiers are 1-based and
correspond to the full set of variables listed in the first array.
Finally, the analysis components pass additional strings from the user’s
``analysis_components`` specification in a Dakota input file through to
the simulator. They allow the development of simulation drivers that are
more flexible, by allowing them to be passed additional specifics at run
time, e.g., the names of model files such as a particular mesh to use.

For :ref:`the APREPRO format option <variables:parameters:aprepro>`,
the same set of data appears as follows:

.. code-block::

   { DAKOTA_VARS     =                      2 }
   { cdv_1           =  1.500000000000000e+00 }
   { cdv_2           =  1.500000000000000e+00 }
   { DAKOTA_FNS      =                      3 }
   { ASV_1           =                      1 }
   { ASV_2           =                      1 }
   { ASV_3           =                      1 }
   { DAKOTA_DER_VARS =                      2 }
   { DVV_1           =                      1 }
   { DVV_2           =                      2 }
   { DAKOTA_AN_COMPS =                      0 }

where the numerical values are associated with their tags within
“``{ tag = value }``” constructs.

The user-supplied simulation interface, comprised of a simulator program
or driver and (optionally) filter programs, is responsible for reading
the parameters file and creating a :ref:`results file that contains the
response data <responses:results>` requested in the ASV. Since the ASV
contains all ones in this case, the response file corresponding to the
above input file would contain values for the three functions:

.. code-block::

   1.250000000000000e-01 f
   1.500000000000000e+00 c1
   1.500000000000000e+00 c2

Since function tags are optional, the following would be equally
acceptable:

.. code-block::

   1.250000000000000e-01
   1.500000000000000e+00
   1.500000000000000e+00

For the same parameters with different ASV components,

.. code-block::

                       2 variables
   1.500000000000000e+00 cdv_1
   1.500000000000000e+00 cdv_2
                       3 functions
                       3 ASV_1
                       3 ASV_2
                       3 ASV_3
                       2 derivative_variables
                       1 DVV_1
                       2 DVV_2
                       0 analysis_components

the following response data is required:

.. code-block::

   1.250000000000000e-01 f
   1.500000000000000e+00 c1
   1.500000000000000e+00 c2
   [ 5.000000000000000e-01 5.000000000000000e-01 ]
   [ 3.000000000000000e+00 -5.000000000000000e-01 ]
   [ -5.000000000000000e-01 3.000000000000000e+00 ]

Here, we need not only the function values, but also each of their
gradients. The derivatives are computed with respect to ``cdv_1`` and
``cdv_2`` as indicated by the DVV values. Another modification to the
ASV components yields the following parameters file:

.. code-block::

                       2 variables
   1.500000000000000e+00 cdv_1
   1.500000000000000e+00 cdv_2
                       3 functions
                       2 ASV_1
                       0 ASV_2
                       2 ASV_3
                       2 derivative_variables
                       1 DVV_1
                       2 DVV_2
                       0 analysis_components

for which the following results file is needed:

.. code-block::

   [ 5.000000000000000e-01 5.000000000000000e-01 ]
   [ -5.000000000000000e-01 3.000000000000000e+00 ]

Here, we need gradients for functions ``f`` and ``c2``, but not for
``c1``, presumably since this constraint is inactive.

A full Newton optimizer might make the following request:

.. code-block::

                       2 variables
   1.500000000000000e+00 cdv_1
   1.500000000000000e+00 cdv_2
                       1 functions
                       7 ASV_1
                       2 derivative_variables
                       1 DVV_1
                       2 DVV_2
                       0 analysis_components

for which the following results file,

.. code-block::

   1.250000000000000e-01 f
   [ 5.000000000000000e-01 5.000000000000000e-01 ]
   [[ 3.000000000000000e+00 0.000000000000000e+00
      0.000000000000000e+00 3.000000000000000e+00 ]]

containing the objective function, its gradient vector, and its Hessian
matrix, is needed. Again, the derivatives (gradient vector and Hessian
matrix) are computed with respect to ``cdv_1`` and ``cdv_2`` as
indicated by the DVV values.

Lastly, a more advanced example could have multiple types of variables
present; in this example, 2 continuous design and 3 discrete design
range, 2 normal uncertain, and 3 continuous state and 2 discrete state
range variables. When a mixture of variable types is present, the
content of the DVV (and therefore the required length of gradient
vectors and Hessian matrices) :ref:`depends upon the type of study being
performed <responses:active>`. For a reliability analysis problem,
the uncertain variables are the active continuous variables and the
following parameters file would be typical:

.. code-block::

                      12 variables
   1.500000000000000e+00 cdv_1
   1.500000000000000e+00 cdv_2
                       2 ddriv_1
                       2 ddriv_2
                       2 ddriv_3
   5.000000000000000e+00 nuv_1
   5.000000000000000e+00 nuv_2
   3.500000000000000e+00 csv_1
   3.500000000000000e+00 csv_2
   3.500000000000000e+00 csv_3
                       4 dsriv_1
                       4 dsriv_2
                       3 functions
                       3 ASV_1
                       3 ASV_2
                       3 ASV_3
                       2 derivative_variables
                       6 DVV_1
                       7 DVV_2
                       2 analysis_components
               mesh1.exo AC_1
                 db1.xml AC_2

Gradients are requested with respect to variable entries 6 and 7, which
correspond to normal uncertain variables ``nuv_1`` and ``nuv_2``. The
following response data would be appropriate:

.. code-block::

   7.943125000000000e+02 f
   1.500000000000000e+00 c1
   1.500000000000000e+00 c2
   [ 2.560000000000000e+02 2.560000000000000e+02 ]
   [ 0.000000000000000e+00 0.000000000000000e+00 ]
   [ 0.000000000000000e+00 0.000000000000000e+00 ]

In a parameter study, however, no distinction is drawn between different
types of continuous variables, and derivatives would be needed with
respect to all continuous variables (:math:`n_{dvv}=7` for the
continuous design variables ``cdv_1`` and ``cdv_2``, the normal
uncertain variables ``nuv_1`` and ``nuv_2``, and the continuous state
variables ``csv_1``, ``csv_2`` and ``csv_3``). The parameters file would
appear as

.. code-block::

                      12 variables
   1.500000000000000e+00 cdv_1
   1.500000000000000e+00 cdv_2
                       2 ddriv_1
                       2 ddriv_2
                       2 ddriv_3
   5.000000000000000e+00 nuv_1
   5.000000000000000e+00 nuv_2
   3.500000000000000e+00 csv_1
   3.500000000000000e+00 csv_2
   3.500000000000000e+00 csv_3
                       4 dsriv_1
                       4 dsriv_2
                       3 functions
                       3 ASV_1
                       3 ASV_2
                       3 ASV_3
                       7 derivative_variables
                       1 DVV_1
                       2 DVV_2
                       6 DVV_3
                       7 DVV_4
                       8 DVV_5
                       9 DVV_6
                      10 DVV_7
                       2 analysis_components
               mesh1.exo AC_1
                 db1.xml AC_2

and the corresponding results would appear as

.. code-block::

   7.943125000000000e+02 f
   1.500000000000000e+00 c1
   1.500000000000000e+00 c2
   [  5.000000000000000e-01  5.000000000000000e-01  2.560000000000000e+02
      2.560000000000000e+02  6.250000000000000e+01  6.250000000000000e+01
      6.250000000000000e+01 ]
   [  3.000000000000000e+00 -5.000000000000000e-01  0.000000000000000e+00
      0.000000000000000e+00  0.000000000000000e+00  0.000000000000000e+00
      0.000000000000000e+00 ]
   [ -5.000000000000000e-01  3.000000000000000e+00  0.000000000000000e+00
      0.000000000000000e+00  0.000000000000000e+00  0.000000000000000e+00
      0.000000000000000e+00 ]
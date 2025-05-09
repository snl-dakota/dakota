.. _dakota_restart_utility:

""""""""""""""""""""""""""
The Dakota Restart Utility
""""""""""""""""""""""""""

The Dakota restart utility program provides a variety of facilities for managing restart files from
Dakota executions. The executable program name is ``dakota_restart_util`` and it has the following
options, as shown by the usage message returned when executing the utility without any options:

.. code-block::

   Usage:
     dakota_restart_util command <arg1> [<arg2> <arg3> ...] --options
       dakota_restart_util print <restart_file>
       dakota_restart_util to_neutral <restart_file> <neutral_file>
       dakota_restart_util from_neutral <neutral_file> <restart_file>
       dakota_restart_util to_tabular <restart_file> <text_file>
         [--custom_annotated [header] [eval_id] [interface_id]] 
         [--output_precision <int>]
       dakota_restart_util remove <double> <old_restart_file> <new_restart_file>
       dakota_restart_util remove_ids <int_1> ... <int_n> <old_restart_file> <new_restart_file>
       dakota_restart_util cat <restart_file_1> ... <restart_file_n> <new_restart_file>
   options:
     --help                       show dakota_restart_util help message
     --custom_annotated arg       tabular file options: header, eval_id, 
                                  interface_id
     --freeform                   tabular file: freeform format
     --output_precision arg (=10) set tabular output precision

Several of these functions involve format conversions. In particular, the binary format used
for restart files can be converted to ASCII text and printed to the screen, converted to and
from a neutral file format, or converted to a tabular format for importing into
3rd-party plotting programs. In addition, a restart file with corrupted data can be repaired by
value or id, and multiple restart files can be combined into a single database.

=============
Print Command
=============

The ``print`` option is useful to show contents of a restart file, since the binary format is not
convenient for direct inspection. The restart data is printed in full precision, so that (near-)exact
matching of points is possible for restarted runs or corrupted data removals. For example,
the following command...

.. code-block::

   dakota_restart_util print dakota.rst 

...results in output similar to the following (output taken from
the :ref:`Cylinder example <additional:cylinder>`):

.. code-block::

   ------------------------------------------
   Restart record    1  (evaluation id    1):
   ------------------------------------------
   Parameters:
                         1.8000000000000000e+00 intake_dia
                         1.0000000000000000e+00 flatness

   Active response data:
   Active set vector = { 3 3 3 3 }
                        -2.4355973813420619e+00 obj_fn
                        -4.7428486677140930e-01 nln_ineq_con_1
                        -4.5000000000000001e-01 nln_ineq_con_2
                         1.3971143170299741e-01 nln_ineq_con_3
    [ -4.3644298963447897e-01  1.4999999999999999e-01 ] obj_fn gradient
    [  1.3855136437818300e-01  0.0000000000000000e+00 ] nln_ineq_con_1 gradient
    [  0.0000000000000000e+00  1.4999999999999999e-01 ] nln_ineq_con_2 gradient
    [  0.0000000000000000e+00 -1.9485571585149869e-01 ] nln_ineq_con_3 gradient

   ------------------------------------------
   Restart record    2  (evaluation id    2):
   ------------------------------------------
   Parameters:
                         2.1640000000000001e+00 intake_dia
                         1.7169994018008317e+00 flatness

   Active response data:
   Active set vector = { 3 3 3 3 }
                        -2.4869127192988878e+00 obj_fn
                         6.9256958799989843e-01 nln_ineq_con_1
                        -3.4245008972987528e-01 nln_ineq_con_2
                         8.7142207937157910e-03 nln_ineq_con_3
    [ -4.3644298963447897e-01  1.4999999999999999e-01 ] obj_fn gradient
    [  2.9814239699997572e+01  0.0000000000000000e+00 ] nln_ineq_con_1 gradient
    [  0.0000000000000000e+00  1.4999999999999999e-01 ] nln_ineq_con_2 gradient
    [  0.0000000000000000e+00 -1.6998301774282701e-01 ] nln_ineq_con_3 gradient

   ...<snip>...

   Restart file processing completed: 11 evaluations retrieved.

===========================
To/From Neutral File Format
===========================

A Dakota restart file can be converted to a neutral file format using a command like the following:

.. code-block::

   dakota_restart_util to_neutral dakota.rst dakota.neu

which results in a report similar to the following:

.. code-block::

   Writing neutral file dakota.neu
   Restart file processing completed: 11 evaluations retrieved.

Similarly, a neutral file can be returned to binary format using a command like the following:

.. code-block::

   dakota_restart_util from_neutral dakota.neu dakota.rst

which results in a report similar to the following:

.. code-block::

   Reading neutral file dakota.neu
   Writing new restart file dakota.rst
   Neutral file processing completed: 11 evaluations retrieved.

The contents of the generated neutral file are similar to the following (from the first
two records for the :ref:`Cylinder example <additional:cylinder>`).

.. code-block::

   6 7 2 1.8000000000000000e+00 intake_dia 1.0000000000000000e+00 flatness 0 0 0 0
   NULL 4 2 1 0 3 3 3 3 1 2 obj_fn nln_ineq_con_1 nln_ineq_con_2 nln_ineq_con_3
     -2.4355973813420619e+00 -4.7428486677140930e-01 -4.5000000000000001e-01
      1.3971143170299741e-01 -4.3644298963447897e-01  1.4999999999999999e-01
      1.3855136437818300e-01  0.0000000000000000e+00  0.0000000000000000e+00
      1.4999999999999999e-01  0.0000000000000000e+00 -1.9485571585149869e-01 1
   6 7 2 2.1640000000000001e+00 intake_dia 1.7169994018008317e+00 flatness 0 0 0 0
   NULL 4 2 1 0 3 3 3 3 1 2 obj_fn nln_ineq_con_1 nln_ineq_con_2 nln_ineq_con_3
     -2.4869127192988878e+00 6.9256958799989843e-01 -3.4245008972987528e-01
      8.7142207937157910e-03 -4.3644298963447897e-01  1.4999999999999999e-01
      2.9814239699997572e+01  0.0000000000000000e+00  0.0000000000000000e+00
      1.4999999999999999e-01  0.0000000000000000e+00 -1.6998301774282701e-01 2

This format is not intended for direct viewing (``print`` should be used for this purpose). Rather,
the neutral file capability has been used in the past for managing portability of restart
data across platforms (recent use of more portable binary formats has largely eliminated this need)
or for advanced repair of restart records (in cases where the remove command was insufficient).

.. _`restart:utility:tabular`:

==============
Tabular Format
==============

Conversion of a binary restart file to a tabular format enables convenient import of this data
into 3rd-party post-processing tools such as Matlab, TECplot, Excel, etc. This facility is nearly
identical to the output activated by the :dakkw:`environment-tabular_data` keyword in the Dakota input
file specification, but with two important differences:

1. No function evaluations are suppressed as they are with :dakkw:`environment-tabular_data`
(i.e., any internal finite difference evaluations are included).
2. The conversion can be performed later, i.e., for Dakota runs executed previously.

An example command for converting a restart file to tabular format is:

.. code-block::

   dakota_restart_util to_tabular dakota.rst dakota.m

which results in a report similar to the following:

.. code-block::

   Writing tabular text file dakota.m
   Restart file processing completed: 10 evaluations tabulated.

The contents of the generated tabular file are similar to the following (from the
:ref:`gradient-based optimization textbook problem example <additional:textbook:examples:gradient2>`).
Note that while evaluations resulting from numerical derivative offsets would be reported
(as described above), derivatives returned as part of the evaluations are not reported (since 
they do not readily fit within a compact tabular format):

.. code-block::

   %eval_id interface             x1             x2         obj_fn nln_ineq_con_1 nln_ineq_con_2 
   1            NO_ID            0.9            1.1         0.0002           0.26           0.76 
   2            NO_ID        0.90009            1.1 0.0001996404857   0.2601620081       0.759955 
   3            NO_ID        0.89991            1.1 0.0002003604863   0.2598380081       0.760045 
   4            NO_ID            0.9        1.10011 0.0002004407265       0.259945   0.7602420121 
   5            NO_ID            0.9        1.09989 0.0001995607255       0.260055   0.7597580121 
   6            NO_ID     0.58256179   0.4772224441   0.1050555937   0.1007670171 -0.06353963386 
   7            NO_ID   0.5826200462   0.4772224441   0.1050386469   0.1008348962 -0.06356876195 
   8            NO_ID   0.5825035339   0.4772224441   0.1050725476   0.1006991449 -0.06351050577 
   9            NO_ID     0.58256179   0.4772701663   0.1050283245    0.100743156 -0.06349408333 
   10           NO_ID     0.58256179   0.4771747219   0.1050828704   0.1007908783 -0.06358517983 
   ...

Controlling tabular format
--------------------------

The command-line options ``--freeform`` and ``--custom_annotated`` give control of headers in the
resulting tabular file. Freeform will generate a tabular file with no leading row nor columns
(variable and response values only). Custom annotated format accepts any or all of the options:

- ``header``: include %-commented header row with labels
- ``eval_id``: include leading column with evaluation ID
- ``interface_id``: include leading column with interface ID

For example, to recover Dakota 6.0 tabular format, which contained a header row,
leading column with evaluation ID, but no interface ID:

.. code-block::

   dakota_restart_util to_tabular dakota.rst dakota.m --custom_annotated header eval_id

Resulting in

.. code-block::

   %eval_id             x1             x2         obj_fn nln_ineq_con_1 nln_ineq_con_2 
   1                   0.9            1.1         0.0002           0.26           0.76 
   2               0.90009            1.1 0.0001996404857   0.2601620081       0.759955 
   3               0.89991            1.1 0.0002003604863   0.2598380081       0.760045 
   ...

Finally, ``--output_precision integer`` will generate tabular output with the specified integer
digits of precision.

=======================================
Concatenation of Multiple Restart Files
=======================================

In some instances, it is useful to combine restart files into a single function
evaluation database. For example, when constructing a data fit surrogate model,
data from previous studies can be pulled in and reused to create a combined data set for the
surrogate fit. An example command for concatenating multiple restart files is:

.. code-block::

   dakota_restart_util cat dakota.rst.1 dakota.rst.2 dakota.rst.3 dakota.rst.all

which results in a report similar to the following:

.. code-block::

   Writing new restart file dakota.rst.all
   dakota.rst.1 processing completed: 10 evaluations retrieved.
   dakota.rst.2 processing completed: 110 evaluations retrieved.
   dakota.rst.3 processing completed: 65 evaluations retrieved.

The dakota.rst.all database now contains 185 evaluations and can be read in for use in
a subsequent Dakota study using the ``-read_restart`` option to the dakota executable.

=========================
Removal of Corrupted Data
=========================

On occasion, a simulation or computer system failure may cause a corruption of the Dakota restart file.
For example, a simulation crash may result in failure of a post-processor to retrieve meaningful data.
If 0's (or other erroneous data) are returned from the user's analysis_driver, then this bad data will
get recorded in the restart file. If there is a clear demarcation of where corruption initiated
(typical in a process with feedback, such as gradient-based optimization), then use of the ``-stop_restart``
option for the Dakota executable can be effective in continuing the study from the point immediately
prior to the introduction of bad data. If, however, there are interspersed corruptions throughout
the restart database (typical in a process without feedback, such as sampling), then the remove
and ``remove_ids`` options of dakota_restart_util can be useful.

An example of the command syntax for the remove option is:

.. code-block::

   dakota_restart_util remove 2.e-04 dakota.rst dakota.rst.repaired

which results in a report similar to the following:

.. code-block::

   Writing new restart file dakota.rst.repaired
   Restart repair completed: 65 evaluations retrieved, 2 removed, 63 saved.

where any evaluations in dakota.rst having an active response function value that matches ``2.e-04``
within machine precision are discarded when creating dakota.rst.repaired.

An example of the command syntax for the ``remove_ids`` option is:

.. code-block::

   dakota_restart_util remove_ids 12 15 23 44 57 dakota.rst dakota.rst.repaired

which results in a report similar to the following:

.. code-block::

   Writing new restart file dakota.rst.repaired
   Restart repair completed: 65 evaluations retrieved, 5 removed, 60 saved.

where evaluation ids 12, 15, 23, 44, and 57 have been discarded when creating dakota.rst.repaired. An
important detail is that, unlike the ``-stop_restart`` option which operates on restart record numbers,
the ``remove_ids`` option operates on evaluation ids. Thus, removal is not necessarily based on the order
of appearance in the restart file. This distinction is important when removing restart records for a run
that contained either asynchronous or duplicate evaluations, since the restart insertion order and evaluation
ids may not correspond in these cases (asynchronous evaluations have ids assigned in the order of job creation
but are inserted in the restart file in the order of job completion, and duplicate evaluations are not recorded
which introduces offsets between evaluation id and record number). This can also be important if removing
records from a concatenated restart file, since the same evaluation id could appear more than once. In this case,
all evaluation records with ids matching the ``remove_ids`` list will be removed.

If neither of these removal options is sufficient to handle a particular restart repair need, then
the fallback position is to resort to direct editing of a neutral file to perform the necessary modifications.

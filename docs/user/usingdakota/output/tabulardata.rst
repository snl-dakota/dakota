.. _`output:tabular`:

===================
Tabular Output Data
===================

In a number of contexts, Dakota can output information in a
whitespace-separated columnar data file, a tabular data file. The most
common usage, to capture the iteration history in a tabular file, is
enabled by including the :dakkw:`environment-tabular_data`
keyword in the specification (see :numref:`output:incont` for an example).

This output format facilitates the transfer of Dakota’s iteration history data to external
mathematical analysis and/or graphics plotting packages (e.g., MATLAB, TECplot, Excel, S-plus, Minitab).

.. note::

   The default file name for the top-level tabular output data is "**dakota_tabular.dat**",
   though you may wish to choose an alternate name. 

------------------
Rosenbrock Example
------------------

.. code-block::

    %eval_id   interface   x1     x2   response_fn_1
           1       NO_ID   -2     -2            3609
           2       NO_ID   -1.5   -2          1812.5
           3       NO_ID   -1     -2             904
           4       NO_ID   -0.5   -2           508.5


The first line contains the names of the variables and responses, as well as headers for the evaluation id and interface
columns:

.. code-block::

    %eval_id   interface   x1     x2   response_fn_1


The number of function evaluations will match the number of evaluations listed in the summary part of the output file for single
method approaches; the names of inputs and outputs will match the descriptors specified in the input file. The interface
column is useful when a Dakota input file contains more than one simulation interface. In this instance, there is only one, and
it has no id interface specified, so Dakota has supplied a default value of NO ID. This file is ideal for import into other
data analysis packages.

------------------------------
Container Optimization Example
------------------------------

Example tabular output
from the “container” optimization problem is shown in
:numref:`output:tabcont`. This :ref:`annotated tabular format <input:tabularformat>`
file contains the complete history of data requests from NPSOL (8
requests map into a total of 40 function evaluations when including the
central finite differencing). The first column is the data request
number, the second column is the interface ID (which is ``NO_ID`` if the
user does not specify a name for the interface), the third and fourth
columns are the design parameter values (labeled in the example as
“``H``” and “``D``”), the fifth column is the objective function
(labeled “``obj_fn``”), and the sixth column is the nonlinear equality
constraint (labeled “``nln_eq_con_1``”).

.. code-block::
   :name: output:tabcont
   :caption: Dakota’s tabular output file showing the iteration history of the “container” optimization problem.

   %eval_id interface             H              D         obj_fn   nln_eq_con_1 
          1     NO_ID           4.5            4.5    107.1314511     8.04440764 
          2     NO_ID   5.801246882    3.596476363    94.33737399    -4.59103645 
          3     NO_ID   5.197920019    3.923577479     97.7797214  -0.6780884711 
          4     NO_ID   4.932877133    4.044776216    98.28930566  -0.1410680284 
          5     NO_ID   4.989328733    4.026133158     98.4270019 -0.005324671422 
          6     NO_ID   4.987494493    4.027041977    98.43249058 -7.307058453e-06 
          7     NO_ID   4.987391669     4.02708372    98.43249809 -2.032538049e-08 
          8     NO_ID   4.987389423    4.027084627    98.43249812 -9.691802916e-12 

Any evaluations from Dakota’s internal finite differencing are
suppressed, to facilitate rapid plotting of the most critical data. This
suppression of lower level data is consistent with the data that is sent
:ref:`to the graphics windows <output:graphics>`. If this data suppression is
undesirable, :ref:`this section <restart:utility:tabular>`
describes an approach where every function evaluation, even the ones
from finite differencing, can be saved to a file in tabular format by
using the Dakota restart utility.

.. note::

   The second column labeled “:ref:`interface <interface>`” is new as of
   Dakota 6.1. It identifies which interface was used to map the variables
   to responses on each line of the tabular file (recall that the interface
   defines which simulation is being run though the :ref:`analysis driver <couplingtosimulations-main>`
   specification). Disambiguating the interface is important when using
   hybrid methods, multi-fidelity methods, or nested models. In more
   common, simpler analyses, users typically ignore the first two columns
   and only focus on the columns of inputs (variables) and outputs
   (responses). *To generate tabular output in Dakota 6.0 format, use the
   :ref:`custom-annotated format <input:tabularformat>`.*

.. note::

   As of Dakota 6.1, the tabular file will include columns
   for all of the variables (both active and inactive) present in a given
   interface. Previously, Dakota only wrote the “active” variables. Recall
   that some variables may be inactive if they are not operated on by a
   particular method (e.g. uncertain variables might not be active in an
   optimization, design variables may not be active in a sampling study).
   The order of the variables printed out will be in Dakota’s standard
   variable ordering, which is indicated by the input specification order,
   and summarized in the Dakota Reference Manual.
.. _`interfaces:components`:

Simulation Interface Components
-------------------------------

:numref:`interfaces:bbinterfacecomp` is an extension of
:numref:`coupling:figure01` that adds details of
the components that make up each of the simulation interfaces (system
call, fork, and direct). These components include an ``input_filter``
(“IFilter”), one or more ``analysis_drivers`` (“Analysis Code/Driver”),
and an ``output_filter`` (“OFilter”). The input and output filters
provide optional facilities for managing simulation pre- and
post-processing, respectively. More specifically, the input filter can
be used to insert the Dakota parameters into the input files required by
the simulator program, and the output filter can be used to recover the
raw data from the simulation results and compute the desired response
data set. If there is a single analysis code, it is often convenient to
combine these pre- and post-processing functions into a single
simulation driver script, and the separate input and output filter
facilities are rarely used in this case. If there are multiple analysis
drivers, however, the input and output filter facilities provide a
convenient means for managing *non-repeated* portions of the pre- and
post-processing for multiple analyses. That is, pre- and post-processing
tasks that must be performed for each analysis can be performed within
the individual analysis drivers, and shared pre- and post-processing
tasks that are only performed once for the set of analyses can be
performed within the input and output filters. 

.. figure:: img/dakota_components.png
   :alt: Components of the simulation interface
   :name: interfaces:bbinterfacecomp
   :width: 400
   :align: center   

   Components of the simulation interface

When spawning function evaluations using system calls or forks, Dakota
must communicate parameter and response data with the analysis drivers
and filters through use of the file system. This is accomplished by
passing the names of the parameters and results files on the command
line when executing an analysis driver or filter. The input filter or
analysis driver read data from the parameters file and the output filter
or analysis driver write the appropriate data to the responses file.
While not essential when the file names are fixed, the file names must
be retrieved from the command line when Dakota is changing the file
names from one function evaluation to the next (i.e., using temporary
files or root names tagged with numerical identifiers). In the case of a
UNIX C-shell script, the two command line arguments are retrieved using
``$argv[1]`` and ``$argv[2]`` (see :cite:p:`And86`).
Similarly, Bourne shell scripts retrieve the two command line arguments
using ``$1`` and ``$2``, and Perl scripts retrieve the two command line
arguments using ``@ARGV[0]`` and ``@ARGV[1]``. In the case of a C or C++
program, command line arguments are retrieved using ``argc`` (argument
count) and ``argv`` (argument vector) :cite:p:`Ker88`, and for
Fortran 77, the ``iargc`` function returns the argument count and the
``getarg`` subroutine returns command line arguments.

.. _`interfaces:components:single1`:

Single analysis driver without filters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If a single ``analysis_driver`` is selected in the interface
specification and filters are not needed (as indicated by omission of
the ``input_filter`` and ``output_filter`` specifications), then only
one process will appear in the execution syntax of the simulation
interface. An example of this syntax in the system call case is:

.. code-block::

   driver params.in results.out

where ``driver`` is the user-specified analysis driver and ``params.in`` and ``results.out`` are the names of the
parameters and results files, respectively, passed on the command line.
In this case, the user need not retrieve the command line arguments
since the same file names will be used each time.

For the same mapping, the fork simulation interface echoes the following
syntax:

.. code-block::

   blocking fork: driver params.in results.out

for which only a single blocking fork is needed to perform the
evaluation.

Executing the same mapping with the direct simulation interface results
in an echo of the following syntax:

.. code-block::

   Direct function: invoking driver

where this analysis driver must be linked as a function within Dakota’s
direct interface (see the :ref:`"Developing a Direct Simulation Interface" section <advint:direct>`).
Note that no parameter or response files are involved, since such values
are passed directly through the function argument lists.

Both the system call and fork interfaces support asynchronous
operations. The asynchronous system call execution syntax involves
executing the system call in the background:

.. code-block::

   driver params.in.1 results.out.1 &

and the asynchronous fork execution syntax involves use of a nonblocking
fork:

.. code-block::

   nonblocking fork: driver params.in.1 results.out.1

where :ref:`file tagging <interfaces:file:tagging1>`)
has been user-specified in both cases to prevent conflicts between
concurrent analysis drivers. In these cases, the user must retrieve the
command line arguments since the file names change on each evaluation.

.. note::

   Execution of the direct interface must currently be performed
   synchronously since multithreading is not yet supported. See
   the :ref:`section on direct function synchronization <parallel:SLP:local:direct>` for more information.

.. _`interfaces:components:single2`:

Single analysis driver with filters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When filters are used, the syntax of the system call that Dakota
performs is:

.. code-block::

   ifilter params.in results.out; driver params.in results.out;
        ofilter params.in results.out

in which the input filter (``ifilter``), analysis driver (``driver``), and output filter (``ofilter``)
processes are combined into a single system call through the use of
semi-colons (see :cite:p:`And86`). All three portions are
passed the names of the parameters and results files on the command
line.

For the same mapping, the fork simulation interface echoes the following
syntax:

.. code-block::

   blocking fork: ifilter params.in results.out;
        driver params.in results.out; ofilter params.in results.out

where a series of three blocking forks is used to perform the
evaluation.

Executing the same mapping with the direct simulation interface results
in an echo of the following syntax:

.. code-block::

   Direct function: invoking { ifilter driver ofilter }

where each of the three components must be linked as a function within
Dakota’s direct interface. Since asynchronous operations are not yet
supported, execution simply involves invocation of each of the three
linked functions in succession. Again, no files are involved since
parameter and response data are passed directly through the function
argument lists.

Asynchronous executions would appear as follows for the system call
interface:

.. code-block::

   (ifilter params.in.1 results.out.1; driver params.in.1 results.out.1;
        ofilter params.in.1 results.out.1) &

and, for the fork interface, as:

.. code-block::

   nonblocking fork: ifilter params.in.1 results.out.1;
        driver params.in.1 results.out.1; ofilter params.in.1 results.out.1

where file tagging of evaluations has again been user-specified in both
cases. For the system call simulation interface, use of parentheses and
semi-colons to bind the three processes into a single system call
simplifies asynchronous process management compared to an approach using
separate system calls. The fork simulation interface, on the other hand,
does not rely on parentheses and accomplishes asynchronous operations by
first forking an intermediate process. This intermediate process is then
reforked for the execution of the input filter, analysis driver, and
output filter. The intermediate process can be blocking or nonblocking
(nonblocking in this case), and the second level of forks can be
blocking or nonblocking (blocking in this case). The fact that forks can
be reforked multiple times using either blocking or nonblocking
approaches provides the enhanced flexibility to support :ref:`a variety of
local parallelism approaches <parallel>`.

.. _`interfaces:components:multiple1`:

Multiple analysis drivers without filters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If a list of ``analysis_drivers`` is specified and filters are not
needed (i.e., neither ``input_filter`` nor ``output_filter`` appears),
then the system call syntax would appear as:

.. code-block::

   driver1 params.in results.out.1; driver2 params.in results.out.2;
        driver3 params.in results.out.3

where ``driver1``, ``driver2``, and ``driver3`` are the user-specified analysis
drivers and ``params.in`` and ``results.out`` are the
user-selected names of the parameters and results files. Note that the
results files for the different analysis drivers have been automatically
tagged to prevent overwriting. This :ref:`automatic tagging of analyses <interfaces:file:tagging2>`
is a separate operation from :ref:`user-selected tagging of evaluations <interfaces:file:tagging1>`.

For the same mapping, the fork simulation interface echoes the following
syntax:

.. code-block::

   blocking fork: driver1 params.in results.out.1;
        driver2 params.in results.out.2; driver3 params.in results.out.3

for which a series of three blocking forks is needed (no reforking of an
intermediate process is required).

Executing the same mapping with the direct simulation interface results
in an echo of the following syntax:

.. code-block::

   Direct function: invoking { driver1 driver2 driver3 }

where, again, each of these components must be linked within Dakota’s
direct interface and no files are involved for parameter and response
data transfer.

Both the system call and fork interfaces support asynchronous function
evaluations. The asynchronous system call execution syntax would be
reported as

.. code-block::

   (driver1 params.in.1 results.out.1.1; driver2 params.in.1 results.out.1.2;
        driver3 params.in.1 results.out.1.3) &

and the nonblocking fork execution syntax would be reported as

.. code-block::

   nonblocking fork: driver1 params.in.1 results.out.1.1;
        driver2 params.in.1 results.out.1.2; driver3 params.in.1 results.out.1.3

where, in both cases, file tagging of evaluations has been
user-specified to prevent conflicts between concurrent analysis drivers
and file tagging of the results files for multiple analyses is
automatically used. In the fork interface case, an intermediate process
is forked to allow a non-blocking function evaluation, and this
intermediate process is then reforked for the execution of each of the
analysis drivers.

.. _`interfaces:components:multiple2`:

Multiple analysis drivers with filters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Finally, when combining filters with multiple ``analysis_drivers``, the
syntax of the system call that Dakota performs is:

.. code-block::

   ifilter params.in.1 results.out.1;
        driver1 params.in.1 results.out.1.1;
        driver2 params.in.1 results.out.1.2;
        driver3 params.in.1 results.out.1.3;
        ofilter params.in.1 results.out.1

in which all processes have again been combined into a single system
call through the use of semi-colons and parentheses. Note that the
secondary file tagging for the results files is only used for the
analysis drivers and not for the filters. This is consistent with the
filters’ defined purpose of managing the non-repeated portions of
analysis pre- and post-processing (e.g., :ref:`overlay of response results
from individual analyses <interfaces:file:tagging2>`).

For the same mapping, the fork simulation interface echoes the following
syntax:

.. code-block::

   blocking fork: ifilter params.in.1 results.out.1;
        driver1 params.in.1 results.out.1.1;
        driver2 params.in.1 results.out.1.2;
        driver3 params.in.1 results.out.1.3;
        ofilter params.in.1 results.out.1

for which a series of five blocking forks is used (no reforking of an
intermediate process is required).

Executing the same mapping with the direct simulation interface results
in an echo of the following syntax:

.. code-block::

   Direct function: invoking { ifilter driver1 driver2 driver3 ofilter }

where each of these components must be linked as a function within
Dakota’s direct interface. Since asynchronous operations are not
supported, execution simply involves invocation of each of the five
linked functions in succession. Again, no files are involved for
parameter and response data transfer since this data is passed directly
through the function argument lists.

Asynchronous executions would appear as follows for the system call
interface:

.. code-block::

   (ifilter params.in.1 results.out.1;
        driver1 params.in.1 results.out.1.1;
        driver2 params.in.1 results.out.1.2;
        driver3 params.in.1 results.out.1.3;
        ofilter params.in.1 results.out.1) &

and for the fork interface:

.. code-block::

   nonblocking fork: ifilter params.in.1 results.out.1;
        driver1 params.in.1 results.out.1.1;
        driver2 params.in.1 results.out.1.2;
        driver3 params.in.1 results.out.1.3;
        ofilter params.in.1 results.out.1

where, again, user-selected file tagging of evaluations is combined with
automatic file tagging of analyses. In the fork interface case, an
intermediate process is forked to allow a non-blocking function
evaluation, and this intermediate process is then reforked for the
execution of the input filter, each of the analysis drivers, and the
output filter.

A complete example of these filters and multi-part drivers can be found
in ``dakota/share/dakota/test/dakota_3pc/dakota_3pc.in``.

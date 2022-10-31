.. _`interfaces:sim`:

"""""""""""""""""""""
Simulation Interfaces
"""""""""""""""""""""

The invocation of a simulation code is performed using either system
calls or forks or via direct linkage. In the system call and fork cases,
a separate process is created for the simulation and communication
between Dakota and the simulation occurs through parameter and response
files. For system call and fork interfaces, the interface section must
specify the details of this data transfer. In the direct case, a
separate process is not created and communication occurs in memory
through a prescribed API.

The following sub-sections provide information on the simulation interfacing approaches:

.. _`interfaces:fork`:

Fork Simulation Interface
~~~~~~~~~~~~~~~~~~~~~~~~~

The :dakkw:`interface-fork` simulation interface is the most common
method for interfacing Dakota with an external simulation model.

The fork simulation interface uses the ``fork``, ``exec``, and ``wait``
families of functions to manage simulation codes or simulation drivers.
Calls to ``fork`` or ``vfork`` create a copy of the Dakota process,
``execvp`` replaces this copy with the simulation code or driver process,
and then Dakota uses the ``wait`` or ``waitpid`` functions to wait for
completion of the new process.

.. note::

   In a native Windows version of Dakota, similar Win32 functions, such
   as ``_spawnvp()``, are used instead.

.. note::

   Transfer of variables and response data between Dakota and the simulator
   code or driver occurs through the file system in exactly the same manner
   as for the :ref:`system call interface <interfaces:system>`.
   
An example of a fork interface specification follows:

.. code::

   interface
       fork
           input_filter    = 'test_3pc_if'
           output_filter   = 'test_3pc_of'
           analysis_driver = 'test_3pc_ac'
           parameters_file = 'tb.in'
           results_file    = 'tb.out'
           file_tag
		   
**Further Reading**

- More detailed examples of using the fork call interface are provided in :ref:`the section below on building a black-box interface to a simulation code <interfaces:building>`.
- This :ref:`compiled Rosenbrock example <user-supplied-sim-code-case1>` may also be instructive for understanding the functionality of the ``fork`` interface.
- Information on asynchronous usage of the fork call interface is provided in :ref:`this section <parallel:SLP:local:fork>`.

.. _`interfaces:direct`:

Direct Simulation Interface
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The direct interface may be used to invoke simulations that are linked
into the Dakota executable. This interface eliminates overhead from
process creation and file I/O and can simplify operations on massively
parallel computers. These advantages are balanced with the practicality
of converting an existing simulation code into a library with a
subroutine interface. Sandia codes for structural dynamics (Salinas),
computational fluid dynamics (Sage), and circuit simulation (Xyce) and
external codes such as Phoenix Integration’s ModelCenter framework and
The Mathworks’ Matlab have been linked in this way, and a direct
interface to Sandia’s SIERRA multiphysics framework is under
development. In the latter case, the additional effort is particularly
justified since SIERRA unifies an entire suite of physics codes.

.. note::

   The "sandwich implementation" of combining a direct interface plug-in
   with Dakota’s library mode is discussed :ref:`here <interfacing_with_dakota_as_library>`.

In addition to direct linking with simulation codes, the direct
interface also provides access to internal polynomial test functions
that are used for algorithm performance and regression testing. The
following test functions are available: ``cantilever``, ``cyl_head``,
``log_ratio``, ``rosenbrock``, ``short_column``, and ``text_book``
(including ``text_book1``, ``text_book2``, ``text_book3``, and
``text_book_ouu``).

While these functions are also available as external
programs in the ``dakota/share/dakota/test`` directory, maintaining internally
linked versions allows more rapid testing. See :ref:`the Additional Examples page <additional>` for
additional information on several of these test problems.

An example input specification for a direct interface follows:

.. code::

   interface
       direct
           analysis_driver = 'rosenbrock'

**Further Reading**

- Additional specification examples are provided in the :ref:`"Getting Started" tutorial <examples-gettingstarted-main>`.
- Additional information on asynchronous usage of the direct function interface is provided in :ref:`this section <parallel:SLP:local:direct>`.
- Guidance for usage of some particular direct simulation interfaces is in :ref:`the section on connecting existing direct interfaces to external simulators. <advint:existingdirect>`
- Details of adding a simulation code to the direct interface are provided in :ref:`this section <advint:direct>`.

.. _`interfaces:system`:

System Call Simulation Interface
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. warning::

   Users are strongly encouraged to use the fork simulation interface if
   possible, though the system interface is still supported for portability
   and backward compatibility.
   
The system call approach invokes a simulation code or simulation driver by
using the :dakkw:`interface-system` function from the standard C library :cite:p:`Ker88`.

In this approach, the system call creates a new process that communicates with Dakota
through parameter and response files. The system call approach allows
the simulation to be initiated via its standard invocation procedure (as
a “black box”) and then coordinated with a variety of tools for pre- and
post-processing. This approach has been widely used in previous
studies :cite:p:`Eld96a,Eld96b,Eld98b`. The system call
approach involves more process creation and file I/O overhead than the
direct function approach, but this extra overhead is usually
insignificant compared with the cost of a simulation. An example of a
system call interface specification follows:

.. code::

   interface
       system
           analysis_driver = 'text_book'
           parameters_file = 'text_book.in'
           results_file    = 'text_book.out'
           file_tag
		   file_save

Information on asynchronous usage of the system interface is provided in
the :ref`section on system call synchronization <parallel:SLP:local:system>`.

.. _`interfaces:syntax`:

Syntax for Filter and Driver Strings
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Dakota’s default behavior is to construct input filter, analysis driver,
and output filter commands by appending the names of the parameters file
and results file for the evaluation/analysis to the user-provided
``input_filter``, ``output_filter``, and ``analysis_drivers`` strings.
After adding its working directory to the ``PATH``, Dakota executes
these commands in its working directory or, if the ``work_directory``
keyword group is present, in a work directory.

Filter and driver strings may contain absolute or relative path
information and whitespace; Dakota will pass them through without
modification.

Quotes are also permitted with the restriction that if double-quotes (")
are used to enclose the driver or filter string as a whole, then only
single quotes (’) are allowed within it, and vice versa. The input
filter string ``’dprepro –var "foo=1"’`` works, as does
``"dprepro –var ’foo=1’"``, but not ``"dprepro –var "foo=1""``.

In some situations, users may not wish Dakota to append the names of the
parameters or results files to filter and driver strings. The
``verbatim`` keyword prevents this behavior, and causes Dakota to
execute filter and driver strings "as is".

Beginning with version 6.10, Dakota will substitute the tokens
``{PARAMETERS}`` and ``{RESULTS}`` in driver and filter strings with the
names of the parameters and results files for that analysis/evaluation
just prior to execution.

For example, if an ``interface`` block in the input file included:

::

     input_filter 'preprocess {PARAMETERS}'
     analysis_drivers 'run_sim.sh'
     output_filter 'postprocess {RESULTS}'
     verbatim

Then, the input filter ``preprocess`` would be run with only the
parameters file as a command line argument, the analysis driver
``run_sim.sh`` would receive no command line arguments, and the output
filter ``postprocess`` would receive only the results file name.

The combination of ``verbatim`` and substitution provide users with
considerable flexibility in specifying the form of filter and driver
commands.

.. _`interfaces:which`:

Fork or System Call: Which to Use?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The primary operational difference between the fork and system call
simulation interfaces is that, in the fork interface, the
``fork``/``exec`` functions return a process identifier that the
``wait``/``waitpid`` functions can use to detect the completion of a
simulation for either synchronous or asynchronous operations. The system
call simulation interface, on the other hand, must use a response file
detection scheme for this purpose in the asynchronous case. Thus, an
important advantage of the fork interface over the system call interface
is that it avoids the potential of a file race condition when employing
asynchronous local parallelism (refer to
:ref:`Asynchronous Local Parallelism <parallel:SLP:local>`). This condition
can occur when the responses file has been created but the writing of
the response data set to this file has not been completed (see
:ref:`System Call Synchronization <parallel:SLP:local:system>`).
While significant care has been taken to manage this file race condition
in the system call case, the fork interface still has the potential to
be more robust when performing function evaluations asynchronously.

Another advantage of the fork interface is that it has additional
asynchronous capabilities when a function evaluation involves multiple
analyses. As shown in :numref:`parallel:table01`,
the fork interface supports asynchronous local and hybrid parallelism
modes for managing concurrent analyses within function evaluations,
whereas the system call interface does not. These additional
capabilities again stem from the ability to track child processes by
their process identifiers.

The only disadvantage to the fork interface compared with the system
interface is that the ``fork``/``exec``/``wait`` functions are not part
of the standard C library, whereas the ``system`` function is. As a
result, support for implementations of the ``fork``/``exec``/``wait``
functions can vary from platform to platform. At one time, these
commands were not available on some of Sandia’s massively parallel
computers. However, in the more mainstream UNIX environments,
availability of ``fork``/``exec``/``wait`` should not be an issue.

In summary, the system call interface has been a workhorse for many
years and is well tested and proven, but the fork interface supports
additional capabilities and is recommended when managing asynchronous
simulation code executions. Having both interfaces available has proven
to be useful on a number of occasions and they will both continue to be
supported for the foreseeable future.
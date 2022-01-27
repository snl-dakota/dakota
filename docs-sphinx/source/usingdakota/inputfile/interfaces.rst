Interfaces
==========

.. _`interfaces:overview`:

Overview
--------

The ``interface`` specification in a Dakota input file controls details
of function evaluations. The mechanisms currently in place for function
evaluations involve interfacing with one or more computational
simulation codes, computing algebraic mappings (refer to
Section `[advint:algebraic] <#advint:algebraic>`__), or a combination of
the two.

This chapter will focus on mechanisms for simulation code invocation,
starting with interface types in Section `1.2 <#interfaces:sim>`__ and
followed by a guide to constructing simulation-based interfaces in
Section `1.3 <#interfaces:building>`__. This chapter also provides an
overview of simulation interface components, covers issues relating to
file management, and presents a number of example data mappings.

For a detailed description of interface specification syntax, refer to
the interface commands chapter in the Dakota Reference
Manual :raw-latex:`\cite{RefMan}`.

.. _`interfaces:sim`:

Simulation Interfaces
---------------------

The invocation of a simulation code is performed using either system
calls or forks or via direct linkage. In the system call and fork cases,
a separate process is created for the simulation and communication
between Dakota and the simulation occurs through parameter and response
files. For system call and fork interfaces, the interface section must
specify the details of this data transfer. In the direct case, a
separate process is not created and communication occurs in memory
through a prescribed API. Sections `1.2.1 <#interfaces:direct>`__
through `1.2.5 <#interfaces:which>`__ provide information on the
simulation interfacing approaches.

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
justified since SIERRA unifies an entire suite of physics codes. [*Note:
the “sandwich implementation” of combining a direct interface plug-in
with Dakota’s library mode is discussed in the Dakota Developers
Manual :raw-latex:`\cite{DevMan}`*].

In addition to direct linking with simulation codes, the direct
interface also provides access to internal polynomial test functions
that are used for algorithm performance and regression testing. The
following test functions are available: ``cantilever``, ``cyl_head``,
``log_ratio``, ``rosenbrock``, ``short_column``, and ``text_book``
(including ``text_book1``, ``text_book2``, ``text_book3``, and
``text_book_ouu``). While these functions are also available as external
programs in the directory, maintaining internally linked versions allows
more rapid testing. See Chapter `[additional] <#additional>`__ for
additional information on several of these test problems. An example
input specification for a direct interface follows:

.. container:: small

   ::

          interface,
                  direct
                    analysis_driver = 'rosenbrock'

Additional specification examples are provided in
Section `[tutorial:examples] <#tutorial:examples>`__ and additional
information on asynchronous usage of the direct function interface is
provided in
Section `[parallel:SLP:local:direct] <#parallel:SLP:local:direct>`__.
Guidance for usage of some particular direct simulation interfaces is in
Section `[advint:existingdirect] <#advint:existingdirect>`__ and the
details of adding a simulation code to the direct interface are provided
in Section `[advint:direct] <#advint:direct>`__.

.. _`interfaces:system`:

System Call Simulation Interface
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Users are strongly encouraged to use the fork simulation interface if
possible, though the system interface is still supported for portability
and backward compatibility.** The system call approach invokes a
simulation code or simulation driver by using the ``system`` function
from the standard C library :raw-latex:`\cite{Ker88}`. In this approach,
the system call creates a new process that communicates with Dakota
through parameter and response files. The system call approach allows
the simulation to be initiated via its standard invocation procedure (as
a “black box”) and then coordinated with a variety of tools for pre- and
post-processing. This approach has been widely used in previous
studies :raw-latex:`\cite{Eld96a,Eld96b,Eld98b}`. The system call
approach involves more process creation and file I/O overhead than the
direct function approach, but this extra overhead is usually
insignificant compared with the cost of a simulation. An example of a
system call interface specification follows:

.. container:: small

   ::

          interface,
                  system
                    analysis_driver = 'text_book'
                    parameters_file = 'text_book.in'
                    results_file    = 'text_book.out'
                    file_tag file_save

Information on asynchronous usage of the system interface is provided in
Section `[parallel:SLP:local:system] <#parallel:SLP:local:system>`__.

.. _`interfaces:fork`:

Fork Simulation Interface
~~~~~~~~~~~~~~~~~~~~~~~~~

The fork simulation interface uses the ``fork``, ``exec``, and ``wait``
families of functions to manage simulation codes or simulation drivers.
(In a native MS Windows version of Dakota, similar Win32 functions, such
as ``_spawnvp()``, are used instead.) Calls to ``fork`` or ``vfork``
create a copy of the Dakota process, ``execvp`` replaces this copy with
the simulation code or driver process, and then Dakota uses the ``wait``
or ``waitpid`` functions to wait for completion of the new process.
Transfer of variables and response data between Dakota and the simulator
code or driver occurs through the file system in exactly the same manner
as for the system call interface. An example of a fork interface
specification follows:

.. container:: small

   ::

          interface,
                  fork
                    input_filter    = 'test_3pc_if'
                    output_filter   = 'test_3pc_of'
                    analysis_driver = 'test_3pc_ac'
                    parameters_file = 'tb.in'
                    results_file    = 'tb.out'
                    file_tag

More detailed examples of using the fork call interface are provided in
Section `[tutorial:examples:user_supply:optimization1] <#tutorial:examples:user_supply:optimization1>`__
and in Section `1.3 <#interfaces:building>`__, and information on
asynchronous usage of the fork call interface is provided in
Section `[parallel:SLP:local:fork] <#parallel:SLP:local:fork>`__.

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
Section `[parallel:SLP:local] <#parallel:SLP:local>`__). This condition
can occur when the responses file has been created but the writing of
the response data set to this file has not been completed (see
Section `[parallel:SLP:local:system] <#parallel:SLP:local:system>`__).
While significant care has been taken to manage this file race condition
in the system call case, the fork interface still has the potential to
be more robust when performing function evaluations asynchronously.

Another advantage of the fork interface is that it has additional
asynchronous capabilities when a function evaluation involves multiple
analyses. As shown in Table `[parallel:table01] <#parallel:table01>`__,
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

.. _`interfaces:building`:

Building a Black-Box Interface to a Simulation Code
---------------------------------------------------

To interface a simulation code to Dakota using one of the black-box
interfaces (system call or fork), pre- and post-processing functionality
typically needs to be supplied (or developed) in order to transfer the
parameters from Dakota to the simulator input file and to extract the
response values of interest from the simulator’s output file for return
to Dakota (see Figures `[intro:bbinterface] <#intro:bbinterface>`__
and `1.1 <#interfaces:bbinterfacecomp>`__). This is often managed
through the use of scripting languages, such as
C-shell :raw-latex:`\cite{And86}`, Bourne
shell :raw-latex:`\cite{Bli96}`, Perl :raw-latex:`\cite{Wal96}`, or
Python :raw-latex:`\cite{Mar03}`. While these are common and convenient
choices for simulation drivers/filters, it is important to recognize
that any executable file can be used. If the user prefers, the desired
pre- and post-processing functionality may also be compiled or
interpreted from any number of programming languages (C, C++, F77, F95,
JAVA, Basic, etc.).

In the directory, a simple example uses the Rosenbrock test function as
a mock engineering simulation code. Several scripts have been included
to demonstrate ways to accomplish the pre- and post-processing needs.
Actual simulation codes will, of course, have different pre- and
post-processing requirements, and as such, this example serves only to
demonstrate the issues associated with interfacing a simulator.
Modifications will almost surely be required for new applications.

.. _`interfaces:generic`:

Generic Script Interface Files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The directory contains four important files: (the Dakota input file),
(the simulation driver script), (a template simulation input file), and
(the Rosenbrock simulator).

The file specifies the study that Dakota will perform and, in the
interface section, describes the components to be used in performing
function evaluations. In particular, it identifies as its
``analysis_driver``, as shown in
Figure `[advint:figure01] <#advint:figure01>`__.

.. container:: bigbox

   .. container:: small

The listed in Figure `[advint:figure02] <#advint:figure02>`__ is a short
driver shell script that Dakota executes to perform each function
evaluation. The names of the parameters and results files are passed to
the script on its command line; they are referenced in the script by
``$1`` and ``$2``, respectively. The is divided into three parts:
pre-processing, analysis, and post-processing.

.. container:: bigbox

   .. container:: small

In the pre-processing portion, the uses , a template processing utility,
to extract the current variable values from a parameters file (``$1``)
and combine them with the simulator template input file () to create a
new input file () for the simulator. Internal to Sandia, the APREPRO
utility is often used for this purpose. For external sites where APREPRO
is not available, ``dprepro`` is an alternative with many of the
capabilities of APREPRO that is specifically tailored for use with
Dakota and is distributed with it (in , or in a binary distribution).
Dakota also provides a second, more general-purpose template processing
tool named ``pyprepro``, which is as a Python-based alternative to
APREPRO. This pair of tools, which permit not only parameter
substitution, but execution of arbitrary Python scripting within
templates, is extensively documented in
Section `1.9 <#interfaces:dprepro-and-pyprepro>`__.

Other preprocessing tools of potential interest are the BPREPRO utility
(see :raw-latex:`\cite{WalXX}`), and at Lockheed Martin sites, the
JPrePost utility, a JAVA pre- and
post-processor :raw-latex:`\cite{Fla}`. The ``dprepro`` script will be
used here for simplicity of discussion. It can use either Dakota’s
``aprepro`` parameters file format (see
Section `[variables:parameters:aprepro] <#variables:parameters:aprepro>`__)
or Dakota’s standard format (see
Section `[variables:parameters:standard] <#variables:parameters:standard>`__),
so either option may be selected in the interface section of the Dakota
input file. The file listed in
Figure `[advint:figure04] <#advint:figure04>`__ is a template simulation
input file which contains targets for the incoming variable values,
identified by the strings “``{x1}``” and “``{x2}``”. These identifiers
match the variable descriptors specified in . The template input file is
contrived as Rosenbrock has nothing to do with finite element analysis;
it only mimics a finite element code to demonstrate the simulator
template process. The ``dprepro`` script will search the simulator
template input file for fields marked with curly brackets and then
create a new file () by replacing these targets with the corresponding
numerical values for the variables. As shown in , the names for the
Dakota parameters file (``$1``), template file (), and generated input
file () must be specified in the ``dprepro`` command line arguments.

.. container:: bigbox

   .. container:: small

The second part of the script executes the simulator. The input and
output file names, and , respectively, are hard-coded into the. When the
simulator is executed, the values for ``x1`` and ``x2`` are read in from
, the Rosenbrock function is evaluated, and the function value is
written out to .

The third part performs the post-processing and writes the response
results to a file for Dakota to read. Using the UNIX “``grep``” utility,
the particular response values of interest are extracted from the raw
simulator output and saved to a temporary file (). When complete, this
file is renamed ``$2``, which in this example is always . Note that
moving or renaming the completed results file avoids any problems with
read race conditions (see
Section `[parallel:SLP:local:system] <#parallel:SLP:local:system>`__).

Because the Dakota input file
(Figure `[advint:figure01] <#advint:figure01>`__) specifies
``work_directory`` and ``directory_tag`` in its interface section, each
invocation of wakes up in its own temporary directory, which Dakota has
populated with the contents of directory . Having a separate directory
for each invocation of simplifies the script when the Dakota input file
specifies ``asynchronous`` (so several instances of might run
simultaneously), as fixed names such as , , and can be used for
intermediate files. If neither ``asynchronous`` nor ``file_tag`` is
specified, and if there is no need (e.g., for debugging) to retain
intermediate files having fixed names, then ``directory_tag`` offers no
benefit and can be omitted. An alternative to ``directory_tag`` is to
proceed as earlier versions of this chapter — prior to Dakota 5.0’s
introduction of ``work_directory`` — recommended: add two more steps to
the , an initial one to create a temporary directory explicitly and copy
to it if needed, and a final step to remove the temporary directory and
any files in it.

When ``work_directory`` is specified, Dakota adjusts the ``$PATH`` seen
by so that simple program names (i.e., names not containing a slash)
that are visible in Dakota’s directory will also be visible in the work
directory. Relative path names — involving an intermediate slash but not
an initial one, such as or — will only be visible in the work directory
if a ``link_files`` or ``copy_files`` specification (see
§\ `1.5.5 <#interfaces:workdir>`__) has made them visible there.

As an example of the data flow on a particular function evaluation,
consider evaluation 60. The parameters file for this evaluation consists
of:

.. container:: small

   ::

                                                2 variables
                            4.664752623441543e-01 x1
                            2.256400864298234e-01 x2
                                                1 functions
                                                3 ASV_1:obj_fn
                                                2 derivative_variables
                                                1 DVV_1:x1
                                                2 DVV_2:x2
                                                0 analysis_components
                                               60 eval_id

This file is called if the line

.. container:: small

   ::

            named 'workdir' file_save  directory_save

in Figure `[advint:figure01] <#advint:figure01>`__ is uncommented. The
first portion of the file indicates that there are two variables,
followed by new values for variables ``x1`` and ``x2``, and one response
function (an objective function), followed by an active set vector (ASV)
value of ``1``. The ASV indicates the need to return the value of the
objective function for these parameters (see
Section `[variables:asv] <#variables:asv>`__). The script reads the
variable values from this file, namely ``4.664752623441543e-01`` and
``2.256400864298234e-01`` for ``x1`` and ``x2`` respectively, and
substitutes them in the ``{x1}`` and ``{x2}`` fields of the file. The
final three lines of the resulting input file () then appear as follows:

.. container:: small

   ::

      variable 1 0.4664752623
      variable 2 0.2256400864
      end

where all other lines are identical to the template file. The simulator
accepts as its input file and generates the following output to the file
:

.. container:: small

   ::

       Beginning execution of model: Rosenbrock black box
       Set up complete.
       Reading nodes.
       Reading elements.
       Reading materials.
       Checking connectivity...OK
       *****************************************************

       Input value for x1 =  4.6647526230000003e-01
       Input value for x2 =  2.2564008640000000e-01

       Computing solution...Done
       *****************************************************
       Function value =   2.9111427884970176e-01
       Function gradient = [ -2.5674048470887652e+00   1.6081832124292317e+00 ]

Next, the appropriate values are extracted from the raw simulator output
and returned in the results file. This post-processing is relatively
trivial in this case, and the uses the ``grep`` and ``cut`` utilities to
extract the value from the “\ ``Function value``" line of the output
file and save it to ``$results``, which is the file for this evaluation.
This single value provides the objective function value requested by the
ASV.

After 132 of these function evaluations, the following Dakota output
shows the final solution using the simulator:

.. container:: footnotesize

   ::

          Exit NPSOL - Optimal solution found.

          Final nonlinear objective value =   0.1165704E-06

         NPSOL exits with INFORM code = 0 (see "Interpretation of output" section in NPSOL manual)

         NOTE: see Fortran device 9 file (fort.9 or ftn09)
               for complete NPSOL iteration history.

         <<<<< Iterator npsol_sqp completed.
         <<<<< Function evaluation summary: 132 total (132 new, 0 duplicate)
         <<<<< Best parameters          =
                               9.9965861667e-01 x1
                               9.9931682203e-01 x2
         <<<<< Best objective function  =
                            1.1657044253e-07
         <<<<< Best data captured at function evaluation 130

         <<<<< Iterator npsol_sqp completed.
         <<<<< Single Method Strategy completed.
         Dakota execution time in seconds:
           Total CPU        =       0.12 [parent =   0.116982, child =   0.003018]
           Total wall clock =    1.47497

Adapting These Scripts to Another Simulation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To adapt this approach for use with another simulator, several steps
need to be performed:

#. Create a template simulation input file by identifying the fields in
   an existing input file that correspond to the variables of interest
   and then replacing them with ``{}`` identifiers (e.g. ``{cdv_1}``,
   ``{cdv_2}``, etc.) which match the Dakota variable descriptors. Copy
   this template input file to a templatedir that will be used to create
   working directories for the simulation.

#. Modify the ``dprepro`` arguments in to reflect names of the Dakota
   parameters file (previously “``$1``”), template file name (previously
   ) and generated input file (previously ). Alternatively, use APREPRO,
   BPREPRO, or JPrePost to perform this step (and adapt the syntax
   accordingly).

#. Modify the analysis section of to replace the function call with the
   new simulator name and command line syntax (typically including the
   input and output file names).

#. Change the post-processing section in to reflect the revised
   extraction process. At a minimum, this would involve changing the
   ``grep`` command to reflect the name of the output file, the string
   to search for, and the characters to cut out of the captured output
   line. For more involved post-processing tasks, invocation of
   additional tools may have to be added to the script.

#. Modify the input file to reflect, at a minimum, updated variables and
   responses specifications.

These nonintrusive interfacing approaches can be used to rapidly
interface with simulation codes. While generally custom for each new
application, typical interface development time is on the order of an
hour or two. Thus, this approach is scalable when dealing with many
different application codes. Weaknesses of this approach include the
potential for loss of data precision (if care is not taken to preserve
precision in pre- and post-processing file I/O), a lack of robustness in
post-processing (if the data capture is too simplistic), and scripting
overhead (only noticeable if the simulation time is on the order of a
second or less).

If the application scope at a particular site is more focused and only a
small number of simulation codes are of interest, then more
sophisticated interfaces may be warranted. For example, the economy of
scale afforded by a common simulation framework justifies additional
effort in the development of a high quality Dakota interface. In these
cases, more sophisticated interfacing approaches could involve a more
thoroughly developed black box interface with robust support of a
variety of inputs and outputs, or it might involve intrusive interfaces
such as the direct simulation interface discussed below in
Section `[advint:direct] <#advint:direct>`__ or the SAND interface
described in Section `[intro:coupling] <#intro:coupling>`__.

Additional Examples
~~~~~~~~~~~~~~~~~~~

A variety of additional examples of black-box interfaces to simulation
codes are maintained in the directory.

.. _`interfaces:components`:

Simulation Interface Components
-------------------------------

Figure `1.1 <#interfaces:bbinterfacecomp>`__ is an extension of
Figure `[intro:bbinterface] <#intro:bbinterface>`__ that adds details of
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

.. figure:: images/dakota_components.png
   :alt: Components of the simulation interface
   :name: interfaces:bbinterfacecomp

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
``$argv[1]`` and ``$argv[2]`` (see :raw-latex:`\cite{And86}`).
Similarly, Bourne shell scripts retrieve the two command line arguments
using ``$1`` and ``$2``, and Perl scripts retrieve the two command line
arguments using ``@ARGV[0]`` and ``@ARGV[1]``. In the case of a C or C++
program, command line arguments are retrieved using ``argc`` (argument
count) and ``argv`` (argument vector) :raw-latex:`\cite{Ker88}`, and for
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

.. container:: small

   ::

          driver params.in results.out

where is the user-specified analysis driver and and are the names of the
parameters and results files, respectively, passed on the command line.
In this case, the user need not retrieve the command line arguments
since the same file names will be used each time.

For the same mapping, the fork simulation interface echoes the following
syntax:

.. container:: small

   ::

          blocking fork: driver params.in results.out

for which only a single blocking fork is needed to perform the
evaluation.

Executing the same mapping with the direct simulation interface results
in an echo of the following syntax:

.. container:: small

   ::

          Direct function: invoking driver

where this analysis driver must be linked as a function within Dakota’s
direct interface (see Section `[advint:direct] <#advint:direct>`__).
Note that no parameter or response files are involved, since such values
are passed directly through the function argument lists.

Both the system call and fork interfaces support asynchronous
operations. The asynchronous system call execution syntax involves
executing the system call in the background:

.. container:: small

   ::

          driver params.in.1 results.out.1 &

and the asynchronous fork execution syntax involves use of a nonblocking
fork:

.. container:: small

   ::

          nonblocking fork: driver params.in.1 results.out.1

where file tagging (see Section `1.5.2 <#interfaces:file:tagging1>`__)
has been user-specified in both cases to prevent conflicts between
concurrent analysis drivers. In these cases, the user must retrieve the
command line arguments since the file names change on each evaluation.
Execution of the direct interface must currently be performed
synchronously since multithreading is not yet supported (see
Section `[parallel:SLP:local:direct] <#parallel:SLP:local:direct>`__).

.. _`interfaces:components:single2`:

Single analysis driver with filters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When filters are used, the syntax of the system call that Dakota
performs is:

.. container:: small

   ::

          ifilter params.in results.out; driver params.in results.out;
               ofilter params.in results.out

in which the input filter (), analysis driver (), and output filter ()
processes are combined into a single system call through the use of
semi-colons (see :raw-latex:`\cite{And86}`). All three portions are
passed the names of the parameters and results files on the command
line.

For the same mapping, the fork simulation interface echoes the following
syntax:

.. container:: small

   ::

          blocking fork: ifilter params.in results.out;
               driver params.in results.out; ofilter params.in results.out

where a series of three blocking forks is used to perform the
evaluation.

Executing the same mapping with the direct simulation interface results
in an echo of the following syntax:

.. container:: small

   ::

          Direct function: invoking { ifilter driver ofilter }

where each of the three components must be linked as a function within
Dakota’s direct interface. Since asynchronous operations are not yet
supported, execution simply involves invocation of each of the three
linked functions in succession. Again, no files are involved since
parameter and response data are passed directly through the function
argument lists.

Asynchronous executions would appear as follows for the system call
interface:

.. container:: small

   ::

          (ifilter params.in.1 results.out.1; driver params.in.1 results.out.1;
               ofilter params.in.1 results.out.1) &

and, for the fork interface, as:

.. container:: small

   ::

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
approaches provides the enhanced flexibility to support a variety of
local parallelism approaches (see Chapter `[parallel] <#parallel>`__).

.. _`interfaces:components:multiple1`:

Multiple analysis drivers without filters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If a list of ``analysis_drivers`` is specified and filters are not
needed (i.e., neither ``input_filter`` nor ``output_filter`` appears),
then the system call syntax would appear as:

.. container:: small

   ::

          driver1 params.in results.out.1; driver2 params.in results.out.2;
               driver3 params.in results.out.3

where , , and are the user-specified analysis drivers and and are the
user-selected names of the parameters and results files. Note that the
results files for the different analysis drivers have been automatically
tagged to prevent overwriting. This automatic tagging of *analyses* (see
Section `1.5.4 <#interfaces:file:tagging2>`__) is a separate operation
from user-selected tagging of *evaluations* (see
Section `1.5.2 <#interfaces:file:tagging1>`__).

For the same mapping, the fork simulation interface echoes the following
syntax:

.. container:: small

   ::

          blocking fork: driver1 params.in results.out.1;
               driver2 params.in results.out.2; driver3 params.in results.out.3

for which a series of three blocking forks is needed (no reforking of an
intermediate process is required).

Executing the same mapping with the direct simulation interface results
in an echo of the following syntax:

.. container:: small

   ::

          Direct function: invoking { driver1 driver2 driver3 }

where, again, each of these components must be linked within Dakota’s
direct interface and no files are involved for parameter and response
data transfer.

Both the system call and fork interfaces support asynchronous function
evaluations. The asynchronous system call execution syntax would be
reported as

.. container:: small

   ::

          (driver1 params.in.1 results.out.1.1; driver2 params.in.1 results.out.1.2;
               driver3 params.in.1 results.out.1.3) &

and the nonblocking fork execution syntax would be reported as

.. container:: small

   ::

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

.. container:: small

   ::

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
analysis pre- and post-processing (e.g., overlay of response results
from individual analyses; see
Section `1.5.4 <#interfaces:file:tagging2>`__ for additional
information).

For the same mapping, the fork simulation interface echoes the following
syntax:

.. container:: small

   ::

          blocking fork: ifilter params.in.1 results.out.1;
               driver1 params.in.1 results.out.1.1;
               driver2 params.in.1 results.out.1.2;
               driver3 params.in.1 results.out.1.3;
               ofilter params.in.1 results.out.1

for which a series of five blocking forks is used (no reforking of an
intermediate process is required).

Executing the same mapping with the direct simulation interface results
in an echo of the following syntax:

.. container:: small

   ::

          Direct function: invoking { ifilter driver1 driver2 driver3 ofilter }

where each of these components must be linked as a function within
Dakota’s direct interface. Since asynchronous operations are not
supported, execution simply involves invocation of each of the five
linked functions in succession. Again, no files are involved for
parameter and response data transfer since this data is passed directly
through the function argument lists.

Asynchronous executions would appear as follows for the system call
interface:

.. container:: small

   ::

          (ifilter params.in.1 results.out.1;
               driver1 params.in.1 results.out.1.1;
               driver2 params.in.1 results.out.1.2;
               driver3 params.in.1 results.out.1.3;
               ofilter params.in.1 results.out.1) &

and for the fork interface:

.. container:: small

   ::

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
in .

.. _`interfaces:file`:

Simulation File Management
--------------------------

This section describes some management features used for files that
transfer data between Dakota and simulation codes (i.e., when the system
call or fork interfaces are used). These features can generate unique
filenames when Dakota executes programs in parallel and can help one
debug the interface between Dakota and a simulation code.

.. _`interfaces:file:saving`:

File Saving
~~~~~~~~~~~

**Before driver execution:** In Dakota 5.0 and newer, an existing
results file will be removed immediately prior to executing the analysis
driver. This new behavior addresses a common user problem resulting from
starting Dakota with stale results files in the run directory. To
override this default behavior and preserve any existing results files,
specify ``allow_existing_results``.

**After driver execution:** The ``file_save`` option in the interface
specification allows the user to control whether parameters and results
files are retained or removed from the working directory after the
analysis completes. Dakota’s default behavior is to remove files once
their use is complete to reduce clutter. If the method output setting is
verbose, a file remove notification will follow the function evaluation
echo, e.g.,

.. container:: small

   ::

          driver /usr/tmp/aaaa20305 /usr/tmp/baaa20305
          Removing /usr/tmp/aaaa20305 and /usr/tmp/baaa20305

However, if ``file_save`` appears in the interface specification, these
files will not be removed. This latter behavior is often useful for
debugging communication between Dakota and simulator programs. An
example of a ``file_save`` specification is shown in the file tagging
example below.

.. _`interfaces:file:tagging1`:

File Tagging for Evaluations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When a user provides ``parameters_file`` and ``results_file``
specifications, the ``file_tag`` option in the interface specification
causes Dakota to make the names of these files unique by appending the
function evaluation number to the root file names. Default behavior is
to not tag these files, which has the advantage of allowing the user to
ignore command line argument passing and always read to and write from
the same file names. However, it has the disadvantage that files may be
overwritten from one function evaluation to the next. When ``file_tag``
appears in the interface specification, the file names are made unique
by the appended evaluation number. This uniqueness requires the user’s
interface to get the names of these files from the command line. The
file tagging feature is most often used when concurrent simulations are
running in a common disk space, since it can prevent conflicts between
the simulations. An example specification of ``file_tag`` and
``file_save`` is shown below:

.. container:: small

   ::

          interface,
                  system
                    analysis_driver = 'text_book'
                    parameters_file = 'text_book.in'
                    results_file    = 'text_book.out'
                    file_tag file_save

*Special case:* When a user specifies names for the parameters and
results files and ``file_save`` is used without ``file_tag``, untagged
files are used in the function evaluation but are then moved to tagged
files after the function evaluation is complete, to prevent overwriting
files for which a ``file_save`` request has been given. If the output
control is set to verbose, then a notification similar to the following
will follow the function evaluation echo:

.. container:: small

   ::

          driver params.in results.out
          Files with non-unique names will be tagged to enable file_save:
          Moving params.in to params.in.1
          Moving results.out to results.out.1

**Hierarchical tagging:** When a model’s specification includes the
``hierarchical_tagging`` keyword, the tag applied to parameter and
results file names of any subordinate interfaces will reflect any model
hierarchy present. This option is useful for studies involving multiple
models with a nested or hierarchical relationship. For example a nested
model has a sub-method, which itself likely operates on a sub-model, or
a hierarchical approximation involves coordination of low and high
fidelity models. Specifying ``hierarchical_tagging`` will yield function
evaluation identifiers (“tags”) composed of the evaluation IDs of the
models involved, e.g., outermodel.innermodel.interfaceid = 4.9.2. This
communicates the outer contexts to the analysis driver when performing a
function evaluation. For an example of using hierarchical tagging in a
nested model context, see .

.. _`interfaces:file:temporary`:

Temporary Files
~~~~~~~~~~~~~~~

If ``parameters_file`` and ``results_file`` are not specified by the
user, temporary files having generated names are used. For example, a
system call to a single analysis driver might appear as:

.. container:: small

   ::

          driver /tmp/dakota_params_aaaa2035 /tmp/dakota_results_baaa2030

and a system call to an analysis driver with filter programs might
appear as:

.. container:: small

   ::

          ifilter /tmp/dakota_params_aaaa2490 /tmp/dakota_results_baaa2490;
               driver /tmp/dakota_params_aaaa2490 tmp/dakota_results_baaa2490;
               ofilter /tmp/dakota_params_aaaa2490 /tmp/dakota_results_baa22490

These files have unique names created by Boost filesystem utilities.
This uniqueness requires the user’s interface to get the names of these
files from the command line. File tagging with evaluation number is
unnecessary with temporary files, but can be helpful for the user
workflow to identify the evaluation number. Thus ``file_tag`` requests
will be honored. A ``file_save`` request will be honored, but it should
be used with care since the temporary file directory could easily become
cluttered without the user noticing.

.. _`interfaces:file:tagging2`:

File Tagging for Analysis Drivers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When multiple analysis drivers are involved in performing a function
evaluation with either the system call or fork simulation interface, a
secondary file tagging is *automatically* used to distinguish the
results files used for the individual analyses. This applies to both the
case of user-specified names for the parameters and results files and
the default temporary file case. Examples for the former case were shown
previously in Section `1.4.3 <#interfaces:components:multiple1>`__ and
Section `1.4.4 <#interfaces:components:multiple2>`__. The following
examples demonstrate the latter temporary file case. Even though Unix
temporary files have unique names for a particular function evaluation,
tagging is still needed to manage the individual contributions of the
different analysis drivers to the response results, since the same root
results filename is used for each component. For the system call
interface, the syntax would be similar to the following:

.. container:: small

   ::

          ifilter /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ;
               driver1 /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ.1;
               driver2 /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ.2;
               driver3 /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ.3;
               ofilter /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ

and, for the fork interface, similar to:

.. container:: small

   ::

          blocking fork:
               ifilter /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ;
               driver1 /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ.1;
               driver2 /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ.2;
               driver3 /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ.3;
               ofilter /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ

Tagging of results files with an analysis identifier is needed since
each analysis driver must contribute a user-defined subset of the total
response results for the evaluation. If an output filter is not
supplied, Dakota will combine these portions through a simple overlaying
of the individual contributions (i.e., summing the results in , , and ).
If this simple approach is inadequate, then an output filter should be
supplied to perform the combination. This is the reason why the results
file for the output filter does not use analysis tagging; it is
responsible for the results combination (i.e., combining , , and into ).
In this case, Dakota will read only the results file from the output
filter (i.e., ) and interpret it as the total response set for the
evaluation.

Parameters files are not currently tagged with an analysis identifier.
This reflects the fact that Dakota does not attempt to subdivide the
requests in the active set vector for different analysis portions.
Rather, the total active set vector is passed to each analysis driver
and the appropriate subdivision of work *must be defined by the user*.
This allows the division of labor to be very flexible. In some cases,
this division might occur across response functions, with different
analysis drivers managing the data requests for different response
functions. And in other cases, the subdivision might occur within
response functions, with different analysis drivers contributing
portions to each of the response functions. The only restriction is that
each of the analysis drivers must follow the response format dictated by
the total active set vector. For response data for which an analysis
driver has no contribution, 0’s must be used as placeholders.

.. _`interfaces:workdir`:

Work Directories
~~~~~~~~~~~~~~~~

Sometimes it is convenient for simulators and filters to run in a
directory different from the one where Dakota is invoked. For instance,
when performing concurrent evaluations and/or analyses, it is often
necessary to cloister input and output files in separate directories to
avoid conflicts. A simulator script used as an ``analysis_driver`` can
of course include commands to change to a different directory if desired
(while still arranging to write a results file in the original
directory), but Dakota has facilities that may simplify the creation of
simulator scripts. When the ``work_directory`` feature is enabled,
Dakota will create a directory for each evaluation/analysis (with
optional tagging and saving as with files). To enable the
``work_directory`` feature an interface specification includes the
keyword

.. container:: small

   ::

             work_directory

then Dakota will arrange for the simulator and any filters to wake up in
the work directory, with $PATH adjusted (if necessary) so programs that
could be invoked without a relative path to them (i.e., by a name not
involving any slashes) from Dakota’s directory can also be invoked from
the simulator’s (and filter’s) directory. On occasion, it is convenient
for the simulator to have various files, e.g., data files, available in
the directory where it runs. If, say, is such a directory (as seen from
Dakota’s directory), the interface specification

.. container:: small

   ::

             work_directory named 'my/special/directory'

would cause Dakota to start the simulator and any filters in that
directory. If the directory did not already exist, Dakota would create
it and would remove it after the simulator (or output filter, if
specified) finished, unless instructed not to do so by the appearance of
``directory_save`` (or its deprecated synonym ``dir_save``) in the
interface specification. If ``named ’...’`` does not appear, then
``directory_save`` cannot appear either, and Dakota creates a temporary
directory (using the ``tmpnam`` function to determine its name) for use
by the simulator and any filters. If you specify ``directory_tag`` (or
the deprecated ``dir_tag``), Dakota causes each invocation of the
simulator and any filters to start in a subdirectory of the work
directory with a name composed of the work directory’s name followed by
a period and the invocation number (1, 2, :math:`...`); this might be
useful in debugging.

Sometimes it can be helpful for the simulator and filters to start in a
new directory populated with some files. Adding

.. container:: small

   ::

             link_files 'templatedir/*'

to the work directory specification would cause the contents of
directory to be linked into the work directory. Linking makes sense if
files are large, but when practical, it is far more reliable to have
copies of the files; adding ``copy_files`` to the specification would
cause the contents of the template directory to be copied to the work
directory. The linking or copying does not overwrite existing files
unless ``replace`` also appears in the specification.

Here is a summary of possibilities for a work directory specification,
with ``[...]`` denoting that :math:`...` is optional:

.. container:: small

   ::

        work_directory [ named '...' ]
          [ directory_tag ]     # (or dir_tag)
          [ directory_save ]    # (or dir_save)
          [ link_files '...' '...' ]
          [ copy_files '...' '...' ]
          [ replace ]

Figure `[fig:interface:workdir] <#fig:interface:workdir>`__ contains an
example of these specifications in a Dakota input file for constrained
optimization.

.. container:: bigbox

   .. container:: small

.. _`interfaces:batch`:

Batched Evaluations
-------------------

Beginning with release 6.11, Dakota provides for execution of
evaluations in batches. Batch mode is intended to allow a user to assume
greater control over where and when to run individual evaluations. It is
activated using the ``batch`` keyword.

In batch mode, Dakota writes the parameters for multiple (a batch of)
evaluations to a single batch parameters file and then invokes the
analysis driver once for the entire batch. The pathname of the combined
parameters file (and of the results file) are communicated to the driver
as command line arguments. After the driver exits, Dakota expects to
find results for the entire batch in a single combined results file.

The analysis driver is responsible for parsing the parameters file and
performing an evaluation for each set of parameters it contains, and for
returning results for all the evaluations to Dakota. The user is free to
set up the driver to perform the evaluations in the batch in a way that
is convenient.

By default, all currently available evaluations are added to a single
batch. For example, in a sampling study that has a 1000 samples, by
default all 1000 evaluations would be added to a single batch. The batch
size may be limited using the ``size`` subkeyword. Setting
``size = 100`` would result in 10 equal-size batches being run one after
another in a 1000-sample study.

File Formats
~~~~~~~~~~~~

The combined parameters file for a batch is simply the concatenation of
all the parameters files for the evaluations in the batch. The
individual parameter sets may use the default Dakota format, or the user
can select the aprepro format.

The following example parameters file contains parameter sets for two
evaluations.

.. container:: small

   ::

                                                1 variables
                           -4.912558193411678e-01 x1
                                                1 functions
                                                1 ASV_1:response_fn_1
                                                1 derivative_variables
                                                1 DVV_1:x1
                                                0 analysis_components
                                              1:1 eval_id
                                                1 variables
                           -2.400695372000337e-01 x1
                                                1 functions
                                                1 ASV_1:response_fn_1
                                                1 derivative_variables
                                                1 DVV_1:x1
                                                0 analysis_components
                                              1:2 eval_id    interface,

Note that the ``eval_id`` contains two pieces of information separated
by a colon. The second is the evaluation number, and the first is the
batch number. The batch number is an incrementing integer that uniquely
identifies the batch.

The combined results file format is likewise a concatenation of the
results for all the evaluations in the batch. However, a line beginning
with the “#” character must separate the results for each evaluation.

The order of the evaluations in the results file must match the order in
the parameters file.

The following is an example batch results file corresponding to the
batch parameters file above. The initial # on the first line is
optional, and a final # (not shown here) is allowed.

.. container:: small

   ::

      #
                           4.945481774823024e+00 f
      #
                           2.364744129789246e+00 f

Work Directories, Tagging, and Other Features
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Each batch is executed in a work directory when this feature is enabled.
The batch number is used to tag files and directories if tagging is
requested (or Dakota automatically applies a tag to safely save a file
or directory). As explained in the previous section, the batch number is
an incrementing integer beginning with 1 that uniquely identifies a
batch.

Batch mode restricts the use of several other Dakota features.

-  No ``input_filter`` or ``output_filter`` is allowed.

-  Only one ``analysis_driver`` is permitted.

-  ``failure_capture`` modes are limited to abort and recover.

-  Asynchronous evaluation is disallowed (only one batch at a time may
   be executed).

.. _`interfaces:mappings`:

Parameter to Response Mapping Examples
--------------------------------------

In this section, interface mapping examples are presented through the
discussion of several parameters files and their corresponding results
files. A typical input file for 2 variables (:math:`n=2`) and 3
functions (:math:`m=3`) using the standard parameters file format (see
Section `[variables:parameters:standard] <#variables:parameters:standard>`__)
is as follows:

.. container:: small

   ::

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
“``value tag``” constructs. The number of design variables (:math:`n`)
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

For the APREPRO format option (see
Section `[variables:parameters:aprepro] <#variables:parameters:aprepro>`__),
the same set of data appears as follows:

.. container:: small

   ::

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
the parameters file and creating a results file that contains the
response data requested in the ASV. This response data is written in the
format described in
Section `[responses:results] <#responses:results>`__. Since the ASV
contains all ones in this case, the response file corresponding to the
above input file would contain values for the three functions:

.. container:: small

   ::

          1.250000000000000e-01 f
          1.500000000000000e+00 c1
          1.500000000000000e+00 c2

Since function tags are optional, the following would be equally
acceptable:

.. container:: small

   ::

          1.250000000000000e-01
          1.500000000000000e+00
          1.500000000000000e+00

For the same parameters with different ASV components,

.. container:: small

   ::

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

.. container:: small

   ::

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

.. container:: small

   ::

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

.. container:: small

   ::

          [ 5.000000000000000e-01 5.000000000000000e-01 ]
          [ -5.000000000000000e-01 3.000000000000000e+00 ]

Here, we need gradients for functions ``f`` and ``c2``, but not for
``c1``, presumably since this constraint is inactive.

A full Newton optimizer might make the following request:

.. container:: small

   ::

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

.. container:: small

   ::

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
vectors and Hessian matrices) depends upon the type of study being
performed (see Section `[responses:active] <#responses:active>`__). For
a reliability analysis problem, the uncertain variables are the active
continuous variables and the following parameters file would be typical:

.. container:: small

   ::

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

.. container:: small

   ::

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

.. container:: small

   ::

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

.. container:: small

   ::

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

.. _`interfaces:dakota.interfacing`:

Parameters and Results Files with dakota.interfacing
----------------------------------------------------

The Python module ``dakota.interfacing`` first was made available with
Dakota 6.6. (It was released with Dakota 6.5 as the module ``dipy``.) By
providing a Python interface to read and write, respectively, Dakota
parameters and results files, ``dakota.interfacing`` can simplify
development of black-box interfaces. The benefit may be greatest when
one or more phases of the interface (pre-processing, execution,
post-processing) is written in Python.

The following sections describe the components of
``dakota.interfacing``. These components include:

-  The ``Parameters`` class. Makes available the variable information
   for a single evaluation

-  The ``Results`` class. Collects results for a single evaluation and
   writes them to file

-  The ``BatchParameters`` and ``BatchResults`` classes. Containers for
   multiple ``Parameters`` and ``Results`` objects; used when
   evaluations are performed by Dakota in batch mode
   (Section `1.6 <#interfaces:batch>`__)

-  The ``read_parameters_file`` function. Constructs ``Parameters``,
   ``Results``, ``BatchParameters``, and ``BatchResults`` objects from a
   Dakota parameters file.

Creating Parameters and Results objects
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``dakota.interfacing`` has one free function, ``read_parameters_file``,
which creates ``Parameters``, ``Results``, ``BatchParameters``, and
``BatchResults`` objects from a Dakota parameters file. For single,
non-batch evaluation, it returns a tuple that contains
``(Parameters, Results)``. For batch evaluations, it instead returns a
tuple containing ``(BatchParameters, BatchResults)``.

Its signature is:

[index:dakota.interfacing.read_parameters_file]\ ``dakota.interfacing.``\ **``read_parameters_file``**\ (*parameters_file=None*,
*results_file=None*,
*ignore_asv=False*, *batch=False*)

*parameters_file* and *results_file* are the names of the parameters
file that is to be read and the results file that ultimately is to be
written. The names can be absolute or relative filepaths or just
filenames. If a parameters file or results file is not provided, it will
be obtained from the command line arguments. (The results filename is
assumed to be the last command line argument, and the parameters file
the second to last.) Note that if the working directory has changed
since script invocation, filenames provided as command line arguments by
Dakota’s ``fork`` or ``system`` interfaces may be incorrect.

| If *results_file* is set to the constant
  ``dakota.interfacing.UNNAMED``, the ``Results`` or ``BatchResults``
  object is constructed without a results file name. In this case, an
  output stream must be provided when
| ``Results.write()`` or ``BatchResults.write()`` is called. Unnamed
  results files are most helpful when no results file will be written,
  as with a script intended purely for pre-processing.

By default, the returned ``Results`` or ``BatchResults`` object enforces
the active set vector (see the ``Results`` class section). This behavior
can be overridden, allowing any property (function, gradient, Hessian)
of a response to be set, by setting *ignore_asv* to ``True``. The
*ignore_asv* option can be useful when setting up or debugging a driver.

The ``batch`` argument must be set to ``True`` when batch evaluation has
been requested in the Dakota input file, and ``False`` when not.

Parameters objects
~~~~~~~~~~~~~~~~~~

``Parameters`` objects make the variables, analysis components,
evaluation ID, and evaluation number read from a Dakota parameters file
available through a combination of key-value access and object
attributes. Although ``Parameters`` objects may be constructed directly,
it is advisable to use the ``read_parameters_file`` function instead.

Variable values can be accessed by Dakota descriptor or by index using
[] on the object itself. Variables types (integer, real, string) are
inferred by first attempting to convert to ``int`` and then, if this
fails, to ``float``.

Analysis components are accessible by index only using the ``an_comps``
attribute. Iterating over a ``Parameters`` object yields the variable
descriptors.

``Parameters`` objects have the attributes:

-  [index:dakota.interfacing.Parameters.an_comps]\ **``an_comps``** List
   of the analysis components (strings).

-  [index:dakota.interfacing.Parameters.eval_id]\ **``eval_id``**
   Evaluation id (string).

-  [index:dakota.interfacing.Parameters.eval_num]\ **``eval_num``**
   Evaluation number (final token in eval_id) (int).

-  [index:dakota.interfacing.Parameters.aprepro_format]\ **``aprepro_format``**
   Boolean indicating whether the parameters file was in aprepro (True)
   or Dakota (False) format.

-  [index:dakota.interfacing.Parameters.descriptors]\ **``descriptors``**
   List of the variable descriptors

-  [index:dakota.interfacing.Parameters.num_variables]\ **``num_variables``**
   Number of variables

-  [index:dakota.interfacing.Parameters.num_an_comps]\ **``num_an_comps``**
   Number of analysis components

Parameters objects have the methods:

-  [index:dakota.interfacing.Parameters.items]\ **``items``**\ () Return
   an iterator that yields tuples of the descriptor and value for each
   parameter. (``Results`` objects also have ``items()``.)

-  [index:dakota.interfacing.Parameters.values]\ **``values``**\ ()
   Return an iterator that yields the value for each parameter.
   (``Results`` objects have the corresponding method ``responses()``.)

Results objects
~~~~~~~~~~~~~~~

``Results`` objects:

-  communicate response requests from Dakota (active set vector and
   derivative variables)

-  collect response data (function values, gradients, and Hessians)

-  write Dakota results files

``Results`` objects are collections of ``Response`` objects, which are
documented in the following section. Each ``Response`` can be accessed
by name (Dakota descriptor) or by index using [] on the ``Results``
object itself. Iterating over a ``Results`` object yields the response
descriptors. Although ``Results`` objects may be constructed directly,
it is advisable to use the ``read_parameters_file`` function instead.

Results objects have the attributes:

-  [index:dakota.interfacing.Results.eval_id]\ **``eval_id``**
   Evaluation id (a string).

-  [index:dakota.interfacing.Results.eval_num]\ **``eval_num``**
   Evaluation number (final token in eval_id) (int).

-  [index:dakota.interfacing.Results.aprepro_format]\ **``aprepro_format``**
   Boolean indicating whether the parameters file was in aprepro (True)
   or Dakota (False) format.

-  [index:dakota.interfacing.Results.descriptors]\ **``descriptors``**
   List of the response descriptors (strings)

-  [index:dakota.interfacing.Results.num_responses]\ **``num_responses``**
   Number of variables (read-only)

-  [index:dakota.interfacing.Results.deriv_vars]\ **``deriv_vars``**
   List of the derivative variables (strings)

-  [index:dakota.interfacing.Results.num_deriv_vars]\ **``num_deriv_vars``**\ Number
   of derivative variables (int)

Results objects have the methods:

-  [index:dakota.interfacing.Results.items]\ **``items``**\ () Return an
   iterator that yields tuples of the descriptor and ``Response`` object
   for each response. (``Parameters`` objects also have ``items()``.)

-  [index:dakota.interfacing.Results.responses]\ **``responses``**\ ()
   Return an iterator that yields the ``Response`` object for each
   response. (``Parameters`` objects have the corresponding method
   ``values()``.)

-  [index:dakota.interfacing.Results.fail]\ **``fail``**\ () Set the
   FAIL attribute. When the results file is written, it will contain
   only the word FAIL, triggering Dakota’s failure capturing behavior
   (See Chapter `[failure] <#failure>`__).

-  [index:dakota.interfacing.Results.write]\ **``write``**\ (*stream=None*,
   *ignore_asv=None*) Write the results to the Dakota results file. If
   *stream* is set, it overrides the results file name provided at
   construct time. It must be an open file-like object, rather than the
   name of a file. If *ignore_asv* is True, the file will be written
   even if information requested via the active set vector is missing.
   Calling ``write()`` on a ``Results`` object that was generated by
   reading a batch parameters file will raise a ``BatchWriteError``.
   Instead, ``write()`` should be called on the containing
   ``BatchResults`` object.

Response object
~~~~~~~~~~~~~~~

``Response`` objects store response information. They typically are
instantiated and accessed through a Results object by index or response
descriptor using [].

``Response``\ s have the attributes:

-  [index:dakota.interfacing.Response.asv]\ **``asv``** a
   ``collections.namedtuple`` with three members, *function*,
   *gradient*, and *hessian*. Each is a boolean indicating whether
   Dakota requested the associated information for the response.
   ``namedtuples`` can be accessed by index or by member.

-  [index:dakota.interfacing.Response.function]\ **``function``**
   Function value for the response. A ResponseError is raised if Dakota
   did not request the function value (and ignore_asv is False).

-  [index:dakota.interfacing.Response.gradient]\ **``gradient``**
   Gradient for the response. Gradients must be a 1D iterable of values
   that can be converted to floats, such as a ``list`` or 1D
   ``numpy array``. A ResponseError is raised if Dakota did not request
   the gradient (and ignore_asv is False), or if the number of elements
   does not equal the number of derivative variables.

-  [index:dakota.interfacing.Response.hessian]\ **``hessian``** Hessian
   value for the response. Hessians must be an iterable of iterables
   (e.g. a 2D ``numpy array`` or list of lists). A ResponseError is
   raised if Dakota did not request the Hessian (and ignore_asv is
   False), or if the dimension does not correspond correctly with the
   number of derivative variables.

BatchParameters object
~~~~~~~~~~~~~~~~~~~~~~

``BatchParameters`` objects are collections of ``Parameters`` objects.
The individual ``Parameters`` objects can be accessed by index ([]) or
by iterating the ``BatchParameters`` object. Although
``BatchParameters`` objects may be constructed directly, it is advisable
to use the ``read_parameters_file`` function instead.

``BatchParameters`` objects have one attribute.

-  [index:dakota.interfacing.BatchParameters.batch_id]\ **``batch_id``**
   The "id" of this batch of evaluations, reported by Dakota (string).

``BatchParameters`` objects have no methods.

BatchResults object
~~~~~~~~~~~~~~~~~~~

``BatchResults`` objects are collections of ``Results`` objects. The
individual ``Results`` objects can be accessed by index ([]) or by
iterating the ``BatchResults`` object. Although ``BatchResults`` objects
may be constructed directly, it is advisable to use the
``read_parameters_file`` function instead.

``BatchResults`` objects have a single attribute:

-  [index:dakota.interfacing.BatchResults.batch_id]\ **``batch_id``**
   The "id" of this batch of evaluations, reported by Dakota (string)

``BatchResults`` objects have a single method:

-  [index:dakota.interfacing.Results.write]\ **``write``**\ (*stream=None*,
   *ignore_asv=None*) Write results for all evaluations to the Dakota
   results file. If *stream* is set, it overrides the results file name
   provided at construct time. It must be an open file-like object,
   rather than the name of a file. If *ignore_asv* is True, the file
   will be written even if information requested via the active set
   vector is missing.

Processing Templates
~~~~~~~~~~~~~~~~~~~~

Dakota is packaged with a sophisticated command-line template processor
called ``dprepro``. It is fully documented in
Section `1.9 <#interfaces:dprepro-and-pyprepro>`__. Templates may be
processed within Python analysis drivers without externally invoking
``dprepro`` by calling the ``dprepro`` function:

TODO Bogus phantom section

If *template* is a string, it is assumed to contain a template. If it is
a file-like object (that has a ``.read()`` method), the template will be
read from it. (Templates that are already in string form can be passed
in by first wrapping them in a ``StringIO`` object.)

``Parameters`` and ``Results`` objects can be made available to the
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

| In addition to those in this section, the folder contains a runnable
  example of a Python analysis driver. This example demonstrates the
| ``dakota.interfacing`` module.

For most applications, using ``dakota.interfacing`` is straightforward.
The first example, in Figure `[diexample:simple] <#diexample:simple>`__,
is a mock analysis driver. Two variables with the descriptors ``x1`` and
``x2`` are read from the Dakota parameters file and used to evaluate the
fictitious user function ``applic_module.run()``. The result, stored in
``f``, is assigned to the ``function`` value of the appropriate
response. (A common error is leaving off the ``function`` attribute,
which is needed to distinguish the function value of the response from
its gradient and Hessian.)

.. container:: bigbox

   .. container:: small

      ::

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

The ``Results`` object exposes the active set vector read from the
parameters file. When analytic gradients or Hessians are available for a
response, the ASV should be queried to determine what Dakota has
requested for an evaluation. If an attempt is made to addunrequested
information to a response, a ``dakota.interface.ResponseError`` is
raised. The same exception results if a requested piece of information
is missing when ``Results.write()`` is called. The *ignore_asv* option
to ``read_parameters_file`` and ``Results.write()`` overrides ASV
checks.

In Figure `[diexample:asv] <#diexample:asv>`__, ``applic_module.run()``
has been modified to return not only the function value of ``f``, but
also its gradient and Hessian. The ``asv`` attribute is examined to
determine which of these to add to ``results["f"]``.

.. container:: bigbox

   .. container:: small

      ::

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

.. _`interfaces:dprepro-and-pyprepro`:

Preprocessing with ``dprepro`` and ``pyprepro``
-----------------------------------------------

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

Changes and Updates to ``dprepro``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
Figure `[advint:dprepro_usage] <#advint:dprepro_usage>`__.

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

.. container:: bigbox

   .. container:: tiny

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

.. _`interfaces:params-and-results`:

``DakotaParams`` and ``DakotaResults``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If the ``dakota`` Python package (see
Section `1.8 <#interfaces:dakota.interfacing>`__) is available for
import (e.g. has been added to the ``PYTHONPATH``), then ``dprepro``
generates ``Parameters`` and ``Results`` objects from the Dakota
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

.. container:: small

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

.. container:: small

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

Strings can be enclosed by either single quotes (````) or double quotes
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

``d/pyprepro`` performs limited searching for included files, first in
the path of the original template, and then in the path where
``pyprepro`` is executed.

.. _`interfaces:immutable-and-mutable`:

Immutable and Mutable
^^^^^^^^^^^^^^^^^^^^^

As explained elsewhere, variables can be defined as ``Immutable(value)``
or ``Mutable(value)``. If a variable is Immutable, it cannot be
reassigned without first explicitly make it Mutable.

*Note*: Unlike variables defined ``--include`` files
(Section `1.9.3.7 <#interfaces:immutable-variables>`__), variables from
files read in using the ``include()`` function are not Immutable by
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
See `Python’s ``math``
library <https://docs.Python.org/3/library/math.html>`__ for more
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

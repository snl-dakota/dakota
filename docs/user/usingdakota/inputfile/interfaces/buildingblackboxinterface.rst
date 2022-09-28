.. _`interfaces:building`:

"""""""""""""""""""""""""""""""""""""""""""""""""""
Building a Black-Box Interface to a Simulation Code
"""""""""""""""""""""""""""""""""""""""""""""""""""

To interface a simulation code to Dakota using one of the black-box
interfaces (system call or fork), pre- and post-processing functionality
typically needs to be supplied (or developed) in order to transfer the
parameters from Dakota to the simulator input file and to extract the
response values of interest from the simulator’s output file for return
to Dakota (see :numref:`coupling:figure01` and :numref:`interfaces:bbinterfacecomp`).
This is often managed through the use of scripting languages, such as
C-shell :cite:p:`And86`, Bourne shell :cite:p:`Bli96`, Perl :cite:p:`Wal96`, or
Python :cite:p:`Mar03`. While these are common and convenient
choices for simulation drivers/filters, it is important to recognize
that any executable file can be used. If the user prefers, the desired
pre- and post-processing functionality may also be compiled or
interpreted from any number of programming languages (C, C++, F77, F95,
JAVA, Basic, etc.).

In the ``dakota/share/dakota/examples/official/drivers/bash/`` directory, a simple example uses the Rosenbrock test function as
a mock engineering simulation code. Several scripts have been included
to demonstrate ways to accomplish the pre- and post-processing needs.
Actual simulation codes will, of course, have different pre- and
post-processing requirements, and as such, this example serves only to
demonstrate the issues associated with interfacing a simulator.
Modifications will almost surely be required for new applications.

.. _`interfaces:generic`:

Generic Script Interface Files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``dakota/share/dakota/examples/official/drivers/bash/`` directory
contains four important files: ``dakota_rosenbrock.in`` (the Dakota
input file), ``simulator_script.sh`` (the simulation driver script),
``templatedir/ros.template`` (a template simulation input file), and
``templatedir/rosenbrock_bb.py`` (the Rosenbrock simulator).

The file ``dakota_rosenbrock.in`` specifies the study that Dakota will
perform and, in the interface section, describes the components to be
used in performing function evaluations. In particular, it identifies
``simulator_script.sh`` as its ``analysis_driver``, as shown in
:numref:`advint:figure01`.

.. literalinclude:: ../../samples/dakota_rosenbrock.in
   :language: dakota
   :tab-width: 2
   :caption: The ``dakota_rosenbrock.in`` input file.
   :name: advint:figure01

The ``simulator_script.sh`` listed in :numref:`advint:figure02` is a
short driver shell script that Dakota executes to perform each
function evaluation. The names of the parameters and results files are
passed to the script on its command line; they are referenced in the
script by ``$1`` and ``$2``, respectively. The ``simulator_script.sh``
is divided into three parts: pre-processing, analysis, and
post-processing.

.. literalinclude:: ../../samples/simulator_script.sh
   :language: dakota
   :tab-width: 2
   :caption: The ``simulator_script.sh`` sample driver script.
   :name: advint:figure02

In the pre-processing portion, the ``simulator_script.sh`` uses ``dprepro``, 
a template processing utility, to extract the current variable values from
a parameters file (``$1``) and combine them with the simulator template
input file (``ros.template``) to create a new input file (``ros.in``) for
the simulator.

Dakota also provides a second, more general-purpose template processing
tool named ``pyprepro``, which provides many of the same features and
functions as ``dprepro``. Both ``pyprepro`` and ``dprepro`` permit parameter
substitution and execution of arbitrary Python scripting within templates.

This pair of tools is extensively documented in the
:ref:`main dprepro and pyprepro section. <interfaces:dprepro-and-pyprepro>`

.. note::
   
   Internal to Sandia, the APREPRO utility is also often used for pre-processing.
   Other preprocessing tools of potential interest are the BPREPRO utility
   (see :cite:p:`WalXX`), and at Lockheed Martin sites, the JPrePost utility,
   a Java pre- and post-processor :cite:p:`Fla`.
   
The ``dprepro`` script will be used from here on out, for simplicity of discussion.
``dprepro`` can use either Dakota’s :ref:`aprepro parameters file format <variables:parameters:aprepro>`
or :ref:`Dakota’s standard format <variables:parameters:standard>`, so either option
may be selected in the interface section of the Dakota input file.

The ``ros.template`` file in :numref:`advint:figure04`
is a template simulation input file which contains targets for the incoming variable values,
identified by the strings “``{x1}``” and “``{x2}``”. These identifiers
match the variable descriptors specified in ``dakota_rosenbrock.in``. The template input file is
contrived as Rosenbrock has nothing to do with finite element analysis;
it only mimics a finite element code to demonstrate the simulator
template process. The ``dprepro`` script will search the simulator
template input file for fields marked with curly brackets and then
create a new file (``ros.in``) by replacing these targets with the corresponding
numerical values for the variables. As shown in ``simulator_script.sh``, the names for the
Dakota parameters file (``$1``), template file (``ros.template``), and generated input
file (``ros.in``) must be specified in the ``dprepro`` command line arguments.

.. literalinclude:: ../../samples/ros.template
   :language: dakota
   :tab-width: 2
   :caption: Listing of the ``ros.template`` file
   :name: advint:figure04

The second part of the script executes the ``rosenbrock_bb.py`` simulator. The input and
output file names, ``ros.in`` and ``ros.out``, respectively, are hard-coded into the simulator. When the
``./rosenbrock_bb.py`` simulator is executed, the values for ``x1`` and ``x2`` are read in from
``ros.in``, the Rosenbrock function is evaluated, and the function value is
written out to ``ros.out``.

The third part performs the post-processing and writes the response
results to a file for Dakota to read. Using the UNIX “``grep``” utility,
the particular response values of interest are extracted from the raw
simulator output and saved to a temporary file (``results.tmp``). When complete, this
file is renamed ``$2``, which in this example is always ``results.out``. Note that
moving or renaming the completed results file avoids any problems with
read race conditions (see the section on
:ref:`system call synchronization <parallel:SLP:local:system>`).

Because the Dakota input file ``dakota_rosenbrock.in`` (:numref:`advint:figure01`) specifies
``work_directory`` and ``directory_tag`` in its interface section, each
invocation of ``simulator_script.sh`` wakes up in its own temporary directory, which Dakota has
populated with the contents of directory ``templatedir/``. Having a separate directory
for each invocation of ``simulator_script.sh`` simplifies the script when the Dakota input file
specifies ``asynchronous`` (so several instances of ``simulator_script.sh`` might run
simultaneously), as fixed names such as ``ros.in``, ``ros.out``, and ``results.tmp`` can be used for
intermediate files. If neither ``asynchronous`` nor ``file_tag`` is
specified, and if there is no need (e.g., for debugging) to retain
intermediate files having fixed names, then ``directory_tag`` offers no
benefit and can be omitted. An alternative to ``directory_tag`` is to
proceed as earlier versions of this chapter — prior to Dakota 5.0’s
introduction of ``work_directory`` — recommended: add two more steps to
the ``simulator_script.sh``, an initial one to create a temporary directory explicitly and copy
``templatedir/`` to it if needed, and a final step to remove the temporary directory and
any files in it.

When ``work_directory`` is specified, Dakota adjusts the ``$PATH`` seen
by ``simulator_script.sh`` so that simple program names (i.e., names not containing a slash)
that are visible in Dakota’s directory will also be visible in the work
directory. Relative path names — involving an intermediate slash but not
an initial one, such as ``./rosenbrock_bb.py`` or ``a/bc/rosenbrock_bb`` — will only be visible in the work directory
if a ``link_files`` or ``copy_files`` specification (see
\ `1.5.5 <#interfaces:workdir>`__) has made them visible there.

As an example of the data flow on a particular function evaluation,
consider evaluation 60. The parameters file for this evaluation consists
of:

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

This file is called ``workdir/workdir.60/params.in`` if the line

::

         named 'workdir' file_save  directory_save

in :numref:`advint:figure01` is uncommented. The
first portion of the file indicates that there are two variables,
followed by new values for variables ``x1`` and ``x2``, and one response
function (an objective function), followed by an active set vector (ASV)
value of ``1``. The ASV indicates the need to return the value of the
objective function for these parameters (see
Section `[variables:asv] <#variables:asv>`__). The ``dprepro`` script reads the
variable values from this file, namely ``4.664752623441543e-01`` and
``2.256400864298234e-01`` for ``x1`` and ``x2`` respectively, and
substitutes them in the ``{x1}`` and ``{x2}`` fields of the ``ros.template`` file. The
final three lines of the resulting input file (``ros.in``) then appear as follows:

::

   variable 1 0.4664752623
   variable 2 0.2256400864
   end

where all other lines are identical to the template file. The ``rosenbrock_bb`` simulator
accepts ``ros.in`` as its input file and generates the following output to the file
``ros.out``:

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
trivial in this case, and the ``simulator_script.sh`` uses the ``grep`` and ``cut`` utilities to
extract the value from the “\ ``Function value``" line of the ``ros.out`` output
file and save it to ``$results``, which is the ``results.out`` file for this evaluation.
This single value provides the objective function value requested by the
ASV.

After 132 of these function evaluations, the following Dakota output
shows the final solution using the ``rosenbrock_bb.py`` simulator:

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
    <<<<< Best evaluation ID: 130
    
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

#. Modify the ``dprepro`` arguments in ``simulator_script.sh`` to reflect names of the Dakota
   parameters file (previously ``$1``), template file name (previously
   ``ros.template``) and generated input file (previously ``ros.in``). Alternatively, use APREPRO,
   BPREPRO, or JPrePost to perform this step (and adapt the syntax
   accordingly).

#. Modify the analysis section of ``simulator_script.sh`` to replace the ``rosenbrock_bb`` function call with the
   new simulator name and command line syntax (typically including the
   input and output file names).

#. Change the post-processing section in ``simulator_script.sh`` to reflect the revised
   extraction process. At a minimum, this would involve changing the
   ``grep`` command to reflect the name of the output file, the string
   to search for, and the characters to cut out of the captured output
   line. For more involved post-processing tasks, invocation of
   additional tools may have to be added to the script.

#. Modify the ``dakota_rosbenbrock.in`` input file to reflect, at a minimum, updated variables and
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
codes are maintained in the ``dakota/share/dakota/examples/official/drivers`` directory.
.. _`output:standard`:

===============
Standard Output
===============

Dakota prints information on algorithm progress, function evaluations,
and summary results to standard out and error (screen, terminal, or
console) as it runs. Detailed information for a function evaluation may
include an evaluation number, parameter values, execution syntax, the
active set vector, and the response data set.

This output may optionally be redirected to a file using :ref:`command-line options <command-line-main>`
or the keyword :dakkw:`environment-output_file`.

.. note::

   Whenever an output_file is specified for a Dakota run, the screen output itself
   becomes quite minimal, consisting of version statements, environment statements and execution times.

---------------------
Simple Output Example
---------------------

Output from a Rosenbrock multidimensional parameter study (see :numref:`tutorial:rosenbrock_multidim`) is shown here:

.. code-block::

    {Writing new restart file dakota.rst
    methodName = multidim_parameter_study
    gradientType = none
    hessianType = none

    >>>>> Running multidim_parameter_study iterator.

    Multidimensional parameter study for variable partitions of
                                         8
                                         8
                                         
                                         
    ------------------------------
    Begin Function Evaluation 1
    ------------------------------
    Parameters for function evaluation 1:
                         -2.0000000000e+00 x1
                         -2.0000000000e+00 x2
                         
    Direct function: invoking rosenbrock

    Active response data for function evaluation 1:
    Active set vector = { 1 }
                          3.6090000000e+03 response_fn_1
    .
    .
    .
    <<<<< Function evaluation summary: 81 total (81 new, 0 duplicate)

    Simple Correlation Matrix among all inputs and outputs:
                           x1           x2  response_fn_1
              x1  1.00000e+00
              x2  1.73472e-17  1.00000e+00
    response_fn_1 -3.00705e-03 -5.01176e-01 1.00000e+00
    .
    .
    .
    <<<<< Iterator multidim_parameter_study completed.}

Excluding the copy of the input file at the beginning and timing information at the end, the file is organized into three basic
parts:

1. Information on the problem

   For this example, we see that a new restart file is being created and Dakota has carried out a multi-dim parameter study with 8 partitions for each of two variables.

2. Information on each function evaluation

   Each function evaluation is numbered. Details for function evaluation 1 show that at input variable values
   ``x1 = −2.0`` and ``x2 = −2.0``, the direct rosenbrock function is being evaluated. There is one response with
   a value of ``3.609e+03``.

3. Summary statistics

   The function evaluation summary is preceded by ``<<<<<``. For this example 81 total evaluations were
   assessed; all were new, none were read in from the restart file. Correlation matrices complete the statistics
   and output for this problem. Successful runs will finish with ``<<<<< Iterator study type completed.``

---------------   
Detailed Output
---------------

Here, optimization of :ref:`the "container" problem <additional:container>` is used as an example to
describe Dakota output. The input file shown in :numref:`output:incont` specifies one equality
constraint, and that Dakota’s finite difference algorithm will provide
central difference numerical gradients to the NPSOL optimizer.

.. literalinclude:: ../samples/container_opt_npsol.in
   :language: dakota
   :tab-width: 2
   :caption: Dakota input file for the “container” test problem – see ``dakota/share/dakota/examples/users/container_opt_npsol.in``
   :name: output:incont

A partial listing of the Dakota output for the container optimization
example follows:

::

   Dakota version 5.4+ (stable) released May  2 2014.
   Subversion revision 2508 built May  2 2014 15:26:14.
   Running MPI Dakota executable in serial mode.
   Start time: Fri May  2 15:38:11 2014

   ----------------------------------------------------------------
   Begin DAKOTA input file
   /home/user/dakota/build/examples/users/container_opt_npsol.in
   ----------------------------------------------------------------
   # Dakota Input File: container_opt_npsol.in
   environment

   <SNIP>

   ---------------------
   End DAKOTA input file
   ---------------------

   Using Dakota input file '/home/user/dakota/build/examples/users/container_opt_npsol.in'
   Writing new restart file dakota.rst

   >>>>> Executing environment.

   >>>>> Running npsol_sqp iterator.

   ------------------------------------------
   Begin Dakota derivative estimation routine
   ------------------------------------------

   >>>>> Initial map for analytic portion of response:

   ---------------------
   Begin Evaluation    1
   ---------------------
   Parameters for evaluation 1:
                         4.5000000000e+00 H
                         4.5000000000e+00 D

   blocking fork: container container.in.1 container.out.1

   Active response data for evaluation 1:
   Active set vector = { 1 1 }
                         1.0713145108e+02 obj_fn
                         8.0444076396e+00 nln_eq_con_1


   >>>>> Dakota finite difference gradient evaluation for x[1] + h:

   ---------------------
   Begin Evaluation    2
   ---------------------
   Parameters for evaluation 2:
                         4.5045000000e+00 H
                         4.5000000000e+00 D

   blocking fork: container container.in.2 container.out.2

   Active response data for evaluation 2:
   Active set vector = { 1 1 }
                         1.0719761302e+02 obj_fn
                         8.1159770472e+00 nln_eq_con_1


   >>>>> Dakota finite difference gradient evaluation for x[1] - h:

   ---------------------
   Begin Evaluation    3
   ---------------------
   Parameters for evaluation 3:
                         4.4955000000e+00 H
                         4.5000000000e+00 D

   blocking fork: container container.in.3 container.out.3

   Active response data for evaluation 3:
   Active set vector = { 1 1 }
                         1.0706528914e+02 obj_fn
                         7.9728382320e+00 nln_eq_con_1


   >>>>> Dakota finite difference gradient evaluation for x[2] + h:

   ---------------------
   Begin Evaluation    4
   ---------------------
   Parameters for evaluation 4:
                         4.5000000000e+00 H
                         4.5045000000e+00 D

   blocking fork: container container.in.4 container.out.4

   Active response data for evaluation 4:
   Active set vector = { 1 1 }
                         1.0727959301e+02 obj_fn
                         8.1876180243e+00 nln_eq_con_1


   >>>>> Dakota finite difference gradient evaluation for x[2] - h:

   ---------------------
   Begin Evaluation    5
   ---------------------
   Parameters for evaluation 5:
                         4.5000000000e+00 H
                         4.4955000000e+00 D

   blocking fork: container container.in.5 container.out.5

   Active response data for evaluation 5:
   Active set vector = { 1 1 }
                         1.0698339109e+02 obj_fn
                         7.9013403937e+00 nln_eq_con_1


   >>>>> Total response returned to iterator:

   Active set vector = { 3 3 } Deriv vars vector = { 1 2 }
                         1.0713145108e+02 obj_fn
                         8.0444076396e+00 nln_eq_con_1
    [  1.4702653619e+01  3.2911324639e+01 ] obj_fn gradient
    [  1.5904312809e+01  3.1808625618e+01 ] nln_eq_con_1 gradient


   <SNIP>


   >>>>> Dakota finite difference gradient evaluation for x[2] - h:

   ---------------------
   Begin Evaluation   40
   ---------------------
   Parameters for evaluation 40:
                         4.9873894231e+00 H
                         4.0230575428e+00 D

   blocking fork: container container.in.40 container.out.40

   Active response data for evaluation 40:
   Active set vector = { 1 1 }
                         9.8301287596e+01 obj_fn
                        -1.2698647501e-01 nln_eq_con_1


   >>>>> Total response returned to iterator:

   Active set vector = { 3 3 } Deriv vars vector = { 1 2 }
                         9.8432498116e+01 obj_fn
                        -9.6918029158e-12 nln_eq_con_1
    [  1.3157517860e+01  3.2590159623e+01 ] obj_fn gradient
    [  1.2737124497e+01  3.1548877601e+01 ] nln_eq_con_1 gradient



   NPSOL exits with INFORM code = 0 (see "Interpretation of output" section in NPSOL manual)

   NOTE: see Fortran device 9 file (fort.9 or ftn09)
         for complete NPSOL iteration history.
   <<<<< Function evaluation summary: 40 total (40 new, 0 duplicate)
   <<<<< Best parameters          =
                         4.9873894231e+00 H
                         4.0270846274e+00 D
   <<<<< Best objective function  =
                         9.8432498116e+01
   <<<<< Best constraint values   =
                        -9.6918029158e-12
   <<<<< Best evaluation ID: 36


   <<<<< Iterator npsol_sqp completed.
   <<<<< Environment execution completed.
   DAKOTA execution time in seconds:
     Total CPU        =       0.03 [parent =   0.023997, child =   0.006003]
     Total wall clock =   0.090703

The output begins with information on the Dakota version, compilation
date, and run mode. It then echos the user input file before proceeding
to execution phase. The lines following
``>>>>> Running npsol_sqp iterator.`` show Dakota performing function
evaluations 1–5 that have been requested by NPSOL. Evaluations 6 through
39 have been omitted from the listing for brevity.

Immediately following the line ``Begin Function Evaluation 1``, the
initial values of the design variables, the syntax of the blocking fork
function evaluation, and the resulting objective and constraint function
values returned by the simulation are listed. The values of the design
variables are labeled with the tags ``H`` and ``D``, respectively,
according to the descriptors given in the input file,
:numref:`output:incont`. The values of the objective
function and volume constraint are labeled with the tags ``obj_fn`` and
``nln_eq_con_1``, respectively. Note that the initial design parameters
are infeasible since the equality constraint is violated
(:math:`\ne 0`). However, by the end of the run, the optimizer finds a
design that is both feasible and optimal for this example. Between the
design variables and response values, the content of the system call to
the simulator is displayed as
``(container container.in.1 container.out.1)``, with ``container``
being the name of the simulator and ``container.in.1`` and
``container.out.1`` being the names of the parameters and results
files, respectively.

Just preceding the output of the objective and constraint function
values is the line ``Active set vector = {1 1}``. The active set
vector indicates the types of data that are required from the simulator
for the objective and constraint functions, and values of “``1``”
indicate that the simulator must return values for these functions
(gradient and Hessian data are not required).

.. note::

   Refer to :ref:`the Active Set Vector section <variables:asv>` for more details.

Since finite difference gradients have been specified, Dakota computes
their values by making additional function evaluation requests to the
simulator at perturbed parameter values. Examples of the
gradient-related function evaluations have been included in the sample
output, beginning with the line that reads
``>>>>> Dakota finite difference evaluation for x[1] + h:``. The
resulting finite difference gradients are listed after function
evaluation 5 beginning with the line
``>>>>> Total response returned to iterator:``. Here, another active
set vector is displayed in the Dakota output file. The line
``Active set vector = { 3 3 }`` indicates that the total response
resulting from the finite differencing contains function values and
gradients.

The final lines of the Dakota output, beginning with the line
``<<<<< Function evaluation summary:``, summarize the results of the
optimization study. The best values of the optimization parameters,
objective function, and volume constraint are presented along with the
function evaluation number where they occurred, total function
evaluation counts, and a timing summary. In the end, the objective
function has been minimized and the equality constraint has been
satisfied (driven to zero within the constraint tolerance).

| The Dakota results may be intermixed with iteration information from
  the NPSOL library. For example lines with the heading
| ``Majr Minr Step Fun Merit function Norm gZ Violtn nZ Penalty Conv``
| come from Fortran write statements within NPSOL. The output is mixed
  since both Dakota and NPSOL are writing to the same standard output
  stream. The relative locations of these output contributions can vary
  depending on the specifics of output buffering and flushing on a
  particular platform and depending on whether or not the standard
  output is being redirected to a file. In some cases, output from the
  optimization library may appear on each iteration (as in this
  example), and in other cases, it may appear at the end of the Dakota
  output. Finally, a more detailed summary of the NPSOL iterations is
  written to the Fortran device 9 file (e.g., ``fort.9`` or ``ftn09``).

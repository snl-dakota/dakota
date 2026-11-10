.. _analysisdriverdeepdive-main:

"""""""""""""""""""""""""""""""""""""""""""""""""
Deep Dive into Dakota's Analysis Driver Process
"""""""""""""""""""""""""""""""""""""""""""""""""

This page looks more closely at how Dakota exchanges data with a simulation
through an analysis driver. For an introduction, start with
:ref:`couplingtosimulations-main`.

*Note on Terminology:* When speaking about coupling Dakota to a simulation model, the terms "parameter", "variable", and "input" may be used interchangeably,
based on the most appropriate word for the context, but they all mean approximately the same thing. Likewise, "response", "output", and "QOI"
(quantity of interest) will also be used interchangeably based on context.

.. _user-supplied-sim-code-case1:

---------------------------
Compiled Rosenbrock Example
---------------------------

For this example, the C++ program ``rosenbrock.cpp`` in ``dakota_source/test`` is used as the black-box simulation code.
This file is compiled to create the standalone rosenbrock executable that is referenced as the analysis driver in the
example input file below. This standalone program performs the same function evaluations as Dakota’s internal Rosenbrock test function.

.. literalinclude:: ../../samples/rosen_syscall.in
   :language: dakota
   :tab-width: 2
   :caption: Dakota input file for gradient-based optimization using
       the fork call interface to an external rosenbrock simulator.
   :name: tutorial:rosenbrock_user

(The input file shown in :numref:`tutorial:rosenbrock_user` is ``rosen_syscall.in``, which is provided in the directory ``dakota/share/dakota/examples/users.``)

The keyword ``fork`` indicates that Dakota will use fork calls to create separate Unix processes for
executions of the user-supplied simulation code. The name of the simulation code (and the names for Dakota’s parameters and
results file) are specified using the ``analysis_drivers``, ``parameters_file``, and ``results_file`` keywords, respectively.

The Dakota command is noted in the comments at the top of this file. Expected copies of the outputs are in the
``dakota/share/dakota/examples/users`` directory, with ``.sav`` appended to the name.

This run of Dakota takes longer to complete than other examples of gradient-based optimization for the Rosenbrock function,
since the fork interface method has additional process creation and file I/O overhead, as compared to the internal communication
that occurs when the direct interface method is used.

.. _couplingtosimulations-paramformat:

Parameter File Format
---------------------

To gain a better understanding of what exactly Dakota is doing with the fork interface approach, add the keywords ``file_tag``
and ``file_save`` to the interface specification and re-run Dakota. Check the listing of the local directory and you will see
many new files with names such as ``params.in.1``, ``params.in.2``, etc., and ``results.out.1``, ``results.out.2``, etc.
There is one ``params.in.X`` file and one ``results.out.X`` file for each of the function evaluations performed by Dakota.
This is the file listing for ``params.in.1``:

.. code-block::

						  2 variables
	 -1.200000000000000e+00 x1
	  1.000000000000000e+00 x2
						  1 functions
						  1 ASV_1:obj_fn
						  2 derivative_variables
						  1 DVV_1:x1
						  2 DVV_2:x2
						  0 analysis_components

The basic pattern is that of array lengths and string identifiers followed by listings of the array entries, where the arrays consist
of the variables, the active set vector (ASV), the derivative values vector (DVV), and the analysis components (AC).

For the variables array, the first line gives the total number of variables (2) and the “variables” string identifier, and the subsequent
two lines provide the array listing for the two variable values (-1.2 and 1.0) and descriptor tags (“x1” and “x2” from the
Dakota input file).

The next array conveys the ASV, which indicates what simulator outputs are needed. The first line of
the array gives the total number of response functions (1) and the “functions” string identifier, followed by one ASV code
and descriptor tag (“ASV 1”) for each function. In this case, the ASV value of 1 indicates that Dakota is requesting that the
simulation code return the response function value in the file results.out.X. (Possible ASV values: 1 = value of response
function value, 2 = response function gradient, 4 = response function Hessian, and any sum of these for combinations up to 7
= response function value, gradient, and Hessian; see 9.7 for more detail.)

The next array provides the DVV, which defines
the variable identifiers used in computing derivatives. The first line of the array gives the number of derivative variables (2)
and the “derivative variables” string identifier, followed by the listing of the two DVV variable identifiers (the first and second
variables) and descriptor tags (“DVV 1” and “DVV 2”). The final array provides the AC array used to provide additional
strings for use by the simulator (e.g., to provide the name of a particular mesh file). The first line of the array gives the total
number of analysis components (0) and the “analysis components” string identifier, followed by the listing of the array, which
is empty in this case.

---

The executable program ``rosenbrock`` reads in the ``params.in.X`` file and evaluates the objective function at the given values
for x1 and x2. Then, rosenbrock writes out the objective function data to the ``results.out.X`` file. Here is the listing for
the file ``results.out.1``:

.. code-block::

   2.420000000000000e+01 f


The value shown above is the value of the objective function, and the descriptor ‘f’ is an optional tag returned by the simulation
code. When the fork call has completed, Dakota reads in the data from the results.in.X file and processes the results.
Dakota then continues with additional executions of the rosenbrock program until the optimization process is complete.

.. _user-supplied-sim-code-case2:

------------------------------
User-Supplied Simulation Codes
------------------------------

In many situations the user-supplied simulation code cannot be modified to read and write the ``params.in.X`` file and the
``results.out.X`` file, as described above. Typically, this occurs when the simulation code is a commercial or proprietary
software product that has specific input file and output file formats. In such cases, it is common to replace the executable
program name in the Dakota input file with the name of a Unix shell script containing a sequence of commands that read and
write the necessary files and run the simulation code.

For example, the executable program named ``rosenbrock`` in the Dakota input file provided above could be replaced by a
Unix Bourne or C-shell script named ``simulator_script``, with the script containing a
sequence of commands to perform the following steps:

- Insert the data from the ``parameters.in.X`` file into the input file of the simulation code.
- Execute the simulation code.
- Post-process the files generated by the simulation code to compute response data, and return the response data to Dakota in the ``results.out.X`` file.

.. _user-supplied-sim-code-sand:

---------------------
SAND Simulation Codes
---------------------

In some cases it is advantageous to have a close coupling between Dakota and the simulation code. This close coupling is an
advanced feature of Dakota and is accomplished through either a direct interface or a *SAND* (Simultaneous ANalysis and Design)
interface. For the direct interface, the user’s simulation code is modified to behave as a function or subroutine under Dakota.
This interface can be considered to be “semi-intrusive” in that it requires relatively minor modifications to the simulation code.
Its major advantage is the elimination of the overhead resulting from file I/O and process creation. It can also be a useful
tool for parallel processing, by encapsulating all computation in a single executable.

A SAND interface approach is “fully intrusive” in that it requires further modifications to the simulation code so
that an optimizer has access to the internal residual vector and Jacobian matrices computed by the simulation code. In a SAND
approach, both the optimization method and a nonlinear simulation code are converged simultaneously. While this approach
can greatly reduce the computational expense of optimization, considerable software development effort must be expended to
achieve this intrusive coupling between SAND optimization methods and the simulation code. SAND may be supported in
future Dakota releases.

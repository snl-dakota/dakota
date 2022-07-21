.. _advint:

Advanced Simulation Code Interfaces
===================================

This chapter extends the interface discussion in
Chapter `[interfaces] <#interfaces>`__ and its discussion of generic
black-box interfaces to simulations
(Section `[interfaces:building] <#interfaces:building>`__). It describes
specialized, tightly integrated, and advanced interfaces through which
Dakota can perform function evaluation mappings. It describes AMPL-based
algebraic mappings (Section `1.1 <#advint:algebraic>`__), tight
integration of a simulation code into Dakota
(Section `1.2 <#advint:direct>`__), and specialized interfaces to
Matlab, Python, and Scilab (Sections `1.3 <#advint:existingdirect>`__
and `1.4 <#advint:scilab>`__).

.. _`advint:algebraic`:

Algebraic Mappings
------------------

If desired, one can define algebraic input-output mappings using the
AMPL code :cite:p:`Fou03` and save these mappings in 3 files:
, , and , where ``stub`` is a particular root name describing a
particular problem. These files names can be communicated to Dakota
using the ``algebraic_mappings`` input.

Dakota will use and to obtain input and output identifier strings,
respectively, and will use the AMPL solver
library :cite:p:`Gay97` to evaluate expressions conveyed in ,
and, if needed, their first and second derivatives.

As a simple example (from ), consider algebraic mappings based on
Newton’s law :math:`F = m a`. The following is an AMPL input file of
variable and expression declarations and output commands:

TODO: Generated input file likely goes here 

When processed by an AMPL processor, three files are created (as
requested by the “option auxfiles" command). The first is the file
containing problem statistics, expression graphs, bounds, etc.:

TODO: Generated input file likely goes here

Next, the file contains the set of variable descriptor strings:

TODO: Generated input file likely goes here

and the file contains the set of response descriptor strings:

TODO: Generated input file likely goes here

The variable and objective function names declared within AMPL should be
a subset of the variable descriptors and response descriptors used by
Dakota (see the Dakota Reference Manual :cite:p:`RefMan` for
information on Dakota variable and response descriptors). Ordering of
the inputs and outputs within the AMPL declaration is not important, as
Dakota will reorder data as needed. The following listing shows an
excerpt from , which demonstrates a combined algebraic/simulation-based
mapping in which algebraic mappings from the ``fma`` definition are
overlaid with simulation-based mappings from ``text_book``:

::

   variables,
           continuous_design = 5
             descriptor    'x1' 'mass' 'a' 'x4' 'v'
             initial_point  0.0  2.0  1.0  0.0  3.0
             lower_bounds  -3.0  0.0 -5.0 -3.0 -5.0
             upper_bounds   3.0 10.0  5.0  3.0  5.0

   interface,
           algebraic_mappings = 'dakota_ampl_fma.nl'
           system
             analysis_driver = 'text_book'
             parameters_file = 'tb.in'
             results_file    = 'tb.out'
             file_tag

   responses,
           response_descriptors = 'force' 'ineq1' 'energy'
           num_objective_functions = 1
           num_nonlinear_inequality_constraints = 1
           num_nonlinear_equality_constraints = 1
           nonlinear_equality_targets = 20.0
           analytic_gradients
           no_hessians

Note that the algebraic inputs and outputs are a subset of the total
inputs and outputs and that Dakota will track the algebraic
contributions to the total response set using the order of the
descriptor strings. In the case where both the algebraic and
simulation-based components contribute to the same function, they are
added together.

To solve ``text_book`` algebraically (refer to
Section `[additional:textbook] <#additional:textbook>`__ for
definition), the following AMPL model file could be used

TODO: Generated input file likely goes here

Note that the nonlinear constraints should not currently be declared as
constraints within AMPL. Since the Dakota variable bounds and constraint
bounds/targets currently take precedence over any AMPL specification,
the current approach is to declare all AMPL outputs as objective
functions and then map them into the appropriate response function type
(objectives, least squares terms, nonlinear inequality/equality
constraints, or generic response functions) within the Dakota input
specification.

.. _`advint:direct`:

Developing a Direct Simulation Interface
----------------------------------------

If a more efficient interface to a simulation is desired (e.g., to
eliminate process creation and file I/O overhead) or if a targeted
computer architecture cannot accommodate separate optimization and
simulation processes (e.g., due to lightweight operating systems on
compute nodes of large parallel computers), then linking a simulation
code directly with Dakota may be desirable. This is an advanced
capability of Dakota, and it requires a user to have access to (and
knowledge of) the Dakota source code, as well as the source code of the
simulation code.

Three approaches are outlined below for developing direct linking
between Dakota and a simulation: extension, derivation, and sandwich.
For additional information, refer to “Interfacing with Dakota as a
Library” in the Dakota Developers Manual :cite:p:`DevMan`.

Once performed, Dakota can bind with the new direct simulation interface
using the ``direct`` interface specification in combination with an
``analysis_driver``, ``input_filter`` or ``output_filter`` specification
that corresponds to the name of the new subroutine.

.. _`advint:direct:extension`:

Extension
~~~~~~~~~

The first approach to using the direct function capability with a new
simulation (or new internal test function) involves *extension* of the
existing **DirectFnApplicInterface** class to include new simulation
member functions. In this case, the following steps are performed:

#. The functions to be invoked (analysis programs, input and output
   filters, internal testers) must have their main programs changed into
   callable functions/subroutines.

#. The resulting callable function can then be added directly to the
   private member functions in **DirectFnApplicInterface** if this
   function will directly access the Dakota data structures (variables,
   active set, and response attributes of the class). It is more common
   to add a wrapper function to **DirectFnApplicInterface** which
   manages the Dakota data structures, but allows the simulator
   subroutine to retain a level of independence from Dakota (see
   Salinas, ModelCenter, and Matlab wrappers as examples).

#. The if-else blocks in the **derived_map_if()**, **derived_map_ac()**,
   and **derived_map_of()** member functions of the
   **DirectFnApplicInterface** class must be extended to include the new
   function names as options. If the new functions are class member
   functions, then Dakota data access may be performed through the
   existing class member attributes and data objects do not need to be
   passed through the function parameter list. In this case, the
   following function prototype is appropriate:

    ::

           int function_name();

   If, however, the new function names are not members of the
   **DirectFnApplicInterface** class, then an ``extern`` declaration may
   additionally be needed and the function prototype should include
   passing of the Variables, ActiveSet, and Response data members:

  ::

         int function_name(const Dakota::Variables& vars,
                           const Dakota::ActiveSet& set, Dakota::Response& response);

#. The Dakota system must be recompiled and linked with the new function
   object files or libraries.

Various header files may have to be included, particularly within the
**DirectFnApplicInterface** class, in order to recognize new external
functions and compile successfully. Refer to the Dakota Developers
Manual :cite:p:`DevMan` for additional information on the
**DirectFnApplicInterface** class and the Dakota data types.

.. _`advint:direct:derivation`:

Derivation
~~~~~~~~~~

As described in “Interfacing with Dakota as a Library” in the Dakota
Developers Manual :cite:p:`DevMan`, a derivation approach can
be employed to further increase the level of independence between Dakota
and the host application. In this case, rather than *adding* a new
function to the existing **DirectFnApplicInterface** class, a new
interface class is derived from **DirectFnApplicInterface** which
*redefines* the **derived_map_if()**, **derived_map_ac()**, and
**derived_map_of()** virtual functions.

In the approach of Section `1.2.3 <#advint:direct:sandwich>`__ below,
the class derivation approach avoids the need to recompile the Dakota
library when the simulation or its direct interface class is modified.

.. _`advint:direct:sandwich`:

Sandwich
~~~~~~~~

In a “sandwich” implementation, a simulator provides both the “front
end” and “back end” with Dakota sandwiched in the middle. To accomplish
this approach, the simulation code is responsible for interacting with
the user (the front end), links Dakota in as a library (refer to
“Interfacing with Dakota as a Library” in the Dakota Developers
Manual :cite:p:`DevMan`), and plugs in a derived direct
interface class to provide a closely-coupled mechanism for performing
function evaluations (the back end). This approach makes Dakota services
available to other codes and frameworks and is currently used by Sandia
codes such as Xyce (electrical simulation), Sage (CFD), and SIERRA
(multiphysics).

.. _`advint:existingdirect`:

Existing Direct Interfaces to External Simulators
-------------------------------------------------

In addition to built-in polynomial test functions described in
Section `[interfaces:direct] <#interfaces:direct>`__, Dakota includes
direct interfaces to Sandia’s Salinas code for structural dynamics,
Phoenix Integration’s ModelCenter framework, The Mathworks’ Matlab
scientific computing environment, Scilab (as described in
Section `1.4 <#advint:scilab>`__), and Python. While these can be
interfaced to with a script-based approach, some usability and
efficiency gains may be realized by re-compiling Dakota with these
direct interfaces enabled. Some details on Matlab and Python interfaces
are provided here. Note that these capabilities permit using Matlab or
Python to evaluate a parameter to response mapping; they do not make
Dakota algorithms available as a service, i.e., as a Matlab toolbox or
Python module.

.. _`advint:existingdirect:matlab`:

Matlab
~~~~~~

Dakota’s direct function interface includes the capability to invoke
Matlab for function evaluations, using the Matlab engine API. When using
this close-coupling, the Matlab engine is started once when Dakota
initializes, and then during analysis function evaluations are performed
exchanging parameters and results through the Matlab C API. This
eliminates the need to use the file system and the expense of
initializing the Matlab engine for each function evaluation.

The Dakota/Matlab interface has been built and tested on 32-bit Linux
with Matlab 7.0 (R14) and on 64-bit Linux with Matlab 7.1 (R14SP3).
Configuration support for other platforms is included, but is untested.
Builds on other platforms or with other versions of Matlab may require
modifications to Dakota including its build system

| To use the Dakota/Matlab interface, Dakota must be configured and
  compiled with the Matlab feature enabled. The Mathworks only provides
  shared object libraries for its engine API, so Dakota must be
  dynamically linked to at least the Matlab libraries. To compile Dakota
  with the Matlab interface enabled, set the CMake variable
  ``DAKOTA_MATLAB:BOOL=ON``, possibly with
  ``MATLAB_DIR:FILEPATH=/path/to/matlab``, where
| ``MATLAB_DIR`` is the root of your Matlab installation (it should be a
  directory containing directories bin/YOURPLATFORM and extern/include).

Since the Matlab libraries are linked dynamically, they must be
accessible at compile time and at run time. Make sure the path to the
appropriate Matlab shared object libraries is on your
``LD_LIBRARY_PATH``. For example to accomplish this in BASH on 32-bit
Linux, one might type

::

   export LD_LIBRARY_PATH=/usr/local/matlab/bin/glnx86:$LD_LIBRARY_PATH

or add such a command to the .bashrc file. Then proceed with compiling
as usual.

Example files corresponding to the following tutorial are available in .

Dakota/Matlab input file specification
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Matlab direct interface is specified with ``matlab`` keywords in an
interface specification. The Matlab m-file which performs the analysis
is specified through the ``analysis_drivers`` keyword. Here is a sample
Dakota ``interface`` specification:

::

     interface,
       matlab
         analysis_drivers = 'myanalysis.m'

Multiple Matlab analysis drivers are supported. Multiple analysis
components are supported as for other interfaces as described in
Section `[interfaces:components] <#interfaces:components>`__. The ``.m``
extension in the ``analysis_drivers`` specification is optional and will
be stripped by the interface before invoking the function. So
``myanalysis`` and ``myanalysis.m`` will both cause the interface to
attempt to execute a Matlab function ``myanalysis`` for the evaluation.

Matlab .m file specification
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Matlab analysis file ``myanalysis.m`` must define a Matlab function
that accepts a Matlab structure as its sole argument and returns the
same structure in a variable called ``Dakota``. A manual execution of
the call to the analysis in Matlab should therefore look like:

::

     >> Dakota = myanalysis(Dakota)

Note that the structure named Dakota will be pushed into the Matlab
workspace before the analysis function is called. The structure passed
from Dakota to the analysis m-function contains essentially the same
information that would be passed to a Dakota direct function included in
``DirectApplicInterface.C``, with fields shown in
Figure `[advint:figure:matlabparams] <#advint:figure:matlabparams>`__.

::

   Dakota.
     numFns              number of functions (responses, constraints)
     numVars             total number of variables
     numACV              number active continuous variables
     numADIV             number active discrete integer variables
     numADRV             number active discrete real variables
     numDerivVars        number of derivative variables specified in directFnDVV
     xC                  continuous variable values ([1 x numACV]) 
     xDI                 discrete integer variable values ([1 x numADIV])
     xDR                 discrete real variable values ([1 x numADRV])
     xCLabels            continuous var labels (cell array of numACV strings)
     xDILabels           discrete integer var labels (cell array of numADIV strings)
     xDRLabels           discrete real var labels (cell array of numADIV strings)
     directFnASV         active set vector ([1 x numFns])
     directFnDVV         derivative variables vector ([1 x numDerivVars])
     fnFlag              nonzero if function values requested
     gradFlag            nonzero if gradients requested
     hessFlag            nonzero if hessians requested
     currEvalId          current evaluation ID

The structure ``Dakota`` returned from the analysis must contain a
subset of the fields shown in
Figure `[advint:figure:matlabresponse] <#advint:figure:matlabresponse>`__.
It may contain additional fields and in fact is permitted to be the
structure passed in, augmented with any required outputs.

::

   Dakota.
     fnVals      ([1 x numFns], required if function values requested)
     fnGrads     ([numFns x numDerivVars], required if gradients  requested)
     fnHessians  ([numFns x numDerivVars x numDerivVars], 
                  required if hessians requested)
     fnLabels    (cell array of numFns strings, optional)
     failure     (optional: zero indicates success, nonzero failure

An example Matlab analysis driver ``rosenbrock.m`` for the Rosenbrock
function is shown in Figure
 `[advint:figure:matlabrosen] <#advint:figure:matlabrosen>`__.

::

   function Dakota = rosenbrock(Dakota)

     Dakota.failure = 0;

     if ( Dakota.numVars ~= 2 | Dakota.numADV | ...
         ( ~isempty( find(Dakota.directFnASM(2,:)) | ...
         find(Dakota.directFnASM(3,:)) ) & Dakota.numDerivVars ~= 2 ) )
       
       sprintf('Error: Bad number of variables in rosenbrock.m fn.\n');
       Dakota.failure = 1;

     elseif (Dakota.numFns > 2) 
     
       % 1 fn -> opt, 2 fns -> least sq
       sprintf('Error: Bad number of functions in rosenbrock.m fn.\n');
       Dakota.failure = 1;

     else
    
       if Dakota.numFns > 1 
         least_sq_flag = true;
       else
         least_sq_flag = false;
       end

       f0 = Dakota.xC(2)-Dakota.xC(1)*Dakota.xC(1);
       f1 = 1.-Dakota.xC(1);
     
       % **** f:
       if (least_sq_flag) 
         if Dakota.directFnASM(1,1)
           Dakota.fnVals(1) = 10*f0;
         end
         if Dakota.directFnASM(1,2)
           Dakota.fnVals(2) = f1;
         end
       else
         if Dakota.directFnASM(1,1)
           Dakota.fnVals(1) = 100.*f0*f0+f1*f1;
         end
       end
     
       % **** df/dx:
       if (least_sq_flag)
         if Dakota.directFnASM(2,1)
           Dakota.fnGrads(1,1) = -20.*Dakota.xC(1);
           Dakota.fnGrads(1,2) =  10.;
         end
         if Dakota.directFnASM(2,2)
           Dakota.fnGrads(2,1) = -1.;
           Dakota.fnGrads(2,2) =  0.;
         end
     
       else 
     
         if Dakota.directFnASM(2,1)
           Dakota.fnGrads(1,1) = -400.*f0*Dakota.xC(1) - 2.*f1;
           Dakota.fnGrads(1,2) =  200.*f0;
         end
         
       end

       % **** d^2f/dx^2:
       if (least_sq_flag)
        
         if Dakota.directFnASM(3,1)
           Dakota.fnHessians(1,1,1) = -20.;
           Dakota.fnHessians(1,1,2) = 0.;
           Dakota.fnHessians(1,2,1) = 0.;
           Dakota.fnHessians(1,2,2) = 0.;
         end
         if Dakota.directFnASM(3,2)
           Dakota.fnHessians(2,1:2,1:2) = 0.;
         end
         
       else
       
         if Dakota.directFnASM(3,1) 
           fx = Dakota.xC(2) - 3.*Dakota.xC(1)*Dakota.xC(1);
           Dakota.fnHessians(1,1,1) = -400.*fx + 2.0;
           Dakota.fnHessians(1,1,2) = -400.*Dakota.xC(1); 
           Dakota.fnHessians(1,2,1) = -400.*Dakota.xC(1);
           Dakota.fnHessians(1,2,2) =  200.;
         end
       
       end
     
       Dakota.fnLabels = {'f1'};
      
     end

.. _`advint:existingdirect:python`:

Python
~~~~~~

Dakota’s Python direct interface has been tested on Linux with Python 2
and 3. When enabled, it allows Dakota to make function evaluation calls
directly to an analysis function in a user-provided Python module. Data
may flow between Dakota and Python either in multiply-subscripted lists
or NumPy arrays.

| The Python direct interface must be enabled when compiling Dakota. Set
  the CMake variable
| ``DAKOTA_PYTHON:BOOL=ON``, and optionally
  ``DAKOTA_PYTHON_NUMPY:BOOL=ON`` (default is ON) to use Dakota’s NumPy
  array interface (requires NumPy installation providing arrayobject.h).
  If NumPy is not enabled, Dakota will use multiply-subscripted lists
  for data flow.

An example of using the Python direct interface with both lists and
arrays is included in . The Python direct driver is selected with, for
example,

::

     interface,
       python
         # numpy
         analysis_drivers = 'python_module:analysis_function'

where ``python_module`` denotes the module (file ) Dakota will attempt
to import into the Python environment and ``analysis_function`` denotes
the function to call when evaluating a parameter set. If the Python
module is not in the directory from which Dakota is started, setting the
``PYTHONPATH`` environment variable to include its location can help the
Python engine find it. The optional ``numpy`` keyword indicates Dakota
will communicate with the Python analysis function using numarray data
structures instead of the default lists.

Whether using the list or array interface, data from Dakota is passed
(via kwargs) into the user function in a dictionary containing the
entries shown in Table `1.1 <#advint:table:pythonparams>`__. The
``analysis_function`` must return a dictionary containing the data
specified by the active set vector with fields “fns”, “fnGrads”, and
“fnHessians”, corresponding to function values, gradients, and Hessians,
respectively. The function may optionally include a failure code in
“failure” (zero indicates success, nonzero failure) and function labels
in “fnLabels”. See the linked interfaces example referenced above for
more details.

.. container::
   :name: advint:table:pythonparams

   .. table:: Data dictionary passed to Python direct interface.

      +---------------------+-----------------------------------------------+
      | **Entry Name**      | **Description**                               |
      +=====================+===============================================+
      | functions           | number of functions (responses, constraints)  |
      +---------------------+-----------------------------------------------+
      | variables           | total number of variables                     |
      +---------------------+-----------------------------------------------+
      | cv                  | list/array of continuous variable values      |
      +---------------------+-----------------------------------------------+
      | div                 | list/array of discrete integer variable       |
      |                     | values                                        |
      +---------------------+-----------------------------------------------+
      | dsv                 | list of discrete string variable values       |
      |                     | (NumPy not supported)                         |
      +---------------------+-----------------------------------------------+
      | drv                 | list/array of discrete real variable values   |
      +---------------------+-----------------------------------------------+
      | av                  | single list/array of all numeric variable     |
      |                     | values (omits string variables)               |
      +---------------------+-----------------------------------------------+
      | cv_labels           | continuous variable labels                    |
      +---------------------+-----------------------------------------------+
      | div_labels          | discrete integer variable labels              |
      +---------------------+-----------------------------------------------+
      | dsv_labels          | discrete string variable labels               |
      +---------------------+-----------------------------------------------+
      | drv_labels          | discrete real variable labels                 |
      +---------------------+-----------------------------------------------+
      | av_labels           | all numeric variable labels                   |
      +---------------------+-----------------------------------------------+
      | asv                 | active set vector                             |
      +---------------------+-----------------------------------------------+
      | dvv                 | derivative variables vector                   |
      +---------------------+-----------------------------------------------+
      | analysis_components | list of analysis components as strings        |
      +---------------------+-----------------------------------------------+
      | currEvalId          | current evaluation ID number                  |
      +---------------------+-----------------------------------------------+

.. _`advint:scilab`:

Scilab Script and Direct Interfaces
-----------------------------------

Scilab is open source computation software which can be used to perform
function evaluations during Dakota studies, for example to calculate the
objective function in optimization. Dakota includes three Scilab
interface variants: scripted, linked, and compiled. In each mode, Dakota
calls Scilab to perform a function evaluation and then retrieves the
Scilab results. Dakota’s Scilab interface was contributed in 2011 by
Yann Collette and Yann Chapalain. The Dakota/Scilab interface variants
are described next.

Scilab Script Interface
~~~~~~~~~~~~~~~~~~~~~~~

Dakota distributions include a directory which demonstrates script-based
interfacing to Scilab. The ``Rosenbrock`` subdirectory contains four
notable files:

-  (the Dakota input file),

-  (the Scilab computation code),

-  (the analysis driver), and

-  (Scilab script).

The file specifies the Dakota study to perform. The interface type is
external (``fork``) and the shell script is the analysis driver used to
perform function evaluations.

The Scilab file accepts variable values and computes the objective,
gradient, and Hessian values of the Rosenbrock function as requested by
Dakota.

The is a short shell driver script, like that described in
Section `[interfaces:building] <#interfaces:building>`__, that Dakota
executes to perform each function evaluation. Dakota passes the names of
the parameters and results files to this script as ``$argv[1]`` and
``$argv[2]``, respectively. The is divided into three parts:
pre-processing, analysis, and post-processing.

In the analysis portion, the uses to extract the current variable values
from the input parameters file (``$argv[1]``) and communicate them to
the computation code in . The resulting objective function is
transmitted to Dakota via the output result file (``$argv[1]``), and the
driver script cleans up any temporary files.

The directory also includes PID and FemTRUSS examples, which are run in
a similar way.

Scilab Linked Interface
~~~~~~~~~~~~~~~~~~~~~~~

The Dakota/Scilab linked interface allows Dakota to communicate directly
with Scilab through in-memory data structures, typically resulting in
faster communication, as it does not rely on files or pipes. In this
mode, Dakota publishes a data structure into the Scilab workspace, and
then invokes the specified Scilab analysis_driver directly. In Scilab,
this structure is an mlist
(http://help.scilab.org/docs/5.3.2/en_US/mlist.html), with the same
fields as in the Matlab
interface `[advint:figure:matlabparams] <#advint:figure:matlabparams>`__,
with the addition of a field ``dakota_type``, which is used to validate
the names of fields in the data structure.

The linked interface is implemented in source files
``src/ScilabInterface.[CH]`` directory, and must be enabled at compile
time when building Dakota from source by setting
``DAKOTA_SCILAB:BOOL=ON``, and setting appropriate environment variables
at compile and run time as described in in . This directory also
contains examples for the Rosenbrock and PID problems.

A few things to note about these examples:

#. There is no shell driver script

#. The Dakota input file specifies the interface as ’scilab’, indicating
   a direct, internal interface to Scilab using the Dakota data
   structure described above:

   ::

      interface,
        scilab
          analysis_driver = 'rosenbrock.sci'

Scilab Compiled Interface
~~~~~~~~~~~~~~~~~~~~~~~~~

| In “compiled interface” mode, the Dakota analysis driver is a
  lightweight shim, which communicates with the running application code
  such as Scilab via named pipes. It is similar to that for Matlab in ,
  whose README is likely instructive. An example of a Scilab compiled
  interface is included in
| .

As with the other Scilab examples, there are computation code and Dakota
input files. Note the difference in the Dakota input file , where the
analysis driver starts the dakscilab shim program and always evaluates
functions, gradients, and Hessians.

::

   interface,
     fork
       analysis_driver = '../dakscilab -d -fp "exec fp.sci" -fpp "exec fpp.sci"'
       parameters_file = 'r.in'
       results_file = 'r.out'
       deactivate active_set_vector

The dakscilab executable results from compiling and has the following
behavior and options. The driver dakscilab launches a server. This
server then facilitates communication between Dakota and Scilab via
named pipes communication. The user can also use the first named pipe
(``${DAKSCILAB_PIPE}1``) to communicate with the server:

::

       echo dbg scilab_script.sce > ${DAKSCILAB_PIPE}1
       echo quit > ${DAKSCILAB_PIPE}1

The first command, with the keyword ’dbg’, launches the script for
evaluation in Scilab. It permits to give instructions to Scilab. The
second command ’quit’ stops the server.

The dakscilab shim supports the following options for the driver call:

#. -s to start the server

#. -si to run an init script

#. -sf to run a final script

#. -f -fp -fpp to specify names of objective function, gradient and
   hessian, then load them.

For the included PID example, the driver call is

::

       analysis_driver = '../dakscilab -d -si "exec init_test_automatic.sce;"
                        -sf "exec visualize_solution.sce;" -f "exec f_pid.sci"'

Here there is an initialization script () which is launched before the
main computation. It initializes a specific Scilab module called xcos. A
finalization script to visualize the xcos solution is also specified ().
Finally, the objective function is given with the computation code
called .

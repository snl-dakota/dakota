.. _running-examples-main:

""""""""""""""""
Running Examples
""""""""""""""""

To run Dakota with a particular input file, the following syntax can be used:

.. code-block::

    dakota -i dakota.in

or more simply:

.. code-block::

    dakota dakota.in

This will echo the standard output (stdout) and standard error (stderr) messages to the terminal. To redirect stdout and stderr
to separate files, the -o and -e command line options may be used:

.. code-block::

    dakota -i dakota.in -o dakota.out -e dakota.err

or

.. code-block::

    dakota -o dakota.out -e dakota.err dakota.in

Alternatively, any of a variety of Unix (or Windows) command-line
redirection variants can be used. The simplest of these redirects
stdout to another file:

.. code-block::

    dakota dakota.in > dakota.out

.. note::

   Command-line redirection will redirect *all* Dakota-generated
   output (or errors if specified) to the specified file, including
   output from underlying third-party libraries. In contrast, the
   command-line ``-output`` and ``-error`` (or analagous input file
   options) will only capture output from core Dakota and may not
   include some elements of console output.

To run the dakota process in the background, append an ampersand symbol (&) to the command with an embedded space, e.g.,\

.. code-block::

    dakota dakota.in > dakota.out &

Refer to :cite:p:`And86` for more information on Unix redirection and background commands.

The specified Dakota input file may instead be an dprepro/aprepro-style template file to be pre-processed prior to running
Dakota. For example it might contain template expressions in curly braces:

.. code-block::

	# {MyLB = 2.0} {MyUB = 8.6}
	variables
	  uniform_uncertain 3
		upper_bounds {MyUB} {MyUB} {MyUB}
		lower_bounds {MyLB} {MyLB} {MyLB}

.. Originally labelled as 'Section 10.9', which is part about pyprepro according to https://www.sandia.gov/app/uploads/sites/241/2023/03/Users-6.16.0.pdf

(See :ref:`dprepro- and pyprepro- specific instructions<interfaces:dprepro-and-pyprepro>` for more information and use cases.) To pre-process the input file, specify the -preproc flag which gen-
erates an intermediate temporary input file for use in Dakota. If Dakota’s pyprepro.py utility is not available on the execution
PATH and/or additional pre-processing options are needed, the tool location and syntax can be specified, for example:

.. code-block::

	# Assumes pyprepro.py is on PATH:
	dakota -i dakota_rosen.tmpl -preproc

	# Specify path/name of pre-processor:
	dakota -i dakota_rosen.tmpl \
	  -preproc "/home/user/dakota/bin/pyprepro"
	  
	# Specify Python interpreter to use, for example on Windows
	dakota -i dakota_rosen.tmpl -preproc "C:/python27/python.exe \
	  C:/dakota/6.10/bin/pyprepro/pyprepro.py"
	  
	# Specify additional options to pyprepro, e.g., include file:
	dakota -i dakota_rosen.tmpl -preproc "pyprepro.py -I default.params"

The “-pre run”, “-run”, and “-post run” options instruct Dakota to run one or more execution phases, excluding others.
For example pre-run might generate variable sets, run (core run) might invoke the simulation to evaluate variables and produce
responses, and post-run might accept variable/response sets and analyzes the results (for example, calculate correlations from
a set of samples). Currently only two modes are supported and only for sampling, parameter study, and DACE methods:

1. pre-run only with optional tabular output of variables:

   .. code-block::

	   dakota -i dakota.in -pre_run [::myvariables.dat]

2. post-run only with required tabular input of variables/responses:

   .. code-block::

	   dakota -i dakota.in -post_run myvarsresponses.dat::

.. _command-line-main:

""""""""""""""""""""
Command-Line Options
""""""""""""""""""""

The Dakota executable file is named ``dakota`` (``dakota.exe`` on Windows). If this command is entered at the command
prompt without any arguments, a usage message similar to the following appears:

.. code-block::

    usage: dakota [options and <args>]
    -help (Print this summary)
    -version [$val] (Print DAKOTA version information or build information related to $val string)
    -input <$val> (REQUIRED DAKOTA input file $val)
    -preproc [$val] (Pre-process input file with pyprepro or tool $val)
    -output <$val> (Redirect DAKOTA standard output to file $val)
    -error <$val> (Redirect DAKOTA standard error to file $val)
    -parser <$val> (Parsing technology: nidr[strict][:dumpfile])
    -no_input_echo (Do not echo DAKOTA input file)
    -check (Perform input checks)
    -pre_run [$val] (Perform pre-run (variables generation) phase)
    -run [$val] (Perform run (model evaluation) phase)
    -post_run [$val] (Perform post-run (final results) phase)
    -read_restart [$val] (Read an existing DAKOTA restart file $val)
    -stop_restart <$val> (Stop restart file processing at restart record $val)
    -write_restart [$val] (Write a new DAKOTA restart file $val)

Of these available command line inputs, only the ``-input`` option is required, and ``-input`` can be omitted if the input file name is the final item on the command line; all other command-line inputs are optional.

- The ``-help`` option prints the usage message above.
- The ``-version`` option prints the version number of the executable. With the optional argument will also print information about the configuration parameters related to that query, e.g., ``--version CXX`` will print information about the C++ compiler used.
- The ``-check`` option invokes a dry-run mode in which the input file is processed and checked for errors, but the study
  is not performed.
- The ``-input`` option provides the name of the Dakota input file, which can optionally be pre-processed as a template using the ``-preproc`` option.
- The ``-output`` and ``-error`` options provide file names for redirection of the Dakota standard output (stdout) and standard
  error (stderr), respectively. By default, Dakota will echo the input file to the output stream, but ``-no input echo`` can override this behavior.
- The ``-parser`` input is for debugging and will not be further described here.
- The ``-read restart`` and ``-write restart`` options provide the names of restart databases to read from and write to, respectively.
- The ``-stop restart`` option limits the number of function evaluations read from the restart database (the default is all the evaluations)
  for those cases in which some evaluations were erroneous or corrupted.

.. note::

   :ref:`Restart management <dakota_restart>` is an important (but advanced) technique for retaining data from expensive engineering applications.

.. note::

   Note that these command line options can be abbreviated so long as the abbreviation is unique. Accordingly, the
   following are valid, unambiguous specifications: ``-h``, ``-v``, ``-c``, ``-i``, ``-o``, ``-e``, ``-re``, ``-s``,
   ``-w``, ``-ru``, and ``-po`` and can be used in place of the longer forms of the command line options.

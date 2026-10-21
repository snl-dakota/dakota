.. _command-line-main:

""""""""""""""""""""
Command-Line Options
""""""""""""""""""""

The Dakota executable file is named ``dakota`` (``dakota.exe`` on Windows). If this command is entered at the command
prompt without any arguments, a usage message similar to the following appears:

.. code-block::

    usage: dakota [options and <args>]
        	-help (Print this summary)
        	-version [$val] (Print Dakota version number)
        	-input <$val> (Freeform Dakota input file $val)
        	-json <$val> (JSON Dakota input file $val)
        	-preproc [$val] (Pre-process freeform input file with pyprepro or tool $val)
        	-output <$val> (Redirect Dakota standard output to file $val)
        	-error <$val> (Redirect Dakota standard error to file $val)
        	-check (Perform input checks)
        	-dump_ir <$val> (Dump parsed IR / ProblemDescDB JSON to file $val)
        	-parser <$val> (Input parser: standard | nidr[strict][:dumpfile])
        	-no_input_echo (Do not echo Dakota input file)
        	-pre_run [$val] (Perform pre-run (variables generation) phase)
        	-run [$val] (Perform run (model evaluation) phase)
        	-post_run [$val] (Perform post-run (final results) phase)
        	-read_restart [$val] (Read an existing Dakota restart file $val)
        	-stop_restart <$val> (Stop restart file processing at evaluation $val)
        	-write_restart [$val] (Write a new DAKOTA restart file $val)
    
Of these available command line inputs, `-input` or `-json` option is required, and `-input` 
can be omitted if a freeform input file name is the final item on the command line; all other 
command-line inputs are optional.

- The ``-help`` option prints the usage message above.
- The ``-version`` option prints the version number of the executable. With the optional
  argument will also print information about the configuration parameters related to that
  query, e.g., ``--version CXX`` will print information about the C++ compiler used.
- The ``-input`` option provides the name of a freeform Dakota input file, which can
  optionally be pre-processed as a template using the ``-preproc`` option.
- The ``-json`` option provides the name of a JSON format Dakota input file. It is mutually exclusive
  with the ``-input`` option. JSON format input files are not pre-processed.
- The ``-preproc`` option directs Dakota to pre-process the input file as a template using
  :ref:`pyprepro <interfaces:dprepro-and-pyprepro>` before executing the study. Only freefrom
  input files can be pre-processed.
- The ``-check`` option invokes a dry-run mode in which the input file is processed and
  checked for errors, but the study is not performed.
- The ``-dump_ir`` option causes the intermediate representation of the user input to
  be dumped to a JSON file. This is primarily for use by developers.
- The ``-parser`` input selects between the default `standard` and deprecated `nidr` input file readers.
- The ``-output`` and ``-error`` options provide file names for redirection of the
  Dakota standard output (stdout) and standard error (stderr), respectively. By default,
  Dakota will echo the input file to the output stream, but ``-no input echo`` can override this behavior.
- The ``-read restart`` and ``-write restart`` options provide the names of restart
  databases to read from and write to, respectively.
- The ``-stop restart`` option limits the number of function evaluations read
  from the restart database (the default is all the evaluations)
  for those cases in which some evaluations were erroneous or corrupted.

.. note::

   :ref:`Restart management <dakota_restart>` is an important (but advanced) technique for retaining data from expensive engineering applications.

.. note::

   Note that these command line options can be abbreviated so long as the abbreviation is unique. Accordingly, the
   following are valid, unambiguous specifications: ``-h``, ``-v``, ``-c``, ``-i``, ``-o``, ``-e``, ``-re``, ``-s``,
   ``-w``, ``-ru``, and ``-po`` and can be used in place of the longer forms of the command line options.

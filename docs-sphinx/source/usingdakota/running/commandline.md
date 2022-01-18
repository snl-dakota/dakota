Command-Line Options
============

The Dakota executable file is named dakota (dakota.exe on Windows). If this command is entered at the command
prompt without any arguments, a usage message similar to the following appears:

```
usage: dakota [options and <args>]
-help (Print this summary)
-version (Print DAKOTA version number)
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
-stop_restart <$val> (Stop restart file processing at evaluation $val)
-write_restart [$val] (Write a new DAKOTA restart file $val)
```

Of these available command line inputs, only the “-input” option is required, and “-input” can be omitted if the input
file name is the final item on the command line; all other command-line inputs are optional. The “-help” option prints the
usage message above. The “-version” option prints the version number of the executable. The “-check” option invokes
a dry-run mode in which the input file is processed and checked for errors, but the study is not performed. The “-input”
option provides the name of the Dakota input file, which can optionally be pre-processed as a template using the “-preproc”
option.

The “-output” and “-error” options provide file names for redirection of the Dakota standard output (stdout) and standard
error (stderr), respectively. By default, Dakota will echo the input file to the output stream, but “-no input echo” can
override this behavior.

The “-parser” input is for debugging and will not be further described here. The “-read restart” and “-write restart”
options provide the names of restart databases to read from and write to, respectively. The “-stop restart” option limits
the number of function evaluations read from the restart database (the default is all the evaluations) for those cases in which
some evaluations were erroneous or corrupted. Restart management is an important technique for retaining data from expen-
sive engineering applications. This advanced topic is discussed in detail in Chapter 18. Note that these command line options
can be abbreviated so long as the abbreviation is unique. Accordingly, the following are valid, unambiguous specifications:
“-h”, “-v”, “-c”, “-i”, “-o”, “-e”, “-re”, “-s”, “-w”, “-ru”, and “-po” and can be used in place of the longer forms of
the command line options.

To run Dakota with a particular input file, the following syntax can be used:
```
dakota -i dakota.in
```

or more simply:

```
dakota dakota.in
```

This will echo the standard output (stdout) and standard error (stderr) messages to the terminal. To redirect stdout and stderr
to separate files, the -o and -e command line options may be used:

```
dakota -i dakota.in -o dakota.out -e dakota.err
```

or

```
dakota -o dakota.out -e dakota.err dakota.in
```

Alternatively, any of a variety of Unix redirection variants can be used. The simplest of these redirects stdout to another file:

```
dakota dakota.in > dakota.out
```
To run the dakota process in the background, append an ampersand symbol (&) to the command with an embedded space, e.g.,\

```
dakota dakota.in > dakota.out &
```

Refer to [5] for more information on Unix redirection and background commands.

The specified Dakota input file may instead be an dprepro/aprepro-style template file to be pre-processed prior to running
Dakota. For example it might contain template expressions in curly braces:

```
# {MyLB = 2.0} {MyUB = 8.6}
variables
  uniform_uncertain 3
    upper_bounds {MyUB} {MyUB} {MyUB}
    lower_bounds {MyLB} {MyLB} {MyLB}
```

(See Section 10.9 for more information and use cases.) To pre-process the input file, specify the -preproc flag which gen-
erates an intermediate temporary input file for use in Dakota. If Dakota’s pyprepro.py utility is not available on the execution
PATH and/or additional pre-processing options are needed, the tool location and syntax can be specified, for example:

```
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
```

The “-pre run”, “-run”, and “-post run” options instruct Dakota to run one or more execution phases, excluding others.
For example pre-run might generate variable sets, run (core run) might invoke the simulation to evaluate variables and produce
responses, and post-run might accept variable/response sets and analyzes the results (for example, calculate correlations from
a set of samples). Currently only two modes are supported and only for sampling, parameter study, and DACE methods: (1)
pre-run only with optional tabular output of variables:

```
dakota -i dakota.in -pre_run [::myvariables.dat]
```

and (2) post-run only with required tabular input of variables/responses:

```
dakota -i dakota.in -post_run myvarsresponses.dat::
```
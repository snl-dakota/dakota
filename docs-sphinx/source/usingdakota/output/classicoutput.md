Examples of Dakota Output
=======================================

Beyond numerical results, all output files provide information that allows the user to check that the actual analysis was the
intended analysis. More details on all outputs can be found in Chapter 13.

## Screen output saved to a file

Whenever an output file is specified for a Dakota run, the screen output itself becomes quite minimal consisting of version
statements, environment statements and execution times.

## Output file
The output file is much more extensive, because it contains information on every function evaluation:

```
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
```

Excluding the copy of the input file at the beginning and timing information at the end, the file is organized into three basic
parts:

1. Information on the problem
For this example, we see that a new restart file is being created and Dakota has carried out a multi-
dim parameter study with 8 partitions for each of two variables.
2. Information on each function evaluation
Each function evaluation is numbered. Details for function evaluation 1 show that at input variable values
x1 = −2.0 and x2 = −2.0, the direct rosenbrock function is being evaluated. There is one response with
a value of 3.609e+03.
3. Summary statistics
The function evaluation summary is preceded by `<<<<<`. For this example 81 total evaluations were
assessed; all were new, none were read in from the restart file. Correlation matrices complete the statistics
and output for this problem. Successful runs will finish with `<<<<< Iterator study type completed.`

## Tabular output file
For this example, the default name for the tabular output file dakota_tabular.dat was changed in the input file to
rosen_multidim.dat. This tab-delimited text file summarizes the inputs and outputs to the function
evaluator:

```
%eval_id   interface   x1     x2   response_fn_1
       1       NO_ID   -2     -2            3609
       2       NO_ID   -1.5   -2          1812.5
       3       NO_ID   -1     -2             904
       4       NO_ID   -0.5   -2           508.5
```

The first line contains the names of the variables and responses, as well as headers for the evaluation id and interface
columns.

```
%eval_id interface x1 x2 response_fn_1
```

The number of function evaluations will match the number of evaluations listed in the summary part of the output file for single
method approaches; the names of inputs and outputs will match the descriptors specified in the input file. The interface
column is useful when a Dakota input file contains more than one simulation interface. In this instance, there is only one, and
it has no id interface specified, so Dakota has supplied a default value of NO ID. This file is ideal for import into other
data analysis packages.
.. _`interfaces:batch`:

"""""""""""""""""""
Batched Evaluations
"""""""""""""""""""

Dakota provides for execution of evaluations in batches.
Batch mode is intended to allow a user to assume greater control over where
and when to run individual evaluations. It is activated using the :dakkw:`interface-batch` keyword.

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
size may be limited using the :dakkw:`interface-batch-size` subkeyword. Setting
``size = 100`` would result in 10 equal-size batches being run one after
another in a 1000-sample study.

File Formats
~~~~~~~~~~~~ 

For :ref:`standard <variables:parameters:standard>`
and :ref:`APREPRO <variables:parameters:aprepro>`-format parameters files,
the combined parameters file for a batch is simply the concatenation of 
the paramters information for all the evaluations in the batch, in standard 
or APREPRO format.

The following example parameters file contains parameter sets for two
evaluations.

.. code-block::

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

A :ref:`JSON format <variables:parameters:json>` batch parameters file contains an array at the top level.
The elements of the array are objects that contain parameters information
for each evaluation in the batch.

The combined results file format is likewise a concatenation of the
results for all the evaluations in the batch. However, a line beginning
with the “#” character must separate the results for each evaluation.

The order of the evaluations in the results file must match the order in
the parameters file.

The following is an example batch results file corresponding to the
batch parameters file above. The initial # on the first line is
optional, and a final # (not shown here) is allowed.

.. code-block::

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

Batch mode restricts the use of several other Dakota features:

-  No :dakkw:`interface-analysis_drivers-input_filter` or
   :dakkw:`interface-analysis_drivers-output_filter` is allowed.
-  Only one :dakkw:`interface-analysis_drivers` keyword is permitted.
-  :dakkw:`interface-failure_capture` modes are limited to abort and recover.
-  Asynchronous evaluation is disallowed (only one batch at a time may
   be executed).

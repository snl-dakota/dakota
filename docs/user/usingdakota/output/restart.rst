.. _dakota_restart:

""""""""""""""
Dakota Restart
""""""""""""""

Dakota is often used to solve problems that require repeatedly running computationally expensive simulation codes. In some cases you may want to repeat
an optimization study, but with a tighter final convergence tolerance. This would be costly if the entire optimization analysis had to be repeated.
Interruptions imposed by computer usage policies, power outages, and system failures could also result in costly delays. However, Dakota automatically
records the variable and response data from all function evaluations so that new executions of Dakota can pick up where previous executions left off.
The Dakota restart file ("**dakota.rst**" by default) archives the tabulated interface evaluations in a binary format. The primary restart commands at
the command line are ``-read_restart``, ``-write_restart``, and ``-stop_restart``.

=====================
Writing Restart Files
=====================

To write a restart file using a particular name, the ``-write_restart`` command line input (may be abbreviated as ``-w``) is used:

.. code_block:

   dakota -i dakota.in -write_restart my_restart_file

If no ``-write_restart`` specification is used, then Dakota will still write a restart file, but using the default name "dakota.rst" instead of a user-specified name.

To turn restart recording off, the user may use the restart_file keyword, in the interface block. This can increase execution speed and reduce disk storage requirements,
but at the expense of a loss in the ability to recover and continue a run that terminates prematurely. This option is not recommended when function evaluations are costly
or prone to failure. Please note that using the deactivate restart_file specification will result in a zero length restart file with the default name "dakota.rst",
which can overwrite an exiting file.

===================
Using Restart Files
===================

To restart Dakota from a restart file, the ``-read_restart`` command line input (may be abbreviated as ``-r``) is used:

.. code_block:

   dakota -i dakota.in -read_restart my_restart_file

If no -read_restart specification is used, then Dakota will not read restart information from any file (i.e., the default is no restart processing).

To read in only a portion of a restart file, the -stop_restart control (may be abbreviated as -s) is used to specify the number of entries to be read from the database. Note that this
integer value corresponds to the restart record processing counter (as can be seen when using the print utility (see The Dakota Restart Utility) which may differ from the evaluation numbers
used in the previous run if, for example, any duplicates were detected (since these duplicates are not recorded in the restart file). In the case of a -stop_restart specification, it is usually
desirable to specify a new restart file using -write_restart so as to remove the records of erroneous or corrupted function evaluations. For example, to read in the first 50 evaluations from dakota.rst:

.. code_block:

   dakota -i dakota.in -r dakota.rst -s 50 -w dakota_new.rst

The dakota_new.rst file will contain the 50 processed evaluations from dakota.rst as well as any new evaluations. All evaluations following the 50th in "dakota.rst"
have been removed from the latest restart record.

===========================
Appending to a Restart File
===========================

If the ``-write_restart`` and ``-read_restart`` specifications identify the same file (including the case where ``-write_restart`` is not specified and ``-read_restart`` identifies "dakota.rst"),
then new evaluations will be appended to the existing restart file.

===================================
Working with multiple Restart Files
===================================

If the ``-write_restart`` and ``-read_restart`` specifications identify different files, then the evaluations read from the file identified by ``-read_restart`` are first written
to the ``-write_restart`` file. Any new evaluations are then appended to the -write_restart file. In this way, restart operations can be chained together indefinitely with the
assurance that all of the relevant evaluations are present in the latest restart file.

============
How it Works
============

Dakota's restart algorithm relies on its duplicate detection capabilities. Processing a restart file populates the list of function evaluations that have been performed.
Then, when the study is restarted, it is started from the beginning (not a warm start) and many of the function evaluations requested by the iterator are intercepted by
the duplicate detection code. This approach has the primary advantage of restoring the complete state of the iteration (including the ability to correctly detect subsequen
duplicates) for all methods/iterators without the need for iterator-specific restart code. However, the possibility exists for numerical round-off error to cause a divergence
between the evaluations performed in the previous and restarted studies. This has been rare in practice. 
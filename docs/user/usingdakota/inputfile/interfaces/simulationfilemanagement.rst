.. _`interfaces:file`:

""""""""""""""""""""""""""
Simulation File Management
""""""""""""""""""""""""""

This section describes some management features used for files that
transfer data between Dakota and simulation codes (i.e., when the system
call or fork interfaces are used). These features can generate unique
filenames when Dakota executes programs in parallel and can help you
debug the interface between Dakota and a simulation code.

.. _`interfaces:file:saving`:

-----------
File Saving
-----------

The :dakkw:`file_save` option in the interface specification allows the user
to control whether parameters and results files are retained or removed from
the working directory after the analysis completes. Dakota’s default behavior
is to remove files once their use is complete to reduce clutter. If the method
output setting is verbose, a file remove notification will follow the function
evaluation echo, e.g.,

.. code-block::

   driver /usr/tmp/aaaa20305 /usr/tmp/baaa20305
   Removing /usr/tmp/aaaa20305 and /usr/tmp/baaa20305

However, if ``file_save`` appears in the interface specification, these
files will not be removed. This latter behavior is often useful for
debugging communication between Dakota and simulator programs. An
example of a ``file_save`` specification is shown in the :ref:`file tagging
example <interfaces:file:tagging1>` below.

.. note::

   Before driver execution, any previous results file will be removed
   immediately prior to executing the analysis driver. This behavior
   addresses a previously common problem resulting from users starting
   Dakota with stale results files in the run directory. To override
   this default behavior and preserve any existing results files,
   you must specify :dakkw:`interface-analysis_drivers-fork-allow_existing_results`.



.. _`interfaces:file:tagging1`:

----------------------------
File Tagging for Evaluations
----------------------------

When a user provides :dakkw:`interface-analysis_drivers-fork-parameters_file`
and :dakkw:`interface-analysis_drivers-fork-results_file` specifications,
the :dakkw:`interface-analysis_drivers-fork-file_tag` option in the interface specification
causes Dakota to make the names of these files unique by appending the
function evaluation number to the root file names. Default behavior is
to not tag these files, which has the advantage of allowing the user to
ignore command line argument passing and always read to and write from
the same file names. However, it has the disadvantage that files may be
overwritten from one function evaluation to the next. When ``file_tag``
appears in the interface specification, the file names are made unique
by the appended evaluation number. This uniqueness requires the user’s
interface to get the names of these files from the command line. The
file tagging feature is most often used when concurrent simulations are
running in a common disk space, since it can prevent conflicts between
the simulations. An example specification of ``file_tag`` and
:dakkw:`interface-analysis_drivers-fork-file_save` is shown below:

.. code-block::

   interface
       system
           analysis_driver = 'text_book'
           parameters_file = 'text_book.in'
           results_file    = 'text_book.out'
           file_tag
		   file_save

.. note::

   When a user specifies names for the parameters and
   results files and ``file_save`` is used without ``file_tag``, untagged
   files are used in the function evaluation but are then moved to tagged
   files after the function evaluation is complete, to prevent overwriting
   files for which a ``file_save`` request has been given. If the output
   control is set to verbose, then a notification similar to the following
   will follow the function evaluation echo:

   .. code-block::

      driver params.in results.out
      Files with non-unique names will be tagged to enable file_save:
      Moving params.in to params.in.1
      Moving results.out to results.out.1

Hierarchical Tagging
--------------------

When a model’s specification includes the
:dakkw:`model-hierarchical_tagging` keyword, the tag applied to parameter and
results file names of any subordinate interfaces will reflect any model
hierarchy present. This option is useful for studies involving multiple
models with a nested or hierarchical relationship. For example a nested
model has a sub-method, which itself likely operates on a sub-model, or
a hierarchical approximation involves coordination of low and high
fidelity models. Specifying ``hierarchical_tagging`` will yield function
evaluation identifiers (“tags”) composed of the evaluation IDs of the
models involved, e.g., ``outermodel.innermodel.interfaceid = 4.9.2``. This
communicates the outer contexts to the analysis driver when performing a
function evaluation.

For an example of using hierarchical tagging in a nested model context,
see ``dakota/share/dakota/test/dakota_uq_timeseries_*_optinterf.in``.

.. _`interfaces:file:temporary`:

---------------
Temporary Files
---------------

If :dakkw:`interface-analysis_drivers-fork-parameters_file`
and :dakkw:`interface-analysis_drivers-fork-results_file` are not specified by the
user, temporary files having generated names are used. For example, a
system call to a single analysis driver might appear as:

.. code-block::

   driver /tmp/dakota_params_aaaa2035 /tmp/dakota_results_baaa2030

and a system call to an analysis driver with filter programs might
appear as:

.. code-block::

   ifilter /tmp/dakota_params_aaaa2490 /tmp/dakota_results_baaa2490;
        driver /tmp/dakota_params_aaaa2490 tmp/dakota_results_baaa2490;
        ofilter /tmp/dakota_params_aaaa2490 /tmp/dakota_results_baa22490

These files have unique names created by Boost filesystem utilities.
This uniqueness requires the user’s interface to get the names of these
files from the command line. File tagging with evaluation number is
unnecessary with temporary files, but can be helpful for the user
workflow to identify the evaluation number. Thus :dakkw:`interface-analysis_drivers-fork-file_tag`
requests will be honored. A :dakkw:`interface-analysis_drivers-fork-file_save`
request will be honored, but it should be used with care since the temporary file
directory could easily become cluttered without the user noticing.

.. _`interfaces:file:tagging2`:

---------------------------------
File Tagging for Analysis Drivers
---------------------------------

When multiple analysis drivers are involved in performing a function
evaluation with either the system call or fork simulation interface, a
secondary file tagging is *automatically* used to distinguish the
results files used for the individual analyses. This applies to both the
case of user-specified names for the parameters and results files and
the default temporary file case. Examples for the former case were shown
previously in the sections on multiple analysis drivers
:ref:`without filters <interfaces:components:multiple1>` and
:ref:`with filters <interfaces:components:multiple2>`.

The following examples demonstrate the latter temporary file case. Even though Unix
temporary files have unique names for a particular function evaluation,
tagging is still needed to manage the individual contributions of the
different analysis drivers to the response results, since the same root
results filename is used for each component. For the system call
interface, the syntax would be similar to the following:

.. code-block::

   ifilter /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ;
        driver1 /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ.1;
        driver2 /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ.2;
        driver3 /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ.3;
        ofilter /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ

and, for the fork interface, similar to:

.. code-block::

   blocking fork:
        ifilter /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ;
        driver1 /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ.1;
        driver2 /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ.2;
        driver3 /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ.3;
        ofilter /var/tmp/aaawkaOKZ /var/tmp/baaxkaOKZ

Tagging of results files with an analysis identifier is needed since
each analysis driver must contribute a user-defined subset of the total
response results for the evaluation. If an output filter is not
supplied, Dakota will combine these portions through a simple overlaying
of the individual contributions (i.e., summing the results in ``/var/tmp/baaxkaOKZ.1``,
``/var/tmp/baaxkaOKZ.2``, and ``/var/tmp/baaxkaOKZ.3``).

If this simple approach is inadequate, then an output filter should be
supplied to perform the combination. This is the reason why the results
file for the output filter does not use analysis tagging; it is
responsible for the results combination (i.e., combining ``/var/tmp/baaxkaOKZ.1``,
``/var/tmp/baaxkaOKZ.2``, and ``/var/tmp/baaxkaOKZ.3`` into ``/var/tmp/baaxkaOKZ``).
In this case, Dakota will read only the results file from the output
filter (i.e., ``/var/tmp/baaxkaOKZ``) and interpret it as the total response set for the
evaluation.

Parameters files are not currently tagged with an analysis identifier.
This reflects the fact that Dakota does not attempt to subdivide the
requests in the active set vector for different analysis portions.
Rather, the total active set vector is passed to each analysis driver
and the appropriate subdivision of work *must be defined by the user*.
This allows the division of labor to be very flexible. In some cases,
this division might occur across response functions, with different
analysis drivers managing the data requests for different response
functions. And in other cases, the subdivision might occur within
response functions, with different analysis drivers contributing
portions to each of the response functions. The only restriction is that
each of the analysis drivers must follow the response format dictated by
the total active set vector. For response data for which an analysis
driver has no contribution, 0’s must be used as placeholders.

.. _`interfaces:workdir`:

----------------
Work Directories
----------------

Sometimes it is convenient for simulators and filters to run in a
directory different from the one where Dakota is invoked. For instance,
when performing concurrent evaluations and/or analyses, it is often
necessary to cloister input and output files in separate directories to
avoid conflicts. A simulator script used as an :ref:`analysis driver <interface-analysis_drivers>`
can, of course, include commands to change to a different directory if desired
(while still arranging to write a results file in the original
directory), but Dakota has facilities that may simplify the creation of
simulator scripts.

When the :ref:`work directory <interface-analysis_drivers-fork-work_directory>` feature is enabled,
Dakota will create a directory for each evaluation/analysis (with
optional tagging and saving as with files). To enable this feature,
an interface specification must include the keyword :dakkw:`interface-analysis_drivers-fork-work_directory`,
then Dakota will arrange for the simulator and any filters to wake up in
the work directory, with $PATH adjusted (if necessary) so programs that
could be invoked without a relative path to them (i.e., by a name not
involving any slashes) from Dakota’s directory can also be invoked from
the simulator’s (and filter’s) directory.

On occasion, it is convenient
for the simulator to have various files, e.g., data files, available in
the directory where it runs. If, say, ``my/special/directory/`` is such a directory (as seen from
Dakota’s directory), the interface specification

.. code-block::

   work_directory
       named 'my/special/directory'

would cause Dakota to start the simulator and any filters in that
directory. If the directory did not already exist, Dakota would create
it and would remove it after the simulator (or output filter, if
specified) finished, unless instructed not to do so by the appearance of
:dakkw:`interface-analysis_drivers-fork-work_directory-directory_save`
in the interface specification. If :dakkw:`interface-analysis_drivers-fork-work_directory-named`
does not appear, then ``directory_save`` cannot appear either, and Dakota creates a temporary
directory (using the ``tmpnam`` function to determine its name) for use
by the simulator and any filters. If you specify
:dakkw:`interface-analysis_drivers-fork-work_directory-directory_tag`,
Dakota causes each invocation of the
simulator and any filters to start in a subdirectory of the work
directory with a name composed of the work directory’s name followed by
a period and the invocation number (1, 2, :math:`...`); this might be
useful in debugging.

Sometimes it can be helpful for the simulator and filters to start in a
new directory populated with some files. Adding

.. code-block::

   link_files 'templatedir/*'

to the work directory specification would cause the contents of
directory ``templatedir/`` to be linked into the work directory. Linking makes sense if
files are large, but when practical, it is far more reliable to have
copies of the files; adding :dakkw:`interface-analysis_drivers-fork-work_directory-copy_files`
to the specification would cause the contents of the template directory to be copied to the work
directory. The linking or copying does not overwrite existing files
unless :dakkw:`interface-analysis_drivers-fork-work_directory-replace` also
appears in the specification.

Here is a summary of possibilities for a work directory specification,
with ``[...]`` denoting that :math:`...` is optional:

.. code-block::

   work_directory
   [ named '...' ]
   [ directory_tag ]
   [ directory_save ]
   [ link_files '...' '...' ]
   [ copy_files '...' '...' ]
   [ replace ]

:numref:`fig:interface:workdir` contains an
example of these specifications in a Dakota input file for constrained
optimization.

.. literalinclude:: ../../samples/workdir_textbook.in
   :language: dakota
   :tab-width: 2
   :caption: The ``workdir_textbook.in`` input file.
   :name: fig:interface:workdir

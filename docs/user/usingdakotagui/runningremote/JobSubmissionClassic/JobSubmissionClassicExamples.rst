.. _gui-job-submission-classic-examples:

""""""""""""""""""""""""""""""
Dakota Job Submission Examples
""""""""""""""""""""""""""""""

The following page gives some specific scripting examples for performing Dakota remote job submission using the classic Job Submission dialog.

----------------------------------
Prerequisites for Non-Sandia Users
----------------------------------

If you use Dakota GUI's job submission feature outside of Sandia, you will not be provided with any default machine definitions or execution templates, and will need to write your own.
However, one machine code, "DAKOTA," is already provided for you - it can be viewed under "Window > Preferences > Job Submission > Codes."

- :ref:`You will need at least one machine definition. <gui-job-submission-classic-creation-machine>`
- :ref:`You should create at least one execution template, especially if you anticipate submitting multiple, similar jobs. <gui-job-submission-classic-exectemplate>`

---------------------------------------------------------------------------
Example 1: Job Submission to a Non-Queued Machine with a Basic Dakota Study
---------------------------------------------------------------------------

.. note::

   Read about the :ref:`definition of a non-queued machine <gui-job-submission-overview-definition-nonqueued>` before proceeding.

Initial Setup
-------------

First, in the Project Explorer, right-click the Dakota study you would like to submit to a remote machine. Choose "Run As > Run Configurations."
Then, in the Run Configurations dialog, double-click "Job Submission" in the configuration type list on the left side of the dialog.

If all goes well, your job submission run configuration should default select the "DAKOTA" code, the first machine you defined, and the first execution template you defined, if you defined an execution template.

Resources Tab
-------------

- In the "Input File" field, verify that your Dakota .in file has been selected.
- In the larger "Input Files" field, check the checkboxes for any associated project files that you would like to have sent to the remote machine selected in the "Machine" dropdown.
- In the "Output Files" field, create filename patterns for anything that you want the GUI to retrieve from the remote machine after the job has completed. This field accepts wildcards for filename patterns.

Machine Tab
-----------

- In the "Job Run Directory" field, set the path on the remote machine that you would like the job to run in. Note that the path must be absolute: it should start with something like ``/home/${username}/``, where "username"
  is the username on the remote machine that you would like the job to run under.
- Check the "Clear job run directory" field if you would like the job to clean up any files in the remote run directory before starting.
- Ignore the "Queue" properties, since we are targeting a non-queued machine in this example.

Execution Instructions Tab
--------------------------

Let's skip the "Code Parameters" tab for now and go to "Execution Instructions."

**Job Commands:**

Copy-paste the following script into the "Job Commands" field:

.. code-block::

    #!/bin/sh

    cd ${remote.dir} || exit 9
    if [ -f /etc/bashrc ]; then
        source /etc/bashrc
    fi
	
    module load dakota
    dakota -input ${input.deck.base.name}.in 1>${input.deck.base.name}.out 2>&1

Note the presence of ``${variable}`` syntax. These are variables which we must define as part of the job submission configuration. Some variables are provided for you by the job submission framework,
but others are Dakota-specific and must be manually defined. Assuming that you have not modified the previous script, you will need to define the following variables:

- preprocessor
- DAKOTA_APPLIC_CONCURRENCY
- clean_subdirectory
- work_subdirectory

To add a custom variable:

- Click on the "Variables..." button to the bottom-right of the "Job Commands" field (take care to click the correct "Variables..." button, as there are multiple.
- In the "Select Variable" pop-up that follows, click "Add."
- In the "Variable Definition" pop-up, fill out each field:

  - *Name:* The name of your variable; in other words, what goes between ``${`` and ``}``.
  - *Label:* A human-readable label for your variable, which will be displayed on the Code Parameters tab of this job submission configuration.
  - *Description:* A written description of the variable, if you wish to provide one.
  - *Required:* Whether the job submission configuration should require that this variable be filled out.
  - *Persisted:* Whether the variable value is persisted.
  - *Type:* The type represented by the variable.
  - *Default Value:* A default value for the variable.
  
Now, fill out custom variable definitions for "preprocessor," "DAKOTA_APPLIC_CONCURRENCY," "clean_subdirectory," and "work_subdirectory."

Code Parameters Tab
-------------------

Once you have defined your custom variables on the "Execution Instructions" tab, return to the third tab - "Code Parameters." You will note that now, there are generated fields in which you can enter values
for each custom variable you defined. If you did not provide a default value for any of these variables, you may provide values on this tab (this is admittedly more readable than using variable default values).

---------------------------------------------
Example 2: Job Submission to a Queued Machine
---------------------------------------------

.. note::

   Read about the :ref:`definition of a queued machine <gui-job-submission-overview-definition-queued>` before proceeding.

Initial Setup
-------------

First, in the Project Explorer, right-click the Dakota study you would like to submit to a remote machine. Choose "Run As > Run Configurations."
Then, in the Run Configurations dialog, double-click "Job Submission" in the configuration type list on the left side of the dialog.

If all goes well, your job submission run configuration should default select the "DAKOTA" code, the first machine you defined, and the first execution template you defined, if you defined an execution template.

Resources Tab
-------------

TODO

Execution Instructions Tab
--------------------------

TODO

**Job commands:**

.. code-block::

   cd ${remote.dir} || exit 9

   if [ -f /etc/bashrc ]; then
       source /etc/bashrc
   fi

   chmod 744 *.sh
   dos2unix *.sh

   echo submitting to the ${queue} queue
   sbatch -N ${num.nodes} --partition=${queue} ${qos} --time=${job.hours}:${job.minutes}:${job.seconds} -A ${account} ${script.name} ${capture.job.id}

**Script contents:**

.. code-block::

    #!/bin/sh

    cd ${remote.dir} || exit 9

    if [ -f /etc/bashrc ]; then
        source /etc/bashrc
    fi

    export RELATIVE_LOCATION=..
    export PREPROCESSOR=${preprocessor}
    export APREPRO="module load seacas && aprepro"
    export DAKOTA_PPN=${ppn} 
    export DAKOTA_APPLIC_CONCURRENCY=${evaluation_concurrency}
    export DAKOTA_APPLIC_PROCS=$((${num.processors}/DAKOTA_APPLIC_CONCURRENCY))

    echo "Checking that constraint between number of processors, number of concurrent runs, and number of processors per run is respected..."
    remainder=$((${num.processors}%DAKOTA_APPLIC_CONCURRENCY))
    if [ $remainder -gt 0 ];
    then
        echo "${num.processors} must be a multiple of $DAKOTA_APPLIC_CONCURRENCY"
        echo "Exiting..."
        exit
    fi

    echo "INFO: DAKOTA will manage ${DAKOTA_APPLIC_CONCURRENCY} concurrent application jobs, each running on ${DAKOTA_APPLIC_PROCS} cores, with ${DAKOTA_PPN} processes per node"

    if [ ${clean_subdirectory} -eq 0 ]; then
        num_dirs=`find . -name ${work_subdirectory}.* -type d | wc -l`
        if [ $num_dirs -gt 0 ]; then
          rm -r ${work_subdirectory}.*
        fi
    fi

    module load dakota
    dakota -input ${input.deck.base.name}.in 1>${input.deck.base.name}.out 2>&1

TODO

Code Parameters
---------------

TODO
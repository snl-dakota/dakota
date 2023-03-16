.. _ngw-node-dakotaQueueSubmit:

.. _gui-job-submission-workflow-dakotaQueueSubmit:

=================
dakotaQueueSubmit
=================

This node is responsible for running Dakota. However, unlike the regular :ref:`dakota <ngw-node-dakota>` node, the dakotaQueueSubmit node should only be
used in the context of a workflow that is :ref:`running on a login node belonging to a high-performance computer with a job queue <gui-job-submission-workflow-example2>`.

Most of the dakotaQueueSubmit node's behavior in-queue can be controlled via editable **control scripts** that are provided with the node:

1. Submit a job into the queue (``submit-dakota.sh``)
2. Check on the job's overall status in the queue (``status-dakota.sh``)
3. Drill into a more detailed description of the job state (``checkjob-dakota.sh``)
4. Cancel the job (``cancel-dakota.sh``)
5. Additionally, dakotaQueueSubmit provides a "runner" script for Dakota itself (``runDakotaRemote.sh``) that is actually responsible for launching Dakota.

:ref:`Care should be taken when using the dakotaQueueSubmit node <gui-job-submission-dos-and-donts>`. You will most likely need to make minor edits to the scripts in
order to get the node running in your environment, so you should be at least moderately familiar with shell-scripting to use this node.

Additionally, the default scripts assume `Slurm commands <https://slurm.schedmd.com/overview.html>`__ (sbatch, srun, etc.); however, your job submission system may demand different commands to submit jobs,
in which case you will definitely have to edit the scripts.

----------
Properties
----------

- **account** - The WCID number to use for job submission. Talk to your system administrator to request an account WCID number.
- **continueOnError** - If checked, any error state reported by the job is ignored while you are waiting for the job to complete.
- **continueAfterSubmit** - If checked, the workflow will immediately proceed after the submit step has been completed, meaning that your workflow
  will proceed without waiting for the end of the queued job. This can be advantageous for long-running jobs where you intend to come back later to
  collect the results.
- **cleanSubdirectory** - If checked, Dakota's working directory from previous jobs will be cleaned out.
- **num.nodes** - The number of compute nodes to request.
- **num.processors** - The total number of processors to request.
- **job.hours** - The number of hours of queue time to request.
- **job.minutes** - The number of minutes of queue time to request.
- **queue** - The partition (queue name) to pass to Slurm - by default, 'batch', but 'short' is an option too.
- **submitScript** - Use this field if you would like to supply your own script responsible for submitting to the job queue, replacing the ``dakota-submit.sh``
  provided by default.
- **statusScript** - Use this field if you would like to supply your own script responsible for submitting to the job queue, replacing the ``dakota-status.sh``
  provided by default.
- **checkjobScript** - Use this field if you would like to supply your own script responsible for submitting to the job queue, replacing the ``dakota-checkjob.sh``
  provided by default.
- **cancelScript** - Use this field if you would like to supply your own script responsible for submitting to the job queue, replacing the ``dakota-cancel.sh``
  provided by default.
- **runDakotaScript** - Use this field if you would like to supply your own script responsible for launching Dakota in-queue, replacing the ``runDakotaRemote.sh``
  provided by default.
  
-----------
Input Ports
-----------

- **stdin**: The Dakota input file to use.
- **preprocFile**: If the Dakota input file itself contains :ref:`pyprepro <interfaces:dprepro-and-pyprepro>` markup, a "preproc" file provides the key-value pairs needed to pre-process the Dakota file at runtime. 
  Providing a preproc file on this input port will activate Dakota's ``-preproc`` :ref:`command-line argument <command-line-main>`.
- **Additional input ports**: In addition, pyprepro markup can be substituted *ad hoc* by new input ports.  The names of these input ports must appear
  in the Dakota input file between pyprepro curly braces ``{}`` in order to be pre-processed.  Note that the preprocFile input port takes precedence
  over any additional input ports if it is connected.

------------
Output Ports
------------

- **stdout**: The stdout stream associated with the submit script, *not* the Dakota process.
- **stderr**: The stderr stream associated with the submit script, *not* the Dakota process.
- **exitCode**: The exit code associated with the submit script, *not* the Dakota process.
- **jobId**: The ID associated with your job. You can use this ID to query the job queue for the status of your job at a later time.


.. _gui-job-submission-workflow-dakotaQueueSubmit-scripts:

-------------------------------------
Usage Notes - Editing Control Scripts
-------------------------------------

dakota-submit.sh
----------------

TODO Tips for editing this script

.. code-block:: bash

    #!/bin/bash

    cd ${remote.dir}
    source /etc/bashrc

    echo submitting to the ${queue} queue
    sbatch -N ${num.nodes} \
           --partition=${queue} \
           --time=${job.hours}:${job.minutes}:0 \
           -A ${account} \
           runDakotaRemote.sh \
           2>dart.id.err | tee dart.id.out

    exitcode=$?

    #
    # see if we have the job id in the file, regardless of any exit code from the job submission script
    #
    AWK=/usr/bin/awk
    jobid=$(${AWK} '/^Submitted/ { print $NF; }' dart.id.out)
      
    if [[ -n $jobid ]]; then
      
      # we found a job id, so we can put into the expected file
      printf "%s\n" $jobid > dart.id
    fi

    exit ${exitcode}


dakota-status.sh
----------------

TODO Tips for editing this script

.. code-block:: bash

    #!/bin/bash

    if [[ -z "$jobid" ]]; then
      jobid=$(cat dart.id)
    fi

    checkFilename=slurm-$jobid.out
    resultFilename="job.props"

    function printResult(){
        if [ $# -eq 0 ] ; then
            return
        fi

        if [ -e $resultFilename ] ; then
            rm $resultFilename
        fi

        line="job.results.status=$1"
        echo "$line" > $resultFilename
        echo $1
    }

    successString="Dakota Run Finished."
    failedString="ERROR"

    if [ -e $checkFilename ] ; then
        if (grep -q "$successString" $checkFilename) then
            printResult "Successful"
        else
            if (grep -q "$failedString" $checkFilename) then
                printResult "Failed"
            else
                printResult "Undefined"
            fi
        fi
    else
        printResult "Undefined"
    fi


dakota-checkjob.sh
------------------

TODO Tips for editing this script

.. code-block:: bash

    #
    # This script attempts to find the state of a specified job. There
    # are a number of parameters that could be passed, but this script
    # uses:
    #    job.id : the id of the job
    #    remote.dir : the remote directory where the job was running
    #    job.id.filename : the filename that includes the job id
    #
    # It will return an exit status:
    #   0  : script will echo:
    #        * The found state from the sacct command
    #        * "COMPLETED", under the assmption that the job wasn't found and
    #           > 5 minutes since the job submission has elapsed
    #        * "" (empty String) : unable to find any information, so
    #          implying carry on
    #
    #  sacct doesn't return any results if pass in a bad job id, such
    #    as one that doesn't exist

    # outputs the msg from $1 to stdout and stderr without a newline
    function err() {
      local msg="$1"
      printf "%s" "${msg}"
      printf "%s" "${msg}" >&2
    }

    #outputs the msg from $1 to stdout without a newline
    function msg() {
      local msg="$1"
      printf "%s" "${msg}"
    }

    # checks to see if the file is not present or is > 5 minutes old; return
    #  1 if either is true, and therefore assume completed,
    #  0 otherwise
    #
    function should_assume_completed() {
      local FL="dart.id"
      
      if [[ !(-f "${FL}") || -n $(find "${FL}" -mmin +5) ]]; then
        return 1
      else
        return 0
      fi
    }

    #
    # try to get the current status using squeue
    #
    function set_output_via_squeue {
      local res
      local ec
      
      res=$(squeue --noheader --jobs=$jobid --format="%.30T" 2>&1)
      ec=$?
      
      if [[ $ec -eq 0 ]]; then
        OUTPUT=$(echo "$res" | head -1 | sed -e 's/^[[:space:]]*//')
        return 0;
      fi
      
      return $ec;
    }

    #
    # b/c sacct has so many issues, also make sure even on a non-zero
    #  return that it doesn't indicate an error. NOTE: this might
    #  not be very locale adjusted; OTOH, sacct might not be either
    #
    function ensure_sacct_doesnt_say_error {
      local inp="$1"
      
      if [[ $inp =~ "error:" ]]; then
        SACCT_IS_BEHAVING=1
        return 1
      fi  
       
      return 0
    }

    #
    # if squeue no longer has information about the job, see if
    #  sacct knows anything
    #
    function set_output_via_sacct {
      local res
      local ec
      
      res=$(sacct --noheader --jobs=$jobid --format="state%30" 2>&1)
      ec=$?

      if [[ $ec -ne 0 ]]; then
        SACCT_IS_BEHAVING=1
        return $ec
      fi

      ensure_sacct_doesnt_say_error "$res"
      if [[ $SACCT_IS_BEHAVING -ne 0 ]]; then
        return 1
      fi
      
      # we had a good return from sacct, and it didn't say error    
      OUTPUT=$(echo "$res" | head -1 | sed -e 's/^[[:space:]]*//')

      return 0
    }

    ######################################################################
    #
    ######################################################################
    OUTPUT=""
    SACCT_IS_BEHAVING=0

    if [[ -z "$jobid" ]]; then
      jobid=$(cat dart.id)
    fi

    #
    # gather the information on the job
    #   if squeue returns non-zero, then didn't know about the job
    #
    set_output_via_squeue
    if [[ $? -ne 0 ]]; then
      set_output_via_sacct
    fi

    #
    # if we know sacct has failed us, then we will want to try back
    #  in the future
    #
    if [[ $SACCT_IS_BEHAVING -ne 0 ]]; then
      msg "SACCT_FAILED"
      EXITSTATUS=0
      exit 0
    fi

    #
    # if we didn't get anything back, then the system does not have any
    # information about the job. There may be two reasons for this:
    #   1. This call has come before the job has had time to be added to the queue
    #   2. The job is no longer in the history
    #
    if [[ -z "${OUTPUT}" ]]; then
      should_assume_completed
      ac=$?
      if [[ $ac -eq 1 ]]; then
        msg "COMPLETED"
      else
        msg "UNKNOWN"
      fi
      
      EXITSTATUS=0
    else
      msg "${OUTPUT}"
    fi


dakota-cancel.sh
----------------

TODO Tips for editing this script

.. code-block:: bash

    if [[ -z "$jobid" ]]; then
      jobid=$(cat dart.id)
    fi

    scancel $jobid


runDakotaRemote.sh
------------------

TODO Tips for editing this script

.. code-block:: bash

    #!/bin/bash

    cd ${remote.dir} || exit 9

    if [ -f /etc/bashrc ]; then
        source /etc/bashrc
    fi

    ######################## EXPORTED VARIABLES #####################
    #
    # This script will export the following variables that will be
    # available for use by Dakota's analysis driver:
    #
    #  RELATIVE_LOCATION - the directory that contains the Dakota scripts
    #  DAKOTA_PPN - the number of processors per node
    #  DAKOTA_APPLIC_CONCURRENCY - the number of concurrent runs
    #  DAKOTA_APPLIC_PROCS - the number of processors per job
    #
    #################################################################

    export RELATIVE_LOCATION=..

    ###################################################
    # --- Step 1. Calculate resource availability --- #
    ###################################################

    export DAKOTA_PPN=${num.processors}/${num.nodes}                              # Number of CPUs per node to use if don't want all used
    export DAKOTA_APPLIC_CONCURRENCY=${DakotaKey--evaluation_concurrency}         # Number of concurrent application analysis jobs. Infer from the number of requested CPUs above, i.e. take total available CPU and divide by CPU per job
    export DAKOTA_APPLIC_PROCS=$((${num.processors}/DAKOTA_APPLIC_CONCURRENCY))   # DAKOTA_APPLIC_PROCS to number CPUs per job

    # !!! It is REQUIRED that DAKOTA_APPLIC_PROCS either divide evenly   !!!
    # !!! into DAKOTA_PPN or be an integer multiple of it (typically is) !!!

    echo "Checking that constraint between number of processors, number of concurrent runs, and number of processors per run is respected..."
    remainder=$((${num.processors}%DAKOTA_APPLIC_CONCURRENCY))
    if [ $remainder -gt 0 ];
    then
        echo "${num.processors} must be a multiple of $DAKOTA_APPLIC_CONCURRENCY"
        echo "Exiting..."
        exit -1
    fi

    echo "INFO: Dakota will manage ${DAKOTA_APPLIC_CONCURRENCY} concurrent application jobs,"
    echo "      each running on ${DAKOTA_APPLIC_PROCS} cores,"
    echo "      with ${DAKOTA_PPN} processes per node"

    ##############################
    # --- Step 2. Run Dakota --- #
    ##############################

    # Optionally remove existing working directories

    if [ ${cleanSubdirectory} -eq 1 ]; then
        num_dirs=`find . -name ${DakotaKey--work_directory}.* -type d | wc -l`
        if [ $num_dirs -gt 0 ]; then
          rm -r ${work_subdirectory}.*
        fi
    fi

    # Load the Dakota module - or replace this section if modules are not available for loading Dakota

    # Uncomment if you need to manually init the module system
    # if [ -f /etc/profile.d/modules.sh ]; then
    #   source /etc/profile.d/modules.sh
    # fi

    export APREPRO="module load seacas && aprepro"
    module load dakota/6.17.0 # <- Replace version number if desired

    # Finally, launch the Dakota process

    dakota -input ${input.file.base.name}.in 1>${input.file.name}.out 2>&1

.. _gui-job-submission-workflow-dakotaQueueSubmit-receivingfiles:

-----------------------------
Usage Notes - Receiving Files
-----------------------------

In addition to the default output ports provided on a dakotaQueueSubmit node (namely the "jobId" output port), you may also opt to capture additional files
using the :ref:`"Grab Output File" feature <gui-job-submission-workflow-remoteNestedWorkflow-receivefiles>`.
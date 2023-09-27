.. _ngw-queue-scripts-common:

"""""""""""""""""""""""""""""
Common Job Submission Scripts
"""""""""""""""""""""""""""""

.. _gui-job-submission-workflow-common-scripts-checkjob:

checkjob.sh
-----------

The "checkjob.sh" script is responsible for checking the status of the job in the queue.

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

.. _gui-job-submission-workflow-dakotaQueueSubmit-scripts-cancel:

.. _gui-job-submission-workflow-common-scripts-status:

status.sh
---------

The "status.sh" script is responsible for checking whether the job, once finished, has completed successfully or not. This is distinct from the role 
of "checkjob.sh," which checks the status of the job while it is still in the job queue. The "status.sh" script can be thought of as more of a post-mortem
script that inspects one or more output files for clues that everything completed correctly.

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

    successString="DAKOTA execution time in seconds:"
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

cancel.sh
---------

The "cancel.sh" script is responsible for stopping the job in the queue if the user stops Next-Gen Workflow.

.. code-block:: bash

    if [[ -z "$jobid" ]]; then
      jobid=$(cat dart.id)
    fi

    scancel $jobid
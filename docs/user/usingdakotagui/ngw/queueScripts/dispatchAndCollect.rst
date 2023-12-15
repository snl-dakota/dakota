.. _ngw-queue-scripts-dispatchAndCollect:

"""""""""""""""""""""""""""""""""""""""""
dispatchAndCollect Job Submission Scripts
"""""""""""""""""""""""""""""""""""""""""

.. _gui-job-submission-workflow-dispatchAndCollect-scripts-submitDispatch:

submit-dispatch.sh
------------------

The "submit-dispatch.sh" script is responsible for submitting the job into the job scheduler (in our example script, the job scheduler is Slurm, and the command used to
submit is "sbatch"). This script is NOT directly responsible for running the inner NGW workflow
(see the :ref:`"dispatchWorkflowRemote.sh" script <gui-job-submission-workflow-dispatchAndCollect-scripts-dispatchWorkflowRemote>` below).

.. code-block:: bash

    #!/bin/bash

    cd ${remote.dir}
    source /etc/bashrc

    echo submitting to the ${queue} queue
    sbatch -N ${num.nodes} \
           --partition=${queue} \
           --time=${job.hours}:${job.minutes}:0 \
           -A ${account} \
           dispatchWorkflowRemote.sh \
           2>dart.id.err | tee dart.id.out

    exitcode=$?

    # see if we have the job id in the file, regardless of any exit code from the job submission script
    AWK=/usr/bin/awk
    jobid=$(${AWK} '/^Submitted/ { print $NF; }' dart.id.out)
      
    if [[ -n $jobid ]]; then
      
      # we found a job id, so we can put into the expected file
      printf "%s\n" $jobid > dart.id
    fi

    exit ${exitcode}

.. _gui-job-submission-workflow-dispatchAndCollect-scripts-dispatchWorkflowRemote:

dispatchWorkflowRemote.sh
-------------------------

The "dispatchWorkflowRemote.sh" script is responsible for actually running the inner NGW workflow within the job queue.

.. code-block:: bash

    #!/bin/bash

    export WFLIB=${wflib.path}

    if [  $WFLIB == "EMBEDDED" ]; then
        exec ~/.dart/runNgw.sh -g ${remote.dir}/globals.in -k -h ${remote.dir} ${driverWorkflow} ${paramsFile} dakResults.txt
    else
        PATH=$WFLIB:$PATH
        
        if [ -f $WFLIB/environment ] ; then
            . $WFLIB/environment
        fi 
        
        java -XX:CICompilerCount=2 \
          -XX:+ReduceSignalUsage \
          -XX:+DisableAttachMechanism \
          -XX:+UseSerialGC \
          -cp $WFLIB/\*:$WFLIB/lib/\*:$WFLIB/plugins/\* gov.sandia.dart.workflow.runtime.Main -g ${remote.dir}/globals.in -k -h ${remote.dir} ${dispatchedWorkflow} ${paramsFile} dakResults.txt
        exit $?
    fi
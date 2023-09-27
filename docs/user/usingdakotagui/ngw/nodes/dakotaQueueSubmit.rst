.. _ngw-node-dakotaQueueSubmit:

.. _gui-job-submission-workflow-dakotaQueueSubmit:

=================
dakotaQueueSubmit
=================

This node is responsible for running Dakota. However, unlike the regular :ref:`dakota <ngw-node-dakota>` node, the dakotaQueueSubmit node should only be
used in the context of a workflow that is :ref:`running on a login node belonging to a high-performance computer with a job queue <gui-job-submission-workflow-example2>`.

Most of the dakotaQueueSubmit node's behavior in-queue can be controlled via :ref:`editable control scripts <ngw-queue-scripts>` that are provided with the node:

1. Submit a job into the queue (``submit-dakota.sh``)
2. Check on the job’s overall status (``status.sh``)
3. Drill into a more detailed description of the job state (``checkjob.sh``)
4. Cancel the job (``cancel.sh``)
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
- **statusScript** - Use this field if you would like to supply your own script responsible for submitting to the job queue, replacing the ``status.sh``
  provided by default.
- **checkjobScript** - Use this field if you would like to supply your own script responsible for submitting to the job queue, replacing the ``checkjob.sh``
  provided by default.
- **cancelScript** - Use this field if you would like to supply your own script responsible for submitting to the job queue, replacing the ``cancel.sh``
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
- **jobId**: The ID associated with your job. You can use this ID to let other nodes query the job queue for the status of your job at a later time.

-------------------------------------
Usage Notes - Editing Control Scripts
-------------------------------------

Refer to the main documentation for each control script to learn more about its function and what to consider editing:

- :ref:`dakota-submit.sh <gui-job-submission-workflow-dakotaQueueSubmit-scripts-submit>`
- :ref:`status.sh <gui-job-submission-workflow-common-scripts-status>`
- :ref:`checkjob.sh <gui-job-submission-workflow-common-scripts-checkjob>`
- :ref:`cancel.sh <gui-job-submission-workflow-dakotaQueueSubmit-scripts-cancel>`
- :ref:`runDakotaRemote.sh <gui-job-submission-workflow-dakotaQueueSubmit-scripts-runDakotaRemote>`

.. _gui-job-submission-workflow-dakotaQueueSubmit-receivingfiles:

-----------------------------
Usage Notes - Receiving Files
-----------------------------

In addition to the default output ports provided on a dakotaQueueSubmit node (namely the "jobId" output port), you may also opt to capture additional files
using the :ref:`"Grab Output File" feature <gui-job-submission-workflow-remoteNestedWorkflow-receivefiles>`.
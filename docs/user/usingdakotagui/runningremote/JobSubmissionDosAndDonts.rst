.. _gui-job-submission-dos-and-donts:

""""""""""""""""""""""""""""""
Job Submission Do's and Don'ts
""""""""""""""""""""""""""""""

-----
DO...
-----

---------
DO NOT... 
---------

Run Dakota on your local machine with remoteNestedWorkflow analysis drivers
---------------------------------------------------------------------------

It is generally considered poor practice to create a workflow that starts Dakota on your local machine, and then issues separate SSH connections to a remote machine
per iteration. For example, you can get into this situation if your Dakota analysis driver is a nested NGW workflow also running on your local machine, and that workflow
contains a :ref:`remoteNestedWorkflow <gui-job-submission-workflow-remoteNestedWorkflow>` node.

The reason this is considered poor practice is that it has the potential to open up an arbitrary number of SSH connections to the remote machine, potentially clogging
traffic on that machine, depending on the evaluation concurrency of your Dakota study. Additionally, there is a significant performance degradataion associated with
establishing a separate SSH connection for every iteration of a Dakota study. Performance will be degraded even further if each of these remote connections submits
work to a queued job submission system, meaning each evaluation will have to separately wait its turn. (It is far more advantageous to have Dakota do its work inside the job
queue in such a scenario).

If you *must* take the approach of running Dakota and your analysis drivers locally and issuing connections to a remote machine per analysis driver, take extra care to ensure
that you 1) control the number of Dakota analysis drivers carefully, and 2) understand the amount of time it takes to complete the remote portion of the job (and therefore how
long the SSH connection will remain open).

Use Job Submission run configurations for NGW-based studies
-----------------------------------------------------------

There is no advantage to mixing Next-Gen Workflow with the Job Submission run configuration type. If you anticipate needing to use Next-Gen Workflow in one part of your study,
Next-Gen Workflow should probably be used to orchestrate job submission as well.
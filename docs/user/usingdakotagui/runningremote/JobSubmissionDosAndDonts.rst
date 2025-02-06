.. _gui-job-submission-dos-and-donts:

""""""""""""""""""""""""""""""
Job Submission Do's and Don'ts
""""""""""""""""""""""""""""""

-----
DO...
-----

Have a clear understanding of where your job is running
-------------------------------------------------------

Even if you plan on primarily working from the context of Dakota GUI, you should have a good idea of where your files are ending up on the remote machine.

- Decide on a naming convention for your remote job folder (or hierarchy of remote job folders), which will be unique to you and to your job.
- Get familiar with logging onto the remote machine through a terminal. This is an unfortunate but necessary fact of life when it comes to troubleshooting remote jobs that
  are misbehaving. :ref:`The Dakota GUI provides built-in facilities for doing this. <gui-job-submission-classic-usage-remote>` You may also decide to use
  `PuTTY <https://putty.org/>`__ (on Windows) or your OS's native terminal to establish remote connections.

Clean up after previous job runs
--------------------------------

It is typical to re-run your job in the same working directory on the remote machine, especially if you are in the process of setting up a new local-to-remote job submission
configuration and are working through the early stages of troubleshooting such a setup. Get in the habit of clearing out your remote run directory, either by manually deleting
the files via a terminal connection to the remote machine, or by configuring such behavior using the GUI (for example, check the "Clear private working directory" on NGW's
remoteNestedWorkflow).

Use the Machines view
---------------------

The Machines view is an extremely useful way to get a lay of the land with regards to remote machines on your network. This view will give you a set of bar charts
that show how much space is available on each machine for running additional jobs. You can also SSH into the machine in question by right-clicking on that machine's
row in this view. You can also get a window into the remote machine's filesystem and dock it in the GUI. From there, you can remotely tail individual logs
that are on the remote machine.

As with all views, you can access the Machines view via "Window > Show View > Other" if you don't already have it in your perspective.

---------
DO NOT... 
---------

Use Job Submission run configurations for NGW-based studies
-----------------------------------------------------------

There is no advantage to mixing Next-Gen Workflow with a :ref:`classic Job Submission run configuration <gui-job-submission-classic>`. If you anticipate needing to use
Next-Gen Workflow in one part of your study, Next-Gen Workflow should probably be used to orchestrate your entire study end-to-end.

----------------------
WE PREFER YOU DON'T... 
----------------------

Run Dakota on your local machine with remoteNestedWorkflow analysis drivers
---------------------------------------------------------------------------

It is generally inadvisable to create a workflow that starts Dakota on your local machine, and then issues separate SSH connections to a remote machine
per evaluation. For example, you can get into this situation if your Dakota analysis driver is a nested NGW workflow also running on your local machine, and that workflow
contains a :ref:`remoteNestedWorkflow <gui-job-submission-workflow-remoteNestedWorkflow>` node.

The reason we don't recommend doing this is that it has the potential to open up an arbitrary number of SSH connections to the remote machine, potentially clogging
traffic on that machine, depending on the evaluation concurrency of your Dakota study. Additionally, there is a significant performance degradataion associated with
establishing a separate SSH connection for every evaluation of a Dakota study, and you are making the success of your workflow dependent on the health and reliability of
multiple network connections. Performance will be degraded even further if each of these remote connections submits work to a queued job submission system, meaning each
individual Dakota evaluation will have to separately wait its turn with other jobs, which puts even more pressure on the SSH connections to remain connected
and healthy for the duration of this process.

If you *must* take the approach of running Dakota and your analysis drivers locally and issuing one SSH connection per analysis driver, take extra care to ensure
that you 1) control the number of Dakota analysis drivers carefully, and 2) understand the amount of time it takes to complete the remote portion of the job (and therefore how
long each SSH connection must remain active).


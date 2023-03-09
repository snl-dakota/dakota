.. _ngw-node-dakotaQueueSubmit:

.. _gui-job-submission-workflow-dakotaQueueSubmit:

=================
dakotaQueueSubmit
=================

The dakotaQueueSubmit node should be used in the context of a workflow that is running on a login node of a high-performance computer with a job queue.
The dakotaQueueSubmit node is designed to provide scripts that:

1. Submit to a queue (``submit-dakota.sh``)
2. Check on the job's status in the queue (``checkjob-dakota.sh`` and ``status.sh``)
3. Cancel the job (``cancel-dakota.sh``)

TODO
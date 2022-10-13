.. _ngw-node-dakotaWorkflowDriver:

====================
dakotaWorkflowDriver
====================

-----------
Description
-----------

This node wraps an IWF file in an OS-executable script that can be called cleanly from Dakota as theanalysis driver of choice.

This node is best used with the Dakota node by creating an ad-hoc input port (for example, ``"DRIVER"``) onthe Dakota node, and then
using dprepro markup within the Dakota input file to pass the workflow driver'sfull file path at runtime (for example, ``"{DRIVER}"``). 

----------
Properties
----------

- **workflowFile**: The file path to the IWF file to call as the Dakota analysis driver.
- **automaticallyReadParameters**: If unchecked, the nested workflow is responsible for manuallyreading the Dakota parameters file.
  Refer to the :ref:`Expert Mode Requirements <ngw-dakotaWorkflowDriver-expertmode>` section for a detailed explanation of what this entails.
- **automaticallyWriteResults**: If unchecked, the nested workflow is responsible for manuallywriting out the Dakota results file.
  Refer to the :ref:`Expert Mode Requirements <ngw-dakotaWorkflowDriver-expertmode>` section for a detailed explanation of what this entails.

-----------
Input Ports
-----------

No default input ports.

------------
Output Ports
------------

- **outputFile**: A file reference for the generated executable script to be used as Dakota's analysis driver(.bat for Windows, .sh for Unix).

.. _ngw-dakotaWorkflowDriver-expertmode:

------------------------
Expert Mode Requirements
------------------------

Dakota communicates with analysis drivers by writing a parameter file that must be read by that analysis driver,
and by expecting a results file to be written out by the analysis driver upon completion.
Ordinarily, Next-Gen Workflow handles the translation step between Dakota's parameter file format and NGW parameter nodes; likewise,
it handles the translation step between NGW response nodes and Dakota's result file format.

"Expert mode" is enabled by unchecking the "automaticallyReadParameters" field, the "automaticallyWriteResults" field,
or both. When running in expert mode, these translation steps are not handled. Instead, it is up to the analyst writing
the analysis driver workflow to manually read from the Dakota parameters file, write the Dakota results file, or both.

- If "automaticallyReadParameters" is unchecked, the analysis driver workflow must contain a "params"
  parameter node, which is the file path to Dakota's parameter file.
- If "automaticallyWriteResults" is unchecked, the analyst must use a "file" node to write the contents of aDakota results file.
- It is ultimately up to the analyst to decide how to manipulate the parameter and results files, but the
  :ref:`dakotaParametersMap <ngw-node-dakotaParametersMap>` and :ref:`dakotaResultsFile <ngw-node-dakotaResultsFile>` nodes can help with performing map-based manipulations of these files.

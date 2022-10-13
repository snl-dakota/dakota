.. _ngw-node-dakotaParametersMap:

===================
dakotaParametersMap
===================

-----------
Description
-----------

When Dakota is communicating with an analysis driver, it creates a parameters file that contains the current Dakota iteration's parameter values, as well as some additional data to inform the analysis driver. This node parses the Dakota parameters file intoappropriate key-value maps that contain the data of the parameters file.

----------
Properties
----------

No properties. 

-----------
Input Ports
-----------

- **parametersFile**: the parameters file to parse.

------------
Output Ports
------------

- **variables**: This port outputs a map data structure where the keys are parameter labels and the valuesare this iteration's parameter values.
- **functions**: Read more about derivate_vars :ref:`here <interfaces:mappings>`.
- **derivative_vars**: Read more about derivate_vars :ref:`here <interfaces:mappings>`.
- **analysis_comps**: Read more about analysis_comps :ref:`here <interfaces:mappings>`.
- **eval_id**: A number representing the current Dakota iteration.

-----
Notes
-----

It is not necessary to use this node if an outer, calling workflow has set its :ref:`dakota <ngw-node-dakota>` node's :ref:`expertMode <ngw-expertmode>` property to false.

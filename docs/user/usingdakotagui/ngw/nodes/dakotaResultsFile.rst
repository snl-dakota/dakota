.. _ngw-node-dakotaResultsFile:

=================
dakotaResultsFile
=================

-----------
Description
-----------

When Dakota is communicating with an analysis driver, it expects the analysis driverto output a results file that contains pairs of response labels and values, which Dakota can then read. This node converts a map of key-value pairs into the format of aDakota results file. 

----------
Properties
----------

No properties.

-----------
Input Ports
-----------

- **inputMap**: the map data structure to convert.
- You can optionally provide additional input ports to this node. The labels of the additional input ports will be used as response names, and the value passed in through each input port will become the associated response value.

------------
Output Ports
------------

- **outputText**: A text string that represents data written in the format of a Dakota results file.

-----
Notes
-----

It is not necessary to use this node if an outer, calling workflow has set its :ref:`dakota <ngw-node-dakota>` node's :ref:`expertMode <ngw-expertmode>` property to false.

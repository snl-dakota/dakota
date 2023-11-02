.. _ngw-node-plotInputGather:

===============
plotInputGather
===============

-----------
Description
-----------

This node can be used to scan Dakota results directories and gather up individual results files for plotting. This circumvents
the need to extract data from Dakota tabular data files or Dakota HDF5 files. This node is compatible with any Chartreuse
plotting node that accepts separate X/Y/Z data streams.

-----
Notes
-----

This node is intended to be used with NGW-based Dakota studies only, as it is equipped to pick up the CSV results files that
Next-Gen Workflow generates. It is not currently equipped to recognize other types of results file formats.

----------
Properties
----------

- **resultsFolder:** The working directory to scan for results files.
- **xLabel:** The parameter name or result name used to collect the data stream for the X axis.
- **yLabel:** The parameter name or result name used to collect the data stream for the Y axis.
- **zLabel:** The parameter name or result name used to collect the data stream for the Z axis (if plotting 3D data).

-----------
Input Ports
-----------

- **resultsFolder:** The working directory to scan for results files.

------------
Output Ports
------------

- **X:** The X data stream collected from the CSV results files.
- **Y** The Y data stream collected from the CSV results files.
- **Z** The Z data stream collected from the CSV results files (optional, if you intend to plot 3D data).

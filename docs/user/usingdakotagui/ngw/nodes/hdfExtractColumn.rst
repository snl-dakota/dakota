.. _ngw-node-hdfExtractColumn:

================
hdfExtractColumn
================

-----------
Description
-----------

Extract a column from a specific HDF dataset inside a given HDF file.

----------
Properties
----------

- **datasetPath**: The full path to the dataset within the HDF file, beginning with a "/".
- **rowIndex**: The 0-based index of the column.

-----------
Input Ports
-----------

- **stdin**: A file reference to the HDF file to read.

------------
Output Ports
------------

- **stdout**: The data column extracted from the HDF dataset.
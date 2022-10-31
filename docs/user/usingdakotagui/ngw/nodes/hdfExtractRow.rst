.. _ngw-node-hdfExtractRow:

=============
hdfExtractRow
=============

-----------
Description
-----------

Extract a row from a specific HDF dataset inside a given HDF file.

----------
Properties
----------

- **datasetPath**: The full path to the dataset within the HDF file, beginning with a "/".
- **rowIndex**: The 0-based index of the row.

-----------
Input Ports
-----------

- **stdin**: A file reference to the HDF file to read.

------------
Output Ports
------------

- **stdout**: The data row extracted from the HDF dataset.
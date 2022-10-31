.. _ngw-node-scatterPlotSuite:

================
scatterPlotSuite
================

-----------
Description
-----------

Using Chartreuse, this node allows you to plot a large quantity of scatter plots (plotting variable against
response) by sub-dividing the scatter plot matrix into a series of reasonably-sized plot files. 

-----
Notes
-----

- This node is specifically tailored to pull data from Dakota-generated HDF files. It
  will not work with other types of Dakota output files, nor will it work with arbitrary HDF databases.

----------
Properties
----------

- **model**: The model from the Dakota study that is parent to the data for the scatter plot matrices.
  You can either provide the ID of the model from the original Dakota input file,
  or the HDF5 database path to a specific model group. If the Dakota study only contained one model,
  then this field can be left blank.
- **outputPlotFiles**: If checked, this node will generate .plot files for each section of the
  scatter plot matrix. These plot files can be subsequently edited after generation.
- **outputPngScreenshots**: If checked, this node will generate .png image files for each section of
  the scatter plot matrix.
- **rowsPerPlot**: The maximum number of rows allowed in each subdivision of the scatter plot matrix.
  There is no enforced upper limit on this field, but we recommend 10 as the maximum value.
- **columnsPerPlot**: The maximum number of columns allowed in each subdivision of the scatter plot matrix.
  There is no enforced upper limit on this field, but we recommend 10 as the maximum value.
- **drawLinearRegression**: Draw a linear regression line on each generated canvas.
- **outerAxisTextOnly**: Do not repeat axis text for canvases that are "inside" the grid. If unchecked,
  this will lead to redundant canvas labels throughout your final plot.
- **ignoreUnchangingData**: If checked, axes containing data that is not changing along
  either the X axis or the Y axis will be omitted.

-----------
Input Ports
-----------

- **hdfFile**: the Dakota-generated HDF input file.

------------
Output Ports
------------

None.

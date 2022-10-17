.. _ngw-node-hdfTraceScatterPlot3d:

=====================
hdfTraceScatterPlot3d
=====================

-----------
Description
-----------

Using Chartreuse, this node creates a 3D scatter plot trace from an HDF5 dataset. 

-----
Notes
-----

This node is specifically tailored to pull data from Dakota-generated HDF files. It
will not work with other types of Dakota output files, nor will it work with arbitrary HDF databases.

---------------------
Dakota HDF Properties
---------------------

- **model**: The model from the Dakota study that is parent to the variables and responses
  of interest. You can either provide the ID of the model from the original Dakota input file,
  or the HDF5 database path to a specific model group. If the Dakota study only contained one model,
  then this field can be left blank.
- **xLabel**: The variable or response label to use for the X data of the scatter plot.
- **yLabel**: The variable or response label to use for the Y data of the scatter plot.
- **zLabel**: The variable or response label to use for the Z data of the scatter plot.

---------------------
Chartreuse Properties
---------------------

- **Trace Name**: The name of the trace. If this node is the final destination on the way to displaying
  the plot, this field will be used as the plot's title that displays across the top. Otherwise,
  this field can be used to identify the trace among several traces (for instance, by using the legend
  provided with the plotWindow node).
- **Dot Color**: You can choose a color for the dots and/or line that comprises your scatter plot trace.
- **Dot Type**: This dropdown allows you to choose a shape for the dots in your scatter plot trace.
- **Connect Dots**: Toggles whether or not a line should be drawn between each dot of your scatter plot trace.

------------
Output Ports
------------

- **plotFileDataOut**: A complete, serialized version of the plot that can be written out to a file for
  later viewing. Use a file node to save your plot to a file.

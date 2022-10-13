.. _ngw-node-plotCanvas:

==========
plotCanvas
==========

-----------
Description
-----------

Aggregates one or more plot traces onto a plot canvas. A plot canvas can be thought of a single set of axes.
Individual plot traces are aggregated by adding an input port to this node for each trace. 

----------
Properties
----------

- **Name**: The name of your canvas. If the plotCanvas node is the final stop on the way to displaying theplot, then this field will be used as the title displayed across the top of the plot.
- **X axis label**: The text that displays along the X axis.
- **Y axis label**: The text that displays along the Y axis.
- **Z axis label**: The text that displays along the Z axis. This is only relevant for 3D plots.
- **Draw axis lines**: Toggles whether or not to draw the main lines of the axis.
- **Draw grid lines**: Toggles whether to draw a light-gray grid behind your plot data.

-----------
Input Ports
-----------

This node has no input ports by default. Add one input port for each plot trace node you want to aggregateonto a single canvas.

------------
Output Ports
------------

- **canvas**: The data of the plot canvas. Use this to forward the plot canvas for further aggregation onto aplot window.
- **plotFileDataOut**: A complete, serialized version of the plot that can be written out to a file for later viewing.Use a file node to save your plot to a file.

-----
Notes
-----

For further reading, refer to :ref:`this tutorial <chartreuse-ngw-main-multiple-trace-tutorial>` that demonstrates aggregating multiple plot traces onto a single plotCanvas node.

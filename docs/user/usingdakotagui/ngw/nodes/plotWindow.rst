.. _ngw-node-plotWindow:

==========
plotWindow
==========

-----------
Description
-----------

Aggregates one or more plot canvases onto a plot window. A plot window is the top-level construct for
Chartreuse plotting, primarily used to arrange multiple plot canvases on a grid. 

----------
Properties
----------

- **Name**: The name of your plot window. This field will be used as the title displayed
  across the top of the plot.
- **Column count**: How many columns your plot window will use to display canvases. When a canvas's
  index overflows past this number, it will be added to the next row. So for example, if you have
  4 canvases and you want to arrange them in a 2x2 grid, use a column count of 2.
- **Show legend**: Toggles whether to display a legend of data next to the plot.
- **Plot window width**: The width in pixels of the final rendered plot. By default, plots are rendered
  at the size of the containing viewer.
- **Plot window height**: The height in pixels of the final rendered plot. By default, plots are rendered
  at the size of the containing viewer.

-----------
Input Ports
-----------

This node has no input ports by default. Add one input port for each plot canvas node you want to aggregate
onto a single plot window.

-----------=
Output Ports
------------

- **plotFileDataOut**: A complete, serialized version of the plot that can be written out to a file for later viewing.
  Use a file node to save your plot to a file.

-----
Notes
-----

For further reading, refer to :ref:`this tutorial <chartreuse-ngw-main-multiple-canvas-tutorial>` that demonstrates aggregating multiple plot canvases onto a single plotWindow node.

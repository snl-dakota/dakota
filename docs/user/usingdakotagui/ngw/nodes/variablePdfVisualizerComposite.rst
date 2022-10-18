.. _ngw-node-variablePdfVisualizerComposite:

==============================
variablePdfVisualizerComposite
==============================

-----------
Description
-----------

This node allows you to visualize the probability density function (PDF) associated with
all uncertain variables in a Dakota study, using Chartreuse as the plotting library. 

----------
Properties
----------

- **outputFilePrefix**: Prepends text to the name of the plot file(s) that get written
  to disk by this node.
- **outputPlotFiles**: If checked, this node will generate .plot files for each Dakota uncertain
  variable. These plot files can be subsequently edited after generation.
- **outputPngScreenshots**: If checked, this node will generate .png image files for each
  Dakota uncertain variable.

-----------
Input Ports
-----------

- **inFile**: the Dakota input file.

------------
Output Ports
------------

None. 
.. _ngw-node-screenshot:

==========
screenshot
==========

-----------
Description
-----------

Takes a screenshot of a Chartreuse plot file.

----------
Properties
----------

- **Destination**: The name of the file to write the screenshot to.
- **Screenshot Width**: The width of the screenshot in pixels. If left blank, the node will use a heuristic
  to determine an acceptable width based on number of canvases and trace point density.
- **Screenshot Height**: The height of the screenshot in pixels. If left blank, the node will use a heuristic
  to determine an acceptable height based on number of canvases and trace point density.

-----------
Input Ports
-----------

- **plotFileDataIn**: The serialized plot object to take a screenshot of. This node accepts plot traces, plot canvases, and full plot windows.

.. _chartreuse-ngw-uncertainty-variables:

.. _ngw-node-variablePdfVisualizer:

=====================
variablePdfVisualizer
=====================

.. image:: img/Plotting_Workflow_15.png
   :alt: PDF visualization node

-----------
Description
-----------

To use this node, simply provide the Dakota input file as a "file" node on the canvas, and pipe its "fileReference" output port to the "inFile"
input port of this node.  In addition, provide the descriptor of the variable you want to visualize in this node's Settings Editor.

Note that this node has the familiar "plotFileDataOut" and "trace" output ports seen on other Chartreuse nodes.  This means that Dakota variable
PDF curve plots can be combined with other types of Chartreuse plotting nodes (for example, :ref:`aggregating traces onto the same canvas <chartreuse-ngw-main-multiple-trace-tutorial>`).

----------
Properties
----------

-----------
Input Ports
-----------

------------
Output Ports
------------

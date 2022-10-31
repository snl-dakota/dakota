.. _ngw-node-posPrePro:

=========
posPrePro
=========

-----------
Description
-----------

This node allows you to replace text in a provided "template" text file, using position offsets
to define the text ranges that should be replaced. This node is extremely useful when preparing
an input file to a program where you would like to replace portions of the input file with new
values. After defining which text ranges to replace (i.e. defining "pre-processors"), the node
will receive information at runtime to inform what values to substitute into your template file.

This node can receive replacement input data from two places. You may either use the inputParameterMap
input port, or you may create individual input ports that receive single values from earlier in
your workflow. 

----------
Properties
----------

- **Template File**: the path to the template file. This path is relative to the location of your workflow file.

This node's settings editor contains a table displaying the text replacements defined so far.

- **Add Pre-processor**: opens a dialog allowing you to define a new label, default value, and range of text to replace.
- **Edit Pre-processor**: opens a dialog allowing you to edit a previously defined pre-processor.
- **Remove Pre-processor**: deletes a previously defined pre-processor.
- **Import Input Ports**: auto-populate this node with empty pre-processors that correspond to single parameter input ports
  you've already defined for this node. See the Description section or Input Ports section of this help page for more
  information on using single parameter input ports.

-----------
Input Ports
-----------

- **inputParametersMap**: takes a map data structure containing key-value pairs of parameter labels and values. This map's
  parameter labels should correspond to parameter labels you have defined as pre-processors for this node. Any labels
  in this map that do not have pre-processors defined will be ignored.

You may also define custom input ports for this node for individual parameter labels. You can do this by dragging
a connector line from a workflow parameter node to blank space on the left side of your posPrePro node.

------------
Output Ports
------------

- **processedFileText**: The text of your template input file, with replacements based on the pre-processors you defined.
- **outputFile**: A file reference to your processed input file, if you prefer to work with a file rather than a text stream.

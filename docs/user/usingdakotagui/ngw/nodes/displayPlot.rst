.. _ngw-node-displayPlot:

===========
displayPlot
===========

-----------
Description
-----------

The displayPlot node can be used to immediately display the results of a Chartreuse plot in the docked editor area of Dakota GUI.
This node essentially functions as a shortcut that performs the work of a "file" node (which will write the Chartreuse data to a file)
and an "openFile" node (which will open the saved plot in the editor). A file for your plot will still be saved at the destination
you specify.

----------
Properties
----------

- **Destination file:** A filepath that declares where you want the intermediate file to be saved. If nothing is provided, the
  plot will be saved with the name "output."
- **Save to .plot format:** Check this box if you want the plot to be saved in Chartreuse's .plot format, which can only be opened 
  in Dakota GUI. Otherwise, the plot will be saved in .html format, so that it can be opened in any web browser.

-----------
Input Ports
-----------

- **plotFileDataIn:** The input data for your plot. This can be connected to any "plotFileDataOut" port from any Chartreuse node.

------------
Output Ports
------------

- **plotFileOut:** A file path to the generated plot file. Note that this output port is NOT "plotFileDataOut," meaning that its
  data is a path to an already-serialized plot file, not a data stream containing the plot information.

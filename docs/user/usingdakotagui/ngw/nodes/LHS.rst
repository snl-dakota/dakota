.. _ngw-node-LHS:

===
LHS
===

-----------
Description
-----------

This node can be used to generate a series of different plots based on a Dakota
:ref:`LHS study. <uq:overview:methods>`

-----
Notes
-----

- LHS stands for "Latin hypercube sampling." 
- This node is specifically tailored to pull data from Dakota-generated HDF files. It
  will not work with other types of Dakota output files, nor will it work with arbitrary HDF databases.
- In addition, only the Dakota sampling method is compatible with this node.
- This node is solely responsible for writing the plot files to disk. It is not
  necessary to use file nodes in your workflow to save the plots.
- After generating and saving the plot file(s), you are free to further modify the plot
  using the "Chartreuse > Edit plot" context menu option.

----------
Properties
----------

- **methodId**: The Dakota method ID used to locate the correct discrete state set variables. If
  the Dakota study only contained one method block, this field can be left blank.
- **outputFilePrefix**: Use this field to apply a common prefix to each generated plot file.
  This can help to prevent unintentional overwriting of plot data if the workflow is run
  multiple times.
- **outputPlotFiles**: If checked, this node will generate .plot files for each piece of information
  about the LHS study. These plot files can be subsequently edited after generation.
- **outputPngScreenshots**: If checked, this node will generate .png image files for each piece of
  information about the LHS study.
- **activeVariablesOnly**: If checked, then only consider active Dakota variables when creating plots.
- **plotPdf**: If checked, then generate probability density function histograms for the study.
- **plotCdf**: If checked, then generate cumulative probability function histograms for the study.
- **plotScatter**: If checked, then generate scatter plots for every unique pairwise combination
  of Dakota variables and responses.
- **plotCorrelation**: If checked, then create correlation coefficient matrix plots.
- **targetPdfBinCount**: Specify a "recommended" number of bins for your PDF plots. Most underlying
  plotting libraries use a smart binning algorithm based on the data, so this value is just a guideline.
- **targetCdfBinCount**: Specify a "recommended" number of bins for your CDF plots. Most underlying
  plotting libraries use a smart binning algorithm based on the data, so this value is just a guideline.

-----------
Input Ports
-----------

- **hdfFile**: the Dakota-generated HDF input file.

------------
Output Ports
------------

None.

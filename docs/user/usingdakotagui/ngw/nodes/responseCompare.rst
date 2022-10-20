.. _ngw-node-responseCompare:

===============
responseCompare
===============

-----------
Description
-----------

Using Chartreuse, this node creates a series of plot canvases that compare output responses,
given varying input parameters. One plot canvas is created per parameter/response combination,
and these canvases are grouped by response. 

-----
Notes
-----

- This node is specifically tailored to pull data from Dakota-generated HDF files. It
  will not work with other types of Dakota output files, nor will it work with arbitrary HDF databases.
- In addition, only the :ref:`centered_parameter_study <method-centered_parameter_study>`,
  :ref:`list_parameter_study <method-list_parameter_study>`,
  :ref:`multidim_parameter_study <method-multidim_parameter_study>`,
  and :ref:`vector_parameter_study <method-vector_parameter_study>` methods are compatible with this node.
- If you are working with discrete state variables, we recommend using the :ref:`"categoricalScatterPlots" <ngw-node-categoricalScatterPlots>` and/or
  :ref:`"categoricalBarCharts" <ngw-node-categoricalBarCharts>` megaplotter nodes instead of this one.
- This node is solely responsible for writing the plot files to disk. It is not
  necessary to use file nodes in your workflow to save the plots.
- After generating and saving the plot file(s), you are free to further modify the plot
  using the "Chartreuse > Edit plot" context menu option.

----------
Properties
----------

- **outputFilePrefix**: Prepends text to the name of the plot file(s) that get written
  to disk by this node.
- **outputPlotFiles**: If checked, this node will generate .plot files for each piece of information
  about the response comparison study. These plot files can be subsequently edited after generation.
- **outputPngScreenshots**: If checked, this node will generate .png image files for each piece of
  information about the response comparison study.
- **activeVariablesOnly**: If checked, then only consider active Dakota variables when creating plots.
- **normalizeData**: If selected, all plot traces will be normalized along the horizontal axis.
- **verticalMinOverride**: Specify a manual minimum for the vertical axis across all canvases.
  If either this property or the verticalMaxOverride property are left blank, then neither property applies
  to the displayed canvases. When using this property, keep in mind that the responseCompare node
  outputs multiple plot files from multiple datasets. If you know in advance that your data will be
  displayed across vastly different scales, it may not be a good idea to use this property, as it will
  apply the override value to all canvases across all plot windows.
- **verticalMaxOverride**: Specify a manual maximum for the vertical axis across all canvases.
  If either this property or the verticalMinOverride property are left blank, then neither property applies
  to the displayed canvases. When using this property, keep in mind that the responseCompare node
  outputs multiple plot files from multiple datasets. If you know in advance that your data will be
  displayed across vastly different scales, it may not be a good idea to use this property, as it will
  apply the override value to all canvases across all plot windows.
- **sameVerticalScale**: If selected, all plot canvases will share the same vertical scale. That scale
  is chosen based on the data displayed across all canvases. However, note that this property will be
  overridden if you provide minimum and maximum manual values for the vertical axis (see the
  verticalMinOverride and verticalMaxOverride properties).

-----------
Input Ports
-----------

- **hdfFile**: the Dakota-generated HDF input file.

------------
Output Ports
------------

None.

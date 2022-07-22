.. _chartreuse-plot-templates:

""""""""""""""
Plot Templates
""""""""""""""

The "Plot Templates" dialog fills a usage gap between the :ref:`Plot Trace Creator <chartreuse-plot-trace>` dialog and the :ref:`Plot Window Manager <chartreuse-plot-manager>` dialog...

- On one hand, the Plot Trace Creator dialog is limited to creating single-trace, single-canvas plots.
- On the other hand, the Plot Window Manager dialog becomes increasingly time-consuming to use as you manually add more canvases and traces.
  For instance, creating a comprehensive view of the Dakota centered parameter study for the cantilever beam model would require 21 traces (7 variables x 3 responses),
  which would take a long time to manually input, not to mention the fact that doing so is error-prone, requiring lots of trial and error to get the plot exactly the way you want it.

The Dakota GUI includes templates that facilitate easy access to the most common types of plots that you would need as part of a study performed by Dakota.  To access the Plotting Templates dialog, either...

- From the Plot Window Manager dialog, click on the "Choose a plotting template" button in the top-left corner.
- Right-click on a file containing plottable data and select Chartreuse > New plot template from this file.

Each available plotting template is described in detail in the following sections.

.. image:: img/Plotting_Templates_Intro_1.png
   :alt: Plot Templates Dialog

.. _chartreuse-plot-templates-barchart:

=============================
Bar Chart Variable Comparison
=============================

.. image:: img/Plotting_Templates_BarChart_1.png
   :alt: Bar chart example
   
|

.. image:: img/PlottingTemplates_BarChart.png
   :alt: Bar chart options

- **Plot Data** Select a plottable data source, as in :ref:`the Plot Trace Creator dialog <chartreuse-plot-trace-getting-data>`.
- **Choose canvas orientation** Determine whether generated canvases will be added horizontally or vertically.
- **Choose bar orientation** Determine whether bars in the bar charts will be oriented vertically or horizontally.
- **Create canvas by** Create one canvas per response or one canvas per variable.  If "Response" is chosen, variables will act as the independent variables
  of each canvas.  If "Variable" is chosen, responses will act as the independent variables of each canvas.  If "All" is chosen, a canvas will be
  created for each variable and each response, and each variable/response relationship will be represented on each canvas.  "All" is not recommended
  unless you have a small number of variables and responses.
- **Autoscale individual canvases** If selected, "autoscale" rules are applied to each canvas, so that each canvas ignores the scale of every other canvas.
- **Sort bars by size (tornado)** Organizes the bars of each bar chart by *absolute value magnitude*, to create the visual image of a tornado.
- **Filter Parameters and Responses** Use these options to filter parameters and responses from being included in the final plot.

.. _chartreuse-plot-templates-cps:

========================
Centered Parameter Study
========================

.. image:: img/Plotting_Templates_CPS_3.png
   :alt: Centered parameter study response comparison

.. image:: img/PlottingTemplates_CPS.png
   :alt: Centered parameter study options

- **Plot Data** Select a plottable data source, as in :ref:`the Plot Trace Creator dialog <chartreuse-plot-trace-getting-data>`.
- **Choose an orientation** Determine whether generated canvases will be added horizontally or vertically.
- **Filter Parameters and Responses** Use these options to filter parameters and responses from being included in the final plot.

.. _chartreuse-plot-templates-corrcoeff:

=============================
Correlation Coefficient Table
=============================

.. image:: img/Plotting_Templates_Correlation_Matrix_3.png
   :alt: Correlation coefficients example
   
|

.. image:: img/PlottingTemplates_CorrCoeff.png
   :alt: Correlation coefficients table options

.. note::
   The color scale for your correlation coefficients table is not controlled by this template.  Instead, this is
   controlled by :ref:`the global Chartreuse preference for default color scale <gui-preferences-chartreuse>`.

- **Plot Data** Select a plottable data source, as in :ref:`the Plot Trace Creator dialog <chartreuse-plot-trace-getting-data>`.
- **Choose an orientation** Determine whether the correlation coefficients table should put responses on the vertical axis and
  variables on the horizontal axis, or vice versa.  You can also choose "Mirror All Variables" to put both variables and responses on both the horizontal and vertical axes.
- **Filter Parameters and Responses** Use these options to filter parameters and responses from being included in the final plot.

.. _chartreuse-plot-templates-iterhistory:

=================
Iteration History
=================

.. image:: img/Plotting_Templates_IterationHistory_1.png
   :alt: Iteration history example

|

.. image:: img/PlottingTemplates_IterHistory.png
   :alt: Iteration history options

- **Plot Data** Select a plottable data source, as in :ref:`the Plot Trace Creator dialog <chartreuse-plot-trace-getting-data>`.
- **Choose an orientation** Determine whether generated canvases will be added horizontally or vertically.
- **Overlay response canvases** Group responses onto a single stack of overlaid canvases.
- **Ignore unchanging data** If a variable's value does not change over the course of the study, it does not get its own canvas in the final plot.
- **Filter Parameters and Responses** Use these options to filter parameters and responses from being included in the final plot. 

.. _chartreuse-plot-templates-scatterplotmatrix:

===================
Scatter Plot Matrix
===================

.. image:: img/Plotting_Templates_Matrix_2.png
   :alt: Scatter plot matrix example
   
|

.. image:: img/PlottingTemplates_ScatterPlotMatrix.png
   :alt: Scatter plot matrix options

- **Plot Data** Select a plottable data source, as in :ref:`the Plot Trace Creator dialog <chartreuse-plot-trace-getting-data>`.
- **Choose a type of scatter plot**

  - **Subset A to Subset B** Allows you to put variables along one axis and responses along the other axis.
  - **All to All** Allows you to put all variables and all responses on each axis.

.. image:: img/PlottingTemplates_ScatterPlotMatrix_2.png
   :alt: Scatter plot matrix options

- **X Axis** Specify the data for the X axis (only available for the "Subset A to Subset B" option)
- **Y Axis** Specify the data for the Y axis (only available for the "Subset A to Subset B" option)
- **Axis** Specify the data for both axes (only available for the "All to All" option)
- **Draw Linear Regression** Draw a linear regression line on each generated canvas.
- **Blank Diagonal** Do not put data along the diagonal, where a variable or response will be mapped against itself (only available for the "All to All" option)
- **Draw Lower Half Only** Do not put mirrored data on the upper triangle of the grid of canvases (only available for the "All to All" option)
- **Text on Outer Axes Only** Do not repeat canvas text for canvases that are "inside" the grid.  If unchecked, this will lead to redundant canvas labels throughout your final plot.

.. _chartreuse-plot-templates-sobol:

==============
Sobol' Indices
==============

.. image:: img/Plotting_Templates_Sobol.png
   :alt: Sobol indices

|

.. image:: img/PlottingTemplates_SobolIndices.png
   :alt: Sobol indices options

.. note::
   If you select a plottable data file that contains Sobol index data, the Plotting Templates dialog will automatically retrieve this data for you. 

- **Plot Data** Select a plottable data source, as in :ref:`the Plot Trace Creator dialog <chartreuse-plot-trace-getting-data>`.
- **Effects Type** For Sobol indices, we can plot the main effects dataset, the total effects dataset, or both.
- **One canvas per effects type** Split main effects and total effects onto separate canvases.
- **One canvas per response type** Create a canvas for each response.
- **Sort** Sort the bars by absolute value magnitude (i.e. a tornado plot).
- **Filter Parameters and Responses** Use these options to filter parameters and responses from being included in the final plot.  
.. _ngw-node-glossary:

"""""""""""""""""""""""""""""""
Next-Gen Workflow Node Glossary
"""""""""""""""""""""""""""""""

.. _ngw-node-help:

For any node not listed in the glossary below, you can open that node's documentation by opening the Settings editor view for that node,
and then clicking on the small "?" button to the right of the Settings editor banner:

.. image:: img/Plotting_HDF_3.png
   :alt: Documentation for individual workflow nodes is available by clicking the small blue ? in the top-right corner of the dialog.

========
Blackbox
========

.. toctree::
   :maxdepth: 1

   nodes/dataFilter

======
Dakota
======

.. toctree::
   :maxdepth: 1
   
   nodes/dakota
   nodes/dakotaParametersMap
   nodes/dakotaQueueSubmit
   nodes/dakotaResultsFile
   nodes/dakotaWorkflowDriver
   nodes/dprepro
   nodes/posPrePro
   nodes/pyprepro
   nodes/qoiExtractor
   nodes/surrogate

=================================
"Data Visualization (Chartreuse)"
=================================

.. toctree::
   :maxdepth: 1
   
   nodes/plotCanvas
   nodes/plotWindow
   nodes/screenshot
   nodes/toPlotlyJS
   nodes/traceBoxPlot
   nodes/traceContour
   nodes/traceHeatMap
   nodes/traceHistogram
   nodes/traceScatterPlot2d
   nodes/traceScatterPlot3d
   nodes/traceSurfacePlot3d

=====================================================
"Data Visualization (Dakota/Chartreuse Simple Plots)"
=====================================================

.. toctree::
   :maxdepth: 1

   nodes/hdfTraceBarChartPartialCorrelation
   nodes/hdfTraceBarChartPartialRankCorrelation
   nodes/hdfTraceBarChartSimpleCorrelation
   nodes/hdfTraceBarChartSimpleRankCorrelation
   nodes/hdfTraceBoxPlot
   nodes/hdfTraceContour
   nodes/hdfTraceHeatMap
   nodes/hdfTraceHeatMapPartialCorrelation
   nodes/hdfTraceHeatMapPartialRankCorrelation
   nodes/hdfTraceHeatMapSimpleCorrelation
   nodes/hdfTraceHeatMapSimpleRankCorrelation
   nodes/hdfTraceHistogram
   nodes/hdfTraceScatterPlot2d
   nodes/hdfTraceScatterPlot3d
   nodes/hdfTraceSurfacePlot3d
   nodes/scatterPlotMatrix
   nodes/sobolIndices
   nodes/variablePdfVisualizer

=====================================================
"Data Visualization (Dakota/Chartreuse Megaplotters)"
=====================================================

.. toctree::
   :maxdepth: 1

   nodes/calibrationHistory
   nodes/categoricalBarCharts
   nodes/categoricalScatterPlots
   nodes/incrementalLHS
   nodes/LHS
   nodes/optimization
   nodes/responseCompare
   nodes/scatterPlotSuite
   nodes/variablePdfVisualizerComposite

===
HDF
===

.. toctree::
   :maxdepth: 1

   nodes/hdfExtractColumn
   nodes/hdfExtractRow
   
=================
Remote Operations
=================

.. toctree::
   :maxdepth: 1

   nodes/remoteNestedWorkflow
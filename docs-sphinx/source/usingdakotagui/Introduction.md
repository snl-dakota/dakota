Introduction
===============

The Dakota GUI is an application that allows you to easily edit Dakota input files, run Dakota, and visually plot Dakota's output data.  The Dakota GUI also aids in the process of defining a simulation model and connecting Dakota to that simulation model, without the need to write pre-processing or post-processing scripts.

This manual is structured as a reference guide.  The sections can be read in any order, depending on which topic you’re interested in learning about.

## Getting Started

* **I'm completely new to Dakota.  Where do I start?**
  * [Watch the introductory series of Dakota screencasts.](FurtherHelpResources.md#screencasts)
  * [Peruse the Eclipse section of this manual to get up to speed on basic GUI terminology and concepts.](Eclipse.md) 
* **Where do I find an example Dakota study to start from?**
  * [Use the Dakota Examples Search dialog.](DakotaExamples.md) 
* **I already have a Dakota study.  How do I import it into the GUI?**
  * [Use basic file importing functionality.](Eclipse.md#file-import)
  * [If the input file has outdated syntax, use the "Dakota Order Input Utility" context menu tool.](ContextMenuTools.md#dakota-order-input-utility)
* **I already have a Dakota study, but I just want to change one block.**
  * [Try one of the Dakota block recipes available in the Dakota Text Editor.](DakotaInputFiles.md#block-recipes)
* **My Dakota study has long arrays of numbers.  Can I modify these numbers without manually editing the text of the input file?**
  * [Use the Dakota Visual Editor for editing long number arrays.](DakotaVisualEditor.md) 
* **How do I create a new Dakota study from scratch, using a script to communicate with my black-box simulation model?**
  * [Step 1. Read the strategy overview.](ExternalSimulationModelOverview.md)
  * [Step 2. Create a parameterized simulation model representing your black box.](BMF.md)
  * [Step 3. Create a driver script to wrap the parameterized simulation model.](Wizards.md#script-based-dakota-driver-wizard)
  * [Step 4. Create a new Dakota study using the Dakota Study Wizard, providing your driver script as the analysis driver.](Wizards.md#dakota-study-wizard)
* **How do I create a new Dakota study from scratch, using Next-Gen Workflow to communicate with my black-box simulation model?**
  * [Step 1. Read the strategy overview.](ExternalSimulationModelOverview.md)
  * [Step 2. Create a parameterized simulation model representing your black box.](BMF.md)
  * [Step 3. Create an empty driver workflow using the Workflow-Based Dakota Driver wizard.](Wizards.md#workflow-based-dakota-driver)
  * [Step 4. Design a workflow that will execute your simulation model.](NextGenWorkflow.md#nested-workflow-tutorial)
  * [Step 5. Create a new Dakota study using the Dakota Study Wizard, providing your workflow as the analysis driver.](Wizards.md#dakota-study-wizard)
  * [Step 6. Create an outer wrapper workflow that will run Dakota.](Wizards.md#dakota-wrapper-workflow-wizard)
* **I already have a driver script for my Dakota study.  How do I get Dakota GUI to acknowledge it?**
  * [Use the "Recognize as Analysis Driver" context menu tool.](ContextMenuTools.md#using-an-existing-script-file-as-a-driver)
* **How do I create plots of my Dakota output data?**
  * [Option 1:  Use the Chartreuse Sandbox View to quickly visualize your data, without any configuration.](ChartreuseSandbox.md) 
  * [Option 2:  Use one of the Chartreuse plotting templates to quickly create a plot tailored to specific types of Dakota studies.](Chartreuse.md#plotting-template-basics)
  * [Option 3:  Use the Plot Trace Creator dialog to take full control over the creation of your plot.](Chartreuse.md#plot-trace-basic)
* **How do I create a plot from a tabular data file, using Next-Gen Workflow?**
  * [Build a data column-extracting workflow in Next-Gen Workflow.](Chartreuse.md#plotting-using-the-workflow-engine)
* **How do I create a plot from an HDF5 database file, using Next-Gen Workflow?**
  * [Option 1:  Use basic HDF/Dakota data plotting nodes.](Chartreuse.md#basic-hdf-plot-nodes)
  * [Option 2:  Use advanced HDF/Dakota "composite plotter" nodes.](Chartreuse.md#plotting-composite-plotters) 

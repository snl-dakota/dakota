.. _dakota-gui-quickstart:

==============
GUI Quickstart
==============

- I'm completely new to Dakota.  Where do I start?

 - :ref:`Watch the introductory series of Dakota screencasts.<help-screencasts>`
 - :ref:`Peruse the Eclipse section of this manual to get up to speed on basic GUI terminology and concepts.<gui-eclipse-main>`
 
- Where do I find an example Dakota study to start from?

 - :ref:`Use the Dakota Examples Search dialog.<gui-examples-main>`
 
- I already have a Dakota study ready to run.  How do I import it into the GUI?

 - :ref:`Use basic file importing functionality.<gui-eclipse-file_import>`
 - :ref:`If the input file has outdated syntax, use the "Dakota Order Input Utility" context menu tool.<gui-context-tools-order-input-util>`
 
- I don't have a Dakota study, but I have a simulation model / black-box model / analysis driver. How do I create a new Dakota study for it?

 - :ref:`Step 1. Read the strategy overview.<couplingtosimulations-main>`
 - :ref:`Step 2. Create inputs and outputs for your simulation model in the GUI.<gui-simulation-models-overview>`
 - :ref:`Step 3. Create a new Next-Gen Workflow using the Workflow-Based Dakota Driver wizard.<wizards-newworkflowdriver-main>`
 - :ref:`Step 4. Design a workflow that will execute your simulation model.<gui-nestedworkflowtutorial-main>`
 - :ref:`Step 5. Create a new Dakota study using the Dakota Study Wizard, providing your workflow as the analysis driver.<wizards-newdakotastudy>`
 - :ref:`Step 6. Choose the "Run As > Dakota (Workflow)" option to run your Dakota study.<gui-run-configurations-workflow>` 
 
- I already have a driver script for my Dakota study.  How do I get Dakota GUI to acknowledge it?

 - :ref:`Use the "Recognize as Analysis Driver" context menu tool.<gui-context-tools-recognize-driver>` 
 
- I already have a Dakota study, but I just want to change one block.

 - :ref:`Try one of the Dakota block recipes available in the Dakota Text Editor.<gui-inputfiles-blockrecipes>`
 
- My Dakota study has long arrays of numbers.  Can I modify these numbers without manually editing the text of the input file?

 - :ref:`Use the Dakota Visual Editor for editing long number arrays.<gui-visual-editor-main>`

- How do I create plots of my Dakota output data?

 - :ref:`Option 1:  Use the Chartreuse Sandbox View to quickly visualize your data, without any configuration.<chartreuse-sandbox-main>`
 - :ref:`Option 2:  Use one of the Chartreuse plotting templates to quickly create a plot tailored to specific types of Dakota studies.<chartreuse-plot-templates>`
 - :ref:`Option 3:  Use the Plot Trace Creator dialog to take full control over the creation of your plot.<chartreuse-plot-trace>`

- How do I create a plot from a tabular data file, using Next-Gen Workflow?

 - :ref:`Build a data column-extracting workflow in Next-Gen Workflow.<chartreuse-ngw-main>`

- How do I create a plot from an HDF5 database file, using Next-Gen Workflow?

 - :ref:`Option 1:  Use basic HDF/Dakota data plotting nodes.<chartreuse-ngw-hdf-tutorial>`
 - :ref:`Option 2:  Use advanced HDF/Dakota "megaplotter" nodes.<chartreuse-ngw-composite-plotters>` 
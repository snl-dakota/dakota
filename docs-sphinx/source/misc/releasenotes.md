Release Notes
=============

# Dakota Release Notes

TODO

# Dakota GUI Release Notes

## [Version 6.15](https://dakota.sandia.gov/content/dakota-615)
### Lakeside Limbo (Released November 2021)

* **Multi-level / Multi-fidelity Editor (Experimental)**
 * New Dakota input file editor tailored to working with multi-level / multi-fidelity studies.  Allows for easy manipulation of model hierarchies and solution level costs.
* **Chartreuse**
 * New Next-Gen Workflow node for simultaneously visualizing the uncertainty distributions of all study variables
 * Support more variable distribution types (histogram\_bin\_uncertain, histogram\_point\_uncertain)
 * Enhancements and bugfixes for the calibrationHistory node in Next-Gen Workflow.
 * Chartreuse section of Preferences dialog now shows you a preview of the default color scale.
* **Dakota Help**
 * GUI now reverts to the most recently released documentation URL if a specific version of Dakota's reference manual cannot be loaded
* **Visual Editor**
 * Added form-based editing support for OPT++ family of methods.
 * Some options now available in context menus throughout the Visual Editor (pre-processor replacement, Dakota reference manual access)

## [Version 6.14](https://dakota.sandia.gov/content/dakota-614)
### Koindozer Klamber (Released May 2021)

* **Dakota Surrogates**
 * New GUI support for serialized Dakota surrogates files.
* **Dakota Input File Editing**
 * Editing support for the coliny_ea method.
 * Editing support for individual response properties.
 * Added support for pre-processor syntax in the Dakota Visual Editor with the hotkey Ctrl+Shift+P.
* **Chartreuse (Next-Gen Workflow Integration)**
 * New CalibrationHistory plotting node.
 * New ScatterPlotSuite node, for paging through large sets of scatter plots.
 * Custom X/Y labels for basic Chartreuse plotting nodes in workflow
* **Next-Gen Workflow**
 * New surrogates node, for applying Dakota surrogates files to workflows. 
 * New variablePdfVisualizer node
 * Refactored the Dakota node for advanced handling of Dakota parameters and results files
* **Chartreuse**
 * Show Dakota eval id on point hover text
 * weibull and log_normal supported for variable distribution visualization
* **Misc.**
 * *Deprecation note:* Output stream scraping has been removed from Dakota GUI

## [Version 6.13](https://dakota.sandia.gov/content/dakota-613)
### Jungle Japes (Released November 2020)

* **Chartreuse**
 * New "Sandbox View" for fast visualizations of generic data using Chartreuse.
 * Support added to Chartreuse for CSV files
 * Four-dimensional Chartreuse scatter plots (i.e. time-based node coloring)
* **Dakota Input File Editing**
 * New form-based editors for Dakota interface blocks and hybrid method blocks.
 * Limited support for visualization of Dakota uncertainty variables (normal, lognormal, weibull)
 * Pre-processing markup supported in Dakota text editor, which also provides a new mechanism for assigning multiple NGW-based analysis drivers at runtime.
 * Dark theme support for Dakota text editor
* **QOI**
 * New column-based QOI extractors for extracting fields from tabular/CSV-based data
 * *Warning:*  The qoiExtractor node in Next-Gen Workflow has received backwards-incompatible changes.  You must delete your qoiExtractor nodes and reconfigure them upon switching to 6.13
* **Misc.**
 * "dprepro" node added to Next-Gen Workflow
 * General enhancements for the New Dakota Study wizard

## [Version 6.12](https://dakota.sandia.gov/content/dakota-612)
### Ice Age Alley (Released June 2020)
* **Chartreuse Plotting**
 * New "relative threshold" feature allows you to define graph color scales on a 0-100 percentage scale.
 * Enabled custom decimal precision for canvas axes.
 * Enabled quadrant labeling for non-categorical heat maps.
 * When plotting correlation or rank correlation datasets, all dataset combinations (variable-response, response-variable, response-response, variable-variable and all-all) are now supported.
 * Better controls for ignoring inactive variables when generating plots.
* **Quantity of Interest extractors:** New "Last Anchor Occurrence" QOI extractor added
* A variety of small bugfixes to the Basic Model Format (BMF) creator, the New Dakota Study wizard, and the Dakota text editor

## [Version 6.11](https://dakota.sandia.gov/content/dakota-611)
### Hot Head Hop (Released November 2019)
The Dakota GUI has added many significant feature improvements over the last year.  The Dakota GUI now allows you to seamlessly browse Dakota's HDF5 database output files, as well as generate sophisticated graphical plots from HDF5 data with just a few clicks.  HDF5 browsing and plot generation can also be driven within Dakota GUI by Next-Gen Workflow, a powerful tool that allows you to construct node-based workflows for complex tasks.

**Enabling / Accessing:**  Dakota GUI ships with Dakota and is available for Windows, Mac, and RHEL7.

**Documentation:**  An enhanced version of the Dakota GUI manual now ships with the GUI, giving you easy access to a wealth of reference material for using the GUI.  

## [Version 6.10](https://dakota.sandia.gov/content/dakota-610)
###Gangplank Galleon (Released May 2019)
The Dakota 6.9 GUI has been updated to work with Dakota 6.10, but not substantially changed. New GUI features will be released with the next Dakota release.

## [Version 6.9](https://dakota.sandia.gov/content/dakota-69)
### Frantic Factory (Released November 2018)
**Highlights:**

* All-new Dakota Study Wizard which guides you through a series of questions to configure a Dakota input file, including choosing an appropriate method.
* Improved simulation inferfacing - Stronger interoperation between wizard-generated Python driver scripts, Next-Gen Workflow drivers, and Dakota studies generated using the New Dakota Study Wizard.
* A variety of Dakota input file text editor enhancements, including completion proposals, alias recognition, more reliable error markers, and formatting options.
* Next-Gen Workflow engine supports graphical plotting nodes.

**Details:**

* New Dakota Study Wizard
 * The wizard supports a host of different data sources for generating Dakota studies.
 * The wizard asks you a series of questions to help you choose a Dakota method.
 * Some chosen methods are pre-populated with heuristic "getting started" settings for your study problem.
 * The wizard has seamless support for both GUI-generated Python scripts and nested workflows as your study's target analysis driver.
* Script-Based Dakota Driver Wizard (formerly Dakota-to-Python Interface Wizard)
 * In addition to a Python driver script, this wizard now generates an "interface manifest" file (see below)
 * Generated Python script can be configured to echo stdout of underlying simulation model, as well as prepend its stdout stream with Dakota run number information.
* Nested Workflow for Dakota Driver Wizard:
 * A new wizard that auto-populates a Next-Gen Workflow file with parameters and responses based on your Dakota study.
* Interface Manifest Support
 * Dakota GUI now accepts arbitrary drivers that define "interface manifest" files - that is, a file that formally states what input and output the driver expects to receive and send.  This interface manifest feature allows files to present themselves as Dakota drivers in multiple contexts, making for easier connection to multiple Dakota studies.
 * Recognized driver formats now include:  Python scripts, SH and CSH scripts, Windows BAT scripts, Next-Gen Workflow files, Perl scripts, VBS scripts.
 * "Recognize as Analysis Driver" context menu option for creating interface manifests for arbitrary files that should be recognized as Dakota drivers.
* Dakota Text Editor
 * Support for Eclipse completion proposals
 * The Dakota text editor now reports errors for duplicate IDs
 * The Dakota text editor now reports that multiple methods without IDs are ambiguous
 * Better error markup for unrecognized keywords and unrecognized parameter list keywords.
 * Text editor now observes both keyword aliases (i.e. alternate keywords according to the Dakota grammar) and partial, non-ambiguous keywords (within reason)
 * Defaults for Dakota text file indentation & quote type are configurable in the Preferences dialog
* Project Navigator View
 * "Add Keyword" context menu option for Dakota input files
* Next-Gen Workflow
 * "Expert mode" flag added to DakotaNode.  Leaving unchecked allows nested workflows to run with fewer nodes.
 * Most Chartreuse plot types supported as Next-Gen Workflow nodes.
 * Changed DakotaNode’s "search order" for the location of the runtime workflow install directory.
* Chartreuse (aka Integrated plotly.js Plotting)
 * Support for contour plots
 * Plot data providers inform the plotting dialogs about completeness of data
 * The Dakota plot data provider now sets reasonable defaults for simple cases.
* Dakota Console
 * Errors related to launching Dakota from the GUI are now explicitly shown via pop-up dialogs.
* Misc
 * More robust behavior around resolving the path to Dakota at launch time.

**Bugfixes:**

* Script-Based Dakota Driver Wizard:  Feature to auto-substitute new interface block into Dakota input file is no longer broken.
* Dakota Text Editor:  The Dakota text editor now scopes variable duplicate checking to be per-block.
* Dakota Console:  Output and error streams no longer interleave characters together.
* Project Navigator View:  Fixed bug where navigator view of Dakota input file wouldn't always update on a change to the text.

## [Version 6.8](https://dakota.sandia.gov/content/dakota-68)
### Elevator Antics (Released May 2018)
Dakota's graphical user interface (GUI) was substantially updated.

* Dakota input files are fully explorable in the project navigator tree.
* Workspace, project, and file/folder management mechanisms that are standard in the Eclipse framework were adopted.
* Dakota may be run through Eclipse run configurations.
* The analysis driver creation wizard was refactored.
* A preliminary (alpha) version of a new workflow engine has been incorporated. It includes specific nodes for pre-processing and post-processing data files (assists with Dakota interfacing with a simulation model) and for launching Dakota as part of a larger workflow

A number of bugfixes and other enhancements were made as well:

* Added support for built-in Plotly plotting on Linux platforms
* Better default behavior for unlabeled response values, e.g., that were causing correlation matrix parsing to fail
* Squelched "phantom" pop-up dialog behind real dialogs on Linux build
* Bugfix: Mac/Linux version of Dakota GUI appears to truncate Dakota output stream

## [Version 6.7](https://dakota.sandia.gov/content/dakota-67)
### Doorstop Dash (Released November 2017)

* GUI plotting enhancements:
 * Support for log scales
 * Ability to show multiple plot "canvases" together on a grid
 * User-created plots are now top-level Dakota project components
* GUI enhancements/bugfixes:
 * Driver script is automatically created and connected if a Dakota study is created using the wizard and a known simulation model
 * Improved controls for determining Dakota's working directory
 * Improved support in the GUI for shipped Dakota examples
 * Support for renaming Dakota projects
 * Reorganized GUI menu options

## [Version 6.6](https://dakota.sandia.gov/content/dakota-66)
### Coral Capers (Released May 2017)

* Plotly Plotting Integration. The GUI now integrates the Plotly plotting library. You can perform basic scatter plots, histograms, and box-and-whisker plots against Dakota tabular output data.
* Input specification ordering. Reorder and clean up interface, environment, and model blocks to follow more natural ordering and support GUI integration.
* Reordering of Dakota Example Inputs. Several of Dakota's shipped example input files have been reordered to work out-of-the-box in the GUI.
* NIDR Parser Utilities. Developer productivity: NIDR utilities built on all platforms to allow fewer NIDR-generated files in repository and input file reordering in GUI
* Use of examples in GUI. Examples from training, users, and advanced directories are now ordered and debugged to work in GUI

## [Version 6.5](https://dakota.sandia.gov/content/dakota-65) (GUI Version 0.2)
### Blazing Bazookas (Released November 2016)

The first beta release of the new Dakota GUI is now available for download.

## [Version 6.4](https://dakota.sandia.gov/release-notes/6.4) (GUI Version 0.1)
### Arctic Abyss (Released May 2016)

The first experimental release of Dakota GUI was created for Dakota version 6.4, but was not advertised.
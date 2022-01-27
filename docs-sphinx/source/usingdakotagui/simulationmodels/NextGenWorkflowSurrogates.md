Dakota Surrogates in Workflows
=================

The Dakota GUI supports replacing components of NGW workflows with Dakota-generated surrogates files.  To do this, drag a "surrogate" node onto the canvas (from the Dakota folder in the Palette).

![alt text](img/NGW_Surrogates_1.png "An unassuming surrogate node")

A new surrogate node will have no input or output ports until we associate it with a surrogate file.  Open the Settings Editor for the node and select a Dakota surrogate file (both .bin and .txt formats are supported).

![alt text](img/NGW_Surrogates_2.png "The surrogateFile field must be populated before the surrogate node can be used!")

After selecting the surrogate, the node will auto-create the necessary input and output ports to interface with the surrogate as part of a workflow.

![alt text](img/NGW_Surrogates_3.png "Inputs and outputs galore!")

![alt text](img/NGW_Surrogates_4.png "An example workflow using a surrogate node")

To learn more about Dakota surrogates, consult [Section 8.4 of the Dakota User's Manual](https://dakota.sandia.gov/content/manuals).
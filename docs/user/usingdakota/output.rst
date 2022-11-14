.. _output:

Dakota Output
=======================================

.. _`output:overview`:

Dakota’s output focuses on succinct, text-based reporting of the iterations
and function evaluations performed by an algorithm.

In a number of contexts, Dakota generates tabular output useful for data analysis
and visualization with external tools. Beyond numerical results, all output files
provide information that allows the user to check that the actual analysis was the intended analysis.

Additionally, the Dakota GUI :ref:`provides visualization facilities <chartreuse-main>`
that have replaced the optional basic graphical output capability provided with Unix versions
of Dakota.

Last but not least, :ref:`HDF5 output <hdf5_output>` has been added to Dakota's suite of output formats.

.. toctree::
   :maxdepth: 1
   
   output/standardoutput
   output/standarderror
   output/tabulardata
   output/hdf
   output/graphicslegacy
   output/otheroutput

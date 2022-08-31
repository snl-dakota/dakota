.. _output:

Dakota Output
=======================================

.. _`output:overview`:

While Dakota primarily targets complex numerical simulation codes that are run on
massively parallel supercomputers, Dakota’s output focuses on succinct,
text-based reporting of the iterations and function evaluations
performed by an algorithm. In a number of contexts, Dakota generates
tabular output useful for data analysis and visualization with external
tools.

Beyond numerical results, all output files provide information that allows
the user to check that the actual analysis was the intended analysis.

The Dakota GUI increasingly provides visualization facilities that have replaced
the optional basic graphical output capability provided with Unix versions
of Dakota. Additionally, HDF5 output has been added to Dakota's suite of
output formats.

.. note::

   Whenever an output file is specified for a Dakota run, the screen output itself becomes quite minimal,
   consisting of version statements, environment statements and execution times.

.. toctree::
   :maxdepth: 1
   
   output/standardoutput
   output/standarderror
   output/tabulardata
   output/restart
   output/restartutility
   output/hdf
   output/graphicslegacy
   output/pceoutput
   output/surrmodeloutput
   output/prerunoutput
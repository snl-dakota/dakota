.. _hdf5_output:

"""""""""""
HDF5 Output
"""""""""""

The results of most Dakota studies, such as correlation matrices computed by sampling studies and the
best parameters discovered in an optimization, can optionally be written to HDF5. Dakota's HDF5 output
file also optionally contains evaluation data (variables and responses for each model or interface
evaluation). Many users may find the HDF5 format more convenient than scraping or copying and 
pasting from Dakota's console output.

To enable HDF5 output, the :dakkw:`environment-results_output` keyword with 
the :dakkw:`environment-results_output-hdf5` option must be added to the Dakota input
file. In additon, Dakota must have been built :ref:`with HDF5 support <configure:hdf5>`. HDF5 is 
enabled in our publicly available downloads.

.. toctree::
   :maxdepth: 1
   
   hdf/concepts
   hdf/accessingresults
   hdf/organizationofresults
   hdf/organizationofevaluations
   hdf/distributionparameters

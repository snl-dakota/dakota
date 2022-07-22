.. _hdf5_output:

"""""""""""
HDF5 Output
"""""""""""

Beginning with release 6.9, Dakota gained the ability to write many method results such as the correlation matrices computed by sampling studies and the best parameters discovered by optimization methods to disk in HDF5. In Dakota 6.10 and above, evaluation data (variables and responses for each model or interface evaluation) may also be written. Many users may find this newly supported format more convenient than scraping or copying and pasting from Dakota's console output.

To enable HDF5 output, the results_output keyword with the hdf5 option must be added to the Dakota input file. In additon, Dakota must have been built with HDF5 support. Beginning with Dakota 6.10, HDF5 is enabled in our publicly available downloads. HDF5 support is considered a somewhat experimental feature. The results of some Dakota methods are not yet written to HDF5, and in a few, limited situations, enabling HDF5 will cause Dakota to crash.

.. toctree::
   :maxdepth: 1
   
   hdf/concepts
   hdf/accessingresults
   hdf/organizationofresults
   hdf/organizationofevaluations
   hdf/distributionparameters
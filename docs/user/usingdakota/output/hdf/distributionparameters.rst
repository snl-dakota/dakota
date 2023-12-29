.. _hdf5_distribution_parameters:

"""""""""""""""""""""""
Distribution Parameters
"""""""""""""""""""""""

Variables are characterized by parameters such as the mean and standard deviation or lower and upper bounds. Typically, users provide these parameters as part of their input to Dakota, but Dakota itself may also compute them as it scales and transforms variables, normalizes empirical distributions (e.g. for ``histogram_bin_uncertain`` variables), or calculates alternative parameterizations (lambda and zeta vs mean and standard deviation for a ``lognormal_uncertain``).

Beginning with release 6.11, models write their variable’s parameters to HDF5. The information is located in each model's ``properties/variable_parameters`` subgroup. Within this group, parameters are stored by Dakota variable type (e.g. ``normal_uncertain``), with one 1D dataset per type. The datasets have the same names as their variable types and have one element per variable. Parameters are stored by name.

Consider the following variable specification, which includes two normal and two uniform variables: 

.. code-block::

  variables 
    normal_uncertain 2 
      descriptors 'nuv_1' 'nuv_2' 
      means 0.0 1.0
      std_devations 1.0 0.5
    uniform_uncertain 2
    descriptors 'uuv_1' 'uuv_2'
      lower_bounds -1.0 0.0
      upper_bounds  1.0 1.0
      
Given this specification, and assuming a model ID of “tb_model”, Dakota will write two 1D datasets, both of length 2, to the group ``/models/simulation/tb_model/metadata/variable_parameters``, the first named ``normal_uncertain``, and the second named ``uniform_uncertain``. Using a JSON-like representation for illustration, the ``normal_uncertain`` dataset will appear as:

.. code-block::

  [
    {
      "mean": 0.0,
      "std_deviation": 1.0,
      "lower_bound": -inf, 
      "upper_bound": inf
    },
    { 
      "mean": 1.0,
      "std_deviation": 0.5,
      "lower_bound": -inf,
      "upper_bound": inf
    }
  ]
  
The uniform_uncertain dataset will contain: 

.. code-block::

  [
    {
      "lower_bound": -1.0, 
      "upper_bound":  1.0
    },
    { 
      "lower_bound": 0.0,
      "upper_bound": 1.0
    }
  ]
  
In these representations of the ``normal_uncertain`` and ``uniform_uncertain`` datasets, the outer square brackets ([]) enclose the dataset, and each element within the datasets are enclosed in curly braces ({}). The curly braces are meant to indicate that the elements are dictionary-like objects that support access by string field name. A bit more concretely, the following code snippet demonstrates reading the mean of the second normal variable, nuv_2.

.. code-block:: python
   :linenos:

   import h5py 
   
   with h5py.File("dakota_results.h5') as h:
       model = h["/models/simulation/tb_model/"]
       # nu_vars is the dataset that contains distribution parameters for
       # normal_uncertain variables
       nu_vars = model["variable_parameters/normal_uncertain"]
       nuv_2_mu = nu_vars[1]["mean"] # 1 is the 0-based index of nuv_2, and 
                                     # "mean" is the name of the field where 
                                     # the mean is stored; nuv_2_mu now contains
                                     # 1.0.
   
The feature in HDF5 that underlies this name-based storage of fields is compound datatypes, which are similar to C/C++ structs or Python dictionaries. Further information about how to work with compound datatypes is available in the h5py documentation.

=============================
Naming Conventions and Layout
=============================

In most cases, datasets for storing parameters have names that match their variable types. The ``normal_uncertain`` and ``uniform_uncertain`` datasets
illustrated above are examples. Exceptions include types such as :ref:`discrete_design_set<variables-discrete_design_set>`, which has string, integer, and
real subtypes. For these, the dataset name is the top-level type with ``_string``, ``_int``, or ``_real`` appended: ``discrete_design_set_string``,
``discrete_design_set_int``, and ``discrete_design_set_real``.

Most Dakota variable types have scalar parameters. For these, the names of the parameters are generally the singular form of the associated Dakota keyword.
For example, :ref:`triangular_uncertain<variables-triangular_uncertain>` variables are characterized in Dakota input using the plural keywords ``modes``,
``lower_bounds``, and ``upper_bounds``. The singular field names are, respectively, "mode", "lower_bound", and "upper_bound". In this case, all three
parameters are real-valued and stored as floating point numbers, but variable types/fields can also be integer-valued
(e.g. ``binomial_uncertain/num_trials``) or string-valued.

Some variable/parameter fields contain 1D arrays or vectors of information. Consider :ref:`histogram_bin_uncertain variables<variables-histogram_bin_uncertain>`,
for which the user specifies not just one value, but an ordered collection of abscissas and corresponding ordinates or counts. Dakota stores the abscissas in the
"abscissas" field, which is a 1D dataset of floating-point numbers. It similarly stores the counts in the "counts" field. (In this case, only the normalized counts
are stored, regardless of whether the user provided counts or ordinates.)

When the user specifies more than one ``histogram_bin_uncertain`` variable, it often is also necessary to include the :ref:`pairs_per_variable<variables-histogram_bin_uncertain-pairs_per_variable>`
keyword to divide the abscissa/count pairs among the variables. This raises the question of how lists of parameters that vary in length across the variables ought to be stored.

Although HDF5 supports variable-length datasets, for simplicity (and due to limitations in h5py at the time of the 6.11 release), Dakota stores vector parameter
fields in conventional fixed-length datasets. The lengths of these datasets are determined at runtime in the following way: For a particular variable type
and field, the field for all variables is sized to be large enough to accommodate the variable with the longest list of parameters. Any unused space for a
particular variable is filled with ``NaN`` (if the parameter is real-valued), ``INTMAX`` (integer-valued), or an empty string (``string-valued``). In addition, each variable
has an additional field, "num_elements", that reports the number of elements in the fields that contain actual data and not fill values.

Consider this example, in which the user has specified a pair of ``histogram_bin_uncertain`` variables. The first has 3 pairs, and the second has 4.

.. code-block::

  variables
    histogram_bin_uncertain 2
      pairs_per_variable 2 3
      abscissas  0.0   0.5  1.0 
                -1.0  -0.5  0.5  1.0 
      counts     0.25  0.75 0.0 
                 0.2   0.4  0.2  0.0
                 
For this specification, Dakota will write a dataset named ``histogram_bin_uncertain`` to the ``metadata/variable_parameters/`` subgroup for the model. It will be of
length 2, one element for each variable, and contain the following:

.. code-block::

  [
    {
      "num_elements": 3,
      "abscissas": [0.0, 0.5, 1.0, NaN],
      "counts": [0.25, 0.75, 0.0, NaN]
    },
    {
      "num_elements": 4,
      "abscissas": [-1.0, -0.5, 0.5, 1.0],
      "counts": [0.2, 0.4, 0.2, 0.0]
    }
  ]

=============
h5py Examples
=============

The fields available for a variable parameters dataset can be determined in h5py by examining the datatype of the dataset.

.. code-block:: python
   :linenos:

    import h5py
    with h5py.File("dakota_results.h5") as h:
        model = h["/models/simulation/NO_MODEL_ID/"]
        md = model["metadata/variable_parameters"]
        nu = md["normal_uncertain"]
        nu_param_names = nu.dtype.names 
        # nu_param_names is a tuple of strings: ('mean', 'std_deviation',
        # 'lower_bound', 'upper_bound')

=================   
Known Limitations
=================

h5py has a known bug that prevents parameters for some types of variables from being accessed (the Python interpreter crashes with a segfault). These include:

 - ``histogram_point_uncertain`` string
 - ``discrete_uncertain_set`` string

========
Metadata
========

The variable parameter datasets have two dimension scales. The first (index 0) contains the variable descriptors, and the second (index 1) contains variable Ids. Available Parameters

===============================
Parameter Listing for All Types
===============================

The table below lists all Dakota variables and parameters that can be stored.

**Distribution Parameters**

+--------------------------------+------------------------+-----------+----------+
| Variable Type                  | Parameter Name         | Type      | Rank     |
+================================+========================+===========+==========+
| continuous_design              | lower_bound            | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | upper_bound            | real      | scalar   |
+--------------------------------+------------------------+-----------+----------+
| discrete_design_range          | lower_bound            | integer   | scalar   |
|                                +------------------------+-----------+----------+
|                                | upper_bound            | integer   | scalar   |
+--------------------------------+------------------------+-----------+----------+
| discrete_design_set_int        | num_elements           | integer   | scalar   |
|                                +------------------------+-----------+----------+
|                                | elements               | integer   | vector   |
+--------------------------------+------------------------+-----------+----------+
| discrete_design_set_string     | num_elements           | integer   | scalar   |
|                                +------------------------+-----------+----------+
|                                | elements               | string    | vector   |
+--------------------------------+------------------------+-----------+----------+
| discrete_design_set_real       | num_elements           | integer   | scalar   |
|                                +------------------------+-----------+----------+
|                                | elements               | real      | vector   |
+--------------------------------+------------------------+-----------+----------+
| normal_uncertain               | mean                   | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | std_deviation          | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | lower_bound            | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | upper_bound            | real      | scalar   |
+--------------------------------+------------------------+-----------+----------+
| lognormal_uncertain            | lower_bound            | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | upper_bound            | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | mean                   | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | std_deviation          | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | error_factor           | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | lambda                 | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | zeta                   | real      | scalar   |
+--------------------------------+------------------------+-----------+----------+
| uniform_uncertain              | lower_bound            | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | upper_bound            | real      | scalar   |
+--------------------------------+------------------------+-----------+----------+
| loguniform_uncertain           | lower_bound            | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | upper_bound            | real      | scalar   |
+--------------------------------+------------------------+-----------+----------+
| triangular_uncertain           | mode                   | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | lower_bound            | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | upper_bound            | real      | scalar   |
+--------------------------------+------------------------+-----------+----------+
| exponential_uncertain          | beta                   | real      | scalar   |
+--------------------------------+------------------------+-----------+----------+
| beta_uncertain                 | alpha                  | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | beta                   | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | lower_bound            | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | upper_bound            | real      | scalar   |
+--------------------------------+------------------------+-----------+----------+
| gamma_uncertain                | alpha                  | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | beta                   | real      | scalar   |
+--------------------------------+------------------------+-----------+----------+
| gumbel_uncertain               | alpha                  | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | beta                   | real      | scalar   |
+--------------------------------+------------------------+-----------+----------+
| frechet_uncertain              | alpha                  | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | beta                   | real      | scalar   |
+--------------------------------+------------------------+-----------+----------+
| weibull_uncertain              | alpha                  | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | beta                   | real      | scalar   |
+--------------------------------+------------------------+-----------+----------+
| histogram_bin_uncertain        | num_elements           | integer   | scalar   |
|                                +------------------------+-----------+----------+
|                                | abscissas              | real      | vector   |
|                                +------------------------+-----------+----------+
|                                | counts                 | real      | vector   |
+--------------------------------+------------------------+-----------+----------+
| poisson_uncertain              | lambda                 | real      | scalar   |
+--------------------------------+------------------------+-----------+----------+
| binomial_uncertain             | probability_per_trial  | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | num_trials             | integer   | scalar   |
+--------------------------------+------------------------+-----------+----------+
| negative_binomial_uncertain    | probability_per_trial  | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | num_trials             | integer   | scalar   |
+--------------------------------+------------------------+-----------+----------+
| geometric_uncertain            | probability_per_trial  | real      | scalar   |
+--------------------------------+------------------------+-----------+----------+
| hypergeometric_uncertain       | total_population       | integer   | scalar   |
|                                +------------------------+-----------+----------+
|                                | selected_population    | integer   | scalar   |
|                                +------------------------+-----------+----------+
|                                | num_drawn              | integer   | scalar   |
+--------------------------------+------------------------+-----------+----------+
| histogram_point_uncertain_int  | num_elements           | integer   | scalar   |
|                                +------------------------+-----------+----------+
|                                | abscissas              | integer   | vector   |
|                                +------------------------+-----------+----------+
|                                | counts                 | real      | vector   |
+--------------------------------+------------------------+-----------+----------+
| histogram_point_uncertain_real | num_elements           | integer   | scalar   |
|                                +------------------------+-----------+----------+
|                                | abscissas              | real      | vector   |
|                                +------------------------+-----------+----------+
|                                | counts                 | real      | vector   |
+--------------------------------+------------------------+-----------+----------+
| continuous_interval_uncertain  | num_elements           | integer   | scalar   |
|                                +------------------------+-----------+----------+
|                                | interval_probabilities | real      | vector   |
|                                +------------------------+-----------+----------+
|                                | lower_bounds           | real      | vector   |
|                                +------------------------+-----------+----------+
|                                | upper_bounds           | real      | vector   |
+--------------------------------+------------------------+-----------+----------+
| discrete_interval_uncertain    | num_elements           | integer   | scalar   |
|                                +------------------------+-----------+----------+
|                                | interval_probabilities | real      | vector   |
|                                +------------------------+-----------+----------+
|                                | lower_bounds           | integer   | vector   |
|                                +------------------------+-----------+----------+
|                                | upper_bounds           | integer   | vector   |
+--------------------------------+------------------------+-----------+----------+
| discrete_uncertain_set_int     | num_elements           | integer   | scalar   |
|                                +------------------------+-----------+----------+
|                                | elements               | integer   | vector   |
|                                +------------------------+-----------+----------+
|                                | set_probabilities      | real      | vector   |
+--------------------------------+------------------------+-----------+----------+
| discrete_uncertain_set_real    | num_elements           | integer   | scalar   |
|                                +------------------------+-----------+----------+
|                                | elements               | real      | vector   |
|                                +------------------------+-----------+----------+
|                                | set_probabilities      | real      | vector   |
+--------------------------------+------------------------+-----------+----------+
| continuous_state               | lower_bound            | real      | scalar   |
|                                +------------------------+-----------+----------+
|                                | upper_bound            | real      | scalar   |
+--------------------------------+------------------------+-----------+----------+
| discrete_state_range           | lower_bound            | integer   | scalar   |
|                                +------------------------+-----------+----------+
|                                | upper_bound            | integer   | scalar   |
+--------------------------------+------------------------+-----------+----------+
| discrete_state_set_int         | num_elements           | integer   | scalar   |
|                                +------------------------+-----------+----------+
|                                | elements               | integer   | vector   |
+--------------------------------+------------------------+-----------+----------+
| discrete_state_set_string      | num_elements           | integer   | scalar   |
|                                +------------------------+-----------+----------+
|                                | elements               | string    | vector   |
+--------------------------------+------------------------+-----------+----------+
| discrete_state_set_real        | num_elements           | integer   | scalar   |
|                                +------------------------+-----------+----------+
|                                | elements               | real      | vector   |
+--------------------------------+------------------------+-----------+----------+

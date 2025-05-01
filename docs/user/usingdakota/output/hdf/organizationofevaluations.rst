.. _hdf-organization-of-evaluations:

"""""""""""""""""""""""""""
Organization of Evaluations
"""""""""""""""""""""""""""

An evaluation is a mapping from variables to responses performed by a Dakota model or interface. Dakota optionally writes
evaluation history to its HDF5 results file. The HDF5 format offers many advantages over existing console output and :ref:`tabular output<environment-tabular_data>`. Requring no "scraping", it is more convenient for most users than the former, and being unrestricted to a two-dimensional, tabular arragnment of information, it is far richer than the latter.

This section begins by describing the Dakota components that can generate evaluation data. It then documents the high-level organization of the data from those components. Detailed documentation of the individual datasets (the "low-level" organization) where data are stored follows. Finally, information is provided concerning input keywords that control which components report evaluations.

.. _hdf5_evaluations-hdf5_eval_sources:

==========================
Sources of Evaluation Data
==========================

Evaluation data are produced by only two kinds of components in Dakota: **models** and **interfaces**. The purpose of this subsection is to provide a basic description of models and interfaces for the purpose of equipping users to manage and understand HDF5-format evaluation data.

Because interfaces and models must be specified in even simple Dakota studies, most novice users of Dakota will have some familiarity with these concepts. However, the exact nature of the relationship between methods, models, and interfaces may be unclear. Moreover, the models and interfaces present in a Dakota study are not always limited to those specified by the user. Some input keywords or combinations of components cause Dakota to create new models or interfaces "behind the scenes" and without the user's direct knowledge. Not only can user-specified models and interfaces write evaluation data to HDF5, but also these auto-generated components. Accordingly, it may be helpful for consumers of Dakota's evaluation data to have a basic understanding of how Dakota creates and employs models and interfaces.

Consider first the input file shown here. 

.. code-block::

    environment
      tabular_data
      results_output
        hdf5

    method
      id_method 'sampling'
      sampling
        samples 20
      model_pointer 'sim'

    model
      id_model 'sim'
      single
      interface_pointer 'tb'

    variables
      uniform_uncertain 2
        descriptors 'x1' 'x2'
        lower_bounds 0.0 0.0
        upper_bounds 1.0 1.0

    responses
      response_functions 1
        descriptors 'f'
      no_gradients
      no_hessians

    interface
      id_interface 'tb'
      fork
        analysis_drivers 'text_book'
        
This simple input file specifies a single method of type :ref:`sampling<method-sampling>`, which also has the Id 'sampling'. The 'sampling' method possesses a :ref:`model<model>` of type :ref:`single<model-single>` (alias simulation) named 'sim', which it uses to perform evaluations. (Dakota would have automatically generated a single model had one not been specified.) That is to say, for each variables-to-response mapping required by the method, it provides variables to the model and receives back responses from it.

Single/simulation models like 'sim' perform evaluations by means of an interface, typically an interface to an external simulation. In this case, the interface is 'tb'. The model passes the variables to 'tb', which executes the text_book driver, and receives back responses.

It is clear that two components produce evaluation data in this study. The first is the single model 'sim', which receives and fulfills evaluation requests from the method 'sampling', and the second is the interface 'tb', which similarly receives requests from 'sim' and fulfills them by running the text_book driver.

Because :ref:`tabular data<environment-tabular_data>` was requested in the environment block, a record of the model's evaluations will be reported to a tabular file. The interface's evaluations could be dumped from the restart file using dakota_restart_util.

If we compared these evaluation histories from 'sim' and 'tb', we would see that they are identical to one another. The model 'sim' is a mere "middle man" whose only responsibility is passing variables from the method down to the interface, executing the interface, and passing responses back up to the method. However, this is not always the case.

For example, if this study were converted to a gradient-based optimzation using :ref:`optpp_q_newton<method-optpp_q_newton>`, and the user specified :ref:`numerical_gradients<responses-numerical_gradients>`: 

.. code-block::

    # model and interface same as above. Replace the method, variables, and responses with:

    method
      id_method 'opt'
      optpp_q_newton

    variables
      continuous_design 2
        descriptors 'x1' 'x2'
        lower_bounds 0.0 0.0
        upper_bounds 1.0 1.0

    responses
       objective_functions 1
        descriptors 'f'
      numerical_gradients
      no_hessians
	  
Then the model would have the responsibility of performing finite differencing to estimate gradients of the response 'f' requested by the method. Multiple function evaluations of 'tb' would map to a single gradient evaluation at the model level, and the evaluation histories of 'sim' and 'tb' would contain different information.

Note that because it is unwieldy to report gradients (or Hessians) in a tabular format, they are not written to the tabular file, and historically were avialable only in the console output. The HDF5 format provides convenient access to both the "raw" evaluations performed by the interface and higher level model evaluations that include estimated gradients.

This pair of examples hopefully provides a basic understanding of the flow of evaluation data between a method, model, and interface, and explains why models and interfaces are producers of evaluation data.

Next consider a somewhat more complex study that includes a Dakota model of type :ref:`surrogate<model-surrogate>`. A surrogate model performs evaluations requested by a method by executing a special kind of interface called an approximation interface, which Dakota implicitly creates without the direct knowledge of the user. Approximation interfaces are a generic container for the various kinds of surrogates Dakota can use, such as :ref:`gaussian processes<model-surrogate-global-gaussian_process>`.

A Dakota model of type global surrogate may use a user-specified :ref:`dace method<model-surrogate-global-dace_method_pointer>` to construct the actual underlying model(s) that it evaluates via its approximation interface. The dace method will have its own model (typically of type single/simulation), which will have a user-specified interface.

In this more complicated case there are at least four components that produce evaluation data: (1) the surrogate model and (2) its approximation interface, and (3) the dace method's model and (4) its interface. Although only components (1), (3), and (4) are user-specified, evaluation data produced by (2) may be written to HDF5, as well. (:ref:`As explained below<hdf5_evaluations-selection>`, only evaluations performed by the surrogate model and the dace interface will be recorded by default. This can be overriden using :ref:`hdf5<environment-results_output-hdf5>` sub-keywords.) This is an example where "extra" and potentially confusing data appears in Dakota's output due to an auto-generated component.

An important family of implicitly-created models is the recast models, which have the responsibility of transforming variables and responses. One type of recast called a data transform model is responsible for computing residuals when a user provides :ref:`experimental data<responses-calibration_terms-calibration_data>` in a calibration study. Scaling recast models are employed when scaling is requested by the user for variables and/or responses.

Recast models work on the principle of function composition, and "wrap" a submodel, which may itself also be a recast model. The innermost model in the recursion often will be the simulation or surrogate model specified by the user in the input file. Dakota is capable of recording evaluation data at each level of recast.

.. _hdf5_evaluations-high_level:

==========================================
High-level Organization of Evaluation Data
==========================================

This subsection describes how evaluation data produced by models and interfaces are organized at high level. A detailed description of the datasets and subgroups that contain evaluation data for a specific model or interface is given in the :ref:`next subsection<hdf5_evaluations-low_level>`.

Two top level groups contain evaluation data, ``/interfaces`` and ``/models``.

**Interfaces**

Because interfaces can be executed by more than one model, interface evaluations are more precisely thought of as evaluations of an interface/model combination. Consequently, interface evaluations are grouped not only by interface Id ('tb' in the example above), but also the Id of the model that requested them ('sim').

.. code-block::

	/interfaces/<interface Id>/<model Id>/

If the user does not provide an Id for an interface that he specifies, Dakota assigns it the Id NO_ID. Approximation interfaces receive the Id ``APPROX_INTERFACE_<N>``, where N is an incrementing integer beginning at 1. Other kinds of automatically generated interfaces are named ``NOSPEC_INTERFACE_ID_<N>``.

**Models**

The top-level group for model evaluations is /models. Within this group, model evaluations are grouped by type: ``simulation``, ``surrogate``, ``nested``, or ``recast``, and then by model Id. That is:

.. code-block::

	/models/<type>/<model Id>/    

Similar to interfaces, user-specified models that lack an Id are given one by Dakota. A single model is named ``NO_MODEL_ID``. Some automatically generated models receive the name ``NOSPEC_MODEL_ID``.

Recast models are a special case and receive the name ``RECAST_<WRAPPED-MODEL>_<TYPE>_<N>``. In this string:

 - WRAPPED-MODEL is the Id of the innermost wrapped model, typically a user-specified model
 - TYPE is the specific kind of recast. The three most common recasts are:

   - RECAST: several generic responsibilities, including summing objective functions to present to a single-objective optimizer
   - DATA_TRANSFORM: Compute residuals in a calibration
   - SCALING: scale variables and responses

 - N is an incrementing integer that begins with 1. It is employed to distinguish recasts of the same type that wrap the same underlying model.

The model's evaluations may be the result of combining information from multiple sources. A simulation/single model will receive all the information it requires from its interface, but more complicated model types may use information not only from interfaces, but also other models and the results of method executions. Nested models, for instance, receive information from a submethod (the mean of a response from a sampling study, for instance) and potentially also an :ref:`optional interface<model-nested-optional_interface_pointer>`.

The sources of a model's evaluations may be roughly identified by examining the contents of that models' ``sources`` group. The ``sources`` group contains softlinks (note: softlinks are an HDF5 feature analogous to soft or symbolic links on many file systems) to groups for the interfaces, models, or methods that the model used to produce its evaluation data. (At this time, Dakota does not report the specific interface or model evaluations or method executions that were used to produce a specific model evaluation, but this is a planned feature.)

Method results likewise have a ``sources`` group that identifies the models or methods employed by that method. By following the softlinks contained in a method's or model's sources group, it is possible to "drill down" from a method to its ultimate sources of information. In the sampling example above, interface evaluations performed via the 'sim' model at the request of the 'sampling' method could be obtained at the HDF5 path: ``/methods/sampling/sources/sim/sources/tb/``

.. _hdf5_evaluations-low_level:

=========================================
Low-Level Organization of Evaluation Data
=========================================

Within each model and interface's "high-level" group, evaluation data are stored according to a "low-level" schema. This section desribes the "low-level" schema.

Data are divided first of all into variables, responses, and properties groups. In addition, if a a user specifies metadata responses in his Dakota input, a metadata dataset will be present.

**Variables**

The variables group contains datasets that store the variables information for each evaluation. Four datasets may be present, one for each "domain": ``continuous``, ``discrete_integer``, ``discrete_string``, and ``discrete_real``. These datasets are two-dimensional, with a row (0th dimension) for each evaluation and a column (1st dimension) for each variable. The 0th dimension has one dimension scale for the integer-valued evaluation Id. The 1st dimension has two scales. The 0th scale contains descriptors of the variables, and the 1st contains their variable Ids. In this context, the Ids are a 1-to-N ranking of the variables in Dakota "input spec" order.

+------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------+
|                              | Variables                                                                                                                                                   |
+==============================+=============================================================================================================================================================+
| Description                  | Values of variables in evaluations                                                                                                                          |
+------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Location                     | variables/{continuous, discrete_integer, discrete_string, discrete_real}                                                                                    |
+------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Shape                        | 2-dimensional: number of evaluations by number of variables                                                                                                 |
+------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Type                         | Real, String, or Integer, as applicable                                                                                                                     |
+------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Scales                       | +-----------+---------+----------------+----------------------+------------------------------------------------------------------------+------------------+ |
|                              | | Dimension | Type    | Label          | Contents             | Notes                                                                  | Literal_contents | |
|                              | +===========+=========+================+======================+========================================================================+==================+ |
|                              | | 0         | Integer | evaluation_ids | Evaluation Ids       | false                                                                  |                  | |
|                              | +-----------+---------+----------------+----------------------+------------------------------------------------------------------------+------------------+ |
|                              | | 1         | String  | \*_descriptors | Variable descriptors | false                                                                  |                  | |
|                              | +-----------+---------+----------------+----------------------+------------------------------------------------------------------------+------------------+ |
|                              | | 1         | Integer | \*_ids         | Variable Ids         | 1-to-N rank of the variable in Dakota input spec order                 | false            | |
|                              | +-----------+---------+----------------+----------------------+------------------------------------------------------------------------+------------------+ |
|                              | | 1         | String  | \*_type        | Variable types       | Type of each variable, e.g. CONTINUOUS_DESIGN, DISCRETE_DESIGN_SET_INT | false            | |
|                              | +-----------+---------+----------------+----------------------+------------------------------------------------------------------------+------------------+ |
+------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------+

**Responses**

The responses group contains datasets for functions and, when available, gradients and Hessians.

*Functions:* The ``functions`` dataset is two-dimensional and contains function values for all responses. Like the variables datasets, evaluations are stored along the 0th dimension, and responses are stored along the 1st. The evaluation Ids and response descriptors are attached as scales to these axes, respectively.

+------------------------------+------------------------------------------------------------------------------------+
|                              | Variables                                                                          |
+==============================+====================================================================================+
| Description                  | Values of functions in evaluations                                                 |
+------------------------------+------------------------------------------------------------------------------------+
| Location                     | responses/functions                                                                |
+------------------------------+------------------------------------------------------------------------------------+
| Shape                        | 2-dimensional: number of evaluations by number of responses                        |
+------------------------------+------------------------------------------------------------------------------------+
| Type                         | Real                                                                               |
+------------------------------+------------------------------------------------------------------------------------+
| Scales                       | +-----------+---------+----------------+----------------------+------------------+ |
|                              | | Dimension | Type    | Label          | Contents             | Literal_contents | |
|                              | +===========+=========+================+======================+==================+ |
|                              | | 0         | Integer | evaluation_ids | Evaluation Ids       | false            | |
|                              | +-----------+---------+----------------+----------------------+------------------+ |
|                              | | 1         | String  | responses      | Response descriptors | false            | |
|                              | +-----------+---------+----------------+----------------------+------------------+ |
+------------------------------+------------------------------------------------------------------------------------+

*Gradients:* The gradients dataset is three-dimensional. It has the shape :math:`evaluations \times responses \times variables`. Dakota supports a specification of :ref:`mixed_gradients<responses-mixed_gradients>`, and the gradients dataset is sized and organized such that only those responses for which gradients are available are stored. When ``mixed_gradients`` are employed, a response will not necessarily have the same index in the functions and gradients datasets.

Because it is possible that the gradient could be computed with respect to any of the continuous variables, active or inactive, that belong to the associated model, the ``gradients`` dataset is sized to accomodate gradients taken with respect to all continuous variables. Components that were not included in a particular evaluation will be set to NaN (not a number), and the ``derivative_variables_vector`` (in the matadata group) for that evaluation can be examined as well.

+------------------------------+------------------------------------------------------------------------------------+
|                              | Gradients                                                                          |
+==============================+====================================================================================+
| Description                  | Values of gradients in evaluations                                                 |
+------------------------------+------------------------------------------------------------------------------------+
| Location                     | responses/gradients                                                                |
+------------------------------+------------------------------------------------------------------------------------+
| Shape                        | 3-dimensional: number of evaluations by number of responses by number of variables |
+------------------------------+------------------------------------------------------------------------------------+
| Type                         | Real                                                                               |
+------------------------------+------------------------------------------------------------------------------------+
| Scales                       | +-----------+---------+----------------+----------------------+------------------+ |
|                              | | Dimension | Type    | Label          | Contents             | Literal_contents | |
|                              | +===========+=========+================+======================+==================+ |
|                              | | 0         | Integer | evaluation_ids | Evaluation Ids       | false            | |
|                              | +-----------+---------+----------------+----------------------+------------------+ |
|                              | | 1         | String  | responses      | Response descriptors | false            | |
|                              | +-----------+---------+----------------+----------------------+------------------+ |
+------------------------------+------------------------------------------------------------------------------------+

*Hessians:* Hessians are stored in a four-dimensional dataset, :math:`evaluations \times responses \times \times variables \times variables`. The ``hessians`` dataset shares many of the characteristics with the ``gradients``: in the mixed_hessians case, it will be smaller in the response dimension than the ``functions`` dataset, and unrequested components are set to NaN.

+------------------------------+------------------------------------------------------------------------------------+
|                              | Hessians                                                                           |
+==============================+====================================================================================+
| Description                  | Values of Hessians in evaluations                                                  |
+------------------------------+------------------------------------------------------------------------------------+
| Location                     | responses/hessians                                                                 |
+------------------------------+------------------------------------------------------------------------------------+
| Shape                        | 4-dimensional: number of evaluations by number of responses by number of variables |
|                              | by number of variables                                                             |
+------------------------------+------------------------------------------------------------------------------------+
| Type                         | Real                                                                               |
+------------------------------+------------------------------------------------------------------------------------+
| Scales                       | +-----------+---------+----------------+----------------------+------------------+ |
|                              | | Dimension | Type    | Label          | Contents             | Literal_contents | |
|                              | +===========+=========+================+======================+==================+ |
|                              | | 0         | Integer | evaluation_ids | Evaluation Ids       | false            | |
|                              | +-----------+---------+----------------+----------------------+------------------+ |
|                              | | 1         | String  | responses      | Response descriptors | false            | |
|                              | +-----------+---------+----------------+----------------------+------------------+ |
+------------------------------+------------------------------------------------------------------------------------+

**Properties**

The properties group contains up to four members.

*Active Set Vector:* The first is the ``active_set_vector`` dataset. It is two dimensional, with rows corresponding to evaluations and columns corresponding to responses. Each element contains an integer in the range 0-7, which indicates the request (function, gradient, Hessian) for the corresponding response for that evaluation. The 0th dimension has the evaluations Ids scale, and the 1st dimension has two scales: the response descriptors and the "default" or "maximal" ASV, an integer 0-7 for each response that indicates the information (function, gradient, Hessian) that possibly could have been requested during the study.

+------------------------------+------------------------------------------------------------------------------------+
|                              | Active Set Vector                                                                  |
+==============================+====================================================================================+
| Description                  | Values of the active set vector in evaluations                                     |
+------------------------------+------------------------------------------------------------------------------------+
| Location                     | metadata/active_set_vector                                                         |
+------------------------------+------------------------------------------------------------------------------------+
| Shape                        | 2-dimensional: number of evaluations by number of responses                        |
+------------------------------+------------------------------------------------------------------------------------+
| Type                         | Integer                                                                            |
+------------------------------+------------------------------------------------------------------------------------+
| Scales                       | +-----------+---------+----------------+----------------------+------------------+ |
|                              | | Dimension | Type    | Label          | Contents             | Literal_contents | |
|                              | +===========+=========+================+======================+==================+ |
|                              | | 0         | Integer | evaluation_ids | Evaluation Ids       | false            | |
|                              | +-----------+---------+----------------+----------------------+------------------+ |
|                              | | 1         | String  | responses      | Response descriptors | false            | |
|                              | +-----------+---------+----------------+----------------------+------------------+ |
+------------------------------+------------------------------------------------------------------------------------+

*Derivative Variables Vector:* The second item in the properties group is the ``derivative_variables_vector`` dataset. It is included only when gradients or Hessians are available. Like the ASV, it is two-dimensional. Each column of the DVV dataset corresponds to a continuous variable and contains a 0 or 1, indicating whether gradients and Hessians were computed with respect to that variaable for the evaluation. The 0th dimension has the evaluation Ids as a scale, and the 1st dimension has two scales. The 0th is the descriptors of the continuous variables. The 1st contains the variable Ids of the continuous variables.

+------------------------------+------------------------------------------------------------------------------------+
|                              | Derivative Variables Vector                                                        |
+==============================+====================================================================================+
| Description                  | Values of the derivative variables vector in evaluations                           |
+------------------------------+------------------------------------------------------------------------------------+
| Location                     | metadata/derivative_variables_vector                                               |
+------------------------------+------------------------------------------------------------------------------------+
| Shape                        | 2-dimensional: number of evaluations by number of continuous variables             |
+------------------------------+------------------------------------------------------------------------------------+
| Type                         | Integer                                                                            |
+------------------------------+------------------------------------------------------------------------------------+
| Scales                       | +-----------+---------+----------------+----------------------+------------------+ |
|                              | | Dimension | Type    | Label          | Contents             | Literal_contents | |
|                              | +===========+=========+================+======================+==================+ |
|                              | | 0         | Integer | evaluation_ids | Evaluation Ids       | false            | |
|                              | +-----------+---------+----------------+----------------------+------------------+ |
|                              | | 1         | String  | variables      | Variable descriptors | false            | |
|                              | +-----------+---------+----------------+----------------------+------------------+ |
+------------------------------+------------------------------------------------------------------------------------+

*Analysis Components:* The third member of the properties group is the ``analysis_components`` dataset. It is a 1D dataset that is present only when the user specified analysis components, and it contains those components as strings.

+------------------------------+------------------------------------------------------------+
|                              | Analysis Components                                        |
+==============================+============================================================+
| Description                  | Values of the analysis components in evaluations           |
+------------------------------+------------------------------------------------------------+
| Location                     | metadata/analysis_components                               |
+------------------------------+------------------------------------------------------------+
| Shape                        | 1-dimensional: number of analysis components               |
+------------------------------+------------------------------------------------------------+
| Type                         | String                                                     |
+------------------------------+------------------------------------------------------------+

The final possible member of the properties group is the ``variable_parameters`` group. It is included only for models, which possess variables, and is described in a separate section below.

**Metadata**

Beginning with release 6.16, Dakota supports response :ref:`metadata<responses-metadata>`. If configured, metadata values are stored in the ``metadata`` dataset.

TODO

.. _hdf5_evaluations-selection:

========================================
Selecting Models and Interfaces to Store
========================================

When HDF5 output is enabled (by including the :ref:`hdf5<environment-results_output-hdf5>` keyword), then by default evaluation data for the following components will be stored:

 - The model that belongs to the top-level method. (Currently, if the top-level method is a metaiterator such as method-hybrid, no model evaluation data will be stored.)
 - All simulation interfaces. (interfaces of type :ref:`fork<interface-analysis_drivers-fork>`, :ref:`system<interface-analysis_drivers-system>`, :ref:`direct<interface-analysis_drivers-direct>`, etc).

The user can override these defaults using the keywords :ref:`model_selection<environment-results_output-hdf5-model_selection>` and :ref:`interface_selection<environment-results_output-hdf5-interface_selection>`.

The choices for ``model_selection`` are:

 - :ref:`top_method<environment-results_output-hdf5-model_selection-top_method>` : (default) Store evaluation data for the top method's model only.
 - :ref:`all_methods<environment-results_output-hdf5-model_selection-all_methods>` : Store evaluation data for all models that belong directly to a method. Note that a these models may be recasts of user-specified models, not the user-specified models themselves.
 - :ref:`all<environment-results_output-hdf5-model_selection-all>` : Store evaluation data for all models.
 - :ref:`none<environment-results_output-hdf5-model_selection-none>` : Store evaluation data for no models.

The choices for interface_selection are:

 - :ref:`simulation<environment-results_output-hdf5-interface_selection-simulation>` : (default) Store evaluation data for simulation interfaces.
 - :ref:`all<environment-results_output-hdf5-interface_selection-all>` : Store evaluation data for all interfaces.
 - :ref:`none<environment-results_output-hdf5-interface_selection-none>` : Store evaluation data for no interfaces.

If a model or interface is excluded from storage by these selections, then they cannot appear in the sources group for methods or models.

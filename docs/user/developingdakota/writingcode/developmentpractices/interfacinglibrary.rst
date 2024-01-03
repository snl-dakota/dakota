""""""""""""""""""""""""""""""""""""
Interfacing with Dakota as a Library
""""""""""""""""""""""""""""""""""""

.. _`interfacing_with_dakota_as_library`:

============
Introduction
============

Tightly integrating or linking Dakota into another application can improve user experience by delivering a more unified, inter-operable software tool for optimization and UQ analyses, improving performance by eliminating file system-based interfaces, and reducing challenges with parallel computing inter-operation. This benefit has been realized within several Sandia and external simulation applications. This section describes how to link Dakota into another C++ application.

Dakota has two primary application programming interfaces (APIs). The LibraryEnvironment class facilitates use of Dakota as an algorithm service library within another application. In this case, the simulation application is providing a "front end" for Dakota. The second API, provided by the DirectApplicInterface class, provides an interface for Dakota to call the simulation code directly to perform function evaluations in core. This permits the simulation to be the "back end" for Dakota. The most complete library integration of Dakota would use both in combination, with the overall simulation framework providing both the front end and back end for Dakota, creating a sandwich, as loosely depicted here:

.. code-block::

	[------------
	[ Application 
	[
	[  ( -----
	[  ( Dakota (LibraryEnvironment)
	[  (
	[  (  { Function evaluation callback to Application (via DirectApplicInterface)
	[  (  {  | 
	[ <------/
	[  (  {  
	[  (    
	[  ( -----
	[
	[------------

**Attention:** Dakota may be integrated as a library in other software applications subject to the terms of the GNU Lesser General Public License (LGPL). Refer to http://www.gnu.org/licenses/lgpl.html or the LICENSE file included with Dakota.

When Dakota is compiled and installed, the relevant library API headers are installed to CMAKE_INSTALL_PREFIX/include and the runtime libraries primarily to CMAKE_INSTALL_PREFIX/lib/ (on some platforms, to CMAKE_INSTALL_PREFIX/bin/. The core C/C++ code is in the library dakota_src, while Fortran code lives in the dakota_src_fortran library. Information on using the API in Dakota headers is included throughout this section, while considerations for configuring and linking against Dakota and its various required and optional third–party libraries are emphasized in the section Linking against the Dakota library.

Steps involved in integrating Dakota into another application typically include:

1. Writing C++ code for your application to instantiate, configure, and execute Dakota's LibraryEnvironment ("front end"); see Basic Dakota library instantiation and Configuring Dakota operation.

2. Writing C++ code for Dakota to call a function in your application to perform function evaluations ("back end"); see Creating a simulator plugin interface.

3. Compiling Dakota and linking into your application (Linking against the Dakota library).

Several source code examples demonstrate Dakota library interfaces. The classes SIM::SerialDirectApplicInterface and SIM::ParallelDirectApplicInterface demonstrate serial and parallel simulation function evaluation plug-ins. The file library_mode.cpp includes a main program that exercises Dakota libraries in serial and parallel modes with these mock simulator programs, with various ways of configuring Dakota problem definition and operation. Finally, library_split.cpp demonstrates running Dakota as a library modular on an MPI sub-communicator.

==================================
Basic Dakota library instantiation
==================================

The function run_dakota_parse() in library_mode.cpp demonstrates the basic use of Dakota library objects as one would in another main application that embeds Dakota. In this example, Dakota is configured based on a typical user-provided text-based Dakota input file (the same that would be provided at the command line with dakota -i dakota_optimization.in) and a function evaluator derived from a DirectApplicInterface is plugged into the Dakota library environment.

First, an object of type ProgramOptions which manages top-level Dakota settings is instantiated and configured to specify the name of the Dakota user input file. Additional options for output and error redirection, restart operation, and more may be set via ProgramOptions. See its class documentation for details.

.. code-block:: cpp

	string dakota_input_file = "dakota_optimization.in";
	Dakota::ProgramOptions opts;
	opts.input_file(dakota_input_file);

Next, a LibraryEnvironment is created, passing the desired settings from opts:

.. code-block:: cpp

	Dakota::LibraryEnvironment env(opts);

This standard constructor will parse the specified input and create Dakota objects. It assumes many default settings, including that the parent application initialized MPI if running in parallel mode. (In this case, Dakota will detect whether MPI was initialized and not call MPI_Init or MPI_Finalize.) For more advanced use cases described below, alternate constructors allow constructing based on MPI communicators, with delayed finalization, and with Dakota database update function callbacks. Then the application's function evaluator implementing Dakota's DirectApplicInterface is plugged in with a convenience function serial_interface_plugin() or parallel_interface_plugin(). Finally, the Dakota analysis is run by calling

.. code-block:: cpp

	env.execute(); 

The next two sections offer additional details on (1) alternative and supplementary ways to configure Dakota's operation (Configuring Dakota operation) and (2) how to specialize Dakota's DirectApplicInterface to provide a function evaluator plugin to Dakota (Creating a simulator plugin interface).

**Remarks**

After LibraryEnvironment construction, all MPI communicator partitioning has been performed and the ParallelLibrary instance may be interrogated for parallel configuration data. For example, the lowest level communicators in Dakota's multilevel parallel partitioning are the analysis communicators, which can be retrieved using:

.. code-block:: cpp
	
	// retrieve the set of analysis communicators for simulation initialization:
    // one analysis comm per ParallelConfiguration (PC), one PC per Model.
	Array<MPI_Comm> analysis_comms = parallel_lib.analysis_intra_communicators();

These communicators can then be used for initializing parallel simulation instances when registering the plugin interface, where the number of MPI communicators in the array corresponds to one communicator per ParallelConfiguration instance. This is demonstrated below in Derivation.

============================
Configuring Dakota operation
============================

This section describes several alternate ways to initially set and later manipulate Dakota's configuration, including alternatives to using a text-based input file. The algorithm configuration for a particular Dakota analysis run is managed in its ProblemDescDB, which can be populated via an input file, string literal, or C++ API, and later modified through Dakota's C++ API. All Dakota objects then draw information from this database upon instantiation.

Input data parsing
------------------

The simplest way for an application to configure a Dakota analysis problem is to use Dakota's normal input parsing system to populate its problem database (ProblemDescDB). This is done by providing standard Dakota input file syntax through the library interface, via either a file name or string literal. An advantage is that native Dakota syntax can be used, but disadvantages include the requirement for an additional input file beyond those already required by the parent application and that application users also need to know Dakota syntax.

The two ways to configure Dakota via input parsing are shown near the beginning of run_dakota_mixed() in library_mode.cpp. Here the ProgramOptions are set to either parse from a named file:

.. code-block:: cpp

	Dakota::ProgramOptions opts;
	opts.input_file(dakota_input_file);

or from a string literal provided by the wrapping application:

.. code-block:: cpp

	string serial_input = "% Dakota input file ...";
	opts.input_string(serial_input);

This library approach is coarse-grained in that input is parsed, objects constructed, and the environment is immediately ready to run. The next approaches are more modular.

Problem database insertion
--------------------------

A second approach to configuring Dakota's operation is to bypass parsing phases and directly populate the ProblemDescDB with information on the methods, variables, interface, responses, etc., that define the Dakota analysis problem. This approach requires more interaction with Dakota classes and data structures. However, it can offer usability benefit when the integrating application does not want their users to interact with the full Dakota syntax, or efficiency benefit when for example there are a large number of variables to configure.

In the direct database population approach, Dakota DataMethod, DataModel, DataVariables, DataInterface, and DataResponses objects are instantiated and populated with the desired problem data. These objects are then published to the problem database using insert_nodes() . An example of this approach is available in run_dakota_data() in library_mode.cpp, where the OPT++ Quasi-Newton method is configured to work on a plugin version of text_book or rosenbrock. The data objects are populated with their default values upon instantiation and are often sufficient for basic Dakota studies. Only the non-default values need to be specified. Moreover the default Dakota Model is a SingleModel, so this object need not be configured unless tailoring its configuration or using a more advanced model type. Refer to the DataMethod, DataModel, DataVariables, DataInterface, and DataResponses class documentation and source code for lists of attributes and their defaults. Here is an excerpt of run_dakota_data() that specifies the OPT++ solver after default construction of DataMethod:

.. code-block:: cpp

	Dakota::DataMethod   dme;
	Dakota::DataMethodRep* dmr = dme.data_rep();
	dmr->methodName = Dakota::OPTPP_Q_NEWTON;

When using direct database population, it is critical to leave the database in an open, accessible state after initial construction. In this run_dakota_data() example, a flag check_bcast_construct is passed into the LibraryEnvironment constructor, indicating that it should not finalize the database and construct Dakota objects. Moreover, it is only necessary to populate the database on rank 0 of the MPI Comm on which Dakota is running. After database objects are inserted or adjusted, the LibraryEnvironment::done_modifying_db() function must be called before proceeding to execute. This synchronizes problem data across all ranks and constructs Dakota objects needed to run the specified analysis.

.. code-block:: cpp

	bool check_bcast_construct = false;
	Dakota::LibraryEnvironment env(MPI_COMM_WORLD, opts, check_bcast_construct);
	if (rank == 0)
	  // insert/modify DB, then lock and proceed:
	env.done_modifying_db();
	env.execute();

Mixed mode, callbacks, and late updates
---------------------------------------

The LibraryEnvironment API also supports mixed approaches that combine the parsing of a Dakota input file (or input string literal) with direct database updates. This approach is motivated by large-scale applications where large vectors are cumbersome to specify in a Dakota input file or where later updates to an input template are needed. The example run_dakota_mixed() in library_mode.cpp demonstrates the combination of these more advanced approaches: (1) input text parsing, (2) database updates via a callback, (3) database updates via direct manipulation, and (4) further runtime updates to the Model before running.

First, a ProgramOptions class is instantiated and configured to parse either an input file or input string literal (as in earlier examples). The passed input data must contain all required inputs so the parser can validate them. Since vector data like variable values/bounds/tags, linear/nonlinear constraint coefficients/bounds, etc., are optional, these potentially large vector specifications can be omitted from the input file and updated later through the database API. Only the variable/response counts necessary for sizing, e.g.:

.. code-block::

	method
	  linear_inequality_constraints = 500

	variables
	  continuous_design = 1000

	responses
	  objective_functions = 1
	  nonlinear_inequality_constraints = 100000

and not the lists of values are required in this case. To update or add data after this initial parse, we use the ProblemDescDB::set() family of overloaded functions, e.g.

.. code-block:: cpp

	Dakota::RealVector drv(1000, 1.); // vector of length 1000, values initialized to 1.
	problem_db.set("variables.continuous_design.initial_point", drv);

where the string identifiers are the same identifiers used when pulling information from the database using one of the get_<datatype>() functions (refer to ProblemDescDB for a full list). However, the supported ProblemDescDB::set() options are a restricted subset of the database attributes, focused on vector inputs that can be large scale.

Second, the example demonstrates a user-provided callback function which Dakota will invoke after input parsing to update ProblemDescDB. In library_mode.cpp, callback_function() is a user-provided post-parse callback that implements the type Dakota::DbCallbackFunction.

.. code-block:: cpp

	static void callback_function(Dakota::ProblemDescDB* db, void *ptr);

When Dakota calls this function it will pass back pointers to the ProblemDescDB instance and to user-provided data, so the application may convey its settings by calling methods on the ProblemDescDB, optionally using the provided data. An example of a user data structure is demonstrated in callback_data. In this case, when the LibraryEnvironment is constructed, it is constructed with the input data to initially parse, the callback function, and to leave it unlocked for further updates:

.. code-block:: cpp

	bool done_with_db = false;
	Dakota::LibraryEnvironment env(opts, done_with_db, callback_function, &data);

Third, the example demonstrates changes to the database after parsing and callback-based updates. Again, these only need happen on Dakota's rank 0 before finalizing the DB with LibraryEnvironment::done_modifying_db(). The example demonstrates:

1. Getting access to the database through env.problem_description_db()
2. Setting the database nodes to the appropriate method through problem_db.resolve_top_method()
3. Getting data from the DB with a get string array function: problem_db.get_sa("interface.application.analysis_drivers")
4. Setting update data with problem_db.set("variables.continuous_design.initial_point", ip);

After any of these three types updates, calling LibraryEnvironment::done_modifying_db() will broadcast any updates (including potentially large vector data and post-process specification data to fill in any vector defaults that have not yet been provided through either file parsing or direct updates. (Note: scalar defaults are handled in the Data class constructors.)

Fourth and finally, run_dakota_mixed() demonstrates modifying a Model's data after database operations and interface plugin are complete. This involves finding the right Model (or other class) instance to modify, and directly adjusting its data through the public API. Since the database is finalized, any updates must be performed through direct set operations on the constructed objects. For example, to update other data such as variable values/bounds/tags or response bounds/targets/tags, refer to the set functions documented in Iterator and Model. As an example, the following code updates the active continuous variable values, which will be employed as the initial guess for certain classes of Iterators:

.. code-block:: cpp

	ModelList& all_models  = problem_db.model_list();
	Model&     first_model = *all_models.begin();
	Dakota::RealVector drv(1000, 1.); // vector of length 1000, values initialized to 1.
	first_model.continuous_variables(drv);

**Remarks**

If performing such data updates within the constructor of a DirectApplicInterface extension/derivation (see Creating a simulator plugin interface), then this code is sufficient since the database is unlocked, the active list nodes of the ProblemDescDB have been set for you, and the correct method/model/variables/interface/responses specification instance will get updated. The difficulty in this case stems from the order of instantiation. Since the Variables and Response instances are constructed in the base Model class, prior to construction of Interface instances in derived Model classes, database information related to Variables and Response objects will have already been extracted by the time the Interface constructor is invoked and the database update will not propagate.

Therefore, it is preferred to perform these database set operations at a higher level (e.g., within your main program), prior to allowing Environment to broadcast, construct, and execute, such that instantiation order is not an issue. However, in this case, it is necessary to explicitly manage the list nodes of the ProblemDescDB using a specification instance identifier that corresponds to an identifier from the input file, e.g.:

.. code-block:: cpp

	problem_db.set_db_variables_node("MY_VARIABLES_ID");
	Dakota::RealVector drv(1000, 1.); // vector of length 1000, values initialized to 1.
	problem_db.set("variables.continuous_design.initial_point", drv);

Alternatively, rather than setting just a single data node, all data nodes may be set using a method specification identifier:

.. code-block:: cpp

	problem_db.set_db_list_nodes("MY_METHOD_ID"); 

since the method specification is responsible for identifying a model specification, which in turn identifies variables, interface, and responses specifications. If hard-wiring specification identifiers is undesirable, then

.. code-block:: cpp

	problem_db.resolve_top_method(); 

can also be used to deduce the active method specification and set all list nodes based on it. This is most appropriate in the case where only single specifications exist for method/model/variables/interface/responses. This is the approach demonstrated in run_dakota_mixed(). In each of these cases, setting list nodes unlocks the corresponding portions of the database, allowing set/get operations.

=====================================
Creating a simulator plugin interface
=====================================

The DirectApplicInterface class provides an interface for Dakota to call the simulation code directly to perform function evaluations mapping variables to responses. This provides the "back end" for Dakota to call back to the simulation framework. Two approaches to defining this direct interface are described here. The first is less common, while the second is recommended when possible.

Extension
---------

The first approach involves extending one of the existing DirectApplicInterface subclasses (TestDriverInterface, MatlabInterface, etc.) to support additional direct simulation interfaces. For example, Dakota algebraic test problems are implemented in TestDriverInterface. One could add additional direct functions to Dakota in TestDriverInterface::derived_map_ac(). In addition, TestDriverInterface::derived_map_if() and TestDriverInterface::derived_map_of() can be extended to perform pre- and post-processing tasks if desired, but this is not required.

While this approach is the simplest, it has the disadvantage that the Dakota library will need to be recompiled when the simulation or its direct interface is modified. If it is desirable to maintain the independence of the Dakota library from the host application, then the derivation approach described in the next section should be employed.

**Remarks**

If the new direct evaluation function implementation will not be a member function of one of the Dakota classes, then the following prototype should be used in order to pass the required data:

.. code-block:: cpp

    int sim(const Dakota::Variables& vars, const Dakota::ActiveSet& set,
    Dakota::Response& response); 
	
If the new function will be a member function, e.g., in TestDriverInterface, then this can be simplified to

.. code-block:: cpp

    int sim();

since the data access can be performed through the DirectApplicInterface class attributes.

Derivation
----------

The second approach is to derive a new interface from DirectApplicInterface and redefine several virtual functions. As demonstrated in SIM::SerialDirectApplicInterface and SIM::ParallelDirectApplicInterface, a typical derived class declaration might be

.. code-block:: cpp

	namespace SIM {
	class SerialDirectApplicInterface: public Dakota::DirectApplicInterface
	{
	public:
	  // Constructor and destructor
	  SerialDirectApplicInterface(const Dakota::ProblemDescDB& problem_db);
	  ~SerialDirectApplicInterface();
	protected:
	  // Virtual function redefinitions
	  int derived_map_if(const Dakota::String& if_name);
	  int derived_map_ac(const Dakota::String& ac_name);
	  int derived_map_of(const Dakota::String& of_name);
	private:
	  // Data
	}
	} // namespace SIM

where the new derived class resides in the simulation's namespace. Similar to the case of Extension, the DirectApplicInterface::derived_map_ac() function is the required redefinition, and DirectApplicInterface::derived_map_if() and DirectApplicInterface::derived_map_of() are optional.

Typically the new derived_map_ac() implementation delegates to the main simulation application for a function evaluation. Here Dakota variables would get mapped into the simulation's data structures, the simulation executed, and derived response data computed for return to Dakota.

Once a derived application class is created, it must be plugged in, or registered, with the appropriate Interface in the LibraryEnvironment. In MPI cases where Dakota is potentially managing concurrent evaluations of the simulation, the plugin must be configured to run on the right MPI sub-communicator, or Dakota analysis_comm. The simpler case is demonstrated in serial_interface_plugin() in library_mode.cpp, while a more advanced case using the analysis communicator is shown in parallel_interface_plugin().

The Dakota LibraryEnvironment provides a convenience function to plugin an Interface. This example will replace any interface found matching the given model, interface, and analysis driver with the passed plugin interface:

.. code-block:: cpp

	std::string model_type(""); // demo: empty string will match any model type
	std::string interf_type("direct");
	std::string an_driver("plugin_rosenbrock");
	Dakota::ProblemDescDB& problem_db = env.problem_description_db();
	std::shared_ptr<Dakota::Interface> serial_iface = 
	  std::make_shared<SIM::SerialDirectApplicInterface>(problem_db);
	bool plugged_in =
	  env.plugin_interface(model_type, interf_type, an_driver, serial_iface);

The LibraryEnvironment also provides convenience functions that allow the client to iterate the lists of available interfaces or models for more advanced cases. For instance if the client knows there is only a single interface active, it could get the list of available interfaces of length 1 and plugin to the first one. In the more advanced case where the simulation interface instance should manage parallel simulations within the context of an MPI communicator, one should pass in the relevant analysis communicator(s) to the derived constructor. For the latter case of looping over a set of models, the simplest approach of passing a single analysis communicator would use code similar to

.. code-block:: cpp

	Dakota::ModelList filt_models = 
	  env.filtered_model_list("single", "direct", "plugin_text_book");
	Dakota::ProblemDescDB& problem_db = env.problem_description_db();
	Dakota::ModelLIter ml_iter;
	for (ml_iter = filt_models.begin(); ml_iter != filt_models.end(); ++ml_iter) {
	  // set DB nodes to input specification for this Model
	  problem_db.set_db_model_nodes(ml_iter->model_id());
	  Dakota::Interface& model_interface = ml_iter->derived_interface();
	  // Parallel case: plug in derived Interface object with an analysisComm.
	  // Note: retrieval and passing of analysisComm is necessary only if
	  // parallel operations will be performed in the derived constructor.
	  // retrieve the currently active analysisComm from the Model.  In the most
	  // general case, need an array of Comms to cover all Model configurations.
	  const MPI_Comm& analysis_comm = ml_iter->analysis_comm();
	  // don't increment ref count since no other envelope shares this letter
	  model_interface.assign_rep(new
		SIM::ParallelDirectApplicInterface(problem_db, analysis_comm), false);
	}

The file library_mode.cpp demonstrates each of these approaches. Since a Model may be used in multiple parallel contexts and may therefore have a set of parallel configurations, a more general approach would extract and pass an array of analysis communicators to allow initialization for each of the parallel configurations.

New derived direct interface instances inherit various attributes of use in configuring the simulation. In particular, the ApplicationInterface::parallelLib reference provides access to MPI communicator data (e.g., the analysis communicators discussed above), DirectApplicInterface::analysisDrivers provides the analysis driver names specified by the user in the input file, and DirectApplicInterface::analysisComponents provides additional analysis component identifiers (such as mesh file names) provided by the user which can be used to distinguish different instances of the same simulation interface. It is worth noting that inherited attributes that are set as part of the parallel configuration (instead of being extracted from the ProblemDescDB) will be set to their defaults following construction of the base class instance for the derived class plug-in. It is not until run-time (i.e., within derived_map_if/derived_map_ac/derived_map_of) that the parallel configuration settings are re-propagated to the plug-in instance. This is the reason that the analysis communicator should be passed in to the constructor of a parallel plug-in, if the constructor will be responsible for parallel application initialization.

===========================
Retrieving data after a run
===========================

After executing the Dakota Environment, final results can be obtained through the use of Environment::variables_results() and Environment::response_results(), e.g.:

.. code-block:: cpp

	// retrieve the final parameter values
	const Variables& vars = env.variables_results();
	// retrieve the final response values
	const Response& resp  = env.response_results();

In the case of optimization, the final design is returned, and in the case of uncertainty quantification, the final statistics are returned. Dakota has a prototype results database, which will eventually provide better access to the results from a study.

==================================
Linking against the Dakota library
==================================

This section presumes Dakota has been configured with CMake, compiled, and installed to a CMAKE_INSTALL_PREFIX using make install or equivalent. The Dakota libraries against which you must link will typically install to CMAKE_INSTALL_PREFIX/bin/ and CMAKE_INSTALL_PREFIX/lib/, while headers are provided in CMAKE_INSTALL_PREFIX/lib/. The core Dakota C and C++ code is in the library dakota_src, while Fortran code lives in the dakota_src_fortran library. Runtime libraries for any configure-enabled Dakota third-party software components (such as DOT, NPSOL, OPT++, LHS, etc.) are also installed to the lib/ directory. Applications link against these Dakota libraries by specifying appropriate include and link directives.

There two primary ways to determine the necessary Dakota-related libraries and link order for linking your application. First, when running CMake, a list of required Dakota and Dakota-included third-party libraries will be output to the console, e.g.,

.. code-block::

	-- Dakota_LIBRARIES: dakota_src;dakota_src_fortran;nidr;teuchos;pecos;pecos_src;lhs;mods;mod;dfftpack;sparsegrid;surfpack;surfpack;surfpack_fortran;utilib;colin;interfaces;scolib;3po;pebbl;tinyxml;conmin;dace;analyzer;random;sampling;bose;dot;fsudace;hopspack;jega;jega_fe;moga;soga;eutils;utilities;ncsuopt;nlpql;cport;nomad;npsol;optpp;psuade;dakota_sciplot;amplsolver

While external dependencies will be output as:

.. code-block::

	-- Dakota_TPL_LIBRARIES: /usr/lib64/libcurl.so;/usr/lib64/openmpi/lib/libmpi_cxx.so;debug;/usr/lib64/libz.so;debug;/usr/lib64/librt.so;debug;/usr/lib64/libdl.so;debug;/usr/lib64/libm.so;debug;/apps/hdf5/1.8.11/lib/libhdf5_hl.so;debug;/apps/hdf5/1.8.11/lib/libhdf5.so;optimized;/usr/lib64/libz.so;optimized;/usr/lib64/librt.so;optimized;/usr/lib64/libdl.so;optimized;/usr/lib64/libm.so;optimized;/apps/hdf5/1.8.11/lib/libhdf5_hl.so;optimized;/apps/hdf5/1.8.11/lib/libhdf5.so;/apps/boost/1.49/lib/libboost_regex.so;/apps/boost/1.49/lib/libboost_filesystem.so;/apps/boost/1.49/lib/libboost_serialization.so;/apps/boost/1.49/lib/libboost_system.so;/apps/boost/1.49/lib/libboost_signals.so;/usr/lib64/libSM.so;/usr/lib64/libICE.so;/usr/lib64/libX11.so;/usr/lib64/libXext.so;/usr/lib64/libXm.so;/usr/lib64/libXpm.so;/usr/lib64/libXmu.so;/usr/lib64/libXt.so;-lpthread;/usr/lib64/liblapack.so;/usr/lib64/libblas.so

Note that depending on how you configured Dakota, some libraries may be omitted from these lists (for example commercial add-ons NPSOL, DOT, and NLPQL), or additional libraries may appear.

A second option is to check which libraries appear in CMAKE_INSTALL_PREFIX/bin/ CMAKE_INSTALL_PREFIX/lib/, or more accurately, see the file Makefile.export.Dakota in the Dakota build/src/ or installation include/ directory. Here are some additional notes on specific libraries:

 - Some Boost libraries (boost_regex, boost_filesystem, boost_system, boost_serialization) are required, and other Boost library components may be required depending on configuration, e.g., boost_signals when configuring with HAVE_ACRO:BOOL=TRUE
 - System compiler and math libraries may need to be included, as may additional system libraries such as Expat and Curl, depending on how Dakota is configured.
 - If configuring with graphics, you will need to add the dakota_sciplot library and system X libraries (partial list here):
 
   .. code-block::
   
      -lXpm -lXm -lXt -lXmu -lXp -lXext -lX11 -lSM -lICE

 - When configuring with AMPL (HAVE_AMPL:BOOL=ON), the AMPL solver library may require dl, funcadd0.o and fl libraries. We have experienced problems with the creation of libamplsolver.a on some platforms; inquire on Dakota's :ref:`dicussions forum<help-discussions>` to get help with any problems related to this.
 - Optional library GSL (discouraged due to GPL license) and if linking with system-provided GSL, gslcblas may be needed if Dakota was configured with them.
 - Newmat: as of Dakota 5.2, -lnewmat is no longer required

Finally, it is important to use the same C++ compiler (possibly an MPI wrapper) for compiling Dakota and your application and potentially include Dakota-related preprocessor defines as emitted by CMake during compilation of Dakota and included in Makefile.export.Dakota. This ensures that the platform configuration settings are properly synchronized across Dakota and your application. 

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

Library clients can construct a study from a complete Dakota input file, an
input string, or the JSON representation accepted by ``LibraryEnvironment``.
The parsed input is stored in Dakota's intermediate representation and exposed
to existing Dakota components through ``ProblemDescDB``. Client code should
treat this database as read-only study configuration after parsing.

Input data parsing
------------------

The ``run_dakota_parse()`` example in ``library_mode.cpp`` shows the supported
file-based workflow:

.. code-block:: cpp

   Dakota::ProgramOptions opts;
   opts.input_file(dakota_input_file);
   Dakota::LibraryEnvironment env(opts);

The input file must contain a complete, valid study. To parse an in-memory
input instead, call ``opts.input_string(input_text)`` before constructing the
environment. Applications that build studies through the C++ dependency
injection API can instead use the JSON ``LibraryEnvironment`` constructor.

Interface injection after parsing
---------------------------------

An application may parse a full input file and then replace its function
evaluator with an in-process implementation. This updates the constructed
runtime ``Interface`` object; it does not mutate schema keys or write directly
to the input intermediate representation.

.. code-block:: cpp

   auto plugin = std::make_shared<MyDirectApplicInterface>(
       env.problem_description_db(), env.parallel_library());
   bool installed = env.plugin_interface(
       "", "direct", "plugin_rosenbrock", plugin);
   if (!installed)
     throw std::runtime_error("matching interface was not found");
   env.execute();

The input must declare a matching interface type and analysis driver. Empty
filter strings match any value. For parallel plugins that need an analysis
communicator, use ``filtered_model_list()`` and install the interface on each
matching model, as demonstrated by ``parallel_interface_plugin()`` in
``library_mode.cpp``.

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

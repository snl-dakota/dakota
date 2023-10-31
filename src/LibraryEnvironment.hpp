/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef LIBRARY_ENVIRONMENT_H
#define LIBRARY_ENVIRONMENT_H

#include "DakotaEnvironment.hpp"

namespace Dakota {


/// Environment corresponding to execution as an embedded library.

/** This environment corresponds to use of Dakota as a library within
    another application, e.g., within library_mode.cpp.  It sets up
    the ParallelLibrary and ProblemDescDB objects without access to
    command line arguments. */

class LibraryEnvironment: public Environment
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  /// default constructor
  LibraryEnvironment();

  /// Primary constructor: program options typically specifies an
  /// input file or input string.  Optionally specify a callback
  /// function to be invoked after parsing.  Set check_bcast_construct
  /// if performing late updates and later calling done_modifying_db().
  LibraryEnvironment(ProgramOptions prog_opts,
		     bool check_bcast_construct = true,
		     DbCallbackFunctionPtr callback = NULL,
		     void* callback_data = NULL);

  /// Alternate constructor accepting communicator, same options as primary
  LibraryEnvironment(MPI_Comm dakota_mpi_comm,
		     ProgramOptions prog_opts = ProgramOptions(),
		     bool check_bcast_construct = true,
		     DbCallbackFunctionPtr callback = NULL,
		     void* callback_data = NULL);

  /// destructor
  ~LibraryEnvironment();


  //
  //- Heading: Virtual function redefinitions
  //

  //void execute(); // base class implementation is sufficient

  //
  //- Heading: Member functions
  //

  /// Insert DB nodes for a {Method,Model,Variables,Interface,Responses} set
  void insert_nodes(Dakota::DataMethod&   dme, Dakota::DataModel&    dmo,
		    Dakota::DataVariables& dv, Dakota::DataInterface& di,
		    Dakota::DataResponses& dr);

  /// Check database contents, broadcast, and construct iterators
  void done_modifying_db();

  /// DEPRECATED raw pointer version: transfers memory ownership to Dakota
  /// Plug-in the passed interface into any interface matching the
  /// specified (possibly empty) model, interface, and driver strings;
  /// returns true if a plugin was performed
  bool plugin_interface(const String& model_type,
			const String& interf_type,
			const String& an_driver,
			Interface* plugin_iface);

  /// Plug-in the passed interface into any interface matching the
  /// specified (possibly empty) model, interface, and driver strings;
  /// returns true if a plugin was performed
  bool plugin_interface(const String& model_type,
			const String& interf_type,
			const String& an_driver,
			std::shared_ptr<Interface> plugin_iface);

  /// filter the available Interface instances based on matching interface
  /// type and analysis drivers (empty String matches any)
  InterfaceList filtered_interface_list(const String& interf_type,
					const String& an_driver);

  /// filter the available Model instances based on matching model
  /// type, interface type, and analysis drivers (empty String matches any)
  ModelList filtered_model_list(const String& model_type,
				const String& interf_type,
				const String& an_driver);

private:

  //
  //- Heading: Convenience member functions
  //
    
  //
  //- Heading: Data members
  //

};

} // namespace Dakota

#endif

/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       LibraryEnvironment
//- Description: 
//- Owner:       Brian Adams
//- Checked by:
//- Version: $Id: LibraryEnvironment.hpp 6492 2009-12-19 00:04:28Z briadam $

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
  /// primary constructor
  LibraryEnvironment(const ProgramOptions& prog_opts);
  /// alternate constructor accepting communicator
  LibraryEnvironment(MPI_Comm dakota_mpi_comm,
		     const ProgramOptions& prog_opts = ProgramOptions());
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

  /// filter the available Interface instances based on matching interface
  /// type and analysis drivers
  InterfaceList filtered_interface_list(const String& interf_type,
					const String& an_driver);
  /// filter the available Model instances based on matching model
  /// type, interface type, and analysis drivers
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

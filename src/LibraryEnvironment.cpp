/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       LibraryEnvironment
//- Description: Implementation code for the LibraryEnvironment class
//- Owner:       Brian Adams
//- Checked by:

#include "dakota_data_util.hpp"
#include "LibraryEnvironment.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaInterface.hpp"

static const char rcsId[]="@(#) $Id: LibraryEnvironment.cpp 6492 2009-12-19 00:04:28Z briadam $";


namespace Dakota {

// BMA TODO: initialize these properly

LibraryEnvironment::LibraryEnvironment():
  Environment(BaseConstructor())
{ }


/** Construct library environment, optionally performing check/bcast
    of database and iterator construction */
LibraryEnvironment::
LibraryEnvironment(const ProgramOptions& prog_opts, bool check_bcast_construct,
		   DbCallbackFunctionPtr callback, void* callback_data):
  Environment(BaseConstructor(), prog_opts)
{
  //  outputManager.output_startup_message();

  // parse input, and instantiate the topLevelIterator
  parse(check_bcast_construct, callback, callback_data);
  if (check_bcast_construct)
    construct();
}


/** Construct library environment on passed MPI Comm, optionally
    performing check/bcast of database and iterator construction */
LibraryEnvironment::
LibraryEnvironment(MPI_Comm dakota_mpi_comm, 
		   const ProgramOptions& prog_opts, bool check_bcast_construct,
		   DbCallbackFunctionPtr callback, void* callback_data):
  Environment(BaseConstructor(), prog_opts, dakota_mpi_comm)
{ 
  //  outputManager.output_startup_message();

  // parse input and instantiate the topLevelIterator
  parse(check_bcast_construct, callback, callback_data);
  if (check_bcast_construct)
    construct();
}


LibraryEnvironment::~LibraryEnvironment()
{ }


void LibraryEnvironment::
insert_nodes(Dakota::DataMethod&   dme, Dakota::DataModel&    dmo,
	     Dakota::DataVariables& dv, Dakota::DataInterface& di,
	     Dakota::DataResponses& dr)
{
  if (parallelLib.world_rank() == 0) {
    // Push instances into the DB
    probDescDB.insert_node(dme);
    probDescDB.insert_node(dmo);
    probDescDB.insert_node(dv);
    probDescDB.insert_node(di);
    probDescDB.insert_node(dr);
  }
}


void LibraryEnvironment::done_modifying_db()
{  
  // always check and broadcast before construction
  probDescDB.check_and_broadcast(programOptions); 
  construct();
}


InterfaceList LibraryEnvironment::
filtered_interface_list(const String& interf_type, const String& an_driver)
{
  InterfaceList filt_interf_list;
  ModelList& models = probDescDB.model_list();
  for (ModelLIter ml_iter=models.begin(); ml_iter!=models.end(); ++ml_iter) {
    Interface& interface = ml_iter->derived_interface();
    if (interface.interface_type() == interf_type &&
      //interface.analysis_drivers().size() == 1  &&
	contains(interface.analysis_drivers(), an_driver))
      filt_interf_list.push_back(interface);
  }
  return filt_interf_list;
}


ModelList LibraryEnvironment::
filtered_model_list(const String& model_type, const String& interf_type,
		    const String& an_driver)
{
  ModelList filt_model_list;
  ModelList& models = probDescDB.model_list();
  for (ModelLIter ml_iter=models.begin(); ml_iter!=models.end(); ++ml_iter) {
    if (ml_iter->model_type() == model_type) {
      Interface& interface = ml_iter->derived_interface();
      if (interface.interface_type() == interf_type &&
	//interface.analysis_drivers().size() == 1  &&
	  contains(interface.analysis_drivers(), an_driver))
	filt_model_list.push_back(*ml_iter);
    }
  }
  return filt_model_list;
}

} // namespace Dakota

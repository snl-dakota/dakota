/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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


LibraryEnvironment::LibraryEnvironment():
  Environment(BaseConstructor())
{ }


/** Construct library environment, optionally performing check/bcast
    of database and iterator construction */
LibraryEnvironment::
LibraryEnvironment(ProgramOptions prog_opts, bool check_bcast_construct,
		   DbCallbackFunctionPtr callback, void* callback_data):
  Environment(BaseConstructor(), prog_opts)
{
  // TODO: support run modes help, version, check
  outputManager.output_startup_message();

  // parse input, and instantiate the topLevelIterator
  parse(check_bcast_construct, callback, callback_data);
  if (check_bcast_construct)
    construct();
}


/** Construct library environment on passed MPI Comm, optionally
    performing check/bcast of database and iterator construction.  MPI
    Comm is first argument so client doesn't have to pass all args */
LibraryEnvironment::
LibraryEnvironment(MPI_Comm dakota_mpi_comm, 
		   ProgramOptions prog_opts, bool check_bcast_construct,
		   DbCallbackFunctionPtr callback, void* callback_data):
  Environment(BaseConstructor(), prog_opts, dakota_mpi_comm)
{ 
  // TODO: support run modes help, version, check
  outputManager.output_startup_message();

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


bool LibraryEnvironment::plugin_interface(const String& model_type,
					  const String& interf_type,
					  const String& an_driver,
					  Interface* plugin_iface)
{
  bool plugged_in = false;
  ModelList filt_models
    = filtered_model_list(model_type, interf_type, an_driver);

  if (filt_models.empty())
    Cerr << "Warning: interface plugin requested, but no interfaces matched "
	 << "specified\n  model type = " << model_type << "\n  interface type = "
	 << interf_type << "\n  driver name = " << an_driver << std::endl;

  size_t model_index = probDescDB.get_db_model_node(); // for restoration
  ModelLIter ml_iter, ml_end = filt_models.end();
  for (ml_iter = filt_models.begin(); ml_iter != ml_end; ++ml_iter) {
    // set DB nodes to input specification for this Model
    probDescDB.set_db_model_nodes(ml_iter->model_id());
    // plugin the Interface
    Interface& model_interface = ml_iter->derived_interface();
    // don't increment ref count since no other envelope shares this letter
    model_interface.assign_rep(plugin_iface, false);
    plugged_in = true;
  }
  probDescDB.set_db_model_nodes(model_index);          // restore

  return plugged_in;
}


/** This convenience function helps clients locate and plugin to the
    right Interface instance for simple cases. Pass an empty string to
    match any instead of a specific instance */
InterfaceList LibraryEnvironment::
filtered_interface_list(const String& interf_type, const String& an_driver)
{
  InterfaceList filt_interf_list;
  ModelList& models = probDescDB.model_list();
  ModelLIter ml_iter, ml_end = models.end();
  for (ml_iter = models.begin(); ml_iter != ml_end; ++ml_iter) {
    Interface& model_interface = ml_iter->derived_interface();
    if ( ( interf_type.empty() || 
	   interface_enum_to_string(model_interface.interface_type()) == 
	   interf_type ) &&
	 ( an_driver.empty() || 
	   //interface.analysis_drivers().size() == 1  &&
	   contains(model_interface.analysis_drivers(), an_driver) ) )
      filt_interf_list.push_back(model_interface);
  }
  return filt_interf_list;
}


/** This convenience function helps clients locate and plugin to the
    right Interface instance for cases where the parallel
    configuration is needed in constructing a parallel plugin.  Pass
    an empty string to match any instead of a specific instance */
ModelList LibraryEnvironment::
filtered_model_list(const String& model_type, const String& interf_type,
		    const String& an_driver)
{
  ModelList filt_model_list;
  ModelList& models = probDescDB.model_list();
  ModelLIter ml_iter, ml_end = models.end();
  for (ml_iter = models.begin(); ml_iter != ml_end; ++ml_iter) {
    if (model_type.empty() || ml_iter->model_type() == model_type) {
      Interface& model_interface = ml_iter->derived_interface();
      if ( ( interf_type.empty() || 
	     interface_enum_to_string(model_interface.interface_type()) == 
	     interf_type ) &&
	   ( an_driver.empty() || 
	     //interface.analysis_drivers().size() == 1  &&
	     contains(model_interface.analysis_drivers(), an_driver) ) )
	filt_model_list.push_back(*ml_iter);
    }
  }
  return filt_model_list;
}

} // namespace Dakota

/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

// Class:        ProblemDescDB
//- Description: Implementation code for the ProblemDescDB class.
//-              It provides storage for problem description database entries
//-              and defines the keyword handlers that yacc calls to populate
//-              the database based on the parsed input.
//- Owner:       Mike Eldred
//- Checked by:

#include "dakota_system_defs.hpp"
#include "iterator_utils.hpp"
#include "model_utils.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "NIDRProblemDescDB.hpp"
#include "ProgramOptions.hpp"
#include "DakotaIterator.hpp"
#include "DakotaInterface.hpp"
#include "WorkdirHelper.hpp"  // bfs utils and prepend_preferred_env_path
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <string>


//#define DEBUG
//#define MPI_DEBUG

static const char rcsId[]="@(#) $Id: ProblemDescDB.cpp 7007 2010-10-06 15:54:39Z wjbohnh $";


namespace Dakota {

extern ParallelLibrary dummy_lib; // defined in dakota_global_defs.cpp
extern ProblemDescDB *Dak_pddb;	  // defined in dakota_global_defs.cpp


/** This constructor is the one which must build the base class data for all
    derived classes.  get_db() instantiates a derived class letter and the
    derived constructor selects this base class constructor in its
    initialization list (to avoid the recursion of the base class constructor
    calling get_db() again).  Since the letter IS the representation, its
    representation pointer is set to NULL. */
ProblemDescDB::ProblemDescDB(BaseConstructor, ParallelLibrary& parallel_lib):
  parallelLib(parallel_lib), environmentCntr(0), methodDBLocked(true),
  modelDBLocked(true), variablesDBLocked(true), interfaceDBLocked(true),
  responsesDBLocked(true)
{ /* empty ctor */ }


/** The default constructor: dbRep is NULL in this case.  This makes
    it necessary to check for NULL in the copy constructor, assignment
    operator, and destructor. */
ProblemDescDB::ProblemDescDB(): parallelLib(dummy_lib)
{ /* empty ctor */ }


/** This is the envelope constructor which uses problem_db to build a
    fully populated db object.  It only needs to extract enough data
    to properly execute get_db(problem_db), since the constructor
    overloaded with BaseConstructor builds the actual base class data
    inherited by the derived classes. */
ProblemDescDB::ProblemDescDB(ParallelLibrary& parallel_lib):
  parallelLib(parallel_lib),
  // Set the rep pointer to the appropriate db type
  dbRep(get_db(parallel_lib))

{
  if (!dbRep) // bad settings or insufficient memory
    abort_handler(-1);
}


/** Initializes dbRep to the appropriate derived type.  The standard
    derived class constructors are invoked.  */
std::shared_ptr<ProblemDescDB>
ProblemDescDB::get_db(ParallelLibrary& parallel_lib)
{
  Dak_pddb = this;	// for use in abort_handler()

  //if (xml_flag)
  //  return new XMLProblemDescDB(parallel_lib);
  //else
  return std::make_shared<NIDRProblemDescDB>(parallel_lib);
}


/** Copy constructor manages sharing of dbRep */
ProblemDescDB::ProblemDescDB(const ProblemDescDB& db):
  parallelLib(db.parallel_library()),
  dbRep(db.dbRep)
{ /* empty ctor */ }


/** Assignment operator shares the dbRep. */
ProblemDescDB ProblemDescDB::operator=(const ProblemDescDB& db)
{
  dbRep = db.dbRep;
  return *this; // calls copy constructor since returned by value
}


/** dbRep only deleted when its reference count reaches zero. */
ProblemDescDB::~ProblemDescDB()
{
  if (this == Dak_pddb)
    Dak_pddb = NULL;
}


/** DB setup phase 1: parse the input file and execute callback
    functions if present.  Rank 0 only.

    DB setup phase 2: optionally insert additional data via late sets.
    Rank 0 only. */
void ProblemDescDB::
parse_inputs(ProgramOptions& prog_opts,
	     DbCallbackFunctionPtr callback, void *callback_data)
{
  if (dbRep) {
    dbRep->parse_inputs(prog_opts, callback, callback_data);
    // BMA TODO: Temporary workaround; can't get callback to work on
    // letter yet. Need to replace Null_rep* with forward to letter
    // and remove dbRep->, but initial cut didn't work.
    if (callback && dbRep->parallelLib.world_rank() == 0)
      (*callback)(this, callback_data);
  }
  else {

    // Only the master parses the input file.
    if (parallelLib.world_rank() == 0) {

      if ( !prog_opts.input_file().empty() &&
	   !prog_opts.input_string().empty() ) {
	Cerr << "\nError: parse_inputs called with both input file and input "
	     << "string." << std::endl;
	abort_handler(PARSE_ERROR);
      }

      if (prog_opts.preproc_input()) {

	if (prog_opts.echo_input()) {
	  echo_input_file(prog_opts.input_file(), prog_opts.input_string(),
			  " template");
	  echo_input_file(prog_opts.preprocessed_file(), "");
	}

	// Parse the input file using one of the derived parser-specific classes
	derived_parse_inputs(prog_opts.preprocessed_file(), "",
			     prog_opts.parser_options());

	// Remove file created by preprocessing input
	boost::filesystem::remove(prog_opts.preprocessed_file());
      }
      else {

	if (prog_opts.echo_input())
	  echo_input_file(prog_opts.input_file(), prog_opts.input_string());

	// Parse the input file using one of the derived parser-specific classes
	derived_parse_inputs(prog_opts.input_file(), prog_opts.input_string(),
			     prog_opts.parser_options());

      }

      // Allow user input by callback function.

      // BMA TODO: Is this comment true?
      // Note: the DB is locked and the list iterators are not defined.  Thus,
      // the user function must do something to put the DB in a usable set/get
      // state (e.g., resolve_top_method() or set_db_list_nodes()).

      // if (callback)
      // 	(*callback)(this, callback_data);

    }

  }
}


/** DB setup phase 3: perform basic checks on keywords counts in
    current DB state, then sync to all processors. */
void ProblemDescDB::check_and_broadcast(const ProgramOptions& prog_opts) {

  if (dbRep)
    dbRep->check_and_broadcast(prog_opts);
  else {

    // Check to make sure at least one of each of the keywords was found
    // in the problem specification file; checks only happen on Dakota rank 0
    if (parallelLib.world_rank() == 0)
      check_input();

    // bcast a minimal MPI buffer containing the input specification
    // data prior to post-processing
    broadcast();

    // After broadcast, perform post-processing on all processors to
    // size default variables/responses specification vectors (avoid
    // sending large vectors over an MPI buffer).
    post_process();

  }
}



void ProblemDescDB::broadcast()
{
  if (dbRep)
    dbRep->broadcast();
  else {
    // DAKOTA's old design for reading the input file was for the master to get
    // the input filename from cmd_line_handler (after MPI_Init) and broadcast
    // the character buffer to all other processors (having every processor
    // query the cmd_line_handler was failing because of the effect of MPI_Init
    // on argc and argv).  Then every processor yyparsed.  This worked fine but
    // was not scalable for MP machines with a limited number of I/O devices.

    // Now, rank 0 yyparse's and sends all the parsed data in a single buffer
    // to all other ranks.
    if (parallelLib.world_size() > 1) {
      if (parallelLib.world_rank() == 0) {
	enforce_unique_ids();
	derived_broadcast(); // pre-processor
	send_db_buffer();
#ifdef MPI_DEBUG
	Cout << "DB buffer to send on world rank " << parallelLib.world_rank()
	     << ":\n" << environmentSpec << dataMethodList << dataVariablesList
	     << dataInterfaceList << dataResponsesList << std::endl;
#endif // MPI_DEBUG
      }
      else {
	receive_db_buffer();
#ifdef MPI_DEBUG
	Cout << "DB buffer received on world rank " << parallelLib.world_rank()
	     << ":\n" << environmentSpec << dataMethodList << dataVariablesList
	     << dataInterfaceList << dataResponsesList << std::endl;
#endif // MPI_DEBUG
	//derived_broadcast(); // post-processor
      }
    }
    else {
#ifdef DEBUG
      Cout << "DB parsed data:\n" << environmentSpec << dataMethodList
	   << dataVariablesList << dataInterfaceList << dataResponsesList
	   << std::endl;
#endif // DEBUG
      enforce_unique_ids();
      derived_broadcast();
    }
  }
}


/** When using library mode in a parallel application, post_process()
    should be called on all processors following broadcast() of a
    minimal problem specification. */
void ProblemDescDB::post_process()
{
  // no base class post-processing operations to perform
  if (dbRep)
    dbRep->derived_post_process();
  else
    derived_post_process();
}


void ProblemDescDB::
derived_parse_inputs(const std::string& dakota_input_file,
		     const std::string& dakota_input_string,
		     const std::string& parser_options)
{
  if (dbRep)
    dbRep->derived_parse_inputs(dakota_input_file, dakota_input_string,
				parser_options);
  else { // this fn must be redefined
    Cerr << "Error: Letter lacking redefinition of virtual derived_parse_inputs"
	 << " function.\n       No default defined at base class." << std::endl;
    abort_handler(-1);
  }
}


void ProblemDescDB::derived_broadcast()
{
  if (dbRep)
    dbRep->derived_broadcast();
  // else do nothing: this fn is optional
}


void ProblemDescDB::derived_post_process()
{
  if (dbRep)
    dbRep->derived_post_process();
  // else do nothing: this fn is optional
}


/** NOTE: when using library mode in a parallel application,
    check_input() should either be called only on worldRank 0, or it
    should follow a matched send_db_buffer()/receive_db_buffer() pair. */
void ProblemDescDB::check_input()
{
  if (dbRep)
    dbRep->check_input();
  else {

    int num_errors = 0;
    //if (!environmentCntr) { // Allow environment omission
    //  Cerr << "No environment specification found in input file.\n";
    //  ++num_errors;
    //}
    if (environmentCntr > 1) {
      Cerr << "Multiple environment specifications not allowed in input "
	   << "file.\n";
      ++num_errors;
    }
    if (dataMethodList.empty()) {
      Cerr << "No method specification found in input file.\n";
      ++num_errors;
    }
    if (dataVariablesList.empty()) {
      Cerr << "No variables specification found in input file.\n";
      ++num_errors;
    }
    if (dataInterfaceList.empty()) {
      // interface spec may be omitted in case of global data fits
      bool interface_reqd = true;
      // global surrogate with data reuse from either restart or points_file
      for (std::list<DataModel>::iterator dm_iter = dataModelList.begin();
	   dm_iter!=dataModelList.end(); ++dm_iter)
	if ( strbegins(dm_iter->dataModelRep->surrogateType, "global_") &&
	     ( ( !dm_iter->dataModelRep->approxPointReuse.empty() &&
		  dm_iter->dataModelRep->approxPointReuse != "none" ) ||
	       !dm_iter->dataModelRep->importBuildPtsFile.empty() ) )
	  interface_reqd = false;
      if (interface_reqd)
	for (std::list<DataMethod>::iterator dm_iter = dataMethodList.begin();
	     dm_iter != dataMethodList.end(); ++dm_iter)
	  if (!dm_iter->dataMethodRep->importBuildPtsFile.empty())
	    interface_reqd = false;
      if (interface_reqd) {
	Cerr << "No interface specification found in input file.\n";
	++num_errors;
      }
      else {
	// needed for setting DB interface node to something; prevents errors
	// in any interface spec data lookups (e.g., Interface base class ctor
	// called from ApproximationInterface ctor)
	DataInterface data_interface; // use defaults
	dataInterfaceList.push_back(data_interface);
      }
    }
    if (dataResponsesList.empty()) {
      Cerr << "No responses specification found in input file.\n";
      ++num_errors;
    }
    if (dataModelList.empty()) { // Allow model omission
      DataModel data_model; // use defaults: modelType == "simulation"
      dataModelList.push_back(data_model);
    }

    if (parallelLib.command_line_user_modes()) {

      if (!parallelLib.command_line_pre_run_input().empty())
	Cerr << "Warning: pre-run input not implemented; ignored.\n";

      if (!parallelLib.command_line_pre_run_output().empty()) {
	if (dataMethodList.size() > 1) {
	  Cerr << "Error: pre-run output only allowed for single method.\n";
	  ++num_errors;
	}
	else if (!dataMethodList.empty()) {
	  // exactly one method
	  // TODO: Test for iterator concurrency
	  std::list<DataMethod>::iterator dm = dataMethodList.begin();
	  unsigned short method_name = dm->dataMethodRep->methodName;
	  if ( !(method_name & PSTUDYDACE_BIT) &&
	       !(method_name == RANDOM_SAMPLING) ) {
	    Cerr << "Error: pre-run output not supported for method "
		 << method_name << "\n       (supported for sampling, "
		 << "parameter study, DDACE, FSUDACE, and PSUADE methods)\n";
	    ++num_errors;
	  }
	}
      }

      if (!parallelLib.command_line_run_input().empty())
	Cerr << "Warning: run input not implemented; ignored.\n";

      if (!parallelLib.command_line_run_output().empty())
	Cerr << "Warning: run output not implemented; ignored.\n";

      if (!parallelLib.command_line_post_run_input().empty()) {
	if (dataMethodList.size() > 1) {
	  Cerr << "Error: post-run input only allowed for single method.\n";
	  ++num_errors;
	}
	else if (!dataMethodList.empty()) {
	  // exactly one method
	  // TODO: Test for iterator concurrency
	  std::list<DataMethod>::iterator dm = dataMethodList.begin();
	  unsigned short method_name = dm->dataMethodRep->methodName;
	  if ( !(method_name & PSTUDYDACE_BIT) &&
	       !(method_name == RANDOM_SAMPLING) ) {
	    Cerr << "Error: post-run input not supported for method "
		 << method_name << "\n       (supported for sampling, "
		 << "parameter study, DDACE, FSUDACE, and PSUADE methods)\n";
	    ++num_errors;
	  }
	}
      }

      if (!parallelLib.command_line_post_run_output().empty())
	Cerr << "Warning: post-run output not implemented; ignored.\n";

    }

    if (num_errors) {
      Cerr << num_errors << " input specification errors detected." <<std::endl;
      abort_handler(PARSE_ERROR);
    }
  }
}


void ProblemDescDB::set_db_list_nodes(const String& method_tag)
{
  if (dbRep)
    dbRep->set_db_list_nodes(method_tag);
  // for simplicity in client logic, allow NO_SPECIFICATION case to fall
  // through: do not update iterators or locks, such that previous
  // specification settings remain active (NO_SPECIFICATION instances
  // within a recursion do not alter list node sequencing).
  else if (!strbegins(method_tag, "NOSPEC_METHOD_ID_")) {
    set_db_method_node(method_tag);
    if (methodDBLocked) {
      modelDBLocked = variablesDBLocked = interfaceDBLocked
	= responsesDBLocked = true;
      // ensure consistency in get_db_{method,model}_node():
      //dataModelIter = dataModelList.end();
    }
    else
      set_db_model_nodes(dataMethodIter->dataMethodRep->modelPointer);
  }
}


void ProblemDescDB::set_db_list_nodes(size_t method_index)
{
  if (dbRep)
    dbRep->set_db_list_nodes(method_index);
  else {
    // Set the correct Index values for all Data class lists.
    set_db_method_node(method_index);
    if (methodDBLocked) {
      modelDBLocked = variablesDBLocked = interfaceDBLocked
	= responsesDBLocked = true;
      // ensure consistency in get_db_{method,model}_node():
      //dataModelIter = dataModelList.end();
    }
    else
      set_db_model_nodes(dataMethodIter->dataMethodRep->modelPointer);
  }
}


void ProblemDescDB::resolve_top_method(bool set_model_nodes)
{
  if (dbRep)
    dbRep->resolve_top_method(set_model_nodes);
  else { // deduce which method spec sits on top
    String& top_meth_ptr = environmentSpec.dataEnvRep->topMethodPointer;
    size_t num_method_spec = dataMethodList.size();
    if (num_method_spec == 1)
      dataMethodIter = dataMethodList.begin();
    else if (!top_meth_ptr.empty())
      dataMethodIter =
	std::find_if( dataMethodList.begin(), dataMethodList.end(),
		      boost::bind(DataMethod::id_compare, _1, top_meth_ptr) );
    else { // identify which id_method does not appear in a method_pointer
      // Collect list of all method id's (including empty ids)
      StringList method_ids;
      for (std::list<DataMethod>::iterator it=dataMethodList.begin();
	   it!=dataMethodList.end(); it++)
	method_ids.push_back(it->dataMethodRep->idMethod);
      // Eliminate sub-method pointers from method specs
      for (std::list<DataMethod>::iterator it=dataMethodList.begin();
	   it!=dataMethodList.end(); it++)
	if (!it->dataMethodRep->subMethodPointer.empty()) {
          StringList::iterator slit
            = std::find(method_ids.begin(), method_ids.end(),
                        it->dataMethodRep->subMethodPointer);
          if (slit != method_ids.end()) method_ids.erase(slit);
	}
      // Eliminate method_pointers from model specs
      for (std::list<DataModel>::iterator it=dataModelList.begin();
	   it!=dataModelList.end(); it++)
	if (!it->dataModelRep->subMethodPointer.empty()) {
          StringList::iterator slit
            = std::find(method_ids.begin(), method_ids.end(),
                        it->dataModelRep->subMethodPointer);
          if (slit != method_ids.end()) method_ids.erase(slit);
	}
      // by process of elimination, select the top method
      if (method_ids.empty() || method_ids.size() > 1) {
	Cerr << "\nError: ProblemDescDB::resolve_top_method() failed to "
	     << "determine active method specification.\n       Please resolve "
	     << "method pointer ambiguities." << std::endl;
	abort_handler(PARSE_ERROR);
      }
      else {
	const String& method_id = *method_ids.begin();
	dataMethodIter
	  = std::find_if( dataMethodList.begin(), dataMethodList.end(),
              boost::bind(DataMethod::id_compare, _1, method_id) );
      }
    }
    methodDBLocked = false; // unlock

    // set all subordinate list nodes for this method
    if (set_model_nodes)
      set_db_model_nodes(dataMethodIter->dataMethodRep->modelPointer);
  }
}


void ProblemDescDB::set_db_method_node(const String& method_tag)
{
  if (dbRep)
    dbRep->set_db_method_node(method_tag);
  // for simplicity in client logic, allow NO_SPECIFICATION case to fall
  // through: do not update dataMethodIter or methodDBLocked, such that
  // previous specification settings remain active (NO_SPECIFICATION
  // instances within a recursion do not alter list node sequencing).
  else if (!strbegins(method_tag, "NOSPEC_METHOD_ID_")) {
    // set the correct Index values for all Data class lists.
    if (method_tag.empty()) { // no pointer specification
      if (dataMethodList.size() == 1) // no ambiguity if only one spec
	dataMethodIter = dataMethodList.begin();
      else { // try to match to a method without an id
	dataMethodIter
	  = std::find_if( dataMethodList.begin(), dataMethodList.end(),
              boost::bind(DataMethod::id_compare, _1, method_tag) );
	if (dataMethodIter == dataMethodList.end()) {
	  if (parallelLib.world_rank() == 0)
	    Cerr << "\nWarning: empty method id string not found.\n         "
		 << "Last method specification parsed will be used.\n";
	  --dataMethodIter; // last entry in list
	}
	else if (parallelLib.world_rank() == 0 &&
		 std::count_if(dataMethodList.begin(), dataMethodList.end(),
                   boost::bind(DataMethod::id_compare, _1, method_tag)) > 1)
	  Cerr << "\nWarning: empty method id string is ambiguous.\n         "
	       << "First matching method specification will be used.\n";
      }
      methodDBLocked = false; // unlock
    }
    else {
      std::list<DataMethod>::iterator dm_it
	= std::find_if( dataMethodList.begin(), dataMethodList.end(),
            boost::bind(DataMethod::id_compare, _1, method_tag) );
      if (dm_it == dataMethodList.end()) {
	methodDBLocked = true; // lock (moot)
	Cerr << "\nError: " << method_tag
	     << " is not a valid method identifier string." << std::endl;
	abort_handler(PARSE_ERROR);
      }
      else {
	methodDBLocked = false; // unlock
	dataMethodIter = dm_it;
	if (parallelLib.world_rank() == 0 &&
	    std::count_if(dataMethodList.begin(), dataMethodList.end(),
			  boost::bind(DataMethod::id_compare,_1,method_tag))>1)
	  Cerr << "\nWarning: method id string " << method_tag
	       << " is ambiguous.\n         First matching method "
	       << "specification will be used.\n";
      }
    }
  }
}


void ProblemDescDB::set_db_method_node(size_t method_index)
{
  if (dbRep)
    dbRep->set_db_method_node(method_index);
  else if (method_index == _NPOS)
    methodDBLocked = true;
  else {
    size_t num_meth_spec = dataMethodList.size();
    // allow advancement up to but not past end()
    if (method_index > num_meth_spec) {
      Cerr << "\nError: method_index sent to set_db_method_node is out of "
	   << "range." << std::endl;
      abort_handler(PARSE_ERROR);
    }
    dataMethodIter = dataMethodList.begin();
    std::advance(dataMethodIter, method_index);
    // unlock if not advanced to end()
    methodDBLocked = (method_index == num_meth_spec);
  }
}


void ProblemDescDB::set_db_model_nodes(size_t model_index)
{
  if (dbRep)
    dbRep->set_db_model_nodes(model_index);
  else if (model_index == _NPOS)
    modelDBLocked = variablesDBLocked = interfaceDBLocked
      = responsesDBLocked = true;
  else {
    size_t num_model_spec = dataModelList.size();
    // allow advancement up to but not past end()
    if (model_index > num_model_spec) {
      Cerr << "\nError: model_index sent to set_db_model_nodes is out of range."
	   << std::endl;
      abort_handler(PARSE_ERROR);
    }
    dataModelIter = dataModelList.begin();
    std::advance(dataModelIter, model_index);
    // unlock if not advanced to end()
    if (model_index == num_model_spec)
      modelDBLocked = variablesDBLocked = interfaceDBLocked = responsesDBLocked
	= true;
    else {
      const DataModelRep& MoRep = *dataModelIter->dataModelRep;
      set_db_variables_node(MoRep.variablesPointer);
      if (model_has_interface(MoRep))
	set_db_interface_node(MoRep.interfacePointer);
      else
	interfaceDBLocked = true;
      set_db_responses_node(MoRep.responsesPointer);
    }
  }
}


void ProblemDescDB::set_db_model_nodes(const String& model_tag)
{
  if (dbRep)
    dbRep->set_db_model_nodes(model_tag);
  // for simplicity in client logic, allow NO_SPECIFICATION case to fall
  // through: do not update model iterators or locks, such that previous
  // specification settings remain active (NO_SPECIFICATION instances
  // within a recursion do not alter list node sequencing).
  else if (! (model_tag == "NO_SPECIFICATION" ||
        strbegins(model_tag, "NOSPEC_MODEL_ID_") ||
        strbegins(model_tag, "RECAST_"))) {
    // set dataModelIter from model_tag
    if (model_tag.empty() || model_tag == "NO_MODEL_ID") { // no pointer specification
      if (dataModelList.empty()) { // Note: check_input() prevents this
	DataModel data_model; // for library mode
	dataModelList.push_back(data_model);
      }
      if (dataModelList.size() == 1) // no ambiguity if only one spec
	dataModelIter = dataModelList.begin();
      else { // try to match to a model without an id
	dataModelIter
	  = std::find_if( dataModelList.begin(), dataModelList.end(),
              boost::bind(DataModel::id_compare, _1, model_tag) );
	if (dataModelIter == dataModelList.end()) {
	  if (parallelLib.world_rank() == 0)
	    Cerr << "\nWarning: empty model id string not found.\n         "
		 << "Last model specification parsed will be used.\n";
	  --dataModelIter; // last entry in list
	}
	else if (parallelLib.world_rank() == 0 &&
		 std::count_if(dataModelList.begin(), dataModelList.end(),
                   boost::bind(DataModel::id_compare, _1, model_tag)) > 1)
	  Cerr << "\nWarning: empty model id string is ambiguous.\n         "
	       << "First matching model specification will be used.\n";
      }
      modelDBLocked = false; // unlock
    }
    else {
      std::list<DataModel>::iterator dm_it
	= std::find_if( dataModelList.begin(), dataModelList.end(),
            boost::bind(DataModel::id_compare, _1, model_tag) );
      if (dm_it == dataModelList.end()) {
	modelDBLocked = true; // lock (moot)
	Cerr << "\nError: " << model_tag
	     << " is not a valid model identifier string." << std::endl;
	abort_handler(PARSE_ERROR);
      }
      else {
	modelDBLocked = false; // unlock
	dataModelIter = dm_it;
	if (parallelLib.world_rank() == 0 &&
	    std::count_if(dataModelList.begin(), dataModelList.end(),
			  boost::bind(DataModel::id_compare, _1, model_tag))>1)
	  Cerr << "\nWarning: model id string " << model_tag << " is ambiguous."
	       << "\n         First matching model specification will be used."
	       << '\n';
      }
    }

    if (modelDBLocked)
      variablesDBLocked = interfaceDBLocked = responsesDBLocked	= true;
    else {
      const DataModelRep& MoRep = *dataModelIter->dataModelRep;
      set_db_variables_node(MoRep.variablesPointer);
      if (model_has_interface(MoRep))
	set_db_interface_node(MoRep.interfacePointer);
      else
	interfaceDBLocked = true;
      set_db_responses_node(MoRep.responsesPointer);
    }
  }
}


void ProblemDescDB::set_db_variables_node(const String& variables_tag)
{
  if (dbRep)
    dbRep->set_db_variables_node(variables_tag);
  // for simplicity in client logic, allow NO_SPECIFICATION case to fall
  // through: do not update dataVariablesIter or variablesDBLocked, such
  // that previous specification remains active (NO_SPECIFICATION
  // instances within a recursion do not alter list node sequencing).
  else if (variables_tag != "NO_SPECIFICATION") { // not currently in use
    // set dataVariablesIter from variables_tag
    if (variables_tag.empty()) { // no pointer specification
      if (dataVariablesList.size() == 1) // no ambiguity if only one spec
	dataVariablesIter = dataVariablesList.begin();
      else { // try to match to a variables without an id
	dataVariablesIter
	  = std::find_if( dataVariablesList.begin(), dataVariablesList.end(),
              boost::bind(DataVariables::id_compare, _1, variables_tag) );
	if (dataVariablesIter == dataVariablesList.end()) {
	  if (parallelLib.world_rank() == 0)
	    Cerr << "\nWarning: empty variables id string not found.\n         "
		 << "Last variables specification parsed will be used.\n";
	  --dataVariablesIter; // last entry in list
	}
	else if (parallelLib.world_rank() == 0 &&
		 std::count_if(dataVariablesList.begin(),
			       dataVariablesList.end(),
			       boost::bind(DataVariables::id_compare, _1,
					   variables_tag)) > 1)
	  Cerr << "\nWarning: empty variables id string is ambiguous."
	       << "\n         First matching variables specification will be "
	       << "used.\n";
      }
      variablesDBLocked = false; // unlock
    }
    else {
      std::list<DataVariables>::iterator dv_it
	= std::find_if( dataVariablesList.begin(), dataVariablesList.end(),
            boost::bind(DataVariables::id_compare, _1, variables_tag) );
      if (dv_it == dataVariablesList.end()) {
	variablesDBLocked = true; // lock (moot)
	Cerr << "\nError: " << variables_tag
	     << " is not a valid variables identifier string." << std::endl;
	abort_handler(PARSE_ERROR);
      }
      else {
	variablesDBLocked = false; // unlock
	dataVariablesIter = dv_it;
	if (parallelLib.world_rank() == 0 &&
	    std::count_if(dataVariablesList.begin(), dataVariablesList.end(),
			  boost::bind(DataVariables::id_compare, _1,
				      variables_tag)) > 1)
	  Cerr << "\nWarning: variables id string " << variables_tag
	       << " is ambiguous.\n         First matching variables "
	       << "specification will be used.\n";
      }
    }
  }
}


void ProblemDescDB::set_db_interface_node(const String& interface_tag)
{
  if (dbRep)
    dbRep->set_db_interface_node(interface_tag);
  // for simplicity in client logic, allow NO_SPECIFICATION case to fall
  // through: do not update dataInterfaceIter or interfaceDBLocked, such
  // that previous specification remains active (NO_SPECIFICATION
  // instances within a recursion do not alter list node sequencing).
  else if (!strbegins(interface_tag, "NOSPEC_INTERFACE_ID_")) {
    const DataModelRep& MoRep = *dataModelIter->dataModelRep;
    // set dataInterfaceIter from interface_tag
    if (interface_tag.empty() || interface_tag == "NO_ID") { // no pointer specification
      if (dataInterfaceList.size() == 1) // no ambiguity if only one spec
	dataInterfaceIter = dataInterfaceList.begin();
      else { // try to match to a interface without an id
	dataInterfaceIter
	  = std::find_if( dataInterfaceList.begin(), dataInterfaceList.end(),
              boost::bind(DataInterface::id_compare, _1, interface_tag) );
	// echo warning if a default interface list entry will be used and more
	// than 1 interface specification is present.  Currently this can only
	// happen for simulation models, since surrogate model specifications
	// do not contain interface ptrs and the omission of an optional
	// interface ptr in nested models indicates the omission of an optional
	// interface (rather than the presence of an unidentified interface).
	if (dataInterfaceIter == dataInterfaceList.end()) {
	  if (parallelLib.world_rank() == 0 &&
	      MoRep.modelType == "simulation")
	    Cerr << "\nWarning: empty interface id string not found.\n         "
		 << "Last interface specification parsed will be used.\n";
	  --dataInterfaceIter; // last entry in list
	}
	else if (parallelLib.world_rank() == 0 &&
		 MoRep.modelType == "simulation"  &&
		 std::count_if(dataInterfaceList.begin(),
			       dataInterfaceList.end(),
			       boost::bind(DataInterface::id_compare, _1,
					   interface_tag)) > 1)
	  Cerr << "\nWarning: empty interface id string is ambiguous."
	       << "\n         First matching interface specification will be "
	       << "used.\n";
      }
      interfaceDBLocked = false; // unlock
    }
    else {
      std::list<DataInterface>::iterator di_it
	= std::find_if( dataInterfaceList.begin(), dataInterfaceList.end(),
            boost::bind(DataInterface::id_compare, _1, interface_tag) );
      if (di_it == dataInterfaceList.end()) {
	interfaceDBLocked = true; // lock (moot)
	Cerr << "\nError: " << interface_tag
	     << " is not a valid interface identifier string." << std::endl;
	abort_handler(PARSE_ERROR);
      }
      else {
	interfaceDBLocked = false; // unlock
	dataInterfaceIter = di_it;
	if (parallelLib.world_rank() == 0 &&
	    std::count_if(dataInterfaceList.begin(), dataInterfaceList.end(),
			  boost::bind(DataInterface::id_compare, _1,
				      interface_tag)) > 1)
	  Cerr << "\nWarning: interface id string " << interface_tag
	       << " is ambiguous.\n         First matching interface "
	       << "specification will be used.\n";
      }
    }
  }
}


void ProblemDescDB::set_db_responses_node(const String& responses_tag)
{
  if (dbRep)
    dbRep->set_db_responses_node(responses_tag);
  // for simplicity in client logic, allow NO_SPECIFICATION case to fall
  // through: do not update dataResponsesIter or responsesDBLocked,
  // such that previous specification remains active (NO_SPECIFICATION
  // instances within a recursion do not alter list node sequencing).
  else if (responses_tag != "NO_SPECIFICATION") {
    // set dataResponsesIter from responses_tag
    if (responses_tag.empty()) { // no pointer specification
      if (dataResponsesList.size() == 1) // no ambiguity if only one spec
	dataResponsesIter = dataResponsesList.begin();
      else { // try to match to a responses without an id
	dataResponsesIter
	  = std::find_if( dataResponsesList.begin(), dataResponsesList.end(),
              boost::bind(DataResponses::id_compare, _1, responses_tag) );
	if (dataResponsesIter == dataResponsesList.end()) {
	  if (parallelLib.world_rank() == 0)
	    Cerr << "\nWarning: empty responses id string not found.\n         "
		 << "Last responses specification parsed will be used.\n";
	  --dataResponsesIter; // last entry in list
	}
	else if (parallelLib.world_rank() == 0 &&
		 std::count_if(dataResponsesList.begin(),
			       dataResponsesList.end(),
			       boost::bind(DataResponses::id_compare, _1,
					   responses_tag)) > 1)
	  Cerr << "\nWarning: empty responses id string is ambiguous."
	       << "\n         First matching responses specification will be "
	       << "used.\n";
      }
      responsesDBLocked = false; // unlock
    }
    else {
      std::list<DataResponses>::iterator dr_it
	= std::find_if( dataResponsesList.begin(), dataResponsesList.end(),
            boost::bind(DataResponses::id_compare, _1, responses_tag) );
      if (dr_it == dataResponsesList.end()) {
	responsesDBLocked = true; // lock (moot)
	Cerr << "\nError: " << responses_tag
	     << " is not a valid responses identifier string." << std::endl;
	abort_handler(PARSE_ERROR);
      }
      else {
	responsesDBLocked = false; // unlock
	dataResponsesIter = dr_it;
	if (parallelLib.world_rank() == 0 &&
	    std::count_if(dataResponsesList.begin(), dataResponsesList.end(),
			  boost::bind(DataResponses::id_compare, _1,
				      responses_tag)) > 1)
	  Cerr << "\nWarning: responses id string " << responses_tag
	       << " is ambiguous.\n         First matching responses "
	       << "specification will be used.\n";
      }
    }
  }
}


void ProblemDescDB::send_db_buffer()
{
  MPIPackBuffer send_buffer;
  send_buffer << environmentSpec   << dataMethodList    << dataModelList
	      << dataVariablesList << dataInterfaceList << dataResponsesList;

  // Broadcast length of buffer so that slaves can allocate MPIUnpackBuffer
  int buffer_len = send_buffer.size();
  parallelLib.bcast_w(buffer_len);

  // Broadcast actual buffer
  parallelLib.bcast_w(send_buffer);
}


void ProblemDescDB::receive_db_buffer()
{
  // receive length of incoming buffer and allocate space for MPIUnpackBuffer
  int buffer_len;
  parallelLib.bcast_w(buffer_len);

  // receive incoming buffer
  MPIUnpackBuffer recv_buffer(buffer_len);
  parallelLib.bcast_w(recv_buffer);
  recv_buffer >> environmentSpec   >> dataMethodList    >> dataModelList
	      >> dataVariablesList >> dataInterfaceList >> dataResponsesList;
}


std::shared_ptr<Iterator> ProblemDescDB::get_iterator()
{
  // ProblemDescDB::get_<object> functions operate at the envelope level
  // so that any passing of *this provides the envelope object.
  if (!dbRep) {
    Cerr << "Error: ProblemDescDB::get_iterator() called for letter object."
	 << std::endl;
    abort_handler(PARSE_ERROR);
  }

  // In general, have to worry about loss of encapsulation and use of context
  // _above_ this specification.  However, any dependence on the environment
  // specification is OK since there is only one.  All other specifications
  // are identified via model_pointer.

  // The DB list nodes are set prior to calling get_iterator():
  // >    method_ptr spec -> id_method must be defined
  // > no method_ptr spec -> id_method is ignored, method spec is last parsed
  // Reuse logic works in both cases -> only a single unreferenced iterator
  // may exist, which corresponds to the last method spec and is reused for
  // all untagged instantiations.
  String id_method = dbRep->dataMethodIter->dataMethodRep->idMethod;
  if(id_method.empty())
    id_method = "NO_METHOD_ID";
  IterLIter i_it
    = std::find_if(dbRep->iteratorList.begin(), dbRep->iteratorList.end(),
                  [&id_method](std::shared_ptr<Iterator> iter) {return iter->method_id() == id_method;});
  if (i_it == dbRep->iteratorList.end()) {
    dbRep->iteratorList.push_back(IteratorUtils::get_iterator(*this));
    i_it = --dbRep->iteratorList.end();
  }
  return *i_it;
}


std::shared_ptr<Iterator> ProblemDescDB::get_iterator(std::shared_ptr<Model> model)
{
  // ProblemDescDB::get_<object> functions operate at the envelope level
  // so that any passing of *this provides the envelope object.
  if (!dbRep) {
    Cerr << "Error: ProblemDescDB::get_iterator() called for letter object."
	 << std::endl;
    abort_handler(PARSE_ERROR);
  }

  String id_method = dbRep->dataMethodIter->dataMethodRep->idMethod;
  if(id_method.empty())
    id_method = "NO_METHOD_ID";
  IterLIter i_it
    = std::find_if(dbRep->iteratorList.begin(), dbRep->iteratorList.end(),
                    [&id_method](std::shared_ptr<Iterator> iter) {return iter->method_id() == id_method;});

  // if Iterator does not already exist, then create it
  if (i_it == dbRep->iteratorList.end()) {
    dbRep->iteratorList.push_back(IteratorUtils::get_iterator(*this, model));
    i_it = --dbRep->iteratorList.end();
  }
  // idMethod already exists, but check for same model.  If !same, instantiate
  // new rather than update (i_it->iterated_model(model)) all shared instances.
  else if (model != i_it->iterated_model()) {
    dbRep->iteratorList.push_back(IteratorUtils::get_iterator(*this, model));
    i_it = --dbRep->iteratorList.end();
  }
  return *i_it;
}


std::shared_ptr<Iterator> ProblemDescDB::
get_iterator(const String& method_name, std::shared_ptr<Model> model)
{
  // ProblemDescDB::get_<object> functions operate at the envelope level
  // so that any passing of *this provides the envelope object.
  if (!dbRep) {
    Cerr << "Error: ProblemDescDB::get_iterator() called for letter object."
	 << std::endl;
    abort_handler(PARSE_ERROR);
  }

  IterLIter i_it
    = std::find_if(dbRep->iteratorByNameList.begin(),
		   dbRep->iteratorByNameList.end(),
                  [&method_name](std::shared_ptr<Iterator> iter) {return iter->method_string() == method_name;});
  // if Iterator does not already exist, then create it
  if (i_it == dbRep->iteratorByNameList.end()) {
    dbRep->iteratorByNameList.push_back(IteratorUtils::get_iterator(method_name, model));
    i_it = --dbRep->iteratorByNameList.end();
  }
  // method_name already exists, but check for same model. If !same, instantiate
  // new rather than update (i_it->iterated_model(model)) all shared instances.
  else if (model != i_it->iterated_model()) {
    dbRep->iteratorByNameList.push_back(IteratorUtils::get_iterator(method_name, model));
    i_it = --dbRep->iteratorByNameList.end();
  }
  return *i_it;
}


std::shared_ptr<Model> ProblemDescDB::get_model()
{
  // ProblemDescDB::get_<object> functions operate at the envelope level
  // so that any passing of *this provides the envelope object.
  if (!dbRep) {
    Cerr << "Error: ProblemDescDB::get_model() called for letter object."
         << std::endl;
    abort_handler(PARSE_ERROR);
  }

  // A model specification identifies its variables, interface, and responses.
  // Have to worry about loss of encapsulation and use of context _above_ this
  // specification, i.e., any dependence on an iterator specification
  // (dependence on the environment spec is OK since there is only one).
  // > method.output
  // > Constraints: variables view

  // The DB list nodes are set prior to calling get_model():
  // >    model_ptr spec -> id_model must be defined
  // > no model_ptr spec -> id_model is ignored, model spec is last parsed
  String id_model = dbRep->dataModelIter->dataModelRep->idModel;
  if(id_model.empty())
    id_model = "NO_MODEL_ID";
  ModelLIter m_it
    = std::find_if(dbRep->modelList.begin(), dbRep->modelList.end(),
                   [&id_model](std::shared_ptr<Model> m) {return m->model_id() == id_model;});
  if (m_it == dbRep->modelList.end()) {
    dbRep->modelList.push_back(ModelUtils::get_model(*this));
    m_it = --dbRep->modelList.end();
  }
  return *m_it;
}


const Variables& ProblemDescDB::get_variables()
{
  // ProblemDescDB::get_<object> functions operate at the envelope level
  // so that any passing of *this provides the envelope object.
  if (!dbRep) {
    Cerr << "Error: ProblemDescDB::get_variables() called for letter object."
	 << std::endl;
    abort_handler(PARSE_ERROR);
  }

  // Have to worry about loss of encapsulation and use of context _above_ this
  // specification, i.e., any dependence on iterator/model/interface/responses
  // specifications (dependence on the environment specification is OK since
  // there is only one).
  // > variables view is method dependent

  // The DB list nodes are set prior to calling get_variables():
  // >    variables_ptr spec -> id_variables must be defined
  // > no variables_ptr spec -> id_variables ignored, vars spec = last parsed
  //const String& id_variables = dbRep->dataVariablesIter->idVariables;

  // Turn off variables reuse for now, since it is problematic with surrogates:
  // a top level variables set followed by a subModel eval which sets subModel
  // vars (where the subModel vars object is reused) results in a top level
  // eval with the wrong vars (e.g., surrogate auto-build in
  // dakota_textbook_lhs_approx.in).
  //
  // In general, variables object reuse should be fine for objects with peer
  // relationships, but are questionable for use among nested/layered levels.
  // Need a way to detect peer vs. nested/layered relationships.
  VarsLIter v_it;
  // = dbRep->variablesList.find(variables_id_compare, &id_variables);
  //if ( v_it == dbRep->variablesList.end() ||
  //     v_it->view() != v_it->get_view(*this) ) {
    Variables new_variables(*this);
    dbRep->variablesList.push_back(new_variables);
    v_it = --dbRep->variablesList.end();
  //}
  return *v_it;
}


const Interface& ProblemDescDB::get_interface()
{
  // ProblemDescDB::get_<object> functions operate at the envelope level
  // so that any passing of *this provides the envelope object.
  if (!dbRep) {
    Cerr << "Error: ProblemDescDB::get_interface() called for letter object."
	 << std::endl;
    abort_handler(PARSE_ERROR);
  }

  // Have to worry about loss of encapsulation and use of context _above_ this
  // specification, i.e., any dependence on iterator/model/variables/responses
  // specifications (dependence on the environment specification is OK since
  // there is only one):
  // > Interface: method.output
  // > ApplicationInterface: responses.gradient_type, responses.hessian_type,
  //     responses.gradients.mixed.id_analytic
  // > DakotaInterface: responses.labels

  // ApproximationInterfaces and related classes are OK, since they are
  // instantiated with assign_rep() for each unique DataFitSurrModel instance:
  // > ApproximationInterface: model.surrogate.function_ids
  // > Approximation: method.output, model.surrogate.type,
  //     model.surrogate.derivative_usage
  // > SurfpackApproximation: model.surrogate.polynomial_order,
  //     model.surrogate.kriging_correlations
  // > TaylorApproximation: model.surrogate.truth_model_pointer,
  //     responses.hessian_type
  // > OrthogPolyApproximation: method.nond.expansion_{terms,order}

  // The DB list nodes are set prior to calling get_interface():
  // >    interface_ptr spec -> id_interface must be defined
  // > no interface_ptr spec -> id_interf ignored, interf spec = last parsed
  String id_interface
    = dbRep->dataInterfaceIter->dataIfaceRep->idInterface;
  if(id_interface.empty())
    id_interface = "NO_ID";

  InterfLIter i_it
    = std::find_if(dbRep->interfaceList.begin(), dbRep->interfaceList.end(),
                   boost::bind(&Interface::interface_id, _1) == id_interface);
  if (i_it == dbRep->interfaceList.end()) {
    Interface new_interface(*this);
    dbRep->interfaceList.push_back(new_interface);
    i_it = --dbRep->interfaceList.end();
  }
  return *i_it;
}


const Response& ProblemDescDB::get_response(short type, const Variables& vars)
{
  // ProblemDescDB::get_<object> functions operate at the envelope level
  // so that any passing of *this provides the envelope object.
  if (!dbRep) {
    Cerr << "Error: ProblemDescDB::get_response() called for letter object."
	 << std::endl;
    abort_handler(PARSE_ERROR);
  }

  // Have to worry about loss of encapsulation and use of context _above_ this
  // specification, i.e., any dependence on iterator/model/variables/interface
  // specifications (dependence on the environment specification is OK since
  // there is only one).
  // > mismatch in vars attributes (cv(),continuous_variable_ids()) should be OK
  //   since derivative arrays are dynamically resized based on current active
  //   set content

  // The DB list nodes are set prior to calling get_response():
  // >    responses_ptr spec -> id_responses must be defined
  // > no responses_ptr spec -> id_responses ignored, resp spec = last parsed
  //const String& id_responses
  //  = dbRep->dataResponsesIter->dataRespRep->idResponses;

  // Turn off response reuse for now, even though it has not yet been
  // problematic.  In general, response object reuse should be fine for objects
  // with peer relationships, but are questionable for use among nested/layered
  // levels.  Need a way to detect peer vs. nested/layered relationships.
  RespLIter r_it;
  // = dbRep->responseList.find(responses_id_compare, &id_responses);
  //if (r_it == dbRep->responseList.end()) { // ||
    //r_it->active_set_derivative_vector() != vars.continuous_variable_ids()) {
    Response new_response(type, vars, *this);
    dbRep->responseList.push_back(new_response);
    r_it = --dbRep->responseList.end();
  //}}
  return *r_it;
}


inline int ProblemDescDB::min_procs_per_ea()
{
  // Note: get_*() requires envelope execution (throws error if !dbRep)

  // Note: DataInterfaceRep::procsPerAnalysis defaults to zero, which is used
  // when the processors_per_analysis spec is unreachable (system/fork/spawn)
  return min_procs_per_level(1, // min_ppa
    get_int("interface.direct.processors_per_analysis"), // 0 for non-direct
    get_int("interface.analysis_servers"));
}


int ProblemDescDB::max_procs_per_ea()
{
  // Note: get_*() requires envelope execution (throws error if !dbRep)

  // TO DO: can we be more fine grained on parallel testers?
  //        default tester could get hidden by plug-in...

  int max_ppa = (get_ushort("interface.type") & DIRECT_INTERFACE_BIT) ?
    parallelLib.world_size() : 1; // system/fork/spawn
  // Note: DataInterfaceRep::procsPerAnalysis defaults to zero, which is used
  // when the processors_per_analysis spec is unreachable (system/fork/spawn)
  return max_procs_per_level(max_ppa,
    get_int("interface.direct.processors_per_analysis"), // 0 for non-direct
    get_int("interface.analysis_servers"),
    get_short("interface.analysis_scheduling"),
    get_int("interface.asynch_local_analysis_concurrency"),
    false, // peer dynamic not supported
    std::max(1, (int)get_sa("interface.application.analysis_drivers").size()));
}


int ProblemDescDB::min_procs_per_ie()
{
  // Note: get_*() requires envelope execution (throws error if !dbRep)

  return min_procs_per_level(min_procs_per_ea(),
			     get_int("interface.processors_per_evaluation"),
			     get_int("interface.evaluation_servers"));
			   //get_short("interface.evaluation_scheduling"));
}


int ProblemDescDB::max_procs_per_ie(int max_eval_concurrency)
{
  // Note: get_*() requires envelope execution (throws error if !dbRep)

  // Define max_procs_per_iterator to estimate maximum processor usage
  // from all lower levels.  With default_config = PUSH_DOWN, this is
  // important to avoid pushing down more resources than can be utilized.
  // The primary input is algorithmic concurrency, but we also incorporate
  // explicit user overrides for _lower_ levels (user overrides for the
  // current level can be managed by resolve_inputs()).

  int max_ea   = max_procs_per_ea(),
      ppe_spec = get_int("interface.processors_per_evaluation"),
      max_pps  = (ppe_spec) ? ppe_spec : max_ea;
  // for peer dynamic, max_pps == 1 is imperfect in that it does not capture
  // all possibilities, but this is conservative and hopefully close enough
  // for this context (an upper bound estimate).
  bool peer_dynamic_avail = (get_short("interface.local_evaluation_scheduling")
			     != STATIC_SCHEDULING && max_pps == 1);

  return max_procs_per_level(max_ea, ppe_spec,
    get_int("interface.evaluation_servers"),
    get_short("interface.evaluation_scheduling"),
    get_int("interface.asynch_local_evaluation_concurrency"),
    peer_dynamic_avail, max_eval_concurrency);
}


static void Bad_name(const String& entry_name, const String& where)
{
  Cerr << "\nBad entry_name '" << entry_name << "' in ProblemDescDB::"
       << where << std::endl;
  abort_handler(PARSE_ERROR);
}

static void Locked_db()
{
  Cerr << "\nError: database is locked.  You must first unlock the database\n"
       << "       by setting the list nodes." << std::endl;
  abort_handler(PARSE_ERROR);
}

static void Null_rep(const String& who)
{
  Cerr << "\nError: ProblemDescDB::" << who
       << " called with NULL representation." << std::endl;
  abort_handler(PARSE_ERROR);
}

// split the entry name on the first period into block.entry
std::pair<std::string, std::string>
split_entry_name(const std::string& entry_name, const std::string& context_msg)
{
  auto first_dot = entry_name.find(".");
  // must find a split point and have trailing lookup entry content
  if (first_dot == std::string::npos || first_dot == entry_name.size()-1)
    Bad_name(entry_name, context_msg);
  const std::string block = entry_name.substr(0, first_dot);
  const std::string entry = entry_name.substr(first_dot + 1,
					      entry_name.size() - first_dot - 1);
  return std::make_pair(block, entry);
}

template <typename T>
T& ProblemDescDB::
get(const std::string& context_msg,
    const std::map<std::string, T DataEnvironmentRep::*>& env_map,
    const std::map<std::string, T DataMethodRep::*>& met_map,
    const std::map<std::string, T DataModelRep::*>& mod_map,
    const std::map<std::string, T DataVariablesRep::*>& var_map,
    const std::map<std::string, T DataInterfaceRep::*>& int_map,
    const std::map<std::string, T DataResponsesRep::*>& res_map,
    const std::string& entry_name,
    const std::shared_ptr<ProblemDescDB>& db_rep) const
{
  if (!db_rep)
    Null_rep(context_msg);
  
  std::string block, entry;
  std::tie(block, entry) = split_entry_name(entry_name, context_msg);

  if (block == "environment") {
    auto it = env_map.find(entry);
    if (it != env_map.end())
      return (db_rep->environmentSpec.dataEnvRep).get()->*(it->second);
  }
  else if (block == "method") {
    if (db_rep->methodDBLocked)
      Locked_db();
    auto it = met_map.find(entry);
    if (it != met_map.end())
      return (db_rep->dataMethodIter->dataMethodRep).get()->*(it->second);
  }
  else if (block == "model") {
    if (db_rep->modelDBLocked)
      Locked_db();
    auto it = mod_map.find(entry);
    if (it != mod_map.end())
      return (db_rep->dataModelIter->dataModelRep).get()->*(it->second);
  }
  else if (block == "variables") {
    if (db_rep->variablesDBLocked)
      Locked_db();
    auto it = var_map.find(entry);
    if (it != var_map.end())
      return (db_rep->dataVariablesIter->dataVarsRep).get()->*(it->second);
  }
  else if (block == "interface") {
    if (db_rep->interfaceDBLocked)
      Locked_db();
    auto it = int_map.find(entry);
    if (it != int_map.end())
      return (db_rep->dataInterfaceIter->dataIfaceRep).get()->*(it->second);
  }
  else if (block == "responses") {
    if (db_rep->responsesDBLocked)
      Locked_db();
    auto it = res_map.find(entry);
    if (it != res_map.end())
      return (db_rep->dataResponsesIter->dataRespRep).get()->*(it->second);
  }
  Bad_name(entry_name, context_msg);
  return abort_handler_t<T&>(PARSE_ERROR);
}

// couldn't get const-correctness right with a simple forwarder...
// template <typename T>
// void ProblemDescDB::LookerUpper<T>::
// set(const std::string& entry_name,
//     std::shared_ptr<ProblemDescDB>& db_rep, const T entry_value) const
// {
//   T& entry_rep_ref = get(entry_name, db_rep);
//   entry_rep_ref = entry_value;
// }


// shorthand for pointer to Data*Rep members for use in key to data maps;
// these names are super terse on purpose and only used in this compilation unit
#define P_ENV &DataEnvironmentRep::
#define P_MET &DataMethodRep::
#define P_MOD &DataModelRep::
#define P_VAR &DataVariablesRep::
#define P_INT &DataInterfaceRep::
#define P_RES &DataResponsesRep::


const RealMatrixArray& ProblemDescDB::get_rma(const String& entry_name) const
{
  return get<RealMatrixArray>
  ( "get_rma()",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"discrete_design_set_int.adjacency_matrix", P_VAR discreteDesignSetIntAdj},
      {"discrete_design_set_real.adjacency_matrix", P_VAR discreteDesignSetRealAdj},
      {"discrete_design_set_str.adjacency_matrix", P_VAR discreteDesignSetStrAdj}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);
}


const RealVector& ProblemDescDB::get_rv(const String& entry_name) const
{  
  return get<const RealVector>
  ( "get_rv()",
    { /* environment */ },
    { /* method */
      {"concurrent.parameter_sets", P_MET concurrentParameterSets},
      {"jega.distance_vector", P_MET distanceVector},
      {"jega.niche_vector", P_MET nicheVector},
      {"nond.data_dist_covariance", P_MET dataDistCovariance},
      {"nond.data_dist_means", P_MET dataDistMeans},
      {"nond.dimension_preference", P_MET anisoDimPref},
      {"nond.hyperprior_alphas", P_MET hyperPriorAlphas},
      {"nond.hyperprior_betas", P_MET hyperPriorBetas},
      {"nond.prediction_configs", P_MET predictionConfigList},
      {"nond.proposal_covariance_data", P_MET proposalCovData},
      {"nond.regression_noise_tolerance", P_MET regressionNoiseTol},
      {"nond.relaxation.factor_sequence", P_MET relaxFactorSequence},
      {"nond.scalarization_response_mapping", P_MET scalarizationRespCoeffs},
      {"parameter_study.final_point", P_MET finalPoint},
      {"parameter_study.list_of_points", P_MET listOfPoints},
      {"parameter_study.step_vector", P_MET stepVector},
      {"trust_region.initial_size", P_MET trustRegionInitSize}
    },
    { /* model */
      {"nested.primary_response_mapping", P_MOD primaryRespCoeffs},
      {"nested.secondary_response_mapping", P_MOD secondaryRespCoeffs},
      {"simulation.solution_level_cost", P_MOD solutionLevelCost},
      {"surrogate.kriging_correlations", P_MOD krigingCorrelations},
      {"surrogate.kriging_max_correlations", P_MOD krigingMaxCorrelations},
      {"surrogate.kriging_min_correlations", P_MOD krigingMinCorrelations}
    },
    { /* variables */
      {"beta_uncertain.alphas", P_VAR betaUncAlphas},
      {"beta_uncertain.betas", P_VAR betaUncBetas},
      {"beta_uncertain.lower_bounds", P_VAR betaUncLowerBnds},
      {"beta_uncertain.upper_bounds", P_VAR betaUncUpperBnds},
      {"binomial_uncertain.prob_per_trial", P_VAR binomialUncProbPerTrial},
      {"continuous_aleatory_uncertain.initial_point",
    	  P_VAR continuousAleatoryUncVars},
      {"continuous_aleatory_uncertain.lower_bounds",
    	  P_VAR continuousAleatoryUncLowerBnds},
      {"continuous_aleatory_uncertain.upper_bounds",
    	  P_VAR continuousAleatoryUncUpperBnds},
      {"continuous_design.initial_point", P_VAR continuousDesignVars},
      {"continuous_design.lower_bounds", P_VAR continuousDesignLowerBnds},
      {"continuous_design.scales", P_VAR continuousDesignScales},
      {"continuous_design.upper_bounds", P_VAR continuousDesignUpperBnds},
      {"continuous_epistemic_uncertain.initial_point",
    	  P_VAR continuousEpistemicUncVars},
      {"continuous_epistemic_uncertain.lower_bounds",
    	  P_VAR continuousEpistemicUncLowerBnds},
      {"continuous_epistemic_uncertain.upper_bounds",
    	  P_VAR continuousEpistemicUncUpperBnds},
      {"continuous_state.initial_state", P_VAR continuousStateVars},
      {"continuous_state.lower_bounds", P_VAR continuousStateLowerBnds},
      {"continuous_state.upper_bounds", P_VAR continuousStateUpperBnds},
      {"discrete_aleatory_uncertain_real.initial_point",
    	  P_VAR discreteRealAleatoryUncVars},
      {"discrete_aleatory_uncertain_real.lower_bounds",
    	  P_VAR discreteRealAleatoryUncLowerBnds},
      {"discrete_aleatory_uncertain_real.upper_bounds",
    	  P_VAR discreteRealAleatoryUncUpperBnds},
      {"discrete_design_set_real.initial_point", P_VAR discreteDesignSetRealVars},
      {"discrete_design_set_real.lower_bounds",
    	  P_VAR discreteDesignSetRealLowerBnds},
      {"discrete_design_set_real.upper_bounds",
    	  P_VAR discreteDesignSetRealUpperBnds},
      {"discrete_epistemic_uncertain_real.initial_point",
    	  P_VAR discreteRealEpistemicUncVars},
      {"discrete_epistemic_uncertain_real.lower_bounds",
    	  P_VAR discreteRealEpistemicUncLowerBnds},
      {"discrete_epistemic_uncertain_real.upper_bounds",
    	  P_VAR discreteRealEpistemicUncUpperBnds},
      {"discrete_state_set_real.initial_state", P_VAR discreteStateSetRealVars},
      {"discrete_state_set_real.lower_bounds",
    	  P_VAR discreteStateSetRealLowerBnds},
      {"discrete_state_set_real.upper_bounds",
    	  P_VAR discreteStateSetRealUpperBnds},
      {"exponential_uncertain.betas", P_VAR exponentialUncBetas},
      {"frechet_uncertain.alphas", P_VAR frechetUncAlphas},
      {"frechet_uncertain.betas", P_VAR frechetUncBetas},
      {"gamma_uncertain.alphas", P_VAR gammaUncAlphas},
      {"gamma_uncertain.betas", P_VAR gammaUncBetas},
      {"geometric_uncertain.prob_per_trial", P_VAR geometricUncProbPerTrial},
      {"gumbel_uncertain.alphas", P_VAR gumbelUncAlphas},
      {"gumbel_uncertain.betas", P_VAR gumbelUncBetas},
      {"linear_equality_constraints", P_VAR linearEqConstraintCoeffs},
      {"linear_equality_scales", P_VAR linearEqScales},
      {"linear_equality_targets", P_VAR linearEqTargets},
      {"linear_inequality_constraints", P_VAR linearIneqConstraintCoeffs},
      {"linear_inequality_lower_bounds", P_VAR linearIneqLowerBnds},
      {"linear_inequality_scales", P_VAR linearIneqScales},
      {"linear_inequality_upper_bounds", P_VAR linearIneqUpperBnds},
      {"lognormal_uncertain.error_factors", P_VAR lognormalUncErrFacts},
      {"lognormal_uncertain.lambdas", P_VAR lognormalUncLambdas},
      {"lognormal_uncertain.lower_bounds", P_VAR lognormalUncLowerBnds},
      {"lognormal_uncertain.means", P_VAR lognormalUncMeans},
      {"lognormal_uncertain.std_deviations", P_VAR lognormalUncStdDevs},
      {"lognormal_uncertain.upper_bounds", P_VAR lognormalUncUpperBnds},
      {"lognormal_uncertain.zetas", P_VAR lognormalUncZetas},
      {"loguniform_uncertain.lower_bounds", P_VAR loguniformUncLowerBnds},
      {"loguniform_uncertain.upper_bounds", P_VAR loguniformUncUpperBnds},
      {"negative_binomial_uncertain.prob_per_trial",
    	  P_VAR negBinomialUncProbPerTrial},
      {"normal_uncertain.lower_bounds", P_VAR normalUncLowerBnds},
      {"normal_uncertain.means", P_VAR normalUncMeans},
      {"normal_uncertain.std_deviations", P_VAR normalUncStdDevs},
      {"normal_uncertain.upper_bounds", P_VAR normalUncUpperBnds},
      {"poisson_uncertain.lambdas", P_VAR poissonUncLambdas},
      {"triangular_uncertain.lower_bounds", P_VAR triangularUncLowerBnds},
      {"triangular_uncertain.modes", P_VAR triangularUncModes},
      {"triangular_uncertain.upper_bounds", P_VAR triangularUncUpperBnds},
      {"uniform_uncertain.lower_bounds", P_VAR uniformUncLowerBnds},
      {"uniform_uncertain.upper_bounds", P_VAR uniformUncUpperBnds},
      {"weibull_uncertain.alphas", P_VAR weibullUncAlphas},
      {"weibull_uncertain.betas", P_VAR weibullUncBetas}
    },
    { /* interface */
      {"failure_capture.recovery_fn_vals", P_INT recoveryFnVals}
    },
    { /* responses */
      {"exp_config_variables", P_RES expConfigVars},
      {"exp_observations", P_RES expObservations},
      {"exp_std_deviations", P_RES expStdDeviations},
      {"fd_gradient_step_size", P_RES fdGradStepSize},
      {"fd_hessian_step_size", P_RES fdHessStepSize},
      {"nonlinear_equality_scales", P_RES nonlinearEqScales},
      {"nonlinear_equality_targets", P_RES nonlinearEqTargets},
      {"nonlinear_inequality_lower_bounds", P_RES nonlinearIneqLowerBnds},
      {"nonlinear_inequality_scales", P_RES nonlinearIneqScales},
      {"nonlinear_inequality_upper_bounds", P_RES nonlinearIneqUpperBnds},
      {"primary_response_fn_scales", P_RES primaryRespFnScales},
      {"primary_response_fn_weights", P_RES primaryRespFnWeights},
      {"simulation_variance", P_RES simVariance}
    },
    entry_name, dbRep);
}


const IntVector& ProblemDescDB::get_iv(const String& entry_name) const
{
  return get<const IntVector>
  ( "get_iv()",
    { /* environment */ },
    { /* method */
      {"fsu_quasi_mc.primeBase", P_MET primeBase},
      {"fsu_quasi_mc.sequenceLeap", P_MET sequenceLeap},
      {"fsu_quasi_mc.sequenceStart", P_MET sequenceStart},
      {"nond.refinement_samples", P_MET refineSamples},
      {"parameter_study.steps_per_variable", P_MET stepsPerVariable},
      {"generating_vector.inline", P_MET generatingVector},
      {"generating_matrices.inline", P_MET generatingMatrices}
    },
    { /* model */
      {"refinement_samples", P_MOD refineSamples}
    },
    { /* variables */
      {"binomial_uncertain.num_trials", P_VAR binomialUncNumTrials},
      {"discrete_aleatory_uncertain_int.initial_point",
    	  P_VAR discreteIntAleatoryUncVars},
      {"discrete_aleatory_uncertain_int.lower_bounds",
    	  P_VAR discreteIntAleatoryUncLowerBnds},
      {"discrete_aleatory_uncertain_int.upper_bounds",
    	  P_VAR discreteIntAleatoryUncUpperBnds},
      {"discrete_design_range.initial_point", P_VAR discreteDesignRangeVars},
      {"discrete_design_range.lower_bounds", P_VAR discreteDesignRangeLowerBnds},
      {"discrete_design_range.upper_bounds", P_VAR discreteDesignRangeUpperBnds},
      {"discrete_design_set_int.initial_point", P_VAR discreteDesignSetIntVars},
      {"discrete_design_set_int.lower_bounds",
    	  P_VAR discreteDesignSetIntLowerBnds},
      {"discrete_design_set_int.upper_bounds",
    	  P_VAR discreteDesignSetIntUpperBnds},
      {"discrete_epistemic_uncertain_int.initial_point",
    	  P_VAR discreteIntEpistemicUncVars},
      {"discrete_epistemic_uncertain_int.lower_bounds",
    	  P_VAR discreteIntEpistemicUncLowerBnds},
      {"discrete_epistemic_uncertain_int.upper_bounds",
    	  P_VAR discreteIntEpistemicUncUpperBnds},
      {"discrete_state_range.initial_state", P_VAR discreteStateRangeVars},
      {"discrete_state_range.lower_bounds", P_VAR discreteStateRangeLowerBnds},
      {"discrete_state_range.upper_bounds", P_VAR discreteStateRangeUpperBnds},
      {"discrete_state_set_int.initial_state", P_VAR discreteStateSetIntVars},
      {"discrete_state_set_int.lower_bounds", P_VAR discreteStateSetIntLowerBnds},
      {"discrete_state_set_int.upper_bounds", P_VAR discreteStateSetIntUpperBnds},
      {"hypergeometric_uncertain.num_drawn", P_VAR hyperGeomUncNumDrawn},
      {"hypergeometric_uncertain.selected_population",
    	  P_VAR hyperGeomUncSelectedPop},
      {"hypergeometric_uncertain.total_population", P_VAR hyperGeomUncTotalPop},
      {"negative_binomial_uncertain.num_trials", P_VAR negBinomialUncNumTrials}
    },
    { /* interface */ },
    { /* responses */
      {"lengths", P_RES fieldLengths},
      {"num_coordinates_per_field", P_RES numCoordsPerField}
    },
    entry_name, dbRep);
}


const BitArray& ProblemDescDB::get_ba(const String& entry_name) const
{
  return get<const BitArray>
  ( "get_ba()",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"binomial_uncertain.categorical", P_VAR binomialUncCat},
      {"discrete_design_range.categorical", P_VAR discreteDesignRangeCat},
      {"discrete_design_set_int.categorical", P_VAR discreteDesignSetIntCat},
      {"discrete_design_set_real.categorical", P_VAR discreteDesignSetRealCat},
      {"discrete_interval_uncertain.categorical", P_VAR discreteIntervalUncCat},
      {"discrete_state_range.categorical", P_VAR discreteStateRangeCat},
      {"discrete_state_set_int.categorical", P_VAR discreteStateSetIntCat},
      {"discrete_state_set_real.categorical", P_VAR discreteStateSetRealCat},
      {"discrete_uncertain_set_int.categorical", P_VAR discreteUncSetIntCat},
      {"discrete_uncertain_set_real.categorical", P_VAR discreteUncSetRealCat},
      {"geometric_uncertain.categorical", P_VAR geometricUncCat},
      {"histogram_uncertain.point_int.categorical",
    	  P_VAR histogramUncPointIntCat},
      {"histogram_uncertain.point_real.categorical",
    	  P_VAR histogramUncPointRealCat},
      {"hypergeometric_uncertain.categorical", P_VAR hyperGeomUncCat},
      {"negative_binomial_uncertain.categorical", P_VAR negBinomialUncCat},
      {"poisson_uncertain.categorical", P_VAR poissonUncCat}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);
}


const SizetArray& ProblemDescDB::get_sza(const String& entry_name) const
{
  return get<const SizetArray>
  ( "get_sza()",
    { /* environment */ },
    { /* method */
      {"nond.c3function_train.start_rank_sequence", P_MET startRankSeq},
      {"nond.collocation_points", P_MET collocationPointsSeq},
      {"nond.expansion_samples", P_MET expansionSamplesSeq},
      {"nond.pilot_samples", P_MET pilotSamples},
      {"random_seed_sequence", P_MET randomSeedSeq}
    },
    { /* model */ },
    { /* variables */ },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);
}


const UShortArray& ProblemDescDB::get_usa(const String& entry_name) const
{
  return get<const UShortArray>
  ( "get_usa()",
    { /* environment */ },
    { /* method */
      {"nond.c3function_train.start_order_sequence", P_MET startOrderSeq},
      {"nond.expansion_order", P_MET expansionOrderSeq},
      {"nond.quadrature_order", P_MET quadratureOrderSeq},
      {"nond.sparse_grid_level", P_MET sparseGridLevelSeq},
      {"nond.tensor_grid_order", P_MET tensorGridOrder},
      {"partitions", P_MET varPartitions}
    },
    { /* model */ },
    { /* variables */ },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);
}


const RealSymMatrix& ProblemDescDB::get_rsm(const String& entry_name) const
{
  return get<const RealSymMatrix>
  ( "get_rsm()",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      { "uncertain.correlation_matrix", P_VAR uncertainCorrelations}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);
}


const RealVectorArray& ProblemDescDB::get_rva(const String& entry_name) const
{
  return get<const RealVectorArray>
  ( "get_rva()",
    { /* environment */ },
    { /* method */
      {"nond.gen_reliability_levels", P_MET genReliabilityLevels},
      {"nond.probability_levels", P_MET probabilityLevels},
      {"nond.reliability_levels", P_MET reliabilityLevels},
      {"nond.response_levels", P_MET responseLevels}
    },
    { /* model */ },
    { /* variables */ },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);
}


const IntVectorArray& ProblemDescDB::get_iva(const String& entry_name) const
{
  // BMA: no current use cases
  return get<const IntVectorArray>
  ( "get_iva()",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */ },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);
}


const IntSet& ProblemDescDB::get_is(const String& entry_name) const
{
  return get<const IntSet>
  ( "get_is()",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */ },
    { /* interface */ },
    { /* responses */
      {"gradients.mixed.id_analytic", P_RES idAnalyticGrads},
      {"gradients.mixed.id_numerical", P_RES idNumericalGrads},
      {"hessians.mixed.id_analytic", P_RES idAnalyticHessians},
      {"hessians.mixed.id_numerical", P_RES idNumericalHessians},
      {"hessians.mixed.id_quasi", P_RES idQuasiHessians}
    },
    entry_name, dbRep);
}


const IntSetArray& ProblemDescDB::get_isa(const String& entry_name) const
{
  return get<const IntSetArray>
  ( "get_isa()",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"discrete_design_set_int.values", P_VAR discreteDesignSetInt},
      {"discrete_state_set_int.values", P_VAR discreteStateSetInt}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);
}


const SizetSet& ProblemDescDB::get_szs(const String& entry_name) const
{
  return get<const SizetSet>
  ( "get_szs()",
    { /* environment */ },
    { /* method */ },
    { /* model */
      {"surrogate.function_indices", P_MOD surrogateFnIndices}
    },
    { /* variables */ },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);
}


const StringSetArray& ProblemDescDB::get_ssa(const String& entry_name) const
{
  return get <const StringSetArray>
  ( "get_ssa()",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"discrete_design_set_string.values", P_VAR discreteDesignSetStr},
      {"discrete_state_set_string.values", P_VAR discreteStateSetStr}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);
}


const RealSetArray& ProblemDescDB::get_rsa(const String& entry_name) const
{
  return get<const RealSetArray>
  ( "get_rsa()",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"discrete_design_set_real.values", P_VAR discreteDesignSetReal},
      {"discrete_state_set_real.values", P_VAR discreteStateSetReal}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);
}


const IntRealMapArray& ProblemDescDB::get_irma(const String& entry_name) const
{
  return get<const IntRealMapArray>
  ( "get_irma()",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"discrete_uncertain_set_int.values_probs",
    	  P_VAR discreteUncSetIntValuesProbs},
      {"histogram_uncertain.point_int_pairs", P_VAR histogramUncPointIntPairs}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);
}

const StringRealMapArray& ProblemDescDB::get_srma(const String& entry_name) const
{
  return get<const StringRealMapArray>
  ( "get_srma()",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"discrete_uncertain_set_string.values_probs",
    	  P_VAR discreteUncSetStrValuesProbs},
      {"histogram_uncertain.point_string_pairs", P_VAR histogramUncPointStrPairs}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);
}


const RealRealMapArray& ProblemDescDB::get_rrma(const String& entry_name) const
{
  return get<const RealRealMapArray>
  ( "get_rrma()",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"discrete_uncertain_set_real.values_probs",
    	  P_VAR discreteUncSetRealValuesProbs},
      {"histogram_uncertain.bin_pairs",   P_VAR histogramUncBinPairs},
      {"histogram_uncertain.point_real_pairs", P_VAR histogramUncPointRealPairs}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);
}


const RealRealPairRealMapArray& ProblemDescDB::
get_rrrma(const String& entry_name) const
{
  return get<const RealRealPairRealMapArray>
  ( "get_rrrma()",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"continuous_interval_uncertain.basic_probs",
    	  P_VAR continuousIntervalUncBasicProbs}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);
}


const IntIntPairRealMapArray& ProblemDescDB::
get_iirma(const String& entry_name) const
{
  return get<const IntIntPairRealMapArray>
  ( "get_iirma()",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"discrete_interval_uncertain.basic_probs",
    	  P_VAR discreteIntervalUncBasicProbs}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);
}


const StringArray& ProblemDescDB::get_sa(const String& entry_name) const
{
  return get<const StringArray>
  ( "get_sa()",
    { /* environment */ },
    { /* method */
      {"coliny.misc_options", P_MET miscOptions},
      {"hybrid.method_names", P_MET hybridMethodNames},
      {"hybrid.method_pointers", P_MET hybridMethodPointers},
      {"hybrid.model_pointers", P_MET hybridModelPointers}
    },
    { /* model */
      {"metrics", P_MOD diagMetrics},
      {"nested.primary_variable_mapping", P_MOD primaryVarMaps},
      {"nested.secondary_variable_mapping", P_MOD secondaryVarMaps},
      {"surrogate.ensemble_model_pointers", P_MOD ensembleModelPointers}
    },
    { /* variables */
      {"continuous_aleatory_uncertain.labels", P_VAR continuousAleatoryUncLabels},
      {"continuous_design.labels", P_VAR continuousDesignLabels},
      {"continuous_design.scale_types", P_VAR continuousDesignScaleTypes},
      {"continuous_epistemic_uncertain.labels",
    	  P_VAR continuousEpistemicUncLabels},
      {"continuous_state.labels", P_VAR continuousStateLabels},
      {"discrete_aleatory_uncertain_int.labels",
    	  P_VAR discreteIntAleatoryUncLabels},
      {"discrete_aleatory_uncertain_real.labels",
    	  P_VAR discreteRealAleatoryUncLabels},
      {"discrete_aleatory_uncertain_string.initial_point",
    	  P_VAR discreteStrAleatoryUncVars},
      {"discrete_aleatory_uncertain_string.labels",
    	  P_VAR discreteStrAleatoryUncLabels},
      {"discrete_aleatory_uncertain_string.lower_bounds",
    	  P_VAR discreteStrAleatoryUncLowerBnds},
      {"discrete_aleatory_uncertain_string.upper_bounds",
    	  P_VAR discreteStrAleatoryUncUpperBnds},
      {"discrete_design_range.labels", P_VAR discreteDesignRangeLabels},
      {"discrete_design_set_int.labels", P_VAR discreteDesignSetIntLabels},
      {"discrete_design_set_real.labels", P_VAR discreteDesignSetRealLabels},
      {"discrete_design_set_string.initial_point", P_VAR discreteDesignSetStrVars},
      {"discrete_design_set_string.labels", P_VAR discreteDesignSetStrLabels},
      {"discrete_design_set_string.lower_bounds", P_VAR discreteDesignSetStrLowerBnds},
      {"discrete_design_set_string.upper_bounds", P_VAR discreteDesignSetStrUpperBnds},
      {"discrete_epistemic_uncertain_int.labels",
    	  P_VAR discreteIntEpistemicUncLabels},
      {"discrete_epistemic_uncertain_real.labels",
    	  P_VAR discreteRealEpistemicUncLabels},
      {"discrete_epistemic_uncertain_string.initial_point",
    	  P_VAR discreteStrEpistemicUncVars},
      {"discrete_epistemic_uncertain_string.labels",
    	  P_VAR discreteStrEpistemicUncLabels},
      {"discrete_epistemic_uncertain_string.lower_bounds",
    	  P_VAR discreteStrEpistemicUncLowerBnds},
      {"discrete_epistemic_uncertain_string.upper_bounds",
    	  P_VAR discreteStrEpistemicUncUpperBnds},
      {"discrete_state_range.labels", P_VAR discreteStateRangeLabels},
      {"discrete_state_set_int.labels", P_VAR discreteStateSetIntLabels},
      {"discrete_state_set_real.labels", P_VAR discreteStateSetRealLabels},
      {"discrete_state_set_string.initial_state", P_VAR discreteStateSetStrVars},
      {"discrete_state_set_string.labels", P_VAR discreteStateSetStrLabels},
      {"discrete_state_set_string.lower_bounds", P_VAR discreteStateSetStrLowerBnds},
      {"discrete_state_set_string.upper_bounds", P_VAR discreteStateSetStrUpperBnds},
      {"discrete_uncertain_set_string.initial_point", P_VAR discreteUncSetStrVars},
      {"linear_equality_scale_types", P_VAR linearEqScaleTypes},
      {"linear_inequality_scale_types", P_VAR linearIneqScaleTypes}
    },
    { /* interface */
      { "application.analysis_drivers", P_INT analysisDrivers},
      { "copyFiles", P_INT copyFiles},
      { "linkFiles", P_INT linkFiles}
    },
    { /* responses */
      { "labels", P_RES responseLabels},
      { "metadata_labels", P_RES metadataLabels},
      { "nonlinear_equality_scale_types", P_RES nonlinearEqScaleTypes},
      { "nonlinear_inequality_scale_types", P_RES nonlinearIneqScaleTypes},
      { "primary_response_fn_scale_types", P_RES primaryRespFnScaleTypes},
      { "primary_response_fn_sense", P_RES primaryRespFnSense},
      { "variance_type", P_RES varianceType}
    },
    entry_name, dbRep);
}


const String2DArray& ProblemDescDB::get_s2a(const String& entry_name) const
{
  return get<const String2DArray>
  ( "get_s2a()",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */ },
    { /* interface */
      {"application.analysis_components", P_INT analysisComponents}
    },
    { /* responses */ },
    entry_name, dbRep);
}


const String& ProblemDescDB::get_string(const String& entry_name) const
{
  return get<const String>
  ( "get_string()",
    { /* environment */
      {"error_file", P_ENV errorFile},
      {"output_file", P_ENV outputFile},
      {"post_run_input", P_ENV postRunInput},
      {"post_run_output", P_ENV postRunOutput},
      {"pre_run_input", P_ENV preRunInput},
      {"pre_run_output", P_ENV preRunOutput},
      {"read_restart", P_ENV readRestart},
      {"results_output_file", P_ENV resultsOutputFile},
      {"run_input", P_ENV runInput},
      {"run_output", P_ENV runOutput},
      {"tabular_graphics_file", P_ENV tabularDataFile},
      {"top_method_pointer", P_ENV topMethodPointer},
      {"write_restart", P_ENV writeRestart}
    },
    { /* method */
      {"advanced_options_file", P_MET advancedOptionsFilename},
      {"asynch_pattern_search.merit_function", P_MET meritFunction},
      {"batch_selection", P_MET batchSelectionType},
      {"coliny.beta_solver_name", P_MET betaSolverName},
      {"coliny.division", P_MET boxDivision},
      {"coliny.exploratory_moves", P_MET exploratoryMoves},
      {"coliny.pattern_basis", P_MET patternBasis},
      {"crossover_type", P_MET crossoverType},
      {"dl_solver.dlDetails", P_MET dlDetails},
      {"export_approx_points_file", P_MET exportApproxPtsFile},
      {"fitness_metric", P_MET fitnessMetricType},
      {"fitness_type", P_MET fitnessType},
      {"flat_file", P_MET flatFile},
      {"hybrid.global_method_name", P_MET hybridGlobalMethodName},
      {"hybrid.global_method_pointer", P_MET hybridGlobalMethodPointer},
      {"hybrid.global_model_pointer", P_MET hybridGlobalModelPointer},
      {"hybrid.local_method_name", P_MET hybridLocalMethodName},
      {"hybrid.local_method_pointer", P_MET hybridLocalMethodPointer},
      {"hybrid.local_model_pointer", P_MET hybridLocalModelPointer},
      {"id", P_MET idMethod},
      {"import_approx_points_file", P_MET importApproxPtsFile},
      {"import_build_points_file", P_MET importBuildPtsFile},
      {"import_candidate_points_file", P_MET importCandPtsFile},
      {"import_prediction_configs", P_MET importPredConfigs},
      {"initialization_type", P_MET initializationType},
      {"jega.convergence_type", P_MET convergenceType},
      {"jega.niching_type", P_MET nichingType},
      {"jega.postprocessor_type", P_MET postProcessorType},
      {"lipschitz", P_MET lipschitzType},
      {"log_file", P_MET logFile},
      {"low_fidelity_model_pointer", P_MET lowFidModelPointer},
      {"mesh_adaptive_search.display_format", P_MET displayFormat},
      {"mesh_adaptive_search.history_file", P_MET historyFile},
      {"mesh_adaptive_search.use_surrogate", P_MET useSurrogate},
      {"model_export_prefix", P_MET modelExportPrefix},
      {"model_pointer", P_MET modelPointer},
      {"mutation_type", P_MET mutationType},
      {"nond.data_dist_cov_type", P_MET dataDistCovInputType},
      {"nond.data_dist_filename", P_MET dataDistFile},
      {"nond.data_dist_type", P_MET dataDistType},
      {"nond.dili_hessian_type", P_MET diliHessianType},
      {"nond.discrepancy_type", P_MET modelDiscrepancyType},
    //{"nond.expansion_sample_type", P_MET expansionSampleType},
      {"nond.dr_scale_type", P_MET drScaleType},
      {"nond.export_corrected_model_file", P_MET exportCorrModelFile},
      {"nond.export_corrected_variance_file", P_MET exportCorrVarFile},
      {"nond.export_discrepancy_file", P_MET exportDiscrepFile},
      {"nond.export_expansion_file", P_MET exportExpansionFile},
      {"nond.export_mcmc_points_file", P_MET exportMCMCPtsFile},
      {"nond.import_expansion_file", P_MET importExpansionFile},
      {"nond.mcmc_type", P_MET mcmcType},
      {"nond.point_reuse", P_MET pointReuse},
      {"nond.posterior_density_export_file", P_MET posteriorDensityExportFilename},
      {"nond.posterior_samples_export_file", P_MET posteriorSamplesExportFilename},
      {"nond.posterior_samples_import_file", P_MET posteriorSamplesImportFilename},
      {"nond.proposal_covariance_filename", P_MET proposalCovFile},
      {"nond.proposal_covariance_input_type", P_MET proposalCovInputType},
      {"nond.proposal_covariance_type", P_MET proposalCovType},
      {"nond.reliability_integration", P_MET reliabilityIntegration},
      {"optpp.search_method", P_MET searchMethod},
      {"pstudy.import_file", P_MET pstudyFilename},
      {"random_number_generator", P_MET rngName},
      {"replacement_type", P_MET replacementType},
      {"sub_method_name", P_MET subMethodName},
      {"sub_method_pointer", P_MET subMethodPointer},
      {"sub_model_pointer", P_MET subModelPointer},
      {"trial_type", P_MET trialType},
      {"generating_vector.file", P_MET generatingVectorFileName},
      {"generating_matrices.file", P_MET generatingMatricesFileName}
    },
    { /* model */
      {"advanced_options_file", P_MOD advancedOptionsFilename},
      {"dace_method_pointer", P_MOD subMethodPointer},
      {"id", P_MOD idModel},
      {"interface_pointer", P_MOD interfacePointer},
      {"nested.sub_method_pointer", P_MOD subMethodPointer},
      {"optional_interface_responses_pointer", P_MOD optionalInterfRespPointer},
      {"rf.propagation_model_pointer", P_MOD propagationModelPointer},
      {"rf_data_file", P_MOD rfDataFileName},
      {"simulation.cost_recovery_metadata", P_MOD costRecoveryMetadata},
      {"simulation.solution_level_control", P_MOD solutionLevelControl},
      {"surrogate.truth_model_pointer", P_MOD truthModelPointer},
      {"surrogate.challenge_points_file", P_MOD importChallengePtsFile},
      {"surrogate.decomp_cell_type", P_MOD decompCellType},
      {"surrogate.export_approx_points_file", P_MOD exportApproxPtsFile},
      {"surrogate.export_approx_variance_file", P_MOD exportApproxVarianceFile},
      {"surrogate.import_build_points_file", P_MOD importBuildPtsFile},
      {"surrogate.kriging_opt_method", P_MOD krigingOptMethod},
      {"surrogate.mars_interpolation", P_MOD marsInterpolation},
      {"surrogate.model_export_prefix", P_MOD modelExportPrefix},
      {"surrogate.model_import_prefix", P_MOD modelImportPrefix},
      {"surrogate.class_path_and_name", P_MOD moduleAndClassName},
      {"surrogate.point_reuse", P_MOD approxPointReuse},
      {"surrogate.refine_cv_metric", P_MOD refineCVMetric},
      {"surrogate.trend_order", P_MOD trendOrder},
      {"surrogate.type", P_MOD surrogateType},
      {"type", P_MOD modelType}
    },
    { /* variables */
      {"id", P_VAR idVariables}
    },
    { /* interface */
      {"algebraic_mappings", P_INT algebraicMappings},
      {"application.input_filter", P_INT inputFilter},
      {"application.output_filter", P_INT outputFilter},
      {"application.parameters_file", P_INT parametersFile},
      {"application.results_file", P_INT resultsFile},
      {"failure_capture.action", P_INT failAction},
      {"id", P_INT idInterface},
      {"plugin_library_path", P_INT pluginLibraryPath},
      {"workDir", P_INT workDir}
    },
    { /* responses */
      {"data_directory", P_RES dataPathPrefix},
      {"fd_gradient_step_type", P_RES fdGradStepType},
      {"fd_hessian_step_type", P_RES fdHessStepType},
      {"gradient_type", P_RES gradientType},
      {"hessian_type", P_RES hessianType},
      {"id", P_RES idResponses},
      {"interval_type", P_RES intervalType},
      {"method_source", P_RES methodSource},
      {"quasi_hessian_type", P_RES quasiHessianType},
      {"scalar_data_filename", P_RES scalarDataFileName}
    },
    entry_name, dbRep);
}


const Real& ProblemDescDB::get_real(const String& entry_name) const
{
  return get<const Real>
  ( "get_real()",
    { /* environment */ },
    { /* method */
      {"asynch_pattern_search.constraint_penalty", P_MET constrPenalty},
      {"asynch_pattern_search.contraction_factor", P_MET contractStepLength},
      {"asynch_pattern_search.initial_delta", P_MET initStepLength},
      {"asynch_pattern_search.smoothing_factor", P_MET smoothFactor},
      {"asynch_pattern_search.solution_target", P_MET solnTarget},
      {"coliny.contraction_factor", P_MET contractFactor},
      {"coliny.global_balance_parameter", P_MET globalBalanceParam},
      {"coliny.initial_delta", P_MET initDelta},
      {"coliny.local_balance_parameter", P_MET localBalanceParam},
      {"coliny.max_boxsize_limit", P_MET maxBoxSize},
      {"coliny.variable_tolerance", P_MET threshDelta},
      {"confidence_level", P_MET wilksConfidenceLevel},
      {"constraint_penalty", P_MET constraintPenalty},
      {"constraint_tolerance", P_MET constraintTolerance},
      {"convergence_tolerance", P_MET convergenceTolerance},
      {"crossover_rate", P_MET crossoverRate},
      {"dream.gr_threshold", P_MET grThreshold},
      {"function_precision", P_MET functionPrecision},
      {"gradient_tolerance", P_MET gradientTolerance},
      {"hybrid.local_search_probability", P_MET hybridLSProb},
      {"jega.fitness_limit", P_MET fitnessLimit},
      {"jega.percent_change", P_MET convergenceTolerance},
      {"jega.shrinkage_percentage", P_MET shrinkagePercent},
      {"mesh_adaptive_search.initial_delta", P_MET initMeshSize},
      {"mesh_adaptive_search.variable_neighborhood_search", P_MET vns},
      {"mesh_adaptive_search.variable_tolerance", P_MET minMeshSize},
      {"min_boxsize_limit", P_MET minBoxSize},
      {"mutation_rate", P_MET mutationRate},
      {"mutation_scale", P_MET mutationScale},
      {"nl2sol.absolute_conv_tol", P_MET absConvTol},
      {"nl2sol.false_conv_tol", P_MET falseConvTol},
      {"nl2sol.initial_trust_radius", P_MET initTRRadius},
      {"nl2sol.singular_conv_tol", P_MET singConvTol},
      {"nl2sol.singular_radius", P_MET singRadius},
      {"nond.am_scale", P_MET amScale},
      {"nond.c3function_train.solver_rounding_tolerance",
       P_MET solverRoundingTol},
      {"nond.c3function_train.solver_tolerance", P_MET solverTol},
      {"nond.c3function_train.stats_rounding_tolerance",
       P_MET statsRoundingTol},
      {"nond.collocation_ratio", P_MET collocationRatio},
      {"nond.collocation_ratio_terms_order", P_MET collocRatioTermsOrder},
      {"nond.dili_hess_tolerance", P_MET diliHessTolerance},
      {"nond.dili_lis_tolerance", P_MET diliLISTolerance},
      {"nond.dili_ses_abs_tol", P_MET diliSesAbsTol},
      {"nond.dili_ses_rel_tol", P_MET diliSesRelTol},
      {"nond.dr_scale", P_MET drScale},
      {"nond.estimator_variance_metric_norm_order",
       P_MET estVarMetricNormOrder},
      {"nond.mala_step_size", P_MET malaStepSize},
      {"nond.multilevel_estimator_rate", P_MET multilevEstimatorRate},
      {"nond.rcond_tol_throttle", P_MET rCondTolThrottle},
      {"nond.regression_penalty", P_MET regressionL2Penalty},
      {"nond.relaxation.fixed_factor", P_MET relaxFixedFactor},
      {"nond.relaxation.recursive_factor", P_MET relaxRecursiveFactor},
      {"npsol.linesearch_tolerance", P_MET lineSearchTolerance},
      {"optpp.centering_parameter", P_MET centeringParam},
      {"optpp.max_step", P_MET maxStep},
      {"optpp.steplength_to_boundary", P_MET stepLenToBoundary},
      {"percent_variance_explained", P_MET percentVarianceExplained},
      {"prior_prop_cov_mult", P_MET priorPropCovMult},
      {"solution_target", P_MET solnTarget},
      {"ti_coverage", P_MET tiCoverage},
      {"ti_confidence_level", P_MET tiConfidenceLevel},
      {"trust_region.contract_threshold", P_MET trustRegionContractTrigger},
      {"trust_region.contraction_factor", P_MET trustRegionContract},
      {"trust_region.expand_threshold", P_MET trustRegionExpandTrigger},
      {"trust_region.expansion_factor", P_MET trustRegionExpand},
      {"trust_region.minimum_size", P_MET trustRegionMinSize},
      {"variable_tolerance", P_MET threshStepLength},
      {"vbd_drop_tolerance", P_MET vbdDropTolerance},
      {"verification.refinement_rate", P_MET refinementRate},
      {"volume_boxsize_limit", P_MET volBoxSize},
      {"x_conv_tol", P_MET xConvTol}
    },
    { /* model */
      {"active_subspace.cv.decrease_tolerance", P_MOD decreaseTolerance},
      {"active_subspace.cv.relative_tolerance", P_MOD relTolerance},
      {"active_subspace.truncation_method.energy.truncation_tolerance",
    	  P_MOD truncationTolerance},
      {"adapted_basis.collocation_ratio", P_MOD adaptedBasisCollocRatio},
      {"adapted_basis.truncation_tolerance", P_MOD adaptedBasisTruncationTolerance},
      {"c3function_train.collocation_ratio", P_MOD collocationRatio},
      {"c3function_train.solver_rounding_tolerance", P_MOD solverRoundingTol},
      {"c3function_train.solver_tolerance", P_MOD solverTol},
      {"c3function_train.stats_rounding_tolerance", P_MOD statsRoundingTol},
      {"convergence_tolerance", P_MOD convergenceTolerance},
      {"surrogate.discont_grad_thresh", P_MOD discontGradThresh},
      {"surrogate.discont_jump_thresh", P_MOD discontJumpThresh},
      {"surrogate.neural_network_range", P_MOD annRange},
      {"surrogate.nugget", P_MOD krigingNugget},
      {"surrogate.percent", P_MOD percentFold},
      {"surrogate.regression_penalty", P_MOD regressionL2Penalty},
      {"truncation_tolerance", P_MOD truncationTolerance}
    },
    { /* variables */ },
    { /* interface */
      {"nearby_evaluation_cache_tolerance", P_INT nearbyEvalCacheTol}
    },
    { /* responses */ },
    entry_name, dbRep);
}


int ProblemDescDB::get_int(const String& entry_name) const
{
  return get<int>
  ( "get_int()",
    { /* environment */
      {"output_precision", P_ENV outputPrecision},
      {"stop_restart", P_ENV stopRestart}
    },
    { /* method */
      {"batch_size", P_MET batchSize},
      {"batch_size.exploration", P_MET batchSizeExplore},
      {"build_samples", P_MET buildSamples},
      {"burn_in_samples", P_MET burnInSamples},
      {"coliny.contract_after_failure", P_MET contractAfterFail},
      {"coliny.expand_after_success", P_MET expandAfterSuccess},
      {"coliny.mutation_range", P_MET mutationRange},
      {"coliny.new_solutions_generated", P_MET newSolnsGenerated},
      {"coliny.number_retained", P_MET numberRetained},
      {"coliny.total_pattern_size", P_MET totalPatternSize},
      {"concurrent.random_jobs", P_MET concurrentRandomJobs},
      {"dream.crossover_chain_pairs", P_MET crossoverChainPairs},
      {"dream.jump_step", P_MET jumpStep},
      {"dream.num_chains", P_MET numChains},
      {"dream.num_cr", P_MET numCR},
      {"evidence_samples", P_MET evidenceSamples},
      {"fsu_cvt.num_trials", P_MET numTrials},
      {"iterator_servers", P_MET iteratorServers},
      {"max_hifi_evaluations", P_MET maxHifiEvals},
      {"mesh_adaptive_search.neighbor_order", P_MET neighborOrder},
      {"nl2sol.covariance", P_MET covarianceType},
      {"nond.am_period_num_steps", P_MET amPeriodNumSteps},
      {"nond.am_starting_step", P_MET amStartingStep},
      {"nond.c3function_train.max_cross_iterations", P_MET maxCrossIterations},
      {"nond.chain_samples", P_MET chainSamples},
      {"nond.dili_adapt_end", P_MET diliAdaptEnd},
      {"nond.dili_adapt_interval", P_MET diliAdaptInterval},
      {"nond.dili_adapt_start", P_MET diliAdaptStart},
      {"nond.dili_initial_weight", P_MET diliInitialWeight},
      {"nond.dili_ses_block_size", P_MET diliSesBlockSize},
      {"nond.dili_ses_exp_rank", P_MET diliSesExpRank},
      {"nond.dili_ses_num_eigs", P_MET diliSesNumEigs},
      {"nond.dili_ses_overs_factor", P_MET diliSesOversFactor},
      {"nond.dr_num_stages", P_MET drNumStages},
      {"nond.prop_cov_update_period", P_MET proposalCovUpdatePeriod},
      {"nond.pushforward_samples", P_MET numPushforwardSamples},
      {"nond.samples_on_emulator", P_MET samplesOnEmulator},
      {"nond.surrogate_order", P_MET emulatorOrder},
      {"npsol.verify_level", P_MET verifyLevel},
      {"optpp.search_scheme_size", P_MET searchSchemeSize},
      {"parameter_study.num_steps", P_MET numSteps},
      {"population_size", P_MET populationSize},
      {"processors_per_iterator", P_MET procsPerIterator},
      {"random_seed", P_MET randomSeed},
      {"samples", P_MET numSamples},
      {"sub_sampling_period", P_MET subSamplingPeriod},
      {"symbols", P_MET numSymbols},
      {"vbd_via_sampling_num_bins", P_MET vbdViaSamplingNumBins},
      {"m_max", P_MET log2MaxPoints},
      {"t_max", P_MET numberOfBits},
      {"t_scramble", P_MET scrambleSize}
    },
    { /* model */
      {"active_subspace.bootstrap_samples", P_MOD numReplicates},
      {"active_subspace.cv.max_rank", P_MOD subspaceCVMaxRank},
      {"c3function_train.max_cross_iterations", P_MOD maxCrossIterations},
      {"initial_samples", P_MOD initialSamples},
      {"nested.iterator_servers", P_MOD subMethodServers},
      {"nested.processors_per_iterator", P_MOD subMethodProcs},
      {"rf.expansion_bases", P_MOD subspaceDimension},
      {"soft_convergence_limit", P_MOD softConvergenceLimit},
      {"subspace.dimension", P_MOD subspaceDimension},
      {"surrogate.decomp_support_layers", P_MOD decompSupportLayers},
      {"surrogate.folds", P_MOD numFolds},
      {"surrogate.num_restarts", P_MOD numRestarts},
      {"surrogate.points_total", P_MOD pointsTotal},
      {"surrogate.refine_cv_folds", P_MOD refineCVFolds}
    },
    { /* variables */ },
    { /* interface */
      {"analysis_servers", P_INT analysisServers},
      {"asynch_local_analysis_concurrency", P_INT asynchLocalAnalysisConcurrency},
      {"asynch_local_evaluation_concurrency", P_INT asynchLocalEvalConcurrency},
      {"direct.processors_per_analysis", P_INT procsPerAnalysis},
      {"evaluation_servers", P_INT evalServers},
      {"failure_capture.retry_limit", P_INT retryLimit},
      {"processors_per_evaluation", P_INT procsPerEval}
    },
    { /* responses */ },
    entry_name, dbRep);
}


short ProblemDescDB::get_short(const String& entry_name) const
{
  return get<short>
  ( "get_short()",
    { /* environment */ },
    { /* method */
      {"iterator_scheduling", P_MET iteratorScheduling},
      {"nond.allocation_target", P_MET allocationTarget},
      {"nond.c3function_train.advancement_type", P_MET c3AdvanceType},
      {"nond.convergence_tolerance_target", P_MET convergenceToleranceTarget},
      {"nond.convergence_tolerance_type", P_MET convergenceToleranceType},
      {"nond.covariance_control", P_MET covarianceControl},
      {"nond.distribution", P_MET distributionType},
      {"nond.emulator", P_MET emulatorType},
      {"nond.ensemble_pilot_solution_mode", P_MET ensemblePilotSolnMode},
      {"nond.estimator_variance_metric", P_MET estVarMetricType},
      {"nond.expansion_basis_type", P_MET expansionBasisType},
      {"nond.expansion_refinement_control", P_MET refinementControl},
      {"nond.expansion_refinement_type", P_MET refinementType},
      {"nond.expansion_type", P_MET expansionType},
      {"nond.final_moments", P_MET finalMomentsType},
      {"nond.final_statistics", P_MET finalStatsType},
      {"nond.group_throttle_type", P_MET groupThrottleType},
      {"nond.growth_override", P_MET growthOverride},
      {"nond.least_squares_regression_type", P_MET lsRegressionType},
      {"nond.model_discrepancy.polynomial_order", P_MET polynomialOrder},
      {"nond.multilevel_allocation_control", P_MET multilevAllocControl},
      {"nond.multilevel_discrepancy_emulation", P_MET multilevDiscrepEmulation},
      {"nond.nesting_override", P_MET nestingOverride},
      {"nond.pilot_samples.mode", P_MET pilotGroupSampling},
      {"nond.qoi_aggregation", P_MET qoiAggregation},
      {"nond.refinement_statistics_mode", P_MET statsMetricMode},
      {"nond.regression_type", P_MET regressionType},
      {"nond.response_level_target", P_MET responseLevelTarget},
      {"nond.response_level_target_reduce", P_MET responseLevelTargetReduce},
      {"nond.search_model_graphs.recursion", P_MET dagRecursionType},
      {"nond.search_model_graphs.selection", P_MET modelSelectType},
      {"optpp.merit_function", P_MET meritFn},
      {"output", P_MET methodOutput},
      {"sbl.acceptance_logic", P_MET surrBasedLocalAcceptLogic},
      {"sbl.constraint_relax", P_MET surrBasedLocalConstrRelax},
      {"sbl.merit_function", P_MET surrBasedLocalMeritFn},
      {"sbl.subproblem_constraints", P_MET surrBasedLocalSubProbCon},
      {"sbl.subproblem_objective", P_MET surrBasedLocalSubProbObj},
      {"synchronization", P_MET evalSynchronize},
      {"wilks.sided_interval", P_MET wilksSidedInterval}
    },
    { /* model */
      {"adapted_basis.rotation_method", P_MOD method_rotation},
      {"c3function_train.advancement_type", P_MOD c3AdvanceType},
      //{"c3function_train.refinement_control", P_MOD refinementControl},
      //{"c3function_train.refinement_type", P_MOD refinementType},
      {"nested.iterator_scheduling", P_MOD subMethodScheduling},
      {"surrogate.correction_order", P_MOD approxCorrectionOrder},
      {"surrogate.correction_type", P_MOD approxCorrectionType},
      {"surrogate.find_nugget", P_MOD krigingFindNugget},
      {"surrogate.kriging_max_trials", P_MOD krigingMaxTrials},
      {"surrogate.mars_max_bases", P_MOD marsMaxBases},
      {"surrogate.mls_weight_function", P_MOD mlsWeightFunction},
      {"surrogate.neural_network_nodes", P_MOD annNodes},
      {"surrogate.neural_network_random_weight", P_MOD annRandomWeight},
      {"surrogate.points_management", P_MOD pointsManagement},
      {"surrogate.polynomial_order", P_MOD polynomialOrder},
      {"surrogate.rbf_bases", P_MOD rbfBases},
      {"surrogate.rbf_max_pts", P_MOD rbfMaxPts},
      {"surrogate.rbf_max_subsets", P_MOD rbfMaxSubsets},
      {"surrogate.rbf_min_partition", P_MOD rbfMinPartition},
      {"surrogate.regression_type", P_MOD regressionType}
    },
    { /* variables */
      {"domain", P_VAR varsDomain},
      {"view", P_VAR varsView}
    },
    { /* interface */
      {"analysis_scheduling", P_INT analysisScheduling},
      {"evaluation_scheduling", P_INT evalScheduling},
      {"local_evaluation_scheduling", P_INT asynchLocalEvalScheduling}
    },
    { /* responses */},
    entry_name, dbRep);
}


unsigned short ProblemDescDB::get_ushort(const String& entry_name) const
{
  return get<unsigned short>
  ( "get_ushort()",
    { /* environment */
      {"interface_evals_selection", P_ENV interfEvalsSelection},
      {"model_evals_selection", P_ENV modelEvalsSelection},
      {"post_run_input_format", P_ENV postRunInputFormat},
      {"pre_run_output_format", P_ENV preRunOutputFormat},
      {"results_output_format", P_ENV resultsOutputFormat},
      {"tabular_format", P_ENV tabularFormat}
    },
    { /* method */
      {"algorithm", P_MET methodName},
      {"export_approx_format", P_MET exportApproxFormat},
      {"import_approx_format", P_MET importApproxFormat},
      {"import_build_format", P_MET importBuildFormat},
      {"import_candidate_format", P_MET importCandFormat},
      {"import_prediction_configs_format", P_MET importPredConfigFormat},
      {"model_export_format", P_MET modelExportFormat},
      {"nond.adapted_basis.advancements", P_MET adaptedBasisAdvancements},
      //{"nond.adapted_basis.initial_level", P_MET adaptedBasisInitLevel},
      {"nond.c3function_train.kick_order", P_MET kickOrder},
      {"nond.c3function_train.max_order", P_MET maxOrder},
      {"nond.c3function_train.start_order", P_MET startOrder},
      {"nond.calibrate_error_mode", P_MET calibrateErrorMode},
      {"nond.cross_validation.max_order_candidates",
       P_MET maxCVOrderCandidates},
      {"nond.cubature_integrand", P_MET cubIntOrder},
      {"nond.expansion_order", P_MET expansionOrder},
      {"nond.export_corrected_model_format", P_MET exportCorrModelFormat},
      {"nond.export_corrected_variance_format", P_MET exportCorrVarFormat},
      {"nond.export_discrep_format", P_MET exportDiscrepFormat},
      {"nond.export_samples_format", P_MET exportSamplesFormat},
      {"nond.graph_depth_limit", P_MET dagDepthLimit},
      {"nond.group_size_throttle", P_MET groupSizeThrottle},
      {"nond.integration_refinement", P_MET integrationRefine},
      {"nond.numerical_solve_mode", P_MET numericalSolveMode},
      {"nond.opt_subproblem_solver", P_MET optSubProbSolver},
      {"nond.quadrature_order", P_MET quadratureOrder},
    //{"nond.reliability_search_type", P_MET reliabilitySearchType},
      {"nond.sparse_grid_level", P_MET sparseGridLevel},
      {"nond.vbd_interaction_order", P_MET vbdOrder},
      {"order", P_MET wilksOrder},
      {"pstudy.import_format", P_MET pstudyFileFormat},
      {"sample_type", P_MET sampleType},
      {"soft_convergence_limit", P_MET softConvLimit},
      {"sub_method", P_MET subMethod},
      {"vbd_via_sampling_method", P_MET vbdViaSamplingMethod}
    },
    { /* model */
      {"active_subspace.cv.id_method", P_MOD subspaceIdCVMethod},
      {"active_subspace.normalization", P_MOD subspaceNormalization},
      {"active_subspace.sample_type", P_MOD subspaceSampleType},
      {"adapted_basis.expansion_order", P_MOD adaptedBasisExpOrder},
      {"adapted_basis.sparse_grid_level", P_MOD adaptedBasisSparseGridLev},
      {"c3function_train.cross_validation.max_order_candidates",
       P_MOD maxCVOrderCandidates},
      {"c3function_train.kick_order", P_MOD kickOrder},
      {"c3function_train.max_order", P_MOD maxOrder},
      {"c3function_train.start_order", P_MOD startOrder},
      {"rf.analytic_covariance", P_MOD analyticCovIdForm},
      {"rf.expansion_form", P_MOD randomFieldIdForm},
      {"surrogate.challenge_points_file_format", P_MOD importChallengeFormat},
      {"surrogate.export_approx_format", P_MOD exportApproxFormat},
      {"surrogate.export_approx_variance_format", P_MOD exportApproxVarianceFormat},
      {"surrogate.import_build_format", P_MOD importBuildFormat},
      {"surrogate.model_export_format", P_MOD modelExportFormat},
      {"surrogate.model_import_format", P_MOD modelImportFormat}
    },
    { /* variables */ },
    { /* interface */
      {"application.parameters_file_format", P_INT parametersFileFormat},
      {"application.results_file_format", P_INT resultsFileFormat},
      {"type", P_INT interfaceType}
    },
    { /* responses */
      {"scalar_data_format", P_RES scalarDataFormat}
    },
    entry_name, dbRep);
}


size_t ProblemDescDB::get_sizet(const String& entry_name) const
{
  // first handle special case for variable group queries

  std::string block, entry;
  std::tie(block, entry) = split_entry_name(entry_name, "get_sizet");

  if (block == "variables") {
    if (!dbRep)
      Null_rep("get_sizet()");
    if (dbRep->variablesDBLocked)
      Locked_db();

    // string for lookup key without the leading "variables."
    auto v_iter = dbRep->dataVariablesIter;
    if (entry == "aleatory_uncertain")
      return v_iter->aleatory_uncertain();
    else if (entry == "continuous")
      return v_iter->continuous_variables();
    else if (entry == "design")
      return v_iter->design();
    else if (entry == "discrete")
      return v_iter->discrete_variables();
    else if (entry == "epistemic_uncertain")
      return v_iter->epistemic_uncertain();
    else if (entry == "state")
      return v_iter->state();
    else if (entry == "total")
      return v_iter->total_variables();
    else if (entry == "uncertain")
      return v_iter->uncertain();
    // else fall through to normal queries
  }

  return get<size_t>
  ( "get_sizet()",
    { /* environment */ },
    { /* method */
      {"final_solutions", P_MET numFinalSolutions},
      {"jega.num_cross_points", P_MET numCrossPoints},
      {"jega.num_designs", P_MET numDesigns},
      {"jega.num_generations", P_MET numGenerations},
      {"jega.num_offspring", P_MET numOffspring},
      {"jega.num_parents", P_MET numParents},
      {"max_function_evaluations", P_MET maxFunctionEvals},
      {"max_iterations", P_MET maxIterations},
      {"nond.c3function_train.kick_rank", P_MET kickRank},
      {"nond.c3function_train.max_rank", P_MET maxRank},
      {"nond.c3function_train.start_rank", P_MET startRank},
      {"nond.collocation_points", P_MET collocationPoints},
      {"nond.cross_validation.max_rank_candidates", P_MET maxCVRankCandidates},
      {"nond.expansion_samples", P_MET expansionSamples},
      {"nond.max_refinement_iterations", P_MET maxRefineIterations},
      {"nond.max_solver_iterations", P_MET maxSolverIterations},
      {"nond.rcond_best_throttle", P_MET rCondBestThrottle},
      {"num_candidate_designs", P_MET numCandidateDesigns},
      {"num_candidates", P_MET numCandidates},
      {"num_prediction_configs", P_MET numPredConfigs}
    },
    { /* model */
      {"c3function_train.collocation_points", P_MOD collocationPoints},
      {"c3function_train.cross_validation.max_rank_candidates",
       P_MOD maxCVRankCandidates},
      {"c3function_train.kick_rank", P_MOD kickRank},
      {"c3function_train.max_rank", P_MOD maxRank},
      {"c3function_train.start_rank", P_MOD startRank},
      //{"c3function_train.verbosity", P_MOD verbosity},
      {"max_function_evals", P_MOD maxFunctionEvals},
      {"max_iterations", P_MOD maxIterations},
      {"max_solver_iterations", P_MOD maxSolverIterations}//,
    },
    { /* variables */
      {"beta_uncertain", P_VAR numBetaUncVars},
      {"binomial_uncertain", P_VAR numBinomialUncVars},
      {"continuous_design", P_VAR numContinuousDesVars},
      {"continuous_interval_uncertain", P_VAR numContinuousIntervalUncVars},
      {"continuous_state", P_VAR numContinuousStateVars},
      {"discrete_design_range", P_VAR numDiscreteDesRangeVars},
      {"discrete_design_set_int", P_VAR numDiscreteDesSetIntVars},
      {"discrete_design_set_real", P_VAR numDiscreteDesSetRealVars},
      {"discrete_design_set_string", P_VAR numDiscreteDesSetStrVars},
      {"discrete_interval_uncertain", P_VAR numDiscreteIntervalUncVars},
      {"discrete_state_range", P_VAR numDiscreteStateRangeVars},
      {"discrete_state_set_int", P_VAR numDiscreteStateSetIntVars},
      {"discrete_state_set_real", P_VAR numDiscreteStateSetRealVars},
      {"discrete_state_set_string", P_VAR numDiscreteStateSetStrVars},
      {"discrete_uncertain_set_int", P_VAR numDiscreteUncSetIntVars},
      {"discrete_uncertain_set_real", P_VAR numDiscreteUncSetRealVars},
      {"discrete_uncertain_set_string", P_VAR numDiscreteUncSetStrVars},
      {"exponential_uncertain", P_VAR numExponentialUncVars},
      {"frechet_uncertain", P_VAR numFrechetUncVars},
      {"gamma_uncertain", P_VAR numGammaUncVars},
      {"geometric_uncertain", P_VAR numGeometricUncVars},
      {"gumbel_uncertain", P_VAR numGumbelUncVars},
      {"histogram_uncertain.bin", P_VAR numHistogramBinUncVars},
      {"histogram_uncertain.point_int", P_VAR numHistogramPtIntUncVars},
      {"histogram_uncertain.point_real", P_VAR numHistogramPtRealUncVars},
      {"histogram_uncertain.point_string", P_VAR numHistogramPtStrUncVars},
      {"hypergeometric_uncertain", P_VAR numHyperGeomUncVars},
      {"lognormal_uncertain", P_VAR numLognormalUncVars},
      {"loguniform_uncertain", P_VAR numLoguniformUncVars},
      {"negative_binomial_uncertain", P_VAR numNegBinomialUncVars},
      {"normal_uncertain", P_VAR numNormalUncVars},
      {"poisson_uncertain", P_VAR numPoissonUncVars},
      {"triangular_uncertain", P_VAR numTriangularUncVars},
      {"uniform_uncertain", P_VAR numUniformUncVars},
      {"weibull_uncertain", P_VAR numWeibullUncVars}
    },
    { /* interface */ },
    { /* responses */
      {"num_calibration_terms", P_RES numLeastSqTerms},
      {"num_config_vars", P_RES numExpConfigVars},
      {"num_experiments", P_RES numExperiments},
      {"num_field_calibration_terms", P_RES numFieldLeastSqTerms},
      {"num_field_nonlinear_equality_constraints",
	  P_RES numFieldNonlinearEqConstraints},
      {"num_field_nonlinear_inequality_constraints",
	  P_RES numFieldNonlinearIneqConstraints},
      {"num_field_objectives", P_RES numFieldObjectiveFunctions},
      {"num_field_responses", P_RES numFieldResponseFunctions},
      {"num_nonlinear_equality_constraints", P_RES numNonlinearEqConstraints},
      {"num_nonlinear_inequality_constraints", P_RES numNonlinearIneqConstraints},
      {"num_objective_functions", P_RES numObjectiveFunctions},
      {"num_response_functions", P_RES numResponseFunctions},
      {"num_scalar_calibration_terms", P_RES numScalarLeastSqTerms},
      {"num_scalar_nonlinear_equality_constraints",
	  P_RES numScalarNonlinearEqConstraints},
      {"num_scalar_nonlinear_inequality_constraints",
	  P_RES numScalarNonlinearIneqConstraints},
      {"num_scalar_objectives", P_RES numScalarObjectiveFunctions},
      {"num_scalar_responses", P_RES numScalarResponseFunctions}
    },
    entry_name, dbRep);
}


bool ProblemDescDB::get_bool(const String& entry_name) const
{
  return get<bool>
  ( "get_bool()",
    { /* environment */
      {"check", P_ENV checkFlag},
      {"graphics", P_ENV graphicsFlag},
      {"post_run", P_ENV postRunFlag},
      {"pre_run", P_ENV preRunFlag},
      {"results_output", P_ENV resultsOutputFlag},
      {"run", P_ENV runFlag},
      {"tabular_graphics_data", P_ENV tabularDataFlag}
    },
    { /* method */
      {"backfill", P_MET backfillFlag},
      {"chain_diagnostics", P_MET chainDiagnostics},
      {"chain_diagnostics.confidence_intervals", P_MET chainDiagnosticsCI},
      {"coliny.constant_penalty", P_MET constantPenalty},
      {"coliny.expansion", P_MET expansionFlag},
      {"coliny.randomize", P_MET randomizeOrderFlag},
      {"coliny.show_misc_options", P_MET showMiscOptions},
      {"derivative_usage", P_MET methodUseDerivsFlag},
      {"export_surrogate", P_MET exportSurrogate},
      {"fixed_seed", P_MET fixedSeedFlag},
      {"fsu_quasi_mc.fixed_sequence", P_MET fixedSequenceFlag},
      {"import_approx_active_only", P_MET importApproxActive},
      {"import_build_active_only", P_MET importBuildActive},
      {"laplace_approx", P_MET modelEvidLaplace},
      {"latinize", P_MET latinizeFlag},
      {"main_effects", P_MET mainEffectsFlag},
      {"mc_approx", P_MET modelEvidMC},
      {"mesh_adaptive_search.display_all_evaluations", P_MET showAllEval},
      {"model_evidence", P_MET modelEvidence},
      {"mutation_adaptive", P_MET mutationAdaptive},
      {"nl2sol.regression_diagnostics", P_MET regressDiag},
      {"nond.adapt_exp_design", P_MET adaptExpDesign},
      {"nond.adaptive_posterior_refinement", P_MET adaptPosteriorRefine},
      {"nond.allocation_target.optimization", P_MET useTargetVarianceOptimizationFlag},
      {"nond.c3function_train.adapt_order", P_MET adaptOrder},
      {"nond.c3function_train.adapt_rank", P_MET adaptRank},
      {"nond.cross_validation", P_MET crossValidation},
      {"nond.cross_validation.noise_only", P_MET crossValidNoiseOnly},
      {"nond.d_optimal", P_MET dOptimal},
      {"nond.evaluate_posterior_density", P_MET evaluatePosteriorDensity},
      {"nond.export_sample_sequence", P_MET exportSampleSeqFlag},
      {"nond.generate_posterior_samples", P_MET generatePosteriorSamples},
      {"nond.gpmsa_normalize", P_MET gpmsaNormalize},
      {"nond.logit_transform", P_MET logitTransform},
      {"nond.model_discrepancy", P_MET calModelDiscrepancy},
      {"nond.mutual_info_ksg2", P_MET mutualInfoKSG2},
      {"nond.normalized", P_MET normalizedCoeffs},
      {"nond.piecewise_basis", P_MET piecewiseBasis},
      {"nond.relative_convergence_metric", P_MET relativeConvMetric},
      {"nond.response_scaling", P_MET respScalingFlag},
      {"nond.standardized_space", P_MET standardizedSpace},
      {"nond.tensor_grid", P_MET tensorGridFlag},
      {"nond.truth_fixed_by_pilot", P_MET truthPilotConstraint},
      {"posterior_stats.kde", P_MET posteriorStatsKDE},
      {"posterior_stats.kl_divergence", P_MET posteriorStatsKL},
      {"posterior_stats.mutual_info", P_MET posteriorStatsMutual},
      {"principal_components", P_MET pcaFlag},
      {"print_each_pop", P_MET printPopFlag},
      {"pstudy.import_active_only", P_MET pstudyFileActive},
      {"quality_metrics", P_MET volQualityFlag},
      {"sbg.replace_points", P_MET surrBasedGlobalReplacePts},
      {"sbl.truth_surrogate_bypass", P_MET surrBasedLocalLayerBypass},
      {"scaling", P_MET methodScaling},
      {"speculative", P_MET speculativeFlag},
      {"std_regression_coeffs", P_MET stdRegressionCoeffs},
      {"tolerance_intervals", P_MET toleranceIntervalsFlag},
      {"variance_based_decomp", P_MET vbdFlag},
      {"wilks", P_MET wilksFlag},
      {"rank_1_lattice", P_MET rank1LatticeFlag},
      {"no_random_shift", P_MET noRandomShiftFlag},
      {"kuo", P_MET kuo},
      {"cools_kuo_nuyens", P_MET cools_kuo_nuyens},
      {"ordering.natural", P_MET naturalOrdering},
      {"ordering.radical_inverse", P_MET radicalInverseOrdering},
      {"digital_net", P_MET digitalNetFlag},
      {"no_digital_shift", P_MET noDigitalShiftFlag},
      {"no_scrambling", P_MET noScramblingFlag},
      {"most_significant_bit_first", P_MET mostSignificantBitFirst},
      {"least_significant_bit_first", P_MET leastSignificantBitFirst},
      {"joe_kuo", P_MET joe_kuo},
      {"sobol_order_2", P_MET sobol_order_2},
      {"gray_code_ordering", P_MET grayCodeOrdering}
    },
    { /* model */
      {"active_subspace.build_surrogate", P_MOD subspaceBuildSurrogate},
      {"active_subspace.cv.incremental", P_MOD subspaceCVIncremental},
      {"active_subspace.truncation_method.bing_li", P_MOD subspaceIdBingLi},
      {"active_subspace.truncation_method.constantine", P_MOD subspaceIdConstantine},
      {"active_subspace.truncation_method.cv", P_MOD subspaceIdCV},
      {"active_subspace.truncation_method.energy", P_MOD subspaceIdEnergy},
      {"c3function_train.adapt_order", P_MOD adaptOrder},
      {"c3function_train.adapt_rank", P_MOD adaptRank},
      {"c3function_train.tensor_grid", P_MOD tensorGridFlag},
      {"hierarchical_tags", P_MOD hierarchicalTags},
      {"nested.identity_resp_map", P_MOD identityRespMap},
      {"surrogate.auto_refine", P_MOD autoRefine},
      {"surrogate.challenge_points_file_active", P_MOD importChallengeActive},
      {"surrogate.challenge_use_variable_labels", P_MOD importChalUseVariableLabels},
      {"surrogate.cross_validate", P_MOD crossValidateFlag},
      {"surrogate.decomp_discont_detect", P_MOD decompDiscontDetect},
      {"surrogate.derivative_usage", P_MOD modelUseDerivsFlag},
      {"surrogate.domain_decomp", P_MOD domainDecomp},
      {"surrogate.export_surrogate", P_MOD exportSurrogate},
      {"surrogate.import_build_active_only", P_MOD importBuildActive},
      {"surrogate.import_surrogate", P_MOD importSurrogate},
      {"surrogate.import_use_variable_labels", P_MOD importUseVariableLabels},
      {"surrogate.point_selection", P_MOD pointSelection},
      {"surrogate.press", P_MOD pressFlag},
      {"surrogate.response_scaling", P_MOD respScalingFlag}
    },
    { /* variables */
      {"uncertain.initial_point_flag", P_VAR uncertainVarsInitPt}
    },
    { /* interface */
      {"active_set_vector", P_INT activeSetVectorFlag},
      {"allow_existing_results", P_INT allowExistingResultsFlag},
      {"application.file_save", P_INT fileSaveFlag},
      {"application.file_tag", P_INT fileTagFlag},
      {"application.verbatim", P_INT verbatimFlag},
      {"asynch", P_INT asynchFlag},
      {"batch", P_INT batchEvalFlag},
      {"dirSave", P_INT dirSave},
      {"dirTag", P_INT dirTag},
      {"evaluation_cache", P_INT evalCacheFlag},
      {"labeled_results", P_INT dakotaResultsFileLabeled},
      {"nearby_evaluation_cache", P_INT nearbyEvalCacheFlag},
      {"python.numpy", P_INT numpyFlag},
      {"restart_file", P_INT restartFileFlag},
      {"templateReplace", P_INT templateReplace},
      {"useWorkdir", P_INT useWorkdir}
    },
    { /* responses */
      {"calibration_data", P_RES calibrationDataFlag},
      {"central_hess", P_RES centralHess},
      {"ignore_bounds", P_RES ignoreBounds},
      {"interpolate", P_RES interpolateFlag},
      {"read_field_coordinates", P_RES readFieldCoords}
    },
    entry_name, dbRep);
}

/** This special case involving pointers doesn't use generic lookups */
void** ProblemDescDB::get_voidss(const String& entry_name) const
{
  if (entry_name == "method.dl_solver.dlLib") {
    if (dbRep->methodDBLocked)
      Locked_db();
    return &dbRep->dataMethodIter->dataMethodRep->dlLib;
  }
  Bad_name(entry_name, "get_voidss");
  return abort_handler_t<void**>(PARSE_ERROR);
}


void ProblemDescDB::set(const String& entry_name, const RealVector& rv)
{
  RealVector& rep_rv = get<RealVector>
  ( "set(RealVector&)",
    { /* environment */ },
    { /* method */ 
      {"nond.scalarization_response_mapping", P_MET scalarizationRespCoeffs}
    },
    { /* model */
      {"nested.primary_response_mapping", P_MOD primaryRespCoeffs},
      {"nested.secondary_response_mapping", P_MOD secondaryRespCoeffs}
    },
    { /* variables */
      {"beta_uncertain.alphas", P_VAR betaUncAlphas},
      {"beta_uncertain.betas", P_VAR betaUncBetas},
      {"beta_uncertain.lower_bounds", P_VAR betaUncLowerBnds},
      {"beta_uncertain.upper_bounds", P_VAR betaUncUpperBnds},
      {"binomial_uncertain.prob_per_trial", P_VAR binomialUncProbPerTrial},
      {"continuous_aleatory_uncertain.initial_point",
	  P_VAR continuousAleatoryUncVars},
      {"continuous_aleatory_uncertain.lower_bounds",
	  P_VAR continuousAleatoryUncLowerBnds},
      {"continuous_aleatory_uncertain.upper_bounds",
	  P_VAR continuousAleatoryUncUpperBnds},
      {"continuous_design.initial_point", P_VAR continuousDesignVars},
      {"continuous_design.initial_point", P_VAR continuousDesignVars},
      {"continuous_design.lower_bounds", P_VAR continuousDesignLowerBnds},
      {"continuous_design.lower_bounds", P_VAR continuousDesignLowerBnds},
      {"continuous_design.scales", P_VAR continuousDesignScales},
      {"continuous_design.upper_bounds", P_VAR continuousDesignUpperBnds},
      {"continuous_design.upper_bounds", P_VAR continuousDesignUpperBnds},
      {"continuous_epistemic_uncertain.initial_point",
	  P_VAR continuousEpistemicUncVars},
      {"continuous_epistemic_uncertain.lower_bounds",
	  P_VAR continuousEpistemicUncLowerBnds},
      {"continuous_epistemic_uncertain.upper_bounds",
	  P_VAR continuousEpistemicUncUpperBnds},
      {"continuous_state.initial_state", P_VAR continuousStateVars},
      {"continuous_state.lower_bounds", P_VAR continuousStateLowerBnds},
      {"continuous_state.upper_bounds", P_VAR continuousStateUpperBnds},
      {"discrete_aleatory_uncertain_real.initial_point",
	  P_VAR discreteRealAleatoryUncVars},
      {"discrete_aleatory_uncertain_real.lower_bounds",
	  P_VAR discreteRealAleatoryUncLowerBnds},
      {"discrete_aleatory_uncertain_real.upper_bounds",
	  P_VAR discreteRealAleatoryUncUpperBnds},
      {"discrete_design_set_real.initial_point", P_VAR discreteDesignSetRealVars},
      {"discrete_design_set_real.initial_point", P_VAR discreteDesignSetRealVars},
      {"discrete_epistemic_uncertain_real.initial_point",
	  P_VAR discreteRealEpistemicUncVars},
      {"discrete_epistemic_uncertain_real.lower_bounds",
	  P_VAR discreteRealEpistemicUncLowerBnds},
      {"discrete_epistemic_uncertain_real.upper_bounds",
	  P_VAR discreteRealEpistemicUncUpperBnds},
      {"discrete_state_set_real.initial_state", P_VAR discreteStateSetRealVars},
      {"exponential_uncertain.betas", P_VAR exponentialUncBetas},
      {"frechet_uncertain.alphas", P_VAR frechetUncAlphas},
      {"frechet_uncertain.betas", P_VAR frechetUncBetas},
      {"gamma_uncertain.alphas", P_VAR gammaUncAlphas},
      {"gamma_uncertain.betas", P_VAR gammaUncBetas},
      {"geometric_uncertain.prob_per_trial", P_VAR geometricUncProbPerTrial},
      {"gumbel_uncertain.alphas", P_VAR gumbelUncAlphas},
      {"gumbel_uncertain.betas", P_VAR gumbelUncBetas},
      {"linear_equality_constraints", P_VAR linearEqConstraintCoeffs},
      {"linear_equality_scales", P_VAR linearEqScales},
      {"linear_equality_targets", P_VAR linearEqTargets},
      {"linear_inequality_constraints", P_VAR linearIneqConstraintCoeffs},
      {"linear_inequality_lower_bounds", P_VAR linearIneqLowerBnds},
      {"linear_inequality_scales", P_VAR linearIneqScales},
      {"linear_inequality_upper_bounds", P_VAR linearIneqUpperBnds},
      {"lognormal_uncertain.error_factors", P_VAR lognormalUncErrFacts},
      {"lognormal_uncertain.lambdas", P_VAR lognormalUncLambdas},
      {"lognormal_uncertain.lower_bounds", P_VAR lognormalUncLowerBnds},
      {"lognormal_uncertain.means", P_VAR lognormalUncMeans},
      {"lognormal_uncertain.std_deviations", P_VAR lognormalUncStdDevs},
      {"lognormal_uncertain.upper_bounds", P_VAR lognormalUncUpperBnds},
      {"lognormal_uncertain.zetas", P_VAR lognormalUncZetas},
      {"loguniform_uncertain.lower_bounds", P_VAR loguniformUncLowerBnds},
      {"loguniform_uncertain.upper_bounds", P_VAR loguniformUncUpperBnds},
      {"negative_binomial_uncertain.prob_per_trial",
	  P_VAR negBinomialUncProbPerTrial},
      {"normal_uncertain.lower_bounds", P_VAR normalUncLowerBnds},
      {"normal_uncertain.means", P_VAR normalUncMeans},
      {"normal_uncertain.std_deviations", P_VAR normalUncStdDevs},
      {"normal_uncertain.upper_bounds", P_VAR normalUncUpperBnds},
      {"poisson_uncertain.lambdas", P_VAR poissonUncLambdas},
      {"triangular_uncertain.lower_bounds", P_VAR triangularUncLowerBnds},
      {"triangular_uncertain.modes", P_VAR triangularUncModes},
      {"triangular_uncertain.upper_bounds", P_VAR triangularUncUpperBnds},
      {"uniform_uncertain.lower_bounds", P_VAR uniformUncLowerBnds},
      {"uniform_uncertain.upper_bounds", P_VAR uniformUncUpperBnds},
      {"weibull_uncertain.alphas", P_VAR weibullUncAlphas},
      {"weibull_uncertain.betas", P_VAR weibullUncBetas}
    },
    { /* interface */ },
    { /* responses */
      {"nonlinear_equality_scales", P_RES nonlinearEqScales},
      {"nonlinear_equality_targets", P_RES nonlinearEqTargets},
      {"nonlinear_inequality_lower_bounds", P_RES nonlinearIneqLowerBnds},
      {"nonlinear_inequality_scales", P_RES nonlinearIneqScales},
      {"nonlinear_inequality_upper_bounds", P_RES nonlinearIneqUpperBnds},
      {"primary_response_fn_scales", P_RES primaryRespFnScales},
      {"primary_response_fn_weights", P_RES primaryRespFnWeights}
    },
    entry_name, dbRep);

  rep_rv = rv;
}


void ProblemDescDB::set(const String& entry_name, const IntVector& iv)
{
  IntVector& rep_iv = get<IntVector>
  ( "set(IntVector&)",
    { /* environment */ },
    { /* method */
      {"generating_vector.inline", P_MET generatingVector},
      {"generating_matrices.inline", P_MET generatingMatrices}
    },
    { /* model */ },
    { /* variables */
      {"binomial_uncertain.num_trials", P_VAR binomialUncNumTrials},
      {"discrete_aleatory_uncertain_int.initial_point",
	  P_VAR discreteIntAleatoryUncVars},
      {"discrete_aleatory_uncertain_int.lower_bounds",
	  P_VAR discreteIntAleatoryUncLowerBnds},
      {"discrete_aleatory_uncertain_int.upper_bounds",
	  P_VAR discreteIntAleatoryUncUpperBnds},
      {"discrete_design_range.initial_point", P_VAR discreteDesignRangeVars},
      {"discrete_design_range.lower_bounds", P_VAR discreteDesignRangeLowerBnds},
      {"discrete_design_range.upper_bounds", P_VAR discreteDesignRangeUpperBnds},
      {"discrete_design_set_int.initial_point", P_VAR discreteDesignSetIntVars},
      {"discrete_epistemic_uncertain_int.initial_point",
	  P_VAR discreteIntEpistemicUncVars},
      {"discrete_epistemic_uncertain_int.lower_bounds",
	  P_VAR discreteIntEpistemicUncLowerBnds},
      {"discrete_epistemic_uncertain_int.upper_bounds",
	  P_VAR discreteIntEpistemicUncUpperBnds},
      {"discrete_state_range.initial_state", P_VAR discreteStateRangeVars},
      {"discrete_state_range.lower_bounds", P_VAR discreteStateRangeLowerBnds},
      {"discrete_state_range.upper_bounds", P_VAR discreteStateRangeUpperBnds},
      {"discrete_state_set_int.initial_state", P_VAR discreteStateSetIntVars},
      {"hypergeometric_uncertain.num_drawn", P_VAR hyperGeomUncNumDrawn},
      {"hypergeometric_uncertain.selected_population",
	  P_VAR hyperGeomUncSelectedPop},
      {"hypergeometric_uncertain.total_population", P_VAR hyperGeomUncTotalPop},
      {"negative_binomial_uncertain.num_trials", P_VAR negBinomialUncNumTrials}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);

  rep_iv = iv;
}


void ProblemDescDB::set(const String& entry_name, const BitArray& ba)
{
  BitArray& rep_ba = get<BitArray>
  ( "set(BitArray&)",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"binomial_uncertain.categorical", P_VAR binomialUncCat},
      {"discrete_design_range.categorical", P_VAR discreteDesignRangeCat},
      {"discrete_design_set_int.categorical", P_VAR discreteDesignSetIntCat},
      {"discrete_design_set_real.categorical", P_VAR discreteDesignSetRealCat},
      {"discrete_interval_uncertain.categorical", P_VAR discreteIntervalUncCat},
      {"discrete_state_range.categorical", P_VAR discreteStateRangeCat},
      {"discrete_state_set_int.categorical", P_VAR discreteStateSetIntCat},
      {"discrete_state_set_real.categorical", P_VAR discreteStateSetRealCat},
      {"discrete_uncertain_set_int.categorical", P_VAR discreteUncSetIntCat},
      {"discrete_uncertain_set_real.categorical", P_VAR discreteUncSetRealCat},
      {"geometric_uncertain.categorical", P_VAR geometricUncCat},
      {"histogram_uncertain.point_int.categorical",
	  P_VAR histogramUncPointIntCat},
      {"histogram_uncertain.point_real.categorical",
	  P_VAR histogramUncPointRealCat},
      {"hypergeometric_uncertain.categorical", P_VAR hyperGeomUncCat},
      {"negative_binomial_uncertain.categorical", P_VAR negBinomialUncCat},
      {"poisson_uncertain.categorical", P_VAR poissonUncCat}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);

  rep_ba = ba;
}


void ProblemDescDB::set(const String& entry_name, const RealSymMatrix& rsm)
{
  RealSymMatrix& rep_rsm = get<RealSymMatrix>
  ( "set(RealSymMatrix&)",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"uncertain.correlation_matrix", P_VAR uncertainCorrelations}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);

  rep_rsm = rsm;
}


void ProblemDescDB::set(const String& entry_name, const RealVectorArray& rva)
{
  RealVectorArray& rep_rva = get<RealVectorArray>
  ( "set(RealVectorArray&)",
    { /* environment */ },
    { /* method */
      {"nond.gen_reliability_levels", P_MET genReliabilityLevels},
      {"nond.probability_levels", P_MET probabilityLevels},
      {"nond.reliability_levels", P_MET reliabilityLevels},
      {"nond.response_levels", P_MET responseLevels}
    },
    { /* model */ },
    { /* variables */ },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);

  rep_rva = rva;
}


void ProblemDescDB::set(const String& entry_name, const IntVectorArray& iva)
{
  IntVectorArray& rep_iva = get<IntVectorArray>
  ( "set(IntVectorArray&)",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */ },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);

  rep_iva = iva;
}


void ProblemDescDB::set(const String& entry_name, const IntSetArray& isa)
{
  IntSetArray& rep_isa = get<IntSetArray>
  ( "set(IntSetArray&)",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"discrete_design_set_int.values", P_VAR discreteDesignSetInt},
      {"discrete_state_set_int.values",  P_VAR discreteStateSetInt}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);

  rep_isa = isa;
}


void ProblemDescDB::set(const String& entry_name, const RealSetArray& rsa)
{
  RealSetArray& rep_rsa = get<RealSetArray>
  ( "set(RealSetArray&)",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"discrete_design_set_real.values", P_VAR discreteDesignSetReal},
      {"discrete_state_set_real.values",  P_VAR discreteStateSetReal}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);

  rep_rsa = rsa;
}


void ProblemDescDB::set(const String& entry_name, const IntRealMapArray& irma)
{
  IntRealMapArray& rep_irma = get<IntRealMapArray>
  ( "set(IntRealMapArray&)",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"discrete_uncertain_set_int.values_probs",
	  P_VAR discreteUncSetIntValuesProbs},
      {"histogram_uncertain.point_int_pairs", P_VAR histogramUncPointIntPairs}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);

  rep_irma = irma;
}


void ProblemDescDB::set(const String& entry_name, const StringRealMapArray& srma)
{
  StringRealMapArray& rep_srma = get<StringRealMapArray>
  ( "set(StringRealMapArray&)",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"histogram_uncertain.point_string_pairs", P_VAR histogramUncPointStrPairs}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);

  rep_srma = srma;
}


void ProblemDescDB::set(const String& entry_name, const RealRealMapArray& rrma)
{
  RealRealMapArray& rep_rrma = get<RealRealMapArray>
  ( "set(RealRealMapArray&)",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"discrete_uncertain_set_real.values_probs",
	  P_VAR discreteUncSetRealValuesProbs}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);

  rep_rrma = rrma;
}

void ProblemDescDB::
set(const String& entry_name, const RealRealPairRealMapArray& rrrma)
{
  RealRealPairRealMapArray& rep_rrrma = get<RealRealPairRealMapArray>
  ( "set(RealRealPairRealMapArray&)",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"continuous_interval_uncertain.basic_probs",
	  P_VAR continuousIntervalUncBasicProbs}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);

  rep_rrrma = rrrma;
}

void ProblemDescDB::
set(const String& entry_name, const IntIntPairRealMapArray& iirma)
{
  IntIntPairRealMapArray& rep_iirma = get<IntIntPairRealMapArray>
  ( "set(IntIntPairRealMapArray&)",
    { /* environment */ },
    { /* method */ },
    { /* model */ },
    { /* variables */
      {"discrete_interval_uncertain.basic_probs",
	  P_VAR discreteIntervalUncBasicProbs}
    },
    { /* interface */ },
    { /* responses */ },
    entry_name, dbRep);

  rep_iirma = iirma;
}


void ProblemDescDB::set(const String& entry_name, const StringArray& sa)
{
  StringArray& rep_sa = get<StringArray>
  ( "set(StringArray&)",
    { /* environment */ },
    { /* method */ },
    { /* model */
      {"diagnostics", P_MOD diagMetrics},
      {"nested.primary_variable_mapping", P_MOD primaryVarMaps},
      {"nested.secondary_variable_mapping", P_MOD secondaryVarMaps}
    },
    { /* variables */
      {"continuous_aleatory_uncertain.labels", P_VAR continuousAleatoryUncLabels},
      {"continuous_design.labels", P_VAR continuousDesignLabels},
      {"continuous_design.scale_types", P_VAR continuousDesignScaleTypes},
      {"continuous_epistemic_uncertain.labels",
	  P_VAR continuousEpistemicUncLabels},
      {"continuous_state.labels", P_VAR continuousStateLabels},
      {"discrete_aleatory_uncertain_int.labels",
	  P_VAR discreteIntAleatoryUncLabels},
      {"discrete_aleatory_uncertain_real.labels",
	  P_VAR discreteRealAleatoryUncLabels},
      {"discrete_design_range.labels", P_VAR discreteDesignRangeLabels},
      {"discrete_design_set_int.labels", P_VAR discreteDesignSetIntLabels},
      {"discrete_design_set_real.labels", P_VAR discreteDesignSetRealLabels},
      {"discrete_epistemic_uncertain_int.labels",
	  P_VAR discreteIntEpistemicUncLabels},
      {"discrete_epistemic_uncertain_real.labels",
	  P_VAR discreteRealEpistemicUncLabels},
      {"discrete_state_range.labels", P_VAR discreteStateRangeLabels},
      {"discrete_state_set_int.labels", P_VAR discreteStateSetIntLabels},
      {"discrete_state_set_real.labels", P_VAR discreteStateSetRealLabels},
      {"discrete_state_set_string.labels", P_VAR discreteStateSetStrLabels},
      {"linear_equality_scale_types", P_VAR linearEqScaleTypes},
      {"linear_inequality_scale_types", P_VAR linearIneqScaleTypes}
    },
    { /* interface */ },
    { /* responses */
      {"labels", P_RES responseLabels },
      {"nonlinear_equality_scale_types", P_RES nonlinearEqScaleTypes },
      {"nonlinear_inequality_scale_types", P_RES nonlinearIneqScaleTypes },
      {"primary_response_fn_scale_types", P_RES primaryRespFnScaleTypes }
    },
    entry_name, dbRep);

  rep_sa = sa;
}


void ProblemDescDB::echo_input_file(const std::string& dakota_input_file,
				    const std::string& dakota_input_string,
				    const std::string& tmpl_qualifier)
{
  if (!dakota_input_string.empty()) {
    size_t header_len = 23;
    std::string header(header_len, '-');
    Cout << header << '\n';
    Cout << "Begin DAKOTA input file" << tmpl_qualifier << "\n";
    if(dakota_input_file == "-")
      Cout << "(from standard input)\n";
    else
      Cout << "(from string)\n";
    Cout << header << std::endl;
    Cout << dakota_input_string << std::endl;
    Cout << "---------------------\n";
    Cout << "End DAKOTA input file\n";
    Cout << "---------------------\n" << std::endl;
  } else if(!dakota_input_file.empty()) {
      std::ifstream inputstream(dakota_input_file.c_str());
      if (!inputstream.good()) {
	Cerr << "\nError: Could not open input file '" << dakota_input_file
	     << "' for reading." << std::endl;
	abort_handler(IO_ERROR);
      }

      // BMA TODO: could enable this now
      // want to output FQ path, but only valid in BFS v3; need wrapper
      //boost::filesystem::path bfs_file(dakota_input_file);
      //boost::filesystem::path bfs_abs_path = bfs_file.absolute();

      // header to span the potentially long filename
      size_t header_len = std::max((size_t) 23,
				   dakota_input_file.size());
      std::string header(header_len, '-');
      Cout << header << '\n';
      Cout << "Begin DAKOTA input file" << tmpl_qualifier << "\n";
      Cout << dakota_input_file << "\n";
      Cout << header << std::endl;
      int inputchar = inputstream.get();
      while (inputstream.good()) {
	Cout << (char) inputchar;
	inputchar = inputstream.get();
      }
      Cout << "---------------------\n";
      Cout << "End DAKOTA input file\n";
      Cout << "---------------------\n" << std::endl;
  }
}

/** Require string idenfitiers id_* to be unique across all blocks of
    each type (method, model, variables, interface, responses

    For now, this allows duplicate empty ID strings. Would be better
    to require unique IDs when more than one block of a given type
    appears in the input file (instead of use-the-last-parsed)
*/
void ProblemDescDB::enforce_unique_ids()
{
  bool found_error = false;
  std::multiset<String> block_ids;

  // Lambda to detect duplicate for the passed id, issuing error
  // message for the specified block_type. Modifies set of block_ids
  // and found_error status.
  auto check_unique = [&block_ids, &found_error] (String block_type, String id) {
    if (!id.empty()) {
      block_ids.insert(id);
      // (Only warn once per unique ID name)
      if (block_ids.count(id) == 2) {
	Cerr << "Error: id_" << block_type << " '" << id
	     << "' appears more than once.\n";
	found_error = true;
      }
    }
  };

  // This could be written more generically if the member was always
  // called idString instead of a different name (idMethod, idModel,
  // etc.) for each Data* class...; then the same code could apply to
  // all data*List
  for (auto data_cont : dataMethodList)
    check_unique("method", data_cont.data_rep()->idMethod);
  block_ids.clear();

  for (auto data_cont : dataModelList)
    check_unique("model", data_cont.data_rep()->idModel);
  block_ids.clear();

  for (auto data_cont : dataVariablesList)
    check_unique("variables", data_cont.data_rep()->idVariables);
  block_ids.clear();

  for (auto data_cont : dataInterfaceList)
    check_unique("interface", data_cont.data_rep()->idInterface);
  block_ids.clear();

  for (auto data_cont : dataResponsesList)
    check_unique("responses", data_cont.data_rep()->idResponses);
  block_ids.clear();

  if (found_error)
    abort_handler(PARSE_ERROR);
}


#undef P_ENV
#undef P_MET
#undef P_MOD
#undef P_VAR
#undef P_INT
#undef P_RES


} // namespace Dakota

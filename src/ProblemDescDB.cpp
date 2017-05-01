/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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
//#define REFCOUNT_DEBUG

static const char rcsId[]="@(#) $Id: ProblemDescDB.cpp 7007 2010-10-06 15:54:39Z wjbohnh $";


namespace Dakota {

extern ParallelLibrary dummy_lib; // defined in dakota_global_defs.cpp
extern ProblemDescDB *Dak_pddb;	  // defined in dakota_global_defs.cpp


/** This constructor is the one which must build the base class data for all
    derived classes.  get_db() instantiates a derived class letter and the
    derived constructor selects this base class constructor in its
    initialization list (to avoid the recursion of the base class constructor
    calling get_db() again).  Since the letter IS the representation, its
    representation pointer is set to NULL (an uninitialized pointer causes
    problems in ~ProblemDescDB). */
ProblemDescDB::ProblemDescDB(BaseConstructor, ParallelLibrary& parallel_lib):
  parallelLib(parallel_lib), environmentCntr(0), methodDBLocked(true),
  modelDBLocked(true), variablesDBLocked(true), interfaceDBLocked(true),
  responsesDBLocked(true), dbRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "ProblemDescDB::ProblemDescDB(BaseConstructor) called to build base "
       << "class data for letter object." << std::endl;
#endif
}


/** The default constructor: dbRep is NULL in this case.  This makes
    it necessary to check for NULL in the copy constructor, assignment
    operator, and destructor. */
ProblemDescDB::ProblemDescDB(): parallelLib(dummy_lib), dbRep(NULL),
  referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "ProblemDescDB::ProblemDescDB() called to build empty db object."
       << std::endl;
#endif
}


/** This is the envelope constructor which uses problem_db to build a
    fully populated db object.  It only needs to extract enough data
    to properly execute get_db(problem_db), since the constructor
    overloaded with BaseConstructor builds the actual base class data
    inherited by the derived classes. */
ProblemDescDB::ProblemDescDB(ParallelLibrary& parallel_lib):
  parallelLib(parallel_lib),
  referenceCount(1) // not used since this is the envelope, not the letter
{
#ifdef REFCOUNT_DEBUG
  Cout << "ProblemDescDB::ProblemDescDB(ParallelLibrary&) called to "
       << "instantiate envelope." << std::endl;
#endif

  // Set the rep pointer to the appropriate db type
  dbRep = get_db(parallel_lib);
  if (!dbRep) // bad settings or insufficient memory
    abort_handler(-1);
}


/** Initializes dbRep to the appropriate derived type.  The standard
    derived class constructors are invoked.  */
ProblemDescDB* ProblemDescDB::get_db(ParallelLibrary& parallel_lib)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Envelope instantiating letter in get_db(ParallelLibrary&)."
       << std::endl;
#endif

   Dak_pddb = this;	// for use in abort_handler()

  //if (xml_flag)
  //  return new XMLProblemDescDB(parallel_lib);
  //else
    return new NIDRProblemDescDB(parallel_lib);
}


/** Copy constructor manages sharing of dbRep and incrementing of
    referenceCount. */
ProblemDescDB::ProblemDescDB(const ProblemDescDB& db):
  parallelLib(db.parallel_library())
{
  // Increment new (no old to decrement)
  dbRep = db.dbRep;
  if (dbRep) // Check for an assignment of NULL
    dbRep->referenceCount++;

#ifdef REFCOUNT_DEBUG
  Cout << "ProblemDescDB::ProblemDescDB(ProblemDescDB&)" << std::endl;
  if (dbRep)
    Cout << "dbRep referenceCount = " << dbRep->referenceCount << std::endl;
#endif
}


/** Assignment operator decrements referenceCount for old dbRep, assigns
    new dbRep, and increments referenceCount for new dbRep. */
ProblemDescDB ProblemDescDB::operator=(const ProblemDescDB& db)
{
  if (dbRep != db.dbRep) { // normal case: old != new
    // Decrement old
    if (dbRep) // Check for NULL
      if ( --dbRep->referenceCount == 0 )
	delete dbRep;
    // Assign and increment new
    dbRep = db.dbRep;
    if (dbRep) // Check for NULL
      dbRep->referenceCount++;
  }
  // else if assigning same rep, then do nothing since referenceCount
  // should already be correct

#ifdef REFCOUNT_DEBUG
  Cout << "ProblemDescDB::operator=(ProblemDescDB&)" << std::endl;
  if (dbRep)
    Cout << "dbRep referenceCount = " << dbRep->referenceCount << std::endl;
#endif

  return *this; // calls copy constructor since returned by value
}


/** Destructor decrements referenceCount and only deletes dbRep
    when referenceCount reaches zero. */
ProblemDescDB::~ProblemDescDB()
{
  if (this == Dak_pddb)
	Dak_pddb = 0;
  // Check for NULL pointer
  if (dbRep) {
    --dbRep->referenceCount;
#ifdef REFCOUNT_DEBUG
    Cout << "dbRep referenceCount decremented to " << dbRep->referenceCount
	 << std::endl;
#endif
    if (dbRep->referenceCount == 0) {
#ifdef REFCOUNT_DEBUG
      Cout << "deleting dbRep" << std::endl;
#endif
      delete dbRep;
    }
  }
}


/** DB setup phase 1: parse the input file and execute callback
    functions if present.  Rank 0 only.

    DB setup phase 2: optionally insert additional data via late sets.
    Rank 0 only. */
void ProblemDescDB::
parse_inputs(const ProgramOptions& prog_opts, 
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

      if (prog_opts.echo_input())
	echo_input_file(prog_opts);

      // Parse the input file using one of the derived parser-specific classes
      derived_parse_inputs(prog_opts);

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
derived_parse_inputs(const ProgramOptions& prog_opts)
{
  if (dbRep)
    dbRep->derived_parse_inputs(prog_opts);
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
  else if (method_tag != "NO_SPECIFICATION") {
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
  else if (method_tag != "NO_SPECIFICATION") {
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
      DataModelRep *MoRep = dataModelIter->dataModelRep;
      set_db_variables_node(MoRep->variablesPointer);
      if (model_has_interface(MoRep))
	set_db_interface_node(MoRep->interfacePointer);
      else
	interfaceDBLocked = true;
      set_db_responses_node(MoRep->responsesPointer);
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
  else if (model_tag != "NO_SPECIFICATION") {
    // set dataModelIter from model_tag
    if (model_tag.empty()) { // no pointer specification
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
      DataModelRep *MoRep = dataModelIter->dataModelRep;
      set_db_variables_node(MoRep->variablesPointer);
      if (model_has_interface(MoRep))
	set_db_interface_node(MoRep->interfacePointer);
      else
	interfaceDBLocked = true;
      set_db_responses_node(MoRep->responsesPointer);
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
  else if (interface_tag != "NO_SPECIFICATION") {
    DataModelRep *MoRep = dataModelIter->dataModelRep;
    // set dataInterfaceIter from interface_tag
    if (interface_tag.empty()) { // no pointer specification
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
	      MoRep->modelType == "simulation")
	    Cerr << "\nWarning: empty interface id string not found.\n         "
		 << "Last interface specification parsed will be used.\n";
	  --dataInterfaceIter; // last entry in list
	}
	else if (parallelLib.world_rank() == 0 &&
		 MoRep->modelType == "simulation"  &&
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


const Iterator& ProblemDescDB::get_iterator()
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
  const String& id_method = dbRep->dataMethodIter->dataMethodRep->idMethod;
  IterLIter i_it
    = std::find_if(dbRep->iteratorList.begin(), dbRep->iteratorList.end(),
                   boost::bind(&Iterator::method_id, _1) == id_method);
  if (i_it == dbRep->iteratorList.end()) {
    Iterator new_iterator(*this);
    dbRep->iteratorList.push_back(new_iterator);
    i_it = --dbRep->iteratorList.end();
  }
  return *i_it;
}


const Iterator& ProblemDescDB::get_iterator(Model& model)
{
  // ProblemDescDB::get_<object> functions operate at the envelope level
  // so that any passing of *this provides the envelope object.
  if (!dbRep) {
    Cerr << "Error: ProblemDescDB::get_iterator() called for letter object."
	 << std::endl;
    abort_handler(PARSE_ERROR);
  }

  const String& id_method = dbRep->dataMethodIter->dataMethodRep->idMethod;
  IterLIter i_it
    = std::find_if(dbRep->iteratorList.begin(), dbRep->iteratorList.end(),
                   boost::bind(&Iterator::method_id, _1) == id_method);
  // if Iterator does not already exist, then create it
  if (i_it == dbRep->iteratorList.end()) {
    Iterator new_iterator(*this, model);
    dbRep->iteratorList.push_back(new_iterator);
    i_it = --dbRep->iteratorList.end();
  }
  // idMethod already exists, but check for same model.  If !same, instantiate
  // new rather than update (i_it->iterated_model(model)) all shared instances.
  else if (model != i_it->iterated_model()) {
    Iterator new_iterator(*this, model);
    dbRep->iteratorList.push_back(new_iterator);
    i_it = --dbRep->iteratorList.end();
  }
  return *i_it;
}


const Iterator& ProblemDescDB::
get_iterator(const String& method_name, Model& model)
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
                   boost::bind(&Iterator::method_string, _1) == method_name);
  // if Iterator does not already exist, then create it
  if (i_it == dbRep->iteratorByNameList.end()) {
    Iterator new_iterator(method_name, model);
    dbRep->iteratorByNameList.push_back(new_iterator);
    i_it = --dbRep->iteratorByNameList.end();
  }
  // method_name already exists, but check for same model. If !same, instantiate
  // new rather than update (i_it->iterated_model(model)) all shared instances.
  else if (model != i_it->iterated_model()) {
    Iterator new_iterator(method_name, model);
    dbRep->iteratorByNameList.push_back(new_iterator);
    i_it = --dbRep->iteratorByNameList.end();
  }
  return *i_it;
}


const Model& ProblemDescDB::get_model()
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
  const String& id_model = dbRep->dataModelIter->dataModelRep->idModel;
  ModelLIter m_it
    = std::find_if(dbRep->modelList.begin(), dbRep->modelList.end(),
                   boost::bind(&Model::model_id, _1) == id_model);
  if (m_it == dbRep->modelList.end()) {
    Model new_model(*this);
    dbRep->modelList.push_back(new_model);
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
  // > TaylorApproximation: model.surrogate.actual_model_pointer,
  //     responses.hessian_type
  // > OrthogPolyApproximation: method.nond.expansion_{terms,order}

  // The DB list nodes are set prior to calling get_interface():
  // >    interface_ptr spec -> id_interface must be defined
  // > no interface_ptr spec -> id_interf ignored, interf spec = last parsed
  const String& id_interface
    = dbRep->dataInterfaceIter->dataIfaceRep->idInterface;
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


 static void*
binsearch(void *kw, size_t kwsize, size_t n, const char* key)
{
	/* Binary search, based loosely on b_search.c in the */
	/* AMPL/solver interface library. */
	char *ow, *ow1, *s;
	int c;
	size_t n1;

	ow = (char*)kw;
	while(n > 0) {
		ow1 = ow + (n1 = n >> 1)*kwsize;
		s = *(char **)ow1;
        if ((c = std::strcmp(key, s)) == 0)
			return ow1;
		if (c < 0)
			n = n1;
		else {
			n -= n1 + 1;
			ow = ow1 + kwsize;
			}
		}
	return 0;
	}

 static const char*
Begins(const String &entry_name, const char *s)
{
	const char *t, *t0;
	t = entry_name.data();
	while(*t++ == *s++)
		if (!*s)
			return t;
	return 0;
	}

template<typename T, class A> struct KW {const char*key; T A::* p;};
#define Binsearch(t,s) binsearch(t, sizeof(t[0]), sizeof(t)/sizeof(t[0]), s)
// L is the length of the prefix already tested, e.g., 7 for "method."

static void Bad_name(String entry_name, const char *where)
{
  Cerr << "\nBad entry_name in ProblemDescDB::" << where << ":  "
       << entry_name << std::endl;
  abort_handler(PARSE_ERROR);
}

static void Locked_db()
{
  Cerr << "\nError: database is locked.  You must first unlock the database\n"
       << "       by setting the list nodes." << std::endl;
  abort_handler(PARSE_ERROR);
}

static void Null_rep(const char *who)
{
  Cerr << "\nError: ProblemDescDB::" << who
       << "() called with NULL representation." << std::endl;
  abort_handler(PARSE_ERROR);
}

static void Null_rep1(const char *who)
{
  Cerr << "\nError: ProblemDescDB::" << who
       << " called with NULL representation." << std::endl;
  abort_handler(PARSE_ERROR);
}

const RealMatrixArray& ProblemDescDB::get_rma(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_rma");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<RealMatrixArray, DataVariablesRep> RMAdv[] = {	
      // must be sorted by string (key)
	{"discrete_design_set_int.adjacency_matrix", P discreteDesignSetIntAdj},
	{"discrete_design_set_real.adjacency_matrix", P discreteDesignSetRealAdj},
	{"discrete_design_set_str.adjacency_matrix", P discreteDesignSetStrAdj}
	};
    #undef P
    KW<RealMatrixArray, DataVariablesRep> *kw;
    if ((kw = (KW<RealMatrixArray, DataVariablesRep>*)Binsearch(RMAdv, L)))
	return dbRep->dataVariablesIter->dataVarsRep->*kw->p;
  }
  Bad_name(entry_name, "get_rma");
  return abort_handler_t<const RealMatrixArray&>(PARSE_ERROR);
}


const RealVector& ProblemDescDB::get_rv(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_rv");
  if ((L = Begins(entry_name, "method."))) {
    if (dbRep->methodDBLocked)
	Locked_db();

    #define P &DataMethodRep::
    static KW<RealVector, DataMethodRep> RVdme[] = {
      // must be sorted by string (key)
	{"concurrent.parameter_sets", P concurrentParameterSets},
	{"jega.distance_vector", P distanceVector},
	{"jega.niche_vector", P nicheVector},
	{"nond.data_dist_covariance", P dataDistCovariance},
	{"nond.data_dist_means", P dataDistMeans},
	{"nond.dimension_preference", P anisoDimPref},
	{"nond.hyperprior_alphas", P hyperPriorAlphas},
	{"nond.hyperprior_betas", P hyperPriorBetas},
	{"nond.prediction_configs", P predictionConfigList},
	{"nond.proposal_covariance_data", P proposalCovData},
	{"nond.regression_noise_tolerance", P regressionNoiseTol},
	{"parameter_study.final_point", P finalPoint},
	{"parameter_study.list_of_points", P listOfPoints},
	{"parameter_study.step_vector", P stepVector},
	{"trust_region.initial_size", P trustRegionInitSize}};
    #undef P

    KW<RealVector, DataMethodRep> *kw;
    if ((kw = (KW<RealVector, DataMethodRep>*)Binsearch(RVdme, L)))
    	return dbRep->dataMethodIter->dataMethodRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "model."))) {
    if (dbRep->methodDBLocked)
	Locked_db();

    #define P &DataModelRep::
    static KW<RealVector, DataModelRep> RVdmo[] = {	
      // must be sorted by string (key)
	{"nested.primary_response_mapping", P primaryRespCoeffs},
	{"nested.secondary_response_mapping", P secondaryRespCoeffs},
	{"simulation.solution_level_cost", P solutionLevelCost},
	{"surrogate.kriging_correlations", P krigingCorrelations},
	{"surrogate.kriging_max_correlations", P krigingMaxCorrelations},
	{"surrogate.kriging_min_correlations", P krigingMinCorrelations}};
    #undef P

    KW<RealVector, DataModelRep> *kw;
    if ((kw = (KW<RealVector, DataModelRep>*)Binsearch(RVdmo, L)))
	return dbRep->dataModelIter->dataModelRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "variables."))) {

    #define P &DataVariablesRep::
    static KW<RealVector, DataVariablesRep> RVdv[] = {	
      // must be sorted by string (key)
	{"beta_uncertain.alphas", P betaUncAlphas},
	{"beta_uncertain.betas", P betaUncBetas},
	{"beta_uncertain.lower_bounds", P betaUncLowerBnds},
	{"beta_uncertain.upper_bounds", P betaUncUpperBnds},
	{"binomial_uncertain.prob_per_trial", P binomialUncProbPerTrial},
	{"continuous_aleatory_uncertain.initial_point",
	 P continuousAleatoryUncVars},
	{"continuous_aleatory_uncertain.lower_bounds",
	 P continuousAleatoryUncLowerBnds},
	{"continuous_aleatory_uncertain.upper_bounds",
	 P continuousAleatoryUncUpperBnds},
	{"continuous_design.initial_point", P continuousDesignVars},
	{"continuous_design.lower_bounds", P continuousDesignLowerBnds},
	{"continuous_design.scales", P continuousDesignScales},
	{"continuous_design.upper_bounds", P continuousDesignUpperBnds},
	{"continuous_epistemic_uncertain.initial_point",
	 P continuousEpistemicUncVars},
	{"continuous_epistemic_uncertain.lower_bounds",
	 P continuousEpistemicUncLowerBnds},
	{"continuous_epistemic_uncertain.upper_bounds",
	 P continuousEpistemicUncUpperBnds},
	{"continuous_state.initial_state", P continuousStateVars},
	{"continuous_state.lower_bounds", P continuousStateLowerBnds},
	{"continuous_state.upper_bounds", P continuousStateUpperBnds},
	{"discrete_aleatory_uncertain_real.initial_point",
	 P discreteRealAleatoryUncVars},
	{"discrete_aleatory_uncertain_real.lower_bounds",
	 P discreteRealAleatoryUncLowerBnds},
	{"discrete_aleatory_uncertain_real.upper_bounds",
	 P discreteRealAleatoryUncUpperBnds},
	{"discrete_design_set_real.initial_point", P discreteDesignSetRealVars},
	{"discrete_design_set_real.lower_bounds",
	 P discreteDesignSetRealLowerBnds},
	{"discrete_design_set_real.upper_bounds",
	 P discreteDesignSetRealUpperBnds},
	{"discrete_epistemic_uncertain_real.initial_point",
	 P discreteRealEpistemicUncVars},
	{"discrete_epistemic_uncertain_real.lower_bounds",
	 P discreteRealEpistemicUncLowerBnds},
	{"discrete_epistemic_uncertain_real.upper_bounds",
	 P discreteRealEpistemicUncUpperBnds},
	{"discrete_state_set_real.initial_state", P discreteStateSetRealVars},
	{"discrete_state_set_real.lower_bounds",
	 P discreteStateSetRealLowerBnds},
	{"discrete_state_set_real.upper_bounds",
	 P discreteStateSetRealUpperBnds},
	{"exponential_uncertain.betas", P exponentialUncBetas},
	{"frechet_uncertain.alphas", P frechetUncAlphas},
	{"frechet_uncertain.betas", P frechetUncBetas},
	{"gamma_uncertain.alphas", P gammaUncAlphas},
	{"gamma_uncertain.betas", P gammaUncBetas},
	{"geometric_uncertain.prob_per_trial", P geometricUncProbPerTrial},
	{"gumbel_uncertain.alphas", P gumbelUncAlphas},
	{"gumbel_uncertain.betas", P gumbelUncBetas},
	{"linear_equality_constraints", P linearEqConstraintCoeffs},
	{"linear_equality_scales", P linearEqScales},
	{"linear_equality_targets", P linearEqTargets},
	{"linear_inequality_constraints", P linearIneqConstraintCoeffs},
	{"linear_inequality_lower_bounds", P linearIneqLowerBnds},
	{"linear_inequality_scales", P linearIneqScales},
	{"linear_inequality_upper_bounds", P linearIneqUpperBnds},
	{"lognormal_uncertain.error_factors", P lognormalUncErrFacts},
	{"lognormal_uncertain.lambdas", P lognormalUncLambdas},
	{"lognormal_uncertain.lower_bounds", P lognormalUncLowerBnds},
	{"lognormal_uncertain.means", P lognormalUncMeans},
	{"lognormal_uncertain.std_deviations", P lognormalUncStdDevs},
	{"lognormal_uncertain.upper_bounds", P lognormalUncUpperBnds},
	{"lognormal_uncertain.zetas", P lognormalUncZetas},
	{"loguniform_uncertain.lower_bounds", P loguniformUncLowerBnds},
	{"loguniform_uncertain.upper_bounds", P loguniformUncUpperBnds},
	{"negative_binomial_uncertain.prob_per_trial",
	 P negBinomialUncProbPerTrial},
	{"normal_uncertain.lower_bounds", P normalUncLowerBnds},
	{"normal_uncertain.means", P normalUncMeans},
	{"normal_uncertain.std_deviations", P normalUncStdDevs},
	{"normal_uncertain.upper_bounds", P normalUncUpperBnds},
	{"poisson_uncertain.lambdas", P poissonUncLambdas},
	{"triangular_uncertain.lower_bounds", P triangularUncLowerBnds},
	{"triangular_uncertain.modes", P triangularUncModes},
	{"triangular_uncertain.upper_bounds", P triangularUncUpperBnds},
	{"uniform_uncertain.lower_bounds", P uniformUncLowerBnds},
	{"uniform_uncertain.upper_bounds", P uniformUncUpperBnds},
	{"weibull_uncertain.alphas", P weibullUncAlphas},
	{"weibull_uncertain.betas", P weibullUncBetas}};
    #undef P

    KW<RealVector, DataVariablesRep> *kw;
    if ((kw = (KW<RealVector, DataVariablesRep>*)Binsearch(RVdv, L)))
	return dbRep->dataVariablesIter->dataVarsRep->*kw->p;
  }
  else if (strbegins(entry_name, "interface.")) {
    if (dbRep->interfaceDBLocked)
	Locked_db();
    else if (strends(entry_name, "failure_capture.recovery_fn_vals"))
      return dbRep->dataInterfaceIter->dataIfaceRep->recoveryFnVals;
  }
  else if ((L = Begins(entry_name, "responses."))) {
    if (dbRep->responsesDBLocked)
	Locked_db();

    #define P &DataResponsesRep::
    static KW<RealVector, DataResponsesRep> RVdr[] = {	
      // must be sorted by string (key)
	{"exp_config_variables", P expConfigVars},
	{"exp_observations", P expObservations},
	{"exp_std_deviations", P expStdDeviations},
	{"fd_gradient_step_size", P fdGradStepSize},
	{"fd_hessian_step_size", P fdHessStepSize},
	{"nonlinear_equality_scales", P nonlinearEqScales},
	{"nonlinear_equality_targets", P nonlinearEqTargets},
	{"nonlinear_inequality_lower_bounds", P nonlinearIneqLowerBnds},
	{"nonlinear_inequality_scales", P nonlinearIneqScales},
	{"nonlinear_inequality_upper_bounds", P nonlinearIneqUpperBnds},
	{"primary_response_fn_scales", P primaryRespFnScales},
	{"primary_response_fn_weights", P primaryRespFnWeights}};
    #undef P

    KW<RealVector, DataResponsesRep> *kw;
    if ((kw = (KW<RealVector, DataResponsesRep>*)Binsearch(RVdr, L)))
	return dbRep->dataResponsesIter->dataRespRep->*kw->p;
  }
  Bad_name(entry_name, "get_rv");
  return abort_handler_t<const RealVector&>(PARSE_ERROR);
}


const IntVector& ProblemDescDB::get_iv(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_iv");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<IntVector, DataVariablesRep> IVdv[] = {	
      // must be sorted by string (key)
	{"binomial_uncertain.num_trials", P binomialUncNumTrials},
	{"discrete_aleatory_uncertain_int.initial_point",
	 P discreteIntAleatoryUncVars},
	{"discrete_aleatory_uncertain_int.lower_bounds",
	 P discreteIntAleatoryUncLowerBnds},
	{"discrete_aleatory_uncertain_int.upper_bounds",
	 P discreteIntAleatoryUncUpperBnds},
	{"discrete_design_range.initial_point", P discreteDesignRangeVars},
	{"discrete_design_range.lower_bounds", P discreteDesignRangeLowerBnds},
	{"discrete_design_range.upper_bounds", P discreteDesignRangeUpperBnds},
	{"discrete_design_set_int.initial_point", P discreteDesignSetIntVars},
	{"discrete_design_set_int.lower_bounds",
	 P discreteDesignSetIntLowerBnds},
	{"discrete_design_set_int.upper_bounds",
	 P discreteDesignSetIntUpperBnds},
	{"discrete_epistemic_uncertain_int.initial_point",
	 P discreteIntEpistemicUncVars},
	{"discrete_epistemic_uncertain_int.lower_bounds",
	 P discreteIntEpistemicUncLowerBnds},
	{"discrete_epistemic_uncertain_int.upper_bounds",
	 P discreteIntEpistemicUncUpperBnds},
	{"discrete_state_range.initial_state", P discreteStateRangeVars},
	{"discrete_state_range.lower_bounds", P discreteStateRangeLowerBnds},
	{"discrete_state_range.upper_bounds", P discreteStateRangeUpperBnds},
	{"discrete_state_set_int.initial_state", P discreteStateSetIntVars},
	{"discrete_state_set_int.lower_bounds", P discreteStateSetIntLowerBnds},
	{"discrete_state_set_int.upper_bounds", P discreteStateSetIntUpperBnds},
	{"hypergeometric_uncertain.num_drawn", P hyperGeomUncNumDrawn},
	{"hypergeometric_uncertain.selected_population",
	 P hyperGeomUncSelectedPop},
	{"hypergeometric_uncertain.total_population", P hyperGeomUncTotalPop},
	{"negative_binomial_uncertain.num_trials", P negBinomialUncNumTrials}};
    #undef P

    KW<IntVector, DataVariablesRep> *kw;
    if ((kw = (KW<IntVector, DataVariablesRep>*)Binsearch(IVdv, L)))
	return dbRep->dataVariablesIter->dataVarsRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "method."))) {
	if (dbRep->methodDBLocked)
		Locked_db();
    #define P &DataMethodRep::
    static KW<IntVector, DataMethodRep> IVdme[] = {	
      // must be sorted by string (key)
	{"fsu_quasi_mc.primeBase", P primeBase},
	{"fsu_quasi_mc.sequenceLeap", P sequenceLeap},
	{"fsu_quasi_mc.sequenceStart", P sequenceStart},
	{"nond.refinement_samples", P refineSamples},
	{"parameter_study.steps_per_variable", P stepsPerVariable}};
    #undef P
    KW<IntVector, DataMethodRep> *kw;
    if ((kw = (KW<IntVector, DataMethodRep>*)Binsearch(IVdme, L)))
	return dbRep->dataMethodIter->dataMethodRep->*kw->p;
  }
  else if (L = Begins(entry_name, "model.")) {
    if (dbRep->modelDBLocked)
      Locked_db();
    #define P &DataModelRep::
    static KW<IntVector, DataModelRep> IVdr[] = {	
      // must be sorted by string (key)
      {"refinement_samples", P refineSamples}};
    #undef P
    KW<IntVector, DataModelRep> *kw;
    if ((kw = (KW<IntVector, DataModelRep>*)Binsearch(IVdr, L)))
      return dbRep->dataModelIter->dataModelRep->*kw->p;
  }
  else if (L = Begins(entry_name, "responses.")) {
    if (dbRep->responsesDBLocked)
      Locked_db();
    #define P &DataResponsesRep::
    static KW<IntVector, DataResponsesRep> IVdr[] = {	
      // must be sorted by string (key)
	{"lengths", P fieldLengths},
	{"num_coordinates_per_field", P numCoordsPerField}};
    #undef P
    KW<IntVector, DataResponsesRep> *kw;
    if ((kw = (KW<IntVector, DataResponsesRep>*)Binsearch(IVdr, L)))
      return dbRep->dataResponsesIter->dataRespRep->*kw->p;
  }
  Bad_name(entry_name, "get_iv");
  return abort_handler_t<const IntVector&>(PARSE_ERROR);
}


const BitArray& ProblemDescDB::get_ba(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
  	Null_rep("get_ba");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<BitArray, DataVariablesRep> BAdv[] = { 
      // must be sorted by string (key)
	{"binomial_uncertain.categorical", P binomialUncCat},
	{"discrete_design_range.categorical", P discreteDesignRangeCat},
	{"discrete_design_set_int.categorical", P discreteDesignSetIntCat},
	{"discrete_design_set_real.categorical", P discreteDesignSetRealCat},
	{"discrete_interval_uncertain.categorical", P discreteIntervalUncCat},
	{"discrete_state_range.categorical", P discreteStateRangeCat},
	{"discrete_state_set_int.categorical", P discreteStateSetIntCat},
	{"discrete_state_set_real.categorical", P discreteStateSetRealCat},
	{"discrete_uncertain_set_int.categorical", P discreteUncSetIntCat},
	{"discrete_uncertain_set_real.categorical", P discreteUncSetRealCat},
	{"geometric_uncertain.categorical", P geometricUncCat},
	{"histogram_uncertain.point_int.categorical",
         P histogramUncPointIntCat},
	{"histogram_uncertain.point_real.categorical",
         P histogramUncPointRealCat},
	{"hypergeometric_uncertain.categorical", P hyperGeomUncCat},
	{"negative_binomial_uncertain.categorical", P negBinomialUncCat},
	{"poisson_uncertain.categorical", P poissonUncCat}};
    #undef P

    KW<BitArray, DataVariablesRep> *kw;
    if ((kw = (KW<BitArray, DataVariablesRep>*)Binsearch(BAdv, L)))
	return dbRep->dataVariablesIter->dataVarsRep->*kw->p;
  }

  Bad_name(entry_name, "get_ba");
  return abort_handler_t<const BitArray&>(PARSE_ERROR);
}


const SizetArray& ProblemDescDB::get_sza(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
  	Null_rep("get_sza");
  if ((L = Begins(entry_name, "method."))) {
    if (dbRep->methodDBLocked)
	Locked_db();
    #define P &DataMethodRep::
    static KW<SizetArray, DataMethodRep> SZAdme[] = {	
      // must be sorted by string (key)
      {"nond.collocation_points", P collocationPoints},
      {"nond.expansion_samples", P expansionSamples},
      {"nond.pilot_samples", P pilotSamples}};
    #undef P

    KW<SizetArray, DataMethodRep> *kw;
    if ((kw = (KW<SizetArray, DataMethodRep>*)Binsearch(SZAdme, L)))
	return dbRep->dataMethodIter->dataMethodRep->*kw->p;
  }

  Bad_name(entry_name, "get_sza");
  return abort_handler_t<const SizetArray&>(PARSE_ERROR);
}


const UShortArray& ProblemDescDB::get_usa(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
  	Null_rep("get_usa");
  if ((L = Begins(entry_name, "method."))) {
    if (dbRep->methodDBLocked)
	Locked_db();
    #define P &DataMethodRep::
    static KW<UShortArray, DataMethodRep> USAdme[] = {	
      // must be sorted by string (key)
	{"nond.expansion_order", P expansionOrder},
	{"nond.quadrature_order", P quadratureOrder},
	{"nond.sparse_grid_level", P sparseGridLevel},
	{"nond.tensor_grid_order", P tensorGridOrder},
	{"partitions", P varPartitions}};
    #undef P

    KW<UShortArray, DataMethodRep> *kw;
    if ((kw = (KW<UShortArray, DataMethodRep>*)Binsearch(USAdme, L)))
	return dbRep->dataMethodIter->dataMethodRep->*kw->p;
  }

  Bad_name(entry_name, "get_usa");
  return abort_handler_t<const UShortArray&>(PARSE_ERROR);
}


const RealSymMatrix& ProblemDescDB::get_rsm(const String& entry_name) const
{
  if (!dbRep)
	Null_rep("get_rsm");
  if (strbegins(entry_name, "variables.")) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    if (strends(entry_name, "uncertain.correlation_matrix"))
      return dbRep->dataVariablesIter->dataVarsRep->uncertainCorrelations;
  }
  Bad_name(entry_name, "get_rsm");
  return abort_handler_t<const RealSymMatrix&>(PARSE_ERROR);
}


const RealVectorArray& ProblemDescDB::get_rva(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
    Null_rep("get_rva");
  if ((L = Begins(entry_name, "method."))) {
    if (dbRep->methodDBLocked)
	Locked_db();
    #define P &DataMethodRep::
    static KW<RealVectorArray, DataMethodRep> RVAdme[] = { 
      // must be sorted by string (key)
	{"nond.gen_reliability_levels", P genReliabilityLevels},
	{"nond.probability_levels", P probabilityLevels},
	{"nond.reliability_levels", P reliabilityLevels},
	{"nond.response_levels", P responseLevels}};
    #undef P

    KW<RealVectorArray, DataMethodRep> *kw;
    if ((kw = (KW<RealVectorArray, DataMethodRep>*)Binsearch(RVAdme, L)))
	return dbRep->dataMethodIter->dataMethodRep->*kw->p;
  }

  Bad_name(entry_name, "get_rva");
  return abort_handler_t<const RealVectorArray&>(PARSE_ERROR);
}


const IntVectorArray& ProblemDescDB::get_iva(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
    Null_rep("get_iva");
  // BMA: no current use cases
  Bad_name(entry_name, "get_iva");
  return abort_handler_t<const IntVectorArray&>(PARSE_ERROR);
}


const IntSet& ProblemDescDB::get_is(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_is");
  if (strbegins(entry_name, "model.")) {
    if (dbRep->modelDBLocked)
	Locked_db();
    if (strends(entry_name, "surrogate.function_indices"))
      return dbRep->dataModelIter->dataModelRep->surrogateFnIndices;
  }
  else if ((L = Begins(entry_name, "responses."))) {
    if (dbRep->responsesDBLocked)
	Locked_db();
    #define P &DataResponsesRep::
    static KW<IntSet, DataResponsesRep> ISdr[] = {	
      // must be sorted by string (key)
	{"gradients.mixed.id_analytic", P idAnalyticGrads},
	{"gradients.mixed.id_numerical", P idNumericalGrads},
	{"hessians.mixed.id_analytic", P idAnalyticHessians},
	{"hessians.mixed.id_numerical", P idNumericalHessians},
	{"hessians.mixed.id_quasi", P idQuasiHessians}};
    #undef P

    KW<IntSet, DataResponsesRep> *kw;
    if ((kw = (KW<IntSet, DataResponsesRep>*)Binsearch(ISdr, L)))
	return dbRep->dataResponsesIter->dataRespRep->*kw->p;
  }
  Bad_name(entry_name, "get_is");
  return abort_handler_t<const IntSet&>(PARSE_ERROR);
}


const IntSetArray& ProblemDescDB::get_isa(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_isa");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<IntSetArray, DataVariablesRep> ISAdv[] = { 
      // must be sorted by string (key)
	{"discrete_design_set_int.values", P discreteDesignSetInt},
	{"discrete_state_set_int.values", P discreteStateSetInt}};
    #undef P

    KW<IntSetArray, DataVariablesRep> *kw;
    if ((kw = (KW<IntSetArray, DataVariablesRep>*)Binsearch(ISAdv, L)))
	return dbRep->dataVariablesIter->dataVarsRep->*kw->p;
  }
  Bad_name(entry_name, "get_isa");
  return abort_handler_t<const IntSetArray&>(PARSE_ERROR);
}

const StringSetArray& ProblemDescDB::get_ssa(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_ssa");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<StringSetArray, DataVariablesRep> SSAdv[] = { 
      // must be sorted by string (key)
      {"discrete_design_set_string.values", P discreteDesignSetStr},
      {"discrete_state_set_string.values", P discreteStateSetStr}};
    #undef P

    KW<StringSetArray, DataVariablesRep> *kw;
    if ((kw = (KW<StringSetArray, DataVariablesRep>*)Binsearch(SSAdv, L)))
	return dbRep->dataVariablesIter->dataVarsRep->*kw->p;
  }
  Bad_name(entry_name, "get_ssa");
  return abort_handler_t<const StringSetArray&>(PARSE_ERROR);
}


const RealSetArray& ProblemDescDB::get_rsa(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_rsa()");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<RealSetArray, DataVariablesRep> RSAdv[] = { 
      // must be sorted by string (key)
	{"discrete_design_set_real.values", P discreteDesignSetReal},
	{"discrete_state_set_real.values", P discreteStateSetReal}};
    #undef P

    KW<RealSetArray, DataVariablesRep> *kw;
    if ((kw = (KW<RealSetArray, DataVariablesRep>*)Binsearch(RSAdv, L)))
	return dbRep->dataVariablesIter->dataVarsRep->*kw->p;
  }
  Bad_name(entry_name, "get_rsa");
  return abort_handler_t<const RealSetArray&>(PARSE_ERROR);
}


const IntRealMapArray& ProblemDescDB::get_irma(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_irma");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<IntRealMapArray, DataVariablesRep> IRMAdv[] = { 
      // must be sorted by string (key)
	{"discrete_uncertain_set_int.values_probs",
	 P discreteUncSetIntValuesProbs},
	{"histogram_uncertain.point_int_pairs", P histogramUncPointIntPairs}};
    #undef P

    KW<IntRealMapArray, DataVariablesRep> *kw;
    if ((kw = (KW<IntRealMapArray, DataVariablesRep>*)Binsearch(IRMAdv, L)))
	return dbRep->dataVariablesIter->dataVarsRep->*kw->p;
  }
  Bad_name(entry_name, "get_irma");
  return abort_handler_t<const IntRealMapArray&>(PARSE_ERROR);
}

const StringRealMapArray& ProblemDescDB::get_srma(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_srma");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<StringRealMapArray, DataVariablesRep> SRMAdv[] = { 
      // must be sorted by string (key)
	{"discrete_uncertain_set_string.values_probs",
	 P discreteUncSetStrValuesProbs},
	{"histogram_uncertain.point_string_pairs", P histogramUncPointStrPairs}};
    #undef P

    KW<StringRealMapArray, DataVariablesRep> *kw;
    if ((kw = (KW<StringRealMapArray, DataVariablesRep>*)Binsearch(SRMAdv, L)))
	return dbRep->dataVariablesIter->dataVarsRep->*kw->p;
  }
  Bad_name(entry_name, "get_srma");
  return abort_handler_t<const StringRealMapArray&>(PARSE_ERROR);
}


const RealRealMapArray& ProblemDescDB::get_rrma(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_rrma()");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<RealRealMapArray, DataVariablesRep> RRMAdv[] = { 
      // must be sorted by string (key)
	{"discrete_uncertain_set_real.values_probs",
	 P discreteUncSetRealValuesProbs},
	{"histogram_uncertain.bin_pairs",   P histogramUncBinPairs},
	{"histogram_uncertain.point_real_pairs", P histogramUncPointRealPairs}};
    #undef P

    KW<RealRealMapArray, DataVariablesRep> *kw;
    if ((kw = (KW<RealRealMapArray, DataVariablesRep>*)Binsearch(RRMAdv, L)))
	return dbRep->dataVariablesIter->dataVarsRep->*kw->p;
  }
  Bad_name(entry_name, "get_rrma");
  return abort_handler_t<const RealRealMapArray&>(PARSE_ERROR);
}


const RealRealPairRealMapArray& ProblemDescDB::
get_rrrma(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_rrrma()");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<RealRealPairRealMapArray, DataVariablesRep> RRRMAdv[] = {
      
      // must be sorted by string (key)
      {"continuous_interval_uncertain.basic_probs",
       P continuousIntervalUncBasicProbs}};
    #undef P

    KW<RealRealPairRealMapArray, DataVariablesRep> *kw;
    if ((kw = (KW<RealRealPairRealMapArray, DataVariablesRep>*)
	 Binsearch(RRRMAdv, L)))
	return dbRep->dataVariablesIter->dataVarsRep->*kw->p;
  }
  Bad_name(entry_name, "get_rrrma");
  return abort_handler_t<const RealRealPairRealMapArray&>(PARSE_ERROR);
}


const IntIntPairRealMapArray& ProblemDescDB::
get_iirma(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_iirma()");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<IntIntPairRealMapArray, DataVariablesRep> IIRMAdv[] = {
      
      // must be sorted by string (key)
      {"discrete_interval_uncertain.basic_probs",
       P discreteIntervalUncBasicProbs}};
    #undef P

    KW<IntIntPairRealMapArray, DataVariablesRep> *kw;
    if ((kw = (KW<IntIntPairRealMapArray, DataVariablesRep>*)
	 Binsearch(IIRMAdv, L)))
	return dbRep->dataVariablesIter->dataVarsRep->*kw->p;
  }
  Bad_name(entry_name, "get_iirma");
  return abort_handler_t<const IntIntPairRealMapArray&>(PARSE_ERROR);
}


const StringArray& ProblemDescDB::get_sa(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_sa");
  // if ((L = Begins(entry_name, "environment."))) {
  //   #define P &DataEnvironmentRep::
  //   static KW<StringArray, DataEnvironmentRep> SAenv[] = {	
  //     // must be sorted by string (key)
  //     {"env_options", P envOptions}};
  //   #undef P

  //   KW<StringArray, DataEnvironmentRep> *kw;
  //   if ((kw = (KW<StringArray, DataEnvironmentRep>*)Binsearch(SAenv, L)))
  // 	return dbRep->environmentSpec.dataEnvRep->*kw->p;
  // }
  // else 
  if ((L = Begins(entry_name, "method."))) {
    if (dbRep->methodDBLocked)
	Locked_db();
    #define P &DataMethodRep::
    static KW<StringArray, DataMethodRep> SAds[] = {	
      // must be sorted by string (key)
	{"coliny.misc_options", P miscOptions},
	{"hybrid.method_names", P hybridMethodNames},
	{"hybrid.method_pointers", P hybridMethodPointers},
	{"hybrid.model_pointers", P hybridModelPointers}};
    #undef P

    KW<StringArray, DataMethodRep> *kw;
    if ((kw = (KW<StringArray, DataMethodRep>*)Binsearch(SAds, L)))
	return dbRep->dataMethodIter->dataMethodRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "model."))) {
    if (dbRep->modelDBLocked)
	Locked_db();
    #define P &DataModelRep::
    static KW<StringArray, DataModelRep> SAdmo[] = {	
      // must be sorted by string (key)
	{"metrics", P diagMetrics},
	{"nested.primary_variable_mapping", P primaryVarMaps},
	{"nested.secondary_variable_mapping", P secondaryVarMaps},
	{"surrogate.ordered_model_pointers", P orderedModelPointers}};
    #undef P

    KW<StringArray, DataModelRep> *kw;
    if ((kw = (KW<StringArray, DataModelRep>*)Binsearch(SAdmo, L)))
	return dbRep->dataModelIter->dataModelRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<StringArray, DataVariablesRep> SAdv[] = {	
      // must be sorted by string (key)
	{"continuous_aleatory_uncertain.labels", P continuousAleatoryUncLabels},
	{"continuous_design.labels", P continuousDesignLabels},
	{"continuous_design.scale_types", P continuousDesignScaleTypes},
	{"continuous_epistemic_uncertain.labels",
	 P continuousEpistemicUncLabels},
	{"continuous_state.labels", P continuousStateLabels},
	{"discrete_aleatory_uncertain_int.labels",
	 P discreteIntAleatoryUncLabels},
	{"discrete_aleatory_uncertain_real.labels",
	 P discreteRealAleatoryUncLabels},
	{"discrete_aleatory_uncertain_string.initial_point",
	 P discreteStrAleatoryUncVars},
	{"discrete_aleatory_uncertain_string.labels",
	 P discreteStrAleatoryUncLabels},
	{"discrete_aleatory_uncertain_string.lower_bounds",
	 P discreteStrAleatoryUncLowerBnds},
	{"discrete_aleatory_uncertain_string.upper_bounds",
	 P discreteStrAleatoryUncUpperBnds},
	{"discrete_design_range.labels", P discreteDesignRangeLabels},
	{"discrete_design_set_int.labels", P discreteDesignSetIntLabels},
	{"discrete_design_set_real.labels", P discreteDesignSetRealLabels},
	{"discrete_design_set_string.initial_point", P discreteDesignSetStrVars},
	{"discrete_design_set_string.labels", P discreteDesignSetStrLabels},
	{"discrete_design_set_string.lower_bounds", P discreteDesignSetStrLowerBnds},
	{"discrete_design_set_string.upper_bounds", P discreteDesignSetStrUpperBnds},
	{"discrete_epistemic_uncertain_int.labels",
	 P discreteIntEpistemicUncLabels},
	{"discrete_epistemic_uncertain_real.labels",
	 P discreteRealEpistemicUncLabels},
	{"discrete_epistemic_uncertain_string.initial_point",
	 P discreteStrEpistemicUncVars},
	{"discrete_epistemic_uncertain_string.labels",
	 P discreteStrEpistemicUncLabels},
	{"discrete_epistemic_uncertain_string.lower_bounds",
	 P discreteStrEpistemicUncLowerBnds},
	{"discrete_epistemic_uncertain_string.upper_bounds",
	 P discreteStrEpistemicUncUpperBnds},
	{"discrete_state_range.labels", P discreteStateRangeLabels},
	{"discrete_state_set_int.labels", P discreteStateSetIntLabels},
	{"discrete_state_set_real.labels", P discreteStateSetRealLabels},
	{"discrete_state_set_string.initial_state", P discreteStateSetStrVars},
	{"discrete_state_set_string.labels", P discreteStateSetStrLabels},
	{"discrete_state_set_string.lower_bounds", P discreteStateSetStrLowerBnds},
	{"discrete_state_set_string.upper_bounds", P discreteStateSetStrUpperBnds},
	{"discrete_uncertain_set_string.initial_point", P discreteUncSetStrVars},
	{"linear_equality_scale_types", P linearEqScaleTypes},
	{"linear_inequality_scale_types", P linearIneqScaleTypes}};
    #undef P

    KW<StringArray, DataVariablesRep> *kw;
    if ((kw = (KW<StringArray, DataVariablesRep>*)Binsearch(SAdv, L)))
	return dbRep->dataVariablesIter->dataVarsRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "interface."))) {
    if (dbRep->interfaceDBLocked)
	Locked_db();
    #define P &DataInterfaceRep::
    static KW<StringArray, DataInterfaceRep> SAdi[] = {	
      // must be sorted by string (key)
	{ "application.analysis_drivers", P analysisDrivers},
	{ "copyFiles", P copyFiles},
	{ "linkFiles", P linkFiles}};
    #undef P

    KW<StringArray, DataInterfaceRep> *kw;
    if ((kw = (KW<StringArray, DataInterfaceRep>*)Binsearch(SAdi, L)))
	return dbRep->dataInterfaceIter->dataIfaceRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "responses."))) {
    if (dbRep->responsesDBLocked)
	Locked_db();
    #define P &DataResponsesRep::
    static KW<StringArray, DataResponsesRep> SAdr[] = {	
      // must be sorted by string (key)
	{ "labels", P responseLabels},
	{ "nonlinear_equality_scale_types", P nonlinearEqScaleTypes},
	{ "nonlinear_inequality_scale_types", P nonlinearIneqScaleTypes},
	{ "primary_response_fn_scale_types", P primaryRespFnScaleTypes},
	{ "primary_response_fn_sense", P primaryRespFnSense},
	{ "variance_type", P varianceType}};
    #undef P

    KW<StringArray, DataResponsesRep> *kw;
    if ((kw = (KW<StringArray, DataResponsesRep>*)Binsearch(SAdr, L)))
	return dbRep->dataResponsesIter->dataRespRep->*kw->p;
  }
  Bad_name(entry_name, "get_sa");
  return abort_handler_t<const StringArray&>(PARSE_ERROR);
}


const String2DArray& ProblemDescDB::get_s2a(const String& entry_name) const
{
  if (!dbRep)
	Null_rep("get_2sa");
  if (strbegins(entry_name, "interface.")) {
    if (dbRep->interfaceDBLocked)
	Locked_db();
    if (strends(entry_name, "application.analysis_components"))
      return dbRep->dataInterfaceIter->dataIfaceRep->analysisComponents;
  }
  Bad_name(entry_name, "get_s2a");
  return abort_handler_t<const String2DArray&>(PARSE_ERROR);
}


const String& ProblemDescDB::get_string(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_string");
  if ((L = Begins(entry_name, "environment."))) {
    #define P &DataEnvironmentRep::
    static KW<String, DataEnvironmentRep> Sde[] = { 
      // must be sorted by string (key)
	{"error_file", P errorFile},
	{"output_file", P outputFile},
	{"post_run_input", P postRunInput},
	{"post_run_output", P postRunOutput},
	{"pre_run_input", P preRunInput},
	{"pre_run_output", P preRunOutput},
	{"read_restart", P readRestart},
	{"results_output_file", P resultsOutputFile},
	{"run_input", P runInput},
	{"run_output", P runOutput},
	{"tabular_graphics_file", P tabularDataFile},
	{"top_method_pointer", P topMethodPointer},
	{"write_restart", P writeRestart}};
    #undef P

    KW<String, DataEnvironmentRep> *kw;
    if ((kw = (KW<String, DataEnvironmentRep>*)Binsearch(Sde, L)))
	return dbRep->environmentSpec.dataEnvRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "method."))) {
    if (dbRep->methodDBLocked)
	Locked_db();
    #define P &DataMethodRep::
    static KW<String, DataMethodRep> Sdme[] = {	
      // must be sorted by string (key)
	{"asynch_pattern_search.merit_function", P meritFunction},
	{"batch_selection", P batchSelectionType},
	{"coliny.beta_solver_name", P betaSolverName},
	{"coliny.division", P boxDivision},
	{"coliny.exploratory_moves", P exploratoryMoves},
	{"coliny.pattern_basis", P patternBasis},
	{"crossover_type", P crossoverType},
	{"dl_solver.dlDetails", P dlDetails},
	{"export_approx_points_file", P exportApproxPtsFile},
	{"fitness_metric", P fitnessMetricType},
	{"fitness_type", P fitnessType},
	{"flat_file", P flatFile},
	{"hybrid.global_method_name", P hybridGlobalMethodName},
	{"hybrid.global_method_pointer", P hybridGlobalMethodPointer},
	{"hybrid.global_model_pointer", P hybridGlobalModelPointer},
	{"hybrid.local_method_name", P hybridLocalMethodName},
	{"hybrid.local_method_pointer", P hybridLocalMethodPointer},
	{"hybrid.local_model_pointer", P hybridLocalModelPointer},
	{"id", P idMethod},
	{"import_approx_points_file", P importApproxPtsFile},
	{"import_build_points_file", P importBuildPtsFile},
	{"import_candidate_points_file", P importCandPtsFile},
	{"import_prediction_configs", P importPredConfigs},
	{"initialization_type", P initializationType},
	{"jega.convergence_type", P convergenceType},
	{"jega.niching_type", P nichingType},
	{"jega.postprocessor_type", P postProcessorType},
	{"lipschitz", P lipschitzType},
	{"log_file", P logFile},
	{"low_fidelity_model_pointer", P lowFidModelPointer},
	{"mesh_adaptive_search.display_format", P displayFormat},
	{"mesh_adaptive_search.history_file", P historyFile},
	{"mesh_adaptive_search.use_surrogate", P useSurrogate},
	{"model_pointer", P modelPointer},
	{"mutation_type", P mutationType},
	{"nond.data_dist_cov_type", P dataDistCovInputType},
        {"nond.data_dist_filename", P dataDistFile},
	{"nond.data_dist_type", P dataDistType},
	{"nond.discrepancy_type", P discrepancyType},
	{"nond.expansion_sample_type", P expansionSampleType},
	{"nond.export_corrected_model_file", P exportCorrModelFile},
	{"nond.export_corrected_variance_file", P exportCorrVarFile},
	{"nond.export_discrepancy_file", P exportDiscrepFile},
	{"nond.export_expansion_file", P exportExpansionFile},
	{"nond.export_mcmc_points_file", P exportMCMCPtsFile},
	{"nond.import_expansion_file", P importExpansionFile},
	{"nond.mcmc_type", P mcmcType},
	{"nond.point_reuse", P pointReuse},
	{"nond.posterior_density_export_file", P posteriorDensityExportFilename},
	{"nond.posterior_samples_export_file", P posteriorSamplesExportFilename},
	{"nond.posterior_samples_import_file", P posteriorSamplesImportFilename},
	{"nond.proposal_covariance_filename", P proposalCovFile},
	{"nond.proposal_covariance_input_type", P proposalCovInputType},
	{"nond.proposal_covariance_type", P proposalCovType},
	{"nond.reliability_integration", P reliabilityIntegration},
	{"optpp.search_method", P searchMethod},
	{"pattern_search.synchronization", P evalSynchronize},
	{"pstudy.import_file", P pstudyFilename},
	{"queso_options_file", P quesoOptionsFilename},
	{"random_number_generator", P rngName},
	{"replacement_type", P replacementType},
	{"sub_method_name", P subMethodName},
	{"sub_method_pointer", P subMethodPointer},
	{"sub_model_pointer", P subModelPointer},
	{"trial_type", P trialType}};
    #undef P

    KW<String, DataMethodRep> *kw;
    if ((kw = (KW<String, DataMethodRep>*)Binsearch(Sdme, L)))
	return dbRep->dataMethodIter->dataMethodRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "model."))) {
    if (dbRep->modelDBLocked)
	Locked_db();
    #define P &DataModelRep::
    static KW<String, DataModelRep> Sdmo[] = {	
      // must be sorted by string (key)
	{"dace_method_pointer", P subMethodPointer},
	{"id", P idModel},
	{"interface_pointer", P interfacePointer},
	{"nested.sub_method_pointer", P subMethodPointer},
	{"optional_interface_responses_pointer", P optionalInterfRespPointer},
	{"rf.propagation_model_pointer", P propagationModelPointer},
	{"rf_data_file", P rfDataFileName},
	{"simulation.solution_level_control", P solutionLevelControl},
	{"surrogate.actual_model_pointer", P actualModelPointer},
	{"surrogate.challenge_points_file", P importChallengePtsFile},
	{"surrogate.decomp_cell_type", P decompCellType},
	{"surrogate.export_approx_points_file", P exportApproxPtsFile},
	{"surrogate.import_build_points_file", P importBuildPtsFile},
	{"surrogate.kriging_opt_method", P krigingOptMethod},
	{"surrogate.mars_interpolation", P marsInterpolation},
	{"surrogate.model_export_prefix", P modelExportPrefix},
	{"surrogate.point_reuse", P approxPointReuse},
	{"surrogate.refine_cv_metric", P refineCVMetric},
        {"surrogate.trend_order", P trendOrder},
	{"surrogate.type", P surrogateType},
	{"type", P modelType}};
    #undef P

    KW<String, DataModelRep> *kw;
    if ((kw = (KW<String, DataModelRep>*)Binsearch(Sdmo, L)))
	return dbRep->dataModelIter->dataModelRep->*kw->p;
  }
  else if (strbegins(entry_name, "variables.")) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    if (strends(entry_name, "id"))
      return dbRep->dataVariablesIter->dataVarsRep->idVariables;
  }
  else if ((L = Begins(entry_name, "interface."))) {
    if (dbRep->interfaceDBLocked)
	Locked_db();
    #define P &DataInterfaceRep::
    static KW<String, DataInterfaceRep> Sdi[] = {	
      // must be sorted by string (key)
	{"algebraic_mappings", P algebraicMappings},
	{"application.input_filter", P inputFilter},
	{"application.output_filter", P outputFilter},
	{"application.parameters_file", P parametersFile},
	{"application.results_file", P resultsFile},
	{"failure_capture.action", P failAction},
	{"id", P idInterface},
	{"workDir", P workDir}};
    #undef P

    KW<String, DataInterfaceRep> *kw;
    if ((kw = (KW<String, DataInterfaceRep>*)Binsearch(Sdi, L)))
	return dbRep->dataInterfaceIter->dataIfaceRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "responses."))) {
    if (dbRep->responsesDBLocked)
	Locked_db();
    #define P &DataResponsesRep::
    static KW<String, DataResponsesRep> Sdr[] = {	
      // must be sorted by string (key)
	{"fd_gradient_step_type", P fdGradStepType},
	{"fd_hessian_step_type", P fdHessStepType},
	{"gradient_type", P gradientType},
	{"hessian_type", P hessianType},
	{"id", P idResponses},
	{"interval_type", P intervalType},
	{"method_source", P methodSource},
	{"quasi_hessian_type", P quasiHessianType},	
	{"scalar_data_filename", P scalarDataFileName}};
    #undef P

    KW<String, DataResponsesRep> *kw;
    if ((kw = (KW<String, DataResponsesRep>*)Binsearch(Sdr, L)))
	return dbRep->dataResponsesIter->dataRespRep->*kw->p;

  }
  Bad_name(entry_name, "get_string");
  return abort_handler_t<const String&>(PARSE_ERROR);
}


const Real& ProblemDescDB::get_real(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_real");
  if ((L = Begins(entry_name, "method."))) {
    if (dbRep->methodDBLocked)
	Locked_db();
    #define P &DataMethodRep::
    static KW<Real, DataMethodRep> Rdme[] = {	
      // must be sorted by string (key)
	{"asynch_pattern_search.constraint_penalty", P constrPenalty},
	{"asynch_pattern_search.contraction_factor", P contractStepLength},
	{"asynch_pattern_search.initial_delta", P initStepLength},
	{"asynch_pattern_search.smoothing_factor", P smoothFactor},
	{"asynch_pattern_search.solution_target", P solnTarget},
	{"asynch_pattern_search.threshold_delta", P threshStepLength},
	{"coliny.contraction_factor", P contractFactor},
	{"coliny.global_balance_parameter", P globalBalanceParam},
	{"coliny.initial_delta", P initDelta},
	{"coliny.local_balance_parameter", P localBalanceParam},
	{"coliny.max_boxsize_limit", P maxBoxSize},
	{"coliny.threshold_delta", P threshDelta},
	{"confidence_level", P wilksConfidenceLevel},
	{"constraint_penalty", P constraintPenalty},
	{"constraint_tolerance", P constraintTolerance},
	{"convergence_tolerance", P convergenceTolerance},
	{"crossover_rate", P crossoverRate},
	{"dream.gr_threshold", P grThreshold},
	{"function_precision", P functionPrecision},
	{"hybrid.local_search_probability", P hybridLSProb},
	{"jega.fitness_limit", P fitnessLimit},
	{"jega.percent_change", P convergenceTolerance},
	{"jega.shrinkage_percentage", P shrinkagePercent},
	{"mesh_adaptive_search.initial_delta", P initMeshSize},
	{"mesh_adaptive_search.threshold_delta", P minMeshSize},
	{"mesh_adaptive_search.variable_neighborhood_search", P vns},
	{"min_boxsize_limit", P minBoxSize},
	{"mutation_rate", P mutationRate},
	{"mutation_scale", P mutationScale},
	{"nl2sol.absolute_conv_tol", P absConvTol},
	{"nl2sol.false_conv_tol", P falseConvTol},
	{"nl2sol.initial_trust_radius", P initTRRadius},
	{"nl2sol.singular_conv_tol", P singConvTol},
	{"nl2sol.singular_radius", P singRadius},
	{"nl2sol.x_conv_tol", P xConvTol},
	{"nond.collocation_ratio", P collocationRatio},
	{"nond.collocation_ratio_terms_order", P collocRatioTermsOrder},
	{"nond.regression_penalty", P regressionL2Penalty},
	{"npsol.linesearch_tolerance", P lineSearchTolerance},
	{"optpp.centering_parameter", P centeringParam},
	{"optpp.gradient_tolerance", P gradientTolerance},
	{"optpp.max_step", P maxStep},
	{"optpp.steplength_to_boundary", P stepLenToBoundary},
	{"percent_variance_explained", P percentVarianceExplained},
	{"solution_target", P solnTarget},
	{"trust_region.contract_threshold", P trustRegionContractTrigger},
	{"trust_region.contraction_factor", P trustRegionContract},
	{"trust_region.expand_threshold", P trustRegionExpandTrigger},
	{"trust_region.expansion_factor", P trustRegionExpand},
	{"trust_region.minimum_size", P trustRegionMinSize},
	{"vbd_drop_tolerance", P vbdDropTolerance},
	{"verification.refinement_rate", P refinementRate},
	{"volume_boxsize_limit", P volBoxSize}};
    #undef P

    KW<Real, DataMethodRep> *kw;
    if ((kw = (KW<Real, DataMethodRep>*)Binsearch(Rdme, L)))
	return dbRep->dataMethodIter->dataMethodRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "model."))) {
    if (dbRep->modelDBLocked)
	Locked_db();
    #define P &DataModelRep::
    static KW<Real, DataModelRep> Rdmo[] = {	
      // must be sorted by string (key)
      {"active_subspace.cv.decrease_tolerance", P decreaseTolerance},
      {"active_subspace.cv.relative_tolerance", P relTolerance},
      {"active_subspace.truncation_method.energy.truncation_tolerance", P truncationTolerance},
      {"adapted_basis.collocation_ratio", P adaptedBasisCollocRatio},
      {"convergence_tolerance", P convergenceTolerance},
      {"surrogate.discont_grad_thresh", P discontGradThresh},
      {"surrogate.discont_jump_thresh", P discontJumpThresh},
      {"surrogate.neural_network_range", P annRange},
      {"surrogate.nugget", P krigingNugget},
      {"surrogate.percent", P percentFold},
      {"truncation_tolerance", P truncationTolerance}};
    #undef P

    KW<Real, DataModelRep> *kw;
    if ((kw = (KW<Real, DataModelRep>*)Binsearch(Rdmo, L)))
	return dbRep->dataModelIter->dataModelRep->*kw->p;
  }
  else if (strbegins(entry_name, "interface.")) {
    if (strends(entry_name, "nearby_evaluation_cache_tolerance"))
      return dbRep->dataInterfaceIter->dataIfaceRep->nearbyEvalCacheTol;
  }
  Bad_name(entry_name, "get_real");
  return abort_handler_t<const Real&>(PARSE_ERROR);
}


int ProblemDescDB::get_int(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_int");
  if ((L = Begins(entry_name, "environment."))) {
    #define P &DataEnvironmentRep::
    static KW<int, DataEnvironmentRep> Ide[] = { 
      // must be sorted by string (key)
        {"output_precision", P outputPrecision},
        {"stop_restart", P stopRestart}};
    #undef P

    KW<int, DataEnvironmentRep> *kw;
    if ((kw = (KW<int, DataEnvironmentRep>*)Binsearch(Ide, L)))
	return dbRep->environmentSpec.dataEnvRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "method."))) {
    if (dbRep->methodDBLocked)
	Locked_db();
    #define P &DataMethodRep::
    static KW<int, DataMethodRep> Idme[] = {	
      // must be sorted by string (key)
	{"build_samples", P buildSamples},
	{"burn_in_samples", P burnInSamples},
	{"coliny.contract_after_failure", P contractAfterFail},
	{"coliny.expand_after_success", P expandAfterSuccess},
	{"coliny.mutation_range", P mutationRange},
	{"coliny.new_solutions_generated", P newSolnsGenerated},
	{"coliny.number_retained", P numberRetained},
	{"coliny.total_pattern_size", P totalPatternSize},
	{"concurrent.random_jobs", P concurrentRandomJobs},
	{"dream.crossover_chain_pairs", P crossoverChainPairs},
	{"dream.jump_step", P jumpStep},
	{"dream.num_chains", P numChains},
	{"dream.num_cr", P numCR},
	{"fsu_cvt.num_trials", P numTrials},
	{"iterator_servers", P iteratorServers},
	{"max_function_evaluations", P maxFunctionEvaluations},
	{"max_hifi_evaluations", P maxHifiEvals},
	{"max_iterations", P maxIterations},
	{"mesh_adaptive_search.neighbor_order", P neighborOrder},
	{"nl2sol.covariance", P covarianceType},
	{"nond.chain_samples", P chainSamples},
	{"nond.max_refinement_iterations", P maxRefineIterations},
	{"nond.max_solver_iterations", P maxSolverIterations},
	{"nond.proposal_covariance_updates", P proposalCovUpdates},
	{"nond.samples_on_emulator", P samplesOnEmulator},
	{"nond.surrogate_order", P emulatorOrder},
	{"npsol.verify_level", P verifyLevel},
	{"optpp.search_scheme_size", P searchSchemeSize},
	{"parameter_study.num_steps", P numSteps},
	{"population_size", P populationSize},
	{"processors_per_iterator", P procsPerIterator},
	{"random_seed", P randomSeed},
	{"samples", P numSamples},
	{"sub_sampling_period", P subSamplingPeriod},
	{"symbols", P numSymbols}};
    #undef P

    KW<int, DataMethodRep> *kw;
    if ((kw = (KW<int, DataMethodRep>*)Binsearch(Idme, L)))
	return dbRep->dataMethodIter->dataMethodRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "model."))) {
    if (dbRep->modelDBLocked)
	Locked_db();
    #define P &DataModelRep::
    static KW<int, DataModelRep> Idmo[] = {	
      // must be sorted by string (key)
        {"active_subspace.bootstrap_samples", P numReplicates},
        {"active_subspace.cv.max_rank", P subspaceCVMaxRank},
        {"active_subspace.dimension", P subspaceDimension},
	{"initial_samples", P initialSamples},
	{"max_function_evals", P maxFunctionEvals},
	{"max_iterations", P maxIterations},
	{"nested.iterator_servers", P subMethodServers},
	{"nested.processors_per_iterator", P subMethodProcs},
        {"rf.expansion_bases", P subspaceDimension},
        {"soft_convergence_limit", P softConvergenceLimit},
        {"surrogate.decomp_support_layers", P decompSupportLayers},
        {"surrogate.folds", P numFolds},
        {"surrogate.points_total", P pointsTotal},
        {"surrogate.refine_cv_folds", P refineCVFolds}};
    #undef P

    KW<int, DataModelRep> *kw;
    if ((kw = (KW<int, DataModelRep>*)Binsearch(Idmo, L)))
	return dbRep->dataModelIter->dataModelRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "interface."))) {
    if (dbRep->interfaceDBLocked)
	Locked_db();
    #define P &DataInterfaceRep::
    static KW<int, DataInterfaceRep> Idi[] = {	
      // must be sorted by string (key)
	{"analysis_servers", P analysisServers},
	{"asynch_local_analysis_concurrency", P asynchLocalAnalysisConcurrency},
	{"asynch_local_evaluation_concurrency", P asynchLocalEvalConcurrency},
	{"direct.processors_per_analysis", P procsPerAnalysis},
	{"evaluation_servers", P evalServers},
	{"failure_capture.retry_limit", P retryLimit},
	{"processors_per_evaluation", P procsPerEval}};
    #undef P

    KW<int, DataInterfaceRep> *kw;
    if ((kw = (KW<int, DataInterfaceRep>*)Binsearch(Idi, L)))
	return dbRep->dataInterfaceIter->dataIfaceRep->*kw->p;
  }
  Bad_name(entry_name, "get_int");
  return abort_handler_t<int>(PARSE_ERROR);
}


short ProblemDescDB::get_short(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_short");
  if ((L = Begins(entry_name, "method."))) {
    if (dbRep->methodDBLocked)
	Locked_db();
    #define P &DataMethodRep::
    static KW<short, DataMethodRep> Shdme[] = {	
      // must be sorted by string (key)
	{"iterator_scheduling", P iteratorScheduling},
	{"nond.correction_order", P approxCorrectionOrder},
	{"nond.covariance_control", P covarianceControl},
	{"nond.distribution", P distributionType},
	{"nond.emulator", P emulatorType},
	{"nond.expansion_basis_type", P expansionBasisType},
	{"nond.expansion_refinement_control", P refinementControl},
	{"nond.expansion_refinement_type", P refinementType},
	{"nond.expansion_type", P expansionType},
	{"nond.final_moments", P finalMomentsType},
	{"nond.growth_override", P growthOverride},
	{"nond.least_squares_regression_type", P lsRegressionType},
	{"nond.nesting_override", P nestingOverride},
	{"nond.regression_type", P regressionType},
	{"nond.response_level_target", P responseLevelTarget},
	{"nond.response_level_target_reduce", P responseLevelTargetReduce},
	{"optpp.merit_function", P meritFn},
	{"output", P methodOutput},
	{"sbl.acceptance_logic", P surrBasedLocalAcceptLogic},
	{"sbl.constraint_relax", P surrBasedLocalConstrRelax},
	{"sbl.merit_function", P surrBasedLocalMeritFn},
	{"sbl.subproblem_constraints", P surrBasedLocalSubProbCon},
	{"sbl.subproblem_objective", P surrBasedLocalSubProbObj},
	{"wilks.sided_interval", P wilksSidedInterval}};
    #undef P

    KW<short, DataMethodRep> *kw;
    if ((kw = (KW<short, DataMethodRep>*)Binsearch(Shdme, L)))
	return dbRep->dataMethodIter->dataMethodRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "model."))) {
    if (dbRep->modelDBLocked)
	Locked_db();
    #define P &DataModelRep::
    static KW<short, DataModelRep> Shdmo[] = {	
      // must be sorted by string (key)
	{"nested.iterator_scheduling", P subMethodScheduling},
	{"surrogate.correction_order", P approxCorrectionOrder},
	{"surrogate.correction_type", P approxCorrectionType},
	{"surrogate.find_nugget", P krigingFindNugget},
	{"surrogate.kriging_max_trials", P krigingMaxTrials},
	{"surrogate.mars_max_bases", P marsMaxBases},
	{"surrogate.mls_weight_function", P mlsWeightFunction},
	{"surrogate.neural_network_nodes", P annNodes},
	{"surrogate.neural_network_random_weight", P annRandomWeight},
	{"surrogate.points_management", P pointsManagement},
	{"surrogate.polynomial_order", P polynomialOrder},
	{"surrogate.rbf_bases", P rbfBases},
	{"surrogate.rbf_max_pts", P rbfMaxPts},
	{"surrogate.rbf_max_subsets", P rbfMaxSubsets},
	{"surrogate.rbf_min_partition", P rbfMinPartition}};
    #undef P

    KW<short, DataModelRep> *kw;
    if ((kw = (KW<short, DataModelRep>*)Binsearch(Shdmo, L)))
	return dbRep->dataModelIter->dataModelRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<short, DataVariablesRep> Shdv[] = { 
      // must be sorted by string (key)
	{"domain", P varsDomain},
	{"view", P varsView}};
    #undef P

    KW<short, DataVariablesRep> *kw;
    if ((kw = (KW<short, DataVariablesRep>*)Binsearch(Shdv, L)))
	return dbRep->dataVariablesIter->dataVarsRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "interface."))) {
    if (dbRep->interfaceDBLocked)
	Locked_db();
    #define P &DataInterfaceRep::
    static KW<short, DataInterfaceRep> Shdi[] = { 
      // must be sorted by string (key)
	{"analysis_scheduling", P analysisScheduling},
	{"evaluation_scheduling", P evalScheduling},
	{"local_evaluation_scheduling", P asynchLocalEvalScheduling},
	{"synchronization", P interfaceSynchronization}};
    #undef P

    KW<short, DataInterfaceRep> *kw;
    if ((kw = (KW<short, DataInterfaceRep>*)Binsearch(Shdi, L)))
	return dbRep->dataInterfaceIter->dataIfaceRep->*kw->p;
  }
  Bad_name(entry_name, "get_short");
  return abort_handler_t<short>(PARSE_ERROR);
}


unsigned short ProblemDescDB::get_ushort(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_ushort");
  if ((L = Begins(entry_name, "environment."))) {
    #define P &DataEnvironmentRep::
    static KW<unsigned short, DataEnvironmentRep> UShde[] = { 
      // must be sorted by string (key)
        {"post_run_input_format", P postRunInputFormat},
        {"pre_run_output_format", P preRunOutputFormat},
        {"tabular_format", P tabularFormat}};
    #undef P

    KW<unsigned short, DataEnvironmentRep> *kw;
    if ((kw = (KW<unsigned short, DataEnvironmentRep>*)Binsearch(UShde, L)))
	return dbRep->environmentSpec.dataEnvRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "method."))) {
    if (dbRep->methodDBLocked)
	Locked_db();
    #define P &DataMethodRep::
    static KW<unsigned short, DataMethodRep> UShdme[] = { 
      // must be sorted by string (key)
	{"algorithm", P methodName},
	{"export_approx_format", P exportApproxFormat},
	{"import_approx_format", P importApproxFormat},
	{"import_build_format", P importBuildFormat},
	{"import_candidate_format", P importCandFormat},
	{"import_prediction_configs_format", P importPredConfigFormat},
	{"nond.adapted_basis.advancements", P adaptedBasisAdvancements},
      //{"nond.adapted_basis.initial_level", P adaptedBasisInitLevel},
	{"nond.calibrate_error_mode", P calibrateErrorMode},
	{"nond.cubature_integrand", P cubIntOrder},
	{"nond.export_corrected_model_format", P exportCorrModelFormat},
	{"nond.export_corrected_variance_format", P exportCorrVarFormat},
	{"nond.export_discrep_format", P exportDiscrepFormat},
	{"nond.export_samples_format", P exportSamplesFormat},
	{"nond.integration_refinement", P integrationRefine},
	{"nond.pre_solve_method", P preSolveMethod},
	{"nond.reliability_search_type", P reliabilitySearchType},
	{"nond.vbd_interaction_order", P vbdOrder},
	{"order", P wilksOrder},
	{"pstudy.import_format", P pstudyFileFormat},
	{"sample_type", P sampleType},
	{"soft_convergence_limit", P softConvLimit},
	{"sub_method", P subMethod}};
    #undef P

    KW<unsigned short, DataMethodRep> *kw;
    if ((kw = (KW<unsigned short, DataMethodRep>*)Binsearch(UShdme, L)))
	return dbRep->dataMethodIter->dataMethodRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "model."))) {
    if (dbRep->modelDBLocked)
	Locked_db();
    #define P &DataModelRep::
    static KW<unsigned short, DataModelRep> UShdmo[] = { 
      // must be sorted by string (key)  
	{"active_subspace.cv.id_method", P subspaceIdCVMethod},
	{"active_subspace.normalization", P subspaceNormalization},
	{"active_subspace.sample_type", P subspaceSampleType},
	{"adapted_basis.expansion_order", P adaptedBasisExpOrder},
	{"adapted_basis.sparse_grid_level", P adaptedBasisSparseGridLev},
	{"rf.analytic_covariance", P analyticCovIdForm},
	{"rf.expansion_form", P randomFieldIdForm},
	{"surrogate.challenge_points_file_format", P importChallengeFormat},
	{"surrogate.export_approx_format", P exportApproxFormat},
	{"surrogate.import_build_format", P importBuildFormat},
	{"surrogate.model_export_format", P modelExportFormat}};
    #undef P

    KW<unsigned short, DataModelRep> *kw;
    if ((kw = (KW<unsigned short, DataModelRep>*)Binsearch(UShdmo, L)))
	return dbRep->dataModelIter->dataModelRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "interface."))) {
    if (dbRep->interfaceDBLocked)
	Locked_db();
    #define P &DataInterfaceRep::
    static KW<unsigned short, DataInterfaceRep> UShdi[] = { 
      // must be sorted by string (key)
	{"application.results_file_format", P resultsFileFormat},
        {"type", P interfaceType}};
    #undef P

    KW<unsigned short, DataInterfaceRep> *kw;
    if ((kw = (KW<unsigned short, DataInterfaceRep>*)Binsearch(UShdi, L)))
	return dbRep->dataInterfaceIter->dataIfaceRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "responses."))) {
    if (dbRep->responsesDBLocked)
	Locked_db();
    #define P &DataResponsesRep::
    static KW<unsigned short, DataResponsesRep> UShdr[] = { 
      // must be sorted by string (key)
        {"scalar_data_format", P scalarDataFormat}};
    #undef P

    KW<unsigned short, DataResponsesRep> *kw;
    if ((kw = (KW<unsigned short, DataResponsesRep>*)Binsearch(UShdr, L)))
	return dbRep->dataResponsesIter->dataRespRep->*kw->p;
  }
  Bad_name(entry_name, "get_ushort");
  return abort_handler_t<unsigned short>(PARSE_ERROR);
}


size_t ProblemDescDB::get_sizet(const String& entry_name) const
{
  const char *L;

  if (!dbRep)
	Null_rep("get_sizet");
  if ((L = Begins(entry_name, "method."))) {
    if (dbRep->methodDBLocked)
	Locked_db();
    #define P &DataMethodRep::
    static KW<size_t, DataMethodRep> Szdmo[] = { 
      // must be sorted by string (key)
	{"final_solutions", P numFinalSolutions},
	{"jega.num_cross_points", P numCrossPoints},
	{"jega.num_designs", P numDesigns},
	{"jega.num_generations", P numGenerations},
	{"jega.num_offspring", P numOffspring},
	{"jega.num_parents", P numParents},
	{"num_candidate_designs", P numCandidateDesigns},
	{"num_candidates", P numCandidates},
	{"num_prediction_configs", P numPredConfigs}
    };
    #undef P

    KW<size_t, DataMethodRep> *kw;
    if ((kw = (KW<size_t, DataMethodRep>*)Binsearch(Szdmo, L)))
	return dbRep->dataMethodIter->dataMethodRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    std::list<DataVariables>::iterator v_iter = dbRep->dataVariablesIter;
    DataVariablesRep* VRep = v_iter->dataVarsRep;

    // DataVariables helper functions
    struct HelperFcn { const char *name; int no; };
    static HelperFcn hf[] = {
	{"aleatory_uncertain", 0},
	{"continuous", 1},
	{"design", 2},
	{"discrete", 3},
	{"epistemic_uncertain", 4},
	{"state", 5},
	{"total", 6},
	{"uncertain", 7}};

    // normal DB lookups
    #define P &DataVariablesRep::
    static KW<size_t, DataVariablesRep> Szdv[] = {	
      // must be sorted by string (key)
	{"beta_uncertain", P numBetaUncVars},
	{"binomial_uncertain", P numBinomialUncVars},
	{"continuous_design", P numContinuousDesVars},
	{"continuous_interval_uncertain", P numContinuousIntervalUncVars},
	{"continuous_state", P numContinuousStateVars},
	{"discrete_design_range", P numDiscreteDesRangeVars},
	{"discrete_design_set_int", P numDiscreteDesSetIntVars},
	{"discrete_design_set_real", P numDiscreteDesSetRealVars},
	{"discrete_design_set_string", P numDiscreteDesSetStrVars},
	{"discrete_interval_uncertain", P numDiscreteIntervalUncVars},
	{"discrete_state_range", P numDiscreteStateRangeVars},
	{"discrete_state_set_int", P numDiscreteStateSetIntVars},
	{"discrete_state_set_real", P numDiscreteStateSetRealVars},
	{"discrete_state_set_string", P numDiscreteStateSetStrVars},
	{"discrete_uncertain_set_int", P numDiscreteUncSetIntVars},
	{"discrete_uncertain_set_real", P numDiscreteUncSetRealVars},
	{"discrete_uncertain_set_string", P numDiscreteUncSetStrVars},
	{"exponential_uncertain", P numExponentialUncVars},
	{"frechet_uncertain", P numFrechetUncVars},
	{"gamma_uncertain", P numGammaUncVars},
	{"geometric_uncertain", P numGeometricUncVars},
	{"gumbel_uncertain", P numGumbelUncVars},
	{"histogram_uncertain.bin", P numHistogramBinUncVars},
	{"histogram_uncertain.point_int", P numHistogramPtIntUncVars},
	{"histogram_uncertain.point_real", P numHistogramPtRealUncVars},
	{"histogram_uncertain.point_string", P numHistogramPtStrUncVars},
	{"hypergeometric_uncertain", P numHyperGeomUncVars},
	{"lognormal_uncertain", P numLognormalUncVars},
	{"loguniform_uncertain", P numLoguniformUncVars},
	{"negative_binomial_uncertain", P numNegBinomialUncVars},
	{"normal_uncertain", P numNormalUncVars},
	{"poisson_uncertain", P numPoissonUncVars},
	{"triangular_uncertain", P numTriangularUncVars},
	{"uniform_uncertain", P numUniformUncVars},
	{"weibull_uncertain", P numWeibullUncVars}};
    #undef P

    HelperFcn *kwh;
    KW<size_t, DataVariablesRep> *kw;

    if ((kwh = (HelperFcn*)Binsearch(hf, L)))
	switch(kwh->no) {
	  case 0: return v_iter->aleatory_uncertain();
	  case 1: return v_iter->continuous_variables();
	  case 2: return v_iter->design();
	  case 3: return v_iter->discrete_variables();
	  case 4: return v_iter->epistemic_uncertain();
	  case 5: return v_iter->state();
	  case 6: return v_iter->total_variables();
	  case 7: return v_iter->uncertain();
	  }
    else if ((kw = (KW<size_t, DataVariablesRep>*)Binsearch(Szdv, L)))
	return VRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "responses.num_"))) {
    if (dbRep->responsesDBLocked)
	Locked_db();
    #define P &DataResponsesRep::
    static KW<size_t, DataResponsesRep> Szdr[] = {	
      // must be sorted by string (key)
	{"calibration_terms", P numLeastSqTerms},
	{"config_vars", P numExpConfigVars},
	{"experiments", P numExperiments},
	{"field_calibration_terms", P numFieldLeastSqTerms},
	{"field_nonlinear_equality_constraints",
	 P numFieldNonlinearEqConstraints},
	{"field_nonlinear_inequality_constraints",
	 P numFieldNonlinearIneqConstraints},
	{"field_objectives", P numFieldObjectiveFunctions},
	{"field_responses", P numFieldResponseFunctions},
	{"nonlinear_equality_constraints", P numNonlinearEqConstraints},
	{"nonlinear_inequality_constraints", P numNonlinearIneqConstraints},
	{"objective_functions", P numObjectiveFunctions},
	{"response_functions", P numResponseFunctions},
	{"scalar_calibration_terms", P numScalarLeastSqTerms},
	{"scalar_nonlinear_equality_constraints",
	 P numScalarNonlinearEqConstraints},
	{"scalar_nonlinear_inequality_constraints",
	 P numScalarNonlinearIneqConstraints},
	{"scalar_objectives", P numScalarObjectiveFunctions},
	{"scalar_responses", P numScalarResponseFunctions}};
    #undef P

    KW<size_t, DataResponsesRep> *kw;
    if ((kw = (KW<size_t, DataResponsesRep>*)Binsearch(Szdr, L)))
	return dbRep->dataResponsesIter->dataRespRep->*kw->p;
  }
  Bad_name(entry_name, "get_sizet");
  return abort_handler_t<size_t>(PARSE_ERROR);
}


bool ProblemDescDB::get_bool(const String& entry_name) const
{
  const char *L;
  if (!dbRep)
	Null_rep("get_bool");
  if ((L = Begins(entry_name, "environment."))) {
    #define P &DataEnvironmentRep::
    static KW<bool, DataEnvironmentRep> Bde[] = { 
      // must be sorted by string (key)
	{"check", P checkFlag},
	{"graphics", P graphicsFlag},
	{"post_run", P postRunFlag},
	{"pre_run", P preRunFlag},
	{"results_output", P resultsOutputFlag},
	{"run", P runFlag},
	{"tabular_graphics_data", P tabularDataFlag}};
    #undef P

    KW<bool, DataEnvironmentRep> *kw;
    if ((kw = (KW<bool, DataEnvironmentRep>*)Binsearch(Bde, L)))
	return dbRep->environmentSpec.dataEnvRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "method."))) {
    if (dbRep->methodDBLocked)
	Locked_db();
    #define P &DataMethodRep::
    static KW<bool, DataMethodRep> Bdme[] = {	
      // must be sorted by string (key)
	{"backfill", P backfillFlag},
	{"coliny.constant_penalty", P constantPenalty},
	{"coliny.expansion", P expansionFlag},
	{"coliny.randomize", P randomizeOrderFlag},
	{"coliny.show_misc_options", P showMiscOptions},
	{"derivative_usage", P methodUseDerivsFlag},
	{"fixed_seed", P fixedSeedFlag},
	{"fsu_quasi_mc.fixed_sequence", P fixedSequenceFlag},
	{"import_approx_active_only", P importApproxActive},
	{"import_build_active_only", P importBuildActive},
	{"latinize", P latinizeFlag},
	{"main_effects", P mainEffectsFlag},
	{"mesh_adaptive_search.display_all_evaluations", P showAllEval},
	{"mutation_adaptive", P mutationAdaptive},
	{"nl2sol.regression_diagnostics", P regressDiag},
	{"nond.adapt_exp_design", P adaptExpDesign},
	{"nond.adaptive_posterior_refinement", P adaptPosteriorRefine},
	{"nond.cross_validation", P crossValidation},
	{"nond.cross_validation.noise_only", P crossValidNoiseOnly},
	{"nond.d_optimal", P dOptimal},
	{"nond.evaluate_posterior_density", P evaluatePosteriorDensity},
	{"nond.export_sample_sequence", P exportSampleSeqFlag},
	{"nond.generate_posterior_samples", P generatePosteriorSamples},
	{"nond.gpmsa_normalize", P gpmsaNormalize},
	{"nond.logit_transform", P logitTransform},
	{"nond.model_discrepancy", P calModelDiscrepancy},
	{"nond.normalized", P normalizedCoeffs},
	{"nond.piecewise_basis", P piecewiseBasis},
	{"nond.standardized_space", P standardizedSpace},
	{"nond.tensor_grid", P tensorGridFlag},
	{"posterior_stats.kl_divergence", P posteriorStatsKL},
	{"posterior_stats.mutual_info", P posteriorStatsMutual},
	{"principal_components", P pcaFlag},
	{"print_each_pop", P printPopFlag},
	{"pstudy.import_active_only", P pstudyFileActive},
	{"quality_metrics", P volQualityFlag},
	{"sbg.replace_points", P surrBasedGlobalReplacePts},
	{"sbl.truth_surrogate_bypass", P surrBasedLocalLayerBypass},
	{"scaling", P methodScaling},
	{"speculative", P speculativeFlag},
	{"variance_based_decomp", P vbdFlag},
	{"wilks", P wilksFlag}};
    #undef P

    KW<bool, DataMethodRep> *kw;
    if ((kw = (KW<bool, DataMethodRep>*)Binsearch(Bdme, L)))
	return dbRep->dataMethodIter->dataMethodRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "model."))) {
    if (dbRep->modelDBLocked)
	Locked_db();
    #define P &DataModelRep::
    static KW<bool, DataModelRep> Bdmo[] = {	
      // must be sorted by string (key)
	{"active_subspace.build_surrogate", P subspaceBuildSurrogate},
	{"active_subspace.cv.incremental", P subspaceCVIncremental},
	{"active_subspace.truncation_method.bing_li", P subspaceIdBingLi},
	{"active_subspace.truncation_method.constantine", P subspaceIdConstantine},
	{"active_subspace.truncation_method.cv", P subspaceIdCV},
	{"active_subspace.truncation_method.energy", P subspaceIdEnergy},
	{"hierarchical_tags", P hierarchicalTags},
	{"surrogate.auto_refine", P autoRefine},
	{"surrogate.challenge_points_file_active", P importChallengeActive},
	{"surrogate.cross_validate", P crossValidateFlag},
	{"surrogate.decomp_discont_detect", P decompDiscontDetect},
	{"surrogate.derivative_usage", P modelUseDerivsFlag},
	{"surrogate.domain_decomp", P domainDecomp},
	{"surrogate.export_surrogate", P exportSurrogate},
	{"surrogate.import_build_active_only", P importBuildActive},
	{"surrogate.point_selection", P pointSelection},
	{"surrogate.press", P pressFlag}};
    #undef P

    KW<bool, DataModelRep> *kw;
    if ((kw = (KW<bool, DataModelRep>*)Binsearch(Bdmo, L)))
	return dbRep->dataModelIter->dataModelRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<bool, DataVariablesRep> Bdv[] = {	
      // must be sorted by string (key)
	{"uncertain.initial_point_flag", P uncertainVarsInitPt}};
    #undef P

    KW<bool, DataVariablesRep> *kw;
    if ((kw = (KW<bool, DataVariablesRep>*)Binsearch(Bdv, L)))
	return dbRep->dataVariablesIter->dataVarsRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "interface."))) {
    if (dbRep->interfaceDBLocked)
	Locked_db();
    #define P &DataInterfaceRep::
    static KW<bool, DataInterfaceRep> Bdi[] = {	
      // must be sorted by string (key)
	{"active_set_vector", P activeSetVectorFlag},
	{"allow_existing_results", P allowExistingResultsFlag},
	{"application.aprepro", P apreproFlag},
	{"application.file_save", P fileSaveFlag},
	{"application.file_tag", P fileTagFlag},
	{"application.verbatim", P verbatimFlag},
	{"dirSave", P dirSave},
	{"dirTag", P dirTag},
	{"evaluation_cache", P evalCacheFlag},
	{"nearby_evaluation_cache", P nearbyEvalCacheFlag},
	{"python.numpy", P numpyFlag},
	{"restart_file", P restartFileFlag},
	{"templateReplace", P templateReplace},
	{"useWorkdir", P useWorkdir}};
    #undef P

    KW<bool, DataInterfaceRep> *kw;
    if ((kw = (KW<bool, DataInterfaceRep>*)Binsearch(Bdi, L)))
	return dbRep->dataInterfaceIter->dataIfaceRep->*kw->p;
  }
  else if ((L = Begins(entry_name, "responses."))) {
    if (dbRep->responsesDBLocked)
	Locked_db();
    #define P &DataResponsesRep::
    static KW<bool, DataResponsesRep> Bdr[] = {	
      // must be sorted by string (key)
	{"calibration_data", P calibrationDataFlag},
	{"central_hess", P centralHess},
	{"ignore_bounds", P ignoreBounds},
	{"interpolate", P interpolateFlag},
	{"read_field_coordinates", P readFieldCoords}};
    #undef P

    KW<bool, DataResponsesRep> *kw;
    if ((kw = (KW<bool, DataResponsesRep>*)Binsearch(Bdr, L)))
	return dbRep->dataResponsesIter->dataRespRep->*kw->p;
  }
  Bad_name(entry_name, "get_bool");
  return abort_handler_t<bool>(PARSE_ERROR);
}

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
  const char *L;
  if (!dbRep)
	Null_rep1("set(RealVector&)");
  if ((L = Begins(entry_name, "model.nested."))) {
    if (dbRep->modelDBLocked)
	Locked_db();
    #define P &DataModelRep::
    static KW<RealVector, DataModelRep> RVdmo[] = {	
      // must be sorted by string (key)
	{"primary_response_mapping", P primaryRespCoeffs},
	{"secondary_response_mapping", P secondaryRespCoeffs}};
    #undef P

    KW<RealVector, DataModelRep> *kw;
    if ((kw = (KW<RealVector, DataModelRep>*)Binsearch(RVdmo, L))) {
	dbRep->dataModelIter->dataModelRep->*kw->p = rv;
	return;
	}
  }
  else if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<RealVector, DataVariablesRep> RVdv[] = {	
      // must be sorted by string (key)
	{"beta_uncertain.alphas", P betaUncAlphas},
	{"beta_uncertain.betas", P betaUncBetas},
	{"beta_uncertain.lower_bounds", P betaUncLowerBnds},
	{"beta_uncertain.upper_bounds", P betaUncUpperBnds},
	{"binomial_uncertain.prob_per_trial", P binomialUncProbPerTrial},
	{"continuous_aleatory_uncertain.initial_point",
	 P continuousAleatoryUncVars},
	{"continuous_aleatory_uncertain.lower_bounds",
	 P continuousAleatoryUncLowerBnds},
	{"continuous_aleatory_uncertain.upper_bounds",
	 P continuousAleatoryUncUpperBnds},
	{"continuous_design.initial_point", P continuousDesignVars},
	{"continuous_design.initial_point", P continuousDesignVars},
	{"continuous_design.lower_bounds", P continuousDesignLowerBnds},
	{"continuous_design.lower_bounds", P continuousDesignLowerBnds},
	{"continuous_design.scales", P continuousDesignScales},
	{"continuous_design.upper_bounds", P continuousDesignUpperBnds},
	{"continuous_design.upper_bounds", P continuousDesignUpperBnds},
	{"continuous_epistemic_uncertain.initial_point",
	 P continuousEpistemicUncVars},
	{"continuous_epistemic_uncertain.lower_bounds",
	 P continuousEpistemicUncLowerBnds},
	{"continuous_epistemic_uncertain.upper_bounds",
	 P continuousEpistemicUncUpperBnds},
	{"continuous_state.initial_state", P continuousStateVars},
	{"continuous_state.lower_bounds", P continuousStateLowerBnds},
	{"continuous_state.upper_bounds", P continuousStateUpperBnds},
	{"discrete_aleatory_uncertain_real.initial_point",
	 P discreteRealAleatoryUncVars},
	{"discrete_aleatory_uncertain_real.lower_bounds",
	 P discreteRealAleatoryUncLowerBnds},
	{"discrete_aleatory_uncertain_real.upper_bounds",
	 P discreteRealAleatoryUncUpperBnds},
	{"discrete_design_set_real.initial_point", P discreteDesignSetRealVars},
	{"discrete_design_set_real.initial_point", P discreteDesignSetRealVars},
	{"discrete_epistemic_uncertain_real.initial_point",
	 P discreteRealEpistemicUncVars},
	{"discrete_epistemic_uncertain_real.lower_bounds",
	 P discreteRealEpistemicUncLowerBnds},
	{"discrete_epistemic_uncertain_real.upper_bounds",
	 P discreteRealEpistemicUncUpperBnds},
	{"discrete_state_set_real.initial_state", P discreteStateSetRealVars},
	{"exponential_uncertain.betas", P exponentialUncBetas},
	{"frechet_uncertain.alphas", P frechetUncAlphas},
	{"frechet_uncertain.betas", P frechetUncBetas},
	{"gamma_uncertain.alphas", P gammaUncAlphas},
	{"gamma_uncertain.betas", P gammaUncBetas},
	{"geometric_uncertain.prob_per_trial", P geometricUncProbPerTrial},
	{"gumbel_uncertain.alphas", P gumbelUncAlphas},
	{"gumbel_uncertain.betas", P gumbelUncBetas},
	{"linear_equality_constraints", P linearEqConstraintCoeffs},
	{"linear_equality_scales", P linearEqScales},
	{"linear_equality_targets", P linearEqTargets},
	{"linear_inequality_constraints", P linearIneqConstraintCoeffs},
	{"linear_inequality_lower_bounds", P linearIneqLowerBnds},
	{"linear_inequality_scales", P linearIneqScales},
	{"linear_inequality_upper_bounds", P linearIneqUpperBnds},
	{"lognormal_uncertain.error_factors", P lognormalUncErrFacts},
	{"lognormal_uncertain.lambdas", P lognormalUncLambdas},
	{"lognormal_uncertain.lower_bounds", P lognormalUncLowerBnds},
	{"lognormal_uncertain.means", P lognormalUncMeans},
	{"lognormal_uncertain.std_deviations", P lognormalUncStdDevs},
	{"lognormal_uncertain.upper_bounds", P lognormalUncUpperBnds},
	{"lognormal_uncertain.zetas", P lognormalUncZetas},
	{"loguniform_uncertain.lower_bounds", P loguniformUncLowerBnds},
	{"loguniform_uncertain.upper_bounds", P loguniformUncUpperBnds},
	{"negative_binomial_uncertain.prob_per_trial",
	 P negBinomialUncProbPerTrial},
	{"normal_uncertain.lower_bounds", P normalUncLowerBnds},
	{"normal_uncertain.means", P normalUncMeans},
	{"normal_uncertain.std_deviations", P normalUncStdDevs},
	{"normal_uncertain.upper_bounds", P normalUncUpperBnds},
	{"poisson_uncertain.lambdas", P poissonUncLambdas},
	{"triangular_uncertain.lower_bounds", P triangularUncLowerBnds},
	{"triangular_uncertain.modes", P triangularUncModes},
	{"triangular_uncertain.upper_bounds", P triangularUncUpperBnds},
	{"uniform_uncertain.lower_bounds", P uniformUncLowerBnds},
	{"uniform_uncertain.upper_bounds", P uniformUncUpperBnds},
	{"weibull_uncertain.alphas", P weibullUncAlphas},
	{"weibull_uncertain.betas", P weibullUncBetas}};
    #undef P

    KW<RealVector, DataVariablesRep> *kw;
    if ((kw = (KW<RealVector, DataVariablesRep>*)Binsearch(RVdv, L))) {
	dbRep->dataVariablesIter->dataVarsRep->*kw->p = rv;
	return;
	}
  }
  else if ((L = Begins(entry_name, "responses."))) {
    if (dbRep->responsesDBLocked)
	Locked_db();
    #define P &DataResponsesRep::
    static KW<RealVector, DataResponsesRep> RVdr[] = {	
      // must be sorted by string (key)
	{"nonlinear_equality_scales", P nonlinearEqScales},
	{"nonlinear_equality_targets", P nonlinearEqTargets},
	{"nonlinear_inequality_lower_bounds", P nonlinearIneqLowerBnds},
	{"nonlinear_inequality_scales", P nonlinearIneqScales},
	{"nonlinear_inequality_upper_bounds", P nonlinearIneqUpperBnds},
	{"primary_response_fn_scales", P primaryRespFnScales},
	{"primary_response_fn_weights", P primaryRespFnWeights}};
    #undef P

    KW<RealVector, DataResponsesRep> *kw;
    if ((kw = (KW<RealVector, DataResponsesRep>*)Binsearch(RVdr, L))) {
	dbRep->dataResponsesIter->dataRespRep->*kw->p = rv;
	return;
	}
  }
  Bad_name(entry_name, "set(RealVector&)");
}


void ProblemDescDB::set(const String& entry_name, const IntVector& iv)
{
  const char *L;
  if (!dbRep)
	Null_rep1("set(IntVector&)");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<IntVector, DataVariablesRep> IVdv[] = {	
      // must be sorted by string (key)
	{"binomial_uncertain.num_trials", P binomialUncNumTrials},
	{"discrete_aleatory_uncertain_int.initial_point",
	 P discreteIntAleatoryUncVars},
	{"discrete_aleatory_uncertain_int.lower_bounds",
	 P discreteIntAleatoryUncLowerBnds},
	{"discrete_aleatory_uncertain_int.upper_bounds",
	 P discreteIntAleatoryUncUpperBnds},
	{"discrete_design_range.initial_point", P discreteDesignRangeVars},
	{"discrete_design_range.lower_bounds", P discreteDesignRangeLowerBnds},
	{"discrete_design_range.upper_bounds", P discreteDesignRangeUpperBnds},
	{"discrete_design_set_int.initial_point", P discreteDesignSetIntVars},
	{"discrete_epistemic_uncertain_int.initial_point",
	 P discreteIntEpistemicUncVars},
	{"discrete_epistemic_uncertain_int.lower_bounds",
	 P discreteIntEpistemicUncLowerBnds},
	{"discrete_epistemic_uncertain_int.upper_bounds",
	 P discreteIntEpistemicUncUpperBnds},
	{"discrete_state_range.initial_state", P discreteStateRangeVars},
	{"discrete_state_range.lower_bounds", P discreteStateRangeLowerBnds},
	{"discrete_state_range.upper_bounds", P discreteStateRangeUpperBnds},
	{"discrete_state_set_int.initial_state", P discreteStateSetIntVars},
	{"hypergeometric_uncertain.num_drawn", P hyperGeomUncNumDrawn},
	{"hypergeometric_uncertain.selected_population",
	 P hyperGeomUncSelectedPop},
	{"hypergeometric_uncertain.total_population", P hyperGeomUncTotalPop},
	{"negative_binomial_uncertain.num_trials", P negBinomialUncNumTrials}};
    #undef P

    KW<IntVector, DataVariablesRep> *kw;
    if ((kw = (KW<IntVector, DataVariablesRep>*)Binsearch(IVdv, L))) {
	dbRep->dataVariablesIter->dataVarsRep->*kw->p = iv;
	return;
	}
  }
  Bad_name(entry_name, "set(IntVector&)");
}


void ProblemDescDB::set(const String& entry_name, const BitArray& ba)
{
  const char *L;
  if (!dbRep)
	Null_rep1("set(BitArray&)");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<BitArray, DataVariablesRep> BAdv[] = {	
      // must be sorted by string (key)
	{"binomial_uncertain.categorical", P binomialUncCat},
	{"discrete_design_range.categorical", P discreteDesignRangeCat},
	{"discrete_design_set_int.categorical", P discreteDesignSetIntCat},
	{"discrete_design_set_real.categorical", P discreteDesignSetRealCat},
	{"discrete_interval_uncertain.categorical", P discreteIntervalUncCat},
	{"discrete_state_range.categorical", P discreteStateRangeCat},
	{"discrete_state_set_int.categorical", P discreteStateSetIntCat},
	{"discrete_state_set_real.categorical", P discreteStateSetRealCat},
	{"discrete_uncertain_set_int.categorical", P discreteUncSetIntCat},
	{"discrete_uncertain_set_real.categorical", P discreteUncSetRealCat},
	{"geometric_uncertain.categorical", P geometricUncCat},
	{"histogram_uncertain.point_int.categorical",
         P histogramUncPointIntCat},
	{"histogram_uncertain.point_real.categorical",
         P histogramUncPointRealCat},
	{"hypergeometric_uncertain.categorical", P hyperGeomUncCat},
	{"negative_binomial_uncertain.categorical", P negBinomialUncCat},
	{"poisson_uncertain.categorical", P poissonUncCat}};
    #undef P

    KW<BitArray, DataVariablesRep> *kw;
    if ((kw = (KW<BitArray, DataVariablesRep>*)Binsearch(BAdv, L))) {
	dbRep->dataVariablesIter->dataVarsRep->*kw->p = ba;
	return;
	}
  }
  Bad_name(entry_name, "set(BitArray&)");
}


void ProblemDescDB::set(const String& entry_name, const RealSymMatrix& rsm)
{
  if (!dbRep)
	Null_rep1("set(RealSymMatrix&)");
  if (strbegins(entry_name, "variables.")) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    if (strends(entry_name, "uncertain.correlation_matrix")) {
	dbRep->dataVariablesIter->dataVarsRep->uncertainCorrelations = rsm;
	return;
	}
  }
  Bad_name(entry_name, "set(RealSymMatrix&)");
}


void ProblemDescDB::set(const String& entry_name, const RealVectorArray& rva)
{
  const char *L;
  if (!dbRep)
    Null_rep1("set(RealVectorArray&)");
  if ((L = Begins(entry_name, "method.nond."))) {
    if (dbRep->methodDBLocked)
      Locked_db();
    #define P &DataMethodRep::
    static KW<RealVectorArray, DataMethodRep> RVAdme[] = { 
      // must be sorted by string (key)
	{"gen_reliability_levels", P genReliabilityLevels},
	{"probability_levels", P probabilityLevels},
	{"reliability_levels", P reliabilityLevels},
	{"response_levels", P responseLevels}};
    #undef P

    KW<RealVectorArray, DataMethodRep> *kw;
    if ((kw = (KW<RealVectorArray, DataMethodRep>*)Binsearch(RVAdme, L))) {
      dbRep->dataMethodIter->dataMethodRep->*kw->p = rva;
      return;
    }
  }
  Bad_name(entry_name, "set(RealVectorArray&)");
}


void ProblemDescDB::set(const String& entry_name, const IntVectorArray& iva)
{
  const char *L;
  if (!dbRep)
    Null_rep1("set(IntVectorArray&)");
  // BMA: No current use cases
  Bad_name(entry_name, "set(IntVectorArray&)");
}


void ProblemDescDB::set(const String& entry_name, const IntSetArray& isa)
{
  const char *L;
  if (!dbRep)
    Null_rep1("set(IntSetArray&)");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
      Locked_db();
    #define P &DataVariablesRep::
    static KW<IntSetArray, DataVariablesRep> ISAdv[] = { 
      // must be sorted by string (key)
	{"discrete_design_set_int.values", P discreteDesignSetInt},
	{"discrete_state_set_int.values",  P discreteStateSetInt}};
    #undef P

    KW<IntSetArray, DataVariablesRep> *kw;
    if ((kw = (KW<IntSetArray, DataVariablesRep>*)Binsearch(ISAdv, L))) {
      dbRep->dataVariablesIter->dataVarsRep->*kw->p = isa;
      return;
    }
  }
  Bad_name(entry_name, "set(IntSetArray&)");
}


void ProblemDescDB::set(const String& entry_name, const RealSetArray& rsa)
{
  const char *L;
  if (!dbRep)
    Null_rep1("set(RealSetArray&)");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
      Locked_db();
    #define P &DataVariablesRep::
    static KW<RealSetArray, DataVariablesRep> RSAdv[] = { 
      // must be sorted by string (key)
	{"discrete_design_set_real.values", P discreteDesignSetReal},
	{"discrete_state_set_real.values",  P discreteStateSetReal}};
    #undef P

    KW<RealSetArray, DataVariablesRep> *kw;
    if ((kw = (KW<RealSetArray, DataVariablesRep>*)Binsearch(RSAdv, L))) {
      dbRep->dataVariablesIter->dataVarsRep->*kw->p = rsa;
      return;
    }
  }
  Bad_name(entry_name, "set(RealSetArray&)");
}


void ProblemDescDB::set(const String& entry_name, const IntRealMapArray& irma)
{
  const char *L;
  if (!dbRep)
    Null_rep1("set(IntRealMapArray&)");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
      Locked_db();
    #define P &DataVariablesRep::
    static KW<IntRealMapArray, DataVariablesRep> IRMAdv[] = { 
      // must be sorted by string (key)
	{"discrete_uncertain_set_int.values_probs",
	 P discreteUncSetIntValuesProbs},
	{"histogram_uncertain.point_int_pairs", P histogramUncPointIntPairs}};
    #undef P

    KW<IntRealMapArray, DataVariablesRep> *kw;
    if ((kw = (KW<IntRealMapArray, DataVariablesRep>*)Binsearch(IRMAdv, L))) {
      dbRep->dataVariablesIter->dataVarsRep->*kw->p = irma;
      return;
    }
  }
  Bad_name(entry_name, "set(IntRealMapArray&)");
}


void ProblemDescDB::set(const String& entry_name, const StringRealMapArray& srma)
{
  const char *L;
  if (!dbRep)
    Null_rep1("set(StringRealMapArray&)");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
      Locked_db();
    #define P &DataVariablesRep::
    static KW<StringRealMapArray, DataVariablesRep> SRMAdv[] = { 
      // must be sorted by string (key)
	{"histogram_uncertain.point_string_pairs", P histogramUncPointStrPairs}};
    #undef P

    KW<StringRealMapArray, DataVariablesRep> *kw;
    if ((kw = (KW<StringRealMapArray, DataVariablesRep>*)Binsearch(SRMAdv, L))) {
      dbRep->dataVariablesIter->dataVarsRep->*kw->p = srma;
      return;
    }
  }
  Bad_name(entry_name, "set(StringRealMapArray&)");
}


void ProblemDescDB::set(const String& entry_name, const RealRealMapArray& rrma)
{
  const char *L;
  if (!dbRep)
    Null_rep1("set(RealRealMapArray&)");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
      Locked_db();
    #define P &DataVariablesRep::
    static KW<RealRealMapArray, DataVariablesRep> RRMAdv[] = { 
      // must be sorted by string (key)
	{"discrete_uncertain_set_real.values_probs",
	 P discreteUncSetRealValuesProbs}};
    #undef P

    KW<RealRealMapArray, DataVariablesRep> *kw;
    if ((kw = (KW<RealRealMapArray, DataVariablesRep>*)Binsearch(RRMAdv, L))) {
      dbRep->dataVariablesIter->dataVarsRep->*kw->p = rrma;
      return;
    }
  }
  Bad_name(entry_name, "set(RealRealMapArray&)");
}

void ProblemDescDB::
set(const String& entry_name, const RealRealPairRealMapArray& rrrma)
{
  const char *L;
  if (!dbRep)
    Null_rep1("set(RealRealPairRealMapArray&)");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
      Locked_db();
    #define P &DataVariablesRep::
    static KW<RealRealPairRealMapArray, DataVariablesRep> RRRMAdv[] = { 
      // must be sorted by string (key)
	{"continuous_interval_uncertain.basic_probs",
	 P continuousIntervalUncBasicProbs}};
    #undef P

    KW<RealRealPairRealMapArray, DataVariablesRep> *kw;
    if ((kw = (KW<RealRealPairRealMapArray, DataVariablesRep>*)Binsearch(RRRMAdv, L))) {
      dbRep->dataVariablesIter->dataVarsRep->*kw->p = rrrma;
      return;
    }
  }
  Bad_name(entry_name, "set(RealRealPairRealMapArray&)");
}

void ProblemDescDB::
set(const String& entry_name, const IntIntPairRealMapArray& iirma)
{
  const char *L;
  if (!dbRep)
    Null_rep1("set(IntIntPairRealMapArray&)");
  if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
      Locked_db();
    #define P &DataVariablesRep::
    static KW<IntIntPairRealMapArray, DataVariablesRep> IIRMAdv[] = { 
      // must be sorted by string (key)
	{"discrete_interval_uncertain.basic_probs",
	 P discreteIntervalUncBasicProbs}};
    #undef P

    KW<IntIntPairRealMapArray, DataVariablesRep> *kw;
    if ((kw = (KW<IntIntPairRealMapArray, DataVariablesRep>*)Binsearch(IIRMAdv, L))) {
      dbRep->dataVariablesIter->dataVarsRep->*kw->p = iirma;
      return;
    }
  }
  Bad_name(entry_name, "set(IntIntPairRealMapArray&)");
}


void ProblemDescDB::set(const String& entry_name, const StringArray& sa)
{
  const char *L;
  if (!dbRep)
	Null_rep1("set(StringArray&)");
  if ((L = Begins(entry_name, "model."))) {
    if (dbRep->modelDBLocked)
	Locked_db();
    #define P &DataModelRep::
    static KW<StringArray, DataModelRep> SAdmo[] = {	
      // must be sorted by string (key)
	{"diagnostics", P diagMetrics},
	{"nested.primary_variable_mapping", P primaryVarMaps},
	{"nested.secondary_variable_mapping", P secondaryVarMaps}};
    #undef P

    KW<StringArray, DataModelRep> *kw;
    if ((kw = (KW<StringArray, DataModelRep>*)Binsearch(SAdmo, L))) {
	dbRep->dataModelIter->dataModelRep->*kw->p = sa;
	return;
	}
  }
  else if ((L = Begins(entry_name, "variables."))) {
    if (dbRep->variablesDBLocked)
	Locked_db();
    #define P &DataVariablesRep::
    static KW<StringArray, DataVariablesRep> SAdv[] = {	
      // must be sorted by string (key)
	{"continuous_aleatory_uncertain.labels", P continuousAleatoryUncLabels},
	{"continuous_design.labels", P continuousDesignLabels},
	{"continuous_design.scale_types", P continuousDesignScaleTypes},
	{"continuous_epistemic_uncertain.labels",
	 P continuousEpistemicUncLabels},
	{"continuous_state.labels", P continuousStateLabels},
	{"discrete_aleatory_uncertain_int.labels",
	 P discreteIntAleatoryUncLabels},
	{"discrete_aleatory_uncertain_real.labels",
	 P discreteRealAleatoryUncLabels},
	{"discrete_design_range.labels", P discreteDesignRangeLabels},
	{"discrete_design_set_int.labels", P discreteDesignSetIntLabels},
	{"discrete_design_set_real.labels", P discreteDesignSetRealLabels},
	{"discrete_epistemic_uncertain_int.labels",
	 P discreteIntEpistemicUncLabels},
	{"discrete_epistemic_uncertain_real.labels",
	 P discreteRealEpistemicUncLabels},
	{"discrete_state_range.labels", P discreteStateRangeLabels},
	{"discrete_state_set_int.labels", P discreteStateSetIntLabels},
	{"discrete_state_set_real.labels", P discreteStateSetRealLabels},
	{"discrete_state_set_string.labels", P discreteStateSetStrLabels},
	{"linear_equality_scale_types", P linearEqScaleTypes},
	{"linear_inequality_scale_types", P linearIneqScaleTypes}};
    #undef P

    KW<StringArray, DataVariablesRep> *kw;
    if ((kw = (KW<StringArray, DataVariablesRep>*)Binsearch(SAdv, L))) {
	dbRep->dataVariablesIter->dataVarsRep->*kw->p = sa;
	return;
	}
  }
  else if ((L = Begins(entry_name, "responses."))) {
    if (dbRep->responsesDBLocked)
	Locked_db();
    #define P &DataResponsesRep::
    static KW<StringArray, DataResponsesRep> SAdr[] = {	
      // must be sorted by string (key)
	{"labels", P responseLabels },
	{"nonlinear_equality_scale_types", P nonlinearEqScaleTypes },
	{"nonlinear_inequality_scale_types", P nonlinearIneqScaleTypes },
	{"primary_response_fn_scale_types", P primaryRespFnScaleTypes }};
    #undef P

    KW<StringArray, DataResponsesRep> *kw;
    if ((kw = (KW<StringArray, DataResponsesRep>*)Binsearch(SAdr, L))) {
	dbRep->dataResponsesIter->dataRespRep->*kw->p = sa;
	return;
	}
  }
  Bad_name(entry_name, "set(StringArray&)");
}


void ProblemDescDB::echo_input_file(const ProgramOptions& prog_opts)
{
  const String& dakota_input_file = prog_opts.input_file();
  if (!dakota_input_file.empty()) {
    bool input_is_stdin = 
      ( dakota_input_file.size() == 1 && dakota_input_file[0] == '-');
    if (!input_is_stdin) {
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
      Cout << "Begin DAKOTA input file\n";
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
  else if (!prog_opts.input_string().empty()) {
    size_t header_len = 23;
    std::string header(header_len, '-');
    Cout << header << '\n';
    Cout << "Begin DAKOTA input file\n";
    Cout << "(from string)\n"; 
    Cout << header << std::endl;
    Cout << prog_opts.input_string() << std::endl;
    Cout << "---------------------\n";
    Cout << "End DAKOTA input file\n";
    Cout << "---------------------\n" << std::endl;
  }
}

} // namespace Dakota

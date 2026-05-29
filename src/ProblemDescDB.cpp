/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
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
#include "dakota_data_util.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "EmptyProblemDescDB.hpp"
#include "InstructionMaterializer.hpp"
#include "IRQuery.hpp"
#include "DakotaIterator.hpp"
#include "DakotaInterface.hpp"
#include "WorkdirHelper.hpp"  // bfs utils and prepend_preferred_env_path
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <string>
#include "delete_study_components.hpp"
#include <cstdlib>
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string_view>
#include <type_traits>
#include <variant>

//#define DEBUG
//#define MPI_DEBUG

#define JSONDB_VERBOSE true

static const char rcsId[]="@(#) $Id: ProblemDescDB.cpp 7007 2010-10-06 15:54:39Z wjbohnh $";


namespace Dakota {

extern ParallelLibrary dummy_lib; // defined in dakota_global_defs.cpp
extern ProblemDescDB *Dak_pddb;	  // defined in dakota_global_defs.cpp

namespace {

using json = nlohmann::json;

json load_json_from_file(const String& filename)
{
  std::ifstream file(filename);
  if (!file.is_open())
    throw std::runtime_error("Could not open the file: " + filename);

  json j;
  file >> j;
  return j;
}

bool ir_debug_logging_enabled()
{
  const char* value = std::getenv("DAKOTA_DEBUG_IR");
  return value && *value;
}

void log_top_level_json_shape(const json& j, const String& filename)
{
  if (!ir_debug_logging_enabled())
    return;

  std::cerr << "ProblemDescDB::enable_json_input: loaded '" << filename << "'";
  if (!j.is_object()) {
    std::cerr << " with top-level type " << j.type_name() << std::endl;
    return;
  }

  std::cerr << " with top-level keys:";
  for (auto it = j.begin(); it != j.end(); ++it) {
    std::cerr << " " << it.key() << "("
              << (it->is_object() ? "object" :
                  it->is_array() ? "array" :
                  it->type_name())
              << ")";
  }
  std::cerr << std::endl;
}

template <class T, class Variant>
struct variant_contains;

template <class T, class... Alts>
struct variant_contains<T, std::variant<Alts...>>
  : std::disjunction<std::is_same<T, Alts>...> {};

template <class T, class Variant>
inline constexpr bool variant_contains_v = variant_contains<T, Variant>::value;

} // namespace


/** This constructor is the one which must build the base class data for all
    derived classes.  get_db() instantiates a derived class letter and the
    derived constructor selects this base class constructor in its
    initialization list (to avoid the recursion of the base class constructor
    calling get_db() again).  Since the letter IS the representation, its
    representation pointer is set to NULL. */
ProblemDescDB::ProblemDescDB(BaseConstructor, int world_size, int world_rank):
  environmentCntr(0), methodDBLocked(true),
  modelDBLocked(true), variablesDBLocked(true), interfaceDBLocked(true),
  responsesDBLocked(true), worldSize(world_size), worldRank(world_rank)
{ /* empty ctor */ }


/** This is the envelope constructor which uses problem_db to build a
    fully populated db object.  It only needs to extract enough data
    to properly execute get_db(problem_db), since the constructor
    overloaded with BaseConstructor builds the actual base class data
    inherited by the derived classes. */
ProblemDescDB::ProblemDescDB(int world_size, int world_rank) :
  // Set the rep pointer to the appropriate db type
  dbRep(get_db(world_size, world_rank))

{
  if (!dbRep) // bad settings or insufficient memory
    abort_handler(-1);
}


/** Initializes dbRep to the appropriate derived type.  The standard
    derived class constructors are invoked.  */
std::shared_ptr<ProblemDescDB>
ProblemDescDB::get_db(int world_size, int world_rank)
{
  Dak_pddb = this;	// for use in abort_handler()

  //if (xml_flag)
  //  return new XMLProblemDescDB(parallel_lib);
  //else
  return std::make_shared<EmptyProblemDescDB>(world_size, world_rank);
}

/** Copy constructor manages sharing of dbRep */
ProblemDescDB::ProblemDescDB(const ProblemDescDB& db):
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
  if(dbRep.use_count() == 1)
    delete_study_components(*this);
}


/** DB setup phase 1: parse the input file and execute callback
    functions if present.  Rank 0 only.

    DB setup phase 2: optionally insert additional data via late sets.
    Rank 0 only. */
void ProblemDescDB::
parse_inputs(const std::string_view input_string, const std::string_view parser_options, bool command_line_run,
	     DbCallbackFunctionPtr callback, void *callback_data)
{
  if (dbRep) {
    dbRep->parse_inputs(input_string, parser_options, command_line_run, callback, callback_data);
    // BMA TODO: Temporary workaround; can't get callback to work on
    // letter yet. Need to replace Null_rep* with forward to letter
    // and remove dbRep->, but initial cut didn't work.
    if (callback && dbRep->worldRank == 0)
      (*callback)(this, callback_data);
  }
  else {

    // Only world rank 0 parses the input file.
    if (worldRank == 0) {
        irState.reset();
        validatedStudyJson = json();
    }

    // Allow user input by callback function.

    // BMA TODO: Is this comment true?
    // Note: the DB is locked and the list iterators are not defined.  Thus,
    // the user function must do something to put the DB in a usable set/get
    // state (e.g., resolve_top_method() or set_db_list_nodes()).

    // if (callback)
    // 	(*callback)(this, callback_data);
  }
  //Cout << "ProblemDescDB::parse_inputs: using data from input ..."
  //     << " dataMethodList is " << String(dataMethodList.empty() ? "" : "NOT")
  //     << " empty." << std::endl;
}

void ProblemDescDB::enable_json_input(const String & json_file)
{
  if (ir_debug_logging_enabled())
    std::cerr << "ProblemDescDB::enable_json_input: starting for '" << json_file << "'" << std::endl;

  const json study_json = load_json_from_file(json_file);
  log_top_level_json_shape(study_json, json_file);

  enable_json_input(study_json);
}

void ProblemDescDB::enable_json_input(const nlohmann::json& study_json)
{
  if (ir_debug_logging_enabled())
    log_top_level_json_shape(study_json, "<in-memory>");

  InstructionMaterializer materializer;
  std::shared_ptr<IRState> materialized;
  try {
    materialized = std::make_shared<IRState>(materializer.materialize(study_json));
  }
  catch (const std::exception& e) {
    std::cerr << "ProblemDescDB::enable_json_input: failed while materializing study JSON: "
              << e.what() << std::endl;
    throw;
  }

  if (dbRep) {
    dbRep->validatedStudyJson = study_json;
    dbRep->irState = std::move(materialized);
    if (dbRep->dataMethodList.empty())
      dbRep->populate_skeleton_data_from_ir();
  }
  else {
    validatedStudyJson = study_json;
    irState = std::move(materialized);
    if (dataMethodList.empty())
      populate_skeleton_data_from_ir();
  }

  if (ir_debug_logging_enabled()) {
    const ProblemDescDB* db = dbRep ? dbRep.get() : this;
    std::cerr << "ProblemDescDB::enable_json_input: materialized IR sizes"
              << " environment=" << db->irState->environment.values().size()
              << " method=" << db->irState->method.size()
              << " model=" << db->irState->model.size()
              << " variables=" << db->irState->variables.size()
              << " interface=" << db->irState->interface.size()
              << " responses=" << db->irState->responses.size()
              << std::endl;
  }
}

/** DB setup phase 3: perform basic checks on keywords counts in
    current DB state, then sync to all processors. */
void ProblemDescDB::check_and_broadcast(const UserModes& user_modes) {

  if (dbRep)
    dbRep->check_and_broadcast(user_modes);
  else {

    // Check to make sure at least one of each of the keywords was found
    // in the problem specification file; checks only happen on Dakota rank 0
    if (worldRank == 0)
      check_input(user_modes);

    // bcast a minimal MPI buffer containing the input specification
    // data prior to post-processing
    broadcast();

    // After broadcast, perform post-processing on all processors to
    // size default variables/responses specification vectors (avoid
    // sending large vectors over an MPI buffer).
    post_process();

  }
}


void ProblemDescDB::populate_skeleton_data_from_ir()
{
  ProblemDescDB* db = dbRep ? dbRep.get() : this;
  if (!db->irState)
    return;

  db->environmentCntr = 0;
  db->environmentSpec = DataEnvironment();
  if (db->irState->environment.contains("top_method_pointer"))
    db->environmentSpec.data_rep()->topMethodPointer =
      db->irState->environment.get<String>("top_method_pointer");
  if (!db->irState->environment.values().empty())
    db->environmentCntr = 1;

  db->dataMethodList.clear();
  for (const auto& store : db->irState->method) {
    DataMethod method;
    if (store.contains("algorithm"))
      method.data_rep()->methodName = store.get<unsigned short>("algorithm");
    if (store.contains("id"))
      method.data_rep()->idMethod = store.get<String>("id");
    if (store.contains("sub_method"))
      method.data_rep()->subMethod = store.get<unsigned short>("sub_method");
    if (store.contains("sub_method_name"))
      method.data_rep()->subMethodName = store.get<String>("sub_method_name");
    if (store.contains("sub_model_pointer"))
      method.data_rep()->subModelPointer = store.get<String>("sub_model_pointer");
    if (store.contains("model_pointer"))
      method.data_rep()->modelPointer = store.get<String>("model_pointer");
    if (store.contains("sub_method_pointer"))
      method.data_rep()->subMethodPointer = store.get<String>("sub_method_pointer");
    db->dataMethodList.push_back(method);
  }

  db->dataModelList.clear();
  for (const auto& store : db->irState->model) {
    DataModel model;
    if (store.contains("id"))
      model.data_rep()->idModel = store.get<String>("id");
    if (store.contains("variables_pointer"))
      model.data_rep()->variablesPointer = store.get<String>("variables_pointer");
    if (store.contains("interface_pointer"))
      model.data_rep()->interfacePointer = store.get<String>("interface_pointer");
    if (store.contains("responses_pointer"))
      model.data_rep()->responsesPointer = store.get<String>("responses_pointer");
    if (store.contains("sub_method_pointer"))
      model.data_rep()->subMethodPointer = store.get<String>("sub_method_pointer");
    if (store.contains("type"))
      model.data_rep()->modelType = store.get<String>("type");
    if (store.contains("surrogate.type"))
      model.data_rep()->surrogateType = store.get<String>("surrogate.type");
    db->dataModelList.push_back(model);
  }

  db->dataVariablesList.clear();
  for (const auto& store : db->irState->variables) {
    DataVariables variables;
    if (store.contains("id"))
      variables.data_rep()->idVariables = store.get<String>("id");
    db->dataVariablesList.push_back(variables);
  }

  db->dataInterfaceList.clear();
  for (const auto& store : db->irState->interface) {
    DataInterface interface;
    if (store.contains("id"))
      interface.data_rep()->idInterface = store.get<String>("id");
    db->dataInterfaceList.push_back(interface);
  }

  db->dataResponsesList.clear();
  for (const auto& store : db->irState->responses) {
    DataResponses responses;
    if (store.contains("id"))
      responses.data_rep()->idResponses = store.get<String>("id");
    db->dataResponsesList.push_back(responses);
  }
}



void ProblemDescDB::broadcast()
{
  if (dbRep)
    dbRep->broadcast();
  else {
    // DAKOTA's old design for reading the input file was for world rank 0 to
    // get the input filename from cmd_line_handler (after MPI_Init) and bcast
    // the character buffer to all other processors (having every processor
    // query the cmd_line_handler was failing because of the effect of MPI_Init
    // on argc and argv).  Then every processor yyparsed.  This worked fine but
    // was not scalable for MP machines with a limited number of I/O devices.

    // Now, world rank 0 yyparse's and sends all the parsed data in a single
    // buffer to all other ranks.
    if (worldSize > 1) {
      if (worldRank == 0) {
	enforce_unique_ids();
	send_db_buffer();
#ifdef MPI_DEBUG
	Cout << "DB buffer to send on world rank " << worldRank
	     << ":\n" << environmentSpec << dataMethodList << dataVariablesList
	     << dataInterfaceList << dataResponsesList << std::endl;
#endif // MPI_DEBUG
      }
      else {
	receive_db_buffer();
#ifdef MPI_DEBUG
	Cout << "DB buffer received on world rank " << worldRank
	     << ":\n" << environmentSpec << dataMethodList << dataVariablesList
	     << dataInterfaceList << dataResponsesList << std::endl;
#endif // MPI_DEBUG
      }
    }
    else {
#ifdef DEBUG
      Cout << "DB parsed data:\n" << environmentSpec << dataMethodList
	   << dataVariablesList << dataInterfaceList << dataResponsesList
	   << std::endl;
#endif // DEBUG
      enforce_unique_ids();
    }
  }
}


/** When using library mode in a parallel application, post_process()
    should be called on all processors following broadcast() of a
    minimal problem specification. */
void ProblemDescDB::post_process()
{
}


/** NOTE: when using library mode in a parallel application,
    check_input() should either be called only on worldRank 0, or it
    should follow a matched send_db_buffer()/receive_db_buffer() pair. */
void ProblemDescDB::check_input(const UserModes& user_modes)
{
  if (dbRep)
    dbRep->check_input(user_modes);
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

    if (user_modes.requestedUserModes) {

      if (!user_modes.postRunInput.empty())
	Cerr << "Warning: pre-run input not implemented; ignored.\n";

      if (!user_modes.preRunOutput.empty()) {
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

      if (!user_modes.runInput.empty())
	Cerr << "Warning: run input not implemented; ignored.\n";

      if (!user_modes.runOutput.empty())
	Cerr << "Warning: run output not implemented; ignored.\n";

      if (!user_modes.postRunInput.empty()) {
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

      if (!user_modes.postRunOutput.empty())
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

    if (irState && !methodDBLocked)
      irState->active.method =
        static_cast<size_t>(std::distance(dataMethodList.begin(), dataMethodIter));
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
	  if (worldRank == 0)
	    Cerr << "\nWarning: empty method id string not found.\n         "
		 << "Last method specification parsed will be used.\n";
	  --dataMethodIter; // last entry in list
	}
	else if (worldRank == 0 &&
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
	if (worldRank == 0 &&
	    std::count_if(dataMethodList.begin(), dataMethodList.end(),
			  boost::bind(DataMethod::id_compare,_1,method_tag))>1)
	  Cerr << "\nWarning: method id string " << method_tag
	       << " is ambiguous.\n         First matching method "
	       << "specification will be used.\n";
      }
    }
    if (irState && !methodDBLocked)
      irState->active.method =
        static_cast<size_t>(std::distance(dataMethodList.begin(), dataMethodIter));
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
    if (irState && method_index < irState->method.size())
      irState->active.method = method_index;
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
      if (irState && model_index < irState->model.size())
        irState->active.model = model_index;
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
	  if (worldRank == 0)
	    Cerr << "\nWarning: empty model id string not found.\n         "
		 << "Last model specification parsed will be used.\n";
	  --dataModelIter; // last entry in list
	}
	else if (worldRank == 0 &&
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
	if (worldRank == 0 &&
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
      if (irState)
        irState->active.model =
          static_cast<size_t>(std::distance(dataModelList.begin(), dataModelIter));
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
	  if (worldRank == 0)
	    Cerr << "\nWarning: empty variables id string not found.\n         "
		 << "Last variables specification parsed will be used.\n";
	  --dataVariablesIter; // last entry in list
	}
	else if (worldRank == 0 &&
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
	if (worldRank == 0 &&
	    std::count_if(dataVariablesList.begin(), dataVariablesList.end(),
			  boost::bind(DataVariables::id_compare, _1,
				      variables_tag)) > 1)
	  Cerr << "\nWarning: variables id string " << variables_tag
	       << " is ambiguous.\n         First matching variables "
	       << "specification will be used.\n";
      }
    }
    if (irState && !variablesDBLocked)
      irState->active.variables =
        static_cast<size_t>(std::distance(dataVariablesList.begin(), dataVariablesIter));
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
	  if (worldRank == 0 &&
	      MoRep.modelType == "simulation")
	    Cerr << "\nWarning: empty interface id string not found.\n         "
		 << "Last interface specification parsed will be used.\n";
	  --dataInterfaceIter; // last entry in list
	}
	else if (worldRank == 0 &&
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
	if (worldRank == 0 &&
	    std::count_if(dataInterfaceList.begin(), dataInterfaceList.end(),
			  boost::bind(DataInterface::id_compare, _1,
				      interface_tag)) > 1)
	  Cerr << "\nWarning: interface id string " << interface_tag
	       << " is ambiguous.\n         First matching interface "
	       << "specification will be used.\n";
      }
    }
    if (irState && !interfaceDBLocked)
      irState->active.interface =
        static_cast<size_t>(std::distance(dataInterfaceList.begin(), dataInterfaceIter));
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
	  if (worldRank == 0)
	    Cerr << "\nWarning: empty responses id string not found.\n         "
		 << "Last responses specification parsed will be used.\n";
	  --dataResponsesIter; // last entry in list
	}
	else if (worldRank == 0 &&
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
	if (worldRank == 0 &&
	    std::count_if(dataResponsesList.begin(), dataResponsesList.end(),
			  boost::bind(DataResponses::id_compare, _1,
				      responses_tag)) > 1)
	  Cerr << "\nWarning: responses id string " << responses_tag
	       << " is ambiguous.\n         First matching responses "
	       << "specification will be used.\n";
      }
    }
    if (irState && !responsesDBLocked)
      irState->active.responses =
        static_cast<size_t>(std::distance(dataResponsesList.begin(), dataResponsesIter));
  }
}


void ProblemDescDB::send_db_buffer()
{
  MPIPackBuffer send_buffer;
  send_buffer << environmentSpec   << dataMethodList    << dataModelList
	      << dataVariablesList << dataInterfaceList << dataResponsesList;

  // Broadcast length of buffer so that servers can allocate MPIUnpackBuffer
  int buffer_len = send_buffer.size();
  //parallelLib.bcast_w(buffer_len);

  // Broadcast actual buffer
  //parallelLib.bcast_w(send_buffer);
}


void ProblemDescDB::receive_db_buffer()
{
  // receive length of incoming buffer and allocate space for MPIUnpackBuffer
  int buffer_len;
  //parallelLib.bcast_w(buffer_len);

  // receive incoming buffer
  MPIUnpackBuffer recv_buffer(buffer_len);
  //parallelLib.bcast_w(recv_buffer);
  recv_buffer >> environmentSpec   >> dataMethodList    >> dataModelList
	      >> dataVariablesList >> dataInterfaceList >> dataResponsesList;
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
  int world_size = (dbRep) ? dbRep->worldSize : worldSize;
  // Note: get_*() requires envelope execution (throws error if !dbRep)

  // TO DO: can we be more fine grained on parallel testers?
  //        default tester could get hidden by plug-in...

  int max_ppa = (get_ushort("interface.type") & DIRECT_INTERFACE_BIT) ?
    world_size : 1; // system/fork/spawn
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
  throw PARSE_ERROR;
}

static void Locked_db()
{
  Cerr << "\nError: database is locked.  You must first unlock the database\n"
       << "       by setting the list nodes." << std::endl;
  abort_handler(PARSE_ERROR);
  throw PARSE_ERROR;
}

static void Null_rep(const String& who)
{
  Cerr << "\nError: ProblemDescDB::" << who
       << " called with NULL representation." << std::endl;
  abort_handler(PARSE_ERROR);
  throw PARSE_ERROR;
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
const T& ProblemDescDB::
get(const std::string& context_msg,
    const std::string& entry_name,
    const std::shared_ptr<ProblemDescDB>& db_rep) const
{
  if (!db_rep)
    Null_rep(context_msg);

  std::string block, entry;
  std::tie(block, entry) = split_entry_name(entry_name, context_msg);

  if (block == "method" && db_rep->methodDBLocked)
    Locked_db();
  else if (block == "model" && db_rep->modelDBLocked)
    Locked_db();
  else if (block == "variables" && db_rep->variablesDBLocked)
    Locked_db();
  else if (block == "interface" && db_rep->interfaceDBLocked)
    Locked_db();
  else if (block == "responses" && db_rep->responsesDBLocked)
    Locked_db();

  using QueryT = std::remove_const_t<T>;
  if constexpr (variant_contains_v<QueryT, IRValue>) {
    if (db_rep->irState) {
      try {
        return ir_query::get<QueryT>(*db_rep->irState, entry_name);
      }
      catch (const std::exception&) {
        // Fall through to legacy Data*Rep access while the IR implementation
        // is still being filled in incrementally.
        Cerr << "\nParser is using NIDR..." << std::endl;
        abort_handler(PARSE_ERROR);
      }
    }
  }

  Bad_name(entry_name, context_msg);
  return abort_handler_t<const T&>(PARSE_ERROR);
}

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
    entry_name, dbRep);
}

const RealVector& ProblemDescDB::get_rv(const String& entry_name) const
{  
  return get<const RealVector>
  ( "get_rv()",
    entry_name, dbRep);
}


const IntVector& ProblemDescDB::get_iv(const String& entry_name) const
{
  return get<const IntVector>
  ( "get_iv()",
    entry_name, dbRep);
}


const BitArray& ProblemDescDB::get_ba(const String& entry_name) const
{
  return get<const BitArray>
  ( "get_ba()",
    entry_name, dbRep);
}


const SizetArray& ProblemDescDB::get_sza(const String& entry_name) const
{
  return get<const SizetArray>
  ( "get_sza()",
    entry_name, dbRep);
}


const UShortArray& ProblemDescDB::get_usa(const String& entry_name) const
{
  return get<const UShortArray>
  ( "get_usa()",
    entry_name, dbRep);
}


const RealSymMatrix& ProblemDescDB::get_rsm(const String& entry_name) const
{
  return get<const RealSymMatrix>
  ( "get_rsm()",
    entry_name, dbRep);
}


const RealVectorArray& ProblemDescDB::get_rva(const String& entry_name) const
{
  return get<const RealVectorArray>
  ( "get_rva()",
    entry_name, dbRep);
}


const IntVectorArray& ProblemDescDB::get_iva(const String& entry_name) const
{
  // BMA: no current use cases
  return get<const IntVectorArray>
  ( "get_iva()",
    entry_name, dbRep);
}


const IntSet& ProblemDescDB::get_is(const String& entry_name) const
{
  return get<const IntSet>
  ( "get_is()",
    entry_name, dbRep);
}


const IntSetArray& ProblemDescDB::get_isa(const String& entry_name) const
{
  return get<const IntSetArray>
  ( "get_isa()",
    entry_name, dbRep);
}


const SizetSet& ProblemDescDB::get_szs(const String& entry_name) const
{
  return get<const SizetSet>
  ( "get_szs()",
    entry_name, dbRep);
}


const StringSetArray& ProblemDescDB::get_ssa(const String& entry_name) const
{
  return get <const StringSetArray>
  ( "get_ssa()",
    entry_name, dbRep);
}


const RealSetArray& ProblemDescDB::get_rsa(const String& entry_name) const
{
  return get<const RealSetArray>
  ( "get_rsa()",
    entry_name, dbRep);
}


const IntRealMapArray& ProblemDescDB::get_irma(const String& entry_name) const
{
  return get<const IntRealMapArray>
  ( "get_irma()",
    entry_name, dbRep);
}

const StringRealMapArray& ProblemDescDB::get_srma(const String& entry_name) const
{
  return get<const StringRealMapArray>
  ( "get_srma()",
    entry_name, dbRep);
}


const RealRealMapArray& ProblemDescDB::get_rrma(const String& entry_name) const
{
  return get<const RealRealMapArray>
  ( "get_rrma()",
    entry_name, dbRep);
}

const RealRealPairRealMapArray& ProblemDescDB::
get_rrrma(const String& entry_name) const
{
  return get<const RealRealPairRealMapArray>
  ( "get_rrrma()",
    entry_name, dbRep);
}

const IntIntPairRealMapArray& ProblemDescDB::
get_iirma(const String& entry_name) const
{
  return get<const IntIntPairRealMapArray>
  ( "get_iirma()",
    entry_name, dbRep);
}

const StringArray& ProblemDescDB::get_sa(const String& entry_name) const
{
  return get<const StringArray>
  ( "get_sa()",
    entry_name, dbRep);
}


const String2DArray& ProblemDescDB::get_s2a(const String& entry_name) const
{
  return get<const String2DArray>
  ( "get_s2a()",
    entry_name, dbRep);
}


const String& ProblemDescDB::get_string(const String& entry_name) const
{
  return get<const String>
  ( "get_string()",
    entry_name, dbRep);
}


const Real& ProblemDescDB::get_real(const String& entry_name) const
{
  return get<const Real>
  ( "get_real()",
    entry_name, dbRep);
}


int ProblemDescDB::get_int(const String& entry_name) const
{
  return get<int>
  ( "get_int()",
    entry_name, dbRep);
}


short ProblemDescDB::get_short(const String& entry_name) const
{
  return get<short>
  ( "get_short()",
    entry_name, dbRep);
}


unsigned short ProblemDescDB::get_ushort(const String& entry_name) const
{
  return get<unsigned short>
  ( "get_ushort()",
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

    if (dbRep->irState) {
      try {
        return ir_query::get<size_t>(*dbRep->irState, entry_name);
      }
      catch (const std::exception&) {
        // Fall through to legacy aggregate helpers while IR coverage is
        // still being filled in incrementally.
      }
    }

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
    entry_name, dbRep);
}


bool ProblemDescDB::get_bool(const String& entry_name) const
{
  return get<bool>
  ( "get_bool()",
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


void ProblemDescDB::lock_method_db()
{
  if (dbRep) dbRep->lock_method_db();
  else       methodDBLocked = true;
}

void ProblemDescDB::unlock_method_db()
{
  if (dbRep) dbRep->unlock_method_db();
  else       methodDBLocked = false;
}

void ProblemDescDB::lock_model_db()
{
  if (dbRep) dbRep->lock_model_db();
  else       modelDBLocked = true;
}

void ProblemDescDB::unlock_model_db()
{
  if (dbRep) dbRep->unlock_model_db();
  else       modelDBLocked = false;
}

void ProblemDescDB::lock_variables_db()
{
  if (dbRep) dbRep->lock_variables_db();
  else       variablesDBLocked = true;
}

void ProblemDescDB::unlock_variables_db()
{
  if (dbRep) dbRep->unlock_variables_db();
  else       variablesDBLocked = false;
}

void ProblemDescDB::lock_interface_db()
{
  if (dbRep) dbRep->lock_interface_db();
  else       interfaceDBLocked = true;
}

void ProblemDescDB::unlock_interface_db()
{
  if (dbRep) dbRep->unlock_interface_db();
  else       interfaceDBLocked = false;
}

void ProblemDescDB::lock_responses_db()
{
  if (dbRep) dbRep->lock_responses_db();
  else       responsesDBLocked = true;
}

void ProblemDescDB::unlock_responses_db()
{
  if (dbRep) dbRep->unlock_responses_db();
  else       responsesDBLocked = false;
}

} // namespace Dakota

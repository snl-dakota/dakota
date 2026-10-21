/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

// Class:        ProblemDescDB
//- Description: IR-backed problem description database implementation.
//- Owner:       Mike Eldred
//- Checked by:

#include "dakota_system_defs.hpp"
#include "dakota_data_util.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaInterfaceEnums.hpp"
#include "ParallelLibrary.hpp"
#include "InstructionMaterializer.hpp"
#include "DakotaIterator.hpp"
#include "DakotaInterface.hpp"
#include "WorkdirHelper.hpp"  // bfs utils and prepend_preferred_env_path
#include <string>
#include "delete_study_components.hpp"
#include <cstdlib>
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <set>
#include <string_view>

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

} // namespace


/** This constructor is the one which must build the base class data for all
    derived classes.  get_db() instantiates a derived class letter and the
    derived constructor selects this base class constructor in its
    initialization list (to avoid the recursion of the base class constructor
    calling get_db() again).  Since the letter IS the representation, its
    representation pointer is set to NULL. */
ProblemDescDB::ProblemDescDB(BaseConstructor, int world_size, int world_rank):
  methodDBLocked(true),
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
  return std::make_shared<ProblemDescDBRep>(world_size, world_rank);
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

  ProblemDescDB* db = dbRep ? dbRep.get() : this;
  db->validatedStudyJson = study_json;
  db->irState = std::move(materialized);

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


namespace {

const String& ir_id(const IRStore& store)
{
  static const String empty;
  return store.contains("id") ? store.get<String>("id") : empty;
}

const String& ir_string(const IRStore& store, const String& key)
{
  static const String empty;
  return store.contains(key) ? store.get<String>(key) : empty;
}

size_t select_ir_store(const std::vector<IRStore>& stores, String tag,
                       const char* block_name, int world_rank,
                       bool warn_for_default = true)
{
  if (stores.empty()) {
    Cerr << "\nError: no " << block_name << " specifications are available."
         << std::endl;
    abort_handler(PARSE_ERROR);
    throw PARSE_ERROR;
  }

  if (tag == "NO_ID" || tag == "NO_MODEL_ID")
    tag.clear();

  if (tag.empty()) {
    if (stores.size() == 1)
      return 0;

    auto first = std::find_if(stores.begin(), stores.end(),
      [](const IRStore& store) { return ir_id(store).empty(); });
    if (first == stores.end()) {
      if (world_rank == 0 && warn_for_default)
        Cerr << "\nWarning: empty " << block_name
             << " id string not found.\n         Last " << block_name
             << " specification parsed will be used.\n";
      return stores.size() - 1;
    }

    if (world_rank == 0 && warn_for_default &&
        std::count_if(stores.begin(), stores.end(),
          [](const IRStore& store) { return ir_id(store).empty(); }) > 1)
      Cerr << "\nWarning: empty " << block_name
           << " id string is ambiguous.\n         First matching "
           << block_name << " specification will be used.\n";
    return static_cast<size_t>(std::distance(stores.begin(), first));
  }

  auto match = std::find_if(stores.begin(), stores.end(),
    [&tag](const IRStore& store) { return ir_id(store) == tag; });
  if (match == stores.end()) {
    Cerr << "\nError: " << tag << " is not a valid " << block_name
         << " identifier string." << std::endl;
    abort_handler(PARSE_ERROR);
    throw PARSE_ERROR;
  }
  return static_cast<size_t>(std::distance(stores.begin(), match));
}

} // namespace

void ProblemDescDB::check_input(const UserModes& user_modes)
{
  if (dbRep) {
    dbRep->check_input(user_modes);
    return;
  }

  if (!irState) {
    Cerr << "No materialized input study is available." << std::endl;
    abort_handler(PARSE_ERROR);
    throw PARSE_ERROR;
  }

  int num_errors = 0;
  if (irState->method.empty()) {
    Cerr << "No method specification found in input file.\n";
    ++num_errors;
  }
  if (irState->variables.empty()) {
    Cerr << "No variables specification found in input file.\n";
    ++num_errors;
  }
  if (irState->responses.empty()) {
    Cerr << "No responses specification found in input file.\n";
    ++num_errors;
  }

  if (user_modes.requestedUserModes) {
    if (!user_modes.preRunInput.empty())
      Cerr << "Warning: pre-run input not implemented; ignored.\n";

    const auto validate_io_method = [&](const char* mode) {
      if (irState->method.size() > 1) {
        Cerr << "Error: " << mode << " only allowed for single method.\n";
        ++num_errors;
      }
      else if (!irState->method.empty()) {
        const unsigned short method_name =
          irState->method.front().get<unsigned short>("algorithm");
        if (!(method_name & PSTUDYDACE_BIT) && method_name != RANDOM_SAMPLING) {
          Cerr << "Error: " << mode << " not supported for method "
               << method_name << "\n       (supported for sampling, "
               << "parameter study, DDACE, FSUDACE, and PSUADE methods)\n";
          ++num_errors;
        }
      }
    };

    if (!user_modes.preRunOutput.empty())
      validate_io_method("pre-run output");
    if (!user_modes.runInput.empty())
      Cerr << "Warning: run input not implemented; ignored.\n";
    if (!user_modes.runOutput.empty())
      Cerr << "Warning: run output not implemented; ignored.\n";
    if (!user_modes.postRunInput.empty())
      validate_io_method("post-run input");
    if (!user_modes.postRunOutput.empty())
      Cerr << "Warning: post-run output not implemented; ignored.\n";
  }

  if (num_errors) {
    Cerr << num_errors << " input specification errors detected." << std::endl;
    abort_handler(PARSE_ERROR);
    throw PARSE_ERROR;
  }
}


void ProblemDescDB::set_db_list_nodes(const String& method_tag)
{
  if (dbRep) {
    dbRep->set_db_list_nodes(method_tag);
    return;
  }
  if (strbegins(method_tag, "NOSPEC_METHOD_ID_"))
    return;

  set_db_method_node(method_tag);
  if (methodDBLocked) {
    modelDBLocked = variablesDBLocked = interfaceDBLocked =
      responsesDBLocked = true;
    return;
  }
  set_db_model_nodes(
    ir_string(irState->method[irState->active.method], "model_pointer"));
}


void ProblemDescDB::set_db_list_nodes(size_t method_index)
{
  if (dbRep) {
    dbRep->set_db_list_nodes(method_index);
    return;
  }

  set_db_method_node(method_index);
  if (methodDBLocked) {
    modelDBLocked = variablesDBLocked = interfaceDBLocked =
      responsesDBLocked = true;
    return;
  }
  set_db_model_nodes(
    ir_string(irState->method[irState->active.method], "model_pointer"));
}


void ProblemDescDB::resolve_top_method(bool set_model_nodes)
{
  if (dbRep) {
    dbRep->resolve_top_method(set_model_nodes);
    return;
  }
  if (!irState || irState->method.empty()) {
    Cerr << "\nError: ProblemDescDB::resolve_top_method() has no method "
         << "specification." << std::endl;
    abort_handler(PARSE_ERROR);
    throw PARSE_ERROR;
  }

  const String& top_method =
    ir_string(irState->environment, "top_method_pointer");
  size_t method_index = 0;
  if (irState->method.size() == 1)
    method_index = 0;
  else if (!top_method.empty())
    method_index = select_ir_store(irState->method, top_method, "method",
                                   worldRank);
  else {
    std::vector<size_t> candidates(irState->method.size());
    std::iota(candidates.begin(), candidates.end(), 0);

    const auto remove_pointer = [&](const String& pointer) {
      if (pointer.empty())
        return;
      candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
        [&](size_t index) { return ir_id(irState->method[index]) == pointer; }),
        candidates.end());
    };

    for (const auto& method : irState->method)
      remove_pointer(ir_string(method, "sub_method_pointer"));
    for (const auto& model : irState->model) {
      String pointer = ir_string(model, "sub_method_pointer");
      if (pointer.empty())
        pointer = ir_string(model, "nested.sub_method_pointer");
      remove_pointer(pointer);
    }

    if (candidates.size() != 1) {
      Cerr << "\nError: ProblemDescDB::resolve_top_method() failed to "
           << "determine active method specification.\n       Please resolve "
           << "method pointer ambiguities." << std::endl;
      abort_handler(PARSE_ERROR);
      throw PARSE_ERROR;
    }
    method_index = candidates.front();
  }

  irState->active.method = method_index;
  methodDBLocked = false;
  if (set_model_nodes)
    set_db_model_nodes(
      ir_string(irState->method[method_index], "model_pointer"));
}


void ProblemDescDB::set_db_method_node(const String& method_tag)
{
  if (dbRep) {
    dbRep->set_db_method_node(method_tag);
    return;
  }
  if (strbegins(method_tag, "NOSPEC_METHOD_ID_"))
    return;
  if (!irState) {
    methodDBLocked = true;
    Cerr << "\nError: no materialized study is available." << std::endl;
    abort_handler(PARSE_ERROR);
    throw PARSE_ERROR;
  }

  irState->active.method =
    select_ir_store(irState->method, method_tag, "method", worldRank);
  methodDBLocked = false;
}


void ProblemDescDB::set_db_method_node(size_t method_index)
{
  if (dbRep) {
    dbRep->set_db_method_node(method_index);
    return;
  }
  if (method_index == _NPOS) {
    methodDBLocked = true;
    return;
  }
  if (!irState || method_index > irState->method.size()) {
    Cerr << "\nError: method_index sent to set_db_method_node is out of range."
         << std::endl;
    abort_handler(PARSE_ERROR);
    throw PARSE_ERROR;
  }

  methodDBLocked = (method_index == irState->method.size());
  if (!methodDBLocked)
    irState->active.method = method_index;
}


void ProblemDescDB::set_db_model_nodes(size_t model_index)
{
  if (dbRep) {
    dbRep->set_db_model_nodes(model_index);
    return;
  }
  if (model_index == _NPOS) {
    modelDBLocked = variablesDBLocked = interfaceDBLocked =
      responsesDBLocked = true;
    return;
  }
  if (!irState || model_index > irState->model.size()) {
    Cerr << "\nError: model_index sent to set_db_model_nodes is out of range."
         << std::endl;
    abort_handler(PARSE_ERROR);
    throw PARSE_ERROR;
  }
  if (model_index == irState->model.size()) {
    modelDBLocked = variablesDBLocked = interfaceDBLocked =
      responsesDBLocked = true;
    return;
  }

  irState->active.model = model_index;
  modelDBLocked = false;
  const IRStore& model = irState->model[model_index];
  set_db_variables_node(ir_string(model, "variables_pointer"));
  if (model_has_interface(model))
    set_db_interface_node(ir_string(model, "interface_pointer"));
  else
    interfaceDBLocked = true;
  set_db_responses_node(ir_string(model, "responses_pointer"));
}


void ProblemDescDB::set_db_model_nodes(const String& model_tag)
{
  if (dbRep) {
    dbRep->set_db_model_nodes(model_tag);
    return;
  }
  if (model_tag == "NO_SPECIFICATION" ||
      strbegins(model_tag, "NOSPEC_MODEL_ID_") ||
      strbegins(model_tag, "RECAST_"))
    return;
  if (!irState) {
    modelDBLocked = variablesDBLocked = interfaceDBLocked =
      responsesDBLocked = true;
    Cerr << "\nError: no materialized study is available." << std::endl;
    abort_handler(PARSE_ERROR);
    throw PARSE_ERROR;
  }

  const size_t model_index =
    select_ir_store(irState->model, model_tag, "model", worldRank);
  set_db_model_nodes(model_index);
}


void ProblemDescDB::set_db_variables_node(const String& variables_tag)
{
  if (dbRep) {
    dbRep->set_db_variables_node(variables_tag);
    return;
  }
  if (variables_tag == "NO_SPECIFICATION")
    return;
  if (!irState) {
    variablesDBLocked = true;
    Cerr << "\nError: no materialized study is available." << std::endl;
    abort_handler(PARSE_ERROR);
    throw PARSE_ERROR;
  }

  irState->active.variables =
    select_ir_store(irState->variables, variables_tag, "variables", worldRank);
  variablesDBLocked = false;
}


void ProblemDescDB::set_db_interface_node(const String& interface_tag)
{
  if (dbRep) {
    dbRep->set_db_interface_node(interface_tag);
    return;
  }
  if (strbegins(interface_tag, "NOSPEC_INTERFACE_ID_"))
    return;
  if (!irState) {
    interfaceDBLocked = true;
    Cerr << "\nError: no materialized study is available." << std::endl;
    abort_handler(PARSE_ERROR);
    throw PARSE_ERROR;
  }

  bool warn_for_default = true;
  if (!modelDBLocked && irState->active.model < irState->model.size())
    warn_for_default =
      ir_string(irState->model[irState->active.model], "type") == "simulation";
  irState->active.interface =
    select_ir_store(irState->interface, interface_tag, "interface", worldRank,
                    warn_for_default);
  interfaceDBLocked = false;
}


void ProblemDescDB::set_db_responses_node(const String& responses_tag)
{
  if (dbRep) {
    dbRep->set_db_responses_node(responses_tag);
    return;
  }
  if (responses_tag == "NO_SPECIFICATION")
    return;
  if (!irState) {
    responsesDBLocked = true;
    Cerr << "\nError: no materialized study is available." << std::endl;
    abort_handler(PARSE_ERROR);
    throw PARSE_ERROR;
  }

  irState->active.responses =
    select_ir_store(irState->responses, responses_tag, "responses", worldRank);
  responsesDBLocked = false;
}


size_t ProblemDescDB::get_db_variables_node(const String& variables_tag) const
{
  const ProblemDescDB* db = dbRep ? dbRep.get() : this;
  if (!db->irState)
    return abort_handler_t<size_t>(PARSE_ERROR);
  return select_ir_store(db->irState->variables, variables_tag, "variables",
                         db->worldRank);
}


size_t ProblemDescDB::get_db_interface_node(const String& interface_tag) const
{
  const ProblemDescDB* db = dbRep ? dbRep.get() : this;
  if (!db->irState)
    return abort_handler_t<size_t>(PARSE_ERROR);
  return select_ir_store(db->irState->interface, interface_tag, "interface",
                         db->worldRank);
}


size_t ProblemDescDB::get_db_responses_node(const String& responses_tag) const
{
  const ProblemDescDB* db = dbRep ? dbRep.get() : this;
  if (!db->irState)
    return abort_handler_t<size_t>(PARSE_ERROR);
  return select_ir_store(db->irState->responses, responses_tag, "responses",
                         db->worldRank);
}


inline int ProblemDescDB::min_procs_per_ea()
{
  // Note: get_*() requires envelope execution (throws error if !dbRep)

  // Note: interface processors_per_analysis defaults to zero, which is used
  // when the processors_per_analysis spec is unreachable (system/fork/spawn)
  return min_procs_per_level(1, // min_ppa
    get<int>("interface.direct.processors_per_analysis"), // 0 for non-direct
    get<int>("interface.analysis_servers"));
}


int ProblemDescDB::max_procs_per_ea()
{
  int world_size = (dbRep) ? dbRep->worldSize : worldSize;
  // Note: get_*() requires envelope execution (throws error if !dbRep)

  // TO DO: can we be more fine grained on parallel testers?
  //        default tester could get hidden by plug-in...

  int max_ppa = (get<unsigned short>("interface.type") & DIRECT_INTERFACE_BIT) ?
    world_size : 1; // system/fork/spawn
  // Note: interface processors_per_analysis defaults to zero, which is used
  // when the processors_per_analysis spec is unreachable (system/fork/spawn)
  return max_procs_per_level(max_ppa,
    get<int>("interface.direct.processors_per_analysis"), // 0 for non-direct
    get<int>("interface.analysis_servers"),
    get<short>("interface.analysis_scheduling"),
    get<int>("interface.asynch_local_analysis_concurrency"),
    false, // peer dynamic not supported
    std::max(1, (int)get<const StringArray>("interface.application.analysis_drivers").size()));
}


int ProblemDescDB::min_procs_per_ie()
{
  // Note: get_*() requires envelope execution (throws error if !dbRep)

  return min_procs_per_level(min_procs_per_ea(),
			     get<int>("interface.processors_per_evaluation"),
			     get<int>("interface.evaluation_servers"));
			   //get<short>("interface.evaluation_scheduling"));
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
      ppe_spec = get<int>("interface.processors_per_evaluation"),
      max_pps  = (ppe_spec) ? ppe_spec : max_ea;
  // for peer dynamic, max_pps == 1 is imperfect in that it does not capture
  // all possibilities, but this is conservative and hopefully close enough
  // for this context (an upper bound estimate).
  bool peer_dynamic_avail = (get<short>("interface.local_evaluation_scheduling")
			     != STATIC_SCHEDULING && max_pps == 1);

  return max_procs_per_level(max_ea, ppe_spec,
    get<int>("interface.evaluation_servers"),
    get<short>("interface.evaluation_scheduling"),
    get<int>("interface.asynch_local_evaluation_concurrency"),
    peer_dynamic_avail, max_eval_concurrency);
}


void ProblemDescDB::enforce_unique_ids()
{
  if (dbRep) {
    dbRep->enforce_unique_ids();
    return;
  }

  if (!irState) {
    Cerr << "No materialized input study is available." << std::endl;
    abort_handler(PARSE_ERROR);
    throw PARSE_ERROR;
  }

  bool found_error = false;
  const auto check_block = [&](const char* block_name,
                               const std::vector<IRStore>& stores) {
    std::set<String> ids;
    for (const auto& store : stores) {
      const String& id = ir_id(store);
      if (!id.empty() && !ids.insert(id).second) {
        Cerr << "Error: id_" << block_name << " " << id
             << " appears more than once.\n";
        found_error = true;
      }
    }
  };

  check_block("method", irState->method);
  check_block("model", irState->model);
  check_block("variables", irState->variables);
  check_block("interface", irState->interface);
  check_block("responses", irState->responses);

  if (found_error) {
    abort_handler(PARSE_ERROR);
    throw PARSE_ERROR;
  }
}


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

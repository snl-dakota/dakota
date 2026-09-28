/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef PROBLEM_DESC_DB_H
#define PROBLEM_DESC_DB_H

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "DakotaMethodEnums.hpp"
#include "IRQuery.hpp"
#include "IRState.hpp"
#include "UserModes.hpp"
#include "ProblemDescDBUtils.hpp"
#include <nlohmann/json.hpp>

#include <type_traits>
#include <variant>

namespace Dakota {

// If IRValue is a std::variant<...>, detect whether T is one of its alternatives
template <class T, class Variant>
struct variant_contains;

template <class T, class... Alts>
struct variant_contains<T, std::variant<Alts...>>
  : std::disjunction<std::is_same<T, Alts>...> {};

template <class T, class Variant>
inline constexpr bool variant_contains_v = variant_contains<T, Variant>::value;

namespace pdb_detail {

// split the entry name on the first period into block.entry
inline std::pair<std::string, std::string>
split_entry_name(const std::string& entry_name, const std::string& context_msg)
{
  auto first_dot = entry_name.find(".");
  if (first_dot == std::string::npos || first_dot == entry_name.size() - 1) {
    Cerr << "\nBad entry_name '" << entry_name
         << "' in ProblemDescDB::" << context_msg << std::endl;
    abort_handler(PARSE_ERROR);
    throw PARSE_ERROR;
  }
  return { entry_name.substr(0, first_dot),
           entry_name.substr(first_dot + 1) };
}

[[noreturn]] inline void Bad_name(const String& entry_name, const String& where)
{
  Cerr << "\nBad entry_name '" << entry_name << "' in ProblemDescDB::"
       << where << std::endl;
  abort_handler(PARSE_ERROR);
  throw PARSE_ERROR;
}

[[noreturn]] inline void Locked_db()
{
  Cerr << "\nError: database is locked.  You must first unlock the database\n"
       << "       by setting the list nodes." << std::endl;
  abort_handler(PARSE_ERROR);
  throw PARSE_ERROR;
}

[[noreturn]] inline void Null_rep(const String& who)
{
  Cerr << "\nError: ProblemDescDB::" << who
       << " called with NULL representation." << std::endl;
  abort_handler(PARSE_ERROR);
  throw PARSE_ERROR;
}

} // namespace pdb_detail

class ProblemDescDB;
class ParallelLibrary;

// define the callback function for user updates to the problem DB
typedef void(*DbCallbackFunctionPtr)(Dakota::ProblemDescDB* db, void *data_ptr);


/// The database containing information parsed from the DAKOTA input file.

/** Database facade over the materialized intermediate representation of a
    Dakota study. */

class ProblemDescDB
{
  
  friend void ProblemDescDBUtils::check_and_broadcast_pdb(ProblemDescDB& problem_db,
    const String& dump_ir_path, const UserModes& user_modes,
    ParallelLibrary& parallel_lib);
  friend nlohmann::json dump_problem_desc_db_json(const ProblemDescDB& db);

public:

  //
  //- Heading: Constructors, destructor, assignment operator
  //

  /// default constructor
  ProblemDescDB() = default;
  /// standard constructor
  ProblemDescDB(int world_size, int world_rank);
  /// copy constructor
  ProblemDescDB(const ProblemDescDB& db);

  /// destructor
  virtual ~ProblemDescDB();

  /// assignment operator
  ProblemDescDB operator=(const ProblemDescDB& db);

  //
  //- Heading: Member methods
  //

  /// Enables JSON input
  void enable_json_input(const String &);
  /// Enables validated JSON input from an in-memory study object
  void enable_json_input(const nlohmann::json&);
  /// verifies that there is at least one of each of the required
  /// keywords in the dakota input file
  void check_input(const UserModes& user_modes);
  /// Locks the database in order to prevent data access when the list nodes
  /// may not be set properly.  Unlocked by a set nodes operation.
  void lock();
  /// Explicitly unlocks the database.  Use with care.
  void unlock();

  void lock_method_db();
  void unlock_method_db();
  void lock_model_db();
  void unlock_model_db();
  void lock_variables_db();
  void unlock_variables_db();
  void lock_interface_db();
  void unlock_interface_db();
  void lock_responses_db();
  void unlock_responses_db();
  


  /// set method selection based on a method identifier string to activate a
  /// particular method specification in method blocks and use pointers from
  /// this method specification to set all other subordinate block selections.
  void set_db_list_nodes(const String& method_tag);
  /// set method selection based on an index within method blocks to activate a
  /// particular method specification and use pointers from this method
  /// specification to set all other subordinate block selections.
  void set_db_list_nodes(size_t method_index);
  /// For a (default) environment lacking a top method pointer, this function
  /// is used to determine which of several potential method specifications
  /// corresponds to the top method and then sets the list nodes accordingly.
  void resolve_top_method(bool set_model_nodes = true);

  /// set method selection based on a method identifier string to activate a
  /// particular method specification (only).
  void set_db_method_node(const String& method_tag);
  /// set method selection based on an index within method blocks to activate a
  /// particular method specification (only).
  void set_db_method_node(size_t method_index);
  /// return the index of the active node in method blocks
  size_t get_db_method_node(); // restoration usage: return by value

  /// set the model subordinate block selections (model selection, variables selection,
  /// interface selection, and responses selection) based on the model
  /// identifier string
  void set_db_model_nodes(const String& model_tag);
  /// set the model subordinate block selections (model selection, variables selection,
  /// interface selection, and responses selection) based on an index
  /// within model blocks
  void set_db_model_nodes(size_t model_index);
  /// return the index of the active node in model blocks
  size_t get_db_model_node(); // restoration usage: return by value

  size_t get_db_responses_node(const String& responses_tag) const;

  size_t get_db_interface_node(const String& interface_tag) const;

  size_t get_db_variables_node(const String& variables_tag) const;

  /// Return the active indices of each top-level specification block.
  /// These are primarily used to synchronize state with the IR runtime.
  int get_active_method_index() const;
  int get_active_model_index() const;
  int get_active_variables_index() const;
  int get_active_interface_index() const;
  int get_active_responses_index() const;

  /// set variables selection based on the variables identifier string
  void set_db_variables_node(const String& variables_tag);
  /// set interface selection based on the interface identifier string
  void set_db_interface_node(const String& interface_tag);
  /// set responses selection based on the responses identifier string
  void set_db_responses_node(const String& responses_tag);

  //
  //- Heading: Set/Inquire functions
  //

  public:

  /// @brief return the name of the currently selected method
  std::string_view method_id() const;

  /// @brief return the name of the currently selected model
  std::string_view model_id() const;

  /// @brief return the name of the currently selected interface
  std::string_view interface_id() const;


  /// @brief return the dbRep
  std::shared_ptr<ProblemDescDB> get_rep() const;

  template <typename T>
  decltype(auto) get(const std::string& entry_name) const;

  // These functions get values out of the database.  A value is found by its
  // entry_name. Need a HashTable or other container with an efficient lookup
  // function here.

  /// write the full stored ProblemDescDB contents to a JSON file for debugging
  void write_json_dump(const String& output_path) const;


  // These functions are more convenient to locate within the DB in
  // terms of data access, parallel existence, and code reuse:

  /// compute minimum partition size for a parallel level based on lower
  /// level overrides
  static int min_procs_per_level(int min_procs_per_server, int pps_spec,
				 int num_serv_spec);//, short sched_spec)
  /// compute maximum partition size for a parallel level based on lower
  /// level overrides
  static int max_procs_per_level(int max_procs_per_server, int pps_spec,
				 int num_serv_spec, short sched_spec,
				 int asynch_local_conc, bool peer_dynamic_avail,
				 int max_concurrency);

  /// compute minimum evaluation partition size based on lower level overrides
  int min_procs_per_ea();
  /// compute maximum evaluation partition size based on lower level overrides
  /// and concurrency levels
  int max_procs_per_ea();

  /// compute minimum iterator partition size based on lower level overrides
  int min_procs_per_ie();
  /// compute maximum iterator partition size based on lower level overrides
  /// and concurrency levels
  int max_procs_per_ie(int max_eval_concurrency);

  /// return methodDBLocked
  bool method_locked() const;
  /// return modelDBLocked
  bool model_locked() const;
  /// return variablesDBLocked
  bool variables_locked() const;
  /// return interfaceDBLocked
  bool interface_locked() const;
  /// return responsesDBLocked
  bool responses_locked() const;

  /// function to check dbRep (does this envelope contain a letter)
  bool is_null() const;

  /// function to check if IR-backed JSON input is inactive
  bool is_json_null() const;

  /// function to check if validated JSON has been materialized into IR state
  bool has_ir_state() const;
  /// function to check if validated JSON study is available
  bool has_validated_json() const;
  /// access the validated JSON study
  const nlohmann::json& validated_json() const;

protected:

  //
  //- Heading: Constructors
  //

  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  ProblemDescDB(BaseConstructor, int world_size, int world_rank);

  //
  //- Heading: Data
  //
 

private:

  //
  //- Heading: Private convenience functions
  //

  // These functions avoid multiple instantiations of the same specification.

  /// Used by the envelope constructor to instantiate the correct letter class
  std::shared_ptr<ProblemDescDB> get_db(int world_size, int world_rank);

  /// helper function for determining whether an interface specification
  /// should be active, based on model type
  bool model_has_interface(const IRStore& model_store) const;

  /// require user-specified block identifiers to be unique
  void enforce_unique_ids();



  //
  //- Heading: Data
  //
 

  /// prevents use of get_<type> retrieval and set_<type> update functions 
  /// prior to setting the list node for the active method specification
  bool methodDBLocked;
  /// prevents use of get_<type> retrieval and set_<type> update functions 
  /// prior to setting the list node for the active model specification
  bool modelDBLocked;
  /// prevents use of get_<type> retrieval and set_<type> update functions 
  /// prior to setting the list node for the active variables specification
  bool variablesDBLocked;
  /// prevents use of get_<type> retrieval and set_<type> update functions 
  /// prior to setting the list node for the active interface specification
  bool interfaceDBLocked;
  /// prevents use of get_<type> retrieval and set_<type> update functions 
  /// prior to setting the list node for the active responses specification
  bool responsesDBLocked;

  /// pointer to the letter (initialized only for the envelope)
  std::shared_ptr<ProblemDescDB> dbRep;

  /// materialized IR for validated JSON input
  std::shared_ptr<IRState> irState;
  /// validated JSON study for broadcast/re-materialization
  nlohmann::json validatedStudyJson;


  /// MPI world rank
  int worldRank;
  /// MPI world size
  int worldSize;
};


template <typename T>
decltype(auto) ProblemDescDB::get(const std::string& entry_name) const
{
  // pick the “active db” (letter if envelope, else this)
  const ProblemDescDB* db = dbRep ? dbRep.get() : this;

  // lock checks
  std::string block, entry;
  std::tie(block, entry) = pdb_detail::split_entry_name(entry_name, "get<T>()");

  if (block == "method" && db->methodDBLocked)
    pdb_detail::Locked_db();
  else if (block == "model" && db->modelDBLocked)
    pdb_detail::Locked_db();
  else if (block == "variables" && db->variablesDBLocked)
    pdb_detail::Locked_db();
  else if (block == "interface" && db->interfaceDBLocked)
    pdb_detail::Locked_db();
  else if (block == "responses" && db->responsesDBLocked)
    pdb_detail::Locked_db();

  using QueryT = std::remove_const_t<T>;

  // Only allow IR-backed queries for types supported by IRValue (same as your cpp)
  if constexpr (variant_contains_v<QueryT, IRValue>) {
    if (db->irState) {
      try {
        return ir_query::get<QueryT>(*db->irState, entry_name);
      }
      catch (const std::exception& e) {
        Cerr << "\nParser failed with exception: " << e.what() << std::endl;
        abort_handler(PARSE_ERROR);
      }
    }
  }

  pdb_detail::Bad_name(entry_name, "get<T>()");
}

inline void ProblemDescDB::lock()
{
  if (dbRep)
    dbRep->methodDBLocked = dbRep->modelDBLocked = dbRep->variablesDBLocked
      = dbRep->interfaceDBLocked = dbRep->responsesDBLocked = true;
  else
    methodDBLocked = modelDBLocked = variablesDBLocked = interfaceDBLocked
      = responsesDBLocked = true;
}


inline void ProblemDescDB::unlock()
{
  if (dbRep)
    dbRep->methodDBLocked = dbRep->modelDBLocked = dbRep->variablesDBLocked
      = dbRep->interfaceDBLocked = dbRep->responsesDBLocked = false;
  else
    methodDBLocked = modelDBLocked = variablesDBLocked = interfaceDBLocked
      = responsesDBLocked = false;
}


inline std::string_view ProblemDescDB::method_id() const {
  const ProblemDescDB* db = dbRep ? dbRep.get() : this;
  static const String empty_id;
  if (db->irState && !db->method_locked() &&
      db->irState->active.method < db->irState->method.size() &&
      db->irState->method[db->irState->active.method].contains("id"))
    return db->irState->method[db->irState->active.method].get<String>("id");
  return empty_id;
}

inline std::string_view ProblemDescDB::model_id() const {
  const ProblemDescDB* db = dbRep ? dbRep.get() : this;
  static const String empty_id;
  if (db->irState && !db->model_locked() &&
      db->irState->active.model < db->irState->model.size() &&
      db->irState->model[db->irState->active.model].contains("id"))
    return db->irState->model[db->irState->active.model].get<String>("id");
  return empty_id;
}

inline std::string_view ProblemDescDB::interface_id() const {
  const ProblemDescDB* db = dbRep ? dbRep.get() : this;
  static const String empty_id;
  if (db->irState && !db->interface_locked() &&
      db->irState->active.interface < db->irState->interface.size() &&
      db->irState->interface[db->irState->active.interface].contains("id"))
    return db->irState->interface[db->irState->active.interface].get<String>("id");
  return empty_id;
}

inline std::shared_ptr<ProblemDescDB> ProblemDescDB::get_rep() const {
  return dbRep;
}

inline size_t ProblemDescDB::get_db_method_node()
{
  if (dbRep)
    return dbRep->get_db_method_node();
  return (!irState || methodDBLocked) ? _NPOS : irState->active.method;
}


inline size_t ProblemDescDB::get_db_model_node()
{
  if (dbRep)
    return dbRep->get_db_model_node();
  return (!irState || modelDBLocked) ? _NPOS : irState->active.model;
}


inline bool ProblemDescDB::method_locked() const
{ return (dbRep) ? dbRep->methodDBLocked : methodDBLocked; }


inline bool ProblemDescDB::model_locked() const
{ return (dbRep) ? dbRep->modelDBLocked : modelDBLocked; }


inline bool ProblemDescDB::variables_locked() const
{ return (dbRep) ? dbRep->variablesDBLocked : variablesDBLocked; }


inline bool ProblemDescDB::interface_locked() const
{ return (dbRep) ? dbRep->interfaceDBLocked : interfaceDBLocked; }


inline bool ProblemDescDB::responses_locked() const
{ return (dbRep) ? dbRep->responsesDBLocked : responsesDBLocked; }


inline bool ProblemDescDB::is_null() const
{ return (dbRep) ? false : true; }

inline bool ProblemDescDB::is_json_null() const
{
  const ProblemDescDB* db = dbRep ? dbRep.get() : this;
  return (db->irState) ? false : true;
}

inline bool ProblemDescDB::has_ir_state() const
{
  const ProblemDescDB* db = dbRep ? dbRep.get() : this;
  return static_cast<bool>(db->irState);
}

inline bool ProblemDescDB::has_validated_json() const
{
  const ProblemDescDB* db = dbRep ? dbRep.get() : this;
  return !db->validatedStudyJson.is_null();
}

inline const nlohmann::json& ProblemDescDB::validated_json() const
{
  const ProblemDescDB* db = dbRep ? dbRep.get() : this;
  return db->validatedStudyJson;
}

inline int ProblemDescDB::get_active_method_index() const
{
  const ProblemDescDB* db = dbRep ? dbRep.get() : this;
  return (db->irState && !db->methodDBLocked) ?
    static_cast<int>(db->irState->active.method) : 0;
}

inline int ProblemDescDB::get_active_model_index() const
{
  const ProblemDescDB* db = dbRep ? dbRep.get() : this;
  return (db->irState && !db->modelDBLocked) ?
    static_cast<int>(db->irState->active.model) : 0;
}

inline int ProblemDescDB::get_active_variables_index() const
{
  const ProblemDescDB* db = dbRep ? dbRep.get() : this;
  return (db->irState && !db->variablesDBLocked) ?
    static_cast<int>(db->irState->active.variables) : 0;
}

inline int ProblemDescDB::get_active_interface_index() const
{
  const ProblemDescDB* db = dbRep ? dbRep.get() : this;
  return (db->irState && !db->interfaceDBLocked) ?
    static_cast<int>(db->irState->active.interface) : 0;
}

inline int ProblemDescDB::get_active_responses_index() const
{
  const ProblemDescDB* db = dbRep ? dbRep.get() : this;
  return (db->irState && !db->responsesDBLocked) ?
    static_cast<int>(db->irState->active.responses) : 0;
}


inline int ProblemDescDB::
min_procs_per_level(int min_procs_per_server, int pps_spec, int num_serv_spec)
                    //, short sched_spec)
{
  int min_procs_per_lev = (pps_spec) ? pps_spec : min_procs_per_server;
  if (num_serv_spec)
    min_procs_per_lev *= num_serv_spec;
  //if (sched_spec == DEDICATED_SCHEDULER_DYNAMIC) ++min_procs_per_lev;
  return min_procs_per_lev;
}


inline int ProblemDescDB::
max_procs_per_level(int max_procs_per_server, int pps_spec, int num_serv_spec,
		    short sched_spec, int asynch_local_conc,
		    bool peer_dynamic_avail, int max_concurrency)
{
  int max_procs_per_lev, max_pps = (pps_spec) ? pps_spec : max_procs_per_server;

  if (num_serv_spec) { // check for dedicated scheduler
    max_procs_per_lev = max_pps * num_serv_spec;
    switch (sched_spec) {
    case DEDICATED_SCHEDULER_DYNAMIC:
      ++max_procs_per_lev; break;
    //case PEER_SCHEDULING: case PEER_STATIC_SCHEDULING:
    //case PEER_DYNAMIC_SCHEDULING:
    //  break;
    case DEFAULT_SCHEDULING: // emulate auto-config logic
      if (!peer_dynamic_avail && num_serv_spec > 1 &&
	  num_serv_spec * std::max(1, asynch_local_conc) < max_concurrency)
	++max_procs_per_lev;
      break;
    }
  }
  else {
    max_procs_per_lev = max_pps * max_concurrency;
    // assume peer partition unless explicit override to ded scheduler,
    // since we don't have avail_procs to estimate need for ded scheduler
    if (sched_spec == DEDICATED_SCHEDULER_DYNAMIC)
      ++max_procs_per_lev;
  }

  return max_procs_per_lev;
}


inline bool ProblemDescDB::model_has_interface(const IRStore& model_store) const
{
  const String& model_type = model_store.get<String>("type");
  const String& surrogate_type = model_store.get<String>("surrogate.type");
  return (model_type == "simulation" || model_type == "nested" ||
          (model_type == "surrogate" && surrogate_type != "ensemble"));
}

/// A minimal letter (Rep) class to preserve the existing LO setup
class ProblemDescDBRep: public ProblemDescDB
{
  public:
    /// constructor
    ProblemDescDBRep(int world_size, int world_rank):
      ProblemDescDB(BaseConstructor(), world_size, world_rank)
  { }

    /// destructor
    ~ProblemDescDBRep() { }
};

} // namespace Dakota

#endif

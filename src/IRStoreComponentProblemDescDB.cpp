#include "IRStoreComponentProblemDescDB.hpp"

#include <nlohmann/json.hpp>

namespace Dakota {
namespace ir_component_db {

namespace {

IRStore default_store(irgen::BlockType block)
{
  static const InstructionMaterializer materializer;
  return materializer.materialize_block(nlohmann::json::object(), block);
}

template <typename T>
const IRStore& provided_or_default(const T* store, const IRStore& fallback)
{
  return store ? *store : fallback;
}

} // namespace

std::shared_ptr<ProblemDescDB> make_problem_db(
  int world_size,
  int world_rank,
  const IRStore* environment_store,
  const IRStore* method_store,
  const IRStore* model_store,
  const IRStore* variables_store,
  const IRStore* interface_store,
  const IRStore* responses_store)
{
  const IRStore default_environment = default_store(irgen::BlockType::Environment);
  const IRStore default_method      = default_store(irgen::BlockType::Method);
  const IRStore default_model       = default_store(irgen::BlockType::Model);
  const IRStore default_variables   = default_store(irgen::BlockType::Variables);
  const IRStore default_interface   = default_store(irgen::BlockType::Interface);
  const IRStore default_responses   = default_store(irgen::BlockType::Responses);

  auto db = std::make_shared<ProblemDescDB>(world_size, world_rank);
  ProblemDescDB* rep = db->dbRep ? db->dbRep.get() : db.get();

  rep->irState = std::make_shared<IRState>();
  rep->unlock();

  rep->irState->environment =
    provided_or_default(environment_store, default_environment);
  rep->irState->method.assign(
    1, provided_or_default(method_store, default_method));
  rep->irState->model.assign(
    1, provided_or_default(model_store, default_model));
  rep->irState->variables.assign(
    1, provided_or_default(variables_store, default_variables));
  rep->irState->interface.assign(
    1, provided_or_default(interface_store, default_interface));
  rep->irState->responses.assign(
    1, provided_or_default(responses_store, default_responses));

  rep->populate_skeleton_data_from_ir();

  // Mirror the legacy single-study access pattern by selecting the sole
  // method/model entries explicitly. This, in turn, activates the associated
  // variables/interface/responses nodes through existing pointer resolution.
  if (!rep->dataMethodList.empty())
    rep->set_db_method_node(0);
  if (!rep->dataModelList.empty())
    rep->set_db_model_nodes(0);

  return db;
}

const ProblemDescDB& temporary_problem_db(
  int world_size,
  int world_rank,
  const IRStore* environment_store,
  const IRStore* method_store,
  const IRStore* model_store,
  const IRStore* variables_store,
  const IRStore* interface_store,
  const IRStore* responses_store)
{
  thread_local std::shared_ptr<ProblemDescDB> db;
  db = make_problem_db(world_size, world_rank, environment_store, method_store,
    model_store, variables_store, interface_store, responses_store);
  return *db;
}

} // namespace ir_component_db
} // namespace Dakota

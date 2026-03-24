/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "InstructionMaterializer.hpp"

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdexcept>

namespace Dakota {

namespace {

bool debug_logging_enabled()
{
  const char* value = std::getenv("DAKOTA_DEBUG_IR");
  return value && *value;
}

const char* block_name(const Dakota::irgen::BlockType block)
{
  using Dakota::irgen::BlockType;
  switch (block) {
  case BlockType::Environment: return "environment";
  case BlockType::Method: return "method";
  case BlockType::Model: return "model";
  case BlockType::Variables: return "variables";
  case BlockType::Interface: return "interface";
  case BlockType::Responses: return "responses";
  }
  return "unknown";
}

} // namespace

bool WriteTracker::mark_written(irgen::BlockType block, const String& local_key)
{
  switch (block) {
  case irgen::BlockType::Environment:
    return environment.insert(local_key).second;
  case irgen::BlockType::Method:
    return method.insert(local_key).second;
  case irgen::BlockType::Model:
    return model.insert(local_key).second;
  case irgen::BlockType::Variables:
    return variables.insert(local_key).second;
  case irgen::BlockType::Interface:
    return interface.insert(local_key).second;
  case irgen::BlockType::Responses:
    return responses.insert(local_key).second;
  }
  return false;
}

const std::unordered_map<irgen::OpKind, InstructionMaterializer::OpHandler>&
InstructionMaterializer::op_handlers()
{
  static const std::unordered_map<irgen::OpKind, OpHandler> handlers{
    {irgen::OpKind::AddToValue, &InstructionMaterializer::handle_add_to_value},
    {irgen::OpKind::DirectValue, &InstructionMaterializer::handle_direct_value},
    {irgen::OpKind::LiteralAssign, &InstructionMaterializer::handle_literal_assign},
    {irgen::OpKind::PresenceBool, &InstructionMaterializer::handle_presence_bool},
    {irgen::OpKind::PresenceLiteral, &InstructionMaterializer::handle_presence_literal},
    {irgen::OpKind::PresenceEnum, &InstructionMaterializer::handle_presence_enum},
    {irgen::OpKind::AugmentEnum, &InstructionMaterializer::handle_augment_enum},
    {irgen::OpKind::Categorical, &InstructionMaterializer::handle_categorical},
    {irgen::OpKind::AdjacencyMatrix, &InstructionMaterializer::handle_adjacency_matrix},
    {irgen::OpKind::DiscreteSetValues, &InstructionMaterializer::handle_discrete_set_values},
    {irgen::OpKind::HistogramBinUncertain, &InstructionMaterializer::handle_histogram_bin_uncertain},
    {irgen::OpKind::HistogramPointUncertain, &InstructionMaterializer::handle_histogram_point_uncertain},
    {irgen::OpKind::DiscreteUncertainSetValuesProbs,
      &InstructionMaterializer::handle_discrete_uncertain_set_values_probs},
    {irgen::OpKind::ContinuousIntervalUncertain,
      &InstructionMaterializer::handle_continuous_interval_uncertain},
    {irgen::OpKind::DiscreteIntervalUncertain,
      &InstructionMaterializer::handle_discrete_interval_uncertain},
    {irgen::OpKind::UncertainCorrelationMatrix,
      &InstructionMaterializer::handle_uncertain_correlation_matrix},
    {irgen::OpKind::UncertainInitPointFlag,
      &InstructionMaterializer::handle_uncertain_init_point_flag},
    {irgen::OpKind::IdToIndexSet,
      &InstructionMaterializer::handle_id_to_index_set},
    {irgen::OpKind::IntSet,
      &InstructionMaterializer::handle_int_set},
    {irgen::OpKind::ResponseLevelsArray,
      &InstructionMaterializer::handle_response_levels_array},
    {irgen::OpKind::AnalysisComponents,
      &InstructionMaterializer::handle_analysis_components},
  };
  return handlers;
}

IRState InstructionMaterializer::materialize(const nlohmann::json& validated_json) const
{
  if (!validated_json.is_object()) {
    throw std::runtime_error(
      "InstructionMaterializer::materialize expected validated top-level object.");
  }

  IRState state;

  if (debug_logging_enabled()) {
    std::cerr << "InstructionMaterializer: top-level keys:";
    for (auto it = validated_json.begin(); it != validated_json.end(); ++it) {
      std::cerr << " " << it.key() << "("
                << (it->is_object() ? "object" :
                    it->is_array() ? "array" :
                    it->type_name())
                << ")";
    }
    std::cerr << std::endl;
  }

  const auto resize_block = [&](const char* key, auto& stores) {
    auto it = validated_json.find(key);
    if (it == validated_json.end())
      return;
    if (!it->is_array()) {
      throw std::runtime_error(
        "InstructionMaterializer::materialize expected top-level array for block '" +
        std::string(key) + "'");
    }
    stores.resize(it->size());
    if (debug_logging_enabled()) {
      std::cerr << "InstructionMaterializer: block '" << key
                << "' has " << stores.size() << " entries" << std::endl;
    }
  };

  resize_block("method", state.method);
  resize_block("model", state.model);
  resize_block("variables", state.variables);
  resize_block("interface", state.interface);
  resize_block("responses", state.responses);

  if (state.model.empty()) {
    state.model.resize(1);
    if (debug_logging_enabled()) {
      std::cerr << "InstructionMaterializer: synthesizing default model block"
                << std::endl;
    }
  }

  if (state.interface.empty()) {
    state.interface.resize(1);
    if (debug_logging_enabled()) {
      std::cerr << "InstructionMaterializer: synthesizing default interface block"
                << std::endl;
    }
  }

  initialize_defaults(state);

  WriteTracker writes;
  if (const auto env_it = validated_json.find("environment");
      env_it != validated_json.end()) {
    if (!env_it->is_object()) {
      throw std::runtime_error(
        "InstructionMaterializer::materialize expected object for top-level block 'environment'");
    }
    if (debug_logging_enabled())
      std::cerr << "InstructionMaterializer: materializing environment block" << std::endl;
    materialize_block(
      *env_it, irgen::BlockType::Environment, state.environment, writes);
  }

  const auto materialize_array_block =
    [&](const char* key, irgen::BlockType block, auto& stores) {
      const auto it = validated_json.find(key);
      if (it == validated_json.end())
        return;
      for (size_t i = 0; i < it->size(); ++i) {
        if (!(*it)[i].is_object()) {
          throw std::runtime_error(
            "InstructionMaterializer::materialize expected object entries in top-level block '" +
            std::string(key) + "'");
        }
        if (debug_logging_enabled()) {
          std::cerr << "InstructionMaterializer: materializing block '" << key
                    << "' index " << i << std::endl;
        }
        materialize_block((*it)[i], block, stores[i], writes);
      }
    };

  materialize_array_block("method", irgen::BlockType::Method, state.method);
  materialize_array_block("model", irgen::BlockType::Model, state.model);
  materialize_array_block("variables", irgen::BlockType::Variables, state.variables);
  materialize_array_block("interface", irgen::BlockType::Interface, state.interface);
  materialize_array_block("responses", irgen::BlockType::Responses, state.responses);

  const auto index_ids =
    [](const auto& stores, auto& map) {
      for (size_t i = 0; i < stores.size(); ++i) {
        if (stores[i].contains("id"))
          map[stores[i].template get<String>("id")] = i;
      }
    };

  index_ids(state.method, state.method_id_to_index);
  index_ids(state.model, state.model_id_to_index);
  index_ids(state.variables, state.variables_id_to_index);
  index_ids(state.interface, state.interface_id_to_index);
  index_ids(state.responses, state.responses_id_to_index);

  if (state.environment.contains("top_method_pointer"))
    state.set_active_method(state.environment.get<String>("top_method_pointer"));

  if (!state.method.empty() &&
      state.active.method < state.method.size() &&
      state.method[state.active.method].contains("model_pointer")) {
    state.set_active_model(
      state.method[state.active.method].get<String>("model_pointer"));
  }
  if (!state.model.empty() &&
      state.active.model < state.model.size()) {
    const auto& model_store = state.model[state.active.model];
    if (model_store.contains("variables_pointer"))
      state.set_active_variables(model_store.get<String>("variables_pointer"));
    if (model_store.contains("interface_pointer"))
      state.set_active_interface(model_store.get<String>("interface_pointer"));
    if (model_store.contains("responses_pointer"))
      state.set_active_responses(model_store.get<String>("responses_pointer"));
  }

  return state;
}

void InstructionMaterializer::initialize_defaults(IRState& state) const
{
  auto initialize_store_defaults =
    [](const irgen::BlockTables& tables, IRStore& store) {
      for (const auto& [local_key, contract] : tables.contracts)
        store.set_value(local_key, contract.default_value);
    };

  initialize_store_defaults(
    irgen::tables_for_block(irgen::BlockType::Environment), state.environment);

  const auto initialize_block_vector =
    [&](irgen::BlockType block, auto& stores) {
      const auto& tables = irgen::tables_for_block(block);
      for (auto& store : stores)
        initialize_store_defaults(tables, store);
    };

  initialize_block_vector(irgen::BlockType::Method, state.method);
  initialize_block_vector(irgen::BlockType::Model, state.model);
  initialize_block_vector(irgen::BlockType::Variables, state.variables);
  initialize_block_vector(irgen::BlockType::Interface, state.interface);
  initialize_block_vector(irgen::BlockType::Responses, state.responses);
}

void InstructionMaterializer::materialize_block(const nlohmann::json& block_json,
                                                irgen::BlockType block,
                                                IRStore& store,
                                                WriteTracker& writes) const
{
  const auto& tables = irgen::tables_for_block(block);
  if (debug_logging_enabled()) {
    std::cerr << "InstructionMaterializer: block tables contracts="
              << tables.contracts.size()
              << " instructions=" << tables.instructions.size()
              << std::endl;
  }

  std::function<void(const nlohmann::json&, std::string_view)> visit =
    [&](const nlohmann::json& node, std::string_view path) {
      const auto it = tables.instructions.find(std::string(path));
      if (it != tables.instructions.end()) {
        if (debug_logging_enabled()) {
          std::cerr << "InstructionMaterializer: path '" << path
                    << "' has " << it->second.size() << " write op(s)" << std::endl;
        }
        for (const auto& op : it->second)
          apply_write_op(block_json, path, op, tables, block, store, writes);
      }

      if (!node.is_object())
        return;

      for (auto iter = node.begin(); iter != node.end(); ++iter) {
        const std::string child_path =
          path.empty() ? iter.key() : std::string(path) + "/" + iter.key();
        visit(iter.value(), child_path);
      }
    };

  visit(block_json, "");
}

void InstructionMaterializer::apply_write_op(const nlohmann::json& block_json,
                                             std::string_view current_path,
                                             const irgen::WriteOp& op,
                                             const irgen::BlockTables& tables,
                                             irgen::BlockType block,
                                             IRStore& store,
                                             WriteTracker& writes) const
{
  const auto contract_it = tables.contracts.find(op.target_local_ir_key);
  if (contract_it == tables.contracts.end()) {
    throw std::runtime_error(
      "InstructionMaterializer::apply_write_op missing contract for target '" +
      op.target_local_ir_key + "'");
  }

  const auto handler_it = op_handlers().find(op.op_kind);
  if (handler_it == op_handlers().end()) {
    throw std::runtime_error(
      "InstructionMaterializer::apply_write_op missing handler for op kind");
  }

  const HandlerContext ctx{block_json, store, current_path};
  if (debug_logging_enabled()) {
    std::cerr << "InstructionMaterializer: applying op kind "
              << static_cast<int>(op.op_kind)
              << " at path '" << current_path
              << "' -> '" << op.target_local_ir_key << "'" << std::endl;
  }
  try {
    handler_it->second(op, contract_it->second, ctx);
  }
  catch (const std::exception& e) {
    if (debug_logging_enabled()) {
      std::cerr << "InstructionMaterializer: handler failed at path '"
                << current_path << "' target '" << op.target_local_ir_key
                << "': " << e.what() << std::endl;
    }
    throw std::runtime_error(
      std::string("InstructionMaterializer: handler failed in block '") +
      block_name(block) + "' at path '" + std::string(current_path) +
      "' -> target '" + op.target_local_ir_key + "' (op kind " +
      std::to_string(static_cast<int>(op.op_kind)) + "): " + e.what());
  }
  writes.mark_written(block, op.target_local_ir_key);
}

} // namespace Dakota

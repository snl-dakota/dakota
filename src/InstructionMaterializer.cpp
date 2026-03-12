/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "InstructionMaterializer.hpp"

#include <nlohmann/json.hpp>

#include <stdexcept>

namespace Dakota {

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
    {irgen::OpKind::IdToIndexSet,
      &InstructionMaterializer::handle_id_to_index_set},
    {irgen::OpKind::IntSet,
      &InstructionMaterializer::handle_int_set},
  };
  return handlers;
}

IRState InstructionMaterializer::materialize(const nlohmann::json& validated_json) const
{
  (void)validated_json;
  throw std::runtime_error(
    "InstructionMaterializer::materialize not implemented yet.");
}

void InstructionMaterializer::initialize_defaults(IRState& state) const
{
  (void)state;
  throw std::runtime_error(
    "InstructionMaterializer::initialize_defaults not implemented yet.");
}

void InstructionMaterializer::materialize_block(const nlohmann::json& block_json,
                                                irgen::BlockType block,
                                                IRStore& store,
                                                WriteTracker& writes) const
{
  (void)block_json;
  (void)block;
  (void)store;
  (void)writes;
  throw std::runtime_error(
    "InstructionMaterializer::materialize_block not implemented yet.");
}

void InstructionMaterializer::apply_write_op(const nlohmann::json& block_json,
                                             std::string_view current_path,
                                             const irgen::WriteOp& op,
                                             const irgen::BlockTables& tables,
                                             irgen::BlockType block,
                                             IRStore& store,
                                             WriteTracker& writes) const
{
  (void)block_json;
  (void)current_path;
  (void)op;
  (void)tables;
  (void)block;
  (void)store;
  (void)writes;
  throw std::runtime_error(
    "InstructionMaterializer::apply_write_op not implemented yet.");
}

} // namespace Dakota

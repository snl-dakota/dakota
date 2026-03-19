/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_INSTRUCTION_MATERIALIZER_H
#define DAKOTA_INSTRUCTION_MATERIALIZER_H

#include "IRState.hpp"
#include "generated_ir_table_types.hpp"

#include <nlohmann/json_fwd.hpp>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace Dakota {

namespace irgen = dakota::irgen;
struct InstructionMaterializerTestAccess;

/// Tracks keys explicitly written during one materialization pass.
struct WriteTracker
{
  std::unordered_set<String> environment;
  std::unordered_set<String> method;
  std::unordered_set<String> model;
  std::unordered_set<String> variables;
  std::unordered_set<String> interface;
  std::unordered_set<String> responses;

  bool mark_written(irgen::BlockType block, const String& local_key);
};

/// Materializes a validated JSON study into block-scoped IRState
/// using generated contracts and keyword instructions.
class InstructionMaterializer
{
public:
  InstructionMaterializer() = default;

  /// Build and return a fully materialized IR state.
  IRState materialize(const nlohmann::json& validated_json) const;

  friend struct InstructionMaterializerTestAccess;

private:
  struct HandlerContext
  {
    const nlohmann::json& block_json;
    IRStore& store;
    std::string_view current_path;
  };

  using OpHandler = std::function<void(
    const irgen::WriteOp& op,
    const irgen::KeyContract& contract,
    const HandlerContext& ctx
  )>;

  /// Registry for OpKind-dispatched keyword handlers.
  static const std::unordered_map<irgen::OpKind, OpHandler>& op_handlers();

  /// Initialize all IR keys from generated contract defaults.
  void initialize_defaults(IRState& state) const;

  /// Execute instruction writes for a single block instance.
  void materialize_block(const nlohmann::json& block_json,
                         irgen::BlockType block,
                         IRStore& store,
                         WriteTracker& writes) const;

  /// Apply one generated write operation.
  void apply_write_op(const nlohmann::json& block_json,
                      std::string_view current_path,
                      const irgen::WriteOp& op,
                      const irgen::BlockTables& tables,
                      irgen::BlockType block,
                      IRStore& store,
                      WriteTracker& writes) const;

  static void handle_direct_value(const irgen::WriteOp& op,
                                  const irgen::KeyContract& contract,
                                  const HandlerContext& ctx);
  static void handle_literal_assign(const irgen::WriteOp& op,
                                    const irgen::KeyContract& contract,
                                    const HandlerContext& ctx);
  static void handle_presence_bool(const irgen::WriteOp& op,
                                   const irgen::KeyContract& contract,
                                   const HandlerContext& ctx);
  static void handle_presence_literal(const irgen::WriteOp& op,
                                      const irgen::KeyContract& contract,
                                      const HandlerContext& ctx);
  static void handle_presence_enum(const irgen::WriteOp& op,
                                   const irgen::KeyContract& contract,
                                   const HandlerContext& ctx);
  static void handle_augment_enum(const irgen::WriteOp& op,
                                  const irgen::KeyContract& contract,
                                  const HandlerContext& ctx);
  static void handle_categorical(const irgen::WriteOp& op,
                                 const irgen::KeyContract& contract,
                                 const HandlerContext& ctx);
  static void handle_adjacency_matrix(const irgen::WriteOp& op,
                                      const irgen::KeyContract& contract,
                                      const HandlerContext& ctx);
  static void handle_discrete_set_values(const irgen::WriteOp& op,
                                         const irgen::KeyContract& contract,
                                         const HandlerContext& ctx);
  static void handle_histogram_bin_uncertain(const irgen::WriteOp& op,
                                             const irgen::KeyContract& contract,
                                             const HandlerContext& ctx);
  static void handle_histogram_point_uncertain(const irgen::WriteOp& op,
                                               const irgen::KeyContract& contract,
                                               const HandlerContext& ctx);
  static void handle_discrete_uncertain_set_values_probs(
    const irgen::WriteOp& op,
    const irgen::KeyContract& contract,
    const HandlerContext& ctx);
  static void handle_continuous_interval_uncertain(const irgen::WriteOp& op,
                                                   const irgen::KeyContract& contract,
                                                   const HandlerContext& ctx);
  static void handle_discrete_interval_uncertain(const irgen::WriteOp& op,
                                                 const irgen::KeyContract& contract,
                                                 const HandlerContext& ctx);
  static void handle_uncertain_correlation_matrix(const irgen::WriteOp& op,
                                                  const irgen::KeyContract& contract,
                                                  const HandlerContext& ctx);
  static void handle_uncertain_init_point_flag(const irgen::WriteOp& op,
                                               const irgen::KeyContract& contract,
                                               const HandlerContext& ctx);
  static void handle_id_to_index_set(const irgen::WriteOp& op,
                                     const irgen::KeyContract& contract,
                                     const HandlerContext& ctx);
  static void handle_int_set(const irgen::WriteOp& op,
                             const irgen::KeyContract& contract,
                             const HandlerContext& ctx);
  static void handle_response_levels_array(const irgen::WriteOp& op,
                                           const irgen::KeyContract& contract,
                                           const HandlerContext& ctx);
};

/// Internal testing seam for focused unit coverage of individual handlers.
struct InstructionMaterializerTestAccess
{
  static void invoke_handler(const irgen::WriteOp& op,
                             const irgen::KeyContract& contract,
                             const nlohmann::json& block_json,
                             std::string_view current_path,
                             IRStore& store)
  {
    const InstructionMaterializer::HandlerContext ctx{
      block_json, store, current_path
    };
    const auto& handler = InstructionMaterializer::op_handlers().at(op.op_kind);
    handler(op, contract, ctx);
  }
};

} // namespace Dakota

#endif // DAKOTA_INSTRUCTION_MATERIALIZER_H

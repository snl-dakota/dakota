/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "InstructionMaterializer.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <stdexcept>
#include <string>
#include <utility>

using json = nlohmann::json;
using namespace Dakota;

namespace {

irgen::KeyContract make_contract(irgen::IrValueType value_type)
{
  return irgen::KeyContract{
    value_type, irgen::DefaultSource::Policy, IRValue(String())
  };
}

irgen::OpLiteral make_literal(irgen::IrValueType value_type, IRValue value)
{
  return irgen::OpLiteral{value_type, std::move(value)};
}

irgen::WriteOp make_op(irgen::OpKind kind,
                       const String& target,
                       irgen::OpLiteral literal = make_literal(
                         irgen::IrValueType::String, IRValue(String())))
{
  return irgen::WriteOp{kind, target, std::move(literal)};
}

IRStore invoke_handler(const irgen::WriteOp& op,
                       const irgen::KeyContract& contract,
                       const json& block_json,
                       std::string_view current_path)
{
  IRStore store;
  InstructionMaterializerTestAccess::invoke_handler(
    op, contract, block_json, current_path, store);
  return store;
}

} // namespace

TEST(instruction_materializer_tests, direct_value_writes_requested_ir_type)
{
  const auto op = make_op(irgen::OpKind::DirectValue, "samples");
  const auto store = invoke_handler(
    op,
    make_contract(irgen::IrValueType::Int),
    json{{"sampling", {{"samples", 12}}}},
    "sampling/samples");

  EXPECT_EQ(store.get<int>("samples"), 12);
}

TEST(instruction_materializer_tests, literal_assign_uses_literal_without_json_lookup)
{
  const auto op = make_op(
    irgen::OpKind::LiteralAssign,
    "strategy",
    make_literal(irgen::IrValueType::String, IRValue(String("lhs"))));
  const auto store = invoke_handler(op, make_contract(irgen::IrValueType::String), json::object(), "");

  EXPECT_EQ(store.get<String>("strategy"), "lhs");
}

TEST(instruction_materializer_tests, presence_bool_treats_object_as_true)
{
  const auto op = make_op(irgen::OpKind::PresenceBool, "enabled");
  const auto store = invoke_handler(
    op,
    make_contract(irgen::IrValueType::Bool),
    json{{"flag", json::object()}},
    "flag");

  EXPECT_TRUE(store.get<bool>("enabled"));
}

TEST(instruction_materializer_tests, presence_literal_writes_literal_when_path_exists)
{
  const auto op = make_op(
    irgen::OpKind::PresenceLiteral,
    "level",
    make_literal(irgen::IrValueType::Int, IRValue(7)));
  const auto store = invoke_handler(
    op,
    make_contract(irgen::IrValueType::Int),
    json{{"present", true}},
    "present");

  EXPECT_EQ(store.get<int>("level"), 7);
}

TEST(instruction_materializer_tests, presence_enum_writes_integral_literal)
{
  const auto op = make_op(
    irgen::OpKind::PresenceEnum,
    "enum_mask",
    make_literal(irgen::IrValueType::Int, IRValue(4)));
  const auto store = invoke_handler(
    op,
    make_contract(irgen::IrValueType::Int),
    json{{"present", true}},
    "present");

  EXPECT_EQ(store.get<int>("enum_mask"), 4);
}

TEST(instruction_materializer_tests, augment_enum_ors_existing_value_with_literal)
{
  IRStore store;
  store.set_value("enum_mask", IRValue(1));
  const auto op = make_op(
    irgen::OpKind::AugmentEnum,
    "enum_mask",
    make_literal(irgen::IrValueType::Int, IRValue(4)));

  InstructionMaterializerTestAccess::invoke_handler(
    op,
    make_contract(irgen::IrValueType::Int),
    json{{"present", true}},
    "present",
    store);

  EXPECT_EQ(store.get<int>("enum_mask"), 5);
}

TEST(instruction_materializer_tests, categorical_maps_truthy_strings_to_bits)
{
  const auto op = make_op(irgen::OpKind::Categorical, "categories");
  const auto store = invoke_handler(
    op,
    make_contract(irgen::IrValueType::BitArray),
    json{{"flags", json::array({"true", "No", "yes", ""})}},
    "flags");
  const auto& bits = store.get<BitArray>("categories");

  ASSERT_EQ(bits.size(), 4U);
  EXPECT_TRUE(bits[0]);
  EXPECT_FALSE(bits[1]);
  EXPECT_TRUE(bits[2]);
  EXPECT_FALSE(bits[3]);
}

TEST(instruction_materializer_tests, adjacency_matrix_reshapes_flat_values_per_variable)
{
  const auto op = make_op(irgen::OpKind::AdjacencyMatrix, "adjacency");
  const auto store = invoke_handler(
    op,
    make_contract(irgen::IrValueType::RealMatrixArray),
    json{{"graph", {{"count", 2}, {"adjacency_matrix", json::array({1, 0, 0, 1, 0, 1, 1, 0})}}}},
    "graph/adjacency_matrix");
  const auto& matrices = store.get<RealMatrixArray>("adjacency");

  ASSERT_EQ(matrices.size(), 2U);
  EXPECT_EQ(matrices[0].numRows(), 2);
  EXPECT_EQ(matrices[0].numCols(), 2);
  EXPECT_DOUBLE_EQ(matrices[0](0, 0), 1.0);
  EXPECT_DOUBLE_EQ(matrices[0](1, 1), 1.0);
  EXPECT_DOUBLE_EQ(matrices[1](0, 1), 1.0);
  EXPECT_DOUBLE_EQ(matrices[1](1, 0), 1.0);
}

TEST(instruction_materializer_tests, discrete_set_values_builds_int_set_array)
{
  const auto op = make_op(irgen::OpKind::DiscreteSetValues, "set_values");
  const auto store = invoke_handler(
    op,
    make_contract(irgen::IrValueType::IntSetArray),
    json{{"sets", {{"count", 2}, {"elements", json::array({1, 2, 3, 4})},
                     {"elements_per_variable", json::array({2, 2})}}}},
    "sets");
  const auto& sets = store.get<IntSetArray>("set_values");

  ASSERT_EQ(sets.size(), 2U);
  EXPECT_EQ(sets[0], IntSet({1, 2}));
  EXPECT_EQ(sets[1], IntSet({3, 4}));
}

TEST(instruction_materializer_tests, histogram_bin_uncertain_normalizes_count_density)
{
  const auto op = make_op(irgen::OpKind::HistogramBinUncertain, "histograms");
  const auto store = invoke_handler(
    op,
    make_contract(irgen::IrValueType::RealRealMapArray),
    json{{"hist", {{"count", 1},
                    {"abscissas", json::array({0.0, 1.0, 2.0})},
                    {"density", {{"counts", json::array({2.0, 2.0, 0.0})}}}}}},
    "hist");
  const auto& histograms = store.get<RealRealMapArray>("histograms");

  ASSERT_EQ(histograms.size(), 1U);
  const auto& histogram = histograms[0];
  EXPECT_DOUBLE_EQ(histogram.at(0.0), 0.5);
  EXPECT_DOUBLE_EQ(histogram.at(1.0), 0.5);
  EXPECT_DOUBLE_EQ(histogram.at(2.0), 0.0);
}

TEST(instruction_materializer_tests, histogram_point_uncertain_reports_not_implemented)
{
  const auto op = make_op(irgen::OpKind::HistogramPointUncertain, "histograms");

  EXPECT_THROW(
    invoke_handler(
      op,
      make_contract(irgen::IrValueType::RealRealMapArray),
      json{{"hist", json::object()}},
      "hist"),
    std::runtime_error);
}

TEST(instruction_materializer_tests, discrete_uncertain_set_values_probs_builds_probability_maps)
{
  const auto op = make_op(irgen::OpKind::DiscreteUncertainSetValuesProbs, "pmf_values");
  const auto store = invoke_handler(
    op,
    make_contract(irgen::IrValueType::IntRealMapArray),
    json{{"uncertain", {{"count", 2},
                         {"elements", json::array({1, 2, 3, 4})},
                         {"elements_per_variable", json::array({2, 2})},
                         {"set_probabilities", json::array({0.2, 0.8, 0.6, 0.4})}}}},
    "uncertain");
  const auto& pmfs = store.get<IntRealMapArray>("pmf_values");

  ASSERT_EQ(pmfs.size(), 2U);
  EXPECT_DOUBLE_EQ(pmfs[0].at(1), 0.2);
  EXPECT_DOUBLE_EQ(pmfs[0].at(2), 0.8);
  EXPECT_DOUBLE_EQ(pmfs[1].at(3), 0.6);
  EXPECT_DOUBLE_EQ(pmfs[1].at(4), 0.4);
}

TEST(instruction_materializer_tests, continuous_interval_uncertain_normalizes_probabilities)
{
  const auto op = make_op(irgen::OpKind::ContinuousIntervalUncertain, "intervals");
  const auto store = invoke_handler(
    op,
    make_contract(irgen::IrValueType::RealRealPairRealMapArray),
    json{{"uncertain", {{"count", 1},
                         {"lower_bounds", json::array({0.0, 1.0})},
                         {"upper_bounds", json::array({1.0, 2.0})},
                         {"interval_probabilities", json::array({2.0, 1.0})}}}},
    "uncertain");
  const auto& intervals = store.get<RealRealPairRealMapArray>("intervals");

  ASSERT_EQ(intervals.size(), 1U);
  const auto& interval_map = intervals[0];
  EXPECT_NEAR(interval_map.at(RealRealPair(0.0, 1.0)), 2.0 / 3.0, 1.e-12);
  EXPECT_NEAR(interval_map.at(RealRealPair(1.0, 2.0)), 1.0 / 3.0, 1.e-12);
}

TEST(instruction_materializer_tests, discrete_interval_uncertain_builds_interval_probability_maps)
{
  const auto op = make_op(irgen::OpKind::DiscreteIntervalUncertain, "intervals");
  const auto store = invoke_handler(
    op,
    make_contract(irgen::IrValueType::IntIntPairRealMapArray),
    json{{"uncertain", {{"count", 2},
                         {"lower_bounds", json::array({1, 3, 10, 20})},
                         {"upper_bounds", json::array({2, 4, 19, 29})},
                         {"num_intervals", json::array({2, 2})}}}},
    "uncertain");
  const auto& intervals = store.get<IntIntPairRealMapArray>("intervals");

  ASSERT_EQ(intervals.size(), 2U);
  EXPECT_DOUBLE_EQ(intervals[0].at(IntIntPair(1, 2)), 0.5);
  EXPECT_DOUBLE_EQ(intervals[0].at(IntIntPair(3, 4)), 0.5);
  EXPECT_DOUBLE_EQ(intervals[1].at(IntIntPair(10, 19)), 0.5);
  EXPECT_DOUBLE_EQ(intervals[1].at(IntIntPair(20, 29)), 0.5);
}

TEST(instruction_materializer_tests, uncertain_correlation_matrix_reshapes_square_input)
{
  const auto op = make_op(irgen::OpKind::UncertainCorrelationMatrix, "correlations");
  const auto store = invoke_handler(
    op,
    make_contract(irgen::IrValueType::RealSymMatrix),
    json{{"corr", json::array({1.0, 0.25, 0.25, 1.0})}},
    "corr");
  const auto& matrix = store.get<RealSymMatrix>("correlations");

  EXPECT_EQ(matrix.numRows(), 2);
  EXPECT_EQ(matrix.numCols(), 2);
  EXPECT_DOUBLE_EQ(matrix(0, 0), 1.0);
  EXPECT_DOUBLE_EQ(matrix(0, 1), 0.25);
  EXPECT_DOUBLE_EQ(matrix(1, 1), 1.0);
}

TEST(instruction_materializer_tests, id_to_index_set_converts_one_based_ids_to_zero_based_indices)
{
  const auto op = make_op(irgen::OpKind::IdToIndexSet, "indices");
  const auto store = invoke_handler(
    op,
    make_contract(irgen::IrValueType::SizetSet),
    json{{"ids", json::array({1, 3, 4})}},
    "ids");

  EXPECT_EQ(store.get<SizetSet>("indices"), SizetSet({0, 2, 3}));
}

TEST(instruction_materializer_tests, int_set_collects_unique_integer_values)
{
  const auto op = make_op(irgen::OpKind::IntSet, "int_values");
  const auto store = invoke_handler(
    op,
    make_contract(irgen::IrValueType::IntSet),
    json{{"values", json::array({3, 1, 3})}},
    "values");

  EXPECT_EQ(store.get<IntSet>("int_values"), IntSet({1, 3}));
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

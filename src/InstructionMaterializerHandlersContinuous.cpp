/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "InstructionMaterializer.hpp"
#include "InstructionMaterializerUtils.hpp"

#include <nlohmann/json.hpp>

#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace Dakota {

void InstructionMaterializer::handle_continuous_interval_uncertain(const irgen::WriteOp& op,
                                                                   const irgen::KeyContract& contract,
                                                                   const HandlerContext& ctx)
{
  (void)op;
  (void)contract;
  const auto& value = InstructionMaterializerUtils::required_path(
    ctx.block_json, ctx.current_path);
  if (!value.is_object()) {
    throw std::runtime_error(
      "handle_continuous_interval_uncertain: expected object at '" +
      std::string(ctx.current_path) + "'");
  }

  const int count = value.at("count").get<int>();
  const auto lbs = value.at("lower_bounds").get<std::vector<Real>>();
  const auto ubs = value.at("upper_bounds").get<std::vector<Real>>();

  const bool has_probs =
    value.contains("interval_probabilities") && value["interval_probabilities"].is_array();
  std::vector<Real> probs;
  if (has_probs)
    probs = value["interval_probabilities"].get<std::vector<Real>>();

  std::vector<int> num_intervals;
  if (value.contains("num_intervals") && value["num_intervals"].is_array()) {
    num_intervals = value["num_intervals"].get<std::vector<int>>();
  }
  else {
    if (lbs.size() % static_cast<size_t>(count) != 0) {
      throw std::runtime_error(
        "handle_continuous_interval_uncertain: cannot infer num_intervals at '" +
        std::string(ctx.current_path) + "'");
    }
    num_intervals.assign(
      static_cast<size_t>(count),
      static_cast<int>(lbs.size() / static_cast<size_t>(count)));
  }

  size_t expected = 0;
  for (const int n : num_intervals)
    expected += static_cast<size_t>(n);
  if (expected != lbs.size() || expected != ubs.size()) {
    throw std::runtime_error(
      "handle_continuous_interval_uncertain: bounds/apportionment mismatch at '" +
      std::string(ctx.current_path) + "'");
  }

  if (has_probs && probs.size() != expected) {
    throw std::runtime_error(
      "handle_continuous_interval_uncertain: interval_probabilities length mismatch at '" +
      std::string(ctx.current_path) + "'");
  }

  RealRealPairRealMapArray out;
  out.resize(num_intervals.size());
  size_t k = 0;
  for (size_t i = 0; i < num_intervals.size(); ++i) {
    const int n = num_intervals[i];
    RealRealPairRealMap& m = out[i];
    const Real default_p = 1.0 / static_cast<Real>(n);

    if (has_probs) {
      Real total = 0.0;
      for (int j = 0; j < n; ++j)
        total += probs[k + static_cast<size_t>(j)];
      if (std::fabs(total - 1.0) > 1.e-10 && total > 0.0) {
        for (int j = 0; j < n; ++j)
          probs[k + static_cast<size_t>(j)] /= total;
      }
    }

    for (int j = 0; j < n; ++j, ++k) {
      const Real lb = lbs[k];
      const Real ub = ubs[k];
      const Real p = has_probs ? probs[k] : default_p;
      const RealRealPair interval(lb, ub);
      m.insert(std::make_pair(interval, p));
    }
  }

  ctx.store.set_value(op.target_local_ir_key, IRValue(std::move(out)));
}

void InstructionMaterializer::handle_uncertain_correlation_matrix(
  const irgen::WriteOp& op,
  const irgen::KeyContract& contract,
  const HandlerContext& ctx)
{
  (void)op;
  (void)contract;
  const auto& value = InstructionMaterializerUtils::required_path(
    ctx.block_json, ctx.current_path);
  if (!value.is_array()) {
    throw std::runtime_error(
      "handle_uncertain_correlation_matrix: expected array at '" +
      std::string(ctx.current_path) + "'");
  }

  std::vector<Real> flat;
  flat.reserve(value.size());
  for (const auto& v : value) {
    if (!v.is_number()) {
      throw std::runtime_error(
        "handle_uncertain_correlation_matrix: expected numeric entries at '" +
        std::string(ctx.current_path) + "'");
    }
    flat.push_back(v.get<Real>());
  }

  const size_t n = flat.size();
  const size_t dim = static_cast<size_t>(std::round(std::sqrt(static_cast<double>(n))));
  if (dim * dim != n) {
    throw std::runtime_error(
      "handle_uncertain_correlation_matrix: flattened matrix size is not square at '" +
      std::string(ctx.current_path) + "'");
  }

  RealSymMatrix rsm;
  rsm.reshape(dim);
  size_t k = 0;
  for (size_t i = 0; i < dim; ++i)
    for (size_t j = 0; j < dim; ++j)
      rsm(i, j) = flat[k++];

  ctx.store.set_value(op.target_local_ir_key, IRValue(std::move(rsm)));
}

} // namespace Dakota


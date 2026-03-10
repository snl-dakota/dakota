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

#include <stdexcept>
#include <string>
#include <vector>

namespace Dakota {

void InstructionMaterializer::handle_histogram_bin_uncertain(const irgen::WriteOp& op,
                                                             const irgen::KeyContract& contract,
                                                             const HandlerContext& ctx)
{
  (void)op;
  (void)contract;
  const auto& value = InstructionMaterializerUtils::required_path(
    ctx.block_json, ctx.current_path);
  if (!value.is_object()) {
    throw std::runtime_error(
      "handle_histogram_bin_uncertain: expected object at '" +
      std::string(ctx.current_path) + "'");
  }

  const auto abscissas = value.at("abscissas").get<std::vector<Real>>();
  const size_t num_a = abscissas.size();

  const bool has_ppv = value.contains("pairs_per_variable") &&
                       !value["pairs_per_variable"].is_null();
  std::vector<int> ppv;
  if (has_ppv)
    ppv = value["pairs_per_variable"].get<std::vector<int>>();

  const size_t count = value.contains("count") ? value["count"].get<size_t>() : 0;

  if (!value.contains("density") || !value["density"].is_object()) {
    throw std::runtime_error(
      "handle_histogram_bin_uncertain: missing density object at '" +
      std::string(ctx.current_path) + "'");
  }
  const auto& density = value["density"];
  const bool has_ordinates = density.contains("ordinates");
  const bool has_counts = density.contains("counts");
  if (has_ordinates == has_counts) {
    throw std::runtime_error(
      "handle_histogram_bin_uncertain: expected exactly one of counts/ordinates at '" +
      std::string(ctx.current_path) + "'");
  }

  std::vector<Real> yvals = has_counts
    ? density["counts"].get<std::vector<Real>>()
    : density["ordinates"].get<std::vector<Real>>();
  if (yvals.size() != num_a) {
    throw std::runtime_error(
      "handle_histogram_bin_uncertain: y-values length mismatch at '" +
      std::string(ctx.current_path) + "'");
  }

  size_t m = 0;
  std::vector<int> pairs_per_var;
  if (has_ppv) {
    m = ppv.size();
    pairs_per_var = ppv;
    size_t total_pairs = 0;
    for (int n : pairs_per_var)
      total_pairs += static_cast<size_t>(n);
    if (total_pairs != num_a) {
      throw std::runtime_error(
        "handle_histogram_bin_uncertain: pairs_per_variable/apportionment mismatch at '" +
        std::string(ctx.current_path) + "'");
    }
  }
  else {
    m = count;
    if (m == 0 || num_a % m != 0) {
      throw std::runtime_error(
        "handle_histogram_bin_uncertain: cannot infer pairs_per_variable at '" +
        std::string(ctx.current_path) + "'");
    }
    pairs_per_var.assign(m, static_cast<int>(num_a / m));
  }

  RealRealMapArray out;
  out.resize(m);
  size_t idx = 0;
  for (size_t i = 0; i < m; ++i) {
    const int n = pairs_per_var[i];
    RealRealMap& map_i = out[i];
    Real norm = 0.0;
    for (int j = 0; j < n; ++j, ++idx) {
      Real x = abscissas[idx];
      Real y = yvals[idx];
      if (j < n - 1) {
        const Real w = abscissas[idx + 1] - x;
        if (has_counts) {
          norm += y;
          y /= w;
        }
        else {
          norm += y * w;
        }
      }
      map_i[x] = y;
    }
    if (map_i.size() >= 2 && norm > 0.0) {
      auto it_end = map_i.end();
      --it_end;
      for (auto it = map_i.begin(); it != it_end; ++it)
        it->second /= norm;
    }
  }

  ctx.store.set_value(op.target_local_ir_key, IRValue(std::move(out)));
}

void InstructionMaterializer::handle_histogram_point_uncertain(const irgen::WriteOp& op,
                                                               const irgen::KeyContract& contract,
                                                               const HandlerContext& ctx)
{
  (void)op;
  (void)contract;
  (void)ctx;
  throw std::runtime_error("handle_histogram_point_uncertain not implemented yet.");
}

} // namespace Dakota


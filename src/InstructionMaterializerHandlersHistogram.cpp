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

namespace {

template <typename MapArray, typename GetX>
MapArray build_histogram_point_maps(const nlohmann::json& value,
                                    const std::vector<Real>& counts,
                                    const std::vector<int>& pairs_per_var,
                                    GetX&& get_x)
{
  MapArray out;
  out.resize(pairs_per_var.size());

  size_t idx = 0;
  for (size_t i = 0; i < pairs_per_var.size(); ++i) {
    auto& map_i = out[i];
    Real sum = 0.0;
    const int n = pairs_per_var[i];
    for (int j = 0; j < n; ++j, ++idx) {
      const auto x = get_x(idx);
      const Real y = counts[idx];
      map_i[x] = y;
      sum += y;
    }
    if (sum > 0.0) {
      for (auto& kv : map_i)
        kv.second /= sum;
    }
  }

  return out;
}

} // namespace

void InstructionMaterializer::handle_histogram_bin_uncertain(const irgen::WriteOp& op,
                                                             const irgen::KeyContract& contract,
                                                             const HandlerContext& ctx)
{
  (void)op;
  (void)contract;
  const auto& value = InstructionMaterializerUtils::required_path(
    ctx.block_json, ctx.current_path);

  const auto abscissas = value.at("abscissas").get<std::vector<Real>>();
  const size_t num_a = abscissas.size();

  const bool has_ppv = value.contains("pairs_per_variable") &&
                       !value["pairs_per_variable"].is_null();
  std::vector<int> ppv;
  if (has_ppv)
    ppv = value["pairs_per_variable"].get<std::vector<int>>();

  const size_t count = value.at("count").get<size_t>();

  const auto& density = value.at("density");
  const bool has_ordinates = density.contains("ordinates");
  const bool has_counts = density.contains("counts");

  std::vector<Real> yvals = has_counts
    ? density["counts"].get<std::vector<Real>>()
    : density["ordinates"].get<std::vector<Real>>();

  size_t m = 0;
  std::vector<int> pairs_per_var;
  if (has_ppv) {
    m = ppv.size();
    pairs_per_var = ppv;
  }
  else {
    m = count;
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
  const auto& value = InstructionMaterializerUtils::required_path(
    ctx.block_json, ctx.current_path);

  switch (contract.ir_value_type) {
  case irgen::IrValueType::IntRealMapArray: {
    const size_t count = value.at("count").get<size_t>();
    const std::vector<Real> counts = value.at("counts").get<std::vector<Real>>();
    const size_t num_a = value["abscissas"].size();
    std::vector<int> pairs_per_var;
    if (value.contains("pairs_per_variable") && !value["pairs_per_variable"].is_null()) {
      pairs_per_var = value["pairs_per_variable"].get<std::vector<int>>();
    }
    else {
      pairs_per_var.assign(count, static_cast<int>(num_a / count));
    }

    auto out = build_histogram_point_maps<IntRealMapArray>(
      value, counts, pairs_per_var,
      [&](size_t idx) { return value["abscissas"][idx].get<int>(); });
    ctx.store.set_value(op.target_local_ir_key, IRValue(std::move(out)));
    return;
  }
  case irgen::IrValueType::StringRealMapArray: {
    const size_t count = value.at("count").get<size_t>();
    const std::vector<Real> counts = value.at("counts").get<std::vector<Real>>();
    const size_t num_a = value["abscissas"].size();
    std::vector<int> pairs_per_var;
    if (value.contains("pairs_per_variable") && !value["pairs_per_variable"].is_null()) {
      pairs_per_var = value["pairs_per_variable"].get<std::vector<int>>();
    }
    else {
      pairs_per_var.assign(count, static_cast<int>(num_a / count));
    }

    auto out = build_histogram_point_maps<StringRealMapArray>(
      value, counts, pairs_per_var,
      [&](size_t idx) { return value["abscissas"][idx].get<String>(); });
    ctx.store.set_value(op.target_local_ir_key, IRValue(std::move(out)));
    return;
  }
  case irgen::IrValueType::RealRealMapArray: {
    const size_t count = value.at("count").get<size_t>();
    const std::vector<Real> counts = value.at("counts").get<std::vector<Real>>();
    const size_t num_a = value["abscissas"].size();
    std::vector<int> pairs_per_var;
    if (value.contains("pairs_per_variable") && !value["pairs_per_variable"].is_null()) {
      pairs_per_var = value["pairs_per_variable"].get<std::vector<int>>();
    }
    else {
      pairs_per_var.assign(count, static_cast<int>(num_a / count));
    }

    auto out = build_histogram_point_maps<RealRealMapArray>(
      value, counts, pairs_per_var,
      [&](size_t idx) { return value["abscissas"][idx].get<Real>(); });
    ctx.store.set_value(op.target_local_ir_key, IRValue(std::move(out)));
    return;
  }
  default:
    throw std::runtime_error(
      "handle_histogram_point_uncertain: unsupported contract type");
  }
}

} // namespace Dakota

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

#if 0
namespace Dakota {

void InstructionMaterializer::handle_adjacency_matrix(const irgen::WriteOp& op,
                                                      const irgen::KeyContract& contract,
                                                      const HandlerContext& ctx)
{
  (void)contract;
  const auto& value = InstructionMaterializerUtils::required_path(
    ctx.block_json, ctx.current_path);
  if (!value.is_array())
    throw std::runtime_error(
      "handle_adjacency_matrix: expected array at '" + std::string(ctx.current_path) + "'");

  std::string rel_path(ctx.current_path);
  auto pos = rel_path.rfind('/');
  if (pos == std::string::npos)
    throw std::runtime_error(
      "handle_adjacency_matrix: invalid path '" + std::string(ctx.current_path) + "'");
  rel_path = rel_path.substr(0, pos); // strip adjacency_matrix
  pos = rel_path.rfind('/');
  if (pos != std::string::npos && rel_path.substr(pos + 1) == "categorical")
    rel_path = rel_path.substr(0, pos);

  std::vector<int> elems_per_var;
  if (const auto* epv = InstructionMaterializerUtils::optional_path(
        ctx.block_json, rel_path + "/elements_per_variable")) {
    elems_per_var = epv->get<std::vector<int>>();
  }
  else {
    const auto& count_j = InstructionMaterializerUtils::required_path(
      ctx.block_json, rel_path + "/count");
    const int num_vars = count_j.get<int>();
    const size_t total = value.size();
    const size_t per_var = total / static_cast<size_t>(num_vars);
    const int n = static_cast<int>(std::round(std::sqrt(static_cast<double>(per_var))));
    elems_per_var.assign(static_cast<size_t>(num_vars), n);
  }

  const auto flat = value.get<std::vector<int>>();
  size_t offset = 0;
  RealMatrixArray rma(elems_per_var.size());
  for (size_t v = 0; v < elems_per_var.size(); ++v) {
    const int n = elems_per_var[v];
    const size_t n_sq = static_cast<size_t>(n) * static_cast<size_t>(n);
    if (offset + n_sq > flat.size()) {
      throw std::runtime_error(
        "handle_adjacency_matrix: reshape overflow at '" + std::string(ctx.current_path) +
        "' for target '" + op.target_local_ir_key + "'");
    }
    RealMatrix& mat = rma[v];
    mat.shapeUninitialized(n, n);
    for (int r = 0; r < n; ++r)
      for (int c = 0; c < n; ++c)
        mat(r, c) = static_cast<Real>(flat[offset + static_cast<size_t>(r) * n + c]);
    offset += n_sq;
  }

  ctx.store.set_value(op.target_local_ir_key, IRValue(std::move(rma)));
}

void InstructionMaterializer::handle_discrete_set_values(const irgen::WriteOp& op,
                                                         const irgen::KeyContract& contract,
                                                         const HandlerContext& ctx)
{
  const auto& value = InstructionMaterializerUtils::required_path(
    ctx.block_json, ctx.current_path);

  const nlohmann::json* set_obj = nullptr;
  std::string rel_base(ctx.current_path);
  if (value.is_object()) {
    set_obj = &value;
  }
  else if (value.is_array()) {
    const auto pos = rel_base.rfind('/');
    if (pos == std::string::npos) {
      throw std::runtime_error(
        "handle_discrete_set_values: invalid legacy elements path '" +
        std::string(ctx.current_path) + "'");
    }
    rel_base = rel_base.substr(0, pos);
    set_obj = &InstructionMaterializerUtils::required_path(ctx.block_json, rel_base);
    if (!set_obj->is_object()) {
      throw std::runtime_error(
        "handle_discrete_set_values: expected object at '" + rel_base + "'");
    }
  }
  else {
    throw std::runtime_error(
      "handle_discrete_set_values: expected object or array at '" +
      std::string(ctx.current_path) + "'");
  }

  const int num_vars = set_obj->at("count").get<int>();
  const nlohmann::json* elements_ptr = nullptr;
  if (set_obj->contains("elements") && (*set_obj)["elements"].is_array())
    elements_ptr = &(*set_obj)["elements"];
  else if (value.is_array())
    elements_ptr = &value;
  if (!elements_ptr) {
    throw std::runtime_error(
      "handle_discrete_set_values: missing elements array for '" +
      std::string(ctx.current_path) + "'");
  }
  const auto& elements = *elements_ptr;

  std::vector<int> elems_per_var;
  if (set_obj->contains("elements_per_variable") &&
      (*set_obj)["elements_per_variable"].is_array()) {
    elems_per_var = (*set_obj)["elements_per_variable"].get<std::vector<int>>();
  }
  else {
    if (elements.size() % static_cast<size_t>(num_vars) != 0) {
      throw std::runtime_error(
        "handle_discrete_set_values: cannot infer per-variable apportionment for '" +
        std::string(ctx.current_path) + "'");
    }
    elems_per_var.assign(
      static_cast<size_t>(num_vars),
      static_cast<int>(elements.size() / static_cast<size_t>(num_vars)));
  }

  size_t expected_size = 0;
  for (const int n : elems_per_var)
    expected_size += static_cast<size_t>(n);
  if (expected_size != elements.size()) {
    throw std::runtime_error(
      "handle_discrete_set_values: elements_per_variable inconsistent with elements at '" +
      std::string(ctx.current_path) + "'");
  }

  size_t offset = 0;
  switch (contract.ir_value_type) {
  case irgen::IrValueType::IntSetArray: {
    IntSetArray sets(static_cast<size_t>(num_vars));
    for (int i = 0; i < num_vars; ++i) {
      IntSet& set_i = sets[static_cast<size_t>(i)];
      for (int j = 0; j < elems_per_var[static_cast<size_t>(i)]; ++j, ++offset)
        set_i.insert(elements[offset].get<int>());
    }
    ctx.store.set_value(op.target_local_ir_key, IRValue(std::move(sets)));
    return;
  }
  case irgen::IrValueType::StringSetArray: {
    StringSetArray sets(static_cast<size_t>(num_vars));
    for (int i = 0; i < num_vars; ++i) {
      StringSet& set_i = sets[static_cast<size_t>(i)];
      for (int j = 0; j < elems_per_var[static_cast<size_t>(i)]; ++j, ++offset)
        set_i.insert(elements[offset].get<String>());
    }
    ctx.store.set_value(op.target_local_ir_key, IRValue(std::move(sets)));
    return;
  }
  case irgen::IrValueType::RealSetArray: {
    RealSetArray sets(static_cast<size_t>(num_vars));
    for (int i = 0; i < num_vars; ++i) {
      RealSet& set_i = sets[static_cast<size_t>(i)];
      for (int j = 0; j < elems_per_var[static_cast<size_t>(i)]; ++j, ++offset) {
        const auto& e = elements[offset];
        const Real v = e.is_number()
          ? e.get<Real>()
          : static_cast<Real>(std::stod(e.get<std::string>()));
        set_i.insert(v);
      }
    }
    ctx.store.set_value(op.target_local_ir_key, IRValue(std::move(sets)));
    return;
  }
  default:
    throw std::runtime_error(
      "handle_discrete_set_values: unsupported ir_value_type for target '" +
      op.target_local_ir_key + "'");
  }
}

void InstructionMaterializer::handle_histogram_bin_uncertain(const irgen::WriteOp& op,
                                                             const irgen::KeyContract& contract,
                                                             const HandlerContext& ctx)
{
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

  bool has_ppv = value.contains("pairs_per_variable") &&
                 !value["pairs_per_variable"].is_null();
  std::vector<int> ppv;
  if (has_ppv)
    ppv = value["pairs_per_variable"].get<std::vector<int>>();

  size_t count = value.contains("count") ? value["count"].get<size_t>() : 0;

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
      --it_end; // terminal y-value
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

void InstructionMaterializer::handle_discrete_uncertain_set_values_probs(
  const irgen::WriteOp& op,
  const irgen::KeyContract& contract,
  const HandlerContext& ctx)
{
  const auto& value = InstructionMaterializerUtils::required_path(
    ctx.block_json, ctx.current_path);
  if (!value.is_object()) {
    throw std::runtime_error(
      "handle_discrete_uncertain_set_values_probs: expected object at '" +
      std::string(ctx.current_path) + "'");
  }

  const int num_vars = value.at("count").get<int>();
  const auto& elements = value.at("elements");

  std::vector<int> elems_per_var;
  if (value.contains("elements_per_variable") && value["elements_per_variable"].is_array()) {
    elems_per_var = value["elements_per_variable"].get<std::vector<int>>();
  }
  else {
    if (elements.size() % static_cast<size_t>(num_vars) != 0) {
      throw std::runtime_error(
        "handle_discrete_uncertain_set_values_probs: cannot infer per-variable apportionment at '" +
        std::string(ctx.current_path) + "'");
    }
    elems_per_var.assign(
      static_cast<size_t>(num_vars),
      static_cast<int>(elements.size() / static_cast<size_t>(num_vars)));
  }

  size_t total_elems = 0;
  for (const int n : elems_per_var)
    total_elems += static_cast<size_t>(n);
  if (total_elems != elements.size()) {
    throw std::runtime_error(
      "handle_discrete_uncertain_set_values_probs: elements_per_variable inconsistent with elements at '" +
      std::string(ctx.current_path) + "'");
  }

  const bool has_probs =
    value.contains("set_probabilities") && value["set_probabilities"].is_array();
  std::vector<Real> probs;
  if (has_probs)
    probs = value["set_probabilities"].get<std::vector<Real>>();

  size_t offset = 0;
  switch (contract.ir_value_type) {
  case irgen::IrValueType::IntRealMapArray: {
    IntRealMapArray maps(static_cast<size_t>(num_vars));
    for (int i = 0; i < num_vars; ++i) {
      IntRealMap& m = maps[static_cast<size_t>(i)];
      const Real default_p = 1.0 / static_cast<Real>(elems_per_var[static_cast<size_t>(i)]);
      for (int j = 0; j < elems_per_var[static_cast<size_t>(i)]; ++j, ++offset) {
        const int v = elements[offset].get<int>();
        const Real p = has_probs ? probs[offset] : default_p;
        if (m.find(v) == m.end())
          m[v] = p;
      }
    }
    ctx.store.set_value(op.target_local_ir_key, IRValue(std::move(maps)));
    return;
  }
  case irgen::IrValueType::StringRealMapArray: {
    StringRealMapArray maps(static_cast<size_t>(num_vars));
    for (int i = 0; i < num_vars; ++i) {
      StringRealMap& m = maps[static_cast<size_t>(i)];
      const Real default_p = 1.0 / static_cast<Real>(elems_per_var[static_cast<size_t>(i)]);
      for (int j = 0; j < elems_per_var[static_cast<size_t>(i)]; ++j, ++offset) {
        const String v = elements[offset].get<String>();
        const Real p = has_probs ? probs[offset] : default_p;
        if (m.find(v) == m.end())
          m[v] = p;
      }
    }
    ctx.store.set_value(op.target_local_ir_key, IRValue(std::move(maps)));
    return;
  }
  case irgen::IrValueType::RealRealMapArray: {
    RealRealMapArray maps(static_cast<size_t>(num_vars));
    for (int i = 0; i < num_vars; ++i) {
      RealRealMap& m = maps[static_cast<size_t>(i)];
      const Real default_p = 1.0 / static_cast<Real>(elems_per_var[static_cast<size_t>(i)]);
      for (int j = 0; j < elems_per_var[static_cast<size_t>(i)]; ++j, ++offset) {
        const auto& e = elements[offset];
        const Real v = e.is_number()
          ? e.get<Real>()
          : static_cast<Real>(std::stod(e.get<std::string>()));
        const Real p = has_probs ? probs[offset] : default_p;
        if (m.find(v) == m.end())
          m[v] = p;
      }
    }
    ctx.store.set_value(op.target_local_ir_key, IRValue(std::move(maps)));
    return;
  }
  default:
    throw std::runtime_error(
      "handle_discrete_uncertain_set_values_probs: unsupported ir_value_type for target '" +
      op.target_local_ir_key + "'");
  }
}

void InstructionMaterializer::handle_continuous_interval_uncertain(const irgen::WriteOp& op,
                                                                   const irgen::KeyContract& contract,
                                                                   const HandlerContext& ctx)
{
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

void InstructionMaterializer::handle_discrete_interval_uncertain(const irgen::WriteOp& op,
                                                                 const irgen::KeyContract& contract,
                                                                 const HandlerContext& ctx)
{
  (void)contract;
  const auto& value = InstructionMaterializerUtils::required_path(
    ctx.block_json, ctx.current_path);
  if (!value.is_object()) {
    throw std::runtime_error(
      "handle_discrete_interval_uncertain: expected object at '" +
      std::string(ctx.current_path) + "'");
  }

  const int count = value.at("count").get<int>();
  const auto lbs = value.at("lower_bounds").get<std::vector<int>>();
  const auto ubs = value.at("upper_bounds").get<std::vector<int>>();

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
        "handle_discrete_interval_uncertain: cannot infer num_intervals at '" +
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
      "handle_discrete_interval_uncertain: bounds/apportionment mismatch at '" +
      std::string(ctx.current_path) + "'");
  }

  if (has_probs && probs.size() != expected) {
    throw std::runtime_error(
      "handle_discrete_interval_uncertain: interval_probabilities length mismatch at '" +
      std::string(ctx.current_path) + "'");
  }

  IntIntPairRealMapArray out;
  out.resize(num_intervals.size());
  size_t k = 0;
  for (size_t i = 0; i < num_intervals.size(); ++i) {
    const int n = num_intervals[i];
    IntIntPairRealMap& m = out[i];
    const Real default_p = 1.0 / static_cast<Real>(n);
    for (int j = 0; j < n; ++j, ++k) {
      const int lb = lbs[k];
      const int ub = ubs[k];
      const Real p = has_probs ? probs[k] : default_p;
      const IntIntPair interval(lb, ub);
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
  (void)contract;
  const auto& value = required_path(ctx.block_json, ctx.current_path);
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
#endif

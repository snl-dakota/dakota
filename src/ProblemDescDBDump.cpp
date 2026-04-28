/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ProblemDescDBDump.hpp"

#include "ProblemDescDB.hpp"
#include "IRState.hpp"
#include "ProblemDescDBLegacyDumpRegistry.hpp"

#include <cmath>
#include <fstream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

namespace Dakota {

namespace {

using json = nlohmann::json;

template <class A, class B>
json dump_json_value(const std::pair<A, B>& value);

template <class T>
json dump_json_value(const std::vector<T>& value);

template <class T>
json dump_json_value(const std::set<T>& value);

template <class K, class V>
json dump_json_value(const std::map<K, V>& value);

template <class T,
          std::enable_if_t<std::is_arithmetic_v<T> &&
                             !std::is_same_v<T, bool>, int> = 0>
json dump_json_value(const T& value);

template <class T>
json dump_numeric_vector(const T& vec)
{
  json out = json::array();
  for (int i = 0; i < vec.length(); ++i)
    out.push_back(dump_json_value(vec[i]));
  return out;
}

json dump_json_value(const RealVector& value)
{ return dump_numeric_vector(value); }

json dump_json_value(const IntVector& value)
{ return dump_numeric_vector(value); }

json dump_json_value(const RealMatrix& value)
{
  json out = json::array();
  for (int r = 0; r < value.numRows(); ++r) {
    json row = json::array();
    for (int c = 0; c < value.numCols(); ++c)
      row.push_back(dump_json_value(value(r, c)));
    out.push_back(std::move(row));
  }
  return out;
}

json dump_json_value(const RealSymMatrix& value)
{
  json out = json::array();
  for (int r = 0; r < value.numRows(); ++r) {
    json row = json::array();
    for (int c = 0; c < value.numRows(); ++c)
      row.push_back(dump_json_value(value(r, c)));
    out.push_back(std::move(row));
  }
  return out;
}

json dump_json_value(const BitArray& value)
{
  json out = json::array();
  for (size_t i = 0; i < value.size(); ++i)
    out.push_back(value.test(i));
  return out;
}

inline json dump_json_value(const String& value)
{ return value; }

inline json dump_json_value(const char* value)
{ return value; }

inline json dump_json_value(const bool& value)
{ return value; }

template <class T,
          std::enable_if_t<std::is_arithmetic_v<T> &&
                             !std::is_same_v<T, bool>, int>>
json dump_json_value(const T& value)
{
  if constexpr (std::is_floating_point_v<T>) {
    if (std::isnan(value))
      return "NaN";
    if (std::isinf(value))
      return std::signbit(value) ? "-Inf" : "Inf";
  }
  return value;
}

inline json dump_json_value(const json& value)
{ return value; }

inline json dump_json_value(const std::monostate&)
{ return nullptr; }

template <class A, class B>
json dump_json_value(const std::pair<A, B>& value)
{
  return json::array({dump_json_value(value.first), dump_json_value(value.second)});
}

template <class K, class V>
json dump_json_value(const std::map<K, V>& value)
{
  json out = json::array();
  for (const auto& [key, mapped] : value)
    out.push_back(json{{"key", dump_json_value(key)}, {"value", dump_json_value(mapped)}});
  return out;
}


template <class T>
json dump_json_value(const std::vector<T>& value)
{
  json out = json::array();
  for (const auto& elem : value)
    out.push_back(dump_json_value(elem));
  return out;
}

template <class T>
json dump_json_value(const std::set<T>& value)
{
  json out = json::array();
  for (const auto& elem : value)
    out.push_back(dump_json_value(elem));
  return out;
}

json dump_ir_value(const IRValue& value)
{
  return std::visit([](const auto& alt) { return dump_json_value(alt); }, value);
}

std::string indexed_key(const std::string_view full_key, size_t index)
{
  const auto dot = full_key.find('.');
  if (dot == std::string_view::npos)
    throw std::runtime_error("ProblemDescDB dump key missing block prefix");

  std::string out;
  out.reserve(full_key.size() + 8);
  out.append(full_key.substr(0, dot));
  out.push_back('[');
  out.append(std::to_string(index));
  out.push_back(']');
  out.append(full_key.substr(dot));
  return out;
}

template <class Value>
void set_dump_value(json& values, const std::string& output_key, const Value& value)
{
  values[output_key] = dump_json_value(value);
}

void write_json_file(const json& document, const String& output_path)
{
  std::ofstream out(output_path);
  if (!out)
    throw std::runtime_error("Failed to open JSON dump path: " + output_path);
  out << document.dump(2) << '\n';
}

} // namespace

nlohmann::json dump_problem_desc_db_json(const ProblemDescDB& db)
{
  ProblemDescDB* const storage =
    db.dbRep ? db.dbRep.get() : const_cast<ProblemDescDB*>(&db);
  json values = json::object();

  const auto& environment_rep = *storage->environmentSpec.data_rep();
  for (const auto& key : problem_desc_db_dump::k_environment_entries)
    problem_desc_db_dump::try_emit_environment_entry(
      environment_rep, key, [&](const auto& value) {
        set_dump_value(values, std::string(key), value);
      });

  size_t index = 0;
  for (auto& method : storage->dataMethodList) {
    const auto& rep = *method.data_rep();
    for (const auto& key : problem_desc_db_dump::k_method_entries)
      problem_desc_db_dump::try_emit_method_entry(
        rep, key, [&](const auto& value) {
          set_dump_value(values, indexed_key(key, index), value);
        });
    ++index;
  }

  index = 0;
  for (auto& model : storage->dataModelList) {
    const auto& rep = *model.data_rep();
    for (const auto& key : problem_desc_db_dump::k_model_entries)
      problem_desc_db_dump::try_emit_model_entry(
        rep, key, [&](const auto& value) {
          set_dump_value(values, indexed_key(key, index), value);
        });
    ++index;
  }

  index = 0;
  for (auto& variables : storage->dataVariablesList) {
    const auto& rep = *variables.data_rep();
    for (const auto& key : problem_desc_db_dump::k_variables_entries)
      problem_desc_db_dump::try_emit_variables_entry(
        rep, key, [&](const auto& value) {
          set_dump_value(values, indexed_key(key, index), value);
        });
    ++index;
  }

  index = 0;
  for (auto& interface : storage->dataInterfaceList) {
    const auto& rep = *interface.data_rep();
    for (const auto& key : problem_desc_db_dump::k_interface_entries)
      problem_desc_db_dump::try_emit_interface_entry(
        rep, key, [&](const auto& value) {
          set_dump_value(values, indexed_key(key, index), value);
        });
    ++index;
  }

  index = 0;
  for (auto& responses : storage->dataResponsesList) {
    const auto& rep = *responses.data_rep();
    for (const auto& key : problem_desc_db_dump::k_responses_entries)
      problem_desc_db_dump::try_emit_responses_entry(
        rep, key, [&](const auto& value) {
          set_dump_value(values, indexed_key(key, index), value);
        });
    ++index;
  }

  return json{
    {"_meta",
     {
       {"format", "problem_desc_db_dump_v1"},
       {"implementation", "legacy_data_class"},
       {"omitted_keys", json::array({std::string(problem_desc_db_dump::kExcludedLegacyVoidKey)})},
     }},
    {"values", std::move(values)},
  };
}

void write_problem_desc_db_json(const ProblemDescDB& db, const String& output_path)
{
  write_json_file(dump_problem_desc_db_json(db), output_path);
}

nlohmann::json dump_ir_state_json(const IRState& state)
{
  json values = json::object();

  for (const auto& [key, value] : state.environment.values())
    values["environment." + key] = dump_ir_value(value);

  auto emit_block = [&](const char* block_name, const auto& stores) {
    for (size_t i = 0; i < stores.size(); ++i) {
      const std::string prefix = std::string(block_name) + "[" + std::to_string(i) + "].";
      for (const auto& [key, value] : stores[i].values())
        values[prefix + key] = dump_ir_value(value);
    }
  };

  emit_block("method", state.method);
  emit_block("model", state.model);
  emit_block("variables", state.variables);
  emit_block("interface", state.interface);
  emit_block("responses", state.responses);

  return json{
    {"_meta",
     {
       {"format", "problem_desc_db_dump_v1"},
       {"implementation", "ir_state"},
       {"omitted_keys", json::array()},
     }},
    {"values", std::move(values)},
  };
}

void write_ir_state_json(const IRState& state, const String& output_path)
{
  write_json_file(dump_ir_state_json(state), output_path);
}

void ProblemDescDB::write_json_dump(const String& output_path) const
{
  const ProblemDescDB* const storage =
    dbRep ? dbRep.get() : this;

  if (storage->irState) {
    write_ir_state_json(*storage->irState, output_path);
    return;
  }

  write_problem_desc_db_json(*this, output_path);
}

} // namespace Dakota

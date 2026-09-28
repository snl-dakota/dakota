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
  const ProblemDescDB* storage = db.dbRep ? db.dbRep.get() : &db;
  if (!storage->irState)
    throw std::runtime_error("Cannot dump an uninitialized ProblemDescDB");
  return dump_ir_state_json(*storage->irState);
}

void write_problem_desc_db_json(const ProblemDescDB& db,
                                const String& output_path)
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
  write_problem_desc_db_json(*this, output_path);
}

} // namespace Dakota

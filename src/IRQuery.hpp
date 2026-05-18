/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_IR_QUERY_H
#define DAKOTA_IR_QUERY_H

#include "IRState.hpp"

#include <stdexcept>

namespace Dakota::ir_query {

struct ParsedIRKey {
  IRBlockType block;
  String local_key;
};

inline ParsedIRKey parse_full_pdb_key(const String& full_key)
{
  const auto dot = full_key.find('.');
  if (dot == String::npos)
    throw std::runtime_error("ir_query::parse_full_pdb_key: missing block prefix for '" + full_key + "'");

  const String block = full_key.substr(0, dot);
  const String local = full_key.substr(dot + 1);

  if (block == "environment") return {IRBlockType::Environment, local};
  if (block == "method") return {IRBlockType::Method, local};
  if (block == "model") return {IRBlockType::Model, local};
  if (block == "variables") return {IRBlockType::Variables, local};
  if (block == "interface") return {IRBlockType::Interface, local};
  if (block == "responses") return {IRBlockType::Responses, local};

  throw std::runtime_error("ir_query::parse_full_pdb_key: unknown block in key '" + full_key + "'");
}

template <class T>
inline const T& get(const IRState& state, const String& full_pdb_key)
{
  const ParsedIRKey parsed = parse_full_pdb_key(full_pdb_key);
  return state.active_store(parsed.block).template get<T>(parsed.local_key);
}

} // namespace Dakota::ir_query

#endif // DAKOTA_IR_QUERY_H

/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaStudyPython.hpp"

#include "InstructionMaterializer.hpp"

#include <nlohmann/json.hpp>

#include <stdexcept>

namespace Dakota::python {
namespace {

IRStore materialize_block(const nlohmann::json& block_json,
                          irgen::BlockType block_type)
{
  return InstructionMaterializer().materialize_block(block_json, block_type);
}

} // namespace

IRStore materialize_method(const nlohmann::json& method_json)
{ return materialize_block(method_json, irgen::BlockType::Method); }

IRStore materialize_model(const nlohmann::json& model_json)
{ return materialize_block(model_json, irgen::BlockType::Model); }

IRStore materialize_variables(const nlohmann::json& variables_json)
{ return materialize_block(variables_json, irgen::BlockType::Variables); }

IRStore materialize_interface(const nlohmann::json& interface_json)
{ return materialize_block(interface_json, irgen::BlockType::Interface); }

IRStore materialize_responses(const nlohmann::json& responses_json)
{ return materialize_block(responses_json, irgen::BlockType::Responses); }

void require_keyword(const nlohmann::json& block_json, const char* keyword,
                     const char* factory_name)
{
  if (!block_json.is_object() || !block_json.contains(keyword))
    throw std::invalid_argument(
      String(factory_name) + " requires a validated dict with top-level '" +
      keyword + "' keyword.");
}

} // namespace Dakota::python

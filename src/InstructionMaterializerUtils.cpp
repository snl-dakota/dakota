/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "InstructionMaterializerUtils.hpp"

#include <nlohmann/json.hpp>

#include <sstream>
#include <stdexcept>
#include <string>

namespace Dakota::InstructionMaterializerUtils {

const nlohmann::json& required_path(const nlohmann::json& block_json,
                                    std::string_view path)
{
  if (path.empty())
    return block_json;

  const nlohmann::json* node = &block_json;
  std::string token;
  std::istringstream iss{std::string(path)};
  while (std::getline(iss, token, '/')) {
    if (token.empty())
      continue;
    if (!node->is_object() || !node->contains(token)) {
      throw std::runtime_error(
        "InstructionMaterializer: missing JSON path '" + std::string(path) + "'");
    }
    node = &((*node)[token]);
  }
  return *node;
}

const nlohmann::json* optional_path(const nlohmann::json& block_json,
                                    std::string_view path)
{
  if (path.empty())
    return &block_json;

  const nlohmann::json* node = &block_json;
  std::string token;
  std::istringstream iss{std::string(path)};
  while (std::getline(iss, token, '/')) {
    if (token.empty())
      continue;
    if (!node->is_object() || !node->contains(token))
      return nullptr;
    node = &((*node)[token]);
  }
  return node;
}

} // namespace Dakota::InstructionMaterializerUtils

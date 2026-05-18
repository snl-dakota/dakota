/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#pragma once

#include <nlohmann/json_fwd.hpp>

#include <string_view>

namespace Dakota::InstructionMaterializerUtils {

const nlohmann::json& required_path(const nlohmann::json& block_json,
                                    std::string_view path);

const nlohmann::json* optional_path(const nlohmann::json& block_json,
                                    std::string_view path);

} // namespace Dakota::InstructionMaterializerUtils


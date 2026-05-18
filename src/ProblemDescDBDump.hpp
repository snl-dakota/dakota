/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_PROBLEM_DESC_DB_DUMP_H
#define DAKOTA_PROBLEM_DESC_DB_DUMP_H

#include "dakota_data_types.hpp"

#include <nlohmann/json.hpp>

namespace Dakota {

class ProblemDescDB;
struct IRState;

nlohmann::json dump_problem_desc_db_json(const ProblemDescDB& db);
void write_problem_desc_db_json(const ProblemDescDB& db, const String& output_path);

nlohmann::json dump_ir_state_json(const IRState& state);
void write_ir_state_json(const IRState& state, const String& output_path);

} // namespace Dakota

#endif // DAKOTA_PROBLEM_DESC_DB_DUMP_H

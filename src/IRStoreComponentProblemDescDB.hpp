#ifndef DAKOTA_IR_STORE_COMPONENT_PROBLEM_DESC_DB_H
#define DAKOTA_IR_STORE_COMPONENT_PROBLEM_DESC_DB_H

#include "InstructionMaterializer.hpp"
#include "ProblemDescDB.hpp"

#include <memory>

namespace Dakota {

class ParallelLibrary;

namespace ir_component_db {

std::shared_ptr<ProblemDescDB> make_problem_db(
  int world_size,
  int world_rank,
  const IRStore* environment_store = nullptr,
  const IRStore* method_store = nullptr,
  const IRStore* model_store = nullptr,
  const IRStore* variables_store = nullptr,
  const IRStore* interface_store = nullptr,
  const IRStore* responses_store = nullptr);

const ProblemDescDB& temporary_problem_db(
  int world_size,
  int world_rank,
  const IRStore* environment_store = nullptr,
  const IRStore* method_store = nullptr,
  const IRStore* model_store = nullptr,
  const IRStore* variables_store = nullptr,
  const IRStore* interface_store = nullptr,
  const IRStore* responses_store = nullptr);

inline std::shared_ptr<ProblemDescDB> make_problem_db(
  const ParallelLibrary& parallel_lib,
  const IRStore* environment_store = nullptr,
  const IRStore* method_store = nullptr,
  const IRStore* model_store = nullptr,
  const IRStore* variables_store = nullptr,
  const IRStore* interface_store = nullptr,
  const IRStore* responses_store = nullptr);

inline const ProblemDescDB& temporary_problem_db(
  const ParallelLibrary& parallel_lib,
  const IRStore* environment_store = nullptr,
  const IRStore* method_store = nullptr,
  const IRStore* model_store = nullptr,
  const IRStore* variables_store = nullptr,
  const IRStore* interface_store = nullptr,
  const IRStore* responses_store = nullptr);

} // namespace ir_component_db
} // namespace Dakota

#include "ParallelLibrary.hpp"

namespace Dakota {
namespace ir_component_db {

inline std::shared_ptr<ProblemDescDB> make_problem_db(
  const ParallelLibrary& parallel_lib,
  const IRStore* environment_store,
  const IRStore* method_store,
  const IRStore* model_store,
  const IRStore* variables_store,
  const IRStore* interface_store,
  const IRStore* responses_store)
{
  return make_problem_db(parallel_lib.world_size(), parallel_lib.world_rank(),
    environment_store, method_store, model_store, variables_store,
    interface_store, responses_store);
}

inline const ProblemDescDB& temporary_problem_db(
  const ParallelLibrary& parallel_lib,
  const IRStore* environment_store,
  const IRStore* method_store,
  const IRStore* model_store,
  const IRStore* variables_store,
  const IRStore* interface_store,
  const IRStore* responses_store)
{
  return temporary_problem_db(parallel_lib.world_size(), parallel_lib.world_rank(),
    environment_store, method_store, model_store, variables_store,
    interface_store, responses_store);
}

} // namespace ir_component_db
} // namespace Dakota

#endif

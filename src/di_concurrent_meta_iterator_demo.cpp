/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ConcurrentMetaIterator.hpp"
#include "DOTOptimizer.hpp"
#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"
#include "Study.hpp"
#include "SimulationModel.hpp"
#include "model_utils.hpp"

#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{
  using namespace Dakota;

  const json multistart_method_json = {
    {"final_solutions", 0},
    {"id_method", "MS"},
    {"output", {{"normal", true}}},
    {"random_starts", {{"count", 3}, {"seed", 123}}},
    {"starting_points", {
      -0.8, -0.8,
      -0.8,  0.8,
       0.8, -0.8,
       0.8,  0.8,
       0.0,  0.0
    }},
    {"sub_method", {{"method_pointer", "NLP"}}}
  };

  const json dot_method_json = {
    {"constraint_tolerance", 0.0},
    {"convergence_tolerance", -1.7976931348623157e+308},
    {"final_solutions", 0},
    {"id_method", "NLP"},
    {"max_function_evaluations", 9223372036854775807LL},
    {"max_iterations", 9223372036854775807LL},
    {"output", {{"normal", true}}}
  };

  const json variables_json = {
    {"continuous_design", {
      {"count", 2},
      {"descriptors", {"x1", "x2"}},
      {"initial_point", {0.0, 0.0}},
      {"lower_bounds", {-1.0, -1.0}},
      {"upper_bounds", {1.0, 1.0}}
    }}
  };

  const json responses_json = {
    {"descriptors", {"obj_fn_1"}},
    {"gradient_type", {{"analytic_gradients", true}}},
    {"hessian_type", {{"no_hessians", true}}},
    {"response_type", {{"objective_functions", {{"count", 1}}}}}
  };

  const json interface_json = {
    {"analysis_drivers", {
      {"drivers", {"quasi_sine_fcn"}},
      {"interface_type",
        {{"fork", {
          {"parameters_file", "params.in"},
          {"results_file", "results.out"},
          {"file_save", true}
        }}}
      },
      {"deactivate", {{"restart_file", true}}}
    }},
    {"failure_capture", {{"abort", true}}}
  };

  Study study;

  std::cout << "Constructing DI concurrent meta-iterator study components...\n";

  Variables variables = study.variables(variables_json);
  Response response = study.responses(responses_json, variables);
  auto interface = study.interface(interface_json);
  auto model = study.model().simulation(
    json::object(), variables, interface, response);
  Cout << "[di_concurrent_meta_iterator_demo] simulation model bounds lower="
       << ModelUtils::continuous_lower_bounds(*model)
       << " upper=" << ModelUtils::continuous_upper_bounds(*model)
       << std::endl;

  auto sub_optimizer = study.method().dot_bfgs(dot_method_json, model);

  Cout << "[di_concurrent_meta_iterator_demo] optimizer iterated model bounds lower="
       << ModelUtils::continuous_lower_bounds(*sub_optimizer->iterated_model())
       << " upper="
       << ModelUtils::continuous_upper_bounds(*sub_optimizer->iterated_model())
       << std::endl;

  auto multistart = study.method().multi_start(
    multistart_method_json, sub_optimizer);

  std::cout << "Running DI multi_start study over DOTOptimizer...\n";
  study.run(multistart);

  std::cout << "Completed DI concurrent meta-iterator study.\n";
  std::cout << "See the multi_start results summary above for per-start optima.\n";

  return 0;
}

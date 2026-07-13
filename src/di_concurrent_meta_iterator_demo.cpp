#include "ConcurrentMetaIterator.hpp"
#include "DOTOptimizer.hpp"
#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"
#include "ForkApplicInterface.hpp"
#include "InstructionMaterializer.hpp"
#include "ExplicitRuntime.hpp"
#include "SimulationModel.hpp"
#include "model_utils.hpp"

#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{
  using namespace Dakota;

  InstructionMaterializer materializer;

  const json multistart_method_json = {
    {"multi_start", {
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
    }}
  };

  const json dot_method_json = {
    {"dot_bfgs", {
      {"constraint_tolerance", 0.0},
      {"convergence_tolerance", -1.7976931348623157e+308},
      {"final_solutions", 0},
      {"id_method", "NLP"},
      {"max_function_evaluations", 9223372036854775807LL},
      {"max_iterations", 9223372036854775807LL},
      {"output", {{"normal", true}}}
    }}
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

  const IRStore multistart_method_store =
    materializer.materialize_block(multistart_method_json,
                                   irgen::BlockType::Method);
  const IRStore dot_method_store =
    materializer.materialize_block(dot_method_json, irgen::BlockType::Method);
  const IRStore variables_store =
    materializer.materialize_block(variables_json, irgen::BlockType::Variables);
  const IRStore responses_store =
    materializer.materialize_block(responses_json, irgen::BlockType::Responses);
  const IRStore interface_store =
    materializer.materialize_block(interface_json, irgen::BlockType::Interface);
  const IRStore model_store =
    materializer.materialize_block(json::object(), irgen::BlockType::Model);

  DemoRuntime runtime;

  std::cout << "Constructing DI concurrent meta-iterator study components...\n";

  Variables variables(variables_store);
  Response response(responses_store, variables);
  auto interface = std::make_shared<ForkApplicInterface>(
    interface_store, runtime.parallelLibrary, runtime.outputManager);
  auto model = std::make_shared<SimulationModel>(
    model_store, variables, interface, response, runtime.services);
  Cout << "[di_concurrent_meta_iterator_demo] simulation model bounds lower="
       << ModelUtils::continuous_lower_bounds(*model)
       << " upper=" << ModelUtils::continuous_upper_bounds(*model)
       << std::endl;

  auto sub_optimizer = std::make_shared<DOTOptimizer>(
    dot_method_store, model, runtime.services);

  Cout << "[di_concurrent_meta_iterator_demo] optimizer iterated model bounds lower="
       << ModelUtils::continuous_lower_bounds(*sub_optimizer->iterated_model())
       << " upper="
       << ModelUtils::continuous_upper_bounds(*sub_optimizer->iterated_model())
       << std::endl;

  ConcurrentMetaIterator multistart(
    multistart_method_store, sub_optimizer, runtime.services);

  std::cout << "Running DI multi_start study over DOTOptimizer...\n";
  runtime.execute_iterator(multistart);

  std::cout << "Completed DI concurrent meta-iterator study.\n";
  std::cout << "See the multi_start results summary above for per-start optima.\n";

  return 0;
}

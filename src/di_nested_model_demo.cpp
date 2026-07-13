#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"
#include "ForkApplicInterface.hpp"
#include "InstructionMaterializer.hpp"
#include "Study.hpp"
#include "NestedModel.hpp"
#include "NonDLHSSampling.hpp"
#include "SimulationModel.hpp"

#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{
  using namespace Dakota;

  InstructionMaterializer materializer;

  // This follows the same semantic shape as /workspace/nested.json:
  // an outer sampling study over design variables s1/s2, a NestedModel that
  // maps s1/s2 into the inner simulation model's state variables, and an
  // inner sampling study that returns eight statistics which are identity-
  // mapped into the NestedModel response.
  const json outer_method_json = {
    {"sampling", {
      {"final_moments", {{"standard", true}}},
      {"final_solutions", 0},
      {"sample_type", {{"lhs", true}}},
      {"samples", 40},
      {"seed", 1234}
    }}
  };

  const json inner_method_json = {
    {"sampling", {
      {"final_moments", {{"standard", true}}},
      {"final_solutions", 0},
      {"probability_levels", {{"values", {0.25, 0.75, 0.25, 0.75}}}},
      {"sample_type", {{"lhs", true}}},
      {"samples", 40},
      {"seed", 1234}
    }}
  };

  const json outer_variables_json = {
    {"active", {{"all", true}}},
    {"continuous_design", {
      {"count", 2},
      {"descriptors", {"s1", "s2"}},
      {"initial_point", {0.0, 0.0}},
      {"lower_bounds", {-1.0, -1.0}},
      {"upper_bounds", {1.0, 1.0}}
    }}
  };

  const json inner_variables_json = {
    {"active", {{"uncertain", true}}}, // necessary because can't be inferred from ProblemDescDB.
    {"continuous_state", {
      {"count", 2},
      {"descriptors", {"s1", "s2"}},
      {"initial_state", {0.0, 0.0}},
      {"lower_bounds", {"-inf", "-inf"}},
      {"upper_bounds", {"inf", "inf"}}
    }},
    {"normal_uncertain", {
      {"count", 2},
      {"descriptors", {"nuv_1", "nuv_2"}},
      {"means", {0.0, 0.0}},
      {"std_deviations", {1.0, 1.0}},
      {"initial_point", {0.0, 0.0}},
      {"lower_bounds", {"-inf", "-inf"}},
      {"upper_bounds", {"inf", "inf"}},
      {"inferred_lower_bounds", {-3.0, -3.0}},
      {"inferred_upper_bounds", {3.0, 3.0}},
      {"initial_point_user_provided", false}
    }}
  };

  const json simulation_responses_json = {
    {"response_type", {{"response_functions", {{"count", 2}}}}},
    {"descriptors", {"response_fn_1", "response_fn_2"}},
    {"gradient_type", {{"no_gradients", true}}},
    {"hessian_type", {{"no_hessians", true}}}
  };

  const json nested_responses_json = {
    {"response_type", {{"objective_functions", {{"count", 8}}}}},
    {"descriptors", {
      "obj_fn_1", "obj_fn_2", "obj_fn_3", "obj_fn_4",
      "obj_fn_5", "obj_fn_6", "obj_fn_7", "obj_fn_8"
    }},
    {"gradient_type", {{"no_gradients", true}}},
    {"hessian_type", {{"no_hessians", true}}}
  };

  const json interface_json = {
    {"analysis_drivers", {
      {"drivers", {"text_book"}},
      {"interface_type",
        {
          {
            "fork", {
              {"parameters_file", "params.in"},
              {"results_file", "results.out"},
              {"file_save", true}
            }
          }
        }
      },
      {"deactivate", {"restart_file", true}}
    }}
  };

  const json nested_model_json = {
    {"nested", {
      {"sub_method_pointer", {
        {"pointer", "inner_method"},
        {"identity_response_mapping", true},
        {"primary_variable_mapping", {"s1", "s2"}}
      }}
    }}
  };

  const IRStore outer_method_store =
    materializer.materialize_block(outer_method_json, irgen::BlockType::Method);
  const IRStore inner_method_store =
    materializer.materialize_block(inner_method_json, irgen::BlockType::Method);
  const IRStore outer_variables_store =
    materializer.materialize_block(outer_variables_json,
                                   irgen::BlockType::Variables);
  const IRStore inner_variables_store =
    materializer.materialize_block(inner_variables_json,
                                   irgen::BlockType::Variables);
  const IRStore simulation_responses_store =
    materializer.materialize_block(simulation_responses_json,
                                   irgen::BlockType::Responses);
  const IRStore nested_responses_store =
    materializer.materialize_block(nested_responses_json,
                                   irgen::BlockType::Responses);
  const IRStore interface_store =
    materializer.materialize_block(interface_json, irgen::BlockType::Interface);
  const IRStore simulation_model_store =
    materializer.materialize_block(json::object(), irgen::BlockType::Model);
  const IRStore nested_model_store =
    materializer.materialize_block(nested_model_json, irgen::BlockType::Model);

  Study study;

  std::cout << "Constructing DI nested-model study components...\n";

  Variables outer_variables(outer_variables_store);
  Variables inner_variables(inner_variables_store);
  Response simulation_response(simulation_responses_store, inner_variables);
  Response nested_response(nested_responses_store, outer_variables);

  auto interface = study.interface(interface_store);
  auto simulation_model = study.model().simulation(
    simulation_model_store, inner_variables, interface, simulation_response);
  auto inner_sampling = study.method().sampling(
    inner_method_store, simulation_model);
  auto nested_model = study.model().nested(
    nested_model_store, inner_sampling, nullptr, outer_variables,
    nested_response);
  auto outer_sampling = study.method().sampling(
    outer_method_store, nested_model);

  std::cout << "Running outer sampling study over NestedModel...\n";
  study.run(outer_sampling);

  const auto& outer_responses = outer_sampling->all_responses();
  Iterator& outer_iterator = *outer_sampling;
  const Response& outer_stats = outer_iterator.response_results();

  std::cout << "Completed nested DI study.\n";
  std::cout << "Outer samples evaluated: " << outer_responses.size() << '\n';
  std::cout << "Outer statistics reported: "
            << outer_stats.num_functions() << '\n';

  const auto& outer_labels = outer_stats.function_labels();
  for (size_t i = 0; i < outer_stats.num_functions(); ++i)
    std::cout << "  " << outer_labels[i] << ": "
              << outer_stats.function_value(i) << '\n';

  return 0;
}

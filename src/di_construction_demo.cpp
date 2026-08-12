/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"
#include "Study.hpp"
#include "StudyConfig.hpp"
#include "NonDLHSSampling.hpp"
#include "SimulationModel.hpp"

#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{
  using namespace Dakota;

  const json method_json = {
    {"sample_type", {{"lhs", true}}},
    {"samples", 10},
    {"seed", 1234}
  };

  const json variables_json = {
    {"active", {{"all", true}}},
    {"uniform_uncertain", {
      {"count", 2},
      {"descriptors", {"x1", "x2"}},
      {"lower_bounds", {0.0, 0.0}},
      {"upper_bounds", {1.0, 1.0}}
    }}
  };

  const json responses_json = {
    {"response_type", {{"response_functions", {{"count", 1}}}}},
    {"descriptors", {"f"}},
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
      {"deactivate", {"restart_file", true}},

    }}
  };

  const json model_json = json::object();

  StudyConfig config;
  config.output.precision = 12;
  config.output.outputFile = "di_construction_demo.out";
  config.output.errorFile = "di_construction_demo.err";
  config.output.writeRestart = "di_construction_demo.rst";
  config.output.resultsOutput = true;
  config.output.resultsOutputFile = "di_construction_demo_results";

  // Study owns the library-mode services used by DI-constructed components.
  Study study(config);

  std::cout << "Constructing DI study components...\n";
  std::cout << "Configured output precision: "
            << study.output_manager()->write_precision() << '\n';
  Variables variables = study.variables(variables_json);
  Response response = study.responses(responses_json, variables);
  auto interface = study.interface(interface_json);
  auto model = study.model().simulation(
    model_json, variables, interface, response);
  auto sampling = study.method().sampling(method_json, model);

  std::cout << "Running sampling study...\n";
  study.run(sampling);

  const auto& responses = sampling->all_responses();
  std::cout << "Completed DI study.\n";
  std::cout << "Samples evaluated: " << responses.size() << '\n';
  if (!responses.empty()) {
    const auto& first_response = responses.begin()->second;
    if (first_response.num_functions() > 0)
      std::cout << "First response value: "
                << first_response.function_value(0) << '\n';
  }

  return 0;
}

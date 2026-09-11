"""Run the DI sampling example with Pydantic configuration objects."""

from dakota.spec.interface import InterfaceConfig
from dakota.spec.method import SamplingConfig
from dakota.spec.model import SingleConfig
from dakota.spec.responses import ResponsesConfig
from dakota.spec.variables import VariablesConfig
from dakota.study import Study, StudyConfig
from dakota.study.di_construction_demo import INTERFACE, METHOD, RESPONSES, VARIABLES


def main() -> None:
    config = StudyConfig()
    config.output.precision = 12
    config.output.output_file = "di_construction_demo.out"
    config.output.error_file = "di_construction_demo.err"
    study = Study(config)
    variables = study.variables(VariablesConfig.model_validate(VARIABLES))
    response = study.responses(variables, ResponsesConfig.model_validate(RESPONSES))
    interface = study.interface(InterfaceConfig.model_validate(INTERFACE))
    model = study.model.simulation(variables, interface, response, SingleConfig())
    sampling = study.method.sampling(model, SamplingConfig.model_validate(METHOD))
    study.run(sampling)
    print(f"Samples evaluated: {sampling.num_responses()}")
    if sampling.num_responses() > 0:
        print(f"First response value: {sampling.first_response_value()}")


if __name__ == "__main__":
    main()

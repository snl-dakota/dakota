# DI Cleanup TODO

Temporary cleanup items introduced while prototyping direct `IRStore`-based
construction for Dakota components.

1. DI multivariate distribution construction currently depends on
   `Variables` retaining its materialized component `IRStore`, and the
   friend free functions `initialize_multivariate_distribution_from_variables(...)`
   and `initialize_distribution_parameters_from_variables(...)` use that
   retained config directly.
   Reason: this removes the public `variables_store` constructor wart from
   `SimulationModel` and restores broad legacy variable-family support, but
   the implementation still interprets raw retained IR rather than a richer
   variables-owned runtime representation.

2. DI `Model` and `Iterator` construction still carry synthetic
   `ProblemDescDB` bridges.
   Reason: the exercised DI pilot path no longer appears to rely heavily on
   `ProblemDescDB` reads in the `Model` / `SimulationModel` hot path, and the
   iterator-side DI path now sources most `method.*` settings from `method_store`,
   but the base `Model` and `Iterator` abstractions still store a
   `ProblemDescDB&` and expose it via API, so the DI path retains bridge
   objects as structural scaffolding.

3. DI construction still uses bridge-based study/runtime seams for some
   non-component-owned concerns.
   Current examples: `outputLevel`, `ScalingOptions`,
   `environment.output_precision`, and `LDDriverAdapter(probDescDB)`.

4. `Model` still converts typed derivative configuration from `Response`
   back into legacy strings.
   Reason: `Response` now owns typed derivative enums/config, but `Model`
   still stores derivative state in string members (`gradientType`,
   `hessianType`, `methodSource`, `intervalType`, `fdGradStepType`,
   `fdHessStepType`, `quasiHessType`) and much of `DakotaModel.cpp` still
   branches on string comparisons. A later cleanup should convert that
   consuming `Model` code to use enums directly and remove the
   `to_legacy_string()` bridge.

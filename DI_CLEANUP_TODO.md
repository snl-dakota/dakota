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

2. `Model` DI construction still relies on a synthetic `ProblemDescDB`.
   Reason: portions of legacy model initialization still pull config through
   `ProblemDescDB` instead of from component-owned objects or component-local
   `IRStore`s.

3. DI `Model` currently uses conservative defaults for some study-wide
   settings instead of reading them from a complete study context.
   Current examples: `outputLevel`, `hierarchicalTagging`, `ScalingOptions`,
   primary response weights, and primary response sense.

4. `Response` / `SharedResponseData` DI construction currently normalizes
   missing response labels by generating defaults.
   Reason: the component-local DI path bypasses some whole-study
   post-processing assumptions present in the legacy construction path.

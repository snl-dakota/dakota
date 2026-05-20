# DI Cleanup TODO

Temporary cleanup items introduced while prototyping direct `IRStore`-based
construction for Dakota components.

1. DI multivariate distribution construction currently depends on
   `Variables` retaining its materialized component `IRStore`, and the
   friend free functions `initialize_multivariate_distribution(...)` and
   `initialize_distribution_parameters(...)` read that config directly.
   Reason: this removes the public `variables_store` constructor wart from
   `SimulationModel`, but the pilot implementation still interprets
   variable distribution config from retained IR rather than from a richer
   variables-owned runtime representation.

2. `ForkApplicInterface` DI construction temporarily takes an injected
   `Response`.
   Reason: `ApplicationInterface` still caches `SharedResponseData` and some
   response-derived evaluation policy for temporary response allocation and ASV
   logic.

3. `Model` DI construction still relies on a synthetic `ProblemDescDB`.
   Reason: portions of legacy model initialization still pull config through
   `ProblemDescDB` instead of from component-owned objects or component-local
   `IRStore`s.

4. DI `Model` currently uses conservative defaults for some study-wide
   settings instead of reading them from a complete study context.
   Current examples: `outputLevel`, `hierarchicalTagging`, `ScalingOptions`,
   primary response weights, and primary response sense.

5. `Response` / `SharedResponseData` DI construction currently normalizes
   missing response labels by generating defaults.
   Reason: the component-local DI path bypasses some whole-study
   post-processing assumptions present in the legacy construction path.

6. The current DI multivariate distribution helper is pilot-oriented and only
   supports all-uniform-uncertain variable sets.
   Reason: this is sufficient for the current `NonDLHSSampling` pilot test,
   but the helper must be generalized to cover the full Dakota variable
   family supported by legacy `Model` initialization.

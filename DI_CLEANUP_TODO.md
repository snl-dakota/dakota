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

2. `Model` DI construction still carries a synthetic `ProblemDescDB`.
   Reason: the exercised DI pilot path no longer appears to rely on
   `ProblemDescDB` reads in the `Model` / `SimulationModel` hot path, but the
   base `Model` abstraction still stores a `ProblemDescDB&` and exposes it via
   API, so the DI path retains a bridge object as structural scaffolding.

3. DI `Model` currently uses conservative defaults for some study-wide
   settings instead of reading them from a complete study context.
   Current examples: `outputLevel` and `ScalingOptions`.

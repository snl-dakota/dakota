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

2. `Variables(const IRStore&)` now builds directly from materialized IR, but
   its component-local `DEFAULT_VIEW` fallback resolves to `ALL_VIEW` because
   the old method/response-driven view inference is intentionally absent from
   the standalone variables constructor.
   Reason: the DI path now avoids the temporary `ProblemDescDB` shim entirely,
   but component-local variables construction still needs a deliberate story
   for any view behavior that used to depend on broader study context.

3. DI `Model` and `Iterator` no longer synthesize IR-backed `ProblemDescDB`
   objects for the pilot path, but the base abstractions still store a
   `ProblemDescDB&` and expose it via API. DI-built instances currently bind
   that reference to `dummy_db`.
   Reason: this preserves legacy API shape while removing the temporary shim,
   but it is still structural scaffolding that should eventually be redesigned.

4. DI construction now uses optional `ParallelLibrary` / `OutputManager`
   injection with internal defaults when callers omit them, but the long-term
   runtime-service split is still unsettled.
   Current examples: `outputLevel`, `ScalingOptions`, and the broader
   separation of `ProgramOptions` command-line behavior from library-mode
   runtime services.

5. `Model` still converts typed derivative configuration from `Response`
   back into legacy strings.
   Reason: `Response` now owns typed derivative enums/config, but `Model`
   still stores derivative state in string members (`gradientType`,
   `hessianType`, `methodSource`, `intervalType`, `fdGradStepType`,
   `fdHessStepType`, `quasiHessType`) and much of `DakotaModel.cpp` still
   branches on string comparisons. A later cleanup should convert that
   consuming `Model` code to use enums directly and remove the
   `to_legacy_string()` bridge.

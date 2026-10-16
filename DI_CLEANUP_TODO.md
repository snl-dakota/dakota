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

2. ~~`Variables(const IRStore&)` now builds directly from materialized IR, but
   its component-local `DEFAULT_VIEW` fallback resolves to `ALL_VIEW` because
   the old method/response-driven view inference is intentionally absent from
   the standalone variables constructor.~~
   **Resolved (see `IteratorViewRequest.hpp`, `Model::assign_inferred_view`,
   and `Iterator::assign_requested_view`).**
   Each DI iterator class declares the view it wants via a `ViewRequest` passed
   through the constructor chain.  `Iterator::assign_requested_view` resolves it
   against the model before `update_from_model` is called.  Method-to-view
   mapping knowledge now lives in the iterator hierarchy, not in `Variables`.
   The legacy `Variables::method_view` / `Variables::response_view` functions
   remain for the ProblemDescDB path and will be removed with those constructors.
   The following divergence from legacy behaviour is intentional and documented:
   when a **sampling** method is used on a model whose Variables have **no
   uncertain variables**, legacy aborts with an error; the DI path silently keeps
   the current view (`ALL`) so that DACE-on-design-only uses continue to work.
   See also `di_construction.cpp` (`study_factory_constructs_data_fit_surrogates`)
   for the relevant test.

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

6. **TODO(recast-function-train-di):** enable
   `Study::ModelFactory::global_surrogate` for
   `global_function_train` after the RecastModel refactor lands.
   The follow-up must establish ownership of the probability transformation,
   ensure an injected DACE iterator samples the transformed model, initialize
   the transformed distribution and basis, and pass function-train
   configuration through the IR-backed approximation path. Remove the
   constructor guard and replace its rejection regression test with successful
   truth-model and DACE construction/execution tests.

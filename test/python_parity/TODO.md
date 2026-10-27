# Remaining verification work

All currently declared method/model factories now have authored execution pairs.
Native pass status is separate from the inventory in `COVERAGE.md`.

- Run and fix the newest 31 pairs in a compatible native build.
- Enable C3 to run function train, multilevel/multifidelity function train, and
  surrogate-based UQ; the supplied build currently has C3 disabled.
- Add separate cases for alternate dispatch paths: global interval/evidence
  optimization (current pairs use LHS), weighted MLMC, recursive ACV/model graph
  searches, and QUESO/GPMSA/DREAM/MUQ Bayesian backends (current pair uses WASABI).
- Add a hierarchical `surrogate_based_local` pair
  (`HierarchSurrBasedLocalMinimizer`) once `ModelFactory.ensemble_surrogate`
  accepts a `correction` specification; hierarchical SBO requires corrections.
- Surrogate-based local/global DI construction supports only
  `approx_method_name`; add `approx_method_pointer` pairs if that is enabled.
- `ModelFactory.random_field` has no pair: `RandomFieldModel` is
  experimental (no regression input; without a DACE iterator it reads a
  hard-coded `rfbuild.test`), and the materialized `model.rf_data_file` is
  never read on either path. Add a pair once the model is regression-tested.
- `ModelFactory.adapted_basis` takes the pilot PCE refinement controls
  (`method.nond.expansion_refinement_*`, covariance, nesting, growth,
  piecewise basis, derivative usage, cross validation) from specification
  defaults; the ProblemDescDB path reads them from the active method block.
- Run all pairs added for the remaining IRStore constructors (linear
  constraints, scaling, calibration data, surrogate types, surrogate import,
  domain decomposition, field surrogates, plugin/python interfaces, subspace
  models); they were authored without a native build.
- Measure native runtimes before describing any new case as fast-running.
- Preserve the deliberate deferral of explicit RecastModel hierarchy bindings;
  these pairs exercise internal method recasts through the exposed factories.

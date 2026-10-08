# Remaining verification work

All currently declared method/model factories now have authored execution pairs.
Native pass status is separate from the inventory in `COVERAGE.md`.

- Run and fix the newest 31 pairs in a compatible native build.
- Enable C3 to run function train, multilevel/multifidelity function train, and
  surrogate-based UQ; the supplied build currently has C3 disabled.
- Add separate cases for alternate dispatch paths: global interval/evidence
  optimization (current pairs use LHS), weighted MLMC, recursive ACV/model graph
  searches, and QUESO/GPMSA/DREAM/MUQ Bayesian backends (current pair uses WASABI).
- Measure native runtimes before describing any new case as fast-running.
- Preserve the deliberate deferral of explicit RecastModel hierarchy bindings;
  these pairs exercise internal method recasts through the exposed factories.

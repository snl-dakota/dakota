# Python paired execution inventory

78 of 79 factory entry points have paired cases.

This lists authored execution pairs, not verified native passes. Optional
factories are included unless `--native` filters to the installed extension.
`ModelFactory.single` is an alias for `simulation`.

| Factory | Paired cases |
| --- | --- |
| `MethodFactory.adaptive_sampling` | method_adaptive_sampling |
| `MethodFactory.approximate_control_variate` | method_approximate_control_variate |
| `MethodFactory.asynch_pattern_search` | method_asynch_pattern_search |
| `MethodFactory.bayes_calibration` | method_bayes_calibration |
| `MethodFactory.branch_and_bound` | method_branch_and_bound |
| `MethodFactory.centered_parameter_study` | pstudy_s4, interface_plugin |
| `MethodFactory.coliny_beta` | method_coliny_beta |
| `MethodFactory.coliny_cobyla` | method_coliny_cobyla |
| `MethodFactory.coliny_direct` | method_coliny_direct |
| `MethodFactory.coliny_ea` | method_coliny_ea, method_hybrid_sequential |
| `MethodFactory.coliny_pattern_search` | method_coliny_pattern_search, method_hybrid_sequential |
| `MethodFactory.coliny_solis_wets` | method_coliny_solis_wets |
| `MethodFactory.conmin_frcg` | method_conmin_frcg, scaling |
| `MethodFactory.conmin_mfd` | method_conmin_mfd, linear_constraints |
| `MethodFactory.dot_bfgs` | method_dot_bfgs |
| `MethodFactory.dot_frcg` | method_dot_frcg |
| `MethodFactory.dot_mmfd` | textbook_s0 |
| `MethodFactory.dot_slp` | method_dot_slp |
| `MethodFactory.dot_sqp` | method_dot_sqp |
| `MethodFactory.efficient_global` | method_efficient_global |
| `MethodFactory.function_train` | method_function_train |
| `MethodFactory.genie_direct` | method_genie_direct |
| `MethodFactory.genie_opt_darts` | method_genie_opt_darts |
| `MethodFactory.global_evidence` | method_global_evidence |
| `MethodFactory.global_interval_est` | method_global_interval_est |
| `MethodFactory.global_reliability` | method_global_reliability |
| `MethodFactory.gpais` | method_gpais |
| `MethodFactory.hybrid` | method_hybrid_sequential |
| `MethodFactory.import_points` | import_points |
| `MethodFactory.importance_sampling` | method_importance_sampling |
| `MethodFactory.list_parameter_study` | pstudy_s3, interface_python, interface_python_callback, interface_python_decorated_callback |
| `MethodFactory.local_evidence` | method_local_evidence |
| `MethodFactory.local_interval_est` | method_local_interval_est |
| `MethodFactory.local_reliability` | method_local_reliability |
| `MethodFactory.mesh_adaptive_search` | method_mesh_adaptive_search |
| `MethodFactory.moga` | method_moga |
| `MethodFactory.multi_start` | multistart_ie_s0 |
| `MethodFactory.multidim_parameter_study` | pstudy_s5 |
| `MethodFactory.multifidelity_function_train` | method_multifidelity_function_train |
| `MethodFactory.multifidelity_polynomial_chaos` | method_multifidelity_polynomial_chaos |
| `MethodFactory.multifidelity_sampling` | method_multifidelity_sampling |
| `MethodFactory.multifidelity_stoch_collocation` | method_multifidelity_stoch_collocation |
| `MethodFactory.multilevel_blue` | method_multilevel_blue |
| `MethodFactory.multilevel_function_train` | method_multilevel_function_train |
| `MethodFactory.multilevel_multifidelity_sampling` | method_multilevel_multifidelity_sampling |
| `MethodFactory.multilevel_polynomial_chaos` | method_multilevel_polynomial_chaos |
| `MethodFactory.multilevel_sampling` | method_multilevel_sampling, ensemble_inferred_view |
| `MethodFactory.ncsu_direct` | method_ncsu_direct |
| `MethodFactory.nl2sol` | method_nl2sol, calibration_data |
| `MethodFactory.nlpql_sqp` | method_nlpql_sqp |
| `MethodFactory.nonlinear_cg` | method_nonlinear_cg |
| `MethodFactory.nowpac` | method_nowpac |
| `MethodFactory.npsol_sqp` | method_npsol_sqp |
| `MethodFactory.optpp_cg` | method_optpp_cg |
| `MethodFactory.optpp_fd_newton` | textbook_nips_s2 |
| `MethodFactory.optpp_newton` | textbook_nips_s4 |
| `MethodFactory.optpp_pds` | method_optpp_pds, local_surrogate_mixed_vars, dace_pstudy_under_optimizer |
| `MethodFactory.optpp_q_newton` | textbook_nips_s0 |
| `MethodFactory.pof_darts` | method_pof_darts |
| `MethodFactory.polynomial_chaos` | method_polynomial_chaos |
| `MethodFactory.richardson_extrap` | method_richardson_extrap |
| `MethodFactory.rkd_darts` | method_rkd_darts |
| `MethodFactory.sampling` | sampling_small, model_nested, method_snowpac, method_surrogate_based_uq, nested_inferred_view, dace_sampling_aleatory_truth, dace_pstudy_under_optimizer, method_surrogate_based_local, method_surrogate_based_global, model_global_field_python, model_adapted_basis, model_active_subspace |
| `MethodFactory.snowpac` | method_snowpac |
| `MethodFactory.soga` | method_soga |
| `MethodFactory.stoch_collocation` | method_stoch_collocation |
| `MethodFactory.surrogate_based_global` | method_surrogate_based_global |
| `MethodFactory.surrogate_based_local` | method_surrogate_based_local |
| `MethodFactory.surrogate_based_uq` | method_surrogate_based_uq |
| `MethodFactory.vector_parameter_study` | pstudy_s0, model_global, model_local, model_multipoint, model_ensemble, model_nested, multistart_ie_s0, nested_inferred_view, model_global_kriging, model_global_gp_dakota, model_global_neural_network, model_global_radial_basis, model_global_mars, model_global_moving_least_squares, model_global_experimental_gaussian_process, model_global_experimental_polynomial, model_global_import, model_global_domain_decomposition |
| `ModelFactory.active_subspace` | model_active_subspace |
| `ModelFactory.adapted_basis` | model_adapted_basis |
| `ModelFactory.ensemble_surrogate` | model_ensemble, method_multilevel_polynomial_chaos, method_multifidelity_polynomial_chaos, method_multifidelity_stoch_collocation, method_multilevel_function_train, method_multifidelity_function_train, method_multilevel_sampling, method_multifidelity_sampling, method_approximate_control_variate, method_multilevel_blue, method_multilevel_multifidelity_sampling, ensemble_inferred_view |
| `ModelFactory.global_surrogate` | model_global, method_surrogate_based_uq, dace_sampling_aleatory_truth, dace_pstudy_under_optimizer, method_surrogate_based_local, method_surrogate_based_global, model_global_kriging, model_global_gp_dakota, model_global_neural_network, model_global_radial_basis, model_global_mars, model_global_moving_least_squares, model_global_experimental_gaussian_process, model_global_experimental_polynomial, model_global_import, model_global_domain_decomposition, model_global_field_python |
| `ModelFactory.local_surrogate` | model_local, local_surrogate_mixed_vars |
| `ModelFactory.multipoint_surrogate` | model_multipoint |
| `ModelFactory.nested` | model_nested, method_snowpac, nested_inferred_view |
| `ModelFactory.random_field` | TODO |
| `ModelFactory.simulation` | pstudy_s0, pstudy_s3, pstudy_s4, pstudy_s5, textbook_s0, textbook_nips_s0, textbook_nips_s2, textbook_nips_s4, sampling_small, import_points, model_local, model_multipoint, model_ensemble, model_nested, method_dot_bfgs, method_dot_frcg, method_dot_slp, method_dot_sqp, method_conmin_frcg, method_conmin_mfd, method_nonlinear_cg, method_nowpac, method_snowpac, method_genie_direct, method_genie_opt_darts, method_coliny_cobyla, method_coliny_pattern_search, multistart_ie_s0, method_coliny_direct, method_coliny_ea, method_coliny_solis_wets, method_optpp_cg, method_optpp_pds, method_ncsu_direct, method_nlpql_sqp, method_npsol_sqp, method_mesh_adaptive_search, method_asynch_pattern_search, method_soga, method_moga, method_local_reliability, method_global_reliability, method_global_interval_est, method_global_evidence, method_local_interval_est, method_local_evidence, method_importance_sampling, method_gpais, method_pof_darts, method_rkd_darts, method_adaptive_sampling, method_polynomial_chaos, method_stoch_collocation, method_function_train, method_surrogate_based_uq, method_multilevel_polynomial_chaos, method_multifidelity_polynomial_chaos, method_multifidelity_stoch_collocation, method_multilevel_function_train, method_multifidelity_function_train, method_multilevel_sampling, method_multifidelity_sampling, method_approximate_control_variate, method_multilevel_blue, method_multilevel_multifidelity_sampling, method_efficient_global, method_coliny_beta, method_nl2sol, method_branch_and_bound, method_richardson_extrap, method_bayes_calibration, nested_inferred_view, dace_sampling_aleatory_truth, local_surrogate_mixed_vars, ensemble_inferred_view, dace_pstudy_under_optimizer, method_hybrid_sequential, method_surrogate_based_local, method_surrogate_based_global, linear_constraints, scaling, calibration_data, interface_plugin, interface_python, interface_python_callback, interface_python_decorated_callback, model_global_field_python, model_adapted_basis, model_active_subspace |

## Native classes exercised by the paired scripts

`APPSOptimizer`, `ActiveSubspaceModel`, `AdaptedBasisModel`, `COLINOptimizer`, `CONMINOptimizer`, `ConcurrentMetaIterator`, `DOTOptimizer`, `DataFitSurrBasedLocalMinimizer`, `DataFitSurrModel`, `EffGlobalMinimizer`, `EnsembleSurrModel`, `JEGAOptimizer`, `NCSUOptimizer`, `NL2SOLLeastSq`, `NLPQLPOptimizer`, `NOWPACOptimizer`, `NPSOLOptimizer`, `NestedModel`, `NomadOptimizer`, `NonDACVSampling`, `NonDAdaptImpSampling`, `NonDAdaptiveSampling`, `NonDC3FunctionTrain`, `NonDGPImpSampling`, `NonDGlobalReliability`, `NonDImportPoints`, `NonDLHSEvidence`, `NonDLHSSampling`, `NonDLHSSingleInterval`, `NonDLocalEvidence`, `NonDLocalReliability`, `NonDLocalSingleInterval`, `NonDMultifidelitySampling`, `NonDMultilevBLUESampling`, `NonDMultilevControlVarSampling`, `NonDMultilevelFunctionTrain`, `NonDMultilevelPolynomialChaos`, `NonDMultilevelSampling`, `NonDMultilevelStochCollocation`, `NonDPOFDarts`, `NonDPolynomialChaos`, `NonDRKDDarts`, `NonDStochCollocation`, `NonDSurrogateExpansion`, `NonDWASABIBayesCalibration`, `NonlinearCGOptimizer`, `OPTPPOptimizer`, `OptDartsOptimizer`, `ParamStudy`, `PebbldMinimizer`, `PluginInterface`, `Pybind11Interface`, `RichExtrapVerification`, `SNLLOptimizer`, `SeqHybridMetaIterator`, `SimulationModel`, `SurrBasedGlobalMinimizer`

Multiple factory entry points can use the same native class; each factory
still needs its own execution case. Timing claims require successful native runs.

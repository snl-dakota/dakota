# Python paired execution inventory

71 of 71 factory entry points have paired cases.

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
| `MethodFactory.centered_parameter_study` | pstudy_s4 |
| `MethodFactory.coliny_beta` | method_coliny_beta |
| `MethodFactory.coliny_cobyla` | method_coliny_cobyla |
| `MethodFactory.coliny_direct` | method_coliny_direct |
| `MethodFactory.coliny_ea` | method_coliny_ea |
| `MethodFactory.coliny_pattern_search` | method_coliny_pattern_search |
| `MethodFactory.coliny_solis_wets` | method_coliny_solis_wets |
| `MethodFactory.conmin_frcg` | method_conmin_frcg |
| `MethodFactory.conmin_mfd` | method_conmin_mfd |
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
| `MethodFactory.import_points` | import_points |
| `MethodFactory.importance_sampling` | method_importance_sampling |
| `MethodFactory.list_parameter_study` | pstudy_s3 |
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
| `MethodFactory.multilevel_sampling` | method_multilevel_sampling |
| `MethodFactory.ncsu_direct` | method_ncsu_direct |
| `MethodFactory.nl2sol` | method_nl2sol |
| `MethodFactory.nlpql_sqp` | method_nlpql_sqp |
| `MethodFactory.nonlinear_cg` | method_nonlinear_cg |
| `MethodFactory.npsol_sqp` | method_npsol_sqp |
| `MethodFactory.optpp_cg` | method_optpp_cg |
| `MethodFactory.optpp_fd_newton` | textbook_nips_s2 |
| `MethodFactory.optpp_newton` | textbook_nips_s4 |
| `MethodFactory.optpp_pds` | method_optpp_pds |
| `MethodFactory.optpp_q_newton` | textbook_nips_s0 |
| `MethodFactory.pof_darts` | method_pof_darts |
| `MethodFactory.polynomial_chaos` | method_polynomial_chaos |
| `MethodFactory.richardson_extrap` | method_richardson_extrap |
| `MethodFactory.rkd_darts` | method_rkd_darts |
| `MethodFactory.sampling` | sampling_small, model_nested, method_surrogate_based_uq |
| `MethodFactory.soga` | method_soga |
| `MethodFactory.stoch_collocation` | method_stoch_collocation |
| `MethodFactory.surrogate_based_uq` | method_surrogate_based_uq |
| `MethodFactory.vector_parameter_study` | pstudy_s0, model_global, model_local, model_multipoint, model_ensemble, model_nested, multistart_ie_s0 |
| `ModelFactory.ensemble_surrogate` | model_ensemble, method_multilevel_polynomial_chaos, method_multifidelity_polynomial_chaos, method_multifidelity_stoch_collocation, method_multilevel_function_train, method_multifidelity_function_train, method_multilevel_sampling, method_multifidelity_sampling, method_approximate_control_variate, method_multilevel_blue, method_multilevel_multifidelity_sampling |
| `ModelFactory.global_surrogate` | model_global, method_surrogate_based_uq |
| `ModelFactory.local_surrogate` | model_local |
| `ModelFactory.multipoint_surrogate` | model_multipoint |
| `ModelFactory.nested` | model_nested |
| `ModelFactory.simulation` | pstudy_s0, pstudy_s3, pstudy_s4, pstudy_s5, textbook_s0, textbook_nips_s0, textbook_nips_s2, textbook_nips_s4, sampling_small, import_points, model_local, model_multipoint, model_ensemble, model_nested, method_dot_bfgs, method_dot_frcg, method_dot_slp, method_dot_sqp, method_conmin_frcg, method_conmin_mfd, method_nonlinear_cg, method_genie_direct, method_genie_opt_darts, method_coliny_cobyla, method_coliny_pattern_search, multistart_ie_s0, method_coliny_direct, method_coliny_ea, method_coliny_solis_wets, method_optpp_cg, method_optpp_pds, method_ncsu_direct, method_nlpql_sqp, method_npsol_sqp, method_mesh_adaptive_search, method_asynch_pattern_search, method_soga, method_moga, method_local_reliability, method_global_reliability, method_global_interval_est, method_global_evidence, method_local_interval_est, method_local_evidence, method_importance_sampling, method_gpais, method_pof_darts, method_rkd_darts, method_adaptive_sampling, method_polynomial_chaos, method_stoch_collocation, method_function_train, method_surrogate_based_uq, method_multilevel_polynomial_chaos, method_multifidelity_polynomial_chaos, method_multifidelity_stoch_collocation, method_multilevel_function_train, method_multifidelity_function_train, method_multilevel_sampling, method_multifidelity_sampling, method_approximate_control_variate, method_multilevel_blue, method_multilevel_multifidelity_sampling, method_efficient_global, method_coliny_beta, method_nl2sol, method_branch_and_bound, method_richardson_extrap, method_bayes_calibration |

## Native classes exercised by the paired scripts

`APPSOptimizer`, `COLINOptimizer`, `CONMINOptimizer`, `ConcurrentMetaIterator`, `DOTOptimizer`, `DataFitSurrModel`, `EffGlobalMinimizer`, `EnsembleSurrModel`, `JEGAOptimizer`, `NCSUOptimizer`, `NL2SOLLeastSq`, `NLPQLPOptimizer`, `NPSOLOptimizer`, `NestedModel`, `NomadOptimizer`, `NonDACVSampling`, `NonDAdaptImpSampling`, `NonDAdaptiveSampling`, `NonDC3FunctionTrain`, `NonDGPImpSampling`, `NonDGlobalReliability`, `NonDImportPoints`, `NonDLHSEvidence`, `NonDLHSSampling`, `NonDLHSSingleInterval`, `NonDLocalEvidence`, `NonDLocalReliability`, `NonDLocalSingleInterval`, `NonDMultifidelitySampling`, `NonDMultilevBLUESampling`, `NonDMultilevControlVarSampling`, `NonDMultilevelFunctionTrain`, `NonDMultilevelPolynomialChaos`, `NonDMultilevelSampling`, `NonDMultilevelStochCollocation`, `NonDPOFDarts`, `NonDPolynomialChaos`, `NonDRKDDarts`, `NonDStochCollocation`, `NonDSurrogateExpansion`, `NonDWASABIBayesCalibration`, `NonlinearCGOptimizer`, `OptDartsOptimizer`, `ParamStudy`, `PebbldMinimizer`, `RichExtrapVerification`, `SNLLOptimizer`, `SimulationModel`

Multiple factory entry points can use the same native class; each factory
still needs its own execution case. Timing claims require successful native runs.

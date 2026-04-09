// Generated from ProblemDescDB getter registry on this branch.
#pragma once
#include <array>
#include <string_view>
namespace Dakota::problem_desc_db_dump {
inline constexpr std::string_view kExcludedLegacyVoidKey = "method.dl_solver.dlLib";

inline constexpr std::array<std::string_view, 28> k_environment_entries = {{
  "environment.error_file",
  "environment.output_file",
  "environment.post_run_input",
  "environment.post_run_output",
  "environment.pre_run_input",
  "environment.pre_run_output",
  "environment.read_restart",
  "environment.results_output_file",
  "environment.run_input",
  "environment.run_output",
  "environment.tabular_graphics_file",
  "environment.top_method_pointer",
  "environment.write_restart",
  "environment.output_precision",
  "environment.stop_restart",
  "environment.interface_evals_selection",
  "environment.model_evals_selection",
  "environment.post_run_input_format",
  "environment.pre_run_output_format",
  "environment.results_output_format",
  "environment.tabular_format",
  "environment.check",
  "environment.graphics",
  "environment.post_run",
  "environment.pre_run",
  "environment.results_output",
  "environment.run",
  "environment.tabular_graphics_data",
}};

inline constexpr std::array<std::string_view, 413> k_method_entries = {{
  "method.concurrent.parameter_sets",
  "method.jega.distance_vector",
  "method.jega.niche_vector",
  "method.nond.data_dist_covariance",
  "method.nond.data_dist_means",
  "method.nond.dimension_preference",
  "method.nond.hyperprior_alphas",
  "method.nond.hyperprior_betas",
  "method.nond.prediction_configs",
  "method.nond.proposal_covariance_data",
  "method.nond.regression_noise_tolerance",
  "method.nond.relaxation.factor_sequence",
  "method.nond.scalarization_response_mapping",
  "method.parameter_study.final_point",
  "method.parameter_study.list_of_points",
  "method.parameter_study.step_vector",
  "method.trust_region.initial_size",
  "method.fsu_quasi_mc.primeBase",
  "method.fsu_quasi_mc.sequenceLeap",
  "method.fsu_quasi_mc.sequenceStart",
  "method.nond.refinement_samples",
  "method.parameter_study.steps_per_variable",
  "method.generating_vector.inline",
  "method.generating_matrices.inline",
  "method.nond.mlmcmc_subsampling_steps",
  "method.nond.c3function_train.start_rank_sequence",
  "method.nond.collocation_points",
  "method.nond.collocation_points_sequence",
  "method.nond.expansion_samples",
  "method.nond.expansion_samples_sequence",
  "method.nond.pilot_samples",
  "method.random_seed_sequence",
  "method.nond.c3function_train.start_order_sequence",
  "method.nond.expansion_order",
  "method.nond.expansion_order_sequence",
  "method.nond.quadrature_order",
  "method.nond.quadrature_order_sequence",
  "method.nond.sparse_grid_level",
  "method.nond.sparse_grid_level_sequence",
  "method.nond.tensor_grid_order",
  "method.partitions",
  "method.nond.gen_reliability_levels",
  "method.nond.probability_levels",
  "method.nond.reliability_levels",
  "method.nond.response_levels",
  "method.coliny.misc_options",
  "method.hybrid.method_names",
  "method.hybrid.method_pointers",
  "method.hybrid.model_pointers",
  "method.advanced_options_file",
  "method.asynch_pattern_search.merit_function",
  "method.batch_selection",
  "method.class_path_and_name",
  "method.coliny.beta_solver_name",
  "method.coliny.division",
  "method.coliny.exploratory_moves",
  "method.coliny.pattern_basis",
  "method.crossover_type",
  "method.dl_solver.dlDetails",
  "method.export_approx_points_file",
  "method.fitness_metric",
  "method.fitness_type",
  "method.flat_file",
  "method.hybrid.global_method_name",
  "method.hybrid.global_method_pointer",
  "method.hybrid.global_model_pointer",
  "method.hybrid.local_method_name",
  "method.hybrid.local_method_pointer",
  "method.hybrid.local_model_pointer",
  "method.id",
  "method.import_approx_points_file",
  "method.import_build_points_file",
  "method.import_points_file",
  "method.import_candidate_points_file",
  "method.import_prediction_configs",
  "method.initialization_type",
  "method.jega.convergence_type",
  "method.jega.niching_type",
  "method.jega.postprocessor_type",
  "method.lipschitz",
  "method.log_file",
  "method.low_fidelity_model_pointer",
  "method.mesh_adaptive_search.display_format",
  "method.mesh_adaptive_search.history_file",
  "method.mesh_adaptive_search.use_surrogate",
  "method.model_export_prefix",
  "method.model_pointer",
  "method.mutation_type",
  "method.nond.data_dist_cov_type",
  "method.nond.data_dist_filename",
  "method.nond.data_dist_type",
  "method.nond.dili_hessian_type",
  "method.nond.discrepancy_type",
  "method.nond.dr_scale_type",
  "method.nond.export_corrected_model_file",
  "method.nond.export_corrected_variance_file",
  "method.nond.export_discrepancy_file",
  "method.nond.export_expansion_file",
  "method.nond.export_mcmc_points_file",
  "method.nond.import_expansion_file",
  "method.nond.mcmc_type",
  "method.nond.point_reuse",
  "method.nond.posterior_density_export_file",
  "method.nond.posterior_samples_export_file",
  "method.nond.posterior_samples_import_file",
  "method.nond.proposal_covariance_filename",
  "method.nond.proposal_covariance_input_type",
  "method.nond.proposal_covariance_type",
  "method.nond.reliability_integration",
  "method.optpp.search_method",
  "method.pstudy.import_file",
  "method.random_number_generator",
  "method.replacement_type",
  "method.sub_method_name",
  "method.sub_method_pointer",
  "method.sub_model_pointer",
  "method.trial_type",
  "method.generating_vector.file",
  "method.generating_matrices.file",
  "method.asynch_pattern_search.constraint_penalty",
  "method.asynch_pattern_search.contraction_factor",
  "method.asynch_pattern_search.initial_delta",
  "method.asynch_pattern_search.smoothing_factor",
  "method.asynch_pattern_search.solution_target",
  "method.coliny.contraction_factor",
  "method.coliny.global_balance_parameter",
  "method.coliny.initial_delta",
  "method.coliny.local_balance_parameter",
  "method.coliny.max_boxsize_limit",
  "method.coliny.variable_tolerance",
  "method.confidence_level",
  "method.constraint_penalty",
  "method.constraint_tolerance",
  "method.convergence_tolerance",
  "method.crossover_rate",
  "method.dream.gr_threshold",
  "method.function_precision",
  "method.gradient_tolerance",
  "method.hybrid.local_search_probability",
  "method.jega.fitness_limit",
  "method.jega.percent_change",
  "method.jega.shrinkage_percentage",
  "method.mesh_adaptive_search.initial_delta",
  "method.mesh_adaptive_search.variable_neighborhood_search",
  "method.mesh_adaptive_search.variable_tolerance",
  "method.min_boxsize_limit",
  "method.mutation_rate",
  "method.mutation_scale",
  "method.nl2sol.absolute_conv_tol",
  "method.nl2sol.false_conv_tol",
  "method.nl2sol.initial_trust_radius",
  "method.nl2sol.singular_conv_tol",
  "method.nl2sol.singular_radius",
  "method.nond.am_scale",
  "method.nond.c3function_train.solver_rounding_tolerance",
  "method.nond.c3function_train.solver_tolerance",
  "method.nond.c3function_train.stats_rounding_tolerance",
  "method.nond.collocation_ratio",
  "method.nond.collocation_ratio_terms_order",
  "method.nond.dili_hess_tolerance",
  "method.nond.dili_lis_tolerance",
  "method.nond.dili_ses_abs_tol",
  "method.nond.dili_ses_rel_tol",
  "method.nond.dr_scale",
  "method.nond.estimator_variance_metric_norm_order",
  "method.nond.mala_step_size",
  "method.nond.mlmcmc_greedy_resampling_factor",
  "method.nond.mlmcmc_target_variance",
  "method.nond.multilevel_estimator_rate",
  "method.nond.rcond_tol_throttle",
  "method.nond.regression_penalty",
  "method.nond.relaxation.fixed_factor",
  "method.nond.relaxation.recursive_factor",
  "method.npsol.linesearch_tolerance",
  "method.optpp.centering_parameter",
  "method.optpp.max_step",
  "method.optpp.steplength_to_boundary",
  "method.percent_variance_explained",
  "method.prior_prop_cov_mult",
  "method.solution_target",
  "method.ti_coverage",
  "method.ti_confidence_level",
  "method.trust_region.contract_threshold",
  "method.trust_region.contraction_factor",
  "method.trust_region.expand_threshold",
  "method.trust_region.expansion_factor",
  "method.trust_region.minimum_size",
  "method.variable_tolerance",
  "method.vbd_drop_tolerance",
  "method.verification.refinement_rate",
  "method.volume_boxsize_limit",
  "method.x_conv_tol",
  "method.adapt_exp_design_samples",
  "method.batch_size",
  "method.batch_size.exploration",
  "method.build_samples",
  "method.burn_in_samples",
  "method.coliny.contract_after_failure",
  "method.coliny.expand_after_success",
  "method.coliny.mutation_range",
  "method.coliny.new_solutions_generated",
  "method.coliny.number_retained",
  "method.coliny.total_pattern_size",
  "method.concurrent.random_jobs",
  "method.dream.crossover_chain_pairs",
  "method.dream.jump_step",
  "method.dream.num_chains",
  "method.dream.num_cr",
  "method.evidence_samples",
  "method.fsu_cvt.num_trials",
  "method.iterator_servers",
  "method.max_hifi_evaluations",
  "method.mesh_adaptive_search.neighbor_order",
  "method.nl2sol.covariance",
  "method.nond.am_period_num_steps",
  "method.nond.am_starting_step",
  "method.nond.c3function_train.max_cross_iterations",
  "method.nond.chain_samples",
  "method.nond.dili_adapt_end",
  "method.nond.dili_adapt_interval",
  "method.nond.dili_adapt_start",
  "method.nond.dili_initial_weight",
  "method.nond.dili_ses_block_size",
  "method.nond.dili_ses_exp_rank",
  "method.nond.dili_ses_num_eigs",
  "method.nond.dili_ses_overs_factor",
  "method.nond.dr_num_stages",
  "method.nond.mlmcmc_initial_chain_samples",
  "method.nond.prop_cov_update_period",
  "method.nond.pushforward_samples",
  "method.nond.samples_on_emulator",
  "method.nond.surrogate_order",
  "method.npsol.verify_level",
  "method.optpp.search_scheme_size",
  "method.parameter_study.num_steps",
  "method.population_size",
  "method.processors_per_iterator",
  "method.random_seed",
  "method.samples",
  "method.sub_sampling_period",
  "method.symbols",
  "method.vbd_via_sampling_num_bins",
  "method.m_max",
  "method.t_max",
  "method.t_scramble",
  "method.iterator_scheduling",
  "method.ld.digitalnet.generating_matrix_scheme",
  "method.ld.digitalnet.ordering",
  "method.ld.rank1.generating_vector_scheme",
  "method.ld.rank1.ordering",
  "method.nond.allocation_target",
  "method.nond.c3function_train.advancement_type",
  "method.nond.convergence_tolerance_target",
  "method.nond.convergence_tolerance_type",
  "method.nond.covariance_control",
  "method.nond.distribution",
  "method.nond.emulator",
  "method.nond.ensemble_pilot_solution_mode",
  "method.nond.estimator_variance_metric",
  "method.nond.expansion_basis_type",
  "method.nond.expansion_refinement_control",
  "method.nond.expansion_refinement_metric",
  "method.nond.expansion_refinement_type",
  "method.nond.expansion_type",
  "method.nond.final_moments",
  "method.nond.final_statistics",
  "method.nond.group_throttle_type",
  "method.nond.growth_override",
  "method.nond.least_squares_regression_type",
  "method.nond.model_discrepancy.polynomial_order",
  "method.nond.multilevel_allocation_control",
  "method.nond.multilevel_discrepancy_emulation",
  "method.nond.nesting_override",
  "method.nond.pilot_samples.mode",
  "method.nond.qoi_aggregation",
  "method.nond.refinement_statistics_mode",
  "method.nond.regression_type",
  "method.nond.response_level_target",
  "method.nond.response_level_target_reduce",
  "method.nond.search_model_graphs.recursion",
  "method.nond.search_model_graphs.selection",
  "method.optpp.merit_function",
  "method.output",
  "method.sbl.acceptance_logic",
  "method.sbl.constraint_relax",
  "method.sbl.merit_function",
  "method.sbl.subproblem_constraints",
  "method.sbl.subproblem_objective",
  "method.synchronization",
  "method.wilks.sided_interval",
  "method.algorithm",
  "method.export_approx_format",
  "method.import_approx_format",
  "method.import_build_format",
  "method.import_points_format",
  "method.import_candidate_format",
  "method.import_prediction_configs_format",
  "method.model_export_format",
  "method.nond.adapted_basis.advancements",
  "method.nond.c3function_train.kick_order",
  "method.nond.c3function_train.max_order",
  "method.nond.c3function_train.start_order",
  "method.nond.calibrate_error_mode",
  "method.nond.cross_validation.max_order_candidates",
  "method.nond.cubature_integrand",
  "method.nond.export_corrected_model_format",
  "method.nond.export_corrected_variance_format",
  "method.nond.export_discrep_format",
  "method.nond.export_samples_format",
  "method.nond.graph_depth_limit",
  "method.nond.group_size_throttle",
  "method.nond.integration_refinement",
  "method.nond.model_reordering",
  "method.nond.numerical_solve_mode",
  "method.nond.opt_subproblem_solver",
  "method.nond.vbd_interaction_order",
  "method.order",
  "method.pstudy.import_format",
  "method.sample_type",
  "method.soft_convergence_limit",
  "method.sub_method",
  "method.vbd_via_sampling_method",
  "method.final_solutions",
  "method.jega.num_cross_points",
  "method.jega.num_designs",
  "method.jega.num_generations",
  "method.jega.num_offspring",
  "method.jega.num_parents",
  "method.max_function_evaluations",
  "method.max_iterations",
  "method.nond.c3function_train.kick_rank",
  "method.nond.c3function_train.max_rank",
  "method.nond.c3function_train.start_rank",
  "method.nond.cross_validation.max_rank_candidates",
  "method.nond.max_refinement_iterations",
  "method.nond.max_solver_iterations",
  "method.nond.rcond_best_throttle",
  "method.num_candidate_designs",
  "method.num_candidates",
  "method.num_prediction_configs",
  "method.backfill",
  "method.chain_diagnostics",
  "method.chain_diagnostics.confidence_intervals",
  "method.coliny.constant_penalty",
  "method.coliny.expansion",
  "method.coliny.randomize",
  "method.coliny.show_misc_options",
  "method.derivative_usage",
  "method.export_surrogate",
  "method.fixed_seed",
  "method.fsu_quasi_mc.fixed_sequence",
  "method.import_approx_active_only",
  "method.import_build_active_only",
  "method.import_points.active_only",
  "method.import_points.use_variable_labels",
  "method.laplace_approx",
  "method.latinize",
  "method.main_effects",
  "method.mc_approx",
  "method.mesh_adaptive_search.display_all_evaluations",
  "method.model_evidence",
  "method.mutation_adaptive",
  "method.nl2sol.regression_diagnostics",
  "method.nond.adapt_exp_design",
  "method.nond.adaptive_posterior_refinement",
  "method.nond.allocation_target.optimization",
  "method.nond.c3function_train.adapt_order",
  "method.nond.c3function_train.adapt_rank",
  "method.nond.cross_validation",
  "method.nond.cross_validation.noise_only",
  "method.nond.d_optimal",
  "method.nond.evaluate_posterior_density",
  "method.nond.export_sample_sequence",
  "method.nond.generate_posterior_samples",
  "method.nond.gpmsa_normalize",
  "method.nond.logit_transform",
  "method.nond.model_discrepancy",
  "method.nond.mutual_info_ksg2",
  "method.nond.normalized",
  "method.nond.piecewise_basis",
  "method.nond.response_scaling",
  "method.nond.standardized_space",
  "method.nond.tensor_grid",
  "method.nond.truth_fixed_by_pilot",
  "method.posterior_stats.kde",
  "method.posterior_stats.kl_divergence",
  "method.posterior_stats.mutual_info",
  "method.principal_components",
  "method.print_each_pop",
  "method.pstudy.import_active_only",
  "method.quality_metrics",
  "method.sbg.replace_points",
  "method.sbl.truth_surrogate_bypass",
  "method.scaling",
  "method.speculative",
  "method.std_regression_coeffs",
  "method.tolerance_intervals",
  "method.variance_based_decomp",
  "method.wilks",
  "method.rank_1_lattice",
  "method.no_random_shift",
  "method.kuo",
  "method.cools_kuo_nuyens",
  "method.ordering.natural",
  "method.ordering.radical_inverse",
  "method.digital_net",
  "method.no_digital_shift",
  "method.no_scrambling",
  "method.most_significant_bit_first",
  "method.least_significant_bit_first",
  "method.joe_kuo",
  "method.sobol_order_2",
  "method.gray_code_ordering",
}};

inline constexpr std::array<std::string_view, 135> k_model_entries = {{
  "model.nested.primary_response_mapping",
  "model.nested.secondary_response_mapping",
  "model.simulation.solution_level_cost",
  "model.surrogate.kriging_correlations",
  "model.refinement_samples",
  "model.surrogate.function_indices",
  "model.metrics",
  "model.nested.primary_variable_mapping",
  "model.nested.secondary_variable_mapping",
  "model.surrogate.ensemble_model_pointers",
  "model.advanced_options_file",
  "model.dace_method_pointer",
  "model.id",
  "model.interface_pointer",
  "model.nested.sub_method_pointer",
  "model.optional_interface_responses_pointer",
  "model.rf.propagation_model_pointer",
  "model.rf_data_file",
  "model.simulation.cost_recovery_metadata",
  "model.simulation.solution_level_control",
  "model.surrogate.truth_model_pointer",
  "model.surrogate.challenge_points_file",
  "model.surrogate.decomp_cell_type",
  "model.surrogate.export_approx_points_file",
  "model.surrogate.export_approx_variance_file",
  "model.surrogate.import_build_points_file",
  "model.surrogate.kriging_opt_method",
  "model.surrogate.mars_interpolation",
  "model.surrogate.model_export_prefix",
  "model.surrogate.model_import_prefix",
  "model.surrogate.class_path_and_name",
  "model.surrogate.point_reuse",
  "model.surrogate.refine_cv_metric",
  "model.surrogate.trend_order",
  "model.surrogate.type",
  "model.type",
  "model.active_subspace.cv.decrease_tolerance",
  "model.active_subspace.cv.relative_tolerance",
  "model.active_subspace.truncation_method.energy.truncation_tolerance",
  "model.adapted_basis.collocation_ratio",
  "model.adapted_basis.truncation_tolerance",
  "model.c3function_train.collocation_ratio",
  "model.c3function_train.solver_rounding_tolerance",
  "model.c3function_train.solver_tolerance",
  "model.c3function_train.stats_rounding_tolerance",
  "model.convergence_tolerance",
  "model.surrogate.discont_grad_thresh",
  "model.surrogate.discont_jump_thresh",
  "model.surrogate.neural_network_range",
  "model.surrogate.nugget",
  "model.surrogate.percent",
  "model.surrogate.regression_penalty",
  "model.truncation_tolerance",
  "model.active_subspace.bootstrap_samples",
  "model.active_subspace.cv.max_rank",
  "model.c3function_train.max_cross_iterations",
  "model.initial_samples",
  "model.nested.iterator_servers",
  "model.nested.processors_per_iterator",
  "model.rf.expansion_bases",
  "model.soft_convergence_limit",
  "model.subspace.dimension",
  "model.surrogate.decomp_support_layers",
  "model.surrogate.folds",
  "model.surrogate.num_restarts",
  "model.surrogate.points_total",
  "model.surrogate.refine_cv_folds",
  "model.adapted_basis.rotation_method",
  "model.c3function_train.advancement_type",
  "model.nested.iterator_scheduling",
  "model.surrogate.correction_order",
  "model.surrogate.correction_type",
  "model.surrogate.find_nugget",
  "model.surrogate.kriging_max_trials",
  "model.surrogate.mars_max_bases",
  "model.surrogate.mls_weight_function",
  "model.surrogate.neural_network_nodes",
  "model.surrogate.neural_network_random_weight",
  "model.surrogate.points_management",
  "model.surrogate.polynomial_order",
  "model.surrogate.rbf_bases",
  "model.surrogate.rbf_max_pts",
  "model.surrogate.rbf_max_subsets",
  "model.surrogate.rbf_min_partition",
  "model.surrogate.regression_type",
  "model.active_subspace.cv.id_method",
  "model.active_subspace.normalization",
  "model.active_subspace.sample_type",
  "model.adapted_basis.expansion_order",
  "model.adapted_basis.sparse_grid_level",
  "model.c3function_train.cross_validation.max_order_candidates",
  "model.c3function_train.kick_order",
  "model.c3function_train.max_order",
  "model.c3function_train.start_order",
  "model.rf.analytic_covariance",
  "model.rf.expansion_form",
  "model.surrogate.challenge_points_file_format",
  "model.surrogate.export_approx_format",
  "model.surrogate.export_approx_variance_format",
  "model.surrogate.import_build_format",
  "model.surrogate.model_export_format",
  "model.surrogate.model_import_format",
  "model.c3function_train.collocation_points",
  "model.c3function_train.cross_validation.max_rank_candidates",
  "model.c3function_train.kick_rank",
  "model.c3function_train.max_rank",
  "model.c3function_train.start_rank",
  "model.max_function_evals",
  "model.max_iterations",
  "model.max_solver_iterations",
  "model.active_subspace.build_surrogate",
  "model.active_subspace.cv.incremental",
  "model.active_subspace.truncation_method.bing_li",
  "model.active_subspace.truncation_method.constantine",
  "model.active_subspace.truncation_method.cv",
  "model.active_subspace.truncation_method.energy",
  "model.c3function_train.adapt_order",
  "model.c3function_train.adapt_rank",
  "model.c3function_train.tensor_grid",
  "model.hierarchical_tags",
  "model.nested.identity_resp_map",
  "model.surrogate.auto_refine",
  "model.surrogate.challenge_points_file_active",
  "model.surrogate.challenge_use_variable_labels",
  "model.surrogate.cross_validate",
  "model.surrogate.decomp_discont_detect",
  "model.surrogate.derivative_usage",
  "model.surrogate.domain_decomp",
  "model.surrogate.export_surrogate",
  "model.surrogate.import_build_active_only",
  "model.surrogate.import_surrogate",
  "model.surrogate.import_use_variable_labels",
  "model.surrogate.point_selection",
  "model.surrogate.press",
  "model.surrogate.response_scaling",
}};

inline constexpr std::array<std::string_view, 299> k_variables_entries = {{
  "variables.discrete_design_set_int.adjacency_matrix",
  "variables.discrete_design_set_real.adjacency_matrix",
  "variables.discrete_design_set_str.adjacency_matrix",
  "variables.beta_uncertain.alphas",
  "variables.beta_uncertain.betas",
  "variables.beta_uncertain.initial_point",
  "variables.beta_uncertain.lower_bounds",
  "variables.beta_uncertain.upper_bounds",
  "variables.binomial_uncertain.prob_per_trial",
  "variables.continuous_aleatory_uncertain.initial_point",
  "variables.continuous_aleatory_uncertain.lower_bounds",
  "variables.continuous_aleatory_uncertain.upper_bounds",
  "variables.continuous_design.initial_point",
  "variables.continuous_design.lower_bounds",
  "variables.continuous_design.scales",
  "variables.continuous_design.upper_bounds",
  "variables.continuous_epistemic_uncertain.initial_point",
  "variables.continuous_epistemic_uncertain.lower_bounds",
  "variables.continuous_epistemic_uncertain.upper_bounds",
  "variables.continuous_interval_uncertain.lower_bounds",
  "variables.continuous_interval_uncertain.upper_bounds",
  "variables.continuous_interval_uncertain.initial_point",
  "variables.continuous_state.initial_state",
  "variables.continuous_state.lower_bounds",
  "variables.continuous_state.upper_bounds",
  "variables.discrete_aleatory_uncertain_real.initial_point",
  "variables.discrete_aleatory_uncertain_real.lower_bounds",
  "variables.discrete_aleatory_uncertain_real.upper_bounds",
  "variables.discrete_design_set_real.initial_point",
  "variables.discrete_design_set_real.lower_bounds",
  "variables.discrete_design_set_real.upper_bounds",
  "variables.discrete_epistemic_uncertain_real.initial_point",
  "variables.discrete_epistemic_uncertain_real.lower_bounds",
  "variables.discrete_epistemic_uncertain_real.upper_bounds",
  "variables.discrete_state_set_real.initial_state",
  "variables.discrete_state_set_real.lower_bounds",
  "variables.discrete_state_set_real.upper_bounds",
  "variables.discrete_uncertain_set_real.lower_bounds",
  "variables.discrete_uncertain_set_real.upper_bounds",
  "variables.discrete_uncertain_set_real.initial_point",
  "variables.exponential_uncertain.betas",
  "variables.exponential_uncertain.lower_bounds",
  "variables.exponential_uncertain.upper_bounds",
  "variables.exponential_uncertain.initial_point",
  "variables.frechet_uncertain.alphas",
  "variables.frechet_uncertain.betas",
  "variables.frechet_uncertain.lower_bounds",
  "variables.frechet_uncertain.upper_bounds",
  "variables.frechet_uncertain.initial_point",
  "variables.gamma_uncertain.alphas",
  "variables.gamma_uncertain.betas",
  "variables.gamma_uncertain.lower_bounds",
  "variables.gamma_uncertain.upper_bounds",
  "variables.gamma_uncertain.initial_point",
  "variables.geometric_uncertain.prob_per_trial",
  "variables.gumbel_uncertain.alphas",
  "variables.gumbel_uncertain.betas",
  "variables.gumbel_uncertain.lower_bounds",
  "variables.gumbel_uncertain.upper_bounds",
  "variables.gumbel_uncertain.initial_point",
  "variables.histogram_bin_uncertain.lower_bounds",
  "variables.histogram_bin_uncertain.upper_bounds",
  "variables.histogram_bin_uncertain.initial_point",
  "variables.histogram_uncertain.point_real.lower_bounds",
  "variables.histogram_uncertain.point_real.upper_bounds",
  "variables.histogram_uncertain.point_real.initial_point",
  "variables.linear_equality_constraints",
  "variables.linear_equality_scales",
  "variables.linear_equality_targets",
  "variables.linear_inequality_constraints",
  "variables.linear_inequality_lower_bounds",
  "variables.linear_inequality_scales",
  "variables.linear_inequality_upper_bounds",
  "variables.lognormal_uncertain.error_factors",
  "variables.lognormal_uncertain.inferred_lower_bounds",
  "variables.lognormal_uncertain.inferred_upper_bounds",
  "variables.lognormal_uncertain.lambdas",
  "variables.lognormal_uncertain.initial_point",
  "variables.lognormal_uncertain.lower_bounds",
  "variables.lognormal_uncertain.means",
  "variables.lognormal_uncertain.std_deviations",
  "variables.lognormal_uncertain.upper_bounds",
  "variables.lognormal_uncertain.zetas",
  "variables.loguniform_uncertain.initial_point",
  "variables.loguniform_uncertain.lower_bounds",
  "variables.loguniform_uncertain.upper_bounds",
  "variables.negative_binomial_uncertain.prob_per_trial",
  "variables.normal_uncertain.inferred_lower_bounds",
  "variables.normal_uncertain.inferred_upper_bounds",
  "variables.normal_uncertain.initial_point",
  "variables.normal_uncertain.lower_bounds",
  "variables.normal_uncertain.means",
  "variables.normal_uncertain.std_deviations",
  "variables.normal_uncertain.upper_bounds",
  "variables.poisson_uncertain.lambdas",
  "variables.triangular_uncertain.initial_point",
  "variables.triangular_uncertain.lower_bounds",
  "variables.triangular_uncertain.modes",
  "variables.triangular_uncertain.upper_bounds",
  "variables.uniform_uncertain.initial_point",
  "variables.uniform_uncertain.lower_bounds",
  "variables.uniform_uncertain.upper_bounds",
  "variables.weibull_uncertain.alphas",
  "variables.weibull_uncertain.betas",
  "variables.weibull_uncertain.lower_bounds",
  "variables.weibull_uncertain.upper_bounds",
  "variables.weibull_uncertain.initial_point",
  "variables.binomial_uncertain.num_trials",
  "variables.binomial_uncertain.lower_bounds",
  "variables.binomial_uncertain.upper_bounds",
  "variables.binomial_uncertain.initial_point",
  "variables.discrete_aleatory_uncertain_int.initial_point",
  "variables.discrete_aleatory_uncertain_int.lower_bounds",
  "variables.discrete_aleatory_uncertain_int.upper_bounds",
  "variables.discrete_design_range.initial_point",
  "variables.discrete_design_range.lower_bounds",
  "variables.discrete_design_range.upper_bounds",
  "variables.discrete_design_set_int.initial_point",
  "variables.discrete_design_set_int.lower_bounds",
  "variables.discrete_design_set_int.upper_bounds",
  "variables.discrete_epistemic_uncertain_int.initial_point",
  "variables.discrete_epistemic_uncertain_int.lower_bounds",
  "variables.discrete_epistemic_uncertain_int.upper_bounds",
  "variables.discrete_interval_uncertain.lower_bounds",
  "variables.discrete_interval_uncertain.upper_bounds",
  "variables.discrete_interval_uncertain.initial_point",
  "variables.discrete_state_range.initial_state",
  "variables.discrete_state_range.lower_bounds",
  "variables.discrete_state_range.upper_bounds",
  "variables.discrete_state_set_int.initial_state",
  "variables.discrete_state_set_int.lower_bounds",
  "variables.discrete_state_set_int.upper_bounds",
  "variables.discrete_uncertain_set_int.lower_bounds",
  "variables.discrete_uncertain_set_int.upper_bounds",
  "variables.discrete_uncertain_set_int.initial_point",
  "variables.geometric_uncertain.lower_bounds",
  "variables.geometric_uncertain.upper_bounds",
  "variables.geometric_uncertain.initial_point",
  "variables.histogram_uncertain.point_int.lower_bounds",
  "variables.histogram_uncertain.point_int.upper_bounds",
  "variables.histogram_uncertain.point_int.initial_point",
  "variables.hypergeometric_uncertain.lower_bounds",
  "variables.hypergeometric_uncertain.upper_bounds",
  "variables.hypergeometric_uncertain.initial_point",
  "variables.hypergeometric_uncertain.num_drawn",
  "variables.hypergeometric_uncertain.selected_population",
  "variables.hypergeometric_uncertain.total_population",
  "variables.negative_binomial_uncertain.lower_bounds",
  "variables.negative_binomial_uncertain.upper_bounds",
  "variables.negative_binomial_uncertain.initial_point",
  "variables.negative_binomial_uncertain.num_trials",
  "variables.poisson_uncertain.lower_bounds",
  "variables.poisson_uncertain.upper_bounds",
  "variables.poisson_uncertain.initial_point",
  "variables.binomial_uncertain.categorical",
  "variables.discrete_design_range.categorical",
  "variables.discrete_design_set_int.categorical",
  "variables.discrete_design_set_real.categorical",
  "variables.discrete_interval_uncertain.categorical",
  "variables.discrete_state_range.categorical",
  "variables.discrete_state_set_int.categorical",
  "variables.discrete_state_set_real.categorical",
  "variables.discrete_uncertain_set_int.categorical",
  "variables.discrete_uncertain_set_real.categorical",
  "variables.geometric_uncertain.categorical",
  "variables.histogram_uncertain.point_int.categorical",
  "variables.histogram_uncertain.point_real.categorical",
  "variables.hypergeometric_uncertain.categorical",
  "variables.negative_binomial_uncertain.categorical",
  "variables.poisson_uncertain.categorical",
  "variables.uncertain.correlation_matrix",
  "variables.discrete_design_set_int.values",
  "variables.discrete_state_set_int.values",
  "variables.discrete_design_set_string.values",
  "variables.discrete_state_set_string.values",
  "variables.discrete_design_set_real.values",
  "variables.discrete_state_set_real.values",
  "variables.discrete_uncertain_set_int.values_probs",
  "variables.histogram_uncertain.point_int_pairs",
  "variables.discrete_uncertain_set_string.values_probs",
  "variables.histogram_uncertain.point_string_pairs",
  "variables.discrete_uncertain_set_real.values_probs",
  "variables.histogram_uncertain.bin_pairs",
  "variables.histogram_uncertain.point_real_pairs",
  "variables.continuous_interval_uncertain.basic_probs",
  "variables.discrete_interval_uncertain.basic_probs",
  "variables.continuous_aleatory_uncertain.labels",
  "variables.normal_uncertain.labels",
  "variables.lognormal_uncertain.labels",
  "variables.uniform_uncertain.labels",
  "variables.loguniform_uncertain.labels",
  "variables.triangular_uncertain.labels",
  "variables.exponential_uncertain.labels",
  "variables.beta_uncertain.labels",
  "variables.gamma_uncertain.labels",
  "variables.gumbel_uncertain.labels",
  "variables.frechet_uncertain.labels",
  "variables.weibull_uncertain.labels",
  "variables.histogram_bin_uncertain.labels",
  "variables.continuous_design.labels",
  "variables.continuous_design.scale_types",
  "variables.continuous_epistemic_uncertain.labels",
  "variables.continuous_interval_uncertain.labels",
  "variables.continuous_state.labels",
  "variables.discrete_aleatory_uncertain_int.labels",
  "variables.poisson_uncertain.labels",
  "variables.binomial_uncertain.labels",
  "variables.negative_binomial_uncertain.labels",
  "variables.geometric_uncertain.labels",
  "variables.hypergeometric_uncertain.labels",
  "variables.histogram_uncertain.point_int.labels",
  "variables.discrete_aleatory_uncertain_real.labels",
  "variables.histogram_uncertain.point_real.labels",
  "variables.discrete_aleatory_uncertain_string.initial_point",
  "variables.discrete_aleatory_uncertain_string.labels",
  "variables.histogram_uncertain.point_string.labels",
  "variables.discrete_aleatory_uncertain_string.lower_bounds",
  "variables.discrete_aleatory_uncertain_string.upper_bounds",
  "variables.discrete_design_range.labels",
  "variables.discrete_design_set_int.labels",
  "variables.discrete_design_set_real.labels",
  "variables.discrete_design_set_string.initial_point",
  "variables.discrete_design_set_string.labels",
  "variables.discrete_design_set_string.lower_bounds",
  "variables.discrete_design_set_string.upper_bounds",
  "variables.discrete_epistemic_uncertain_int.labels",
  "variables.discrete_interval_uncertain.labels",
  "variables.discrete_uncertain_set_int.labels",
  "variables.discrete_epistemic_uncertain_real.labels",
  "variables.discrete_uncertain_set_real.labels",
  "variables.discrete_epistemic_uncertain_string.initial_point",
  "variables.discrete_epistemic_uncertain_string.labels",
  "variables.discrete_uncertain_set_string.labels",
  "variables.discrete_epistemic_uncertain_string.lower_bounds",
  "variables.discrete_epistemic_uncertain_string.upper_bounds",
  "variables.discrete_state_range.labels",
  "variables.discrete_state_set_int.labels",
  "variables.discrete_state_set_real.labels",
  "variables.discrete_state_set_string.initial_state",
  "variables.discrete_state_set_string.labels",
  "variables.discrete_state_set_string.lower_bounds",
  "variables.discrete_state_set_string.upper_bounds",
  "variables.discrete_uncertain_set_string.lower_bounds",
  "variables.discrete_uncertain_set_string.upper_bounds",
  "variables.discrete_uncertain_set_string.initial_point",
  "variables.histogram_uncertain.point_string.lower_bounds",
  "variables.histogram_uncertain.point_string.upper_bounds",
  "variables.histogram_uncertain.point_string.initial_point",
  "variables.linear_equality_scale_types",
  "variables.linear_inequality_scale_types",
  "variables.id",
  "variables.domain",
  "variables.view",
  "variables.aleatory_uncertain",
  "variables.beta_uncertain",
  "variables.binomial_uncertain",
  "variables.continuous",
  "variables.continuous_design",
  "variables.continuous_interval_uncertain",
  "variables.continuous_state",
  "variables.design",
  "variables.discrete",
  "variables.discrete_design_range",
  "variables.discrete_design_set_int",
  "variables.discrete_design_set_real",
  "variables.discrete_design_set_string",
  "variables.discrete_interval_uncertain",
  "variables.epistemic_uncertain",
  "variables.discrete_state_range",
  "variables.discrete_state_set_int",
  "variables.discrete_state_set_real",
  "variables.discrete_state_set_string",
  "variables.discrete_uncertain_set_int",
  "variables.discrete_uncertain_set_real",
  "variables.discrete_uncertain_set_string",
  "variables.exponential_uncertain",
  "variables.frechet_uncertain",
  "variables.gamma_uncertain",
  "variables.geometric_uncertain",
  "variables.gumbel_uncertain",
  "variables.histogram_uncertain.bin",
  "variables.histogram_uncertain.point_int",
  "variables.histogram_uncertain.point_real",
  "variables.histogram_uncertain.point_string",
  "variables.hypergeometric_uncertain",
  "variables.lognormal_uncertain",
  "variables.loguniform_uncertain",
  "variables.negative_binomial_uncertain",
  "variables.normal_uncertain",
  "variables.poisson_uncertain",
  "variables.state",
  "variables.total",
  "variables.triangular_uncertain",
  "variables.uncertain",
  "variables.uniform_uncertain",
  "variables.weibull_uncertain",
  "variables.uncertain.initial_point_flag",
}};

inline constexpr std::array<std::string_view, 44> k_interface_entries = {{
  "interface.failure_capture.recovery_fn_vals",
  "interface.application.analysis_drivers",
  "interface.copyFiles",
  "interface.linkFiles",
  "interface.application.analysis_components",
  "interface.algebraic_mappings",
  "interface.application.input_filter",
  "interface.application.output_filter",
  "interface.application.parameters_file",
  "interface.application.results_file",
  "interface.failure_capture.action",
  "interface.id",
  "interface.plugin_library_path",
  "interface.workDir",
  "interface.nearby_evaluation_cache_tolerance",
  "interface.analysis_servers",
  "interface.asynch_local_analysis_concurrency",
  "interface.asynch_local_evaluation_concurrency",
  "interface.direct.processors_per_analysis",
  "interface.evaluation_servers",
  "interface.failure_capture.retry_limit",
  "interface.processors_per_evaluation",
  "interface.analysis_scheduling",
  "interface.evaluation_scheduling",
  "interface.local_evaluation_scheduling",
  "interface.application.parameters_file_format",
  "interface.application.results_file_format",
  "interface.type",
  "interface.active_set_vector",
  "interface.allow_existing_results",
  "interface.application.file_save",
  "interface.application.file_tag",
  "interface.application.verbatim",
  "interface.asynch",
  "interface.batch",
  "interface.dirSave",
  "interface.dirTag",
  "interface.evaluation_cache",
  "interface.labeled_results",
  "interface.nearby_evaluation_cache",
  "interface.python.numpy",
  "interface.restart_file",
  "interface.templateReplace",
  "interface.useWorkdir",
}};

inline constexpr std::array<std::string_view, 57> k_responses_entries = {{
  "responses.fd_gradient_step_size",
  "responses.fd_hessian_step_size",
  "responses.nonlinear_equality_scales",
  "responses.nonlinear_equality_targets",
  "responses.nonlinear_inequality_lower_bounds",
  "responses.nonlinear_inequality_scales",
  "responses.nonlinear_inequality_upper_bounds",
  "responses.primary_response_fn_scales",
  "responses.primary_response_fn_weights",
  "responses.simulation_variance",
  "responses.lengths",
  "responses.num_coordinates_per_field",
  "responses.gradients.mixed.id_analytic",
  "responses.gradients.mixed.id_numerical",
  "responses.hessians.mixed.id_analytic",
  "responses.hessians.mixed.id_numerical",
  "responses.hessians.mixed.id_quasi",
  "responses.labels",
  "responses.metadata_labels",
  "responses.nonlinear_equality_scale_types",
  "responses.nonlinear_inequality_scale_types",
  "responses.primary_response_fn_scale_types",
  "responses.primary_response_fn_sense",
  "responses.variance_type",
  "responses.data_directory",
  "responses.fd_gradient_step_type",
  "responses.fd_hessian_step_type",
  "responses.gradient_type",
  "responses.hessian_type",
  "responses.id",
  "responses.interval_type",
  "responses.method_source",
  "responses.quasi_hessian_type",
  "responses.scalar_data_filename",
  "responses.scalar_data_format",
  "responses.num_calibration_terms",
  "responses.num_config_vars",
  "responses.num_experiments",
  "responses.num_field_calibration_terms",
  "responses.num_field_nonlinear_equality_constraints",
  "responses.num_field_nonlinear_inequality_constraints",
  "responses.num_field_objectives",
  "responses.num_field_responses",
  "responses.num_nonlinear_equality_constraints",
  "responses.num_nonlinear_inequality_constraints",
  "responses.num_objective_functions",
  "responses.num_response_functions",
  "responses.num_scalar_calibration_terms",
  "responses.num_scalar_nonlinear_equality_constraints",
  "responses.num_scalar_nonlinear_inequality_constraints",
  "responses.num_scalar_objectives",
  "responses.num_scalar_responses",
  "responses.calibration_data",
  "responses.central_hess",
  "responses.ignore_bounds",
  "responses.interpolate",
  "responses.read_field_coordinates",
}};

template <class Emit>
bool try_emit_environment_entry(const DataEnvironmentRep& rep, std::string_view full_key, Emit&& emit)
{
  if (full_key == "environment.error_file") { emit(rep.errorFile); return true; }
  if (full_key == "environment.output_file") { emit(rep.outputFile); return true; }
  if (full_key == "environment.post_run_input") { emit(rep.postRunInput); return true; }
  if (full_key == "environment.post_run_output") { emit(rep.postRunOutput); return true; }
  if (full_key == "environment.pre_run_input") { emit(rep.preRunInput); return true; }
  if (full_key == "environment.pre_run_output") { emit(rep.preRunOutput); return true; }
  if (full_key == "environment.read_restart") { emit(rep.readRestart); return true; }
  if (full_key == "environment.results_output_file") { emit(rep.resultsOutputFile); return true; }
  if (full_key == "environment.run_input") { emit(rep.runInput); return true; }
  if (full_key == "environment.run_output") { emit(rep.runOutput); return true; }
  if (full_key == "environment.tabular_graphics_file") { emit(rep.tabularDataFile); return true; }
  if (full_key == "environment.top_method_pointer") { emit(rep.topMethodPointer); return true; }
  if (full_key == "environment.write_restart") { emit(rep.writeRestart); return true; }
  if (full_key == "environment.output_precision") { emit(rep.outputPrecision); return true; }
  if (full_key == "environment.stop_restart") { emit(rep.stopRestart); return true; }
  if (full_key == "environment.interface_evals_selection") { emit(rep.interfEvalsSelection); return true; }
  if (full_key == "environment.model_evals_selection") { emit(rep.modelEvalsSelection); return true; }
  if (full_key == "environment.post_run_input_format") { emit(rep.postRunInputFormat); return true; }
  if (full_key == "environment.pre_run_output_format") { emit(rep.preRunOutputFormat); return true; }
  if (full_key == "environment.results_output_format") { emit(rep.resultsOutputFormat); return true; }
  if (full_key == "environment.tabular_format") { emit(rep.tabularFormat); return true; }
  if (full_key == "environment.check") { emit(rep.checkFlag); return true; }
  if (full_key == "environment.graphics") { emit(rep.graphicsFlag); return true; }
  if (full_key == "environment.post_run") { emit(rep.postRunFlag); return true; }
  if (full_key == "environment.pre_run") { emit(rep.preRunFlag); return true; }
  if (full_key == "environment.results_output") { emit(rep.resultsOutputFlag); return true; }
  if (full_key == "environment.run") { emit(rep.runFlag); return true; }
  if (full_key == "environment.tabular_graphics_data") { emit(rep.tabularDataFlag); return true; }
  return false;
}

template <class Emit>
bool try_emit_method_entry(const DataMethodRep& rep, std::string_view full_key, Emit&& emit)
{
  if (full_key == "method.concurrent.parameter_sets") { emit(rep.concurrentParameterSets); return true; }
  if (full_key == "method.jega.distance_vector") { emit(rep.distanceVector); return true; }
  if (full_key == "method.jega.niche_vector") { emit(rep.nicheVector); return true; }
  if (full_key == "method.nond.data_dist_covariance") { emit(rep.dataDistCovariance); return true; }
  if (full_key == "method.nond.data_dist_means") { emit(rep.dataDistMeans); return true; }
  if (full_key == "method.nond.dimension_preference") { emit(rep.anisoDimPref); return true; }
  if (full_key == "method.nond.hyperprior_alphas") { emit(rep.hyperPriorAlphas); return true; }
  if (full_key == "method.nond.hyperprior_betas") { emit(rep.hyperPriorBetas); return true; }
  if (full_key == "method.nond.prediction_configs") { emit(rep.predictionConfigList); return true; }
  if (full_key == "method.nond.proposal_covariance_data") { emit(rep.proposalCovData); return true; }
  if (full_key == "method.nond.regression_noise_tolerance") { emit(rep.regressionNoiseTol); return true; }
  if (full_key == "method.nond.relaxation.factor_sequence") { emit(rep.relaxFactorSequence); return true; }
  if (full_key == "method.nond.scalarization_response_mapping") { emit(rep.scalarizationRespCoeffs); return true; }
  if (full_key == "method.parameter_study.final_point") { emit(rep.finalPoint); return true; }
  if (full_key == "method.parameter_study.list_of_points") { emit(rep.listOfPoints); return true; }
  if (full_key == "method.parameter_study.step_vector") { emit(rep.stepVector); return true; }
  if (full_key == "method.trust_region.initial_size") { emit(rep.trustRegionInitSize); return true; }
  if (full_key == "method.fsu_quasi_mc.primeBase") { emit(rep.primeBase); return true; }
  if (full_key == "method.fsu_quasi_mc.sequenceLeap") { emit(rep.sequenceLeap); return true; }
  if (full_key == "method.fsu_quasi_mc.sequenceStart") { emit(rep.sequenceStart); return true; }
  if (full_key == "method.nond.refinement_samples") { emit(rep.refineSamples); return true; }
  if (full_key == "method.parameter_study.steps_per_variable") { emit(rep.stepsPerVariable); return true; }
  if (full_key == "method.generating_vector.inline") { emit(rep.generatingVector); return true; }
  if (full_key == "method.generating_matrices.inline") { emit(rep.generatingMatrices); return true; }
  if (full_key == "method.nond.mlmcmc_subsampling_steps") { emit(rep.mlmcmcSubsamplingSteps); return true; }
  if (full_key == "method.nond.c3function_train.start_rank_sequence") { emit(rep.startRankSeq); return true; }
  if (full_key == "method.nond.collocation_points") {
    emit(rep.collocationPoints);
    return true;
  }
  if (full_key == "method.nond.collocation_points_sequence") {
    emit(rep.collocationPointsSeq);
    return true;
  }
  if (full_key == "method.nond.expansion_samples") {
    emit(rep.expansionSamples);
    return true;
  }
  if (full_key == "method.nond.expansion_samples_sequence") {
    emit(rep.expansionSamplesSeq);
    return true;
  }
  if (full_key == "method.nond.pilot_samples") { emit(rep.pilotSamples); return true; }
  if (full_key == "method.random_seed_sequence") { emit(rep.randomSeedSeq); return true; }
  if (full_key == "method.nond.c3function_train.start_order_sequence") { emit(rep.startOrderSeq); return true; }
  if (full_key == "method.nond.expansion_order") { emit(rep.expansionOrder); return true; }
  if (full_key == "method.nond.expansion_order_sequence") {
    emit(rep.expansionOrderSeq);
    return true;
  }
  if (full_key == "method.nond.quadrature_order") {
    emit(rep.quadratureOrder);
    return true;
  }
  if (full_key == "method.nond.quadrature_order_sequence") {
    emit(rep.quadratureOrderSeq);
    return true;
  }
  if (full_key == "method.nond.sparse_grid_level") {
    emit(rep.sparseGridLevel);
    return true;
  }
  if (full_key == "method.nond.sparse_grid_level_sequence") {
    emit(rep.sparseGridLevelSeq);
    return true;
  }
  if (full_key == "method.nond.tensor_grid_order") { emit(rep.tensorGridOrder); return true; }
  if (full_key == "method.partitions") { emit(rep.varPartitions); return true; }
  if (full_key == "method.nond.gen_reliability_levels") { emit(rep.genReliabilityLevels); return true; }
  if (full_key == "method.nond.probability_levels") { emit(rep.probabilityLevels); return true; }
  if (full_key == "method.nond.reliability_levels") { emit(rep.reliabilityLevels); return true; }
  if (full_key == "method.nond.response_levels") { emit(rep.responseLevels); return true; }
  if (full_key == "method.coliny.misc_options") { emit(rep.miscOptions); return true; }
  if (full_key == "method.hybrid.method_names") { emit(rep.hybridMethodNames); return true; }
  if (full_key == "method.hybrid.method_pointers") { emit(rep.hybridMethodPointers); return true; }
  if (full_key == "method.hybrid.model_pointers") { emit(rep.hybridModelPointers); return true; }
  if (full_key == "method.advanced_options_file") { emit(rep.advancedOptionsFilename); return true; }
  if (full_key == "method.asynch_pattern_search.merit_function") { emit(rep.meritFunction); return true; }
  if (full_key == "method.batch_selection") { emit(rep.batchSelectionType); return true; }
  if (full_key == "method.class_path_and_name") { emit(rep.moduleAndClassName); return true; }
  if (full_key == "method.coliny.beta_solver_name") { emit(rep.betaSolverName); return true; }
  if (full_key == "method.coliny.division") { emit(rep.boxDivision); return true; }
  if (full_key == "method.coliny.exploratory_moves") { emit(rep.exploratoryMoves); return true; }
  if (full_key == "method.coliny.pattern_basis") { emit(rep.patternBasis); return true; }
  if (full_key == "method.crossover_type") { emit(rep.crossoverType); return true; }
  if (full_key == "method.dl_solver.dlDetails") { emit(rep.dlDetails); return true; }
  if (full_key == "method.export_approx_points_file") { emit(rep.exportApproxPtsFile); return true; }
  if (full_key == "method.fitness_metric") { emit(rep.fitnessMetricType); return true; }
  if (full_key == "method.fitness_type") { emit(rep.fitnessType); return true; }
  if (full_key == "method.flat_file") { emit(rep.flatFile); return true; }
  if (full_key == "method.hybrid.global_method_name") { emit(rep.hybridGlobalMethodName); return true; }
  if (full_key == "method.hybrid.global_method_pointer") { emit(rep.hybridGlobalMethodPointer); return true; }
  if (full_key == "method.hybrid.global_model_pointer") { emit(rep.hybridGlobalModelPointer); return true; }
  if (full_key == "method.hybrid.local_method_name") { emit(rep.hybridLocalMethodName); return true; }
  if (full_key == "method.hybrid.local_method_pointer") { emit(rep.hybridLocalMethodPointer); return true; }
  if (full_key == "method.hybrid.local_model_pointer") { emit(rep.hybridLocalModelPointer); return true; }
  if (full_key == "method.id") { emit(rep.idMethod); return true; }
  if (full_key == "method.import_approx_points_file") { emit(rep.importApproxPtsFile); return true; }
  if (full_key == "method.import_build_points_file") { emit(rep.importBuildPtsFile); return true; }
  if (full_key == "method.import_points_file") { emit(rep.importPtsFile); return true; }
  if (full_key == "method.import_candidate_points_file") { emit(rep.importCandPtsFile); return true; }
  if (full_key == "method.import_prediction_configs") { emit(rep.importPredConfigs); return true; }
  if (full_key == "method.initialization_type") { emit(rep.initializationType); return true; }
  if (full_key == "method.jega.convergence_type") { emit(rep.convergenceType); return true; }
  if (full_key == "method.jega.niching_type") { emit(rep.nichingType); return true; }
  if (full_key == "method.jega.postprocessor_type") { emit(rep.postProcessorType); return true; }
  if (full_key == "method.lipschitz") { emit(rep.lipschitzType); return true; }
  if (full_key == "method.log_file") { emit(rep.logFile); return true; }
  if (full_key == "method.low_fidelity_model_pointer") { emit(rep.lowFidModelPointer); return true; }
  if (full_key == "method.mesh_adaptive_search.display_format") { emit(rep.displayFormat); return true; }
  if (full_key == "method.mesh_adaptive_search.history_file") { emit(rep.historyFile); return true; }
  if (full_key == "method.mesh_adaptive_search.use_surrogate") { emit(rep.useSurrogate); return true; }
  if (full_key == "method.model_export_prefix") { emit(rep.modelExportPrefix); return true; }
  if (full_key == "method.model_pointer") { emit(rep.modelPointer); return true; }
  if (full_key == "method.mutation_type") { emit(rep.mutationType); return true; }
  if (full_key == "method.nond.data_dist_cov_type") { emit(rep.dataDistCovInputType); return true; }
  if (full_key == "method.nond.data_dist_filename") { emit(rep.dataDistFile); return true; }
  if (full_key == "method.nond.data_dist_type") { emit(rep.dataDistType); return true; }
  if (full_key == "method.nond.dili_hessian_type") { emit(rep.diliHessianType); return true; }
  if (full_key == "method.nond.discrepancy_type") { emit(rep.modelDiscrepancyType); return true; }
  if (full_key == "method.nond.dr_scale_type") { emit(rep.drScaleType); return true; }
  if (full_key == "method.nond.export_corrected_model_file") { emit(rep.exportCorrModelFile); return true; }
  if (full_key == "method.nond.export_corrected_variance_file") { emit(rep.exportCorrVarFile); return true; }
  if (full_key == "method.nond.export_discrepancy_file") { emit(rep.exportDiscrepFile); return true; }
  if (full_key == "method.nond.export_expansion_file") { emit(rep.exportExpansionFile); return true; }
  if (full_key == "method.nond.export_mcmc_points_file") { emit(rep.exportMCMCPtsFile); return true; }
  if (full_key == "method.nond.import_expansion_file") { emit(rep.importExpansionFile); return true; }
  if (full_key == "method.nond.mcmc_type") { emit(rep.mcmcType); return true; }
  if (full_key == "method.nond.point_reuse") { emit(rep.pointReuse); return true; }
  if (full_key == "method.nond.posterior_density_export_file") { emit(rep.posteriorDensityExportFilename); return true; }
  if (full_key == "method.nond.posterior_samples_export_file") { emit(rep.posteriorSamplesExportFilename); return true; }
  if (full_key == "method.nond.posterior_samples_import_file") { emit(rep.posteriorSamplesImportFilename); return true; }
  if (full_key == "method.nond.proposal_covariance_filename") { emit(rep.proposalCovFile); return true; }
  if (full_key == "method.nond.proposal_covariance_input_type") { emit(rep.proposalCovInputType); return true; }
  if (full_key == "method.nond.proposal_covariance_type") { emit(rep.proposalCovType); return true; }
  if (full_key == "method.nond.reliability_integration") { emit(rep.reliabilityIntegration); return true; }
  if (full_key == "method.optpp.search_method") { emit(rep.searchMethod); return true; }
  if (full_key == "method.pstudy.import_file") { emit(rep.pstudyFilename); return true; }
  if (full_key == "method.random_number_generator") { emit(rep.rngName); return true; }
  if (full_key == "method.replacement_type") { emit(rep.replacementType); return true; }
  if (full_key == "method.sub_method_name") { emit(rep.subMethodName); return true; }
  if (full_key == "method.sub_method_pointer") { emit(rep.subMethodPointer); return true; }
  if (full_key == "method.sub_model_pointer") { emit(rep.subModelPointer); return true; }
  if (full_key == "method.trial_type") { emit(rep.trialType); return true; }
  if (full_key == "method.generating_vector.file") { emit(rep.generatingVectorFileName); return true; }
  if (full_key == "method.generating_matrices.file") { emit(rep.generatingMatricesFileName); return true; }
  if (full_key == "method.asynch_pattern_search.constraint_penalty") { emit(rep.constrPenalty); return true; }
  if (full_key == "method.asynch_pattern_search.contraction_factor") { emit(rep.contractStepLength); return true; }
  if (full_key == "method.asynch_pattern_search.initial_delta") { emit(rep.initStepLength); return true; }
  if (full_key == "method.asynch_pattern_search.smoothing_factor") { emit(rep.smoothFactor); return true; }
  if (full_key == "method.asynch_pattern_search.solution_target") { emit(rep.solnTarget); return true; }
  if (full_key == "method.coliny.contraction_factor") { emit(rep.contractFactor); return true; }
  if (full_key == "method.coliny.global_balance_parameter") { emit(rep.globalBalanceParam); return true; }
  if (full_key == "method.coliny.initial_delta") { emit(rep.initDelta); return true; }
  if (full_key == "method.coliny.local_balance_parameter") { emit(rep.localBalanceParam); return true; }
  if (full_key == "method.coliny.max_boxsize_limit") { emit(rep.maxBoxSize); return true; }
  if (full_key == "method.coliny.variable_tolerance") { emit(rep.threshDelta); return true; }
  if (full_key == "method.confidence_level") { emit(rep.wilksConfidenceLevel); return true; }
  if (full_key == "method.constraint_penalty") { emit(rep.constraintPenalty); return true; }
  if (full_key == "method.constraint_tolerance") { emit(rep.constraintTolerance); return true; }
  if (full_key == "method.convergence_tolerance") { emit(rep.convergenceTolerance); return true; }
  if (full_key == "method.crossover_rate") { emit(rep.crossoverRate); return true; }
  if (full_key == "method.dream.gr_threshold") { emit(rep.grThreshold); return true; }
  if (full_key == "method.function_precision") { emit(rep.functionPrecision); return true; }
  if (full_key == "method.gradient_tolerance") { emit(rep.gradientTolerance); return true; }
  if (full_key == "method.hybrid.local_search_probability") { emit(rep.hybridLSProb); return true; }
  if (full_key == "method.jega.fitness_limit") { emit(rep.fitnessLimit); return true; }
  if (full_key == "method.jega.percent_change") { emit(rep.convergenceTolerance); return true; }
  if (full_key == "method.jega.shrinkage_percentage") { emit(rep.shrinkagePercent); return true; }
  if (full_key == "method.mesh_adaptive_search.initial_delta") { emit(rep.initMeshSize); return true; }
  if (full_key == "method.mesh_adaptive_search.variable_neighborhood_search") { emit(rep.vns); return true; }
  if (full_key == "method.mesh_adaptive_search.variable_tolerance") { emit(rep.minMeshSize); return true; }
  if (full_key == "method.min_boxsize_limit") { emit(rep.minBoxSize); return true; }
  if (full_key == "method.mutation_rate") { emit(rep.mutationRate); return true; }
  if (full_key == "method.mutation_scale") { emit(rep.mutationScale); return true; }
  if (full_key == "method.nl2sol.absolute_conv_tol") { emit(rep.absConvTol); return true; }
  if (full_key == "method.nl2sol.false_conv_tol") { emit(rep.falseConvTol); return true; }
  if (full_key == "method.nl2sol.initial_trust_radius") { emit(rep.initTRRadius); return true; }
  if (full_key == "method.nl2sol.singular_conv_tol") { emit(rep.singConvTol); return true; }
  if (full_key == "method.nl2sol.singular_radius") { emit(rep.singRadius); return true; }
  if (full_key == "method.nond.am_scale") { emit(rep.amScale); return true; }
  if (full_key == "method.nond.c3function_train.solver_rounding_tolerance") { emit(rep.solverRoundingTol); return true; }
  if (full_key == "method.nond.c3function_train.solver_tolerance") { emit(rep.solverTol); return true; }
  if (full_key == "method.nond.c3function_train.stats_rounding_tolerance") { emit(rep.statsRoundingTol); return true; }
  if (full_key == "method.nond.collocation_ratio") { emit(rep.collocationRatio); return true; }
  if (full_key == "method.nond.collocation_ratio_terms_order") { emit(rep.collocRatioTermsOrder); return true; }
  if (full_key == "method.nond.dili_hess_tolerance") { emit(rep.diliHessTolerance); return true; }
  if (full_key == "method.nond.dili_lis_tolerance") { emit(rep.diliLISTolerance); return true; }
  if (full_key == "method.nond.dili_ses_abs_tol") { emit(rep.diliSesAbsTol); return true; }
  if (full_key == "method.nond.dili_ses_rel_tol") { emit(rep.diliSesRelTol); return true; }
  if (full_key == "method.nond.dr_scale") { emit(rep.drScale); return true; }
  if (full_key == "method.nond.estimator_variance_metric_norm_order") { emit(rep.estVarMetricNormOrder); return true; }
  if (full_key == "method.nond.mala_step_size") { emit(rep.malaStepSize); return true; }
  if (full_key == "method.nond.mlmcmc_greedy_resampling_factor") { emit(rep.mlmcmcGreedyResamplingFactor); return true; }
  if (full_key == "method.nond.mlmcmc_target_variance") { emit(rep.mlmcmcTargetVariance); return true; }
  if (full_key == "method.nond.multilevel_estimator_rate") { emit(rep.multilevEstimatorRate); return true; }
  if (full_key == "method.nond.rcond_tol_throttle") { emit(rep.rCondTolThrottle); return true; }
  if (full_key == "method.nond.regression_penalty") { emit(rep.regressionL2Penalty); return true; }
  if (full_key == "method.nond.relaxation.fixed_factor") { emit(rep.relaxFixedFactor); return true; }
  if (full_key == "method.nond.relaxation.recursive_factor") { emit(rep.relaxRecursiveFactor); return true; }
  if (full_key == "method.npsol.linesearch_tolerance") { emit(rep.lineSearchTolerance); return true; }
  if (full_key == "method.optpp.centering_parameter") { emit(rep.centeringParam); return true; }
  if (full_key == "method.optpp.max_step") { emit(rep.maxStep); return true; }
  if (full_key == "method.optpp.steplength_to_boundary") { emit(rep.stepLenToBoundary); return true; }
  if (full_key == "method.percent_variance_explained") { emit(rep.percentVarianceExplained); return true; }
  if (full_key == "method.prior_prop_cov_mult") { emit(rep.priorPropCovMult); return true; }
  if (full_key == "method.solution_target") { emit(rep.solnTarget); return true; }
  if (full_key == "method.ti_coverage") { emit(rep.tiCoverage); return true; }
  if (full_key == "method.ti_confidence_level") { emit(rep.tiConfidenceLevel); return true; }
  if (full_key == "method.trust_region.contract_threshold") { emit(rep.trustRegionContractTrigger); return true; }
  if (full_key == "method.trust_region.contraction_factor") { emit(rep.trustRegionContract); return true; }
  if (full_key == "method.trust_region.expand_threshold") { emit(rep.trustRegionExpandTrigger); return true; }
  if (full_key == "method.trust_region.expansion_factor") { emit(rep.trustRegionExpand); return true; }
  if (full_key == "method.trust_region.minimum_size") { emit(rep.trustRegionMinSize); return true; }
  if (full_key == "method.variable_tolerance") { emit(rep.threshStepLength); return true; }
  if (full_key == "method.vbd_drop_tolerance") { emit(rep.vbdDropTolerance); return true; }
  if (full_key == "method.verification.refinement_rate") { emit(rep.refinementRate); return true; }
  if (full_key == "method.volume_boxsize_limit") { emit(rep.volBoxSize); return true; }
  if (full_key == "method.x_conv_tol") { emit(rep.xConvTol); return true; }
  if (full_key == "method.adapt_exp_design_samples") { emit(rep.adaptExpSamples); return true; }
  if (full_key == "method.batch_size") { emit(rep.batchSize); return true; }
  if (full_key == "method.batch_size.exploration") { emit(rep.batchSizeExplore); return true; }
  if (full_key == "method.build_samples") { emit(rep.buildSamples); return true; }
  if (full_key == "method.burn_in_samples") { emit(rep.burnInSamples); return true; }
  if (full_key == "method.coliny.contract_after_failure") { emit(rep.contractAfterFail); return true; }
  if (full_key == "method.coliny.expand_after_success") { emit(rep.expandAfterSuccess); return true; }
  if (full_key == "method.coliny.mutation_range") { emit(rep.mutationRange); return true; }
  if (full_key == "method.coliny.new_solutions_generated") { emit(rep.newSolnsGenerated); return true; }
  if (full_key == "method.coliny.number_retained") { emit(rep.numberRetained); return true; }
  if (full_key == "method.coliny.total_pattern_size") { emit(rep.totalPatternSize); return true; }
  if (full_key == "method.concurrent.random_jobs") { emit(rep.concurrentRandomJobs); return true; }
  if (full_key == "method.dream.crossover_chain_pairs") { emit(rep.crossoverChainPairs); return true; }
  if (full_key == "method.dream.jump_step") { emit(rep.jumpStep); return true; }
  if (full_key == "method.dream.num_chains") { emit(rep.numChains); return true; }
  if (full_key == "method.dream.num_cr") { emit(rep.numCR); return true; }
  if (full_key == "method.evidence_samples") { emit(rep.evidenceSamples); return true; }
  if (full_key == "method.fsu_cvt.num_trials") { emit(rep.numTrials); return true; }
  if (full_key == "method.iterator_servers") { emit(rep.iteratorServers); return true; }
  if (full_key == "method.max_hifi_evaluations") { emit(rep.maxHifiEvals); return true; }
  if (full_key == "method.mesh_adaptive_search.neighbor_order") { emit(rep.neighborOrder); return true; }
  if (full_key == "method.nl2sol.covariance") { emit(rep.covarianceType); return true; }
  if (full_key == "method.nond.am_period_num_steps") { emit(rep.amPeriodNumSteps); return true; }
  if (full_key == "method.nond.am_starting_step") { emit(rep.amStartingStep); return true; }
  if (full_key == "method.nond.c3function_train.max_cross_iterations") { emit(rep.maxCrossIterations); return true; }
  if (full_key == "method.nond.chain_samples") { emit(rep.chainSamples); return true; }
  if (full_key == "method.nond.dili_adapt_end") { emit(rep.diliAdaptEnd); return true; }
  if (full_key == "method.nond.dili_adapt_interval") { emit(rep.diliAdaptInterval); return true; }
  if (full_key == "method.nond.dili_adapt_start") { emit(rep.diliAdaptStart); return true; }
  if (full_key == "method.nond.dili_initial_weight") { emit(rep.diliInitialWeight); return true; }
  if (full_key == "method.nond.dili_ses_block_size") { emit(rep.diliSesBlockSize); return true; }
  if (full_key == "method.nond.dili_ses_exp_rank") { emit(rep.diliSesExpRank); return true; }
  if (full_key == "method.nond.dili_ses_num_eigs") { emit(rep.diliSesNumEigs); return true; }
  if (full_key == "method.nond.dili_ses_overs_factor") { emit(rep.diliSesOversFactor); return true; }
  if (full_key == "method.nond.dr_num_stages") { emit(rep.drNumStages); return true; }
  if (full_key == "method.nond.mlmcmc_initial_chain_samples") { emit(rep.mlmcmcInitialChainSamples); return true; }
  if (full_key == "method.nond.prop_cov_update_period") { emit(rep.proposalCovUpdatePeriod); return true; }
  if (full_key == "method.nond.pushforward_samples") { emit(rep.numPushforwardSamples); return true; }
  if (full_key == "method.nond.samples_on_emulator") { emit(rep.samplesOnEmulator); return true; }
  if (full_key == "method.nond.surrogate_order") { emit(rep.emulatorOrder); return true; }
  if (full_key == "method.npsol.verify_level") { emit(rep.verifyLevel); return true; }
  if (full_key == "method.optpp.search_scheme_size") { emit(rep.searchSchemeSize); return true; }
  if (full_key == "method.parameter_study.num_steps") { emit(rep.numSteps); return true; }
  if (full_key == "method.population_size") { emit(rep.populationSize); return true; }
  if (full_key == "method.processors_per_iterator") { emit(rep.procsPerIterator); return true; }
  if (full_key == "method.random_seed") { emit(rep.randomSeed); return true; }
  if (full_key == "method.samples") { emit(rep.numSamples); return true; }
  if (full_key == "method.sub_sampling_period") { emit(rep.subSamplingPeriod); return true; }
  if (full_key == "method.symbols") { emit(rep.numSymbols); return true; }
  if (full_key == "method.vbd_via_sampling_num_bins") { emit(rep.vbdViaSamplingNumBins); return true; }
  if (full_key == "method.m_max") { emit(rep.log2MaxPoints); return true; }
  if (full_key == "method.t_max") { emit(rep.numberOfBits); return true; }
  if (full_key == "method.t_scramble") { emit(rep.scrambleSize); return true; }
  if (full_key == "method.iterator_scheduling") { emit(rep.iteratorScheduling); return true; }
  if (full_key == "method.ld.digitalnet.generating_matrix_scheme") { emit(rep.digitalNetGeneratingMatrixScheme); return true; }
  if (full_key == "method.ld.digitalnet.ordering") { emit(rep.digitalNetOrdering); return true; }
  if (full_key == "method.ld.rank1.generating_vector_scheme") { emit(rep.rank1GeneratingVectorScheme); return true; }
  if (full_key == "method.ld.rank1.ordering") { emit(rep.rank1Ordering); return true; }
  if (full_key == "method.nond.allocation_target") { emit(rep.allocationTarget); return true; }
  if (full_key == "method.nond.c3function_train.advancement_type") { emit(rep.c3AdvanceType); return true; }
  if (full_key == "method.nond.convergence_tolerance_target") { emit(rep.convergenceToleranceTarget); return true; }
  if (full_key == "method.nond.convergence_tolerance_type") { emit(rep.convergenceToleranceType); return true; }
  if (full_key == "method.nond.covariance_control") { emit(rep.covarianceControl); return true; }
  if (full_key == "method.nond.distribution") { emit(rep.distributionType); return true; }
  if (full_key == "method.nond.emulator") { emit(rep.emulatorType); return true; }
  if (full_key == "method.nond.ensemble_pilot_solution_mode") { emit(rep.ensemblePilotSolnMode); return true; }
  if (full_key == "method.nond.estimator_variance_metric") { emit(rep.estVarMetricType); return true; }
  if (full_key == "method.nond.expansion_basis_type") { emit(rep.expansionBasisType); return true; }
  if (full_key == "method.nond.expansion_refinement_control") { emit(rep.refinementControl); return true; }
  if (full_key == "method.nond.expansion_refinement_metric") { emit(rep.refinementMetric); return true; }
  if (full_key == "method.nond.expansion_refinement_type") { emit(rep.refinementType); return true; }
  if (full_key == "method.nond.expansion_type") { emit(rep.expansionType); return true; }
  if (full_key == "method.nond.final_moments") { emit(rep.finalMomentsType); return true; }
  if (full_key == "method.nond.final_statistics") { emit(rep.finalStatsType); return true; }
  if (full_key == "method.nond.group_throttle_type") { emit(rep.groupThrottleType); return true; }
  if (full_key == "method.nond.growth_override") { emit(rep.growthOverride); return true; }
  if (full_key == "method.nond.least_squares_regression_type") { emit(rep.lsRegressionType); return true; }
  if (full_key == "method.nond.model_discrepancy.polynomial_order") { emit(rep.polynomialOrder); return true; }
  if (full_key == "method.nond.multilevel_allocation_control") { emit(rep.multilevAllocControl); return true; }
  if (full_key == "method.nond.multilevel_discrepancy_emulation") { emit(rep.multilevDiscrepEmulation); return true; }
  if (full_key == "method.nond.nesting_override") { emit(rep.nestingOverride); return true; }
  if (full_key == "method.nond.pilot_samples.mode") { emit(rep.pilotGroupSampling); return true; }
  if (full_key == "method.nond.qoi_aggregation") { emit(rep.qoiAggregation); return true; }
  if (full_key == "method.nond.refinement_statistics_mode") { emit(rep.statsMetricMode); return true; }
  if (full_key == "method.nond.regression_type") { emit(rep.regressionType); return true; }
  if (full_key == "method.nond.response_level_target") { emit(rep.responseLevelTarget); return true; }
  if (full_key == "method.nond.response_level_target_reduce") { emit(rep.responseLevelTargetReduce); return true; }
  if (full_key == "method.nond.search_model_graphs.recursion") { emit(rep.dagRecursionType); return true; }
  if (full_key == "method.nond.search_model_graphs.selection") { emit(rep.modelSelectType); return true; }
  if (full_key == "method.optpp.merit_function") { emit(rep.meritFn); return true; }
  if (full_key == "method.output") { emit(rep.methodOutput); return true; }
  if (full_key == "method.sbl.acceptance_logic") { emit(rep.surrBasedLocalAcceptLogic); return true; }
  if (full_key == "method.sbl.constraint_relax") { emit(rep.surrBasedLocalConstrRelax); return true; }
  if (full_key == "method.sbl.merit_function") { emit(rep.surrBasedLocalMeritFn); return true; }
  if (full_key == "method.sbl.subproblem_constraints") { emit(rep.surrBasedLocalSubProbCon); return true; }
  if (full_key == "method.sbl.subproblem_objective") { emit(rep.surrBasedLocalSubProbObj); return true; }
  if (full_key == "method.synchronization") { emit(rep.evalSynchronize); return true; }
  if (full_key == "method.wilks.sided_interval") { emit(rep.wilksSidedInterval); return true; }
  if (full_key == "method.algorithm") { emit(rep.methodName); return true; }
  if (full_key == "method.export_approx_format") { emit(rep.exportApproxFormat); return true; }
  if (full_key == "method.import_approx_format") { emit(rep.importApproxFormat); return true; }
  if (full_key == "method.import_build_format") { emit(rep.importBuildFormat); return true; }
  if (full_key == "method.import_points_format") { emit(rep.importPtsFormat); return true; }
  if (full_key == "method.import_candidate_format") { emit(rep.importCandFormat); return true; }
  if (full_key == "method.import_prediction_configs_format") { emit(rep.importPredConfigFormat); return true; }
  if (full_key == "method.model_export_format") { emit(rep.modelExportFormat); return true; }
  if (full_key == "method.nond.adapted_basis.advancements") { emit(rep.adaptedBasisAdvancements); return true; }
  if (full_key == "method.nond.c3function_train.kick_order") { emit(rep.kickOrder); return true; }
  if (full_key == "method.nond.c3function_train.max_order") { emit(rep.maxOrder); return true; }
  if (full_key == "method.nond.c3function_train.start_order") { emit(rep.startOrder); return true; }
  if (full_key == "method.nond.calibrate_error_mode") { emit(rep.calibrateErrorMode); return true; }
  if (full_key == "method.nond.cross_validation.max_order_candidates") { emit(rep.maxCVOrderCandidates); return true; }
  if (full_key == "method.nond.cubature_integrand") { emit(rep.cubIntOrder); return true; }
  if (full_key == "method.nond.export_corrected_model_format") { emit(rep.exportCorrModelFormat); return true; }
  if (full_key == "method.nond.export_corrected_variance_format") { emit(rep.exportCorrVarFormat); return true; }
  if (full_key == "method.nond.export_discrep_format") { emit(rep.exportDiscrepFormat); return true; }
  if (full_key == "method.nond.export_samples_format") { emit(rep.exportSamplesFormat); return true; }
  if (full_key == "method.nond.graph_depth_limit") { emit(rep.dagDepthLimit); return true; }
  if (full_key == "method.nond.group_size_throttle") { emit(rep.groupSizeThrottle); return true; }
  if (full_key == "method.nond.integration_refinement") { emit(rep.integrationRefine); return true; }
  if (full_key == "method.nond.model_reordering") { emit(rep.modelReordering); return true; }
  if (full_key == "method.nond.numerical_solve_mode") { emit(rep.numericalSolveMode); return true; }
  if (full_key == "method.nond.opt_subproblem_solver") { emit(rep.optSubProbSolver); return true; }
  if (full_key == "method.nond.vbd_interaction_order") { emit(rep.vbdOrder); return true; }
  if (full_key == "method.order") { emit(rep.wilksOrder); return true; }
  if (full_key == "method.pstudy.import_format") { emit(rep.pstudyFileFormat); return true; }
  if (full_key == "method.sample_type") { emit(rep.sampleType); return true; }
  if (full_key == "method.soft_convergence_limit") { emit(rep.softConvLimit); return true; }
  if (full_key == "method.sub_method") { emit(rep.subMethod); return true; }
  if (full_key == "method.vbd_via_sampling_method") { emit(rep.vbdViaSamplingMethod); return true; }
  if (full_key == "method.final_solutions") { emit(rep.numFinalSolutions); return true; }
  if (full_key == "method.jega.num_cross_points") { emit(rep.numCrossPoints); return true; }
  if (full_key == "method.jega.num_designs") { emit(rep.numDesigns); return true; }
  if (full_key == "method.jega.num_generations") { emit(rep.numGenerations); return true; }
  if (full_key == "method.jega.num_offspring") { emit(rep.numOffspring); return true; }
  if (full_key == "method.jega.num_parents") { emit(rep.numParents); return true; }
  if (full_key == "method.max_function_evaluations") { emit(rep.maxFunctionEvals); return true; }
  if (full_key == "method.max_iterations") { emit(rep.maxIterations); return true; }
  if (full_key == "method.nond.c3function_train.kick_rank") { emit(rep.kickRank); return true; }
  if (full_key == "method.nond.c3function_train.max_rank") { emit(rep.maxRank); return true; }
  if (full_key == "method.nond.c3function_train.start_rank") { emit(rep.startRank); return true; }
  if (full_key == "method.nond.cross_validation.max_rank_candidates") { emit(rep.maxCVRankCandidates); return true; }
  if (full_key == "method.nond.max_refinement_iterations") { emit(rep.maxRefineIterations); return true; }
  if (full_key == "method.nond.max_solver_iterations") { emit(rep.maxSolverIterations); return true; }
  if (full_key == "method.nond.rcond_best_throttle") { emit(rep.rCondBestThrottle); return true; }
  if (full_key == "method.num_candidate_designs") { emit(rep.numCandidateDesigns); return true; }
  if (full_key == "method.num_candidates") { emit(rep.numCandidates); return true; }
  if (full_key == "method.num_prediction_configs") { emit(rep.numPredConfigs); return true; }
  if (full_key == "method.backfill") { emit(rep.backfillFlag); return true; }
  if (full_key == "method.chain_diagnostics") { emit(rep.chainDiagnostics); return true; }
  if (full_key == "method.chain_diagnostics.confidence_intervals") { emit(rep.chainDiagnosticsCI); return true; }
  if (full_key == "method.coliny.constant_penalty") { emit(rep.constantPenalty); return true; }
  if (full_key == "method.coliny.expansion") { emit(rep.expansionFlag); return true; }
  if (full_key == "method.coliny.randomize") { emit(rep.randomizeOrderFlag); return true; }
  if (full_key == "method.coliny.show_misc_options") { emit(rep.showMiscOptions); return true; }
  if (full_key == "method.derivative_usage") { emit(rep.methodUseDerivsFlag); return true; }
  if (full_key == "method.export_surrogate") { emit(rep.exportSurrogate); return true; }
  if (full_key == "method.fixed_seed") { emit(rep.fixedSeedFlag); return true; }
  if (full_key == "method.fsu_quasi_mc.fixed_sequence") { emit(rep.fixedSequenceFlag); return true; }
  if (full_key == "method.import_approx_active_only") { emit(rep.importApproxActive); return true; }
  if (full_key == "method.import_build_active_only") { emit(rep.importBuildActive); return true; }
  if (full_key == "method.import_points.active_only") { emit(rep.importPtsActive); return true; }
  if (full_key == "method.import_points.use_variable_labels") { emit(rep.importPtsUseVariableLabels); return true; }
  if (full_key == "method.laplace_approx") { emit(rep.modelEvidLaplace); return true; }
  if (full_key == "method.latinize") { emit(rep.latinizeFlag); return true; }
  if (full_key == "method.main_effects") { emit(rep.mainEffectsFlag); return true; }
  if (full_key == "method.mc_approx") { emit(rep.modelEvidMC); return true; }
  if (full_key == "method.mesh_adaptive_search.display_all_evaluations") { emit(rep.showAllEval); return true; }
  if (full_key == "method.model_evidence") { emit(rep.modelEvidence); return true; }
  if (full_key == "method.mutation_adaptive") { emit(rep.mutationAdaptive); return true; }
  if (full_key == "method.nl2sol.regression_diagnostics") { emit(rep.regressDiag); return true; }
  if (full_key == "method.nond.adapt_exp_design") { emit(rep.adaptExpDesign); return true; }
  if (full_key == "method.nond.adaptive_posterior_refinement") { emit(rep.adaptPosteriorRefine); return true; }
  if (full_key == "method.nond.allocation_target.optimization") { emit(rep.useTargetVarianceOptimizationFlag); return true; }
  if (full_key == "method.nond.c3function_train.adapt_order") { emit(rep.adaptOrder); return true; }
  if (full_key == "method.nond.c3function_train.adapt_rank") { emit(rep.adaptRank); return true; }
  if (full_key == "method.nond.cross_validation") { emit(rep.crossValidation); return true; }
  if (full_key == "method.nond.cross_validation.noise_only") { emit(rep.crossValidNoiseOnly); return true; }
  if (full_key == "method.nond.d_optimal") { emit(rep.dOptimal); return true; }
  if (full_key == "method.nond.evaluate_posterior_density") { emit(rep.evaluatePosteriorDensity); return true; }
  if (full_key == "method.nond.export_sample_sequence") { emit(rep.exportSampleSeqFlag); return true; }
  if (full_key == "method.nond.generate_posterior_samples") { emit(rep.generatePosteriorSamples); return true; }
  if (full_key == "method.nond.gpmsa_normalize") { emit(rep.gpmsaNormalize); return true; }
  if (full_key == "method.nond.logit_transform") { emit(rep.logitTransform); return true; }
  if (full_key == "method.nond.model_discrepancy") { emit(rep.calModelDiscrepancy); return true; }
  if (full_key == "method.nond.mutual_info_ksg2") { emit(rep.mutualInfoKSG2); return true; }
  if (full_key == "method.nond.normalized") { emit(rep.normalizedCoeffs); return true; }
  if (full_key == "method.nond.piecewise_basis") { emit(rep.piecewiseBasis); return true; }
  if (full_key == "method.nond.response_scaling") { emit(rep.respScalingFlag); return true; }
  if (full_key == "method.nond.standardized_space") { emit(rep.standardizedSpace); return true; }
  if (full_key == "method.nond.tensor_grid") { emit(rep.tensorGridFlag); return true; }
  if (full_key == "method.nond.truth_fixed_by_pilot") { emit(rep.truthPilotConstraint); return true; }
  if (full_key == "method.posterior_stats.kde") { emit(rep.posteriorStatsKDE); return true; }
  if (full_key == "method.posterior_stats.kl_divergence") { emit(rep.posteriorStatsKL); return true; }
  if (full_key == "method.posterior_stats.mutual_info") { emit(rep.posteriorStatsMutual); return true; }
  if (full_key == "method.principal_components") { emit(rep.pcaFlag); return true; }
  if (full_key == "method.print_each_pop") { emit(rep.printPopFlag); return true; }
  if (full_key == "method.pstudy.import_active_only") { emit(rep.pstudyFileActive); return true; }
  if (full_key == "method.quality_metrics") { emit(rep.volQualityFlag); return true; }
  if (full_key == "method.sbg.replace_points") { emit(rep.surrBasedGlobalReplacePts); return true; }
  if (full_key == "method.sbl.truth_surrogate_bypass") { emit(rep.surrBasedLocalLayerBypass); return true; }
  if (full_key == "method.scaling") { emit(rep.methodScaling); return true; }
  if (full_key == "method.speculative") { emit(rep.speculativeFlag); return true; }
  if (full_key == "method.std_regression_coeffs") { emit(rep.stdRegressionCoeffs); return true; }
  if (full_key == "method.tolerance_intervals") { emit(rep.toleranceIntervalsFlag); return true; }
  if (full_key == "method.variance_based_decomp") { emit(rep.vbdFlag); return true; }
  if (full_key == "method.wilks") { emit(rep.wilksFlag); return true; }
  if (full_key == "method.rank_1_lattice") { emit(rep.rank1LatticeFlag); return true; }
  if (full_key == "method.no_random_shift") { emit(rep.noRandomShiftFlag); return true; }
  if (full_key == "method.kuo") { emit(rep.kuo); return true; }
  if (full_key == "method.cools_kuo_nuyens") { emit(rep.cools_kuo_nuyens); return true; }
  if (full_key == "method.ordering.natural") { emit(rep.naturalOrdering); return true; }
  if (full_key == "method.ordering.radical_inverse") { emit(rep.radicalInverseOrdering); return true; }
  if (full_key == "method.digital_net") { emit(rep.digitalNetFlag); return true; }
  if (full_key == "method.no_digital_shift") { emit(rep.noDigitalShiftFlag); return true; }
  if (full_key == "method.no_scrambling") { emit(rep.noScramblingFlag); return true; }
  if (full_key == "method.most_significant_bit_first") { emit(rep.mostSignificantBitFirst); return true; }
  if (full_key == "method.least_significant_bit_first") { emit(rep.leastSignificantBitFirst); return true; }
  if (full_key == "method.joe_kuo") { emit(rep.joe_kuo); return true; }
  if (full_key == "method.sobol_order_2") { emit(rep.sobol_order_2); return true; }
  if (full_key == "method.gray_code_ordering") { emit(rep.grayCodeOrdering); return true; }
  return false;
}

template <class Emit>
bool try_emit_model_entry(const DataModelRep& rep, std::string_view full_key, Emit&& emit)
{
  if (full_key == "model.nested.primary_response_mapping") { emit(rep.primaryRespCoeffs); return true; }
  if (full_key == "model.nested.secondary_response_mapping") { emit(rep.secondaryRespCoeffs); return true; }
  if (full_key == "model.simulation.solution_level_cost") { emit(rep.solutionLevelCost); return true; }
  if (full_key == "model.surrogate.kriging_correlations") { emit(rep.krigingCorrelations); return true; }
  if (full_key == "model.refinement_samples") { emit(rep.refineSamples); return true; }
  if (full_key == "model.surrogate.function_indices") { emit(rep.surrogateFnIndices); return true; }
  if (full_key == "model.metrics") { emit(rep.diagMetrics); return true; }
  if (full_key == "model.nested.primary_variable_mapping") { emit(rep.primaryVarMaps); return true; }
  if (full_key == "model.nested.secondary_variable_mapping") { emit(rep.secondaryVarMaps); return true; }
  if (full_key == "model.surrogate.ensemble_model_pointers") { emit(rep.ensembleModelPointers); return true; }
  if (full_key == "model.advanced_options_file") { emit(rep.advancedOptionsFilename); return true; }
  if (full_key == "model.dace_method_pointer") { emit(rep.subMethodPointer); return true; }
  if (full_key == "model.id") { emit(rep.idModel); return true; }
  if (full_key == "model.interface_pointer") { emit(rep.interfacePointer); return true; }
  if (full_key == "model.nested.sub_method_pointer") { emit(rep.subMethodPointer); return true; }
  if (full_key == "model.optional_interface_responses_pointer") { emit(rep.optionalInterfRespPointer); return true; }
  if (full_key == "model.rf.propagation_model_pointer") { emit(rep.propagationModelPointer); return true; }
  if (full_key == "model.rf_data_file") { emit(rep.rfDataFileName); return true; }
  if (full_key == "model.simulation.cost_recovery_metadata") { emit(rep.costRecoveryMetadata); return true; }
  if (full_key == "model.simulation.solution_level_control") { emit(rep.solutionLevelControl); return true; }
  if (full_key == "model.surrogate.truth_model_pointer") { emit(rep.truthModelPointer); return true; }
  if (full_key == "model.surrogate.challenge_points_file") { emit(rep.importChallengePtsFile); return true; }
  if (full_key == "model.surrogate.decomp_cell_type") { emit(rep.decompCellType); return true; }
  if (full_key == "model.surrogate.export_approx_points_file") { emit(rep.exportApproxPtsFile); return true; }
  if (full_key == "model.surrogate.export_approx_variance_file") { emit(rep.exportApproxVarianceFile); return true; }
  if (full_key == "model.surrogate.import_build_points_file") { emit(rep.importBuildPtsFile); return true; }
  if (full_key == "model.surrogate.kriging_opt_method") { emit(rep.krigingOptMethod); return true; }
  if (full_key == "model.surrogate.mars_interpolation") { emit(rep.marsInterpolation); return true; }
  if (full_key == "model.surrogate.model_export_prefix") { emit(rep.modelExportPrefix); return true; }
  if (full_key == "model.surrogate.model_import_prefix") { emit(rep.modelImportPrefix); return true; }
  if (full_key == "model.surrogate.class_path_and_name") { emit(rep.moduleAndClassName); return true; }
  if (full_key == "model.surrogate.point_reuse") { emit(rep.approxPointReuse); return true; }
  if (full_key == "model.surrogate.refine_cv_metric") { emit(rep.refineCVMetric); return true; }
  if (full_key == "model.surrogate.trend_order") { emit(rep.trendOrder); return true; }
  if (full_key == "model.surrogate.type") { emit(rep.surrogateType); return true; }
  if (full_key == "model.type") { emit(rep.modelType); return true; }
  if (full_key == "model.active_subspace.cv.decrease_tolerance") { emit(rep.decreaseTolerance); return true; }
  if (full_key == "model.active_subspace.cv.relative_tolerance") { emit(rep.relTolerance); return true; }
  if (full_key == "model.active_subspace.truncation_method.energy.truncation_tolerance") { emit(rep.truncationTolerance); return true; }
  if (full_key == "model.adapted_basis.collocation_ratio") { emit(rep.adaptedBasisCollocRatio); return true; }
  if (full_key == "model.adapted_basis.truncation_tolerance") { emit(rep.adaptedBasisTruncationTolerance); return true; }
  if (full_key == "model.c3function_train.collocation_ratio") { emit(rep.collocationRatio); return true; }
  if (full_key == "model.c3function_train.solver_rounding_tolerance") { emit(rep.solverRoundingTol); return true; }
  if (full_key == "model.c3function_train.solver_tolerance") { emit(rep.solverTol); return true; }
  if (full_key == "model.c3function_train.stats_rounding_tolerance") { emit(rep.statsRoundingTol); return true; }
  if (full_key == "model.convergence_tolerance") { emit(rep.convergenceTolerance); return true; }
  if (full_key == "model.surrogate.discont_grad_thresh") { emit(rep.discontGradThresh); return true; }
  if (full_key == "model.surrogate.discont_jump_thresh") { emit(rep.discontJumpThresh); return true; }
  if (full_key == "model.surrogate.neural_network_range") { emit(rep.annRange); return true; }
  if (full_key == "model.surrogate.nugget") { emit(rep.krigingNugget); return true; }
  if (full_key == "model.surrogate.percent") { emit(rep.percentFold); return true; }
  if (full_key == "model.surrogate.regression_penalty") { emit(rep.regressionL2Penalty); return true; }
  if (full_key == "model.truncation_tolerance") { emit(rep.truncationTolerance); return true; }
  if (full_key == "model.active_subspace.bootstrap_samples") { emit(rep.numReplicates); return true; }
  if (full_key == "model.active_subspace.cv.max_rank") { emit(rep.subspaceCVMaxRank); return true; }
  if (full_key == "model.c3function_train.max_cross_iterations") { emit(rep.maxCrossIterations); return true; }
  if (full_key == "model.initial_samples") { emit(rep.initialSamples); return true; }
  if (full_key == "model.nested.iterator_servers") { emit(rep.subMethodServers); return true; }
  if (full_key == "model.nested.processors_per_iterator") { emit(rep.subMethodProcs); return true; }
  if (full_key == "model.rf.expansion_bases") { emit(rep.subspaceDimension); return true; }
  if (full_key == "model.soft_convergence_limit") { emit(rep.softConvergenceLimit); return true; }
  if (full_key == "model.subspace.dimension") { emit(rep.subspaceDimension); return true; }
  if (full_key == "model.surrogate.decomp_support_layers") { emit(rep.decompSupportLayers); return true; }
  if (full_key == "model.surrogate.folds") { emit(rep.numFolds); return true; }
  if (full_key == "model.surrogate.num_restarts") { emit(rep.numRestarts); return true; }
  if (full_key == "model.surrogate.points_total") { emit(rep.pointsTotal); return true; }
  if (full_key == "model.surrogate.refine_cv_folds") { emit(rep.refineCVFolds); return true; }
  if (full_key == "model.adapted_basis.rotation_method") { emit(rep.methodRotation); return true; }
  if (full_key == "model.c3function_train.advancement_type") { emit(rep.c3AdvanceType); return true; }
  if (full_key == "model.nested.iterator_scheduling") { emit(rep.subMethodScheduling); return true; }
  if (full_key == "model.surrogate.correction_order") { emit(rep.approxCorrectionOrder); return true; }
  if (full_key == "model.surrogate.correction_type") { emit(rep.approxCorrectionType); return true; }
  if (full_key == "model.surrogate.find_nugget") { emit(rep.krigingFindNugget); return true; }
  if (full_key == "model.surrogate.kriging_max_trials") { emit(rep.krigingMaxTrials); return true; }
  if (full_key == "model.surrogate.mars_max_bases") { emit(rep.marsMaxBases); return true; }
  if (full_key == "model.surrogate.mls_weight_function") { emit(rep.mlsWeightFunction); return true; }
  if (full_key == "model.surrogate.neural_network_nodes") { emit(rep.annNodes); return true; }
  if (full_key == "model.surrogate.neural_network_random_weight") { emit(rep.annRandomWeight); return true; }
  if (full_key == "model.surrogate.points_management") { emit(rep.pointsManagement); return true; }
  if (full_key == "model.surrogate.polynomial_order") { emit(rep.polynomialOrder); return true; }
  if (full_key == "model.surrogate.rbf_bases") { emit(rep.rbfBases); return true; }
  if (full_key == "model.surrogate.rbf_max_pts") { emit(rep.rbfMaxPts); return true; }
  if (full_key == "model.surrogate.rbf_max_subsets") { emit(rep.rbfMaxSubsets); return true; }
  if (full_key == "model.surrogate.rbf_min_partition") { emit(rep.rbfMinPartition); return true; }
  if (full_key == "model.surrogate.regression_type") { emit(rep.regressionType); return true; }
  if (full_key == "model.active_subspace.cv.id_method") { emit(rep.subspaceIdCVMethod); return true; }
  if (full_key == "model.active_subspace.normalization") { emit(rep.subspaceNormalization); return true; }
  if (full_key == "model.active_subspace.sample_type") { emit(rep.subspaceSampleType); return true; }
  if (full_key == "model.adapted_basis.expansion_order") { emit(rep.adaptedBasisExpOrder); return true; }
  if (full_key == "model.adapted_basis.sparse_grid_level") { emit(rep.adaptedBasisSparseGridLev); return true; }
  if (full_key == "model.c3function_train.cross_validation.max_order_candidates") { emit(rep.maxCVOrderCandidates); return true; }
  if (full_key == "model.c3function_train.kick_order") { emit(rep.kickOrder); return true; }
  if (full_key == "model.c3function_train.max_order") { emit(rep.maxOrder); return true; }
  if (full_key == "model.c3function_train.start_order") { emit(rep.startOrder); return true; }
  if (full_key == "model.rf.analytic_covariance") { emit(rep.analyticCovIdForm); return true; }
  if (full_key == "model.rf.expansion_form") { emit(rep.randomFieldIdForm); return true; }
  if (full_key == "model.surrogate.challenge_points_file_format") { emit(rep.importChallengeFormat); return true; }
  if (full_key == "model.surrogate.export_approx_format") { emit(rep.exportApproxFormat); return true; }
  if (full_key == "model.surrogate.export_approx_variance_format") { emit(rep.exportApproxVarianceFormat); return true; }
  if (full_key == "model.surrogate.import_build_format") { emit(rep.importBuildFormat); return true; }
  if (full_key == "model.surrogate.model_export_format") { emit(rep.modelExportFormat); return true; }
  if (full_key == "model.surrogate.model_import_format") { emit(rep.modelImportFormat); return true; }
  if (full_key == "model.c3function_train.collocation_points") { emit(rep.collocationPoints); return true; }
  if (full_key == "model.c3function_train.cross_validation.max_rank_candidates") { emit(rep.maxCVRankCandidates); return true; }
  if (full_key == "model.c3function_train.kick_rank") { emit(rep.kickRank); return true; }
  if (full_key == "model.c3function_train.max_rank") { emit(rep.maxRank); return true; }
  if (full_key == "model.c3function_train.start_rank") { emit(rep.startRank); return true; }
  if (full_key == "model.max_function_evals") { emit(rep.maxFunctionEvals); return true; }
  if (full_key == "model.max_iterations") { emit(rep.maxIterations); return true; }
  if (full_key == "model.max_solver_iterations") { emit(rep.maxSolverIterations); return true; }
  if (full_key == "model.active_subspace.build_surrogate") { emit(rep.subspaceBuildSurrogate); return true; }
  if (full_key == "model.active_subspace.cv.incremental") { emit(rep.subspaceCVIncremental); return true; }
  if (full_key == "model.active_subspace.truncation_method.bing_li") { emit(rep.subspaceIdBingLi); return true; }
  if (full_key == "model.active_subspace.truncation_method.constantine") { emit(rep.subspaceIdConstantine); return true; }
  if (full_key == "model.active_subspace.truncation_method.cv") { emit(rep.subspaceIdCV); return true; }
  if (full_key == "model.active_subspace.truncation_method.energy") { emit(rep.subspaceIdEnergy); return true; }
  if (full_key == "model.c3function_train.adapt_order") { emit(rep.adaptOrder); return true; }
  if (full_key == "model.c3function_train.adapt_rank") { emit(rep.adaptRank); return true; }
  if (full_key == "model.c3function_train.tensor_grid") { emit(rep.tensorGridFlag); return true; }
  if (full_key == "model.hierarchical_tags") { emit(rep.hierarchicalTags); return true; }
  if (full_key == "model.nested.identity_resp_map") { emit(rep.identityRespMap); return true; }
  if (full_key == "model.surrogate.auto_refine") { emit(rep.autoRefine); return true; }
  if (full_key == "model.surrogate.challenge_points_file_active") { emit(rep.importChallengeActive); return true; }
  if (full_key == "model.surrogate.challenge_use_variable_labels") { emit(rep.importChalUseVariableLabels); return true; }
  if (full_key == "model.surrogate.cross_validate") { emit(rep.crossValidateFlag); return true; }
  if (full_key == "model.surrogate.decomp_discont_detect") { emit(rep.decompDiscontDetect); return true; }
  if (full_key == "model.surrogate.derivative_usage") { emit(rep.modelUseDerivsFlag); return true; }
  if (full_key == "model.surrogate.domain_decomp") { emit(rep.domainDecomp); return true; }
  if (full_key == "model.surrogate.export_surrogate") { emit(rep.exportSurrogate); return true; }
  if (full_key == "model.surrogate.import_build_active_only") { emit(rep.importBuildActive); return true; }
  if (full_key == "model.surrogate.import_surrogate") { emit(rep.importSurrogate); return true; }
  if (full_key == "model.surrogate.import_use_variable_labels") { emit(rep.importUseVariableLabels); return true; }
  if (full_key == "model.surrogate.point_selection") { emit(rep.pointSelection); return true; }
  if (full_key == "model.surrogate.press") { emit(rep.pressFlag); return true; }
  if (full_key == "model.surrogate.response_scaling") { emit(rep.respScalingFlag); return true; }
  return false;
}

template <class Emit>
bool try_emit_variables_entry(const DataVariablesRep& rep, std::string_view full_key, Emit&& emit)
{
  if (full_key == "variables.discrete_design_set_int.adjacency_matrix") { emit(rep.discreteDesignSetIntAdj); return true; }
  if (full_key == "variables.discrete_design_set_real.adjacency_matrix") { emit(rep.discreteDesignSetRealAdj); return true; }
  if (full_key == "variables.discrete_design_set_str.adjacency_matrix") { emit(rep.discreteDesignSetStrAdj); return true; }
  if (full_key == "variables.beta_uncertain.alphas") { emit(rep.betaUncAlphas); return true; }
  if (full_key == "variables.beta_uncertain.betas") { emit(rep.betaUncBetas); return true; }
  if (full_key == "variables.beta_uncertain.initial_point") { emit(rep.betaUncVars); return true; }
  if (full_key == "variables.beta_uncertain.lower_bounds") { emit(rep.betaUncLowerBnds); return true; }
  if (full_key == "variables.beta_uncertain.upper_bounds") { emit(rep.betaUncUpperBnds); return true; }
  if (full_key == "variables.binomial_uncertain.prob_per_trial") { emit(rep.binomialUncProbPerTrial); return true; }
  if (full_key == "variables.continuous_aleatory_uncertain.initial_point") { emit(rep.continuousAleatoryUncVars); return true; }
  if (full_key == "variables.continuous_aleatory_uncertain.lower_bounds") { emit(rep.continuousAleatoryUncLowerBnds); return true; }
  if (full_key == "variables.continuous_aleatory_uncertain.upper_bounds") { emit(rep.continuousAleatoryUncUpperBnds); return true; }
  if (full_key == "variables.continuous_design.initial_point") { emit(rep.continuousDesignVars); return true; }
  if (full_key == "variables.continuous_design.lower_bounds") { emit(rep.continuousDesignLowerBnds); return true; }
  if (full_key == "variables.continuous_design.scales") { emit(rep.continuousDesignScales); return true; }
  if (full_key == "variables.continuous_design.upper_bounds") { emit(rep.continuousDesignUpperBnds); return true; }
  if (full_key == "variables.continuous_epistemic_uncertain.initial_point") { emit(rep.continuousEpistemicUncVars); return true; }
  if (full_key == "variables.continuous_epistemic_uncertain.lower_bounds") { emit(rep.continuousEpistemicUncLowerBnds); return true; }
  if (full_key == "variables.continuous_epistemic_uncertain.upper_bounds") { emit(rep.continuousEpistemicUncUpperBnds); return true; }
  if (full_key == "variables.continuous_interval_uncertain.lower_bounds") { emit(rep.continuousIntervalUncLowerBnds); return true; }
  if (full_key == "variables.continuous_interval_uncertain.upper_bounds") { emit(rep.continuousIntervalUncUpperBnds); return true; }
  if (full_key == "variables.continuous_interval_uncertain.initial_point") { emit(rep.continuousIntervalUncVars); return true; }
  if (full_key == "variables.continuous_state.initial_state") { emit(rep.continuousStateVars); return true; }
  if (full_key == "variables.continuous_state.lower_bounds") { emit(rep.continuousStateLowerBnds); return true; }
  if (full_key == "variables.continuous_state.upper_bounds") { emit(rep.continuousStateUpperBnds); return true; }
  if (full_key == "variables.discrete_aleatory_uncertain_real.initial_point") { emit(rep.discreteRealAleatoryUncVars); return true; }
  if (full_key == "variables.discrete_aleatory_uncertain_real.lower_bounds") { emit(rep.discreteRealAleatoryUncLowerBnds); return true; }
  if (full_key == "variables.discrete_aleatory_uncertain_real.upper_bounds") { emit(rep.discreteRealAleatoryUncUpperBnds); return true; }
  if (full_key == "variables.discrete_design_set_real.initial_point") { emit(rep.discreteDesignSetRealVars); return true; }
  if (full_key == "variables.discrete_design_set_real.lower_bounds") { emit(rep.discreteDesignSetRealLowerBnds); return true; }
  if (full_key == "variables.discrete_design_set_real.upper_bounds") { emit(rep.discreteDesignSetRealUpperBnds); return true; }
  if (full_key == "variables.discrete_epistemic_uncertain_real.initial_point") { emit(rep.discreteRealEpistemicUncVars); return true; }
  if (full_key == "variables.discrete_epistemic_uncertain_real.lower_bounds") { emit(rep.discreteRealEpistemicUncLowerBnds); return true; }
  if (full_key == "variables.discrete_epistemic_uncertain_real.upper_bounds") { emit(rep.discreteRealEpistemicUncUpperBnds); return true; }
  if (full_key == "variables.discrete_state_set_real.initial_state") { emit(rep.discreteStateSetRealVars); return true; }
  if (full_key == "variables.discrete_state_set_real.lower_bounds") { emit(rep.discreteStateSetRealLowerBnds); return true; }
  if (full_key == "variables.discrete_state_set_real.upper_bounds") { emit(rep.discreteStateSetRealUpperBnds); return true; }
  if (full_key == "variables.discrete_uncertain_set_real.lower_bounds") { emit(rep.discreteUncSetRealLowerBnds); return true; }
  if (full_key == "variables.discrete_uncertain_set_real.upper_bounds") { emit(rep.discreteUncSetRealUpperBnds); return true; }
  if (full_key == "variables.discrete_uncertain_set_real.initial_point") { emit(rep.discreteUncSetRealVars); return true; }
  if (full_key == "variables.exponential_uncertain.betas") { emit(rep.exponentialUncBetas); return true; }
  if (full_key == "variables.exponential_uncertain.lower_bounds") { emit(rep.exponentialUncLowerBnds); return true; }
  if (full_key == "variables.exponential_uncertain.upper_bounds") { emit(rep.exponentialUncUpperBnds); return true; }
  if (full_key == "variables.exponential_uncertain.initial_point") { emit(rep.exponentialUncVars); return true; }
  if (full_key == "variables.frechet_uncertain.alphas") { emit(rep.frechetUncAlphas); return true; }
  if (full_key == "variables.frechet_uncertain.betas") { emit(rep.frechetUncBetas); return true; }
  if (full_key == "variables.frechet_uncertain.lower_bounds") { emit(rep.frechetUncLowerBnds); return true; }
  if (full_key == "variables.frechet_uncertain.upper_bounds") { emit(rep.frechetUncUpperBnds); return true; }
  if (full_key == "variables.frechet_uncertain.initial_point") { emit(rep.frechetUncVars); return true; }
  if (full_key == "variables.gamma_uncertain.alphas") { emit(rep.gammaUncAlphas); return true; }
  if (full_key == "variables.gamma_uncertain.betas") { emit(rep.gammaUncBetas); return true; }
  if (full_key == "variables.gamma_uncertain.lower_bounds") { emit(rep.gammaUncLowerBnds); return true; }
  if (full_key == "variables.gamma_uncertain.upper_bounds") { emit(rep.gammaUncUpperBnds); return true; }
  if (full_key == "variables.gamma_uncertain.initial_point") { emit(rep.gammaUncVars); return true; }
  if (full_key == "variables.geometric_uncertain.prob_per_trial") { emit(rep.geometricUncProbPerTrial); return true; }
  if (full_key == "variables.gumbel_uncertain.alphas") { emit(rep.gumbelUncAlphas); return true; }
  if (full_key == "variables.gumbel_uncertain.betas") { emit(rep.gumbelUncBetas); return true; }
  if (full_key == "variables.gumbel_uncertain.lower_bounds") { emit(rep.gumbelUncLowerBnds); return true; }
  if (full_key == "variables.gumbel_uncertain.upper_bounds") { emit(rep.gumbelUncUpperBnds); return true; }
  if (full_key == "variables.gumbel_uncertain.initial_point") { emit(rep.gumbelUncVars); return true; }
  if (full_key == "variables.histogram_bin_uncertain.lower_bounds") { emit(rep.histogramBinUncLowerBnds); return true; }
  if (full_key == "variables.histogram_bin_uncertain.upper_bounds") { emit(rep.histogramBinUncUpperBnds); return true; }
  if (full_key == "variables.histogram_bin_uncertain.initial_point") { emit(rep.histogramBinUncVars); return true; }
  if (full_key == "variables.histogram_uncertain.point_real.lower_bounds") { emit(rep.histogramPointRealUncLowerBnds); return true; }
  if (full_key == "variables.histogram_uncertain.point_real.upper_bounds") { emit(rep.histogramPointRealUncUpperBnds); return true; }
  if (full_key == "variables.histogram_uncertain.point_real.initial_point") { emit(rep.histogramPointRealUncVars); return true; }
  if (full_key == "variables.linear_equality_constraints") { emit(rep.linearEqConstraintCoeffs); return true; }
  if (full_key == "variables.linear_equality_scales") { emit(rep.linearEqScales); return true; }
  if (full_key == "variables.linear_equality_targets") { emit(rep.linearEqTargets); return true; }
  if (full_key == "variables.linear_inequality_constraints") { emit(rep.linearIneqConstraintCoeffs); return true; }
  if (full_key == "variables.linear_inequality_lower_bounds") { emit(rep.linearIneqLowerBnds); return true; }
  if (full_key == "variables.linear_inequality_scales") { emit(rep.linearIneqScales); return true; }
  if (full_key == "variables.linear_inequality_upper_bounds") { emit(rep.linearIneqUpperBnds); return true; }
  if (full_key == "variables.lognormal_uncertain.error_factors") { emit(rep.lognormalUncErrFacts); return true; }
  if (full_key == "variables.lognormal_uncertain.inferred_upper_bounds") { emit(rep.lognormalUncInferredUpperBnds); return true; }
  if (full_key == "variables.lognormal_uncertain.lambdas") { emit(rep.lognormalUncLambdas); return true; }
  if (full_key == "variables.lognormal_uncertain.initial_point") { emit(rep.lognormalUncVars); return true; }
  if (full_key == "variables.lognormal_uncertain.lower_bounds") { emit(rep.lognormalUncLowerBnds); return true; }
  if (full_key == "variables.lognormal_uncertain.means") { emit(rep.lognormalUncMeans); return true; }
  if (full_key == "variables.lognormal_uncertain.std_deviations") { emit(rep.lognormalUncStdDevs); return true; }
  if (full_key == "variables.lognormal_uncertain.upper_bounds") { emit(rep.lognormalUncUpperBnds); return true; }
  if (full_key == "variables.lognormal_uncertain.zetas") { emit(rep.lognormalUncZetas); return true; }
  if (full_key == "variables.loguniform_uncertain.initial_point") { emit(rep.loguniformUncVars); return true; }
  if (full_key == "variables.loguniform_uncertain.lower_bounds") { emit(rep.loguniformUncLowerBnds); return true; }
  if (full_key == "variables.loguniform_uncertain.upper_bounds") { emit(rep.loguniformUncUpperBnds); return true; }
  if (full_key == "variables.negative_binomial_uncertain.prob_per_trial") { emit(rep.negBinomialUncProbPerTrial); return true; }
  if (full_key == "variables.normal_uncertain.inferred_lower_bounds") { emit(rep.normalUncInferredLowerBnds); return true; }
  if (full_key == "variables.normal_uncertain.inferred_upper_bounds") { emit(rep.normalUncInferredUpperBnds); return true; }
  if (full_key == "variables.normal_uncertain.initial_point") { emit(rep.normalUncVars); return true; }
  if (full_key == "variables.normal_uncertain.lower_bounds") { emit(rep.normalUncLowerBnds); return true; }
  if (full_key == "variables.normal_uncertain.means") { emit(rep.normalUncMeans); return true; }
  if (full_key == "variables.normal_uncertain.std_deviations") { emit(rep.normalUncStdDevs); return true; }
  if (full_key == "variables.normal_uncertain.upper_bounds") { emit(rep.normalUncUpperBnds); return true; }
  if (full_key == "variables.poisson_uncertain.lambdas") { emit(rep.poissonUncLambdas); return true; }
  if (full_key == "variables.triangular_uncertain.initial_point") { emit(rep.triangularUncVars); return true; }
  if (full_key == "variables.triangular_uncertain.lower_bounds") { emit(rep.triangularUncLowerBnds); return true; }
  if (full_key == "variables.triangular_uncertain.modes") { emit(rep.triangularUncModes); return true; }
  if (full_key == "variables.triangular_uncertain.upper_bounds") { emit(rep.triangularUncUpperBnds); return true; }
  if (full_key == "variables.uniform_uncertain.initial_point") { emit(rep.uniformUncVars); return true; }
  if (full_key == "variables.uniform_uncertain.lower_bounds") { emit(rep.uniformUncLowerBnds); return true; }
  if (full_key == "variables.uniform_uncertain.upper_bounds") { emit(rep.uniformUncUpperBnds); return true; }
  if (full_key == "variables.weibull_uncertain.alphas") { emit(rep.weibullUncAlphas); return true; }
  if (full_key == "variables.weibull_uncertain.betas") { emit(rep.weibullUncBetas); return true; }
  if (full_key == "variables.weibull_uncertain.lower_bounds") { emit(rep.weibullUncLowerBnds); return true; }
  if (full_key == "variables.weibull_uncertain.upper_bounds") { emit(rep.weibullUncUpperBnds); return true; }
  if (full_key == "variables.weibull_uncertain.initial_point") { emit(rep.weibullUncVars); return true; }
  if (full_key == "variables.binomial_uncertain.num_trials") { emit(rep.binomialUncNumTrials); return true; }
  if (full_key == "variables.binomial_uncertain.lower_bounds") { emit(rep.binomialUncLowerBnds); return true; }
  if (full_key == "variables.binomial_uncertain.upper_bounds") { emit(rep.binomialUncUpperBnds); return true; }
  if (full_key == "variables.binomial_uncertain.initial_point") { emit(rep.binomialUncVars); return true; }
  if (full_key == "variables.discrete_aleatory_uncertain_int.initial_point") { emit(rep.discreteIntAleatoryUncVars); return true; }
  if (full_key == "variables.discrete_aleatory_uncertain_int.lower_bounds") { emit(rep.discreteIntAleatoryUncLowerBnds); return true; }
  if (full_key == "variables.discrete_aleatory_uncertain_int.upper_bounds") { emit(rep.discreteIntAleatoryUncUpperBnds); return true; }
  if (full_key == "variables.discrete_design_range.initial_point") { emit(rep.discreteDesignRangeVars); return true; }
  if (full_key == "variables.discrete_design_range.lower_bounds") { emit(rep.discreteDesignRangeLowerBnds); return true; }
  if (full_key == "variables.discrete_design_range.upper_bounds") { emit(rep.discreteDesignRangeUpperBnds); return true; }
  if (full_key == "variables.discrete_design_set_int.initial_point") { emit(rep.discreteDesignSetIntVars); return true; }
  if (full_key == "variables.discrete_design_set_int.lower_bounds") { emit(rep.discreteDesignSetIntLowerBnds); return true; }
  if (full_key == "variables.discrete_design_set_int.upper_bounds") { emit(rep.discreteDesignSetIntUpperBnds); return true; }
  if (full_key == "variables.discrete_epistemic_uncertain_int.initial_point") { emit(rep.discreteIntEpistemicUncVars); return true; }
  if (full_key == "variables.discrete_epistemic_uncertain_int.lower_bounds") { emit(rep.discreteIntEpistemicUncLowerBnds); return true; }
  if (full_key == "variables.discrete_epistemic_uncertain_int.upper_bounds") { emit(rep.discreteIntEpistemicUncUpperBnds); return true; }
  if (full_key == "variables.discrete_interval_uncertain.lower_bounds") { emit(rep.discreteIntervalUncLowerBnds); return true; }
  if (full_key == "variables.discrete_interval_uncertain.upper_bounds") { emit(rep.discreteIntervalUncUpperBnds); return true; }
  if (full_key == "variables.discrete_interval_uncertain.initial_point") { emit(rep.discreteIntervalUncVars); return true; }
  if (full_key == "variables.discrete_state_range.initial_state") { emit(rep.discreteStateRangeVars); return true; }
  if (full_key == "variables.discrete_state_range.lower_bounds") { emit(rep.discreteStateRangeLowerBnds); return true; }
  if (full_key == "variables.discrete_state_range.upper_bounds") { emit(rep.discreteStateRangeUpperBnds); return true; }
  if (full_key == "variables.discrete_state_set_int.initial_state") { emit(rep.discreteStateSetIntVars); return true; }
  if (full_key == "variables.discrete_state_set_int.lower_bounds") { emit(rep.discreteStateSetIntLowerBnds); return true; }
  if (full_key == "variables.discrete_state_set_int.upper_bounds") { emit(rep.discreteStateSetIntUpperBnds); return true; }
  if (full_key == "variables.discrete_uncertain_set_int.lower_bounds") { emit(rep.discreteUncSetIntLowerBnds); return true; }
  if (full_key == "variables.discrete_uncertain_set_int.upper_bounds") { emit(rep.discreteUncSetIntUpperBnds); return true; }
  if (full_key == "variables.discrete_uncertain_set_int.initial_point") { emit(rep.discreteUncSetIntVars); return true; }
  if (full_key == "variables.geometric_uncertain.lower_bounds") { emit(rep.geometricUncLowerBnds); return true; }
  if (full_key == "variables.geometric_uncertain.upper_bounds") { emit(rep.geometricUncUpperBnds); return true; }
  if (full_key == "variables.geometric_uncertain.initial_point") { emit(rep.geometricUncVars); return true; }
  if (full_key == "variables.histogram_uncertain.point_int.lower_bounds") { emit(rep.histogramPointIntUncLowerBnds); return true; }
  if (full_key == "variables.histogram_uncertain.point_int.upper_bounds") { emit(rep.histogramPointIntUncUpperBnds); return true; }
  if (full_key == "variables.histogram_uncertain.point_int.initial_point") { emit(rep.histogramPointIntUncVars); return true; }
  if (full_key == "variables.hypergeometric_uncertain.lower_bounds") { emit(rep.hyperGeomUncLowerBnds); return true; }
  if (full_key == "variables.hypergeometric_uncertain.upper_bounds") { emit(rep.hyperGeomUncUpperBnds); return true; }
  if (full_key == "variables.hypergeometric_uncertain.initial_point") { emit(rep.hyperGeomUncVars); return true; }
  if (full_key == "variables.hypergeometric_uncertain.num_drawn") { emit(rep.hyperGeomUncNumDrawn); return true; }
  if (full_key == "variables.hypergeometric_uncertain.selected_population") { emit(rep.hyperGeomUncSelectedPop); return true; }
  if (full_key == "variables.hypergeometric_uncertain.total_population") { emit(rep.hyperGeomUncTotalPop); return true; }
  if (full_key == "variables.negative_binomial_uncertain.lower_bounds") { emit(rep.negBinomialUncLowerBnds); return true; }
  if (full_key == "variables.negative_binomial_uncertain.upper_bounds") { emit(rep.negBinomialUncUpperBnds); return true; }
  if (full_key == "variables.negative_binomial_uncertain.initial_point") { emit(rep.negBinomialUncVars); return true; }
  if (full_key == "variables.negative_binomial_uncertain.num_trials") { emit(rep.negBinomialUncNumTrials); return true; }
  if (full_key == "variables.poisson_uncertain.lower_bounds") { emit(rep.poissonUncLowerBnds); return true; }
  if (full_key == "variables.poisson_uncertain.upper_bounds") { emit(rep.poissonUncUpperBnds); return true; }
  if (full_key == "variables.poisson_uncertain.initial_point") { emit(rep.poissonUncVars); return true; }
  if (full_key == "variables.binomial_uncertain.categorical") { emit(rep.binomialUncCat); return true; }
  if (full_key == "variables.discrete_design_range.categorical") { emit(rep.discreteDesignRangeCat); return true; }
  if (full_key == "variables.discrete_design_set_int.categorical") { emit(rep.discreteDesignSetIntCat); return true; }
  if (full_key == "variables.discrete_design_set_real.categorical") { emit(rep.discreteDesignSetRealCat); return true; }
  if (full_key == "variables.discrete_interval_uncertain.categorical") { emit(rep.discreteIntervalUncCat); return true; }
  if (full_key == "variables.discrete_state_range.categorical") { emit(rep.discreteStateRangeCat); return true; }
  if (full_key == "variables.discrete_state_set_int.categorical") { emit(rep.discreteStateSetIntCat); return true; }
  if (full_key == "variables.discrete_state_set_real.categorical") { emit(rep.discreteStateSetRealCat); return true; }
  if (full_key == "variables.discrete_uncertain_set_int.categorical") { emit(rep.discreteUncSetIntCat); return true; }
  if (full_key == "variables.discrete_uncertain_set_real.categorical") { emit(rep.discreteUncSetRealCat); return true; }
  if (full_key == "variables.geometric_uncertain.categorical") { emit(rep.geometricUncCat); return true; }
  if (full_key == "variables.histogram_uncertain.point_int.categorical") { emit(rep.histogramUncPointIntCat); return true; }
  if (full_key == "variables.histogram_uncertain.point_real.categorical") { emit(rep.histogramUncPointRealCat); return true; }
  if (full_key == "variables.hypergeometric_uncertain.categorical") { emit(rep.hyperGeomUncCat); return true; }
  if (full_key == "variables.negative_binomial_uncertain.categorical") { emit(rep.negBinomialUncCat); return true; }
  if (full_key == "variables.poisson_uncertain.categorical") { emit(rep.poissonUncCat); return true; }
  if (full_key == "variables.uncertain.correlation_matrix") { emit(rep.uncertainCorrelations); return true; }
  if (full_key == "variables.discrete_design_set_int.values") { emit(rep.discreteDesignSetInt); return true; }
  if (full_key == "variables.discrete_state_set_int.values") { emit(rep.discreteStateSetInt); return true; }
  if (full_key == "variables.discrete_design_set_string.values") { emit(rep.discreteDesignSetStr); return true; }
  if (full_key == "variables.discrete_state_set_string.values") { emit(rep.discreteStateSetStr); return true; }
  if (full_key == "variables.discrete_design_set_real.values") { emit(rep.discreteDesignSetReal); return true; }
  if (full_key == "variables.discrete_state_set_real.values") { emit(rep.discreteStateSetReal); return true; }
  if (full_key == "variables.discrete_uncertain_set_int.values_probs") { emit(rep.discreteUncSetIntValuesProbs); return true; }
  if (full_key == "variables.histogram_uncertain.point_int_pairs") { emit(rep.histogramUncPointIntPairs); return true; }
  if (full_key == "variables.discrete_uncertain_set_string.values_probs") { emit(rep.discreteUncSetStrValuesProbs); return true; }
  if (full_key == "variables.histogram_uncertain.point_string_pairs") { emit(rep.histogramUncPointStrPairs); return true; }
  if (full_key == "variables.discrete_uncertain_set_real.values_probs") { emit(rep.discreteUncSetRealValuesProbs); return true; }
  if (full_key == "variables.histogram_uncertain.bin_pairs") { emit(rep.histogramUncBinPairs); return true; }
  if (full_key == "variables.histogram_uncertain.point_real_pairs") { emit(rep.histogramUncPointRealPairs); return true; }
  if (full_key == "variables.continuous_interval_uncertain.basic_probs") { emit(rep.continuousIntervalUncBasicProbs); return true; }
  if (full_key == "variables.discrete_interval_uncertain.basic_probs") { emit(rep.discreteIntervalUncBasicProbs); return true; }
  if (full_key == "variables.continuous_aleatory_uncertain.labels") { emit(rep.continuousAleatoryUncLabels); return true; }
  if (full_key == "variables.normal_uncertain.labels") { emit(rep.normalUncLabels); return true; }
  if (full_key == "variables.lognormal_uncertain.labels") { emit(rep.lognormalUncLabels); return true; }
  if (full_key == "variables.uniform_uncertain.labels") { emit(rep.uniformUncLabels); return true; }
  if (full_key == "variables.loguniform_uncertain.labels") { emit(rep.loguniformUncLabels); return true; }
  if (full_key == "variables.triangular_uncertain.labels") { emit(rep.triangularUncLabels); return true; }
  if (full_key == "variables.exponential_uncertain.labels") { emit(rep.exponentialUncLabels); return true; }
  if (full_key == "variables.beta_uncertain.labels") { emit(rep.betaUncLabels); return true; }
  if (full_key == "variables.gamma_uncertain.labels") { emit(rep.gammaUncLabels); return true; }
  if (full_key == "variables.gumbel_uncertain.labels") { emit(rep.gumbelUncLabels); return true; }
  if (full_key == "variables.frechet_uncertain.labels") { emit(rep.frechetUncLabels); return true; }
  if (full_key == "variables.weibull_uncertain.labels") { emit(rep.weibullUncLabels); return true; }
  if (full_key == "variables.histogram_bin_uncertain.labels") { emit(rep.histogramBinUncLabels); return true; }
  if (full_key == "variables.continuous_design.labels") { emit(rep.continuousDesignLabels); return true; }
  if (full_key == "variables.continuous_design.scale_types") { emit(rep.continuousDesignScaleTypes); return true; }
  if (full_key == "variables.continuous_epistemic_uncertain.labels") { emit(rep.continuousEpistemicUncLabels); return true; }
  if (full_key == "variables.continuous_interval_uncertain.labels") { emit(rep.continuousIntervalUncLabels); return true; }
  if (full_key == "variables.continuous_state.labels") { emit(rep.continuousStateLabels); return true; }
  if (full_key == "variables.discrete_aleatory_uncertain_int.labels") { emit(rep.discreteIntAleatoryUncLabels); return true; }
  if (full_key == "variables.poisson_uncertain.labels") { emit(rep.poissonUncLabels); return true; }
  if (full_key == "variables.binomial_uncertain.labels") { emit(rep.binomialUncLabels); return true; }
  if (full_key == "variables.negative_binomial_uncertain.labels") { emit(rep.negBinomialUncLabels); return true; }
  if (full_key == "variables.geometric_uncertain.labels") { emit(rep.geometricUncLabels); return true; }
  if (full_key == "variables.hypergeometric_uncertain.labels") { emit(rep.hyperGeomUncLabels); return true; }
  if (full_key == "variables.histogram_uncertain.point_int.labels") { emit(rep.histogramPointIntUncLabels); return true; }
  if (full_key == "variables.discrete_aleatory_uncertain_real.labels") { emit(rep.discreteRealAleatoryUncLabels); return true; }
  if (full_key == "variables.histogram_uncertain.point_real.labels") { emit(rep.histogramPointRealUncLabels); return true; }
  if (full_key == "variables.discrete_aleatory_uncertain_string.initial_point") { emit(rep.discreteStrAleatoryUncVars); return true; }
  if (full_key == "variables.discrete_aleatory_uncertain_string.labels") { emit(rep.discreteStrAleatoryUncLabels); return true; }
  if (full_key == "variables.histogram_uncertain.point_string.labels") { emit(rep.histogramPointStrUncLabels); return true; }
  if (full_key == "variables.discrete_aleatory_uncertain_string.lower_bounds") { emit(rep.discreteStrAleatoryUncLowerBnds); return true; }
  if (full_key == "variables.discrete_aleatory_uncertain_string.upper_bounds") { emit(rep.discreteStrAleatoryUncUpperBnds); return true; }
  if (full_key == "variables.discrete_design_range.labels") { emit(rep.discreteDesignRangeLabels); return true; }
  if (full_key == "variables.discrete_design_set_int.labels") { emit(rep.discreteDesignSetIntLabels); return true; }
  if (full_key == "variables.discrete_design_set_real.labels") { emit(rep.discreteDesignSetRealLabels); return true; }
  if (full_key == "variables.discrete_design_set_string.initial_point") { emit(rep.discreteDesignSetStrVars); return true; }
  if (full_key == "variables.discrete_design_set_string.labels") { emit(rep.discreteDesignSetStrLabels); return true; }
  if (full_key == "variables.discrete_design_set_string.lower_bounds") { emit(rep.discreteDesignSetStrLowerBnds); return true; }
  if (full_key == "variables.discrete_design_set_string.upper_bounds") { emit(rep.discreteDesignSetStrUpperBnds); return true; }
  if (full_key == "variables.discrete_epistemic_uncertain_int.labels") { emit(rep.discreteIntEpistemicUncLabels); return true; }
  if (full_key == "variables.discrete_interval_uncertain.labels") { emit(rep.discreteIntervalUncLabels); return true; }
  if (full_key == "variables.discrete_uncertain_set_int.labels") { emit(rep.discreteUncSetIntLabels); return true; }
  if (full_key == "variables.discrete_epistemic_uncertain_real.labels") { emit(rep.discreteRealEpistemicUncLabels); return true; }
  if (full_key == "variables.discrete_uncertain_set_real.labels") { emit(rep.discreteUncSetRealLabels); return true; }
  if (full_key == "variables.discrete_epistemic_uncertain_string.initial_point") { emit(rep.discreteStrEpistemicUncVars); return true; }
  if (full_key == "variables.discrete_epistemic_uncertain_string.labels") { emit(rep.discreteStrEpistemicUncLabels); return true; }
  if (full_key == "variables.discrete_uncertain_set_string.labels") { emit(rep.discreteUncSetStrLabels); return true; }
  if (full_key == "variables.discrete_epistemic_uncertain_string.lower_bounds") { emit(rep.discreteStrEpistemicUncLowerBnds); return true; }
  if (full_key == "variables.discrete_epistemic_uncertain_string.upper_bounds") { emit(rep.discreteStrEpistemicUncUpperBnds); return true; }
  if (full_key == "variables.discrete_state_range.labels") { emit(rep.discreteStateRangeLabels); return true; }
  if (full_key == "variables.discrete_state_set_int.labels") { emit(rep.discreteStateSetIntLabels); return true; }
  if (full_key == "variables.discrete_state_set_real.labels") { emit(rep.discreteStateSetRealLabels); return true; }
  if (full_key == "variables.discrete_state_set_string.initial_state") { emit(rep.discreteStateSetStrVars); return true; }
  if (full_key == "variables.discrete_state_set_string.labels") { emit(rep.discreteStateSetStrLabels); return true; }
  if (full_key == "variables.discrete_state_set_string.lower_bounds") { emit(rep.discreteStateSetStrLowerBnds); return true; }
  if (full_key == "variables.discrete_state_set_string.upper_bounds") { emit(rep.discreteStateSetStrUpperBnds); return true; }
  if (full_key == "variables.discrete_uncertain_set_string.lower_bounds") { emit(rep.discreteUncSetStrLowerBnds); return true; }
  if (full_key == "variables.discrete_uncertain_set_string.upper_bounds") { emit(rep.discreteUncSetStrUpperBnds); return true; }
  if (full_key == "variables.discrete_uncertain_set_string.initial_point") { emit(rep.discreteUncSetStrVars); return true; }
  if (full_key == "variables.histogram_uncertain.point_string.lower_bounds") { emit(rep.histogramPointStrUncLowerBnds); return true; }
  if (full_key == "variables.histogram_uncertain.point_string.upper_bounds") { emit(rep.histogramPointStrUncUpperBnds); return true; }
  if (full_key == "variables.histogram_uncertain.point_string.initial_point") { emit(rep.histogramPointStrUncVars); return true; }
  if (full_key == "variables.linear_equality_scale_types") { emit(rep.linearEqScaleTypes); return true; }
  if (full_key == "variables.linear_inequality_scale_types") { emit(rep.linearIneqScaleTypes); return true; }
  if (full_key == "variables.id") { emit(rep.idVariables); return true; }
  if (full_key == "variables.domain") { emit(rep.varsDomain); return true; }
  if (full_key == "variables.view") { emit(rep.varsView); return true; }
  if (full_key == "variables.aleatory_uncertain") {
    emit(rep.numNormalUncVars + rep.numLognormalUncVars + rep.numUniformUncVars +
         rep.numLoguniformUncVars + rep.numTriangularUncVars +
         rep.numExponentialUncVars + rep.numBetaUncVars + rep.numGammaUncVars +
         rep.numGumbelUncVars + rep.numFrechetUncVars + rep.numWeibullUncVars +
         rep.numHistogramBinUncVars + rep.numPoissonUncVars +
         rep.numBinomialUncVars + rep.numNegBinomialUncVars +
         rep.numGeometricUncVars + rep.numHyperGeomUncVars +
         rep.numHistogramPtIntUncVars + rep.numHistogramPtRealUncVars +
         rep.numHistogramPtStrUncVars);
    return true;
  }
  if (full_key == "variables.beta_uncertain") { emit(rep.numBetaUncVars); return true; }
  if (full_key == "variables.binomial_uncertain") { emit(rep.numBinomialUncVars); return true; }
  if (full_key == "variables.continuous") {
    emit(rep.numContinuousDesVars + rep.numNormalUncVars + rep.numLognormalUncVars +
         rep.numUniformUncVars + rep.numLoguniformUncVars +
         rep.numTriangularUncVars + rep.numExponentialUncVars +
         rep.numBetaUncVars + rep.numGammaUncVars + rep.numGumbelUncVars +
         rep.numFrechetUncVars + rep.numWeibullUncVars +
         rep.numHistogramBinUncVars + rep.numContinuousIntervalUncVars +
         rep.numContinuousStateVars);
    return true;
  }
  if (full_key == "variables.continuous_design") { emit(rep.numContinuousDesVars); return true; }
  if (full_key == "variables.continuous_interval_uncertain") { emit(rep.numContinuousIntervalUncVars); return true; }
  if (full_key == "variables.continuous_state") { emit(rep.numContinuousStateVars); return true; }
  if (full_key == "variables.design") {
    emit(rep.numContinuousDesVars + rep.numDiscreteDesRangeVars +
         rep.numDiscreteDesSetIntVars + rep.numDiscreteDesSetRealVars +
         rep.numDiscreteDesSetStrVars);
    return true;
  }
  if (full_key == "variables.discrete") {
    emit(rep.numDiscreteDesRangeVars + rep.numDiscreteDesSetIntVars +
         rep.numDiscreteDesSetRealVars + rep.numDiscreteDesSetStrVars +
         rep.numPoissonUncVars + rep.numBinomialUncVars +
         rep.numNegBinomialUncVars + rep.numGeometricUncVars +
         rep.numHyperGeomUncVars + rep.numHistogramPtIntUncVars +
         rep.numHistogramPtRealUncVars +
         rep.numHistogramPtStrUncVars + rep.numDiscreteIntervalUncVars +
         rep.numDiscreteUncSetIntVars + rep.numDiscreteUncSetRealVars +
         rep.numDiscreteUncSetStrVars + rep.numDiscreteStateRangeVars +
         rep.numDiscreteStateSetIntVars + rep.numDiscreteStateSetRealVars +
         rep.numDiscreteStateSetStrVars);
    return true;
  }
  if (full_key == "variables.discrete_design_range") { emit(rep.numDiscreteDesRangeVars); return true; }
  if (full_key == "variables.discrete_design_set_int") { emit(rep.numDiscreteDesSetIntVars); return true; }
  if (full_key == "variables.discrete_design_set_real") { emit(rep.numDiscreteDesSetRealVars); return true; }
  if (full_key == "variables.discrete_design_set_string") { emit(rep.numDiscreteDesSetStrVars); return true; }
  if (full_key == "variables.discrete_interval_uncertain") { emit(rep.numDiscreteIntervalUncVars); return true; }
  if (full_key == "variables.epistemic_uncertain") {
    emit(rep.numContinuousIntervalUncVars + rep.numDiscreteIntervalUncVars +
         rep.numDiscreteUncSetIntVars + rep.numDiscreteUncSetRealVars +
         rep.numDiscreteUncSetStrVars);
    return true;
  }
  if (full_key == "variables.discrete_state_range") { emit(rep.numDiscreteStateRangeVars); return true; }
  if (full_key == "variables.discrete_state_set_int") { emit(rep.numDiscreteStateSetIntVars); return true; }
  if (full_key == "variables.discrete_state_set_real") { emit(rep.numDiscreteStateSetRealVars); return true; }
  if (full_key == "variables.discrete_state_set_string") { emit(rep.numDiscreteStateSetStrVars); return true; }
  if (full_key == "variables.discrete_uncertain_set_int") { emit(rep.numDiscreteUncSetIntVars); return true; }
  if (full_key == "variables.discrete_uncertain_set_real") { emit(rep.numDiscreteUncSetRealVars); return true; }
  if (full_key == "variables.discrete_uncertain_set_string") { emit(rep.numDiscreteUncSetStrVars); return true; }
  if (full_key == "variables.exponential_uncertain") { emit(rep.numExponentialUncVars); return true; }
  if (full_key == "variables.frechet_uncertain") { emit(rep.numFrechetUncVars); return true; }
  if (full_key == "variables.gamma_uncertain") { emit(rep.numGammaUncVars); return true; }
  if (full_key == "variables.geometric_uncertain") { emit(rep.numGeometricUncVars); return true; }
  if (full_key == "variables.gumbel_uncertain") { emit(rep.numGumbelUncVars); return true; }
  if (full_key == "variables.histogram_uncertain.bin") { emit(rep.numHistogramBinUncVars); return true; }
  if (full_key == "variables.histogram_uncertain.point_int") { emit(rep.numHistogramPtIntUncVars); return true; }
  if (full_key == "variables.histogram_uncertain.point_real") { emit(rep.numHistogramPtRealUncVars); return true; }
  if (full_key == "variables.histogram_uncertain.point_string") { emit(rep.numHistogramPtStrUncVars); return true; }
  if (full_key == "variables.hypergeometric_uncertain") { emit(rep.numHyperGeomUncVars); return true; }
  if (full_key == "variables.lognormal_uncertain") { emit(rep.numLognormalUncVars); return true; }
  if (full_key == "variables.loguniform_uncertain") { emit(rep.numLoguniformUncVars); return true; }
  if (full_key == "variables.negative_binomial_uncertain") { emit(rep.numNegBinomialUncVars); return true; }
  if (full_key == "variables.normal_uncertain") { emit(rep.numNormalUncVars); return true; }
  if (full_key == "variables.poisson_uncertain") { emit(rep.numPoissonUncVars); return true; }
  if (full_key == "variables.state") {
    emit(rep.numContinuousStateVars + rep.numDiscreteStateRangeVars +
         rep.numDiscreteStateSetIntVars + rep.numDiscreteStateSetRealVars +
         rep.numDiscreteStateSetStrVars);
    return true;
  }
  if (full_key == "variables.total") {
    emit(rep.numContinuousDesVars + rep.numDiscreteDesRangeVars +
         rep.numDiscreteDesSetIntVars + rep.numDiscreteDesSetRealVars +
         rep.numDiscreteDesSetStrVars + rep.numNormalUncVars +
         rep.numLognormalUncVars + rep.numUniformUncVars +
         rep.numLoguniformUncVars + rep.numTriangularUncVars +
         rep.numExponentialUncVars + rep.numBetaUncVars + rep.numGammaUncVars +
         rep.numGumbelUncVars + rep.numFrechetUncVars + rep.numWeibullUncVars +
         rep.numHistogramBinUncVars + rep.numPoissonUncVars +
         rep.numBinomialUncVars + rep.numNegBinomialUncVars +
         rep.numGeometricUncVars + rep.numHyperGeomUncVars +
         rep.numHistogramPtIntUncVars + rep.numHistogramPtRealUncVars +
         rep.numHistogramPtStrUncVars + rep.numContinuousIntervalUncVars +
         rep.numDiscreteIntervalUncVars + rep.numDiscreteUncSetIntVars +
         rep.numDiscreteUncSetRealVars + rep.numDiscreteUncSetStrVars +
         rep.numContinuousStateVars + rep.numDiscreteStateRangeVars +
         rep.numDiscreteStateSetIntVars + rep.numDiscreteStateSetRealVars +
         rep.numDiscreteStateSetStrVars);
    return true;
  }
  if (full_key == "variables.triangular_uncertain") { emit(rep.numTriangularUncVars); return true; }
  if (full_key == "variables.uncertain") {
    emit(rep.numNormalUncVars + rep.numLognormalUncVars + rep.numUniformUncVars +
         rep.numLoguniformUncVars + rep.numTriangularUncVars +
         rep.numExponentialUncVars + rep.numBetaUncVars + rep.numGammaUncVars +
         rep.numGumbelUncVars + rep.numFrechetUncVars + rep.numWeibullUncVars +
         rep.numHistogramBinUncVars + rep.numPoissonUncVars +
         rep.numBinomialUncVars + rep.numNegBinomialUncVars +
         rep.numGeometricUncVars + rep.numHyperGeomUncVars +
         rep.numHistogramPtIntUncVars + rep.numHistogramPtRealUncVars +
         rep.numHistogramPtStrUncVars + rep.numContinuousIntervalUncVars +
         rep.numDiscreteIntervalUncVars + rep.numDiscreteUncSetIntVars +
         rep.numDiscreteUncSetRealVars + rep.numDiscreteUncSetStrVars);
    return true;
  }
  if (full_key == "variables.uniform_uncertain") { emit(rep.numUniformUncVars); return true; }
  if (full_key == "variables.weibull_uncertain") { emit(rep.numWeibullUncVars); return true; }
  if (full_key == "variables.uncertain.initial_point_flag") { emit(rep.uncertainVarsInitPt); return true; }
  return false;
}

template <class Emit>
bool try_emit_interface_entry(const DataInterfaceRep& rep, std::string_view full_key, Emit&& emit)
{
  if (full_key == "interface.failure_capture.recovery_fn_vals") { emit(rep.recoveryFnVals); return true; }
  if (full_key == "interface.application.analysis_drivers") { emit(rep.analysisDrivers); return true; }
  if (full_key == "interface.copyFiles") { emit(rep.copyFiles); return true; }
  if (full_key == "interface.linkFiles") { emit(rep.linkFiles); return true; }
  if (full_key == "interface.application.analysis_components") { emit(rep.analysisComponents); return true; }
  if (full_key == "interface.algebraic_mappings") { emit(rep.algebraicMappings); return true; }
  if (full_key == "interface.application.input_filter") { emit(rep.inputFilter); return true; }
  if (full_key == "interface.application.output_filter") { emit(rep.outputFilter); return true; }
  if (full_key == "interface.application.parameters_file") { emit(rep.parametersFile); return true; }
  if (full_key == "interface.application.results_file") { emit(rep.resultsFile); return true; }
  if (full_key == "interface.failure_capture.action") { emit(rep.failAction); return true; }
  if (full_key == "interface.id") { emit(rep.idInterface); return true; }
  if (full_key == "interface.plugin_library_path") { emit(rep.pluginLibraryPath); return true; }
  if (full_key == "interface.workDir") { emit(rep.workDir); return true; }
  if (full_key == "interface.nearby_evaluation_cache_tolerance") { emit(rep.nearbyEvalCacheTol); return true; }
  if (full_key == "interface.analysis_servers") { emit(rep.analysisServers); return true; }
  if (full_key == "interface.asynch_local_analysis_concurrency") { emit(rep.asynchLocalAnalysisConcurrency); return true; }
  if (full_key == "interface.asynch_local_evaluation_concurrency") { emit(rep.asynchLocalEvalConcurrency); return true; }
  if (full_key == "interface.direct.processors_per_analysis") { emit(rep.procsPerAnalysis); return true; }
  if (full_key == "interface.evaluation_servers") { emit(rep.evalServers); return true; }
  if (full_key == "interface.failure_capture.retry_limit") { emit(rep.retryLimit); return true; }
  if (full_key == "interface.processors_per_evaluation") { emit(rep.procsPerEval); return true; }
  if (full_key == "interface.analysis_scheduling") { emit(rep.analysisScheduling); return true; }
  if (full_key == "interface.evaluation_scheduling") { emit(rep.evalScheduling); return true; }
  if (full_key == "interface.local_evaluation_scheduling") { emit(rep.asynchLocalEvalScheduling); return true; }
  if (full_key == "interface.application.parameters_file_format") { emit(rep.parametersFileFormat); return true; }
  if (full_key == "interface.application.results_file_format") { emit(rep.resultsFileFormat); return true; }
  if (full_key == "interface.type") { emit(rep.interfaceType); return true; }
  if (full_key == "interface.active_set_vector") { emit(rep.activeSetVectorFlag); return true; }
  if (full_key == "interface.allow_existing_results") { emit(rep.allowExistingResultsFlag); return true; }
  if (full_key == "interface.application.file_save") { emit(rep.fileSaveFlag); return true; }
  if (full_key == "interface.application.file_tag") { emit(rep.fileTagFlag); return true; }
  if (full_key == "interface.application.verbatim") { emit(rep.verbatimFlag); return true; }
  if (full_key == "interface.asynch") { emit(rep.asynchFlag); return true; }
  if (full_key == "interface.batch") { emit(rep.batchEvalFlag); return true; }
  if (full_key == "interface.dirSave") { emit(rep.dirSave); return true; }
  if (full_key == "interface.dirTag") { emit(rep.dirTag); return true; }
  if (full_key == "interface.evaluation_cache") { emit(rep.evalCacheFlag); return true; }
  if (full_key == "interface.labeled_results") { emit(rep.dakotaResultsFileLabeled); return true; }
  if (full_key == "interface.nearby_evaluation_cache") { emit(rep.nearbyEvalCacheFlag); return true; }
  if (full_key == "interface.python.numpy") { emit(rep.numpyFlag); return true; }
  if (full_key == "interface.restart_file") { emit(rep.restartFileFlag); return true; }
  if (full_key == "interface.templateReplace") { emit(rep.templateReplace); return true; }
  if (full_key == "interface.useWorkdir") { emit(rep.useWorkdir); return true; }
  return false;
}

template <class Emit>
bool try_emit_responses_entry(const DataResponsesRep& rep, std::string_view full_key, Emit&& emit)
{
  if (full_key == "responses.fd_gradient_step_size") { emit(rep.fdGradStepSize); return true; }
  if (full_key == "responses.fd_hessian_step_size") { emit(rep.fdHessStepSize); return true; }
  if (full_key == "responses.nonlinear_equality_scales") { emit(rep.nonlinearEqScales); return true; }
  if (full_key == "responses.nonlinear_equality_targets") { emit(rep.nonlinearEqTargets); return true; }
  if (full_key == "responses.nonlinear_inequality_lower_bounds") { emit(rep.nonlinearIneqLowerBnds); return true; }
  if (full_key == "responses.nonlinear_inequality_scales") { emit(rep.nonlinearIneqScales); return true; }
  if (full_key == "responses.nonlinear_inequality_upper_bounds") { emit(rep.nonlinearIneqUpperBnds); return true; }
  if (full_key == "responses.primary_response_fn_scales") { emit(rep.primaryRespFnScales); return true; }
  if (full_key == "responses.primary_response_fn_weights") { emit(rep.primaryRespFnWeights); return true; }
  if (full_key == "responses.simulation_variance") { emit(rep.simVariance); return true; }
  if (full_key == "responses.lengths") { emit(rep.fieldLengths); return true; }
  if (full_key == "responses.num_coordinates_per_field") { emit(rep.numCoordsPerField); return true; }
  if (full_key == "responses.gradients.mixed.id_analytic") { emit(rep.idAnalyticGrads); return true; }
  if (full_key == "responses.gradients.mixed.id_numerical") { emit(rep.idNumericalGrads); return true; }
  if (full_key == "responses.hessians.mixed.id_analytic") { emit(rep.idAnalyticHessians); return true; }
  if (full_key == "responses.hessians.mixed.id_numerical") { emit(rep.idNumericalHessians); return true; }
  if (full_key == "responses.hessians.mixed.id_quasi") { emit(rep.idQuasiHessians); return true; }
  if (full_key == "responses.labels") { emit(rep.responseLabels); return true; }
  if (full_key == "responses.metadata_labels") { emit(rep.metadataLabels); return true; }
  if (full_key == "responses.nonlinear_equality_scale_types") { emit(rep.nonlinearEqScaleTypes); return true; }
  if (full_key == "responses.nonlinear_inequality_scale_types") { emit(rep.nonlinearIneqScaleTypes); return true; }
  if (full_key == "responses.primary_response_fn_scale_types") { emit(rep.primaryRespFnScaleTypes); return true; }
  if (full_key == "responses.primary_response_fn_sense") { emit(rep.primaryRespFnSense); return true; }
  if (full_key == "responses.variance_type") { emit(rep.varianceType); return true; }
  if (full_key == "responses.data_directory") { emit(rep.dataPathPrefix); return true; }
  if (full_key == "responses.fd_gradient_step_type") { emit(rep.fdGradStepType); return true; }
  if (full_key == "responses.fd_hessian_step_type") { emit(rep.fdHessStepType); return true; }
  if (full_key == "responses.gradient_type") { emit(rep.gradientType); return true; }
  if (full_key == "responses.hessian_type") { emit(rep.hessianType); return true; }
  if (full_key == "responses.id") { emit(rep.idResponses); return true; }
  if (full_key == "responses.interval_type") { emit(rep.intervalType); return true; }
  if (full_key == "responses.method_source") { emit(rep.methodSource); return true; }
  if (full_key == "responses.quasi_hessian_type") { emit(rep.quasiHessianType); return true; }
  if (full_key == "responses.scalar_data_filename") { emit(rep.scalarDataFileName); return true; }
  if (full_key == "responses.scalar_data_format") { emit(rep.scalarDataFormat); return true; }
  if (full_key == "responses.num_calibration_terms") { emit(rep.numLeastSqTerms); return true; }
  if (full_key == "responses.num_config_vars") { emit(rep.numExpConfigVars); return true; }
  if (full_key == "responses.num_experiments") { emit(rep.numExperiments); return true; }
  if (full_key == "responses.num_field_calibration_terms") { emit(rep.numFieldLeastSqTerms); return true; }
  if (full_key == "responses.num_field_nonlinear_equality_constraints") { emit(rep.numFieldNonlinearEqConstraints); return true; }
  if (full_key == "responses.num_field_nonlinear_inequality_constraints") { emit(rep.numFieldNonlinearIneqConstraints); return true; }
  if (full_key == "responses.num_field_objectives") { emit(rep.numFieldObjectiveFunctions); return true; }
  if (full_key == "responses.num_field_responses") { emit(rep.numFieldResponseFunctions); return true; }
  if (full_key == "responses.num_nonlinear_equality_constraints") { emit(rep.numNonlinearEqConstraints); return true; }
  if (full_key == "responses.num_nonlinear_inequality_constraints") { emit(rep.numNonlinearIneqConstraints); return true; }
  if (full_key == "responses.num_objective_functions") { emit(rep.numObjectiveFunctions); return true; }
  if (full_key == "responses.num_response_functions") { emit(rep.numResponseFunctions); return true; }
  if (full_key == "responses.num_scalar_calibration_terms") { emit(rep.numScalarLeastSqTerms); return true; }
  if (full_key == "responses.num_scalar_nonlinear_equality_constraints") { emit(rep.numScalarNonlinearEqConstraints); return true; }
  if (full_key == "responses.num_scalar_nonlinear_inequality_constraints") { emit(rep.numScalarNonlinearIneqConstraints); return true; }
  if (full_key == "responses.num_scalar_objectives") { emit(rep.numScalarObjectiveFunctions); return true; }
  if (full_key == "responses.num_scalar_responses") { emit(rep.numScalarResponseFunctions); return true; }
  if (full_key == "responses.calibration_data") { emit(rep.calibrationDataFlag); return true; }
  if (full_key == "responses.central_hess") { emit(rep.centralHess); return true; }
  if (full_key == "responses.ignore_bounds") { emit(rep.ignoreBounds); return true; }
  if (full_key == "responses.interpolate") { emit(rep.interpolateFlag); return true; }
  if (full_key == "responses.read_field_coordinates") { emit(rep.readFieldCoords); return true; }
  return false;
}

} // namespace Dakota::problem_desc_db_dump

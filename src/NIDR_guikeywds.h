
namespace Dakota {

/** 911 distinct keywords (plus 120 aliases) **/

static GuiKeyWord
	kw_1[3] = {
		{"active_set_vector",8,0,1,0,1655},
		{"evaluation_cache",8,0,2,0,1657},
		{"restart_file",8,0,3,0,1659}
		},
	kw_2[1] = {
		{"processors_per_analysis",9,0,1,0,1639,0,0.,0.,0.,0,"{Number of processors per analysis} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicDF"}
		},
	kw_3[4] = {
		{"abort",8,0,1,1,1645,0,0.,0.,0.,0,"@[CHOOSE failure mitigation]"},
		{"continuation",8,0,1,1,1651},
		{"recover",14,0,1,1,1649},
		{"retry",9,0,1,1,1647}
		},
	kw_4[2] = {
		{"copy",8,0,1,0,1633,0,0.,0.,0.,0,"{Copy template files} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"replace",8,0,2,0,1635,0,0.,0.,0.,0,"{Replace existing files} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"}
		},
	kw_5[7] = {
		{"dir_save",0,0,3,0,1626},
		{"dir_tag",0,0,2,0,1624},
		{"directory_save",8,0,3,0,1627,0,0.,0.,0.,0,"{Save work directory} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"directory_tag",8,0,2,0,1625,0,0.,0.,0.,0,"{Tag work directory} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"named",11,0,1,0,1623,0,0.,0.,0.,0,"{Name of work directory} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"template_directory",11,2,4,0,1629,kw_4,0.,0.,0.,0,"{Template directory} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"template_files",15,2,4,0,1631,kw_4,0.,0.,0.,0,"{Template files} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"}
		},
	kw_6[8] = {
		{"allow_existing_results",8,0,3,0,1611,0,0.,0.,0.,0,"{Allow existing results files} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"aprepro",8,0,5,0,1615,0,0.,0.,0.,0,"{Aprepro parameters file format} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"file_save",8,0,7,0,1619,0,0.,0.,0.,0,"{Parameters and results file saving} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"file_tag",8,0,6,0,1617,0,0.,0.,0.,0,"{Parameters and results file tagging} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"parameters_file",11,0,1,0,1607,0,0.,0.,0.,0,"{Parameters file name} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"results_file",11,0,2,0,1609,0,0.,0.,0.,0,"{Results file name} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"verbatim",8,0,4,0,1613,0,0.,0.,0.,0,"{Verbatim driver/filter invocation syntax} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"work_directory",8,7,8,0,1621,kw_5,0.,0.,0.,0,"{Create work directory} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"}
		},
	kw_7[9] = {
		{"analysis_components",15,0,1,0,1597,0,0.,0.,0.,0,"{Additional identifiers for use by the analysis_drivers} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"deactivate",8,3,6,0,1653,kw_1,0.,0.,0.,0,"{Feature deactivation} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"direct",8,1,4,1,1637,kw_2,0.,0.,0.,0,"[CHOOSE interface type]{Direct function interface } http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicDF"},
		{"failure_capture",8,4,5,0,1643,kw_3,0.,0.,0.,0,"{Failure capturing} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"fork",8,8,4,1,1605,kw_6,0.,0.,0.,0,"@"},
		{"grid",8,0,4,1,1641,0,0.,0.,0.,0,"{Grid interface } http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicG"},
		{"input_filter",11,0,2,0,1599,0,0.,0.,0.,0,"{Input filter} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"output_filter",11,0,3,0,1601,0,0.,0.,0.,0,"{Output filter} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"system",8,8,4,1,1603,kw_6,0.,0.,0.,0,"{System call interface } http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"}
		},
	kw_8[4] = {
		{"analysis_concurrency",9,0,3,0,1669,0,0.,0.,0.,0,"{Asynchronous analysis concurrency} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"evaluation_concurrency",9,0,1,0,1663,0,0.,0.,0.,0,"{Asynchronous evaluation concurrency} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"local_evaluation_self_scheduling",8,0,2,0,1665,0,0.,0.,0.,0,"{Self-schedule local evals} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"local_evaluation_static_scheduling",8,0,2,0,1667,0,0.,0.,0.,0,"{Static-schedule local evals} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"}
		},
	kw_9[10] = {
		{"algebraic_mappings",11,0,2,0,1593,0,0.,0.,0.,0,"{Algebraic mappings file} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfAlgebraic"},
		{"analysis_drivers",15,9,3,0,1595,kw_7,0.,0.,0.,0,"{Analysis drivers} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"analysis_self_scheduling",8,0,8,0,1679,0,0.,0.,0.,0,"[CHOOSE analysis sched.]{Self scheduling of analyses} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"analysis_servers",9,0,7,0,1677,0,0.,0.,0.,0,"{Number of analysis servers} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"analysis_static_scheduling",8,0,8,0,1681,0,0.,0.,0.,0,"{Static scheduling of analyses} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"asynchronous",8,4,4,0,1661,kw_8,0.,0.,0.,0,"{Asynchronous interface usage} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"evaluation_self_scheduling",8,0,6,0,1673,0,0.,0.,0.,0,"[CHOOSE evaluation sched.]{Self scheduling of evaluations} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"evaluation_servers",9,0,5,0,1671,0,0.,0.,0.,0,"{Number of evaluation servers} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"evaluation_static_scheduling",8,0,6,0,1675,0,0.,0.,0.,0,"{Static scheduling of evaluations} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"id_interface",11,0,1,0,1591,0,0.,0.,0.,0,"{Interface set identifier} http://dakota.sandia.gov/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"}
		},
	kw_10[7] = {
		{"merit1",8,0,1,1,261,0,0.,0.,0.,0,"[CHOOSE merit function]"},
		{"merit1_smooth",8,0,1,1,263},
		{"merit2",8,0,1,1,265},
		{"merit2_smooth",8,0,1,1,267,0,0.,0.,0.,0,"@"},
		{"merit2_squared",8,0,1,1,269},
		{"merit_max",8,0,1,1,257},
		{"merit_max_smooth",8,0,1,1,259}
		},
	kw_11[2] = {
		{"blocking",8,0,1,1,251,0,0.,0.,0.,0,"[CHOOSE synchronization]"},
		{"nonblocking",8,0,1,1,253,0,0.,0.,0.,0,"@"}
		},
	kw_12[18] = {
		{"constraint_penalty",10,0,7,0,271,0,0.,0.,0.,0,"{Constraint penalty} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodAPPSDC"},
		{"contraction_factor",10,0,2,0,243,0,0.,0.,0.,0,"{Pattern contraction factor} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodAPPSDC"},
		{"initial_delta",10,0,1,0,241,0,0.,0.,0.,0,"{Initial offset value} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodAPPSDC"},
		{"linear_equality_constraint_matrix",14,0,14,0,403,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,16,0,407,0,0.,0.,0.,0,"{Linear equality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,17,0,409,0,0.,0.,0.,0,"{Linear equality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,15,0,405,0,0.,0.,0.,0,"{Linear equality targets} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,9,0,393,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,10,0,395,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,12,0,399,0,0.,0.,0.,0,"{Linear inequality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,13,0,401,0,0.,0.,0.,0,"{Linear inequality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,11,0,397,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"merit_function",8,7,6,0,255,kw_10,0.,0.,0.,0,"{Merit function} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodAPPSDC"},
		{"smoothing_factor",10,0,8,0,273,0,0.,0.,0.,0,"{Smoothing factor} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodAPPSDC"},
		{"solution_accuracy",2,0,4,0,246},
		{"solution_target",10,0,4,0,247,0,0.,0.,0.,0,"{Solution target} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodAPPSDC"},
		{"synchronization",8,2,5,0,249,kw_11,0.,0.,0.,0,"{Evaluation synchronization} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodAPPSDC"},
		{"threshold_delta",10,0,3,0,245,0,0.,0.,0.,0,"{Threshold for offset values} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodAPPSDC"}
		},
	kw_13[1] = {
		{"emulator_samples",9,0,1,1,859}
		},
	kw_14[2] = {
		{"adaptive",8,0,1,1,871},
		{"hastings",8,0,1,1,869}
		},
	kw_15[1] = {
		{"emulator_samples",9,0,1,0,847}
		},
	kw_16[1] = {
		{"sparse_grid_level",13,0,1,0,851}
		},
	kw_17[1] = {
		{"sparse_grid_level",13,0,1,0,855}
		},
	kw_18[4] = {
		{"gaussian_process",8,1,1,1,845,kw_15},
		{"gp",0,1,1,1,844,kw_15},
		{"pce",8,1,1,1,849,kw_16},
		{"sc",8,1,1,1,853,kw_17}
		},
	kw_19[1] = {
		{"emulator",8,4,1,0,843,kw_18}
		},
	kw_20[2] = {
		{"delayed",8,0,1,1,865},
		{"standard",8,0,1,1,863}
		},
	kw_21[2] = {
		{"mt19937",8,0,1,1,877},
		{"rnum2",8,0,1,1,879}
		},
	kw_22[10] = {
		{"gpmsa",8,1,1,1,857,kw_13},
		{"likelihood_scale",10,0,7,0,883},
		{"metropolis",8,2,3,0,867,kw_14},
		{"proposal_covariance_scale",10,0,6,0,881},
		{"queso",8,1,1,1,841,kw_19},
		{"rejection",8,2,2,0,861,kw_20},
		{"rng",8,2,5,0,875,kw_21},
		{"samples",9,0,9,0,939,0,0.,0.,0.,0,"{Number of samples} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,8,0,941,0,0.,0.,0.,0,"{Random seed} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodEG"},
		{"use_derivatives",8,0,4,0,873}
		},
	kw_23[3] = {
		{"deltas_per_variable",5,0,2,2,1128},
		{"step_vector",14,0,1,1,1127,0,0.,0.,0.,0,"{Step vector} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodPSCPS"},
		{"steps_per_variable",13,0,2,2,1129,0,0.,0.,0.,0,"{Number of steps per variable} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodPSCPS"}
		},
	kw_24[7] = {
		{"initial_delta",10,0,5,1,459,0,0.,0.,0.,0,"{Initial offset value} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"misc_options",15,0,4,0,545,0,0.,0.,0.,0,"{Specify miscellaneous options} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"seed",9,0,2,0,541,0,0.,0.,0.,0,"{Random seed for stochastic pattern search} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"show_misc_options",8,0,3,0,543,0,0.,0.,0.,0,"{Show miscellaneous options} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"solution_accuracy",2,0,1,0,538},
		{"solution_target",10,0,1,0,539,0,0.,0.,0.,0,"{Desired solution target} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"threshold_delta",10,0,6,2,461,0,0.,0.,0.,0,"{Threshold for offset values} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"}
		},
	kw_25[2] = {
		{"all_dimensions",8,0,1,1,469},
		{"major_dimension",8,0,1,1,467}
		},
	kw_26[11] = {
		{"constraint_penalty",10,0,6,0,479,0,0.,0.,0.,0,"{Constraint penalty} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDIR"},
		{"division",8,2,1,0,465,kw_25,0.,0.,0.,0,"{Box subdivision approach} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDIR"},
		{"global_balance_parameter",10,0,2,0,471,0,0.,0.,0.,0,"{Global search balancing parameter} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDIR"},
		{"local_balance_parameter",10,0,3,0,473,0,0.,0.,0.,0,"{Local search balancing parameter} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDIR"},
		{"max_boxsize_limit",10,0,4,0,475,0,0.,0.,0.,0,"{Maximum boxsize limit} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDIR"},
		{"min_boxsize_limit",10,0,5,0,477,0,0.,0.,0.,0,"{Minimum boxsize limit} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDIR"},
		{"misc_options",15,0,10,0,545,0,0.,0.,0.,0,"{Specify miscellaneous options} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"seed",9,0,8,0,541,0,0.,0.,0.,0,"{Random seed for stochastic pattern search} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"show_misc_options",8,0,9,0,543,0,0.,0.,0.,0,"{Show miscellaneous options} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"solution_accuracy",2,0,7,0,538},
		{"solution_target",10,0,7,0,539,0,0.,0.,0.,0,"{Desired solution target} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"}
		},
	kw_27[3] = {
		{"blend",8,0,1,1,515},
		{"two_point",8,0,1,1,513},
		{"uniform",8,0,1,1,517}
		},
	kw_28[2] = {
		{"linear_rank",8,0,1,1,495},
		{"merit_function",8,0,1,1,497}
		},
	kw_29[3] = {
		{"flat_file",11,0,1,1,491},
		{"simple_random",8,0,1,1,487},
		{"unique_random",8,0,1,1,489}
		},
	kw_30[2] = {
		{"mutation_range",9,0,2,0,533,0,0.,0.,0.,0,"{Mutation range} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"mutation_scale",10,0,1,0,531,0,0.,0.,0.,0,"{Mutation scale} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"}
		},
	kw_31[5] = {
		{"non_adaptive",8,0,2,0,535,0,0.,0.,0.,0,"{Non-adaptive mutation flag} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"offset_cauchy",8,2,1,1,527,kw_30},
		{"offset_normal",8,2,1,1,525,kw_30},
		{"offset_uniform",8,2,1,1,529,kw_30},
		{"replace_uniform",8,0,1,1,523}
		},
	kw_32[4] = {
		{"chc",9,0,1,1,503,0,0.,0.,0.,0,"{CHC replacement type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"elitist",9,0,1,1,505,0,0.,0.,0.,0,"{Elitist replacement type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"new_solutions_generated",9,0,2,0,507,0,0.,0.,0.,0,"{New solutions generated} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"random",9,0,1,1,501,0,0.,0.,0.,0,"{Random replacement type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"}
		},
	kw_33[14] = {
		{"constraint_penalty",10,0,9,0,537},
		{"crossover_rate",10,0,5,0,509,0,0.,0.,0.,0,"{Crossover rate} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"crossover_type",8,3,6,0,511,kw_27,0.,0.,0.,0,"{Crossover type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"fitness_type",8,2,3,0,493,kw_28,0.,0.,0.,0,"{Fitness type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"initialization_type",8,3,2,0,485,kw_29,0.,0.,0.,0,"{Initialization type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"misc_options",15,0,13,0,545,0,0.,0.,0.,0,"{Specify miscellaneous options} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"mutation_rate",10,0,7,0,519,0,0.,0.,0.,0,"{Mutation rate} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"mutation_type",8,5,8,0,521,kw_31,0.,0.,0.,0,"{Mutation type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"population_size",9,0,1,0,483,0,0.,0.,0.,0,"{Number of population members} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"replacement_type",8,4,4,0,499,kw_32,0.,0.,0.,0,"{Replacement type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"seed",9,0,11,0,541,0,0.,0.,0.,0,"{Random seed for stochastic pattern search} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"show_misc_options",8,0,12,0,543,0,0.,0.,0.,0,"{Show miscellaneous options} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"solution_accuracy",2,0,10,0,538},
		{"solution_target",10,0,10,0,539,0,0.,0.,0.,0,"{Desired solution target} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"}
		},
	kw_34[3] = {
		{"adaptive_pattern",8,0,1,1,433},
		{"basic_pattern",8,0,1,1,435},
		{"multi_step",8,0,1,1,431}
		},
	kw_35[2] = {
		{"coordinate",8,0,1,1,421},
		{"simplex",8,0,1,1,423}
		},
	kw_36[2] = {
		{"blocking",8,0,1,1,439},
		{"nonblocking",8,0,1,1,441}
		},
	kw_37[17] = {
		{"constant_penalty",8,0,1,0,413,0,0.,0.,0.,0,"{Control of dynamic penalty} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"constraint_penalty",10,0,16,0,455,0,0.,0.,0.,0,"{Constraint penalty} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"contraction_factor",10,0,15,0,453,0,0.,0.,0.,0,"{Pattern contraction factor} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"expand_after_success",9,0,3,0,417,0,0.,0.,0.,0,"{Number of consecutive improvements before expansion} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"exploratory_moves",8,3,7,0,429,kw_34,0.,0.,0.,0,"{Exploratory moves selection} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"initial_delta",10,0,13,1,459,0,0.,0.,0.,0,"{Initial offset value} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"misc_options",15,0,12,0,545,0,0.,0.,0.,0,"{Specify miscellaneous options} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"no_expansion",8,0,2,0,415,0,0.,0.,0.,0,"{No expansion flag} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"pattern_basis",8,2,4,0,419,kw_35,0.,0.,0.,0,"{Pattern basis selection} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"seed",9,0,10,0,541,0,0.,0.,0.,0,"{Random seed for stochastic pattern search} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"show_misc_options",8,0,11,0,543,0,0.,0.,0.,0,"{Show miscellaneous options} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"solution_accuracy",2,0,9,0,538},
		{"solution_target",10,0,9,0,539,0,0.,0.,0.,0,"{Desired solution target} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"stochastic",8,0,5,0,425,0,0.,0.,0.,0,"{Stochastic pattern search} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"synchronization",8,2,8,0,437,kw_36,0.,0.,0.,0,"{Evaluation synchronization} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"threshold_delta",10,0,14,2,461,0,0.,0.,0.,0,"{Threshold for offset values} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"total_pattern_size",9,0,6,0,427,0,0.,0.,0.,0,"{Total number of points in pattern} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"}
		},
	kw_38[13] = {
		{"constant_penalty",8,0,4,0,451,0,0.,0.,0.,0,"{Control of dynamic penalty} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYSW"},
		{"constraint_penalty",10,0,12,0,455,0,0.,0.,0.,0,"{Constraint penalty} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"contract_after_failure",9,0,1,0,445,0,0.,0.,0.,0,"{Number of consecutive failures before contraction} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYSW"},
		{"contraction_factor",10,0,11,0,453,0,0.,0.,0.,0,"{Pattern contraction factor} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"expand_after_success",9,0,3,0,449,0,0.,0.,0.,0,"{Number of consecutive improvements before expansion} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYSW"},
		{"initial_delta",10,0,9,1,459,0,0.,0.,0.,0,"{Initial offset value} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"misc_options",15,0,8,0,545,0,0.,0.,0.,0,"{Specify miscellaneous options} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"no_expansion",8,0,2,0,447,0,0.,0.,0.,0,"{No expansion flag} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYSW"},
		{"seed",9,0,6,0,541,0,0.,0.,0.,0,"{Random seed for stochastic pattern search} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"show_misc_options",8,0,7,0,543,0,0.,0.,0.,0,"{Show miscellaneous options} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"solution_accuracy",2,0,5,0,538},
		{"solution_target",10,0,5,0,539,0,0.,0.,0.,0,"{Desired solution target} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"threshold_delta",10,0,10,2,461,0,0.,0.,0.,0,"{Threshold for offset values} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"}
		},
	kw_39[9] = {
		{"linear_equality_constraint_matrix",14,0,14,0,403,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,16,0,407,0,0.,0.,0.,0,"{Linear equality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,17,0,409,0,0.,0.,0.,0,"{Linear equality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,15,0,405,0,0.,0.,0.,0,"{Linear equality targets} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,9,0,393,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,10,0,395,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,12,0,399,0,0.,0.,0.,0,"{Linear inequality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,13,0,401,0,0.,0.,0.,0,"{Linear inequality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,11,0,397,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"}
		},
	kw_40[1] = {
		{"drop_tolerance",10,0,1,0,907}
		},
	kw_41[14] = {
		{"box_behnken",8,0,1,1,897,0,0.,0.,0.,0,"[CHOOSE DACE type]"},
		{"central_composite",8,0,1,1,899},
		{"fixed_seed",8,0,5,0,909,0,0.,0.,0.,0,"{Fixed seed flag} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodDDACE"},
		{"grid",8,0,1,1,887},
		{"lhs",8,0,1,1,893},
		{"main_effects",8,0,2,0,901,0,0.,0.,0.,0,"{Main effects} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodDDACE"},
		{"oa_lhs",8,0,1,1,895},
		{"oas",8,0,1,1,891},
		{"quality_metrics",8,0,3,0,903,0,0.,0.,0.,0,"{Quality metrics} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodDDACE"},
		{"random",8,0,1,1,889},
		{"samples",9,0,8,0,939,0,0.,0.,0.,0,"{Number of samples} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,7,0,941,0,0.,0.,0.,0,"{Random seed} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodEG"},
		{"symbols",9,0,6,0,911,0,0.,0.,0.,0,"{Number of symbols} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodDDACE"},
		{"variance_based_decomp",8,1,4,0,905,kw_40,0.,0.,0.,0,"{Variance based decomposition} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodDDACE"}
		},
	kw_42[2] = {
		{"maximize",8,0,1,1,177},
		{"minimize",8,0,1,1,175}
		},
	kw_43[15] = {
		{"bfgs",8,0,1,1,167},
		{"frcg",8,0,1,1,163},
		{"linear_equality_constraint_matrix",14,0,7,0,403,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,9,0,407,0,0.,0.,0.,0,"{Linear equality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,10,0,409,0,0.,0.,0.,0,"{Linear equality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,8,0,405,0,0.,0.,0.,0,"{Linear equality targets} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,2,0,393,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,3,0,395,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,5,0,399,0,0.,0.,0.,0,"{Linear inequality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,6,0,401,0,0.,0.,0.,0,"{Linear inequality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,4,0,397,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"mmfd",8,0,1,1,165},
		{"optimization_type",8,2,11,0,173,kw_42,0.,0.,0.,0,"{Optimization type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodDOTDC"},
		{"slp",8,0,1,1,169},
		{"sqp",8,0,1,1,171}
		},
	kw_44[10] = {
		{"linear_equality_constraint_matrix",14,0,6,0,403,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,8,0,407,0,0.,0.,0.,0,"{Linear equality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,9,0,409,0,0.,0.,0.,0,"{Linear equality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,7,0,405,0,0.,0.,0.,0,"{Linear equality targets} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,1,0,393,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,2,0,395,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,4,0,399,0,0.,0.,0.,0,"{Linear inequality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,5,0,401,0,0.,0.,0.,0,"{Linear inequality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,3,0,397,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"optimization_type",8,2,10,0,173,kw_42,0.,0.,0.,0,"{Optimization type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodDOTDC"}
		},
	kw_45[2] = {
		{"dakota",8,0,1,1,585},
		{"surfpack",8,0,1,1,583}
		},
	kw_46[4] = {
		{"gaussian_process",8,2,1,0,581,kw_45},
		{"kriging",0,2,1,0,580,kw_45},
		{"seed",9,0,3,0,941,0,0.,0.,0.,0,"{Random seed} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodEG"},
		{"use_derivatives",8,0,2,0,587}
		},
	kw_47[3] = {
		{"grid",8,0,1,1,927,0,0.,0.,0.,0,"[CHOOSE trial type]"},
		{"halton",8,0,1,1,929},
		{"random",8,0,1,1,931,0,0.,0.,0.,0,"@"}
		},
	kw_48[1] = {
		{"drop_tolerance",10,0,1,0,921}
		},
	kw_49[8] = {
		{"fixed_seed",8,0,4,0,923,0,0.,0.,0.,0,"{Fixed seed flag} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"latinize",8,0,1,0,915,0,0.,0.,0.,0,"{Latinization of samples} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"num_trials",9,0,6,0,933,0,0.,0.,0.,0,"{Number of trials  } http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"quality_metrics",8,0,2,0,917,0,0.,0.,0.,0,"{Quality metrics} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"samples",9,0,8,0,939,0,0.,0.,0.,0,"{Number of samples} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,7,0,941,0,0.,0.,0.,0,"{Random seed} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodEG"},
		{"trial_type",8,3,5,0,925,kw_47,0.,0.,0.,0,"{Trial type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"variance_based_decomp",8,1,3,0,919,kw_48,0.,0.,0.,0,"{Variance based decomposition} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"}
		},
	kw_50[1] = {
		{"drop_tolerance",10,0,1,0,1101}
		},
	kw_51[10] = {
		{"fixed_sequence",8,0,6,0,1105,0,0.,0.,0.,0,"{Fixed sequence flag} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"halton",8,0,1,1,1091,0,0.,0.,0.,0,"[CHOOSE sequence type]"},
		{"hammersley",8,0,1,1,1093},
		{"latinize",8,0,2,0,1095,0,0.,0.,0.,0,"{Latinization of samples} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"prime_base",13,0,9,0,1111,0,0.,0.,0.,0,"{Prime bases for sequences} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"quality_metrics",8,0,3,0,1097,0,0.,0.,0.,0,"{Quality metrics} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"samples",9,0,5,0,1103,0,0.,0.,0.,0,"{Number of samples} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"sequence_leap",13,0,8,0,1109,0,0.,0.,0.,0,"{Sequence leaping indices} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"sequence_start",13,0,7,0,1107,0,0.,0.,0.,0,"{Sequence starting indices} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"variance_based_decomp",8,1,4,0,1099,kw_50,0.,0.,0.,0,"{Variance based decomposition} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"}
		},
	kw_52[2] = {
		{"complementary",8,0,1,1,803},
		{"cumulative",8,0,1,1,801}
		},
	kw_53[1] = {
		{"num_gen_reliability_levels",13,0,1,0,811,0,0.,0.,0.,0,"{Number of generalized reliability levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"}
		},
	kw_54[1] = {
		{"num_probability_levels",13,0,1,0,807,0,0.,0.,0.,0,"{Number of probability levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"}
		},
	kw_55[2] = {
		{"mt19937",8,0,1,1,815},
		{"rnum2",8,0,1,1,817}
		},
	kw_56[2] = {
		{"dakota",8,0,1,1,783},
		{"surfpack",8,0,1,1,781}
		},
	kw_57[3] = {
		{"gaussian_process",8,2,1,0,779,kw_56},
		{"kriging",0,2,1,0,778,kw_56},
		{"use_derivatives",8,0,2,0,785}
		},
	kw_58[2] = {
		{"gen_reliabilities",8,0,1,1,797},
		{"probabilities",8,0,1,1,795}
		},
	kw_59[2] = {
		{"compute",8,2,2,0,793,kw_58},
		{"num_response_levels",13,0,1,0,791}
		},
	kw_60[9] = {
		{"distribution",8,2,5,0,799,kw_52,0.,0.,0.,0,"{Distribution type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"ego",8,3,1,0,777,kw_57},
		{"gen_reliability_levels",14,1,7,0,809,kw_53,0.,0.,0.,0,"{Generalized reliability levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"lhs",8,0,1,0,787},
		{"probability_levels",14,1,6,0,805,kw_54,0.,0.,0.,0,"{Probability levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"response_levels",14,2,2,0,789,kw_59},
		{"rng",8,2,8,0,813,kw_55,0.,0.,0.,0,"{Random number generator} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"samples",9,0,4,0,939,0,0.,0.,0.,0,"{Number of samples} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,3,0,941,0,0.,0.,0.,0,"{Random seed} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodEG"}
		},
	kw_61[2] = {
		{"dakota",8,0,1,1,827},
		{"surfpack",8,0,1,1,825}
		},
	kw_62[3] = {
		{"gaussian_process",8,2,1,0,823,kw_61},
		{"kriging",0,2,1,0,822,kw_61},
		{"use_derivatives",8,0,2,0,829}
		},
	kw_63[2] = {
		{"mt19937",8,0,1,1,835},
		{"rnum2",8,0,1,1,837}
		},
	kw_64[5] = {
		{"ego",8,3,1,0,821,kw_62},
		{"lhs",8,0,1,0,831},
		{"rng",8,2,2,0,833,kw_63,0.,0.,0.,0,"{Random seed generator} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDGlobalIntervalEst"},
		{"samples",9,0,4,0,939,0,0.,0.,0.,0,"{Number of samples} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,3,0,941,0,0.,0.,0.,0,"{Random seed} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodEG"}
		},
	kw_65[2] = {
		{"complementary",8,0,1,1,1079},
		{"cumulative",8,0,1,1,1077}
		},
	kw_66[1] = {
		{"num_gen_reliability_levels",13,0,1,0,1087}
		},
	kw_67[1] = {
		{"num_probability_levels",13,0,1,0,1083}
		},
	kw_68[2] = {
		{"gen_reliabilities",8,0,1,1,1073},
		{"probabilities",8,0,1,1,1071}
		},
	kw_69[2] = {
		{"compute",8,2,2,0,1069,kw_68},
		{"num_response_levels",13,0,1,0,1067}
		},
	kw_70[2] = {
		{"mt19937",8,0,1,1,1061},
		{"rnum2",8,0,1,1,1063}
		},
	kw_71[2] = {
		{"dakota",8,0,1,0,1051},
		{"surfpack",8,0,1,0,1049}
		},
	kw_72[12] = {
		{"all_variables",8,0,2,0,1053,0,0.,0.,0.,0,"{All variables flag} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDGlobalRel"},
		{"distribution",8,2,7,0,1075,kw_65},
		{"gen_reliability_levels",14,1,9,0,1085,kw_66},
		{"probability_levels",14,1,8,0,1081,kw_67},
		{"response_levels",14,2,6,0,1065,kw_69},
		{"rng",8,2,5,0,1059,kw_70},
		{"seed",9,0,4,0,1057,0,0.,0.,0.,0,"{Refinement seed} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDLocalRel"},
		{"u_gaussian_process",8,2,1,1,1047,kw_71},
		{"u_kriging",0,0,1,1,1046},
		{"use_derivatives",8,0,3,0,1055},
		{"x_gaussian_process",8,2,1,1,1045,kw_71},
		{"x_kriging",0,2,1,1,1044,kw_71}
		},
	kw_73[2] = {
		{"gen_reliabilities",8,0,1,1,773},
		{"probabilities",8,0,1,1,771}
		},
	kw_74[2] = {
		{"compute",8,2,2,0,769,kw_73},
		{"num_response_levels",13,0,1,0,767}
		},
	kw_75[7] = {
		{"distribution",8,2,4,0,799,kw_52,0.,0.,0.,0,"{Distribution type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"gen_reliability_levels",14,1,6,0,809,kw_53,0.,0.,0.,0,"{Generalized reliability levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"probability_levels",14,1,5,0,805,kw_54,0.,0.,0.,0,"{Probability levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"response_levels",14,2,1,0,765,kw_74},
		{"rng",8,2,7,0,813,kw_55,0.,0.,0.,0,"{Random number generator} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"samples",9,0,3,0,939,0,0.,0.,0.,0,"{Number of samples} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,2,0,941,0,0.,0.,0.,0,"{Random seed} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodEG"}
		},
	kw_76[1] = {
		{"list_of_points",14,0,1,1,1123,0,0.,0.,0.,0,"{List of points to evaluate} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodPSLPS"}
		},
	kw_77[2] = {
		{"complementary",8,0,1,1,979},
		{"cumulative",8,0,1,1,977}
		},
	kw_78[1] = {
		{"num_gen_reliability_levels",13,0,1,0,973}
		},
	kw_79[1] = {
		{"num_probability_levels",13,0,1,0,969}
		},
	kw_80[2] = {
		{"gen_reliabilities",8,0,1,1,965},
		{"probabilities",8,0,1,1,963}
		},
	kw_81[2] = {
		{"compute",8,2,2,0,961,kw_80},
		{"num_response_levels",13,0,1,0,959}
		},
	kw_82[6] = {
		{"distribution",8,2,5,0,975,kw_77},
		{"gen_reliability_levels",14,1,4,0,971,kw_78},
		{"nip",8,0,1,0,955},
		{"probability_levels",14,1,3,0,967,kw_79},
		{"response_levels",14,2,2,0,957,kw_81},
		{"sqp",8,0,1,0,953}
		},
	kw_83[2] = {
		{"nip",8,0,1,0,985},
		{"sqp",8,0,1,0,983}
		},
	kw_84[5] = {
		{"adapt_import",8,0,1,1,1019},
		{"import",8,0,1,1,1017},
		{"mm_adapt_import",8,0,1,1,1021},
		{"samples",9,0,2,0,1023,0,0.,0.,0.,0,"{Refinement samples} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDLocalRel"},
		{"seed",9,0,3,0,1025,0,0.,0.,0.,0,"{Random seed} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"}
		},
	kw_85[3] = {
		{"first_order",8,0,1,1,1011},
		{"sample_refinement",8,5,2,0,1015,kw_84},
		{"second_order",8,0,1,1,1013}
		},
	kw_86[9] = {
		{"nip",8,0,2,0,1007},
		{"no_approx",8,0,1,1,1003},
		{"sqp",8,0,2,0,1005},
		{"u_taylor_mean",8,0,1,1,993},
		{"u_taylor_mpp",8,0,1,1,997},
		{"u_two_point",8,0,1,1,1001},
		{"x_taylor_mean",8,0,1,1,991},
		{"x_taylor_mpp",8,0,1,1,995},
		{"x_two_point",8,0,1,1,999}
		},
	kw_87[1] = {
		{"num_reliability_levels",13,0,1,0,1041}
		},
	kw_88[3] = {
		{"gen_reliabilities",8,0,1,1,1037},
		{"probabilities",8,0,1,1,1033},
		{"reliabilities",8,0,1,1,1035}
		},
	kw_89[2] = {
		{"compute",8,3,2,0,1031,kw_88},
		{"num_response_levels",13,0,1,0,1029}
		},
	kw_90[7] = {
		{"distribution",8,2,5,0,1075,kw_65},
		{"gen_reliability_levels",14,1,7,0,1085,kw_66},
		{"integration",8,3,2,0,1009,kw_85,0.,0.,0.,0,"{Integration method} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDLocalRel"},
		{"mpp_search",8,9,1,0,989,kw_86,0.,0.,0.,0,"{MPP search type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDLocalRel"},
		{"probability_levels",14,1,6,0,1081,kw_67},
		{"reliability_levels",14,1,4,0,1039,kw_87},
		{"response_levels",14,2,3,0,1027,kw_89}
		},
	kw_91[2] = {
		{"num_offspring",0x19,0,2,0,371,0,0.,0.,0.,0,"{Number of offspring in random shuffle crossover} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"num_parents",0x19,0,1,0,369,0,0.,0.,0.,0,"{Number of parents in random shuffle crossover} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_92[5] = {
		{"crossover_rate",10,0,2,0,373,0,0.,0.,0.,0,"{Crossover rate} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"multi_point_binary",9,0,1,1,361,0,0.,0.,0.,0,"{Multi point binary crossover} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"multi_point_parameterized_binary",9,0,1,1,363,0,0.,0.,0.,0,"{Multi point parameterized binary crossover} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"multi_point_real",9,0,1,1,365,0,0.,0.,0.,0,"{Multi point real crossover} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"shuffle_random",8,2,1,1,367,kw_91,0.,0.,0.,0,"{Random shuffle crossover} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_93[3] = {
		{"flat_file",11,0,1,1,357},
		{"simple_random",8,0,1,1,353},
		{"unique_random",8,0,1,1,355}
		},
	kw_94[1] = {
		{"mutation_scale",10,0,1,0,387,0,0.,0.,0.,0,"{Mutation scale} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_95[6] = {
		{"bit_random",8,0,1,1,377},
		{"mutation_rate",10,0,2,0,389,0,0.,0.,0.,0,"{Mutation rate} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"offset_cauchy",8,1,1,1,383,kw_94},
		{"offset_normal",8,1,1,1,381,kw_94},
		{"offset_uniform",8,1,1,1,385,kw_94},
		{"replace_uniform",8,0,1,1,379}
		},
	kw_96[3] = {
		{"metric_tracker",8,0,1,1,303,0,0.,0.,0.,0,"{Convergence type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{"num_generations",0x29,0,3,0,307,0,0.,0.,0.,0,"{Number generations for metric_tracker converger} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{"percent_change",10,0,2,0,305,0,0.,0.,0.,0,"{Percent change limit for metric_tracker converger} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"}
		},
	kw_97[2] = {
		{"domination_count",8,0,1,1,281},
		{"layer_rank",8,0,1,1,279}
		},
	kw_98[2] = {
		{"distance",14,0,1,1,299},
		{"radial",14,0,1,1,297}
		},
	kw_99[1] = {
		{"orthogonal_distance",14,0,1,1,311,0,0.,0.,0.,0,"{Post_processor distance} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"}
		},
	kw_100[2] = {
		{"shrinkage_fraction",10,0,1,0,293},
		{"shrinkage_percentage",2,0,1,0,292}
		},
	kw_101[4] = {
		{"below_limit",10,2,1,1,291,kw_100,0.,0.,0.,0,"{Below limit selection} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"elitist",8,0,1,1,285},
		{"roulette_wheel",8,0,1,1,287},
		{"unique_roulette_wheel",8,0,1,1,289}
		},
	kw_102[21] = {
		{"convergence_type",8,3,4,0,301,kw_96},
		{"crossover_type",8,5,19,0,359,kw_92,0.,0.,0.,0,"{Crossover type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"fitness_type",8,2,1,0,277,kw_97,0.,0.,0.,0,"{Fitness type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{"initialization_type",8,3,18,0,351,kw_93,0.,0.,0.,0,"{Initialization type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"linear_equality_constraint_matrix",14,0,11,0,403,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,13,0,407,0,0.,0.,0.,0,"{Linear equality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,14,0,409,0,0.,0.,0.,0,"{Linear equality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,12,0,405,0,0.,0.,0.,0,"{Linear equality targets} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,6,0,393,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,7,0,395,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,9,0,399,0,0.,0.,0.,0,"{Linear inequality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,10,0,401,0,0.,0.,0.,0,"{Linear inequality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,8,0,397,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"log_file",11,0,16,0,347,0,0.,0.,0.,0,"{Log file} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"mutation_type",8,6,20,0,375,kw_95,0.,0.,0.,0,"{Mutation type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"niching_type",8,2,3,0,295,kw_98,0.,0.,0.,0,"{Niche pressure type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{"population_size",9,0,15,0,345,0,0.,0.,0.,0,"{Number of population members} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"postprocessor_type",8,1,5,0,309,kw_99,0.,0.,0.,0,"{Post_processor type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{"print_each_pop",8,0,17,0,349,0,0.,0.,0.,0,"{Population output} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"replacement_type",8,4,2,0,283,kw_101,0.,0.,0.,0,"{Replacement type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"seed",9,0,21,0,391,0,0.,0.,0.,0,"{Random seed} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_103[1] = {
		{"partitions",13,0,1,1,1133,0,0.,0.,0.,0,"{Partitions per variable} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodPSMPS"}
		},
	kw_104[4] = {
		{"min_boxsize_limit",10,0,2,0,947,0,0.,0.,0.,0,"{Min boxsize limit} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNCSUDC"},
		{"solution_accuracy",2,0,1,0,944},
		{"solution_target",10,0,1,0,945,0,0.,0.,0.,0,"{Solution Target } http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNCSUDC"},
		{"volume_boxsize_limit",10,0,3,0,949,0,0.,0.,0.,0,"{Volume boxsize limit} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNCSUDC"}
		},
	kw_105[9] = {
		{"absolute_conv_tol",10,0,2,0,551,0,0.,0.,0.,0,"{Absolute function convergence tolerance} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"},
		{"covariance",9,0,8,0,563,0,0.,0.,0.,0,"{Covariance post-processing} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"},
		{"false_conv_tol",10,0,6,0,559,0,0.,0.,0.,0,"{False convergence tolerance} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"},
		{"function_precision",10,0,1,0,549,0,0.,0.,0.,0,"{Relative precision in least squares terms} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"},
		{"initial_trust_radius",10,0,7,0,561,0,0.,0.,0.,0,"{Initial trust region radius} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"},
		{"regression_diagnostics",8,0,9,0,565,0,0.,0.,0.,0,"{Regression diagnostics post-processing} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"},
		{"singular_conv_tol",10,0,4,0,555,0,0.,0.,0.,0,"{Singular convergence tolerance} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"},
		{"singular_radius",10,0,5,0,557,0,0.,0.,0.,0,"{Step limit for sctol} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"},
		{"x_conv_tol",10,0,3,0,553,0,0.,0.,0.,0,"{Convergence tolerance for change in parameter vector} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"}
		},
	kw_106[1] = {
		{"num_reliability_levels",13,0,1,0,749,0,0.,0.,0.,0,"{Number of reliability levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"}
		},
	kw_107[3] = {
		{"gen_reliabilities",8,0,1,1,761},
		{"probabilities",8,0,1,1,757},
		{"reliabilities",8,0,1,1,759}
		},
	kw_108[2] = {
		{"compute",8,3,2,0,755,kw_107,0.,0.,0.,0,"{Target statistics for response levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"num_response_levels",13,0,1,0,753,0,0.,0.,0.,0,"{Number of response levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"}
		},
	kw_109[7] = {
		{"expansion_order",13,0,5,1,641,0,0.,0.,0.,0,"{Expansion order} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"expansion_terms",9,0,5,1,643,0,0.,0.,0.,0,"{Expansion terms} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"ratio_order",10,0,1,0,627},
		{"reuse_points",8,0,2,0,629,0,0.,0.,0.,0,"{Reuse points flag for PCE coefficient estimation by collocation_points or collocation_ratio} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"reuse_samples",0,0,2,0,628},
		{"tensor_grid",8,0,4,0,633},
		{"use_derivatives",8,0,3,0,631,0,0.,0.,0.,0,"{Derivative usage flag for PCE coefficient estimation by collocation_points or collocation_ratio} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"}
		},
	kw_110[2] = {
		{"expansion_order",13,0,5,1,641,0,0.,0.,0.,0,"{Expansion order} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"expansion_terms",9,0,5,1,643,0,0.,0.,0.,0,"{Expansion terms} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"}
		},
	kw_111[3] = {
		{"expansion_order",13,0,2,1,641,0,0.,0.,0.,0,"{Expansion order} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"expansion_terms",9,0,2,1,643,0,0.,0.,0.,0,"{Expansion terms} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"incremental_lhs",8,0,1,0,637,0,0.,0.,0.,0,"{Incremental LHS flag for PCE coefficient estimation by expansion_samples} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"}
		},
	kw_112[3] = {
		{"decay",8,0,1,1,599},
		{"generalized",8,0,1,1,601},
		{"sobol",8,0,1,1,597}
		},
	kw_113[2] = {
		{"dimension_adaptive",8,3,1,1,595,kw_112},
		{"uniform",8,0,1,1,593}
		},
	kw_114[3] = {
		{"dimension_preference",14,0,1,0,615,0,0.,0.,0.,0,"{Dimension preference for anisotropic tensor and sparse grids} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"nested",8,0,2,0,617},
		{"non_nested",8,0,2,0,619}
		},
	kw_115[3] = {
		{"adapt_import",8,0,1,1,655},
		{"import",8,0,1,1,653},
		{"mm_adapt_import",8,0,1,1,657}
		},
	kw_116[2] = {
		{"lhs",8,0,1,1,661},
		{"random",8,0,1,1,663}
		},
	kw_117[5] = {
		{"dimension_preference",14,0,2,0,615,0,0.,0.,0.,0,"{Dimension preference for anisotropic tensor and sparse grids} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"nested",8,0,3,0,617},
		{"non_nested",8,0,3,0,619},
		{"restricted",8,0,1,0,611},
		{"unrestricted",8,0,1,0,613}
		},
	kw_118[2] = {
		{"drop_tolerance",10,0,2,0,649,0,0.,0.,0.,0,"{VBD tolerance for omitting small indices} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"univariate_effects",8,0,1,0,647,0,0.,0.,0.,0,"{Restriction of VBD indices to main/total} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"}
		},
	kw_119[23] = {
		{"all_variables",8,0,15,0,743,0,0.,0.,0.,0,"{All variables flag} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"askey",8,0,2,0,603},
		{"collocation_points",9,7,3,1,623,kw_109,0.,0.,0.,0,"{Number of collocation points for PCE coefficient estimation} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"collocation_ratio",10,7,3,1,625,kw_109,0.,0.,0.,0,"{Collocation point oversampling ratio for PCE coefficient estimation} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"cubature_integrand",9,0,3,1,621,0,0.,0.,0.,0,"{Cubature integrand order for PCE coefficient estimation} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"distribution",8,2,9,0,799,kw_52,0.,0.,0.,0,"{Distribution type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"expansion_import_file",11,2,3,1,639,kw_110,0.,0.,0.,0,"{File name for import of PCE coefficients} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"expansion_samples",9,3,3,1,635,kw_111,0.,0.,0.,0,"{Number of LHS simulation samples for PCE coefficient estimation} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"fixed_seed",8,0,16,0,745,0,0.,0.,0.,0,"{Fixed seed flag} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"gen_reliability_levels",14,1,11,0,809,kw_53,0.,0.,0.,0,"{Generalized reliability levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"p_refinement",8,2,1,0,591,kw_113,0.,0.,0.,0,"{Automated polynomial order refinement} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"probability_levels",14,1,10,0,805,kw_54,0.,0.,0.,0,"{Probability levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"quadrature_order",13,3,3,1,607,kw_114,0.,0.,0.,0,"{Quadrature order for PCE coefficient estimation} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"reliability_levels",14,1,13,0,747,kw_106,0.,0.,0.,0,"{Reliability levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"response_levels",14,2,14,0,751,kw_108,0.,0.,0.,0,"{Response levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"rng",8,2,12,0,813,kw_55,0.,0.,0.,0,"{Random number generator} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"sample_refinement",8,3,5,0,651,kw_115,0.,0.,0.,0,"{Importance sampling refinement} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"sample_type",8,2,6,0,659,kw_116,0.,0.,0.,0,"{Sampling type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"samples",9,0,8,0,939,0,0.,0.,0.,0,"{Number of samples} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,7,0,941,0,0.,0.,0.,0,"{Random seed} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodEG"},
		{"sparse_grid_level",13,5,3,1,609,kw_117,0.,0.,0.,0,"{Sparse grid level for PCE coefficient estimation} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"variance_based_decomp",8,2,4,0,645,kw_118,0.,0.,0.,0,"{Variance based decomposition (VBD)} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"wiener",8,0,2,0,605}
		},
	kw_120[1] = {
		{"previous_samples",9,0,1,1,737,0,0.,0.,0.,0,"{Previous samples for incremental approaches} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"}
		},
	kw_121[4] = {
		{"incremental_lhs",8,1,1,1,733,kw_120},
		{"incremental_random",8,1,1,1,735,kw_120},
		{"lhs",8,0,1,1,731},
		{"random",8,0,1,1,729}
		},
	kw_122[1] = {
		{"drop_tolerance",10,0,1,0,741}
		},
	kw_123[12] = {
		{"all_variables",8,0,11,0,743,0,0.,0.,0.,0,"{All variables flag} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"distribution",8,2,5,0,799,kw_52,0.,0.,0.,0,"{Distribution type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"fixed_seed",8,0,12,0,745,0,0.,0.,0.,0,"{Fixed seed flag} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"gen_reliability_levels",14,1,7,0,809,kw_53,0.,0.,0.,0,"{Generalized reliability levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"probability_levels",14,1,6,0,805,kw_54,0.,0.,0.,0,"{Probability levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"reliability_levels",14,1,9,0,747,kw_106,0.,0.,0.,0,"{Reliability levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"response_levels",14,2,10,0,751,kw_108,0.,0.,0.,0,"{Response levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"rng",8,2,8,0,813,kw_55,0.,0.,0.,0,"{Random number generator} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"sample_type",8,4,1,0,727,kw_121},
		{"samples",9,0,4,0,939,0,0.,0.,0.,0,"{Number of samples} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,3,0,941,0,0.,0.,0.,0,"{Random seed} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodEG"},
		{"variance_based_decomp",8,1,2,0,739,kw_122}
		},
	kw_124[2] = {
		{"generalized",8,0,1,1,677},
		{"sobol",8,0,1,1,675}
		},
	kw_125[2] = {
		{"dimension_adaptive",8,2,1,1,673,kw_124},
		{"uniform",8,0,1,1,671}
		},
	kw_126[2] = {
		{"hierarchical",8,0,1,0,683},
		{"nodal",8,0,1,0,681}
		},
	kw_127[3] = {
		{"adapt_import",8,0,1,1,715},
		{"import",8,0,1,1,713},
		{"mm_adapt_import",8,0,1,1,717}
		},
	kw_128[2] = {
		{"lhs",8,0,1,1,721},
		{"random",8,0,1,1,723}
		},
	kw_129[2] = {
		{"restricted",8,0,1,0,693},
		{"unrestricted",8,0,1,0,695}
		},
	kw_130[2] = {
		{"drop_tolerance",10,0,2,0,709,0,0.,0.,0.,0,"{VBD tolerance for omitting small indices} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"},
		{"univariate_effects",8,0,1,0,707,0,0.,0.,0.,0,"{Restriction of VBD indices to main/total} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"}
		},
	kw_131[24] = {
		{"all_variables",8,0,18,0,743,0,0.,0.,0.,0,"{All variables flag} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"askey",8,0,2,0,685},
		{"dimension_preference",14,0,4,0,697,0,0.,0.,0.,0,"{Dimension preference for anisotropic tensor and sparse grids} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"},
		{"distribution",8,2,12,0,799,kw_52,0.,0.,0.,0,"{Distribution type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"fixed_seed",8,0,19,0,745,0,0.,0.,0.,0,"{Fixed seed flag} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"gen_reliability_levels",14,1,14,0,809,kw_53,0.,0.,0.,0,"{Generalized reliability levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"h_refinement",8,2,1,0,669,kw_125},
		{"nested",8,0,6,0,701},
		{"non_nested",8,0,6,0,703},
		{"p_refinement",8,2,1,0,667,kw_125},
		{"piecewise",8,2,2,0,679,kw_126},
		{"probability_levels",14,1,13,0,805,kw_54,0.,0.,0.,0,"{Probability levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"quadrature_order",13,0,3,1,689,0,0.,0.,0.,0,"{Quadrature order for collocation points} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"},
		{"reliability_levels",14,1,16,0,747,kw_106,0.,0.,0.,0,"{Reliability levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"response_levels",14,2,17,0,751,kw_108,0.,0.,0.,0,"{Response levels} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"rng",8,2,15,0,813,kw_55,0.,0.,0.,0,"{Random number generator} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"sample_refinement",8,3,8,0,711,kw_127},
		{"sample_type",8,2,9,0,719,kw_128},
		{"samples",9,0,11,0,939,0,0.,0.,0.,0,"{Number of samples} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,10,0,941,0,0.,0.,0.,0,"{Random seed} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodEG"},
		{"sparse_grid_level",13,2,3,1,691,kw_129,0.,0.,0.,0,"{Sparse grid level for collocation points} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"},
		{"use_derivatives",8,0,5,0,699,0,0.,0.,0.,0,"{Derivative usage flag} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"},
		{"variance_based_decomp",8,2,7,0,705,kw_130,0.,0.,0.,0,"{Variance-based decomposition (VBD)} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"},
		{"wiener",8,0,2,0,687}
		},
	kw_132[1] = {
		{"misc_options",15,0,1,0,569}
		},
	kw_133[12] = {
		{"function_precision",10,0,11,0,197,0,0.,0.,0.,0,"{Function precision} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNPSOLDC"},
		{"linear_equality_constraint_matrix",14,0,6,0,403,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,8,0,407,0,0.,0.,0.,0,"{Linear equality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,9,0,409,0,0.,0.,0.,0,"{Linear equality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,7,0,405,0,0.,0.,0.,0,"{Linear equality targets} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,1,0,393,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,2,0,395,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,4,0,399,0,0.,0.,0.,0,"{Linear inequality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,5,0,401,0,0.,0.,0.,0,"{Linear inequality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,3,0,397,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linesearch_tolerance",10,0,12,0,199,0,0.,0.,0.,0,"{Line search tolerance} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNPSOLDC"},
		{"verify_level",9,0,10,0,195,0,0.,0.,0.,0,"{Gradient verification level} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNPSOLDC"}
		},
	kw_134[11] = {
		{"gradient_tolerance",10,0,11,0,233},
		{"linear_equality_constraint_matrix",14,0,6,0,403,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,8,0,407,0,0.,0.,0.,0,"{Linear equality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,9,0,409,0,0.,0.,0.,0,"{Linear equality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,7,0,405,0,0.,0.,0.,0,"{Linear equality targets} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,1,0,393,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,2,0,395,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,4,0,399,0,0.,0.,0.,0,"{Linear inequality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,5,0,401,0,0.,0.,0.,0,"{Linear inequality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,3,0,397,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"max_step",10,0,10,0,231}
		},
	kw_135[10] = {
		{"linear_equality_constraint_matrix",14,0,7,0,403,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,9,0,407,0,0.,0.,0.,0,"{Linear equality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,10,0,409,0,0.,0.,0.,0,"{Linear equality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,8,0,405,0,0.,0.,0.,0,"{Linear equality targets} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,2,0,393,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,3,0,395,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,5,0,399,0,0.,0.,0.,0,"{Linear inequality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,6,0,401,0,0.,0.,0.,0,"{Linear inequality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,4,0,397,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"search_scheme_size",9,0,1,0,237}
		},
	kw_136[4] = {
		{"gradient_based_line_search",8,0,1,1,217,0,0.,0.,0.,0,"[CHOOSE line search type]"},
		{"tr_pds",8,0,1,1,221},
		{"trust_region",8,0,1,1,219},
		{"value_based_line_search",8,0,1,1,215}
		},
	kw_137[16] = {
		{"centering_parameter",10,0,5,0,229},
		{"central_path",11,0,3,0,225},
		{"gradient_tolerance",10,0,16,0,233},
		{"linear_equality_constraint_matrix",14,0,11,0,403,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,13,0,407,0,0.,0.,0.,0,"{Linear equality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,14,0,409,0,0.,0.,0.,0,"{Linear equality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,12,0,405,0,0.,0.,0.,0,"{Linear equality targets} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,6,0,393,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,7,0,395,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,9,0,399,0,0.,0.,0.,0,"{Linear inequality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,10,0,401,0,0.,0.,0.,0,"{Linear inequality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,8,0,397,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"max_step",10,0,15,0,231},
		{"merit_function",11,0,2,0,223},
		{"search_method",8,4,1,0,213,kw_136},
		{"steplength_to_boundary",10,0,4,0,227}
		},
	kw_138[5] = {
		{"debug",8,0,1,1,67,0,0.,0.,0.,0,"[CHOOSE output level]"},
		{"normal",8,0,1,1,71},
		{"quiet",8,0,1,1,73},
		{"silent",8,0,1,1,75},
		{"verbose",8,0,1,1,69}
		},
	kw_139[3] = {
		{"partitions",13,0,1,0,937,0,0.,0.,0.,0,"{Number of partitions} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodPSUADE"},
		{"samples",9,0,3,0,939,0,0.,0.,0.,0,"{Number of samples} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,2,0,941,0,0.,0.,0.,0,"{Random seed} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodEG"}
		},
	kw_140[4] = {
		{"converge_order",8,0,1,1,1139},
		{"converge_qoi",8,0,1,1,1141},
		{"estimate_order",8,0,1,1,1137},
		{"refinement_rate",10,0,2,0,1143,0,0.,0.,0.,0,"{Refinement rate} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSolnRichardson"}
		},
	kw_141[2] = {
		{"num_generations",0x29,0,2,0,343},
		{"percent_change",10,0,1,0,341}
		},
	kw_142[2] = {
		{"num_generations",0x29,0,2,0,337,0,0.,0.,0.,0,"{Number of generations (for convergence test) } http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{"percent_change",10,0,1,0,335,0,0.,0.,0.,0,"{Percent change in fitness} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"}
		},
	kw_143[2] = {
		{"average_fitness_tracker",8,2,1,1,339,kw_141},
		{"best_fitness_tracker",8,2,1,1,333,kw_142}
		},
	kw_144[2] = {
		{"constraint_penalty",10,0,2,0,319,0,0.,0.,0.,0,"{Constraint penalty in merit function} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{"merit_function",8,0,1,1,317}
		},
	kw_145[4] = {
		{"elitist",8,0,1,1,323},
		{"favor_feasible",8,0,1,1,325},
		{"roulette_wheel",8,0,1,1,327},
		{"unique_roulette_wheel",8,0,1,1,329}
		},
	kw_146[19] = {
		{"convergence_type",8,2,3,0,331,kw_143,0.,0.,0.,0,"{Convergence type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{"crossover_type",8,5,17,0,359,kw_92,0.,0.,0.,0,"{Crossover type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"fitness_type",8,2,1,0,315,kw_144,0.,0.,0.,0,"{Fitness type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{"initialization_type",8,3,16,0,351,kw_93,0.,0.,0.,0,"{Initialization type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"linear_equality_constraint_matrix",14,0,9,0,403,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,11,0,407,0,0.,0.,0.,0,"{Linear equality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,12,0,409,0,0.,0.,0.,0,"{Linear equality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,10,0,405,0,0.,0.,0.,0,"{Linear equality targets} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,4,0,393,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,5,0,395,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,7,0,399,0,0.,0.,0.,0,"{Linear inequality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,8,0,401,0,0.,0.,0.,0,"{Linear inequality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,6,0,397,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"log_file",11,0,14,0,347,0,0.,0.,0.,0,"{Log file} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"mutation_type",8,6,18,0,375,kw_95,0.,0.,0.,0,"{Mutation type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"population_size",9,0,13,0,345,0,0.,0.,0.,0,"{Number of population members} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"print_each_pop",8,0,15,0,349,0,0.,0.,0.,0,"{Population output} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"replacement_type",8,4,2,0,321,kw_145,0.,0.,0.,0,"{Replacement type} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{"seed",9,0,19,0,391,0,0.,0.,0.,0,"{Random seed} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_147[14] = {
		{"function_precision",10,0,12,0,197,0,0.,0.,0.,0,"{Function precision} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNPSOLDC"},
		{"linear_equality_constraint_matrix",14,0,7,0,403,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,9,0,407,0,0.,0.,0.,0,"{Linear equality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,10,0,409,0,0.,0.,0.,0,"{Linear equality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,8,0,405,0,0.,0.,0.,0,"{Linear equality targets} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,2,0,393,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,3,0,395,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,5,0,399,0,0.,0.,0.,0,"{Linear inequality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,6,0,401,0,0.,0.,0.,0,"{Linear inequality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,4,0,397,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linesearch_tolerance",10,0,13,0,199,0,0.,0.,0.,0,"{Line search tolerance} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNPSOLDC"},
		{"nlssol",8,0,1,1,193},
		{"npsol",8,0,1,1,191},
		{"verify_level",9,0,11,0,195,0,0.,0.,0.,0,"{Gradient verification level} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNPSOLDC"}
		},
	kw_148[3] = {
		{"approx_method_name",11,0,1,1,573,0,0.,0.,0.,0,"[CHOOSE sub-method ref.]{Approximate sub-problem minimization method name} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBG"},
		{"approx_method_pointer",11,0,1,1,575,0,0.,0.,0.,0,"{Approximate sub-problem minimization method pointer} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBG"},
		{"replace_points",8,0,2,0,577,0,0.,0.,0.,0,"{Replace points used in surrogate construction with best points from previous iteration} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBG"}
		},
	kw_149[2] = {
		{"filter",8,0,1,1,145,0,0.,0.,0.,0,"@[CHOOSE acceptance logic]"},
		{"tr_ratio",8,0,1,1,143}
		},
	kw_150[7] = {
		{"augmented_lagrangian_objective",8,0,1,1,121,0,0.,0.,0.,0,"[CHOOSE objective formulation]"},
		{"lagrangian_objective",8,0,1,1,123},
		{"linearized_constraints",8,0,2,2,127,0,0.,0.,0.,0,"[CHOOSE constraint formulation]"},
		{"no_constraints",8,0,2,2,129},
		{"original_constraints",8,0,2,2,125,0,0.,0.,0.,0,"@"},
		{"original_primary",8,0,1,1,117,0,0.,0.,0.,0,"@"},
		{"single_objective",8,0,1,1,119}
		},
	kw_151[1] = {
		{"homotopy",8,0,1,1,149}
		},
	kw_152[4] = {
		{"adaptive_penalty_merit",8,0,1,1,135,0,0.,0.,0.,0,"[CHOOSE merit function]"},
		{"augmented_lagrangian_merit",8,0,1,1,139,0,0.,0.,0.,0,"@"},
		{"lagrangian_merit",8,0,1,1,137},
		{"penalty_merit",8,0,1,1,133}
		},
	kw_153[6] = {
		{"contract_threshold",10,0,3,0,107,0,0.,0.,0.,0,"{Shrink trust region if trust region ratio is below this value} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"contraction_factor",10,0,5,0,111,0,0.,0.,0.,0,"{Trust region contraction factor} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"expand_threshold",10,0,4,0,109,0,0.,0.,0.,0,"{Expand trust region if trust region ratio is above this value} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"expansion_factor",10,0,6,0,113,0,0.,0.,0.,0,"{Trust region expansion factor} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"initial_size",10,0,1,0,103,0,0.,0.,0.,0,"{Trust region initial size (relative to bounds)} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"minimum_size",10,0,2,0,105,0,0.,0.,0.,0,"{Trust region minimum size} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBL"}
		},
	kw_154[18] = {
		{"acceptance_logic",8,2,7,0,141,kw_149,0.,0.,0.,0,"{SBL iterate acceptance logic} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"approx_method_name",11,0,1,1,93,0,0.,0.,0.,0,"[CHOOSE sub-method ref.]{Approximate sub-problem minimization method name} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"approx_method_pointer",11,0,1,1,95,0,0.,0.,0.,0,"{Approximate sub-problem minimization method pointer} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"approx_subproblem",8,7,5,0,115,kw_150,0.,0.,0.,0,"{Approximate subproblem formulation} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"constraint_relax",8,1,8,0,147,kw_151,0.,0.,0.,0,"{SBL constraint relaxation method for infeasible iterates} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"linear_equality_constraint_matrix",14,0,14,0,403,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,16,0,407,0,0.,0.,0.,0,"{Linear equality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,17,0,409,0,0.,0.,0.,0,"{Linear equality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,15,0,405,0,0.,0.,0.,0,"{Linear equality targets} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,9,0,393,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,10,0,395,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,12,0,399,0,0.,0.,0.,0,"{Linear inequality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,13,0,401,0,0.,0.,0.,0,"{Linear inequality scales} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,11,0,397,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"merit_function",8,4,6,0,131,kw_152,0.,0.,0.,0,"{SBL merit function} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"soft_convergence_limit",9,0,2,0,97,0,0.,0.,0.,0,"{Soft convergence limit for SBL iterations} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"trust_region",8,6,4,0,101,kw_153,0.,0.,0.,0,"{Trust region group specification} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"truth_surrogate_bypass",8,0,3,0,99,0,0.,0.,0.,0,"{Flag for bypassing lower level surrogates in truth verifications} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSBL"}
		},
	kw_155[3] = {
		{"final_point",14,0,1,1,1115,0,0.,0.,0.,0,"[CHOOSE final pt or increment]{Termination point of vector} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodPSVPS"},
		{"num_steps",9,0,2,2,1119,0,0.,0.,0.,0,"{Number of steps along vector} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodPSVPS"},
		{"step_vector",14,0,1,1,1117,0,0.,0.,0.,0,"{Step vector} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodPSVPS"}
		},
	kw_156[75] = {
		{"asynch_pattern_search",8,18,11,1,239,kw_12,0.,0.,0.,0,0,"Optimization: Local, Derivative-free"},
		{"bayes_calibration",8,10,11,1,839,kw_22},
		{"centered_parameter_study",8,3,11,1,1125,kw_23,0.,0.,0.,0,"[CHOOSE PSTUDY method]{Centered parameter study} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodPSCPS","Parameter Studies"},
		{"coliny_apps",0,18,11,1,238,kw_12},
		{"coliny_cobyla",8,7,11,1,457,kw_24,0.,0.,0.,0,0,"Optimization: Local, Derivative-free"},
		{"coliny_direct",8,11,11,1,463,kw_26,0.,0.,0.,0,0,"Optimization: Global"},
		{"coliny_ea",8,14,11,1,481,kw_33,0.,0.,0.,0,0,"Optimization: Global"},
		{"coliny_pattern_search",8,17,11,1,411,kw_37,0.,0.,0.,0,0,"Optimization: Local, Derivative-free"},
		{"coliny_solis_wets",8,13,11,1,443,kw_38,0.,0.,0.,0,0,"Optimization: Local, Derivative-free"},
		{"conmin_frcg",8,9,11,1,179,kw_39,0.,0.,0.,0,"[CHOOSE OPT method]","Optimization: Local, Derivative-based"},
		{"conmin_mfd",8,9,11,1,181,kw_39,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"constraint_tolerance",10,0,8,0,85,0,0.,0.,0.,0,"{Constraint tolerance} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"convergence_tolerance",10,0,7,0,83,0,0.,0.,0.,0,"{Convergence tolerance} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"dace",8,14,11,1,885,kw_41,0.,0.,0.,0,0,"[CHOOSE method category]DACE"},
		{"dl_solver",11,9,11,1,183,kw_39,0.,0.,0.,0,0,"Optimization: Plug-in"},
		{"dot",8,15,11,1,161,kw_43},
		{"dot_bfgs",8,10,11,1,155,kw_44,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"dot_frcg",8,10,11,1,151,kw_44,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"dot_mmfd",8,10,11,1,153,kw_44,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"dot_slp",8,10,11,1,157,kw_44,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"dot_sqp",8,10,11,1,159,kw_44,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"efficient_global",8,4,11,1,579,kw_46,0.,0.,0.,0,0,"Surrogate-based Methods"},
		{"final_solutions",0x29,0,10,0,89,0,0.,0.,0.,0,"{Final solutions} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"fsu_cvt",8,8,11,1,913,kw_49,0.,0.,0.,0,0,"DACE"},
		{"fsu_quasi_mc",8,10,11,1,1089,kw_51,0.,0.,0.,0,0,"DACE"},
		{"global_evidence",8,9,11,1,775,kw_60,0.,0.,0.,0,0,"Uncertainty Quantification"},
		{"global_interval_est",8,5,11,1,819,kw_64,0.,0.,0.,0,0,"Uncertainty Quantification"},
		{"global_reliability",8,12,11,1,1043,kw_72,0.,0.,0.,0,0,"Uncertainty Quantification"},
		{"id_method",11,0,1,0,61,0,0.,0.,0.,0,"{Method set identifier} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"importance_sampling",8,7,11,1,763,kw_75,0.,0.,0.,0,0,"Uncertainty Quantification"},
		{"list_parameter_study",8,1,11,1,1121,kw_76,0.,0.,0.,0,"{List parameter study} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodPSLPS","Parameter Studies"},
		{"local_evidence",8,6,11,1,951,kw_82,0.,0.,0.,0,0,"Uncertainty Quantification"},
		{"local_interval_est",8,2,11,1,981,kw_83,0.,0.,0.,0,0,"Uncertainty Quantification"},
		{"local_reliability",8,7,11,1,987,kw_90,0.,0.,0.,0,"{Reliability method} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDLocalRel","Uncertainty Quantification"},
		{"max_function_evaluations",9,0,5,0,79,0,0.,0.,0.,0,"{Maximum function evaluations} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"max_iterations",9,0,4,0,77,0,0.,0.,0.,0,"{Maximum iterations} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"model_pointer",11,0,2,0,63,0,0.,0.,0.,0,"{Model pointer} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"moga",8,21,11,1,275,kw_102,0.,0.,0.,0,0,"Optimization: Global"},
		{"multidim_parameter_study",8,1,11,1,1131,kw_103,0.,0.,0.,0,"{Multidimensional parameter study} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodPSMPS","Parameter Studies"},
		{"ncsu_direct",8,4,11,1,943,kw_104,0.,0.,0.,0,0,"Optimization: Global"},
		{"nl2sol",8,9,11,1,547,kw_105,0.,0.,0.,0,"[CHOOSE LSQ method]","Nonlinear Least Squares"},
		{"nlpql_sqp",8,9,11,1,201,kw_39,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"nlssol_sqp",8,12,11,1,187,kw_133,0.,0.,0.,0,0,"Nonlinear Least Squares"},
		{"nond_bayes_calibration",0,10,11,1,838,kw_22},
		{"nond_global_evidence",0,9,11,1,774,kw_60},
		{"nond_global_interval_est",0,5,11,1,818,kw_64},
		{"nond_global_reliability",0,12,11,1,1042,kw_72},
		{"nond_importance_sampling",0,7,11,1,762,kw_75},
		{"nond_local_evidence",0,6,11,1,950,kw_82},
		{"nond_local_interval_est",0,2,11,1,980,kw_83},
		{"nond_local_reliability",0,7,11,1,986,kw_90},
		{"nond_polynomial_chaos",0,23,11,1,588,kw_119},
		{"nond_sampling",0,12,11,1,724,kw_123},
		{"nond_stoch_collocation",0,24,11,1,664,kw_131},
		{"nonlinear_cg",8,1,11,1,567,kw_132,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"npsol_sqp",8,12,11,1,185,kw_133,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"optpp_cg",8,11,11,1,203,kw_134,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"optpp_fd_newton",8,16,11,1,207,kw_137,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"optpp_g_newton",8,16,11,1,209,kw_137,0.,0.,0.,0,0,"Nonlinear Least Squares"},
		{"optpp_newton",8,16,11,1,211,kw_137,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"optpp_pds",8,10,11,1,235,kw_135,0.,0.,0.,0,0,"Optimization: Local, Derivative-free"},
		{"optpp_q_newton",8,16,11,1,205,kw_137,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"output",8,5,3,0,65,kw_138,0.,0.,0.,0,"{Output verbosity} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"polynomial_chaos",8,23,11,1,589,kw_119,0.,0.,0.,0,0,"Uncertainty Quantification"},
		{"psuade_moat",8,3,11,1,935,kw_139,0.,0.,0.,0,"{PSUADE MOAT method} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodPSUADE","DACE"},
		{"richardson_extrap",8,4,11,1,1135,kw_140,0.,0.,0.,0,"{Richardson extrapolation} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodSolnRichardson"},
		{"sampling",8,12,11,1,725,kw_123,0.,0.,0.,0,"{Nondeterministic sampling method} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC","Uncertainty Quantification"},
		{"scaling",8,0,9,0,87,0,0.,0.,0.,0,"{Scaling flag} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"soga",8,19,11,1,313,kw_146,0.,0.,0.,0,0,"Optimization: Global"},
		{"speculative",8,0,6,0,81,0,0.,0.,0.,0,"{Speculative gradients and Hessians} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"stanford",8,14,11,1,189,kw_147},
		{"stoch_collocation",8,24,11,1,665,kw_131,0.,0.,0.,0,0,"Uncertainty Quantification"},
		{"surrogate_based_global",8,3,11,1,571,kw_148,0.,0.,0.,0,0,"Surrogate-based Methods"},
		{"surrogate_based_local",8,18,11,1,91,kw_154,0.,0.,0.,0,0,"Surrogate-based Methods"},
		{"vector_parameter_study",8,3,11,1,1113,kw_155,0.,0.,0.,0,"{Vector parameter study} http://dakota.sandia.gov/licensing/votd/html-ref/MethodCommands.html#MethodPSVPS","Parameter Studies"}
		},
	kw_157[1] = {
		{"optional_interface_responses_pointer",11,0,1,0,1311,0,0.,0.,0.,0,"{Responses pointer for nested model optional interfaces} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelNested"}
		},
	kw_158[4] = {
		{"primary_response_mapping",14,0,3,0,1319,0,0.,0.,0.,0,"{Primary response mappings for nested models} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelNested"},
		{"primary_variable_mapping",15,0,1,0,1315,0,0.,0.,0.,0,"{Primary variable mappings for nested models} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelNested"},
		{"secondary_response_mapping",14,0,4,0,1321,0,0.,0.,0.,0,"{Secondary response mappings for nested models} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelNested"},
		{"secondary_variable_mapping",15,0,2,0,1317,0,0.,0.,0.,0,"{Secondary variable mappings for nested models} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelNested"}
		},
	kw_159[2] = {
		{"optional_interface_pointer",11,1,1,0,1309,kw_157,0.,0.,0.,0,"{Optional interface set pointer} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelNested"},
		{"sub_method_pointer",11,4,2,1,1313,kw_158,0.,0.,0.,0,"{Sub-method pointer for nested models} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelNested"}
		},
	kw_160[1] = {
		{"interface_pointer",11,0,1,0,1155,0,0.,0.,0.,0,"{Interface set pointer} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSingle"}
		},
	kw_161[6] = {
		{"additive",8,0,2,2,1269,0,0.,0.,0.,0,"[CHOOSE correction type]"},
		{"combined",8,0,2,2,1273},
		{"first_order",8,0,1,1,1265,0,0.,0.,0.,0,"[CHOOSE correction order]"},
		{"multiplicative",8,0,2,2,1271},
		{"second_order",8,0,1,1,1267},
		{"zeroth_order",8,0,1,1,1263}
		},
	kw_162[3] = {
		{"constant",8,0,1,1,1171},
		{"linear",8,0,1,1,1173},
		{"reduced_quadratic",8,0,1,1,1175}
		},
	kw_163[2] = {
		{"point_selection",8,0,1,0,1167,0,0.,0.,0.,0,"{GP point selection} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"trend",8,3,2,0,1169,kw_162,0.,0.,0.,0,"{GP trend function} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_164[4] = {
		{"constant",8,0,1,1,1181},
		{"linear",8,0,1,1,1183},
		{"quadratic",8,0,1,1,1187},
		{"reduced_quadratic",8,0,1,1,1185}
		},
	kw_165[4] = {
		{"correlation_lengths",14,0,4,0,1193,0,0.,0.,0.,0,"{Kriging correlation lengths} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"max_trials",0x19,0,3,0,1191,0,0.,0.,0.,0,"{Kriging maximum trials} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"optimization_method",11,0,2,0,1189,0,0.,0.,0.,0,"{Kriging optimization method} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"trend",8,4,1,0,1179,kw_164,0.,0.,0.,0,"{Kriging trend function} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_166[2] = {
		{"dakota",8,2,1,1,1165,kw_163},
		{"surfpack",8,4,1,1,1177,kw_165}
		},
	kw_167[2] = {
		{"cubic",8,0,1,1,1203},
		{"linear",8,0,1,1,1201}
		},
	kw_168[2] = {
		{"interpolation",8,2,2,0,1199,kw_167,0.,0.,0.,0,"{MARS interpolation} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"max_bases",9,0,1,0,1197,0,0.,0.,0.,0,"{MARS maximum bases} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_169[2] = {
		{"poly_order",9,0,1,0,1207,0,0.,0.,0.,0,"{MLS polynomial order} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"weight_function",9,0,2,0,1209,0,0.,0.,0.,0,"{MLS weight function} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_170[3] = {
		{"nodes",9,0,1,0,1213,0,0.,0.,0.,0,"{ANN number nodes} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"random_weight",9,0,3,0,1217,0,0.,0.,0.,0,"{ANN random weight} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"range",10,0,2,0,1215,0,0.,0.,0.,0,"{ANN range} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_171[2] = {
		{"annotated",8,0,1,0,1255},
		{"freeform",8,0,1,0,1257}
		},
	kw_172[3] = {
		{"cubic",8,0,1,1,1235,0,0.,0.,0.,0,"[CHOOSE polynomial order]"},
		{"linear",8,0,1,1,1231},
		{"quadratic",8,0,1,1,1233}
		},
	kw_173[4] = {
		{"bases",9,0,1,0,1221,0,0.,0.,0.,0,"{RBF number of bases} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"max_pts",9,0,2,0,1223,0,0.,0.,0.,0,"{RBF maximum points} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"max_subsets",9,0,4,0,1227},
		{"min_partition",9,0,3,0,1225,0,0.,0.,0.,0,"{RBF minimum partitions} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_174[3] = {
		{"all",8,0,1,1,1247},
		{"none",8,0,1,1,1251},
		{"region",8,0,1,1,1249}
		},
	kw_175[18] = {
		{"correction",8,6,7,0,1261,kw_161,0.,0.,0.,0,"{Surrogate correction approach} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"dace_method_pointer",11,0,3,0,1243,0,0.,0.,0.,0,"{Design of experiments method pointer} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"diagnostics",15,0,8,0,1275,0,0.,0.,0.,0,"{Print diagnostic metrics about the surrogate goodness of fit} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"gaussian_process",8,2,1,1,1163,kw_166,0.,0.,0.,0,"[CHOOSE surrogate type]{Gaussian process} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"kriging",0,2,1,1,1162,kw_166},
		{"mars",8,2,1,1,1195,kw_168,0.,0.,0.,0,"{Multivariate adaptive regression splines} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"minimum_points",8,0,2,0,1239},
		{"moving_least_squares",8,2,1,1,1205,kw_169,0.,0.,0.,0,"{Moving least squares} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"neural_network",8,3,1,1,1211,kw_170,0.,0.,0.,0,"{Artificial neural network} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"points_file",11,2,5,0,1253,kw_171},
		{"polynomial",8,3,1,1,1229,kw_172,0.,0.,0.,0,"{Polynomial} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"radial_basis",8,4,1,1,1219,kw_173},
		{"recommended_points",8,0,2,0,1241},
		{"reuse_points",8,3,4,0,1245,kw_174},
		{"reuse_samples",0,3,4,0,1244,kw_174},
		{"samples_file",3,2,5,0,1252,kw_171},
		{"total_points",9,0,2,0,1237},
		{"use_derivatives",8,0,6,0,1259,0,0.,0.,0.,0,"{Kriging gradient enhancement} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_176[6] = {
		{"additive",8,0,2,2,1301,0,0.,0.,0.,0,"[CHOOSE correction type]"},
		{"combined",8,0,2,2,1305},
		{"first_order",8,0,1,1,1297,0,0.,0.,0.,0,"[CHOOSE correction order]"},
		{"multiplicative",8,0,2,2,1303},
		{"second_order",8,0,1,1,1299},
		{"zeroth_order",8,0,1,1,1295}
		},
	kw_177[3] = {
		{"correction",8,6,3,3,1293,kw_176,0.,0.,0.,0,"{Surrogate correction approach} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrH"},
		{"high_fidelity_model_pointer",11,0,2,2,1291,0,0.,0.,0.,0,"{Pointer to the high fidelity model specification} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrH"},
		{"low_fidelity_model_pointer",11,0,1,1,1289,0,0.,0.,0.,0,"{Pointer to the low fidelity model specification} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrH"}
		},
	kw_178[2] = {
		{"actual_model_pointer",11,0,2,2,1285,0,0.,0.,0.,0,"{Pointer to the truth model specification} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrMP"},
		{"taylor_series",8,0,1,1,1283,0,0.,0.,0.,0,"{Taylor series local approximation } http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrL"}
		},
	kw_179[2] = {
		{"actual_model_pointer",11,0,2,2,1285,0,0.,0.,0.,0,"{Pointer to the truth model specification} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrMP"},
		{"tana",8,0,1,1,1279,0,0.,0.,0.,0,"{Two-point adaptive nonlinear approximation } http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrMP"}
		},
	kw_180[5] = {
		{"global",8,18,2,1,1161,kw_175,0.,0.,0.,0,"[CHOOSE surrogate category]{Global approximations } http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"hierarchical",8,3,2,1,1287,kw_177,0.,0.,0.,0,"{Hierarchical approximation } http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrH"},
		{"id_surrogates",13,0,1,0,1159,0,0.,0.,0.,0,"{Surrogate response ids} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrogate"},
		{"local",8,2,2,1,1281,kw_178,0.,0.,0.,0,"{Local approximation} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrL"},
		{"multipoint",8,2,2,1,1277,kw_179,0.,0.,0.,0,"{Multipoint approximation} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelSurrMP"}
		},
	kw_181[6] = {
		{"id_model",11,0,1,0,1147,0,0.,0.,0.,0,"{Model set identifier} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelIndControl"},
		{"nested",8,2,4,1,1307,kw_159,0.,0.,0.,0,"[CHOOSE model type]"},
		{"responses_pointer",11,0,3,0,1151,0,0.,0.,0.,0,"{Responses set pointer} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelIndControl"},
		{"single",8,1,4,1,1153,kw_160,0.,0.,0.,0,"@"},
		{"surrogate",8,5,4,1,1157,kw_180},
		{"variables_pointer",11,0,2,0,1149,0,0.,0.,0.,0,"{Variables set pointer} http://dakota.sandia.gov/licensing/votd/html-ref/ModelCommands.html#ModelIndControl"}
		},
	kw_182[5] = {
		{"annotated",8,0,2,0,1727,0,0.,0.,0.,0,"{Data file in annotated format} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS"},
		{"freeform",8,0,2,0,1729,0,0.,0.,0.,0,"{Data file in freeform format} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS"},
		{"num_config_variables",0x29,0,3,0,1731,0,0.,0.,0.,0,"{Number configuration variable columns in file} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS"},
		{"num_experiments",0x29,0,1,0,1725,0,0.,0.,0.,0,"{Number experiments (rows) in file} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS"},
		{"num_std_deviations",0x29,0,4,0,1733,0,0.,0.,0.,0,"{Number standard deviation columns in file} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS"}
		},
	kw_183[3] = {
		{"nonlinear_equality_scale_types",0x80f,0,2,0,1749,0,0.,0.,0.,0,"{Nonlinear equality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"nonlinear_equality_constraints"},
		{"nonlinear_equality_scales",0x80e,0,3,0,1751,0,0.,0.,0.,0,"{Nonlinear equality constraint scales} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"nonlinear_equality_constraints"},
		{"nonlinear_equality_targets",14,0,1,0,1747,0,0.,0.,0.,0,"{Nonlinear equality constraint targets} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"nonlinear_equality_constraints"}
		},
	kw_184[4] = {
		{"nonlinear_inequality_lower_bounds",14,0,1,0,1737,0,0.,0.,0.,0,"{Nonlinear inequality constraint lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"nonlinear_inequality_constraints"},
		{"nonlinear_inequality_scale_types",0x80f,0,3,0,1741,0,0.,0.,0.,0,"{Nonlinear inequality scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"nonlinear_inequality_constraints"},
		{"nonlinear_inequality_scales",0x80e,0,4,0,1743,0,0.,0.,0.,0,"{Nonlinear inequality constraint scales} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"nonlinear_inequality_constraints"},
		{"nonlinear_inequality_upper_bounds",14,0,2,0,1739,0,0.,0.,0.,0,"{Nonlinear inequality constraint upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"nonlinear_inequality_constraints"}
		},
	kw_185[11] = {
		{"calibration_data_file",11,5,4,0,1723,kw_182,0.,0.,0.,0,"{Calibration data file name} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS"},
		{"calibration_term_scale_types",0x80f,0,1,0,1717,0,0.,0.,0.,0,"{Calibration scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"calibration_terms"},
		{"calibration_term_scales",0x80e,0,2,0,1719,0,0.,0.,0.,0,"{Calibration scales} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"calibration_terms"},
		{"calibration_weights",14,0,3,0,1721,0,0.,0.,0.,0,"{Calibration term weights} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"calibration_terms"},
		{"least_squares_term_scale_types",0x807,0,1,0,1716,0,0.,0.,0.,0,0,0,"calibration_terms"},
		{"least_squares_term_scales",0x806,0,2,0,1718,0,0.,0.,0.,0,0,0,"calibration_terms"},
		{"least_squares_weights",6,0,3,0,1720,0,0.,0.,0.,0,0,0,"calibration_terms"},
		{"nonlinear_equality_constraints",0x29,3,6,0,1745,kw_183,0.,0.,0.,0,"{Number of nonlinear equality constraints} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS"},
		{"nonlinear_inequality_constraints",0x29,4,5,0,1735,kw_184,0.,0.,0.,0,"{Number of nonlinear inequality constraints} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS"},
		{"num_nonlinear_equality_constraints",0x21,3,6,0,1744,kw_183},
		{"num_nonlinear_inequality_constraints",0x21,4,5,0,1734,kw_184}
		},
	kw_186[1] = {
		{"ignore_bounds",8,0,1,0,1771,0,0.,0.,0.,0,"{Ignore variable bounds} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespGradMixed"}
		},
	kw_187[10] = {
		{"central",8,0,6,0,1779,0,0.,0.,0.,0,"[CHOOSE difference interval]"},
		{"dakota",8,1,4,0,1769,kw_186,0.,0.,0.,0,"@[CHOOSE gradient source]"},
		{"fd_gradient_step_size",0x406,0,7,0,1780,0,0.,0.,0.001},
		{"fd_step_size",0x40e,0,7,0,1781,0,0.,0.,0.001,0,"{Finite difference step size} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespGradMixed"},
		{"forward",8,0,6,0,1777,0,0.,0.,0.,0,"@"},
		{"id_analytic_gradients",13,0,2,2,1763,0,0.,0.,0.,0,"{Analytic derivatives function list} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespGradMixed"},
		{"id_numerical_gradients",13,0,1,1,1761,0,0.,0.,0.,0,"{Numerical derivatives function list} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespGradMixed"},
		{"interval_type",8,0,5,0,1775,0,0.,0.,0.,0,"{Interval type} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespGradNum"},
		{"method_source",8,0,3,0,1767,0,0.,0.,0.,0,"{Method source} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespGradNum"},
		{"vendor",8,0,4,0,1773}
		},
	kw_188[2] = {
		{"fd_hessian_step_size",6,0,1,0,1806},
		{"fd_step_size",14,0,1,0,1807,0,0.,0.,0.,0,"{Finite difference step size} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespHessMixed"}
		},
	kw_189[1] = {
		{"damped",8,0,1,0,1817,0,0.,0.,0.,0,"{Numerical safeguarding of BFGS update} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespHessMixed"}
		},
	kw_190[2] = {
		{"bfgs",8,1,1,1,1815,kw_189,0.,0.,0.,0,"[CHOOSE Hessian approx.]"},
		{"sr1",8,0,1,1,1819}
		},
	kw_191[5] = {
		{"central",8,0,2,0,1811,0,0.,0.,0.,0,"[CHOOSE difference interval]"},
		{"forward",8,0,2,0,1809,0,0.,0.,0.,0,"@"},
		{"id_analytic_hessians",13,0,4,0,1821,0,0.,0.,0.,0,"{Analytic Hessians function list} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespHessMixed"},
		{"id_numerical_hessians",13,2,1,0,1805,kw_188,0.,0.,0.,0,"{Numerical Hessians function list} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespHessMixed"},
		{"id_quasi_hessians",13,2,3,0,1813,kw_190,0.,0.,0.,0,"{Quasi Hessians function list} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespHessMixed"}
		},
	kw_192[3] = {
		{"nonlinear_equality_scale_types",0x80f,0,2,0,1711,0,0.,0.,0.,0,"{Nonlinear equality constraint scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"nonlinear_equality_constraints"},
		{"nonlinear_equality_scales",0x80e,0,3,0,1713,0,0.,0.,0.,0,"{Nonlinear equality constraint scales} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"nonlinear_equality_constraints"},
		{"nonlinear_equality_targets",14,0,1,0,1709,0,0.,0.,0.,0,"{Nonlinear equality constraint targets} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"nonlinear_equality_constraints"}
		},
	kw_193[4] = {
		{"nonlinear_inequality_lower_bounds",14,0,1,0,1699,0,0.,0.,0.,0,"{Nonlinear inequality constraint lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"nonlinear_inequality_constraints"},
		{"nonlinear_inequality_scale_types",0x80f,0,3,0,1703,0,0.,0.,0.,0,"{Nonlinear inequality constraint scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"nonlinear_inequality_constraints"},
		{"nonlinear_inequality_scales",0x80e,0,4,0,1705,0,0.,0.,0.,0,"{Nonlinear inequality constraint scales} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"nonlinear_inequality_constraints"},
		{"nonlinear_inequality_upper_bounds",14,0,2,0,1701,0,0.,0.,0.,0,"{Nonlinear inequality constraint upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"nonlinear_inequality_constraints"}
		},
	kw_194[7] = {
		{"multi_objective_weights",14,0,3,0,1695,0,0.,0.,0.,0,"{Multiobjective weightings} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"objective_functions"},
		{"nonlinear_equality_constraints",0x29,3,5,0,1707,kw_192,0.,0.,0.,0,"{Number of nonlinear equality constraints} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnOpt"},
		{"nonlinear_inequality_constraints",0x29,4,4,0,1697,kw_193,0.,0.,0.,0,"{Number of nonlinear inequality constraints} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnOpt"},
		{"num_nonlinear_equality_constraints",0x21,3,5,0,1706,kw_192},
		{"num_nonlinear_inequality_constraints",0x21,4,4,0,1696,kw_193},
		{"objective_function_scale_types",0x80f,0,1,0,1691,0,0.,0.,0.,0,"{Objective function scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"objective_functions"},
		{"objective_function_scales",0x80e,0,2,0,1693,0,0.,0.,0.,0,"{Objective function scales} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"objective_functions"}
		},
	kw_195[8] = {
		{"central",8,0,6,0,1779,0,0.,0.,0.,0,"[CHOOSE difference interval]"},
		{"dakota",8,1,4,0,1769,kw_186,0.,0.,0.,0,"@[CHOOSE gradient source]"},
		{"fd_gradient_step_size",0x406,0,7,0,1780,0,0.,0.,0.001},
		{"fd_step_size",0x40e,0,7,0,1781,0,0.,0.,0.001,0,"{Finite difference step size} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespGradMixed"},
		{"forward",8,0,6,0,1777,0,0.,0.,0.,0,"@"},
		{"interval_type",8,0,5,0,1775,0,0.,0.,0.,0,"{Interval type} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespGradNum"},
		{"method_source",8,0,3,0,1767,0,0.,0.,0.,0,"{Method source} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespGradNum"},
		{"vendor",8,0,4,0,1773}
		},
	kw_196[4] = {
		{"central",8,0,2,0,1791,0,0.,0.,0.,0,"[CHOOSE difference interval]"},
		{"fd_hessian_step_size",6,0,1,0,1786},
		{"fd_step_size",14,0,1,0,1787,0,0.,0.,0.,0,"{Finite difference step size} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespHessNum"},
		{"forward",8,0,2,0,1789,0,0.,0.,0.,0,"@"}
		},
	kw_197[1] = {
		{"damped",8,0,1,0,1797,0,0.,0.,0.,0,"{Numerical safeguarding of BFGS update} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespHessQuasi"}
		},
	kw_198[2] = {
		{"bfgs",8,1,1,1,1795,kw_197,0.,0.,0.,0,"[CHOOSE Hessian approx.]"},
		{"sr1",8,0,1,1,1799}
		},
	kw_199[18] = {
		{"analytic_gradients",8,0,4,2,1757,0,0.,0.,0.,0,"[CHOOSE gradient type]"},
		{"analytic_hessians",8,0,5,3,1801,0,0.,0.,0.,0,"[CHOOSE Hessian type]"},
		{"calibration_terms",0x29,11,3,1,1715,kw_185,0.,0.,0.,0,"{Number of calibration terms} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnLS"},
		{"descriptors",15,0,2,0,1687,0,0.,0.,0.,0,"{Response labels} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespLabels"},
		{"id_responses",11,0,1,0,1685,0,0.,0.,0.,0,"{Responses set identifier} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespSetId"},
		{"mixed_gradients",8,10,4,2,1759,kw_187,0.,0.,0.,0,"{Mixed gradients} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespGradMixed"},
		{"mixed_hessians",8,5,5,3,1803,kw_191,0.,0.,0.,0,"{Mixed Hessians} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespHessMixed"},
		{"no_gradients",8,0,4,2,1755,0,0.,0.,0.,0,"@"},
		{"no_hessians",8,0,5,3,1783,0,0.,0.,0.,0,"@"},
		{"num_least_squares_terms",0x21,11,3,1,1714,kw_185},
		{"num_objective_functions",0x21,7,3,1,1688,kw_194},
		{"num_response_functions",0x21,0,3,1,1752},
		{"numerical_gradients",8,8,4,2,1765,kw_195,0.,0.,0.,0,"{Numerical gradients} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespGradNum"},
		{"numerical_hessians",8,4,5,3,1785,kw_196,0.,0.,0.,0,"{Numerical Hessians} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespHessNum"},
		{"objective_functions",0x29,7,3,1,1689,kw_194,0.,0.,0.,0,"{Number of objective functions} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnOpt"},
		{"quasi_hessians",8,2,5,3,1793,kw_198,0.,0.,0.,0,"{Quasi Hessians} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespHessQuasi"},
		{"response_descriptors",7,0,2,0,1686},
		{"response_functions",0x29,0,3,1,1753,0,0.,0.,0.,0,"{Number of response functions} http://dakota.sandia.gov/licensing/votd/html-ref/RespCommands.html#RespFnGen"}
		},
	kw_200[1] = {
		{"method_list",15,0,1,1,33,0,0.,0.,0.,0,"{List of methods} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratHybrid"}
		},
	kw_201[3] = {
		{"global_method_pointer",11,0,1,1,25,0,0.,0.,0.,0,"{Pointer to the global method specification} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"local_method_pointer",11,0,2,2,27,0,0.,0.,0.,0,"{Pointer to the local method specification} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"local_search_probability",10,0,3,0,29,0,0.,0.,0.,0,"{Probability of executing local searches} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratHybrid"}
		},
	kw_202[1] = {
		{"method_list",15,0,1,1,21,0,0.,0.,0.,0,"{List of methods} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratHybrid"}
		},
	kw_203[5] = {
		{"collaborative",8,1,1,1,31,kw_200,0.,0.,0.,0,"[CHOOSE hybrid type]{Collaborative hybrid} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"coupled",0,3,1,1,22,kw_201},
		{"embedded",8,3,1,1,23,kw_201,0.,0.,0.,0,"{Embedded hybrid} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"sequential",8,1,1,1,19,kw_202,0.,0.,0.,0,"{Sequential hybrid} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"uncoupled",0,1,1,1,18,kw_202}
		},
	kw_204[1] = {
		{"seed",9,0,1,0,41,0,0.,0.,0.,0,"{Seed for random starting points} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratMultiStart"}
		},
	kw_205[3] = {
		{"method_pointer",11,0,1,1,37,0,0.,0.,0.,0,"{Method pointer} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratMultiStart"},
		{"random_starts",9,1,2,0,39,kw_204,0.,0.,0.,0,"{Number of random starting points} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratMultiStart"},
		{"starting_points",14,0,3,0,43,0,0.,0.,0.,0,"{List of user-specified starting points} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratMultiStart"}
		},
	kw_206[1] = {
		{"seed",9,0,1,0,51,0,0.,0.,0.,0,"{Seed for random weighting sets} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratParetoSet"}
		},
	kw_207[5] = {
		{"method_pointer",11,0,1,1,47,0,0.,0.,0.,0,"{Optimization method pointer} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratParetoSet"},
		{"multi_objective_weight_sets",6,0,3,0,52},
		{"opt_method_pointer",3,0,1,1,46},
		{"random_weight_sets",9,1,2,0,49,kw_206,0.,0.,0.,0,"{Number of random weighting sets} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratParetoSet"},
		{"weight_sets",14,0,3,0,53,0,0.,0.,0.,0,"{List of user-specified weighting sets} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratParetoSet"}
		},
	kw_208[1] = {
		{"method_pointer",11,0,1,0,57,0,0.,0.,0.,0,"{Method pointer} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratSingle"}
		},
	kw_209[1] = {
		{"tabular_graphics_file",11,0,1,0,7,0,0.,0.,0.,0,"{File name for tabular graphics data} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratIndControl"}
		},
	kw_210[10] = {
		{"graphics",8,0,1,0,3,0,0.,0.,0.,0,"{Graphics flag} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratIndControl"},
		{"hybrid",8,5,7,1,17,kw_203,0.,0.,0.,0,"[CHOOSE strategy type]{Hybrid strategy} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"iterator_self_scheduling",8,0,5,0,13,0,0.,0.,0.,0,"{Self-scheduling of iterator jobs} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratIndControl"},
		{"iterator_servers",9,0,4,0,11,0,0.,0.,0.,0,"{Number of iterator servers} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratIndControl"},
		{"iterator_static_scheduling",8,0,6,0,15,0,0.,0.,0.,0,"{Static scheduling of iterator jobs} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratIndControl"},
		{"multi_start",8,3,7,1,35,kw_205,0.,0.,0.,0,"{Multi-start iteration strategy} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratMultiStart"},
		{"output_precision",0x29,0,3,0,9,0,0.,0.,0.,0,"{Numeric output precision} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratIndControl"},
		{"pareto_set",8,5,7,1,45,kw_207,0.,0.,0.,0,"{Pareto set optimization strategy} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratParetoSet"},
		{"single_method",8,1,7,1,55,kw_208,0.,0.,0.,0,"@{Single method strategy} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratSingle"},
		{"tabular_graphics_data",8,1,2,0,5,kw_209,0.,0.,0.,0,"{Tabulation of graphics data} http://dakota.sandia.gov/licensing/votd/html-ref/StratCommands.html#StratIndControl"}
		},
	kw_211[10] = {
		{"alphas",14,0,1,1,1435,0,0.,0.,0.,0,"{beta uncertain alphas} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta",0,"beta_uncertain"},
		{"betas",14,0,2,2,1437,0,0.,0.,0.,0,"{beta uncertain betas} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta",0,"beta_uncertain"},
		{"buv_alphas",6,0,1,1,1434,0,0.,0.,0.,0,0,0,"beta_uncertain"},
		{"buv_betas",6,0,2,2,1436,0,0.,0.,0.,0,0,0,"beta_uncertain"},
		{"buv_descriptors",7,0,5,0,1442,0,0.,0.,0.,0,0,0,"beta_uncertain"},
		{"buv_lower_bounds",6,0,3,3,1438,0,0.,0.,0.,0,0,0,"beta_uncertain"},
		{"buv_upper_bounds",6,0,4,4,1440,0,0.,0.,0.,0,0,0,"beta_uncertain"},
		{"descriptors",15,0,5,0,1443,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta",0,"beta_uncertain"},
		{"lower_bounds",14,0,3,3,1439,0,0.,0.,0.,0,"{Distribution lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta",0,"beta_uncertain"},
		{"upper_bounds",14,0,4,4,1441,0,0.,0.,0.,0,"{Distribution upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta",0,"beta_uncertain"}
		},
	kw_212[3] = {
		{"descriptors",15,0,3,0,1501,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Binomial",0,"binomial_uncertain"},
		{"num_trials",13,0,2,2,1499,0,0.,0.,0.,0,"{binomial uncertain num_trials} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Binomial",0,"binomial_uncertain"},
		{"prob_per_trial",14,0,1,1,1497,0,0.,0.,0.,0,"{binomial uncertain prob_per_trial} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Binomial",0,"binomial_uncertain"}
		},
	kw_213[12] = {
		{"cdv_descriptors",7,0,6,0,1338,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"cdv_initial_point",6,0,1,0,1328,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"cdv_lower_bounds",6,0,2,0,1330,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"cdv_scale_types",0x807,0,4,0,1334,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"cdv_scales",0x806,0,5,0,1336,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"cdv_upper_bounds",6,0,3,0,1332,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"descriptors",15,0,6,0,1339,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"},
		{"initial_point",14,0,1,0,1329,0,0.,0.,0.,0,"{Initial point} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"},
		{"lower_bounds",14,0,2,0,1331,0,0.,0.,0.,0,"{Lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"},
		{"scale_types",0x80f,0,4,0,1335,0,0.,0.,0.,0,"{Scaling types} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"},
		{"scales",0x80e,0,5,0,1337,0,0.,0.,0.,0,"{Scales} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"},
		{"upper_bounds",14,0,3,0,1333,0,0.,0.,0.,0,"{Upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"}
		},
	kw_214[8] = {
		{"csv_descriptors",7,0,4,0,1556,0,0.,0.,0.,0,0,0,"continuous_state"},
		{"csv_initial_state",6,0,1,0,1550,0,0.,0.,0.,0,0,0,"continuous_state"},
		{"csv_lower_bounds",6,0,2,0,1552,0,0.,0.,0.,0,0,0,"continuous_state"},
		{"csv_upper_bounds",6,0,3,0,1554,0,0.,0.,0.,0,0,0,"continuous_state"},
		{"descriptors",15,0,4,0,1557,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCSV",0,"continuous_state"},
		{"initial_state",14,0,1,0,1551,0,0.,0.,0.,0,"{Initial states} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCSV",0,"continuous_state"},
		{"lower_bounds",14,0,2,0,1553,0,0.,0.,0.,0,"{Lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCSV",0,"continuous_state"},
		{"upper_bounds",14,0,3,0,1555,0,0.,0.,0.,0,"{Upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCSV",0,"continuous_state"}
		},
	kw_215[8] = {
		{"ddv_descriptors",7,0,4,0,1348,0,0.,0.,0.,0,0,0,"discrete_design_range"},
		{"ddv_initial_point",5,0,1,0,1342,0,0.,0.,0.,0,0,0,"discrete_design_range"},
		{"ddv_lower_bounds",5,0,2,0,1344,0,0.,0.,0.,0,0,0,"discrete_design_range"},
		{"ddv_upper_bounds",5,0,3,0,1346,0,0.,0.,0.,0,0,0,"discrete_design_range"},
		{"descriptors",15,0,4,0,1349,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDDRIV",0,"discrete_design_range"},
		{"initial_point",13,0,1,0,1343,0,0.,0.,0.,0,"{Initial point} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDDRIV",0,"discrete_design_range"},
		{"lower_bounds",13,0,2,0,1345,0,0.,0.,0.,0,"{Lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDDRIV",0,"discrete_design_range"},
		{"upper_bounds",13,0,3,0,1347,0,0.,0.,0.,0,"{Upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDDRIV",0,"discrete_design_range"}
		},
	kw_216[4] = {
		{"descriptors",15,0,4,0,1359,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDDSIV",0,"discrete_design_set_integer"},
		{"initial_point",13,0,1,0,1353,0,0.,0.,0.,0,"{Initial point} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDDSIV",0,"discrete_design_set_integer"},
		{"num_set_values",13,0,2,0,1355,0,0.,0.,0.,0,"{Number of values for each variable} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDDSIV",0,"discrete_design_set_integer"},
		{"set_values",13,0,3,1,1357,0,0.,0.,0.,0,"{Set values} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDDSIV"}
		},
	kw_217[4] = {
		{"descriptors",15,0,4,0,1369,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDDSRV",0,"discrete_design_set_real"},
		{"initial_point",14,0,1,0,1363,0,0.,0.,0.,0,"{Initial point} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDDSRV",0,"discrete_design_set_real"},
		{"num_set_values",13,0,2,0,1365,0,0.,0.,0.,0,"{Number of values for each variable} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDDSRV",0,"discrete_design_set_real"},
		{"set_values",14,0,3,1,1367,0,0.,0.,0.,0,"{Set values} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDDSRV"}
		},
	kw_218[8] = {
		{"descriptors",15,0,4,0,1567,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDSRIV",0,"discrete_state_range"},
		{"dsv_descriptors",7,0,4,0,1566,0,0.,0.,0.,0,0,0,"discrete_state_range"},
		{"dsv_initial_state",5,0,1,0,1560,0,0.,0.,0.,0,0,0,"discrete_state_range"},
		{"dsv_lower_bounds",5,0,2,0,1562,0,0.,0.,0.,0,0,0,"discrete_state_range"},
		{"dsv_upper_bounds",5,0,3,0,1564,0,0.,0.,0.,0,0,0,"discrete_state_range"},
		{"initial_state",13,0,1,0,1561,0,0.,0.,0.,0,"{Initial states} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDSRIV",0,"discrete_state_range"},
		{"lower_bounds",13,0,2,0,1563,0,0.,0.,0.,0,"{Lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDSRIV",0,"discrete_state_range"},
		{"upper_bounds",13,0,3,0,1565,0,0.,0.,0.,0,"{Upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDSRIV",0,"discrete_state_range"}
		},
	kw_219[4] = {
		{"descriptors",15,0,4,0,1577,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDSSIV",0,"discrete_state_set_integer"},
		{"initial_state",13,0,1,0,1571,0,0.,0.,0.,0,"{Initial state} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDSSIV",0,"discrete_state_set_integer"},
		{"num_set_values",13,0,2,0,1573,0,0.,0.,0.,0,"{Number of values for each variable} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDSSIV",0,"discrete_state_set_integer"},
		{"set_values",13,0,3,1,1575,0,0.,0.,0.,0,"{Set values} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDSSIV"}
		},
	kw_220[4] = {
		{"descriptors",15,0,4,0,1587,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDSSRV",0,"discrete_state_set_real"},
		{"initial_state",14,0,1,0,1581,0,0.,0.,0.,0,"{Initial state} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDSSRV",0,"discrete_state_set_real"},
		{"num_set_values",13,0,2,0,1583,0,0.,0.,0.,0,"{Number of values for each variable} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDSSRV",0,"discrete_state_set_real"},
		{"set_values",14,0,3,1,1585,0,0.,0.,0.,0,"{Set values} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDSSRV"}
		},
	kw_221[4] = {
		{"betas",14,0,1,1,1429,0,0.,0.,0.,0,"{exponential uncertain betas} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Exponential",0,"exponential_uncertain"},
		{"descriptors",15,0,2,0,1431,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Exponential",0,"exponential_uncertain"},
		{"euv_betas",6,0,1,1,1428,0,0.,0.,0.,0,0,0,"exponential_uncertain"},
		{"euv_descriptors",7,0,2,0,1430,0,0.,0.,0.,0,0,0,"exponential_uncertain"}
		},
	kw_222[6] = {
		{"alphas",14,0,1,1,1463,0,0.,0.,0.,0,"{frechet uncertain alphas} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Frechet",0,"frechet_uncertain"},
		{"betas",14,0,2,2,1465,0,0.,0.,0.,0,"{frechet uncertain betas} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Frechet",0,"frechet_uncertain"},
		{"descriptors",15,0,3,0,1467,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Frechet",0,"frechet_uncertain"},
		{"fuv_alphas",6,0,1,1,1462,0,0.,0.,0.,0,0,0,"frechet_uncertain"},
		{"fuv_betas",6,0,2,2,1464,0,0.,0.,0.,0,0,0,"frechet_uncertain"},
		{"fuv_descriptors",7,0,3,0,1466,0,0.,0.,0.,0,0,0,"frechet_uncertain"}
		},
	kw_223[6] = {
		{"alphas",14,0,1,1,1447,0,0.,0.,0.,0,"{gamma uncertain alphas} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gamma",0,"gamma_uncertain"},
		{"betas",14,0,2,2,1449,0,0.,0.,0.,0,"{gamma uncertain betas} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gamma",0,"gamma_uncertain"},
		{"descriptors",15,0,3,0,1451,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gamma",0,"gamma_uncertain"},
		{"gauv_alphas",6,0,1,1,1446,0,0.,0.,0.,0,0,0,"gamma_uncertain"},
		{"gauv_betas",6,0,2,2,1448,0,0.,0.,0.,0,0,0,"gamma_uncertain"},
		{"gauv_descriptors",7,0,3,0,1450,0,0.,0.,0.,0,0,0,"gamma_uncertain"}
		},
	kw_224[2] = {
		{"descriptors",15,0,2,0,1515,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Geometric",0,"geometric_uncertain"},
		{"prob_per_trial",14,0,1,1,1513,0,0.,0.,0.,0,"{geometric uncertain prob_per_trial} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Geometric",0,"geometric_uncertain"}
		},
	kw_225[6] = {
		{"alphas",14,0,1,1,1455,0,0.,0.,0.,0,"{gumbel uncertain alphas} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gumbel",0,"gumbel_uncertain"},
		{"betas",14,0,2,2,1457,0,0.,0.,0.,0,"{gumbel uncertain betas} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gumbel",0,"gumbel_uncertain"},
		{"descriptors",15,0,3,0,1459,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gumbel",0,"gumbel_uncertain"},
		{"guuv_alphas",6,0,1,1,1454,0,0.,0.,0.,0,0,0,"gumbel_uncertain"},
		{"guuv_betas",6,0,2,2,1456,0,0.,0.,0.,0,0,0,"gumbel_uncertain"},
		{"guuv_descriptors",7,0,3,0,1458,0,0.,0.,0.,0,0,0,"gumbel_uncertain"}
		},
	kw_226[10] = {
		{"abscissas",14,0,2,1,1481,0,0.,0.,0.,0,"{sets of abscissas for bin-based histogram variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram"},
		{"counts",14,0,3,2,1485,0,0.,0.,0.,0,"{sets of counts for bin-based histogram variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram"},
		{"descriptors",15,0,4,0,1487,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram",0,"histogram_bin_uncertain"},
		{"huv_bin_abscissas",6,0,2,1,1480},
		{"huv_bin_counts",6,0,3,2,1484},
		{"huv_bin_descriptors",7,0,4,0,1486,0,0.,0.,0.,0,0,0,"histogram_bin_uncertain"},
		{"huv_bin_ordinates",6,0,3,2,1482},
		{"huv_num_bin_pairs",5,0,1,0,1478,0,0.,0.,0.,0,0,0,"histogram_bin_uncertain"},
		{"num_pairs",13,0,1,0,1479,0,0.,0.,0.,0,"{key to apportionment among bin-based histogram variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram",0,"histogram_bin_uncertain"},
		{"ordinates",14,0,3,2,1483,0,0.,0.,0.,0,"{sets of ordinates for bin-based histogram variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram"}
		},
	kw_227[8] = {
		{"abscissas",14,0,2,1,1531,0,0.,0.,0.,0,"{sets of abscissas for point-based histogram variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Point_Histogram"},
		{"counts",14,0,3,2,1533,0,0.,0.,0.,0,"{sets of counts for point-based histogram variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Point_Histogram"},
		{"descriptors",15,0,4,0,1535,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Point_Histogram",0,"histogram_point_uncertain"},
		{"huv_num_point_pairs",5,0,1,0,1528,0,0.,0.,0.,0,0,0,"histogram_point_uncertain"},
		{"huv_point_abscissas",6,0,2,1,1530},
		{"huv_point_counts",6,0,3,2,1532},
		{"huv_point_descriptors",7,0,4,0,1534,0,0.,0.,0.,0,0,0,"histogram_point_uncertain"},
		{"num_pairs",13,0,1,0,1529,0,0.,0.,0.,0,"{key to apportionment among point-based histogram variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Point_Histogram",0,"histogram_point_uncertain"}
		},
	kw_228[4] = {
		{"descriptors",15,0,4,0,1525,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Hypergeometric",0,"hypergeometric_uncertain"},
		{"num_drawn",13,0,3,3,1523,0,0.,0.,0.,0,"{hypergeometric uncertain num_drawn } http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Hypergeometric",0,"hypergeometric_uncertain"},
		{"selected_population",13,0,2,2,1521,0,0.,0.,0.,0,"{hypergeometric uncertain selected_population} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Hypergeometric",0,"hypergeometric_uncertain"},
		{"total_population",13,0,1,1,1519,0,0.,0.,0.,0,"{hypergeometric uncertain total_population} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Hypergeometric",0,"hypergeometric_uncertain"}
		},
	kw_229[8] = {
		{"descriptors",15,0,4,0,1547,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCEUV_Interval",0,"interval_uncertain"},
		{"interval_bounds",14,0,3,2,1545,0,0.,0.,0.,0,"{bounds per interval} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCEUV_Interval"},
		{"interval_probs",14,0,2,1,1543,0,0.,0.,0.,0,"{basic probability assignments per interval} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCEUV_Interval"},
		{"iuv_descriptors",7,0,4,0,1546,0,0.,0.,0.,0,0,0,"interval_uncertain"},
		{"iuv_interval_bounds",6,0,3,2,1544},
		{"iuv_interval_probs",6,0,2,1,1542},
		{"iuv_num_intervals",5,0,1,0,1540,0,0.,0.,0.,0,0,0,"interval_uncertain"},
		{"num_intervals",13,0,1,0,1541,0,0.,0.,0.,0,"{number of intervals defined for each interval variable} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCEUV_Interval",0,"interval_uncertain"}
		},
	kw_230[2] = {
		{"lnuv_zetas",6,0,1,1,1386,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"zetas",14,0,1,1,1387,0,0.,0.,0.,0,"{lognormal uncertain zetas} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"}
		},
	kw_231[4] = {
		{"error_factors",14,0,1,1,1393,0,0.,0.,0.,0,"[CHOOSE variance spec.]{lognormal uncertain error factors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"},
		{"lnuv_error_factors",6,0,1,1,1392,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lnuv_std_deviations",6,0,1,1,1390,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"std_deviations",14,0,1,1,1391,0,0.,0.,0.,0,"@{lognormal uncertain standard deviations} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"}
		},
	kw_232[10] = {
		{"descriptors",15,0,4,0,1399,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"},
		{"lambdas",14,2,1,1,1385,kw_230,0.,0.,0.,0,"[CHOOSE characterization]{lognormal uncertain lambdas} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"},
		{"lnuv_descriptors",7,0,4,0,1398,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lnuv_lambdas",6,2,1,1,1384,kw_230,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lnuv_lower_bounds",6,0,2,0,1394,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lnuv_means",6,4,1,1,1388,kw_231,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lnuv_upper_bounds",6,0,3,0,1396,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lower_bounds",14,0,2,0,1395,0,0.,0.,0.,0,"{Distribution lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"},
		{"means",14,4,1,1,1389,kw_231,0.,0.,0.,0,"@{lognormal uncertain means} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"},
		{"upper_bounds",14,0,3,0,1397,0,0.,0.,0.,0,"{Distribution upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"}
		},
	kw_233[6] = {
		{"descriptors",15,0,3,0,1415,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Loguniform",0,"loguniform_uncertain"},
		{"lower_bounds",14,0,1,1,1411,0,0.,0.,0.,0,"{Distribution lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Loguniform",0,"loguniform_uncertain"},
		{"luuv_descriptors",7,0,3,0,1414,0,0.,0.,0.,0,0,0,"loguniform_uncertain"},
		{"luuv_lower_bounds",6,0,1,1,1410,0,0.,0.,0.,0,0,0,"loguniform_uncertain"},
		{"luuv_upper_bounds",6,0,2,2,1412,0,0.,0.,0.,0,0,0,"loguniform_uncertain"},
		{"upper_bounds",14,0,2,2,1413,0,0.,0.,0.,0,"{Distribution upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Loguniform",0,"loguniform_uncertain"}
		},
	kw_234[3] = {
		{"descriptors",15,0,3,0,1509,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Negative_Binomial",0,"negative_binomial_uncertain"},
		{"num_trials",13,0,2,2,1507,0,0.,0.,0.,0,"{negative binomial uncertain success num_trials} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Negative_Binomial",0,"negative_binomial_uncertain"},
		{"prob_per_trial",14,0,1,1,1505,0,0.,0.,0.,0,"{negative binomial uncertain success prob_per_trial} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Negative_Binomial",0,"negative_binomial_uncertain"}
		},
	kw_235[10] = {
		{"descriptors",15,0,5,0,1381,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal",0,"normal_uncertain"},
		{"lower_bounds",14,0,3,0,1377,0,0.,0.,0.,0,"{Distribution lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal",0,"normal_uncertain"},
		{"means",14,0,1,1,1373,0,0.,0.,0.,0,"{normal uncertain means} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal",0,"normal_uncertain"},
		{"nuv_descriptors",7,0,5,0,1380,0,0.,0.,0.,0,0,0,"normal_uncertain"},
		{"nuv_lower_bounds",6,0,3,0,1376,0,0.,0.,0.,0,0,0,"normal_uncertain"},
		{"nuv_means",6,0,1,1,1372,0,0.,0.,0.,0,0,0,"normal_uncertain"},
		{"nuv_std_deviations",6,0,2,2,1374,0,0.,0.,0.,0,0,0,"normal_uncertain"},
		{"nuv_upper_bounds",6,0,4,0,1378,0,0.,0.,0.,0,0,0,"normal_uncertain"},
		{"std_deviations",14,0,2,2,1375,0,0.,0.,0.,0,"{normal uncertain standard deviations} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal",0,"normal_uncertain"},
		{"upper_bounds",14,0,4,0,1379,0,0.,0.,0.,0,"{Distribution upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal",0,"normal_uncertain"}
		},
	kw_236[2] = {
		{"descriptors",15,0,2,0,1493,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Poisson",0,"poisson_uncertain"},
		{"lambdas",14,0,1,1,1491,0,0.,0.,0.,0,"{poisson uncertain lambdas} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Poisson",0,"poisson_uncertain"}
		},
	kw_237[8] = {
		{"descriptors",15,0,4,0,1425,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Triangular",0,"triangular_uncertain"},
		{"lower_bounds",14,0,2,2,1421,0,0.,0.,0.,0,"{Distribution lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Triangular",0,"triangular_uncertain"},
		{"modes",14,0,1,1,1419,0,0.,0.,0.,0,"{triangular uncertain modes} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Triangular",0,"triangular_uncertain"},
		{"tuv_descriptors",7,0,4,0,1424,0,0.,0.,0.,0,0,0,"triangular_uncertain"},
		{"tuv_lower_bounds",6,0,2,2,1420,0,0.,0.,0.,0,0,0,"triangular_uncertain"},
		{"tuv_modes",6,0,1,1,1418,0,0.,0.,0.,0,0,0,"triangular_uncertain"},
		{"tuv_upper_bounds",6,0,3,3,1422,0,0.,0.,0.,0,0,0,"triangular_uncertain"},
		{"upper_bounds",14,0,3,3,1423,0,0.,0.,0.,0,"{Distribution upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Triangular",0,"triangular_uncertain"}
		},
	kw_238[6] = {
		{"descriptors",15,0,3,0,1407,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Uniform",0,"uniform_uncertain"},
		{"lower_bounds",14,0,1,1,1403,0,0.,0.,0.,0,"{Distribution lower bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Uniform",0,"uniform_uncertain"},
		{"upper_bounds",14,0,2,2,1405,0,0.,0.,0.,0,"{Distribution upper bounds} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Uniform",0,"uniform_uncertain"},
		{"uuv_descriptors",7,0,3,0,1406,0,0.,0.,0.,0,0,0,"uniform_uncertain"},
		{"uuv_lower_bounds",6,0,1,1,1402,0,0.,0.,0.,0,0,0,"uniform_uncertain"},
		{"uuv_upper_bounds",6,0,2,2,1404,0,0.,0.,0.,0,0,0,"uniform_uncertain"}
		},
	kw_239[6] = {
		{"alphas",14,0,1,1,1471,0,0.,0.,0.,0,"{weibull uncertain alphas} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Weibull",0,"weibull_uncertain"},
		{"betas",14,0,2,2,1473,0,0.,0.,0.,0,"{weibull uncertain betas} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Weibull",0,"weibull_uncertain"},
		{"descriptors",15,0,3,0,1475,0,0.,0.,0.,0,"{Descriptors} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Weibull",0,"weibull_uncertain"},
		{"wuv_alphas",6,0,1,1,1470,0,0.,0.,0.,0,0,0,"weibull_uncertain"},
		{"wuv_betas",6,0,2,2,1472,0,0.,0.,0.,0,0,0,"weibull_uncertain"},
		{"wuv_descriptors",7,0,3,0,1474,0,0.,0.,0.,0,0,0,"weibull_uncertain"}
		},
	kw_240[29] = {
		{"beta_uncertain",0x19,10,12,0,1433,kw_211,0.,0.,0.,0,"{beta uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta","Continuous Aleatory Uncertain"},
		{"binomial_uncertain",0x19,3,19,0,1495,kw_212,0.,0.,0.,0,"{binomial uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Binomial","Discrete Aleatory Uncertain"},
		{"continuous_design",0x19,12,2,0,1327,kw_213,0.,0.,0.,0,"{Continuous design variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCDV","Design Variables"},
		{"continuous_state",0x19,8,26,0,1549,kw_214,0.,0.,0.,0,"{Continuous state variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCSV","State Variables"},
		{"discrete_design_range",0x19,8,3,0,1341,kw_215,0.,0.,0.,0,"{Discrete design range variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDDRIV","Design Variables"},
		{"discrete_design_set_integer",0x19,4,4,0,1351,kw_216,0.,0.,0.,0,"{Discrete design set of integer variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDDSIV","Design Variables"},
		{"discrete_design_set_real",0x19,4,5,0,1361,kw_217,0.,0.,0.,0,"{Discrete design set of real variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDDSRV","Design Variables"},
		{"discrete_state_range",0x19,8,27,0,1559,kw_218,0.,0.,0.,0,"{Discrete state range variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDSRIV","State Variables"},
		{"discrete_state_set_integer",0x19,4,28,0,1569,kw_219,0.,0.,0.,0,"{Discrete state set of integer variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDSSIV","State Variables"},
		{"discrete_state_set_real",0x19,4,29,0,1579,kw_220,0.,0.,0.,0,"{Discrete state set of real variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDSSRV","State Variables"},
		{"exponential_uncertain",0x19,4,11,0,1427,kw_221,0.,0.,0.,0,"{exponential uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Exponential","Continuous Aleatory Uncertain"},
		{"frechet_uncertain",0x19,6,15,0,1461,kw_222,0.,0.,0.,0,"{frechet uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Frechet","Continuous Aleatory Uncertain"},
		{"gamma_uncertain",0x19,6,13,0,1445,kw_223,0.,0.,0.,0,"{gamma uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gamma","Continuous Aleatory Uncertain"},
		{"geometric_uncertain",0x19,2,21,0,1511,kw_224,0.,0.,0.,0,"{geometric uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Geometric","Discrete Aleatory Uncertain"},
		{"gumbel_uncertain",0x19,6,14,0,1453,kw_225,0.,0.,0.,0,"{gumbel uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gumbel","Continuous Aleatory Uncertain"},
		{"histogram_bin_uncertain",0x19,10,17,0,1477,kw_226,0.,0.,0.,0,"{histogram bin uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram","Continuous Aleatory Uncertain"},
		{"histogram_point_uncertain",0x19,8,23,0,1527,kw_227,0.,0.,0.,0,"{histogram point uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Point_Histogram","Discrete Aleatory Uncertain"},
		{"hypergeometric_uncertain",0x19,4,22,0,1517,kw_228,0.,0.,0.,0,"{hypergeometric uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Hypergeometric","Discrete Aleatory Uncertain"},
		{"id_variables",11,0,1,0,1325,0,0.,0.,0.,0,"{Variables set identifier} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarSetId"},
		{"interval_uncertain",0x19,8,25,0,1539,kw_229,0.,0.,0.,0,"{interval uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCEUV_Interval","Epistemic Uncertain"},
		{"lognormal_uncertain",0x19,10,7,0,1383,kw_232,0.,0.,0.,0,"{lognormal uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal","Continuous Aleatory Uncertain"},
		{"loguniform_uncertain",0x19,6,9,0,1409,kw_233,0.,0.,0.,0,"{loguniform uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Loguniform","Continuous Aleatory Uncertain"},
		{"negative_binomial_uncertain",0x19,3,20,0,1503,kw_234,0.,0.,0.,0,"{negative binomial uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Negative_Binomial","Discrete Aleatory Uncertain"},
		{"normal_uncertain",0x19,10,6,0,1371,kw_235,0.,0.,0.,0,"{normal uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal","Continuous Aleatory Uncertain"},
		{"poisson_uncertain",0x19,2,18,0,1489,kw_236,0.,0.,0.,0,"{poisson uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarDAUV_Poisson","Discrete Aleatory Uncertain"},
		{"triangular_uncertain",0x19,8,10,0,1417,kw_237,0.,0.,0.,0,"{triangular uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Triangular","Continuous Aleatory Uncertain"},
		{"uncertain_correlation_matrix",14,0,24,0,1537,0,0.,0.,0.,0,"{correlations in aleatory uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarAUV_Correlations","Aleatory Uncertain Correlations"},
		{"uniform_uncertain",0x19,6,8,0,1401,kw_238,0.,0.,0.,0,"{uniform uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Uniform","Continuous Aleatory Uncertain"},
		{"weibull_uncertain",0x19,6,16,0,1469,kw_239,0.,0.,0.,0,"{weibull uncertain variables} http://dakota.sandia.gov/licensing/votd/html-ref/VarCommands.html#VarCAUV_Weibull","Continuous Aleatory Uncertain"}
		},
	kw_241[6] = {
		{"interface",0x308,10,5,5,1589,kw_9,0.,0.,0.,0,"{Interface} An interface specifies how function evaluations will be performed in order to map a set of parameters into a set of responses. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/InterfCommands.html"},
		{"method",0x308,75,2,2,59,kw_156,0.,0.,0.,0,"{Method} A method specifies the name and controls of an iterative procedure, e.g., a sensitivity analysis, uncertainty quantification, or optimization method. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/MethodCommands.html"},
		{"model",8,6,3,3,1145,kw_181,0.,0.,0.,0,"{Model} A model consists of a model type and maps specified variables through an interface to generate responses. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/ModelCommands.html"},
		{"responses",0x308,18,6,6,1683,kw_199,0.,0.,0.,0,"{Responses} A responses object specifies the data that can be returned to DAKOTA through the interface after the completion of a function evaluation. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/RespCommands.html"},
		{"strategy",0x108,10,1,1,1,kw_210,0.,0.,0.,0,"{Strategy} The strategy specifies the top level technique which will govern the management of iterators and models in the solution of the problem of interest. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/StratCommands.html"},
		{"variables",0x308,29,4,4,1323,kw_240,0.,0.,0.,0,"{Variables} A variables object specifies the parameter set to be iterated by a particular method. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/VarCommands.html"}
		};

} // namespace Dakota

KeyWord Dakota_Keyword_Top = {"KeywordTop",0,6,0,0,Dakota::kw_241};

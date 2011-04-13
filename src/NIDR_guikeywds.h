
namespace Dakota {

/** 871 distinct keywords (plus 103 aliases) **/

static GuiKeyWord
	kw_1[3] = {
		{"active_set_vector",8,0,1,0,1585},
		{"evaluation_cache",8,0,2,0,1587},
		{"restart_file",8,0,3,0,1589}
		},
	kw_2[1] = {
		{"processors_per_analysis",9,0,1,0,1569,0,0.,0.,0.,0,"{Number of processors per analysis} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicDF"}
		},
	kw_3[4] = {
		{"abort",8,0,1,1,1575,0,0.,0.,0.,0,"@[CHOOSE failure mitigation]"},
		{"continuation",8,0,1,1,1581},
		{"recover",14,0,1,1,1579},
		{"retry",9,0,1,1,1577}
		},
	kw_4[2] = {
		{"copy",8,0,1,0,1563,0,0.,0.,0.,0,"{Copy template files} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"replace",8,0,2,0,1565,0,0.,0.,0.,0,"{Replace existing files} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"}
		},
	kw_5[7] = {
		{"dir_save",0,0,3,0,1556},
		{"dir_tag",0,0,2,0,1554},
		{"directory_save",8,0,3,0,1557,0,0.,0.,0.,0,"{Save work directory} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"directory_tag",8,0,2,0,1555,0,0.,0.,0.,0,"{Tag work directory} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"named",11,0,1,0,1553,0,0.,0.,0.,0,"{Name of work directory} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"template_directory",11,2,4,0,1559,kw_4,0.,0.,0.,0,"{Template directory} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"template_files",15,2,4,0,1561,kw_4,0.,0.,0.,0,"{Template files} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"}
		},
	kw_6[8] = {
		{"allow_existing_results",8,0,3,0,1541,0,0.,0.,0.,0,"{Allow existing results files} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"aprepro",8,0,5,0,1545,0,0.,0.,0.,0,"{Aprepro parameters file format} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"file_save",8,0,7,0,1549,0,0.,0.,0.,0,"{Parameters and results file saving} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"file_tag",8,0,6,0,1547,0,0.,0.,0.,0,"{Parameters and results file tagging} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"parameters_file",11,0,1,0,1537,0,0.,0.,0.,0,"{Parameters file name} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"results_file",11,0,2,0,1539,0,0.,0.,0.,0,"{Results file name} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"verbatim",8,0,4,0,1543,0,0.,0.,0.,0,"{Verbatim driver/filter invocation syntax} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"work_directory",8,7,8,0,1551,kw_5,0.,0.,0.,0,"{Create work directory} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"}
		},
	kw_7[9] = {
		{"analysis_components",15,0,1,0,1527,0,0.,0.,0.,0,"{Additional identifiers for use by the analysis_drivers} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"deactivate",8,3,6,0,1583,kw_1,0.,0.,0.,0,"{Feature deactivation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"direct",8,1,4,1,1567,kw_2,0.,0.,0.,0,"[CHOOSE interface type]{Direct function interface } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicDF"},
		{"failure_capture",8,4,5,0,1573,kw_3,0.,0.,0.,0,"{Failure capturing} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"fork",8,8,4,1,1535,kw_6,0.,0.,0.,0,"@"},
		{"grid",8,0,4,1,1571,0,0.,0.,0.,0,"{Grid interface } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicG"},
		{"input_filter",11,0,2,0,1529,0,0.,0.,0.,0,"{Input filter} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"output_filter",11,0,3,0,1531,0,0.,0.,0.,0,"{Output filter} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"system",8,8,4,1,1533,kw_6,0.,0.,0.,0,"{System call interface } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"}
		},
	kw_8[4] = {
		{"analysis_concurrency",9,0,3,0,1599,0,0.,0.,0.,0,"{Asynchronous analysis concurrency} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"evaluation_concurrency",9,0,1,0,1593,0,0.,0.,0.,0,"{Asynchronous evaluation concurrency} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"local_evaluation_self_scheduling",8,0,2,0,1595,0,0.,0.,0.,0,"{Self-schedule local evals} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"local_evaluation_static_scheduling",8,0,2,0,1597,0,0.,0.,0.,0,"{Static-schedule local evals} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"}
		},
	kw_9[10] = {
		{"algebraic_mappings",11,0,2,0,1523,0,0.,0.,0.,0,"{Algebraic mappings file} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfAlgebraic"},
		{"analysis_drivers",15,9,3,0,1525,kw_7,0.,0.,0.,0,"{Analysis drivers} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"analysis_self_scheduling",8,0,8,0,1609,0,0.,0.,0.,0,"[CHOOSE analysis sched.]{Self scheduling of analyses} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"analysis_servers",9,0,7,0,1607,0,0.,0.,0.,0,"{Number of analysis servers} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"analysis_static_scheduling",8,0,8,0,1611,0,0.,0.,0.,0,"{Static scheduling of analyses} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"asynchronous",8,4,4,0,1591,kw_8,0.,0.,0.,0,"{Asynchronous interface usage} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"evaluation_self_scheduling",8,0,6,0,1603,0,0.,0.,0.,0,"[CHOOSE evaluation sched.]{Self scheduling of evaluations} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"evaluation_servers",9,0,5,0,1601,0,0.,0.,0.,0,"{Number of evaluation servers} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"evaluation_static_scheduling",8,0,6,0,1605,0,0.,0.,0.,0,"{Static scheduling of evaluations} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"id_interface",11,0,1,0,1521,0,0.,0.,0.,0,"{Interface set identifier} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"}
		},
	kw_10[7] = {
		{"merit1",8,0,1,1,259,0,0.,0.,0.,0,"[CHOOSE merit function]"},
		{"merit1_smooth",8,0,1,1,261},
		{"merit2",8,0,1,1,263},
		{"merit2_smooth",8,0,1,1,265,0,0.,0.,0.,0,"@"},
		{"merit2_squared",8,0,1,1,267},
		{"merit_max",8,0,1,1,255},
		{"merit_max_smooth",8,0,1,1,257}
		},
	kw_11[2] = {
		{"blocking",8,0,1,1,249,0,0.,0.,0.,0,"[CHOOSE synchronization]"},
		{"nonblocking",8,0,1,1,251,0,0.,0.,0.,0,"@"}
		},
	kw_12[18] = {
		{"constraint_penalty",10,0,7,0,269,0,0.,0.,0.,0,"{Constraint penalty} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodAPPSDC"},
		{"contraction_factor",10,0,2,0,241,0,0.,0.,0.,0,"{Pattern contraction factor} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodAPPSDC"},
		{"initial_delta",10,0,1,0,239,0,0.,0.,0.,0,"{Initial offset value} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodAPPSDC"},
		{"linear_equality_constraint_matrix",14,0,14,0,401,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,16,0,405,0,0.,0.,0.,0,"{Linear equality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,17,0,407,0,0.,0.,0.,0,"{Linear equality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,15,0,403,0,0.,0.,0.,0,"{Linear equality targets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,9,0,391,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,10,0,393,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,12,0,397,0,0.,0.,0.,0,"{Linear inequality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,13,0,399,0,0.,0.,0.,0,"{Linear inequality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,11,0,395,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"merit_function",8,7,6,0,253,kw_10,0.,0.,0.,0,"{Merit function} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodAPPSDC"},
		{"smoothing_factor",10,0,8,0,271,0,0.,0.,0.,0,"{Smoothing factor} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodAPPSDC"},
		{"solution_accuracy",2,0,4,0,244},
		{"solution_target",10,0,4,0,245,0,0.,0.,0.,0,"{Solution target} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodAPPSDC"},
		{"synchronization",8,2,5,0,247,kw_11,0.,0.,0.,0,"{Evaluation synchronization} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodAPPSDC"},
		{"threshold_delta",10,0,3,0,243,0,0.,0.,0.,0,"{Threshold for offset values} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodAPPSDC"}
		},
	kw_13[2] = {
		{"mt19937",8,0,1,1,815},
		{"rnum2",8,0,1,1,817}
		},
	kw_14[9] = {
		{"gpmsa",8,0,1,1,809},
		{"queso",8,0,1,1,807},
		{"rng",8,2,3,0,813,kw_13},
		{"samples",9,0,8,0,879,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,7,0,881,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodEG"},
		{"use_derivatives",8,0,2,0,811},
		{"x_obs_data_file",11,0,4,0,819},
		{"y_obs_data_file",11,0,5,0,821},
		{"y_std_data_file",11,0,6,0,823}
		},
	kw_15[3] = {
		{"deltas_per_variable",5,0,2,2,1064},
		{"step_vector",14,0,1,1,1063,0,0.,0.,0.,0,"{Step vector} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSCPS"},
		{"steps_per_variable",13,0,2,2,1065,0,0.,0.,0.,0,"{Number of steps per variable} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSCPS"}
		},
	kw_16[7] = {
		{"initial_delta",10,0,5,1,457,0,0.,0.,0.,0,"{Initial offset value} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"misc_options",15,0,4,0,543,0,0.,0.,0.,0,"{Specify miscellaneous options} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"seed",9,0,2,0,539,0,0.,0.,0.,0,"{Random seed for stochastic pattern search} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"show_misc_options",8,0,3,0,541,0,0.,0.,0.,0,"{Show miscellaneous options} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"solution_accuracy",2,0,1,0,536},
		{"solution_target",10,0,1,0,537,0,0.,0.,0.,0,"{Desired solution target} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"threshold_delta",10,0,6,2,459,0,0.,0.,0.,0,"{Threshold for offset values} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"}
		},
	kw_17[2] = {
		{"all_dimensions",8,0,1,1,467},
		{"major_dimension",8,0,1,1,465}
		},
	kw_18[11] = {
		{"constraint_penalty",10,0,6,0,477,0,0.,0.,0.,0,"{Constraint penalty} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDIR"},
		{"division",8,2,1,0,463,kw_17,0.,0.,0.,0,"{Box subdivision approach} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDIR"},
		{"global_balance_parameter",10,0,2,0,469,0,0.,0.,0.,0,"{Global search balancing parameter} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDIR"},
		{"local_balance_parameter",10,0,3,0,471,0,0.,0.,0.,0,"{Local search balancing parameter} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDIR"},
		{"max_boxsize_limit",10,0,4,0,473,0,0.,0.,0.,0,"{Maximum boxsize limit} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDIR"},
		{"min_boxsize_limit",10,0,5,0,475,0,0.,0.,0.,0,"{Minimum boxsize limit} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDIR"},
		{"misc_options",15,0,10,0,543,0,0.,0.,0.,0,"{Specify miscellaneous options} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"seed",9,0,8,0,539,0,0.,0.,0.,0,"{Random seed for stochastic pattern search} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"show_misc_options",8,0,9,0,541,0,0.,0.,0.,0,"{Show miscellaneous options} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"solution_accuracy",2,0,7,0,536},
		{"solution_target",10,0,7,0,537,0,0.,0.,0.,0,"{Desired solution target} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"}
		},
	kw_19[3] = {
		{"blend",8,0,1,1,513},
		{"two_point",8,0,1,1,511},
		{"uniform",8,0,1,1,515}
		},
	kw_20[2] = {
		{"linear_rank",8,0,1,1,493},
		{"merit_function",8,0,1,1,495}
		},
	kw_21[3] = {
		{"flat_file",11,0,1,1,489},
		{"simple_random",8,0,1,1,485},
		{"unique_random",8,0,1,1,487}
		},
	kw_22[2] = {
		{"mutation_range",9,0,2,0,531,0,0.,0.,0.,0,"{Mutation range} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"mutation_scale",10,0,1,0,529,0,0.,0.,0.,0,"{Mutation scale} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"}
		},
	kw_23[5] = {
		{"non_adaptive",8,0,2,0,533,0,0.,0.,0.,0,"{Non-adaptive mutation flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"offset_cauchy",8,2,1,1,525,kw_22},
		{"offset_normal",8,2,1,1,523,kw_22},
		{"offset_uniform",8,2,1,1,527,kw_22},
		{"replace_uniform",8,0,1,1,521}
		},
	kw_24[4] = {
		{"chc",9,0,1,1,501,0,0.,0.,0.,0,"{CHC replacement type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"elitist",9,0,1,1,503,0,0.,0.,0.,0,"{Elitist replacement type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"new_solutions_generated",9,0,2,0,505,0,0.,0.,0.,0,"{New solutions generated} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"random",9,0,1,1,499,0,0.,0.,0.,0,"{Random replacement type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"}
		},
	kw_25[14] = {
		{"constraint_penalty",10,0,9,0,535},
		{"crossover_rate",10,0,5,0,507,0,0.,0.,0.,0,"{Crossover rate} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"crossover_type",8,3,6,0,509,kw_19,0.,0.,0.,0,"{Crossover type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"fitness_type",8,2,3,0,491,kw_20,0.,0.,0.,0,"{Fitness type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"initialization_type",8,3,2,0,483,kw_21,0.,0.,0.,0,"{Initialization type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"misc_options",15,0,13,0,543,0,0.,0.,0.,0,"{Specify miscellaneous options} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"mutation_rate",10,0,7,0,517,0,0.,0.,0.,0,"{Mutation rate} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"mutation_type",8,5,8,0,519,kw_23,0.,0.,0.,0,"{Mutation type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"population_size",9,0,1,0,481,0,0.,0.,0.,0,"{Number of population members} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"replacement_type",8,4,4,0,497,kw_24,0.,0.,0.,0,"{Replacement type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYEA"},
		{"seed",9,0,11,0,539,0,0.,0.,0.,0,"{Random seed for stochastic pattern search} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"show_misc_options",8,0,12,0,541,0,0.,0.,0.,0,"{Show miscellaneous options} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"solution_accuracy",2,0,10,0,536},
		{"solution_target",10,0,10,0,537,0,0.,0.,0.,0,"{Desired solution target} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"}
		},
	kw_26[3] = {
		{"adaptive_pattern",8,0,1,1,431},
		{"basic_pattern",8,0,1,1,433},
		{"multi_step",8,0,1,1,429}
		},
	kw_27[2] = {
		{"coordinate",8,0,1,1,419},
		{"simplex",8,0,1,1,421}
		},
	kw_28[2] = {
		{"blocking",8,0,1,1,437},
		{"nonblocking",8,0,1,1,439}
		},
	kw_29[17] = {
		{"constant_penalty",8,0,1,0,411,0,0.,0.,0.,0,"{Control of dynamic penalty} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"constraint_penalty",10,0,16,0,453,0,0.,0.,0.,0,"{Constraint penalty} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"contraction_factor",10,0,15,0,451,0,0.,0.,0.,0,"{Pattern contraction factor} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"expand_after_success",9,0,3,0,415,0,0.,0.,0.,0,"{Number of consecutive improvements before expansion} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"exploratory_moves",8,3,7,0,427,kw_26,0.,0.,0.,0,"{Exploratory moves selection} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"initial_delta",10,0,13,1,457,0,0.,0.,0.,0,"{Initial offset value} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"misc_options",15,0,12,0,543,0,0.,0.,0.,0,"{Specify miscellaneous options} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"no_expansion",8,0,2,0,413,0,0.,0.,0.,0,"{No expansion flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"pattern_basis",8,2,4,0,417,kw_27,0.,0.,0.,0,"{Pattern basis selection} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"seed",9,0,10,0,539,0,0.,0.,0.,0,"{Random seed for stochastic pattern search} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"show_misc_options",8,0,11,0,541,0,0.,0.,0.,0,"{Show miscellaneous options} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"solution_accuracy",2,0,9,0,536},
		{"solution_target",10,0,9,0,537,0,0.,0.,0.,0,"{Desired solution target} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"stochastic",8,0,5,0,423,0,0.,0.,0.,0,"{Stochastic pattern search} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"synchronization",8,2,8,0,435,kw_28,0.,0.,0.,0,"{Evaluation synchronization} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"threshold_delta",10,0,14,2,459,0,0.,0.,0.,0,"{Threshold for offset values} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"total_pattern_size",9,0,6,0,425,0,0.,0.,0.,0,"{Total number of points in pattern} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"}
		},
	kw_30[13] = {
		{"constant_penalty",8,0,4,0,449,0,0.,0.,0.,0,"{Control of dynamic penalty} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYSW"},
		{"constraint_penalty",10,0,12,0,453,0,0.,0.,0.,0,"{Constraint penalty} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"contract_after_failure",9,0,1,0,443,0,0.,0.,0.,0,"{Number of consecutive failures before contraction} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYSW"},
		{"contraction_factor",10,0,11,0,451,0,0.,0.,0.,0,"{Pattern contraction factor} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"expand_after_success",9,0,3,0,447,0,0.,0.,0.,0,"{Number of consecutive improvements before expansion} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYSW"},
		{"initial_delta",10,0,9,1,457,0,0.,0.,0.,0,"{Initial offset value} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"misc_options",15,0,8,0,543,0,0.,0.,0.,0,"{Specify miscellaneous options} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"no_expansion",8,0,2,0,445,0,0.,0.,0.,0,"{No expansion flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYSW"},
		{"seed",9,0,6,0,539,0,0.,0.,0.,0,"{Random seed for stochastic pattern search} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"},
		{"show_misc_options",8,0,7,0,541,0,0.,0.,0.,0,"{Show miscellaneous options} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"solution_accuracy",2,0,5,0,536},
		{"solution_target",10,0,5,0,537,0,0.,0.,0.,0,"{Desired solution target} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYDC"},
		{"threshold_delta",10,0,10,2,459,0,0.,0.,0.,0,"{Threshold for offset values} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodCOLINYPS"}
		},
	kw_31[9] = {
		{"linear_equality_constraint_matrix",14,0,14,0,401,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,16,0,405,0,0.,0.,0.,0,"{Linear equality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,17,0,407,0,0.,0.,0.,0,"{Linear equality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,15,0,403,0,0.,0.,0.,0,"{Linear equality targets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,9,0,391,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,10,0,393,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,12,0,397,0,0.,0.,0.,0,"{Linear inequality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,13,0,399,0,0.,0.,0.,0,"{Linear inequality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,11,0,395,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"}
		},
	kw_32[1] = {
		{"drop_tolerance",10,0,1,0,847}
		},
	kw_33[14] = {
		{"box_behnken",8,0,1,1,837,0,0.,0.,0.,0,"[CHOOSE DACE type]"},
		{"central_composite",8,0,1,1,839},
		{"fixed_seed",8,0,5,0,849,0,0.,0.,0.,0,"{Fixed seed flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodDDACE"},
		{"grid",8,0,1,1,827},
		{"lhs",8,0,1,1,833},
		{"main_effects",8,0,2,0,841,0,0.,0.,0.,0,"{Main effects} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodDDACE"},
		{"oa_lhs",8,0,1,1,835},
		{"oas",8,0,1,1,831},
		{"quality_metrics",8,0,3,0,843,0,0.,0.,0.,0,"{Quality metrics} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodDDACE"},
		{"random",8,0,1,1,829},
		{"samples",9,0,8,0,879,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,7,0,881,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodEG"},
		{"symbols",9,0,6,0,851,0,0.,0.,0.,0,"{Number of symbols} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodDDACE"},
		{"variance_based_decomp",8,1,4,0,845,kw_32,0.,0.,0.,0,"{Variance based decomposition} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodDDACE"}
		},
	kw_34[2] = {
		{"maximize",8,0,1,1,175},
		{"minimize",8,0,1,1,173}
		},
	kw_35[15] = {
		{"bfgs",8,0,1,1,165},
		{"frcg",8,0,1,1,161},
		{"linear_equality_constraint_matrix",14,0,7,0,401,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,9,0,405,0,0.,0.,0.,0,"{Linear equality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,10,0,407,0,0.,0.,0.,0,"{Linear equality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,8,0,403,0,0.,0.,0.,0,"{Linear equality targets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,2,0,391,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,3,0,393,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,5,0,397,0,0.,0.,0.,0,"{Linear inequality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,6,0,399,0,0.,0.,0.,0,"{Linear inequality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,4,0,395,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"mmfd",8,0,1,1,163},
		{"optimization_type",8,2,11,0,171,kw_34,0.,0.,0.,0,"{Optimization type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodDOTDC"},
		{"slp",8,0,1,1,167},
		{"sqp",8,0,1,1,169}
		},
	kw_36[10] = {
		{"linear_equality_constraint_matrix",14,0,6,0,401,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,8,0,405,0,0.,0.,0.,0,"{Linear equality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,9,0,407,0,0.,0.,0.,0,"{Linear equality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,7,0,403,0,0.,0.,0.,0,"{Linear equality targets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,1,0,391,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,2,0,393,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,4,0,397,0,0.,0.,0.,0,"{Linear inequality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,5,0,399,0,0.,0.,0.,0,"{Linear inequality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,3,0,395,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"optimization_type",8,2,10,0,171,kw_34,0.,0.,0.,0,"{Optimization type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodDOTDC"}
		},
	kw_37[2] = {
		{"seed",9,0,2,0,881,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodEG"},
		{"use_derivatives",8,0,1,0,579}
		},
	kw_38[3] = {
		{"grid",8,0,1,1,867,0,0.,0.,0.,0,"[CHOOSE trial type]"},
		{"halton",8,0,1,1,869},
		{"random",8,0,1,1,871,0,0.,0.,0.,0,"@"}
		},
	kw_39[1] = {
		{"drop_tolerance",10,0,1,0,861}
		},
	kw_40[8] = {
		{"fixed_seed",8,0,4,0,863,0,0.,0.,0.,0,"{Fixed seed flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"latinize",8,0,1,0,855,0,0.,0.,0.,0,"{Latinization of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"num_trials",9,0,6,0,873,0,0.,0.,0.,0,"{Number of trials  } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"quality_metrics",8,0,2,0,857,0,0.,0.,0.,0,"{Quality metrics} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"samples",9,0,8,0,879,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,7,0,881,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodEG"},
		{"trial_type",8,3,5,0,865,kw_38,0.,0.,0.,0,"{Trial type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"variance_based_decomp",8,1,3,0,859,kw_39,0.,0.,0.,0,"{Variance based decomposition} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"}
		},
	kw_41[1] = {
		{"drop_tolerance",10,0,1,0,1037}
		},
	kw_42[10] = {
		{"fixed_sequence",8,0,6,0,1041,0,0.,0.,0.,0,"{Fixed sequence flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"halton",8,0,1,1,1027,0,0.,0.,0.,0,"[CHOOSE sequence type]"},
		{"hammersley",8,0,1,1,1029},
		{"latinize",8,0,2,0,1031,0,0.,0.,0.,0,"{Latinization of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"prime_base",13,0,9,0,1047,0,0.,0.,0.,0,"{Prime bases for sequences} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"quality_metrics",8,0,3,0,1033,0,0.,0.,0.,0,"{Quality metrics} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"samples",9,0,5,0,1039,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"sequence_leap",13,0,8,0,1045,0,0.,0.,0.,0,"{Sequence leaping indices} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"sequence_start",13,0,7,0,1043,0,0.,0.,0.,0,"{Sequence starting indices} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"variance_based_decomp",8,1,4,0,1035,kw_41,0.,0.,0.,0,"{Variance based decomposition} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"}
		},
	kw_43[2] = {
		{"complementary",8,0,1,1,775},
		{"cumulative",8,0,1,1,773}
		},
	kw_44[1] = {
		{"num_gen_reliability_levels",13,0,1,0,783,0,0.,0.,0.,0,"{Number of generalized reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"}
		},
	kw_45[1] = {
		{"num_probability_levels",13,0,1,0,779,0,0.,0.,0.,0,"{Number of probability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"}
		},
	kw_46[2] = {
		{"mt19937",8,0,1,1,787},
		{"rnum2",8,0,1,1,789}
		},
	kw_47[1] = {
		{"use_derivatives",8,0,1,0,757}
		},
	kw_48[2] = {
		{"gen_reliabilities",8,0,1,1,769},
		{"probabilities",8,0,1,1,767}
		},
	kw_49[2] = {
		{"compute",8,2,2,0,765,kw_48},
		{"num_response_levels",13,0,1,0,763}
		},
	kw_50[9] = {
		{"distribution",8,2,5,0,771,kw_43,0.,0.,0.,0,"{Distribution type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"ego",8,1,1,0,755,kw_47},
		{"gen_reliability_levels",14,1,7,0,781,kw_44,0.,0.,0.,0,"{Generalized reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"lhs",8,0,1,0,759},
		{"probability_levels",14,1,6,0,777,kw_45,0.,0.,0.,0,"{Probability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"response_levels",14,2,2,0,761,kw_49},
		{"rng",8,2,8,0,785,kw_46,0.,0.,0.,0,"{Random number generator} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"samples",9,0,4,0,879,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,3,0,881,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodEG"}
		},
	kw_51[1] = {
		{"use_derivatives",8,0,1,0,795}
		},
	kw_52[2] = {
		{"mt19937",8,0,1,1,801},
		{"rnum2",8,0,1,1,803}
		},
	kw_53[5] = {
		{"ego",8,1,1,0,793,kw_51},
		{"lhs",8,0,1,0,797},
		{"rng",8,2,2,0,799,kw_52,0.,0.,0.,0,"{Random seed generator} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDGlobalIntervalEst"},
		{"samples",9,0,4,0,879,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,3,0,881,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodEG"}
		},
	kw_54[2] = {
		{"complementary",8,0,1,1,1015},
		{"cumulative",8,0,1,1,1013}
		},
	kw_55[1] = {
		{"num_gen_reliability_levels",13,0,1,0,1023}
		},
	kw_56[1] = {
		{"num_probability_levels",13,0,1,0,1019}
		},
	kw_57[2] = {
		{"gen_reliabilities",8,0,1,1,1009},
		{"probabilities",8,0,1,1,1007}
		},
	kw_58[2] = {
		{"compute",8,2,2,0,1005,kw_57},
		{"num_response_levels",13,0,1,0,1003}
		},
	kw_59[2] = {
		{"mt19937",8,0,1,1,997},
		{"rnum2",8,0,1,1,999}
		},
	kw_60[10] = {
		{"all_variables",8,0,2,0,989,0,0.,0.,0.,0,"{All variables flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDGlobalRel"},
		{"distribution",8,2,7,0,1011,kw_54},
		{"gen_reliability_levels",14,1,9,0,1021,kw_55},
		{"probability_levels",14,1,8,0,1017,kw_56},
		{"response_levels",14,2,6,0,1001,kw_58},
		{"rng",8,2,5,0,995,kw_59},
		{"seed",9,0,4,0,993,0,0.,0.,0.,0,"{Refinement seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDLocalRel"},
		{"u_gaussian_process",8,0,1,1,987},
		{"use_derivatives",8,0,3,0,991},
		{"x_gaussian_process",8,0,1,1,985}
		},
	kw_61[2] = {
		{"gen_reliabilities",8,0,1,1,751},
		{"probabilities",8,0,1,1,749}
		},
	kw_62[2] = {
		{"compute",8,2,2,0,747,kw_61},
		{"num_response_levels",13,0,1,0,745}
		},
	kw_63[7] = {
		{"distribution",8,2,4,0,771,kw_43,0.,0.,0.,0,"{Distribution type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"gen_reliability_levels",14,1,6,0,781,kw_44,0.,0.,0.,0,"{Generalized reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"probability_levels",14,1,5,0,777,kw_45,0.,0.,0.,0,"{Probability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"response_levels",14,2,1,0,743,kw_62},
		{"rng",8,2,7,0,785,kw_46,0.,0.,0.,0,"{Random number generator} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"samples",9,0,3,0,879,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,2,0,881,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodEG"}
		},
	kw_64[1] = {
		{"list_of_points",14,0,1,1,1059,0,0.,0.,0.,0,"{List of points to evaluate} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSLPS"}
		},
	kw_65[2] = {
		{"complementary",8,0,1,1,919},
		{"cumulative",8,0,1,1,917}
		},
	kw_66[1] = {
		{"num_gen_reliability_levels",13,0,1,0,913}
		},
	kw_67[1] = {
		{"num_probability_levels",13,0,1,0,909}
		},
	kw_68[2] = {
		{"gen_reliabilities",8,0,1,1,905},
		{"probabilities",8,0,1,1,903}
		},
	kw_69[2] = {
		{"compute",8,2,2,0,901,kw_68},
		{"num_response_levels",13,0,1,0,899}
		},
	kw_70[6] = {
		{"distribution",8,2,5,0,915,kw_65},
		{"gen_reliability_levels",14,1,4,0,911,kw_66},
		{"nip",8,0,1,0,895},
		{"probability_levels",14,1,3,0,907,kw_67},
		{"response_levels",14,2,2,0,897,kw_69},
		{"sqp",8,0,1,0,893}
		},
	kw_71[2] = {
		{"nip",8,0,1,0,925},
		{"sqp",8,0,1,0,923}
		},
	kw_72[5] = {
		{"adapt_import",8,0,1,1,959},
		{"import",8,0,1,1,957},
		{"mm_adapt_import",8,0,1,1,961},
		{"samples",9,0,2,0,963,0,0.,0.,0.,0,"{Refinement samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDLocalRel"},
		{"seed",9,0,3,0,965,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"}
		},
	kw_73[3] = {
		{"first_order",8,0,1,1,951},
		{"sample_refinement",8,5,2,0,955,kw_72},
		{"second_order",8,0,1,1,953}
		},
	kw_74[9] = {
		{"nip",8,0,2,0,947},
		{"no_approx",8,0,1,1,943},
		{"sqp",8,0,2,0,945},
		{"u_taylor_mean",8,0,1,1,933},
		{"u_taylor_mpp",8,0,1,1,937},
		{"u_two_point",8,0,1,1,941},
		{"x_taylor_mean",8,0,1,1,931},
		{"x_taylor_mpp",8,0,1,1,935},
		{"x_two_point",8,0,1,1,939}
		},
	kw_75[1] = {
		{"num_reliability_levels",13,0,1,0,981}
		},
	kw_76[3] = {
		{"gen_reliabilities",8,0,1,1,977},
		{"probabilities",8,0,1,1,973},
		{"reliabilities",8,0,1,1,975}
		},
	kw_77[2] = {
		{"compute",8,3,2,0,971,kw_76},
		{"num_response_levels",13,0,1,0,969}
		},
	kw_78[7] = {
		{"distribution",8,2,5,0,1011,kw_54},
		{"gen_reliability_levels",14,1,7,0,1021,kw_55},
		{"integration",8,3,2,0,949,kw_73,0.,0.,0.,0,"{Integration method} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDLocalRel"},
		{"mpp_search",8,9,1,0,929,kw_74,0.,0.,0.,0,"{MPP search type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDLocalRel"},
		{"probability_levels",14,1,6,0,1017,kw_56},
		{"reliability_levels",14,1,4,0,979,kw_75},
		{"response_levels",14,2,3,0,967,kw_77}
		},
	kw_79[2] = {
		{"num_offspring",0x19,0,2,0,369,0,0.,0.,0.,0,"{Number of offspring in random shuffle crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"num_parents",0x19,0,1,0,367,0,0.,0.,0.,0,"{Number of parents in random shuffle crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_80[5] = {
		{"crossover_rate",10,0,2,0,371,0,0.,0.,0.,0,"{Crossover rate} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"multi_point_binary",9,0,1,1,359,0,0.,0.,0.,0,"{Multi point binary crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"multi_point_parameterized_binary",9,0,1,1,361,0,0.,0.,0.,0,"{Multi point parameterized binary crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"multi_point_real",9,0,1,1,363,0,0.,0.,0.,0,"{Multi point real crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"shuffle_random",8,2,1,1,365,kw_79,0.,0.,0.,0,"{Random shuffle crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_81[3] = {
		{"flat_file",11,0,1,1,355},
		{"simple_random",8,0,1,1,351},
		{"unique_random",8,0,1,1,353}
		},
	kw_82[1] = {
		{"mutation_scale",10,0,1,0,385,0,0.,0.,0.,0,"{Mutation scale} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_83[6] = {
		{"bit_random",8,0,1,1,375},
		{"mutation_rate",10,0,2,0,387,0,0.,0.,0.,0,"{Mutation rate} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"offset_cauchy",8,1,1,1,381,kw_82},
		{"offset_normal",8,1,1,1,379,kw_82},
		{"offset_uniform",8,1,1,1,383,kw_82},
		{"replace_uniform",8,0,1,1,377}
		},
	kw_84[3] = {
		{"metric_tracker",8,0,1,1,301,0,0.,0.,0.,0,"{Convergence type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{"num_generations",0x29,0,3,0,305,0,0.,0.,0.,0,"{Number generations for metric_tracker converger} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{"percent_change",10,0,2,0,303,0,0.,0.,0.,0,"{Percent change limit for metric_tracker converger} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"}
		},
	kw_85[2] = {
		{"domination_count",8,0,1,1,279},
		{"layer_rank",8,0,1,1,277}
		},
	kw_86[2] = {
		{"distance",14,0,1,1,297},
		{"radial",14,0,1,1,295}
		},
	kw_87[1] = {
		{"orthogonal_distance",14,0,1,1,309,0,0.,0.,0.,0,"{Post_processor distance} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"}
		},
	kw_88[2] = {
		{"shrinkage_fraction",10,0,1,0,291},
		{"shrinkage_percentage",2,0,1,0,290}
		},
	kw_89[4] = {
		{"below_limit",10,2,1,1,289,kw_88,0.,0.,0.,0,"{Below limit selection} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"elitist",8,0,1,1,283},
		{"roulette_wheel",8,0,1,1,285},
		{"unique_roulette_wheel",8,0,1,1,287}
		},
	kw_90[21] = {
		{"convergence_type",8,3,4,0,299,kw_84},
		{"crossover_type",8,5,19,0,357,kw_80,0.,0.,0.,0,"{Crossover type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"fitness_type",8,2,1,0,275,kw_85,0.,0.,0.,0,"{Fitness type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{"initialization_type",8,3,18,0,349,kw_81,0.,0.,0.,0,"{Initialization type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"linear_equality_constraint_matrix",14,0,11,0,401,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,13,0,405,0,0.,0.,0.,0,"{Linear equality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,14,0,407,0,0.,0.,0.,0,"{Linear equality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,12,0,403,0,0.,0.,0.,0,"{Linear equality targets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,6,0,391,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,7,0,393,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,9,0,397,0,0.,0.,0.,0,"{Linear inequality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,10,0,399,0,0.,0.,0.,0,"{Linear inequality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,8,0,395,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"log_file",11,0,16,0,345,0,0.,0.,0.,0,"{Log file} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"mutation_type",8,6,20,0,373,kw_83,0.,0.,0.,0,"{Mutation type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"niching_type",8,2,3,0,293,kw_86,0.,0.,0.,0,"{Niche pressure type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{"population_size",9,0,15,0,343,0,0.,0.,0.,0,"{Number of population members} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"postprocessor_type",8,1,5,0,307,kw_87,0.,0.,0.,0,"{Post_processor type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{"print_each_pop",8,0,17,0,347,0,0.,0.,0.,0,"{Population output} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"replacement_type",8,4,2,0,281,kw_89,0.,0.,0.,0,"{Replacement type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"seed",9,0,21,0,389,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_91[1] = {
		{"partitions",13,0,1,1,1069,0,0.,0.,0.,0,"{Partitions per variable} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSMPS"}
		},
	kw_92[4] = {
		{"min_boxsize_limit",10,0,2,0,887,0,0.,0.,0.,0,"{Min boxsize limit} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNCSUDC"},
		{"solution_accuracy",2,0,1,0,884},
		{"solution_target",10,0,1,0,885,0,0.,0.,0.,0,"{Solution Target } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNCSUDC"},
		{"volume_boxsize_limit",10,0,3,0,889}
		},
	kw_93[9] = {
		{"absolute_conv_tol",10,0,2,0,549,0,0.,0.,0.,0,"{Absolute function convergence tolerance} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"},
		{"covariance",9,0,8,0,561,0,0.,0.,0.,0,"{Covariance post-processing} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"},
		{"false_conv_tol",10,0,6,0,557,0,0.,0.,0.,0,"{False convergence tolerance} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"},
		{"function_precision",10,0,1,0,547,0,0.,0.,0.,0,"{Relative precision in least squares terms} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"},
		{"initial_trust_radius",10,0,7,0,559,0,0.,0.,0.,0,"{Initial trust region radius} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"},
		{"regression_diagnostics",8,0,9,0,563,0,0.,0.,0.,0,"{Regression diagnostics post-processing} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"},
		{"singular_conv_tol",10,0,4,0,553,0,0.,0.,0.,0,"{Singular convergence tolerance} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"},
		{"singular_radius",10,0,5,0,555,0,0.,0.,0.,0,"{Step limit for sctol} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"},
		{"x_conv_tol",10,0,3,0,551,0,0.,0.,0.,0,"{Convergence tolerance for change in parameter vector} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodLSNL2SOL"}
		},
	kw_94[1] = {
		{"num_reliability_levels",13,0,1,0,727,0,0.,0.,0.,0,"{Number of reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"}
		},
	kw_95[3] = {
		{"gen_reliabilities",8,0,1,1,739},
		{"probabilities",8,0,1,1,735},
		{"reliabilities",8,0,1,1,737}
		},
	kw_96[2] = {
		{"compute",8,3,2,0,733,kw_95,0.,0.,0.,0,"{Target statistics for response levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"num_response_levels",13,0,1,0,731,0,0.,0.,0.,0,"{Number of response levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"}
		},
	kw_97[6] = {
		{"expansion_order",13,0,4,1,627,0,0.,0.,0.,0,"{Expansion order} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"expansion_terms",9,0,4,1,629,0,0.,0.,0.,0,"{Expansion terms} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"reuse_points",8,0,1,0,615,0,0.,0.,0.,0,"{Reuse points flag for PCE coefficient estimation by collocation_points or collocation_ratio} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"reuse_samples",0,0,1,0,614},
		{"tensor_grid",8,0,3,0,619},
		{"use_derivatives",8,0,2,0,617,0,0.,0.,0.,0,"{Derivative usage flag for PCE coefficient estimation by collocation_points or collocation_ratio} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"}
		},
	kw_98[2] = {
		{"expansion_order",13,0,4,1,627,0,0.,0.,0.,0,"{Expansion order} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"expansion_terms",9,0,4,1,629,0,0.,0.,0.,0,"{Expansion terms} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"}
		},
	kw_99[3] = {
		{"expansion_order",13,0,2,1,627,0,0.,0.,0.,0,"{Expansion order} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"expansion_terms",9,0,2,1,629,0,0.,0.,0.,0,"{Expansion terms} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"incremental_lhs",8,0,1,0,623,0,0.,0.,0.,0,"{Incremental LHS flag for PCE coefficient estimation by expansion_samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"}
		},
	kw_100[3] = {
		{"decay",8,0,1,1,591},
		{"generalized",8,0,1,1,593},
		{"sobol",8,0,1,1,589}
		},
	kw_101[2] = {
		{"dimension_adaptive",8,3,1,1,587,kw_100},
		{"uniform",8,0,1,1,585}
		},
	kw_102[2] = {
		{"nested",8,0,1,0,605},
		{"non_nested",8,0,1,0,607}
		},
	kw_103[3] = {
		{"adapt_import",8,0,1,1,641},
		{"import",8,0,1,1,639},
		{"mm_adapt_import",8,0,1,1,643}
		},
	kw_104[2] = {
		{"lhs",8,0,1,1,647},
		{"random",8,0,1,1,649}
		},
	kw_105[3] = {
		{"dimension_preference",14,0,1,0,603,0,0.,0.,0.,0,"{Sparse grid dimension preference} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"nested",8,0,2,0,605},
		{"non_nested",8,0,2,0,607}
		},
	kw_106[2] = {
		{"drop_tolerance",10,0,2,0,635,0,0.,0.,0.,0,"{VBD tolerance for omitting small indices} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"univariate_effects",8,0,1,0,633,0,0.,0.,0.,0,"{Restriction of VBD indices to main/total} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"}
		},
	kw_107[23] = {
		{"all_variables",8,0,15,0,721,0,0.,0.,0.,0,"{All variables flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"askey",8,0,2,0,595},
		{"collocation_points",9,6,3,1,611,kw_97,0.,0.,0.,0,"{Number of collocation points for PCE coefficient estimation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"collocation_ratio",10,6,3,1,613,kw_97,0.,0.,0.,0,"{Collocation point oversampling ratio for PCE coefficient estimation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"cubature_integrand",9,0,3,1,609,0,0.,0.,0.,0,"{Cubature integrand order for PCE coefficient estimation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"distribution",8,2,9,0,771,kw_43,0.,0.,0.,0,"{Distribution type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"expansion_import_file",11,2,3,1,625,kw_98,0.,0.,0.,0,"{File name for import of PCE coefficients} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"expansion_samples",9,3,3,1,621,kw_99,0.,0.,0.,0,"{Number of simulation samples for PCE coefficient estimation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"fixed_seed",8,0,16,0,723,0,0.,0.,0.,0,"{Fixed seed flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"gen_reliability_levels",14,1,11,0,781,kw_44,0.,0.,0.,0,"{Generalized reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"p_refinement",8,2,1,0,583,kw_101,0.,0.,0.,0,"{Automated polynomial order refinement} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"probability_levels",14,1,10,0,777,kw_45,0.,0.,0.,0,"{Probability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"quadrature_order",13,2,3,1,599,kw_102,0.,0.,0.,0,"{Quadrature order for PCE coefficient estimation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"reliability_levels",14,1,13,0,725,kw_94,0.,0.,0.,0,"{Reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"response_levels",14,2,14,0,729,kw_96,0.,0.,0.,0,"{Response levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"rng",8,2,12,0,785,kw_46,0.,0.,0.,0,"{Random number generator} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"sample_refinement",8,3,5,0,637,kw_103,0.,0.,0.,0,"{Importance sampling refinement} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"sample_type",8,2,6,0,645,kw_104,0.,0.,0.,0,"{Sampling type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"samples",9,0,8,0,879,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,7,0,881,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodEG"},
		{"sparse_grid_level",9,3,3,1,601,kw_105,0.,0.,0.,0,"{Sparse grid level for PCE coefficient estimation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"variance_based_decomp",8,2,4,0,631,kw_106,0.,0.,0.,0,"{Variance based decomposition (VBD)} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"wiener",8,0,2,0,597}
		},
	kw_108[1] = {
		{"previous_samples",9,0,1,1,715,0,0.,0.,0.,0,"{Previous samples for incremental approaches} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"}
		},
	kw_109[4] = {
		{"incremental_lhs",8,1,1,1,711,kw_108},
		{"incremental_random",8,1,1,1,713,kw_108},
		{"lhs",8,0,1,1,709},
		{"random",8,0,1,1,707}
		},
	kw_110[1] = {
		{"drop_tolerance",10,0,1,0,719}
		},
	kw_111[12] = {
		{"all_variables",8,0,11,0,721,0,0.,0.,0.,0,"{All variables flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"distribution",8,2,5,0,771,kw_43,0.,0.,0.,0,"{Distribution type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"fixed_seed",8,0,12,0,723,0,0.,0.,0.,0,"{Fixed seed flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"gen_reliability_levels",14,1,7,0,781,kw_44,0.,0.,0.,0,"{Generalized reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"probability_levels",14,1,6,0,777,kw_45,0.,0.,0.,0,"{Probability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"reliability_levels",14,1,9,0,725,kw_94,0.,0.,0.,0,"{Reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"response_levels",14,2,10,0,729,kw_96,0.,0.,0.,0,"{Response levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"rng",8,2,8,0,785,kw_46,0.,0.,0.,0,"{Random number generator} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"sample_type",8,4,1,0,705,kw_109},
		{"samples",9,0,4,0,879,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,3,0,881,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodEG"},
		{"variance_based_decomp",8,1,2,0,717,kw_110}
		},
	kw_112[2] = {
		{"generalized",8,0,1,1,663},
		{"sobol",8,0,1,1,661}
		},
	kw_113[2] = {
		{"dimension_adaptive",8,2,1,1,659,kw_112},
		{"uniform",8,0,1,1,657}
		},
	kw_114[3] = {
		{"adapt_import",8,0,1,1,693},
		{"import",8,0,1,1,691},
		{"mm_adapt_import",8,0,1,1,695}
		},
	kw_115[2] = {
		{"lhs",8,0,1,1,699},
		{"random",8,0,1,1,701}
		},
	kw_116[1] = {
		{"dimension_preference",14,0,1,0,675,0,0.,0.,0.,0,"{Sparse grid dimension preference} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"}
		},
	kw_117[2] = {
		{"drop_tolerance",10,0,2,0,687,0,0.,0.,0.,0,"{VBD tolerance for omitting small indices} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"},
		{"univariate_effects",8,0,1,0,685,0,0.,0.,0.,0,"{Restriction of VBD indices to main/total} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"}
		},
	kw_118[23] = {
		{"all_variables",8,0,17,0,721,0,0.,0.,0.,0,"{All variables flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"askey",8,0,2,0,665},
		{"distribution",8,2,11,0,771,kw_43,0.,0.,0.,0,"{Distribution type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"fixed_seed",8,0,18,0,723,0,0.,0.,0.,0,"{Fixed seed flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"gen_reliability_levels",14,1,13,0,781,kw_44,0.,0.,0.,0,"{Generalized reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"h_refinement",8,2,1,0,655,kw_113},
		{"nested",8,0,5,0,679},
		{"non_nested",8,0,5,0,681},
		{"p_refinement",8,2,1,0,653,kw_113,0.,0.,0.,0,"{Automated polynomial order refinement} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"},
		{"piecewise",8,0,2,0,669},
		{"probability_levels",14,1,12,0,777,kw_45,0.,0.,0.,0,"{Probability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"quadrature_order",13,0,3,1,671,0,0.,0.,0.,0,"{Quadrature order for collocation points} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"},
		{"reliability_levels",14,1,15,0,725,kw_94,0.,0.,0.,0,"{Reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"response_levels",14,2,16,0,729,kw_96,0.,0.,0.,0,"{Response levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"rng",8,2,14,0,785,kw_46,0.,0.,0.,0,"{Random number generator} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"sample_refinement",8,3,7,0,689,kw_114,0.,0.,0.,0,"{Importance sampling refinement} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"},
		{"sample_type",8,2,8,0,697,kw_115,0.,0.,0.,0,"{Sampling type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"},
		{"samples",9,0,10,0,879,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,9,0,881,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodEG"},
		{"sparse_grid_level",9,1,3,1,673,kw_116,0.,0.,0.,0,"{Sparse grid level for collocation points} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"},
		{"use_derivatives",8,0,4,0,677},
		{"variance_based_decomp",8,2,6,0,683,kw_117,0.,0.,0.,0,"{Variance-based decomposition (VBD)} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"},
		{"wiener",8,0,2,0,667}
		},
	kw_119[1] = {
		{"misc_options",15,0,1,0,567}
		},
	kw_120[12] = {
		{"function_precision",10,0,11,0,195,0,0.,0.,0.,0,"{Function precision} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNPSOLDC"},
		{"linear_equality_constraint_matrix",14,0,6,0,401,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,8,0,405,0,0.,0.,0.,0,"{Linear equality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,9,0,407,0,0.,0.,0.,0,"{Linear equality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,7,0,403,0,0.,0.,0.,0,"{Linear equality targets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,1,0,391,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,2,0,393,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,4,0,397,0,0.,0.,0.,0,"{Linear inequality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,5,0,399,0,0.,0.,0.,0,"{Linear inequality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,3,0,395,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linesearch_tolerance",10,0,12,0,197,0,0.,0.,0.,0,"{Line search tolerance} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNPSOLDC"},
		{"verify_level",9,0,10,0,193,0,0.,0.,0.,0,"{Gradient verification level} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNPSOLDC"}
		},
	kw_121[11] = {
		{"gradient_tolerance",10,0,11,0,231},
		{"linear_equality_constraint_matrix",14,0,6,0,401,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,8,0,405,0,0.,0.,0.,0,"{Linear equality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,9,0,407,0,0.,0.,0.,0,"{Linear equality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,7,0,403,0,0.,0.,0.,0,"{Linear equality targets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,1,0,391,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,2,0,393,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,4,0,397,0,0.,0.,0.,0,"{Linear inequality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,5,0,399,0,0.,0.,0.,0,"{Linear inequality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,3,0,395,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"max_step",10,0,10,0,229}
		},
	kw_122[10] = {
		{"linear_equality_constraint_matrix",14,0,7,0,401,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,9,0,405,0,0.,0.,0.,0,"{Linear equality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,10,0,407,0,0.,0.,0.,0,"{Linear equality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,8,0,403,0,0.,0.,0.,0,"{Linear equality targets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,2,0,391,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,3,0,393,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,5,0,397,0,0.,0.,0.,0,"{Linear inequality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,6,0,399,0,0.,0.,0.,0,"{Linear inequality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,4,0,395,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"search_scheme_size",9,0,1,0,235}
		},
	kw_123[4] = {
		{"gradient_based_line_search",8,0,1,1,215,0,0.,0.,0.,0,"[CHOOSE line search type]"},
		{"tr_pds",8,0,1,1,219},
		{"trust_region",8,0,1,1,217},
		{"value_based_line_search",8,0,1,1,213}
		},
	kw_124[16] = {
		{"centering_parameter",10,0,5,0,227},
		{"central_path",11,0,3,0,223},
		{"gradient_tolerance",10,0,16,0,231},
		{"linear_equality_constraint_matrix",14,0,11,0,401,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,13,0,405,0,0.,0.,0.,0,"{Linear equality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,14,0,407,0,0.,0.,0.,0,"{Linear equality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,12,0,403,0,0.,0.,0.,0,"{Linear equality targets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,6,0,391,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,7,0,393,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,9,0,397,0,0.,0.,0.,0,"{Linear inequality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,10,0,399,0,0.,0.,0.,0,"{Linear inequality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,8,0,395,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"max_step",10,0,15,0,229},
		{"merit_function",11,0,2,0,221},
		{"search_method",8,4,1,0,211,kw_123},
		{"steplength_to_boundary",10,0,4,0,225}
		},
	kw_125[4] = {
		{"debug",8,0,1,1,67,0,0.,0.,0.,0,"[CHOOSE output level]"},
		{"quiet",8,0,1,1,71},
		{"silent",8,0,1,1,73},
		{"verbose",8,0,1,1,69}
		},
	kw_126[3] = {
		{"partitions",13,0,1,0,877,0,0.,0.,0.,0,"{Number of partitions} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSUADE"},
		{"samples",9,0,3,0,879,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,2,0,881,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodEG"}
		},
	kw_127[4] = {
		{"converge_order",8,0,1,1,1075},
		{"converge_qoi",8,0,1,1,1077},
		{"estimate_order",8,0,1,1,1073},
		{"refinement_rate",10,0,2,0,1079,0,0.,0.,0.,0,"{Refinement rate} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSolnRichardson"}
		},
	kw_128[2] = {
		{"num_generations",0x29,0,2,0,341},
		{"percent_change",10,0,1,0,339}
		},
	kw_129[2] = {
		{"num_generations",0x29,0,2,0,335,0,0.,0.,0.,0,"{Number of generations (for convergence test) } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{"percent_change",10,0,1,0,333,0,0.,0.,0.,0,"{Percent change in fitness} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"}
		},
	kw_130[2] = {
		{"average_fitness_tracker",8,2,1,1,337,kw_128},
		{"best_fitness_tracker",8,2,1,1,331,kw_129}
		},
	kw_131[2] = {
		{"constraint_penalty",10,0,2,0,317,0,0.,0.,0.,0,"{Constraint penalty in merit function} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{"merit_function",8,0,1,1,315}
		},
	kw_132[4] = {
		{"elitist",8,0,1,1,321},
		{"favor_feasible",8,0,1,1,323},
		{"roulette_wheel",8,0,1,1,325},
		{"unique_roulette_wheel",8,0,1,1,327}
		},
	kw_133[19] = {
		{"convergence_type",8,2,3,0,329,kw_130,0.,0.,0.,0,"{Convergence type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{"crossover_type",8,5,17,0,357,kw_80,0.,0.,0.,0,"{Crossover type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"fitness_type",8,2,1,0,313,kw_131,0.,0.,0.,0,"{Fitness type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{"initialization_type",8,3,16,0,349,kw_81,0.,0.,0.,0,"{Initialization type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"linear_equality_constraint_matrix",14,0,9,0,401,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,11,0,405,0,0.,0.,0.,0,"{Linear equality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,12,0,407,0,0.,0.,0.,0,"{Linear equality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,10,0,403,0,0.,0.,0.,0,"{Linear equality targets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,4,0,391,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,5,0,393,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,7,0,397,0,0.,0.,0.,0,"{Linear inequality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,8,0,399,0,0.,0.,0.,0,"{Linear inequality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,6,0,395,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"log_file",11,0,14,0,345,0,0.,0.,0.,0,"{Log file} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"mutation_type",8,6,18,0,373,kw_83,0.,0.,0.,0,"{Mutation type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"population_size",9,0,13,0,343,0,0.,0.,0.,0,"{Number of population members} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"print_each_pop",8,0,15,0,347,0,0.,0.,0.,0,"{Population output} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"replacement_type",8,4,2,0,319,kw_132,0.,0.,0.,0,"{Replacement type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{"seed",9,0,19,0,389,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_134[14] = {
		{"function_precision",10,0,12,0,195,0,0.,0.,0.,0,"{Function precision} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNPSOLDC"},
		{"linear_equality_constraint_matrix",14,0,7,0,401,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,9,0,405,0,0.,0.,0.,0,"{Linear equality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,10,0,407,0,0.,0.,0.,0,"{Linear equality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,8,0,403,0,0.,0.,0.,0,"{Linear equality targets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,2,0,391,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,3,0,393,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,5,0,397,0,0.,0.,0.,0,"{Linear inequality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,6,0,399,0,0.,0.,0.,0,"{Linear inequality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,4,0,395,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linesearch_tolerance",10,0,13,0,197,0,0.,0.,0.,0,"{Line search tolerance} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNPSOLDC"},
		{"nlssol",8,0,1,1,191},
		{"npsol",8,0,1,1,189},
		{"verify_level",9,0,11,0,193,0,0.,0.,0.,0,"{Gradient verification level} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNPSOLDC"}
		},
	kw_135[3] = {
		{"approx_method_name",11,0,1,1,571,0,0.,0.,0.,0,"[CHOOSE sub-method ref.]{Approximate sub-problem minimization method name} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBG"},
		{"approx_method_pointer",11,0,1,1,573,0,0.,0.,0.,0,"{Approximate sub-problem minimization method pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBG"},
		{"replace_points",8,0,2,0,575,0,0.,0.,0.,0,"{Replace points used in surrogate construction with best points from previous iteration} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBG"}
		},
	kw_136[2] = {
		{"filter",8,0,1,1,143,0,0.,0.,0.,0,"@[CHOOSE acceptance logic]"},
		{"tr_ratio",8,0,1,1,141}
		},
	kw_137[7] = {
		{"augmented_lagrangian_objective",8,0,1,1,119,0,0.,0.,0.,0,"[CHOOSE objective formulation]"},
		{"lagrangian_objective",8,0,1,1,121},
		{"linearized_constraints",8,0,2,2,125,0,0.,0.,0.,0,"[CHOOSE constraint formulation]"},
		{"no_constraints",8,0,2,2,127},
		{"original_constraints",8,0,2,2,123,0,0.,0.,0.,0,"@"},
		{"original_primary",8,0,1,1,115,0,0.,0.,0.,0,"@"},
		{"single_objective",8,0,1,1,117}
		},
	kw_138[1] = {
		{"homotopy",8,0,1,1,147}
		},
	kw_139[4] = {
		{"adaptive_penalty_merit",8,0,1,1,133,0,0.,0.,0.,0,"[CHOOSE merit function]"},
		{"augmented_lagrangian_merit",8,0,1,1,137,0,0.,0.,0.,0,"@"},
		{"lagrangian_merit",8,0,1,1,135},
		{"penalty_merit",8,0,1,1,131}
		},
	kw_140[6] = {
		{"contract_threshold",10,0,3,0,105,0,0.,0.,0.,0,"{Shrink trust region if trust region ratio is below this value} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"contraction_factor",10,0,5,0,109,0,0.,0.,0.,0,"{Trust region contraction factor} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"expand_threshold",10,0,4,0,107,0,0.,0.,0.,0,"{Expand trust region if trust region ratio is above this value} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"expansion_factor",10,0,6,0,111,0,0.,0.,0.,0,"{Trust region expansion factor} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"initial_size",10,0,1,0,101,0,0.,0.,0.,0,"{Trust region initial size (relative to bounds)} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"minimum_size",10,0,2,0,103,0,0.,0.,0.,0,"{Trust region minimum size} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"}
		},
	kw_141[18] = {
		{"acceptance_logic",8,2,7,0,139,kw_136,0.,0.,0.,0,"{SBL iterate acceptance logic} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"approx_method_name",11,0,1,1,91,0,0.,0.,0.,0,"[CHOOSE sub-method ref.]{Approximate sub-problem minimization method name} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"approx_method_pointer",11,0,1,1,93,0,0.,0.,0.,0,"{Approximate sub-problem minimization method pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"approx_subproblem",8,7,5,0,113,kw_137,0.,0.,0.,0,"{Approximate subproblem formulation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"constraint_relax",8,1,8,0,145,kw_138,0.,0.,0.,0,"{SBL constraint relaxation method for infeasible iterates} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"linear_equality_constraint_matrix",14,0,14,0,401,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,16,0,405,0,0.,0.,0.,0,"{Linear equality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,17,0,407,0,0.,0.,0.,0,"{Linear equality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,15,0,403,0,0.,0.,0.,0,"{Linear equality targets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,9,0,391,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,10,0,393,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,12,0,397,0,0.,0.,0.,0,"{Linear inequality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,13,0,399,0,0.,0.,0.,0,"{Linear inequality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,11,0,395,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"merit_function",8,4,6,0,129,kw_139,0.,0.,0.,0,"{SBL merit function} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"soft_convergence_limit",9,0,2,0,95,0,0.,0.,0.,0,"{Soft convergence limit for SBL iterations} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"trust_region",8,6,4,0,99,kw_140,0.,0.,0.,0,"{Trust region group specification} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"truth_surrogate_bypass",8,0,3,0,97,0,0.,0.,0.,0,"{Flag for bypassing lower level surrogates in truth verifications} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"}
		},
	kw_142[3] = {
		{"final_point",14,0,1,1,1051,0,0.,0.,0.,0,"[CHOOSE final pt or increment]{Termination point of vector} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSVPS"},
		{"num_steps",9,0,2,2,1055,0,0.,0.,0.,0,"{Number of steps along vector} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSVPS"},
		{"step_vector",14,0,1,1,1053,0,0.,0.,0.,0,"{Step vector} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSVPS"}
		},
	kw_143[75] = {
		{"asynch_pattern_search",8,18,11,1,237,kw_12,0.,0.,0.,0,0,"Optimization: Local, Derivative-free"},
		{"bayes_calibration",8,9,11,1,805,kw_14},
		{"centered_parameter_study",8,3,11,1,1061,kw_15,0.,0.,0.,0,"[CHOOSE PSTUDY method]{Centered parameter study} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSCPS","Parameter Studies"},
		{"coliny_apps",0,18,11,1,236,kw_12},
		{"coliny_cobyla",8,7,11,1,455,kw_16,0.,0.,0.,0,0,"Optimization: Local, Derivative-free"},
		{"coliny_direct",8,11,11,1,461,kw_18,0.,0.,0.,0,0,"Optimization: Global"},
		{"coliny_ea",8,14,11,1,479,kw_25,0.,0.,0.,0,0,"Optimization: Global"},
		{"coliny_pattern_search",8,17,11,1,409,kw_29,0.,0.,0.,0,0,"Optimization: Local, Derivative-free"},
		{"coliny_solis_wets",8,13,11,1,441,kw_30,0.,0.,0.,0,0,"Optimization: Local, Derivative-free"},
		{"conmin_frcg",8,9,11,1,177,kw_31,0.,0.,0.,0,"[CHOOSE OPT method]","Optimization: Local, Derivative-based"},
		{"conmin_mfd",8,9,11,1,179,kw_31,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"constraint_tolerance",10,0,8,0,83,0,0.,0.,0.,0,"{Constraint tolerance} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"convergence_tolerance",10,0,7,0,81,0,0.,0.,0.,0,"{Convergence tolerance} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"dace",8,14,11,1,825,kw_33,0.,0.,0.,0,0,"[CHOOSE method category]DACE"},
		{"dl_solver",11,9,11,1,181,kw_31,0.,0.,0.,0,0,"Optimization: Plug-in"},
		{"dot",8,15,11,1,159,kw_35},
		{"dot_bfgs",8,10,11,1,153,kw_36,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"dot_frcg",8,10,11,1,149,kw_36,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"dot_mmfd",8,10,11,1,151,kw_36,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"dot_slp",8,10,11,1,155,kw_36,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"dot_sqp",8,10,11,1,157,kw_36,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"efficient_global",8,2,11,1,577,kw_37,0.,0.,0.,0,0,"Surrogate-based Methods"},
		{"final_solutions",0x29,0,10,0,87,0,0.,0.,0.,0,"{Final solutions} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"fsu_cvt",8,8,11,1,853,kw_40,0.,0.,0.,0,0,"DACE"},
		{"fsu_quasi_mc",8,10,11,1,1025,kw_42,0.,0.,0.,0,0,"DACE"},
		{"global_evidence",8,9,11,1,753,kw_50,0.,0.,0.,0,0,"Uncertainty Quantification"},
		{"global_interval_est",8,5,11,1,791,kw_53,0.,0.,0.,0,0,"Uncertainty Quantification"},
		{"global_reliability",8,10,11,1,983,kw_60,0.,0.,0.,0,0,"Uncertainty Quantification"},
		{"id_method",11,0,1,0,61,0,0.,0.,0.,0,"{Method set identifier} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"importance_sampling",8,7,11,1,741,kw_63,0.,0.,0.,0,0,"Uncertainty Quantification"},
		{"list_parameter_study",8,1,11,1,1057,kw_64,0.,0.,0.,0,"{List parameter study} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSLPS","Parameter Studies"},
		{"local_evidence",8,6,11,1,891,kw_70,0.,0.,0.,0,0,"Uncertainty Quantification"},
		{"local_interval_est",8,2,11,1,921,kw_71,0.,0.,0.,0,0,"Uncertainty Quantification"},
		{"local_reliability",8,7,11,1,927,kw_78,0.,0.,0.,0,"{Reliability method} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDLocalRel","Uncertainty Quantification"},
		{"max_function_evaluations",9,0,5,0,77,0,0.,0.,0.,0,"{Maximum function evaluations} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"max_iterations",9,0,4,0,75,0,0.,0.,0.,0,"{Maximum iterations} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"model_pointer",11,0,2,0,63,0,0.,0.,0.,0,"{Model pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"moga",8,21,11,1,273,kw_90,0.,0.,0.,0,0,"Optimization: Global"},
		{"multidim_parameter_study",8,1,11,1,1067,kw_91,0.,0.,0.,0,"{Multidimensional parameter study} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSMPS","Parameter Studies"},
		{"ncsu_direct",8,4,11,1,883,kw_92,0.,0.,0.,0,0,"Optimization: Global"},
		{"nl2sol",8,9,11,1,545,kw_93,0.,0.,0.,0,"[CHOOSE LSQ method]","Nonlinear Least Squares"},
		{"nlpql_sqp",8,9,11,1,199,kw_31,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"nlssol_sqp",8,12,11,1,185,kw_120,0.,0.,0.,0,0,"Nonlinear Least Squares"},
		{"nond_bayes_calibration",0,9,11,1,804,kw_14},
		{"nond_global_evidence",0,9,11,1,752,kw_50},
		{"nond_global_interval_est",0,5,11,1,790,kw_53},
		{"nond_global_reliability",0,10,11,1,982,kw_60},
		{"nond_importance_sampling",0,7,11,1,740,kw_63},
		{"nond_local_evidence",0,6,11,1,890,kw_70},
		{"nond_local_interval_est",0,2,11,1,920,kw_71},
		{"nond_local_reliability",0,7,11,1,926,kw_78},
		{"nond_polynomial_chaos",0,23,11,1,580,kw_107},
		{"nond_sampling",0,12,11,1,702,kw_111},
		{"nond_stoch_collocation",0,23,11,1,650,kw_118},
		{"nonlinear_cg",8,1,11,1,565,kw_119,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"npsol_sqp",8,12,11,1,183,kw_120,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"optpp_cg",8,11,11,1,201,kw_121,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"optpp_fd_newton",8,16,11,1,205,kw_124,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"optpp_g_newton",8,16,11,1,207,kw_124,0.,0.,0.,0,0,"Nonlinear Least Squares"},
		{"optpp_newton",8,16,11,1,209,kw_124,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"optpp_pds",8,10,11,1,233,kw_122,0.,0.,0.,0,0,"Optimization: Local, Derivative-free"},
		{"optpp_q_newton",8,16,11,1,203,kw_124,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"output",8,4,3,0,65,kw_125,0.,0.,0.,0,"{Output verbosity} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"polynomial_chaos",8,23,11,1,581,kw_107,0.,0.,0.,0,0,"Uncertainty Quantification"},
		{"psuade_moat",8,3,11,1,875,kw_126,0.,0.,0.,0,"{PSUADE MOAT method} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSUADE","DACE"},
		{"richardson_extrap",8,4,11,1,1071,kw_127,0.,0.,0.,0,"{Richardson extrapolation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSolnRichardson"},
		{"sampling",8,12,11,1,703,kw_111,0.,0.,0.,0,"{Nondeterministic sampling method} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC","Uncertainty Quantification"},
		{"scaling",8,0,9,0,85,0,0.,0.,0.,0,"{Scaling flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"soga",8,19,11,1,311,kw_133,0.,0.,0.,0,0,"Optimization: Global"},
		{"speculative",8,0,6,0,79,0,0.,0.,0.,0,"{Speculative gradients and Hessians} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"stanford",8,14,11,1,187,kw_134},
		{"stoch_collocation",8,23,11,1,651,kw_118,0.,0.,0.,0,0,"Uncertainty Quantification"},
		{"surrogate_based_global",8,3,11,1,569,kw_135,0.,0.,0.,0,0,"Surrogate-based Methods"},
		{"surrogate_based_local",8,18,11,1,89,kw_141,0.,0.,0.,0,0,"Surrogate-based Methods"},
		{"vector_parameter_study",8,3,11,1,1049,kw_142,0.,0.,0.,0,"{Vector parameter study} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSVPS","Parameter Studies"}
		},
	kw_144[1] = {
		{"optional_interface_responses_pointer",11,0,1,0,1241,0,0.,0.,0.,0,"{Responses pointer for nested model optional interfaces} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelNested"}
		},
	kw_145[4] = {
		{"primary_response_mapping",14,0,3,0,1249,0,0.,0.,0.,0,"{Primary response mappings for nested models} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelNested"},
		{"primary_variable_mapping",15,0,1,0,1245,0,0.,0.,0.,0,"{Primary variable mappings for nested models} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelNested"},
		{"secondary_response_mapping",14,0,4,0,1251,0,0.,0.,0.,0,"{Secondary response mappings for nested models} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelNested"},
		{"secondary_variable_mapping",15,0,2,0,1247,0,0.,0.,0.,0,"{Secondary variable mappings for nested models} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelNested"}
		},
	kw_146[2] = {
		{"optional_interface_pointer",11,1,1,0,1239,kw_144,0.,0.,0.,0,"{Optional interface set pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelNested"},
		{"sub_method_pointer",11,4,2,1,1243,kw_145,0.,0.,0.,0,"{Sub-method pointer for nested models} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelNested"}
		},
	kw_147[1] = {
		{"interface_pointer",11,0,1,0,1091,0,0.,0.,0.,0,"{Interface set pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSingle"}
		},
	kw_148[6] = {
		{"additive",8,0,2,2,1199,0,0.,0.,0.,0,"[CHOOSE correction type]"},
		{"combined",8,0,2,2,1203},
		{"first_order",8,0,1,1,1195,0,0.,0.,0.,0,"[CHOOSE correction order]"},
		{"multiplicative",8,0,2,2,1201},
		{"second_order",8,0,1,1,1197},
		{"zeroth_order",8,0,1,1,1193}
		},
	kw_149[3] = {
		{"constant",8,0,1,1,1105,0,0.,0.,0.,0,"[CHOOSE trend type]"},
		{"linear",8,0,1,1,1107},
		{"reduced_quadratic",8,0,1,1,1109}
		},
	kw_150[2] = {
		{"point_selection",8,0,1,0,1101,0,0.,0.,0.,0,"{GP point selection} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"trend",8,3,2,0,1103,kw_149,0.,0.,0.,0,"{GP trend function} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_151[4] = {
		{"constant",8,0,1,1,1157},
		{"linear",8,0,1,1,1159},
		{"quadratic",8,0,1,1,1163},
		{"reduced_quadratic",8,0,1,1,1161}
		},
	kw_152[4] = {
		{"correlation_lengths",14,0,4,0,1169,0,0.,0.,0.,0,"{Kriging correlation lengths} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"max_trials",0x19,0,3,0,1167,0,0.,0.,0.,0,"{Kriging maximum trials} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"optimization_method",11,0,2,0,1165,0,0.,0.,0.,0,"{Kriging optimization method} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"trend",8,4,1,0,1155,kw_151,0.,0.,0.,0,"{Kriging trend function} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_153[2] = {
		{"cubic",8,0,1,1,1119},
		{"linear",8,0,1,1,1117}
		},
	kw_154[2] = {
		{"interpolation",8,2,2,0,1115,kw_153,0.,0.,0.,0,"{MARS interpolation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"max_bases",9,0,1,0,1113,0,0.,0.,0.,0,"{MARS maximum bases} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_155[2] = {
		{"poly_order",9,0,1,0,1123,0,0.,0.,0.,0,"{MLS polynomial order} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"weight_function",9,0,2,0,1125,0,0.,0.,0.,0,"{MLS weight function} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_156[3] = {
		{"nodes",9,0,1,0,1129,0,0.,0.,0.,0,"{ANN number nodes} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"random_weight",9,0,3,0,1133,0,0.,0.,0.,0,"{ANN random weight} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"range",10,0,2,0,1131,0,0.,0.,0.,0,"{ANN range} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_157[3] = {
		{"cubic",8,0,1,1,1151,0,0.,0.,0.,0,"[CHOOSE polynomial order]"},
		{"linear",8,0,1,1,1147},
		{"quadratic",8,0,1,1,1149}
		},
	kw_158[4] = {
		{"bases",9,0,1,0,1137,0,0.,0.,0.,0,"{RBF number of bases} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"max_pts",9,0,2,0,1139,0,0.,0.,0.,0,"{RBF maximum points} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"max_subsets",9,0,4,0,1143},
		{"min_partition",9,0,3,0,1141,0,0.,0.,0.,0,"{RBF minimum partitions} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_159[3] = {
		{"all",8,0,1,1,1181},
		{"none",8,0,1,1,1185},
		{"region",8,0,1,1,1183}
		},
	kw_160[18] = {
		{"correction",8,6,7,0,1191,kw_148,0.,0.,0.,0,"{Surrogate correction approach} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"dace_method_pointer",11,0,3,0,1177,0,0.,0.,0.,0,"{Design of experiments method pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"diagnostics",15,0,8,0,1205,0,0.,0.,0.,0,"{Print diagnostic metrics about the surrogate goodness of fit} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"gaussian_process",8,2,1,1,1099,kw_150,0.,0.,0.,0,"[CHOOSE surrogate type]{Gaussian process} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"kriging",8,4,1,1,1153,kw_152,0.,0.,0.,0,"{Kriging interpolation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"mars",8,2,1,1,1111,kw_154,0.,0.,0.,0,"{Multivariate adaptive regression splines} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"minimum_points",8,0,2,0,1173},
		{"moving_least_squares",8,2,1,1,1121,kw_155,0.,0.,0.,0,"{Moving least squares} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"neural_network",8,3,1,1,1127,kw_156,0.,0.,0.,0,"{Artificial neural network} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"points_file",11,0,5,0,1187},
		{"polynomial",8,3,1,1,1145,kw_157,0.,0.,0.,0,"{Polynomial} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"radial_basis",8,4,1,1,1135,kw_158},
		{"recommended_points",8,0,2,0,1175},
		{"reuse_points",8,3,4,0,1179,kw_159},
		{"reuse_samples",0,3,4,0,1178,kw_159},
		{"samples_file",3,0,5,0,1186},
		{"total_points",9,0,2,0,1171},
		{"use_derivatives",8,0,6,0,1189}
		},
	kw_161[6] = {
		{"additive",8,0,2,2,1231,0,0.,0.,0.,0,"[CHOOSE correction type]"},
		{"combined",8,0,2,2,1235},
		{"first_order",8,0,1,1,1227,0,0.,0.,0.,0,"[CHOOSE correction order]"},
		{"multiplicative",8,0,2,2,1233},
		{"second_order",8,0,1,1,1229},
		{"zeroth_order",8,0,1,1,1225}
		},
	kw_162[3] = {
		{"correction",8,6,3,3,1223,kw_161,0.,0.,0.,0,"{Surrogate correction approach} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrH"},
		{"high_fidelity_model_pointer",11,0,2,2,1221,0,0.,0.,0.,0,"{Pointer to the high fidelity model specification} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrH"},
		{"low_fidelity_model_pointer",11,0,1,1,1219,0,0.,0.,0.,0,"{Pointer to the low fidelity model specification} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrH"}
		},
	kw_163[2] = {
		{"actual_model_pointer",11,0,2,2,1215,0,0.,0.,0.,0,"{Pointer to the truth model specification} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrMP"},
		{"taylor_series",8,0,1,1,1213,0,0.,0.,0.,0,"{Taylor series local approximation } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrL"}
		},
	kw_164[2] = {
		{"actual_model_pointer",11,0,2,2,1215,0,0.,0.,0.,0,"{Pointer to the truth model specification} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrMP"},
		{"tana",8,0,1,1,1209,0,0.,0.,0.,0,"{Two-point adaptive nonlinear approximation } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrMP"}
		},
	kw_165[5] = {
		{"global",8,18,2,1,1097,kw_160,0.,0.,0.,0,"[CHOOSE surrogate category]{Global approximations } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"hierarchical",8,3,2,1,1217,kw_162,0.,0.,0.,0,"{Hierarchical approximation } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrH"},
		{"id_surrogates",13,0,1,0,1095,0,0.,0.,0.,0,"{Surrogate response ids} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrogate"},
		{"local",8,2,2,1,1211,kw_163,0.,0.,0.,0,"{Local approximation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrL"},
		{"multipoint",8,2,2,1,1207,kw_164,0.,0.,0.,0,"{Multipoint approximation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrMP"}
		},
	kw_166[6] = {
		{"id_model",11,0,1,0,1083,0,0.,0.,0.,0,"{Model set identifier} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelIndControl"},
		{"nested",8,2,4,1,1237,kw_146,0.,0.,0.,0,"[CHOOSE model type]"},
		{"responses_pointer",11,0,3,0,1087,0,0.,0.,0.,0,"{Responses set pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelIndControl"},
		{"single",8,1,4,1,1089,kw_147,0.,0.,0.,0,"@"},
		{"surrogate",8,5,4,1,1093,kw_165},
		{"variables_pointer",11,0,2,0,1085,0,0.,0.,0.,0,"{Variables set pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelIndControl"}
		},
	kw_167[1] = {
		{"ignore_bounds",8,0,1,0,1691,0,0.,0.,0.,0,"{Ignore variable bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradMixed"}
		},
	kw_168[10] = {
		{"central",8,0,6,0,1699,0,0.,0.,0.,0,"[CHOOSE difference interval]"},
		{"dakota",8,1,4,0,1689,kw_167,0.,0.,0.,0,"@[CHOOSE gradient source]"},
		{"fd_gradient_step_size",0x406,0,7,0,1700,0,0.,0.,0.001},
		{"fd_step_size",0x40e,0,7,0,1701,0,0.,0.,0.001,0,"{Finite difference step size} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradMixed"},
		{"forward",8,0,6,0,1697,0,0.,0.,0.,0,"@"},
		{"id_analytic_gradients",13,0,2,2,1683,0,0.,0.,0.,0,"{Analytic derivatives function list} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradMixed"},
		{"id_numerical_gradients",13,0,1,1,1681,0,0.,0.,0.,0,"{Numerical derivatives function list} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradMixed"},
		{"interval_type",8,0,5,0,1695,0,0.,0.,0.,0,"{Interval type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradNum"},
		{"method_source",8,0,3,0,1687,0,0.,0.,0.,0,"{Method source} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradNum"},
		{"vendor",8,0,4,0,1693}
		},
	kw_169[2] = {
		{"fd_hessian_step_size",6,0,1,0,1726},
		{"fd_step_size",14,0,1,0,1727,0,0.,0.,0.,0,"{Finite difference step size} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessMixed"}
		},
	kw_170[1] = {
		{"damped",8,0,1,0,1737,0,0.,0.,0.,0,"{Numerical safeguarding of BFGS update} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessMixed"}
		},
	kw_171[2] = {
		{"bfgs",8,1,1,1,1735,kw_170,0.,0.,0.,0,"[CHOOSE Hessian approx.]"},
		{"sr1",8,0,1,1,1739}
		},
	kw_172[5] = {
		{"central",8,0,2,0,1731,0,0.,0.,0.,0,"[CHOOSE difference interval]"},
		{"forward",8,0,2,0,1729,0,0.,0.,0.,0,"@"},
		{"id_analytic_hessians",13,0,4,0,1741,0,0.,0.,0.,0,"{Analytic Hessians function list} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessMixed"},
		{"id_numerical_hessians",13,2,1,0,1725,kw_169,0.,0.,0.,0,"{Numerical Hessians function list} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessMixed"},
		{"id_quasi_hessians",13,2,3,0,1733,kw_171,0.,0.,0.,0,"{Quasi Hessians function list} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessMixed"}
		},
	kw_173[3] = {
		{"nonlinear_equality_scale_types",0x80f,0,2,0,1669,0,0.,0.,0.,0,"{Nonlinear equality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_nonlinear_equality_constraints"},
		{"nonlinear_equality_scales",0x80e,0,3,0,1671,0,0.,0.,0.,0,"{Nonlinear equality constraint scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_nonlinear_equality_constraints"},
		{"nonlinear_equality_targets",14,0,1,0,1667,0,0.,0.,0.,0,"{Nonlinear equality constraint targets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_nonlinear_equality_constraints"}
		},
	kw_174[4] = {
		{"nonlinear_inequality_lower_bounds",14,0,1,0,1657,0,0.,0.,0.,0,"{Nonlinear inequality constraint lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_nonlinear_inequality_constraints"},
		{"nonlinear_inequality_scale_types",0x80f,0,3,0,1661,0,0.,0.,0.,0,"{Nonlinear inequality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_nonlinear_inequality_constraints"},
		{"nonlinear_inequality_scales",0x80e,0,4,0,1663,0,0.,0.,0.,0,"{Nonlinear inequality constraint scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_nonlinear_inequality_constraints"},
		{"nonlinear_inequality_upper_bounds",14,0,2,0,1659,0,0.,0.,0.,0,"{Nonlinear inequality constraint upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_nonlinear_inequality_constraints"}
		},
	kw_175[6] = {
		{"least_squares_data_file",11,0,1,0,1647,0,0.,0.,0.,0,"{Least squares data source file} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS"},
		{"least_squares_term_scale_types",0x80f,0,2,0,1649,0,0.,0.,0.,0,"{Least squares term scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_least_squares_terms"},
		{"least_squares_term_scales",0x80e,0,3,0,1651,0,0.,0.,0.,0,"{Least squares terms scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_least_squares_terms"},
		{"least_squares_weights",14,0,4,0,1653,0,0.,0.,0.,0,"{Least squares terms weightings} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_least_squares_terms"},
		{"num_nonlinear_equality_constraints",0x29,3,6,0,1665,kw_173,0.,0.,0.,0,"{Number of nonlinear equality constraints} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS"},
		{"num_nonlinear_inequality_constraints",0x29,4,5,0,1655,kw_174,0.,0.,0.,0,"{Number of nonlinear inequality constraints} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS"}
		},
	kw_176[3] = {
		{"nonlinear_equality_scale_types",0x80f,0,2,0,1641,0,0.,0.,0.,0,"{Nonlinear equality constraint scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_nonlinear_equality_constraints"},
		{"nonlinear_equality_scales",0x80e,0,3,0,1643,0,0.,0.,0.,0,"{Nonlinear equality constraint scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_nonlinear_equality_constraints"},
		{"nonlinear_equality_targets",14,0,1,0,1639,0,0.,0.,0.,0,"{Nonlinear equality constraint targets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_nonlinear_equality_constraints"}
		},
	kw_177[4] = {
		{"nonlinear_inequality_lower_bounds",14,0,1,0,1629,0,0.,0.,0.,0,"{Nonlinear inequality constraint lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_nonlinear_inequality_constraints"},
		{"nonlinear_inequality_scale_types",0x80f,0,3,0,1633,0,0.,0.,0.,0,"{Nonlinear inequality constraint scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_nonlinear_inequality_constraints"},
		{"nonlinear_inequality_scales",0x80e,0,4,0,1635,0,0.,0.,0.,0,"{Nonlinear inequality constraint scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_nonlinear_inequality_constraints"},
		{"nonlinear_inequality_upper_bounds",14,0,2,0,1631,0,0.,0.,0.,0,"{Nonlinear inequality constraint upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_nonlinear_inequality_constraints"}
		},
	kw_178[5] = {
		{"multi_objective_weights",14,0,3,0,1625,0,0.,0.,0.,0,"{Multiobjective weightings} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_objective_functions"},
		{"num_nonlinear_equality_constraints",0x29,3,5,0,1637,kw_176,0.,0.,0.,0,"{Number of nonlinear equality constraints} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_nonlinear_equality_constraints"},
		{"num_nonlinear_inequality_constraints",0x29,4,4,0,1627,kw_177,0.,0.,0.,0,"{Number of nonlinear inequality constraints} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt"},
		{"objective_function_scale_types",0x80f,0,1,0,1621,0,0.,0.,0.,0,"{Objective function scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_objective_functions"},
		{"objective_function_scales",0x80e,0,2,0,1623,0,0.,0.,0.,0,"{Objective function scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_objective_functions"}
		},
	kw_179[8] = {
		{"central",8,0,6,0,1699,0,0.,0.,0.,0,"[CHOOSE difference interval]"},
		{"dakota",8,1,4,0,1689,kw_167,0.,0.,0.,0,"@[CHOOSE gradient source]"},
		{"fd_gradient_step_size",0x406,0,7,0,1700,0,0.,0.,0.001},
		{"fd_step_size",0x40e,0,7,0,1701,0,0.,0.,0.001,0,"{Finite difference step size} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradMixed"},
		{"forward",8,0,6,0,1697,0,0.,0.,0.,0,"@"},
		{"interval_type",8,0,5,0,1695,0,0.,0.,0.,0,"{Interval type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradNum"},
		{"method_source",8,0,3,0,1687,0,0.,0.,0.,0,"{Method source} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradNum"},
		{"vendor",8,0,4,0,1693}
		},
	kw_180[4] = {
		{"central",8,0,2,0,1711,0,0.,0.,0.,0,"[CHOOSE difference interval]"},
		{"fd_hessian_step_size",6,0,1,0,1706},
		{"fd_step_size",14,0,1,0,1707,0,0.,0.,0.,0,"{Finite difference step size} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessNum"},
		{"forward",8,0,2,0,1709,0,0.,0.,0.,0,"@"}
		},
	kw_181[1] = {
		{"damped",8,0,1,0,1717,0,0.,0.,0.,0,"{Numerical safeguarding of BFGS update} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessQuasi"}
		},
	kw_182[2] = {
		{"bfgs",8,1,1,1,1715,kw_181,0.,0.,0.,0,"[CHOOSE Hessian approx.]"},
		{"sr1",8,0,1,1,1719}
		},
	kw_183[15] = {
		{"analytic_gradients",8,0,4,2,1677,0,0.,0.,0.,0,"[CHOOSE gradient type]"},
		{"analytic_hessians",8,0,5,3,1721,0,0.,0.,0.,0,"[CHOOSE Hessian type]"},
		{"descriptors",15,0,2,0,1617,0,0.,0.,0.,0,"{Response labels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespLabels"},
		{"id_responses",11,0,1,0,1615,0,0.,0.,0.,0,"{Responses set identifier} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespSetId"},
		{"mixed_gradients",8,10,4,2,1679,kw_168,0.,0.,0.,0,"{Mixed gradients} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradMixed"},
		{"mixed_hessians",8,5,5,3,1723,kw_172,0.,0.,0.,0,"{Mixed Hessians} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessMixed"},
		{"no_gradients",8,0,4,2,1675,0,0.,0.,0.,0,"@"},
		{"no_hessians",8,0,5,3,1703,0,0.,0.,0.,0,"@"},
		{"num_least_squares_terms",0x29,6,3,1,1645,kw_175,0.,0.,0.,0,"[CHOOSE response type]{{Least squares (calibration)} Number of least squares terms} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS"},
		{"num_objective_functions",0x29,5,3,1,1619,kw_178,0.,0.,0.,0,"{{Optimization} Number of objective functions} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt"},
		{"num_response_functions",0x29,0,3,1,1673,0,0.,0.,0.,0,"{{Generic responses} Number of response functions} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnGen"},
		{"numerical_gradients",8,8,4,2,1685,kw_179,0.,0.,0.,0,"{Numerical gradients} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradNum"},
		{"numerical_hessians",8,4,5,3,1705,kw_180,0.,0.,0.,0,"{Numerical Hessians} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessNum"},
		{"quasi_hessians",8,2,5,3,1713,kw_182,0.,0.,0.,0,"{Quasi Hessians} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessQuasi"},
		{"response_descriptors",7,0,2,0,1616}
		},
	kw_184[1] = {
		{"method_list",15,0,1,1,33,0,0.,0.,0.,0,"{List of methods} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"}
		},
	kw_185[3] = {
		{"global_method_pointer",11,0,1,1,25,0,0.,0.,0.,0,"{Pointer to the global method specification} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"local_method_pointer",11,0,2,2,27,0,0.,0.,0.,0,"{Pointer to the local method specification} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"local_search_probability",10,0,3,0,29,0,0.,0.,0.,0,"{Probability of executing local searches} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"}
		},
	kw_186[1] = {
		{"method_list",15,0,1,1,21,0,0.,0.,0.,0,"{List of methods} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"}
		},
	kw_187[5] = {
		{"collaborative",8,1,1,1,31,kw_184,0.,0.,0.,0,"[CHOOSE hybrid type]{Collaborative hybrid} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"coupled",0,3,1,1,22,kw_185},
		{"embedded",8,3,1,1,23,kw_185,0.,0.,0.,0,"{Embedded hybrid} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"sequential",8,1,1,1,19,kw_186,0.,0.,0.,0,"{Sequential hybrid} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"uncoupled",0,1,1,1,18,kw_186}
		},
	kw_188[1] = {
		{"seed",9,0,1,0,41,0,0.,0.,0.,0,"{Seed for random starting points} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratMultiStart"}
		},
	kw_189[3] = {
		{"method_pointer",11,0,1,1,37,0,0.,0.,0.,0,"{Method pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratMultiStart"},
		{"random_starts",9,1,2,0,39,kw_188,0.,0.,0.,0,"{Number of random starting points} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratMultiStart"},
		{"starting_points",14,0,3,0,43,0,0.,0.,0.,0,"{List of user-specified starting points} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratMultiStart"}
		},
	kw_190[1] = {
		{"seed",9,0,1,0,51,0,0.,0.,0.,0,"{Seed for random weighting sets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratParetoSet"}
		},
	kw_191[5] = {
		{"method_pointer",11,0,1,1,47,0,0.,0.,0.,0,"{Optimization method pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratParetoSet"},
		{"multi_objective_weight_sets",6,0,3,0,52},
		{"opt_method_pointer",3,0,1,1,46},
		{"random_weight_sets",9,1,2,0,49,kw_190,0.,0.,0.,0,"{Number of random weighting sets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratParetoSet"},
		{"weight_sets",14,0,3,0,53,0,0.,0.,0.,0,"{List of user-specified weighting sets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratParetoSet"}
		},
	kw_192[1] = {
		{"method_pointer",11,0,1,0,57,0,0.,0.,0.,0,"{Method pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratSingle"}
		},
	kw_193[1] = {
		{"tabular_graphics_file",11,0,1,0,7,0,0.,0.,0.,0,"{File name for tabular graphics data} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratIndControl"}
		},
	kw_194[10] = {
		{"graphics",8,0,1,0,3,0,0.,0.,0.,0,"{Graphics flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratIndControl"},
		{"hybrid",8,5,7,1,17,kw_187,0.,0.,0.,0,"[CHOOSE strategy type]{Hybrid strategy} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"iterator_self_scheduling",8,0,5,0,13,0,0.,0.,0.,0,"{Self-scheduling of iterator jobs} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratIndControl"},
		{"iterator_servers",9,0,4,0,11,0,0.,0.,0.,0,"{Number of iterator servers} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratIndControl"},
		{"iterator_static_scheduling",8,0,6,0,15,0,0.,0.,0.,0,"{Static scheduling of iterator jobs} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratIndControl"},
		{"multi_start",8,3,7,1,35,kw_189,0.,0.,0.,0,"{Multi-start iteration strategy} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratMultiStart"},
		{"output_precision",0x29,0,3,0,9,0,0.,0.,0.,0,"{Numeric output precision} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratIndControl"},
		{"pareto_set",8,5,7,1,45,kw_191,0.,0.,0.,0,"{Pareto set optimization strategy} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratParetoSet"},
		{"single_method",8,1,7,1,55,kw_192,0.,0.,0.,0,"@{Single method strategy} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratSingle"},
		{"tabular_graphics_data",8,1,2,0,5,kw_193,0.,0.,0.,0,"{Tabulation of graphics data} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratIndControl"}
		},
	kw_195[10] = {
		{"alphas",14,0,1,1,1365,0,0.,0.,0.,0,"{beta uncertain alphas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta",0,"beta_uncertain"},
		{"betas",14,0,2,2,1367,0,0.,0.,0.,0,"{beta uncertain betas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta",0,"beta_uncertain"},
		{"buv_alphas",6,0,1,1,1364,0,0.,0.,0.,0,0,0,"beta_uncertain"},
		{"buv_betas",6,0,2,2,1366,0,0.,0.,0.,0,0,0,"beta_uncertain"},
		{"buv_descriptors",7,0,5,0,1372,0,0.,0.,0.,0,0,0,"beta_uncertain"},
		{"buv_lower_bounds",6,0,3,3,1368,0,0.,0.,0.,0,0,0,"beta_uncertain"},
		{"buv_upper_bounds",6,0,4,4,1370,0,0.,0.,0.,0,0,0,"beta_uncertain"},
		{"descriptors",15,0,5,0,1373,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta",0,"beta_uncertain"},
		{"lower_bounds",14,0,3,3,1369,0,0.,0.,0.,0,"{Distribution lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta",0,"beta_uncertain"},
		{"upper_bounds",14,0,4,4,1371,0,0.,0.,0.,0,"{Distribution upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta",0,"beta_uncertain"}
		},
	kw_196[3] = {
		{"descriptors",15,0,3,0,1431,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Binomial",0,"binomial_uncertain"},
		{"num_trials",13,0,2,2,1429,0,0.,0.,0.,0,"{binomial uncertain num_trials} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Binomial",0,"binomial_uncertain"},
		{"prob_per_trial",14,0,1,1,1427,0,0.,0.,0.,0,"{binomial uncertain prob_per_trial} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Binomial",0,"binomial_uncertain"}
		},
	kw_197[12] = {
		{"cdv_descriptors",7,0,6,0,1268,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"cdv_initial_point",6,0,1,0,1258,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"cdv_lower_bounds",6,0,2,0,1260,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"cdv_scale_types",0x807,0,4,0,1264,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"cdv_scales",0x806,0,5,0,1266,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"cdv_upper_bounds",6,0,3,0,1262,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"descriptors",15,0,6,0,1269,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"},
		{"initial_point",14,0,1,0,1259,0,0.,0.,0.,0,"{Initial point} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"},
		{"lower_bounds",14,0,2,0,1261,0,0.,0.,0.,0,"{Lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"},
		{"scale_types",0x80f,0,4,0,1265,0,0.,0.,0.,0,"{Scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"},
		{"scales",0x80e,0,5,0,1267,0,0.,0.,0.,0,"{Scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"},
		{"upper_bounds",14,0,3,0,1263,0,0.,0.,0.,0,"{Upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"}
		},
	kw_198[8] = {
		{"csv_descriptors",7,0,4,0,1486,0,0.,0.,0.,0,0,0,"continuous_state"},
		{"csv_initial_state",6,0,1,0,1480,0,0.,0.,0.,0,0,0,"continuous_state"},
		{"csv_lower_bounds",6,0,2,0,1482,0,0.,0.,0.,0,0,0,"continuous_state"},
		{"csv_upper_bounds",6,0,3,0,1484,0,0.,0.,0.,0,0,0,"continuous_state"},
		{"descriptors",15,0,4,0,1487,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCSV",0,"continuous_state"},
		{"initial_state",14,0,1,0,1481,0,0.,0.,0.,0,"{Initial states} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCSV",0,"continuous_state"},
		{"lower_bounds",14,0,2,0,1483,0,0.,0.,0.,0,"{Lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCSV",0,"continuous_state"},
		{"upper_bounds",14,0,3,0,1485,0,0.,0.,0.,0,"{Upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCSV",0,"continuous_state"}
		},
	kw_199[8] = {
		{"ddv_descriptors",7,0,4,0,1278,0,0.,0.,0.,0,0,0,"discrete_design_range"},
		{"ddv_initial_point",5,0,1,0,1272,0,0.,0.,0.,0,0,0,"discrete_design_range"},
		{"ddv_lower_bounds",5,0,2,0,1274,0,0.,0.,0.,0,0,0,"discrete_design_range"},
		{"ddv_upper_bounds",5,0,3,0,1276,0,0.,0.,0.,0,0,0,"discrete_design_range"},
		{"descriptors",15,0,4,0,1279,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDRIV",0,"discrete_design_range"},
		{"initial_point",13,0,1,0,1273,0,0.,0.,0.,0,"{Initial point} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDRIV",0,"discrete_design_range"},
		{"lower_bounds",13,0,2,0,1275,0,0.,0.,0.,0,"{Lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDRIV",0,"discrete_design_range"},
		{"upper_bounds",13,0,3,0,1277,0,0.,0.,0.,0,"{Upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDRIV",0,"discrete_design_range"}
		},
	kw_200[4] = {
		{"descriptors",15,0,4,0,1289,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSIV",0,"discrete_design_set_integer"},
		{"initial_point",13,0,1,0,1283,0,0.,0.,0.,0,"{Initial point} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSIV",0,"discrete_design_set_integer"},
		{"num_set_values",13,0,2,0,1285,0,0.,0.,0.,0,"{Number of values for each variable} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSIV",0,"discrete_design_set_integer"},
		{"set_values",13,0,3,1,1287,0,0.,0.,0.,0,"{Set values} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSIV"}
		},
	kw_201[4] = {
		{"descriptors",15,0,4,0,1299,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSRV",0,"discrete_design_set_real"},
		{"initial_point",14,0,1,0,1293,0,0.,0.,0.,0,"{Initial point} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSRV",0,"discrete_design_set_real"},
		{"num_set_values",13,0,2,0,1295,0,0.,0.,0.,0,"{Number of values for each variable} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSRV",0,"discrete_design_set_real"},
		{"set_values",14,0,3,1,1297,0,0.,0.,0.,0,"{Set values} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSRV"}
		},
	kw_202[8] = {
		{"descriptors",15,0,4,0,1497,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSRIV",0,"discrete_state_range"},
		{"dsv_descriptors",7,0,4,0,1496,0,0.,0.,0.,0,0,0,"discrete_state_range"},
		{"dsv_initial_state",5,0,1,0,1490,0,0.,0.,0.,0,0,0,"discrete_state_range"},
		{"dsv_lower_bounds",5,0,2,0,1492,0,0.,0.,0.,0,0,0,"discrete_state_range"},
		{"dsv_upper_bounds",5,0,3,0,1494,0,0.,0.,0.,0,0,0,"discrete_state_range"},
		{"initial_state",13,0,1,0,1491,0,0.,0.,0.,0,"{Initial states} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSRIV",0,"discrete_state_range"},
		{"lower_bounds",13,0,2,0,1493,0,0.,0.,0.,0,"{Lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSRIV",0,"discrete_state_range"},
		{"upper_bounds",13,0,3,0,1495,0,0.,0.,0.,0,"{Upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSRIV",0,"discrete_state_range"}
		},
	kw_203[4] = {
		{"descriptors",15,0,4,0,1507,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSIV",0,"discrete_state_set_integer"},
		{"initial_state",13,0,1,0,1501,0,0.,0.,0.,0,"{Initial state} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSIV",0,"discrete_state_set_integer"},
		{"num_set_values",13,0,2,0,1503,0,0.,0.,0.,0,"{Number of values for each variable} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSIV",0,"discrete_state_set_integer"},
		{"set_values",13,0,3,1,1505,0,0.,0.,0.,0,"{Set values} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSIV"}
		},
	kw_204[4] = {
		{"descriptors",15,0,4,0,1517,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSRV",0,"discrete_state_set_real"},
		{"initial_state",14,0,1,0,1511,0,0.,0.,0.,0,"{Initial state} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSRV",0,"discrete_state_set_real"},
		{"num_set_values",13,0,2,0,1513,0,0.,0.,0.,0,"{Number of values for each variable} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSRV",0,"discrete_state_set_real"},
		{"set_values",14,0,3,1,1515,0,0.,0.,0.,0,"{Set values} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSRV"}
		},
	kw_205[4] = {
		{"betas",14,0,1,1,1359,0,0.,0.,0.,0,"{exponential uncertain betas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Exponential",0,"exponential_uncertain"},
		{"descriptors",15,0,2,0,1361,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Exponential",0,"exponential_uncertain"},
		{"euv_betas",6,0,1,1,1358,0,0.,0.,0.,0,0,0,"exponential_uncertain"},
		{"euv_descriptors",7,0,2,0,1360,0,0.,0.,0.,0,0,0,"exponential_uncertain"}
		},
	kw_206[6] = {
		{"alphas",14,0,1,1,1393,0,0.,0.,0.,0,"{frechet uncertain alphas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Frechet",0,"frechet_uncertain"},
		{"betas",14,0,2,2,1395,0,0.,0.,0.,0,"{frechet uncertain betas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Frechet",0,"frechet_uncertain"},
		{"descriptors",15,0,3,0,1397,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Frechet",0,"frechet_uncertain"},
		{"fuv_alphas",6,0,1,1,1392,0,0.,0.,0.,0,0,0,"frechet_uncertain"},
		{"fuv_betas",6,0,2,2,1394,0,0.,0.,0.,0,0,0,"frechet_uncertain"},
		{"fuv_descriptors",7,0,3,0,1396,0,0.,0.,0.,0,0,0,"frechet_uncertain"}
		},
	kw_207[6] = {
		{"alphas",14,0,1,1,1377,0,0.,0.,0.,0,"{gamma uncertain alphas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gamma",0,"gamma_uncertain"},
		{"betas",14,0,2,2,1379,0,0.,0.,0.,0,"{gamma uncertain betas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gamma",0,"gamma_uncertain"},
		{"descriptors",15,0,3,0,1381,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gamma",0,"gamma_uncertain"},
		{"gauv_alphas",6,0,1,1,1376,0,0.,0.,0.,0,0,0,"gamma_uncertain"},
		{"gauv_betas",6,0,2,2,1378,0,0.,0.,0.,0,0,0,"gamma_uncertain"},
		{"gauv_descriptors",7,0,3,0,1380,0,0.,0.,0.,0,0,0,"gamma_uncertain"}
		},
	kw_208[2] = {
		{"descriptors",15,0,2,0,1445,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Geometric",0,"geometric_uncertain"},
		{"prob_per_trial",14,0,1,1,1443,0,0.,0.,0.,0,"{geometric uncertain prob_per_trial} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Geometric",0,"geometric_uncertain"}
		},
	kw_209[6] = {
		{"alphas",14,0,1,1,1385,0,0.,0.,0.,0,"{gumbel uncertain alphas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gumbel",0,"gumbel_uncertain"},
		{"betas",14,0,2,2,1387,0,0.,0.,0.,0,"{gumbel uncertain betas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gumbel",0,"gumbel_uncertain"},
		{"descriptors",15,0,3,0,1389,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gumbel",0,"gumbel_uncertain"},
		{"guuv_alphas",6,0,1,1,1384,0,0.,0.,0.,0,0,0,"gumbel_uncertain"},
		{"guuv_betas",6,0,2,2,1386,0,0.,0.,0.,0,0,0,"gumbel_uncertain"},
		{"guuv_descriptors",7,0,3,0,1388,0,0.,0.,0.,0,0,0,"gumbel_uncertain"}
		},
	kw_210[10] = {
		{"abscissas",14,0,2,1,1411,0,0.,0.,0.,0,"{sets of abscissas for bin-based histogram variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram"},
		{"counts",14,0,3,2,1415,0,0.,0.,0.,0,"{sets of counts for bin-based histogram variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram"},
		{"descriptors",15,0,4,0,1417,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram",0,"histogram_bin_uncertain"},
		{"huv_bin_abscissas",6,0,2,1,1410},
		{"huv_bin_counts",6,0,3,2,1414},
		{"huv_bin_descriptors",7,0,4,0,1416,0,0.,0.,0.,0,0,0,"histogram_bin_uncertain"},
		{"huv_bin_ordinates",6,0,3,2,1412},
		{"huv_num_bin_pairs",5,0,1,0,1408,0,0.,0.,0.,0,0,0,"histogram_bin_uncertain"},
		{"num_pairs",13,0,1,0,1409,0,0.,0.,0.,0,"{key to apportionment among bin-based histogram variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram",0,"histogram_bin_uncertain"},
		{"ordinates",14,0,3,2,1413,0,0.,0.,0.,0,"{sets of ordinates for bin-based histogram variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram"}
		},
	kw_211[8] = {
		{"abscissas",14,0,2,1,1461,0,0.,0.,0.,0,"{sets of abscissas for point-based histogram variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Point_Histogram"},
		{"counts",14,0,3,2,1463,0,0.,0.,0.,0,"{sets of counts for point-based histogram variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Point_Histogram"},
		{"descriptors",15,0,4,0,1465,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Point_Histogram",0,"histogram_point_uncertain"},
		{"huv_num_point_pairs",5,0,1,0,1458,0,0.,0.,0.,0,0,0,"histogram_point_uncertain"},
		{"huv_point_abscissas",6,0,2,1,1460},
		{"huv_point_counts",6,0,3,2,1462},
		{"huv_point_descriptors",7,0,4,0,1464,0,0.,0.,0.,0,0,0,"histogram_point_uncertain"},
		{"num_pairs",13,0,1,0,1459,0,0.,0.,0.,0,"{key to apportionment among point-based histogram variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Point_Histogram",0,"histogram_point_uncertain"}
		},
	kw_212[4] = {
		{"descriptors",15,0,4,0,1455,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Hypergeometric",0,"hypergeometric_uncertain"},
		{"num_drawn",13,0,3,3,1453,0,0.,0.,0.,0,"{hypergeometric uncertain num_drawn } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Hypergeometric",0,"hypergeometric_uncertain"},
		{"selected_population",13,0,2,2,1451,0,0.,0.,0.,0,"{hypergeometric uncertain selected_population} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Hypergeometric",0,"hypergeometric_uncertain"},
		{"total_population",13,0,1,1,1449,0,0.,0.,0.,0,"{hypergeometric uncertain total_population} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Hypergeometric",0,"hypergeometric_uncertain"}
		},
	kw_213[8] = {
		{"descriptors",15,0,4,0,1477,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCEUV_Interval",0,"interval_uncertain"},
		{"interval_bounds",14,0,3,2,1475,0,0.,0.,0.,0,"{bounds per interval} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCEUV_Interval"},
		{"interval_probs",14,0,2,1,1473,0,0.,0.,0.,0,"{basic probability assignments per interval} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCEUV_Interval"},
		{"iuv_descriptors",7,0,4,0,1476,0,0.,0.,0.,0,0,0,"interval_uncertain"},
		{"iuv_interval_bounds",6,0,3,2,1474},
		{"iuv_interval_probs",6,0,2,1,1472},
		{"iuv_num_intervals",5,0,1,0,1470,0,0.,0.,0.,0,0,0,"interval_uncertain"},
		{"num_intervals",13,0,1,0,1471,0,0.,0.,0.,0,"{number of intervals defined for each interval variable} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCEUV_Interval",0,"interval_uncertain"}
		},
	kw_214[2] = {
		{"lnuv_zetas",6,0,1,1,1316,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"zetas",14,0,1,1,1317,0,0.,0.,0.,0,"{lognormal uncertain zetas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"}
		},
	kw_215[4] = {
		{"error_factors",14,0,1,1,1323,0,0.,0.,0.,0,"[CHOOSE variance spec.]{lognormal uncertain error factors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"},
		{"lnuv_error_factors",6,0,1,1,1322,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lnuv_std_deviations",6,0,1,1,1320,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"std_deviations",14,0,1,1,1321,0,0.,0.,0.,0,"@{lognormal uncertain standard deviations} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"}
		},
	kw_216[10] = {
		{"descriptors",15,0,4,0,1329,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"},
		{"lambdas",14,2,1,1,1315,kw_214,0.,0.,0.,0,"[CHOOSE characterization]{lognormal uncertain lambdas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"},
		{"lnuv_descriptors",7,0,4,0,1328,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lnuv_lambdas",6,2,1,1,1314,kw_214,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lnuv_lower_bounds",6,0,2,0,1324,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lnuv_means",6,4,1,1,1318,kw_215,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lnuv_upper_bounds",6,0,3,0,1326,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lower_bounds",14,0,2,0,1325,0,0.,0.,0.,0,"{Distribution lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"},
		{"means",14,4,1,1,1319,kw_215,0.,0.,0.,0,"@{lognormal uncertain means} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"},
		{"upper_bounds",14,0,3,0,1327,0,0.,0.,0.,0,"{Distribution upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"}
		},
	kw_217[6] = {
		{"descriptors",15,0,3,0,1345,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Loguniform",0,"loguniform_uncertain"},
		{"lower_bounds",14,0,1,1,1341,0,0.,0.,0.,0,"{Distribution lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Loguniform",0,"loguniform_uncertain"},
		{"luuv_descriptors",7,0,3,0,1344,0,0.,0.,0.,0,0,0,"loguniform_uncertain"},
		{"luuv_lower_bounds",6,0,1,1,1340,0,0.,0.,0.,0,0,0,"loguniform_uncertain"},
		{"luuv_upper_bounds",6,0,2,2,1342,0,0.,0.,0.,0,0,0,"loguniform_uncertain"},
		{"upper_bounds",14,0,2,2,1343,0,0.,0.,0.,0,"{Distribution upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Loguniform",0,"loguniform_uncertain"}
		},
	kw_218[3] = {
		{"descriptors",15,0,3,0,1439,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Negative_Binomial",0,"negative_binomial_uncertain"},
		{"num_trials",13,0,2,2,1437,0,0.,0.,0.,0,"{negative binomial uncertain success num_trials} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Negative_Binomial",0,"negative_binomial_uncertain"},
		{"prob_per_trial",14,0,1,1,1435,0,0.,0.,0.,0,"{negative binomial uncertain success prob_per_trial} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Negative_Binomial",0,"negative_binomial_uncertain"}
		},
	kw_219[10] = {
		{"descriptors",15,0,5,0,1311,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal",0,"normal_uncertain"},
		{"lower_bounds",14,0,3,0,1307,0,0.,0.,0.,0,"{Distribution lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal",0,"normal_uncertain"},
		{"means",14,0,1,1,1303,0,0.,0.,0.,0,"{normal uncertain means} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal",0,"normal_uncertain"},
		{"nuv_descriptors",7,0,5,0,1310,0,0.,0.,0.,0,0,0,"normal_uncertain"},
		{"nuv_lower_bounds",6,0,3,0,1306,0,0.,0.,0.,0,0,0,"normal_uncertain"},
		{"nuv_means",6,0,1,1,1302,0,0.,0.,0.,0,0,0,"normal_uncertain"},
		{"nuv_std_deviations",6,0,2,2,1304,0,0.,0.,0.,0,0,0,"normal_uncertain"},
		{"nuv_upper_bounds",6,0,4,0,1308,0,0.,0.,0.,0,0,0,"normal_uncertain"},
		{"std_deviations",14,0,2,2,1305,0,0.,0.,0.,0,"{normal uncertain standard deviations} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal",0,"normal_uncertain"},
		{"upper_bounds",14,0,4,0,1309,0,0.,0.,0.,0,"{Distribution upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal",0,"normal_uncertain"}
		},
	kw_220[2] = {
		{"descriptors",15,0,2,0,1423,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Poisson",0,"poisson_uncertain"},
		{"lambdas",14,0,1,1,1421,0,0.,0.,0.,0,"{poisson uncertain lambdas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Poisson",0,"poisson_uncertain"}
		},
	kw_221[8] = {
		{"descriptors",15,0,4,0,1355,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Triangular",0,"triangular_uncertain"},
		{"lower_bounds",14,0,2,2,1351,0,0.,0.,0.,0,"{Distribution lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Triangular",0,"triangular_uncertain"},
		{"modes",14,0,1,1,1349,0,0.,0.,0.,0,"{triangular uncertain modes} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Triangular",0,"triangular_uncertain"},
		{"tuv_descriptors",7,0,4,0,1354,0,0.,0.,0.,0,0,0,"triangular_uncertain"},
		{"tuv_lower_bounds",6,0,2,2,1350,0,0.,0.,0.,0,0,0,"triangular_uncertain"},
		{"tuv_modes",6,0,1,1,1348,0,0.,0.,0.,0,0,0,"triangular_uncertain"},
		{"tuv_upper_bounds",6,0,3,3,1352,0,0.,0.,0.,0,0,0,"triangular_uncertain"},
		{"upper_bounds",14,0,3,3,1353,0,0.,0.,0.,0,"{Distribution upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Triangular",0,"triangular_uncertain"}
		},
	kw_222[6] = {
		{"descriptors",15,0,3,0,1337,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Uniform",0,"uniform_uncertain"},
		{"lower_bounds",14,0,1,1,1333,0,0.,0.,0.,0,"{Distribution lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Uniform",0,"uniform_uncertain"},
		{"upper_bounds",14,0,2,2,1335,0,0.,0.,0.,0,"{Distribution upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Uniform",0,"uniform_uncertain"},
		{"uuv_descriptors",7,0,3,0,1336,0,0.,0.,0.,0,0,0,"uniform_uncertain"},
		{"uuv_lower_bounds",6,0,1,1,1332,0,0.,0.,0.,0,0,0,"uniform_uncertain"},
		{"uuv_upper_bounds",6,0,2,2,1334,0,0.,0.,0.,0,0,0,"uniform_uncertain"}
		},
	kw_223[6] = {
		{"alphas",14,0,1,1,1401,0,0.,0.,0.,0,"{weibull uncertain alphas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Weibull",0,"weibull_uncertain"},
		{"betas",14,0,2,2,1403,0,0.,0.,0.,0,"{weibull uncertain betas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Weibull",0,"weibull_uncertain"},
		{"descriptors",15,0,3,0,1405,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Weibull",0,"weibull_uncertain"},
		{"wuv_alphas",6,0,1,1,1400,0,0.,0.,0.,0,0,0,"weibull_uncertain"},
		{"wuv_betas",6,0,2,2,1402,0,0.,0.,0.,0,0,0,"weibull_uncertain"},
		{"wuv_descriptors",7,0,3,0,1404,0,0.,0.,0.,0,0,0,"weibull_uncertain"}
		},
	kw_224[29] = {
		{"beta_uncertain",0x19,10,12,0,1363,kw_195,0.,0.,0.,0,"{beta uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta","Continuous Aleatory Uncertain"},
		{"binomial_uncertain",0x19,3,19,0,1425,kw_196,0.,0.,0.,0,"{binomial uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Binomial","Discrete Aleatory Uncertain"},
		{"continuous_design",0x19,12,2,0,1257,kw_197,0.,0.,0.,0,"{Continuous design variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCDV","Design Variables"},
		{"continuous_state",0x19,8,26,0,1479,kw_198,0.,0.,0.,0,"{Continuous state variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCSV","State Variables"},
		{"discrete_design_range",0x19,8,3,0,1271,kw_199,0.,0.,0.,0,"{Discrete design range variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDRIV","Design Variables"},
		{"discrete_design_set_integer",0x19,4,4,0,1281,kw_200,0.,0.,0.,0,"{Discrete design set of integer variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSIV","Design Variables"},
		{"discrete_design_set_real",0x19,4,5,0,1291,kw_201,0.,0.,0.,0,"{Discrete design set of real variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSRV","Design Variables"},
		{"discrete_state_range",0x19,8,27,0,1489,kw_202,0.,0.,0.,0,"{Discrete state range variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSRIV","State Variables"},
		{"discrete_state_set_integer",0x19,4,28,0,1499,kw_203,0.,0.,0.,0,"{Discrete state set of integer variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSIV","State Variables"},
		{"discrete_state_set_real",0x19,4,29,0,1509,kw_204,0.,0.,0.,0,"{Discrete state set of real variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSRV","State Variables"},
		{"exponential_uncertain",0x19,4,11,0,1357,kw_205,0.,0.,0.,0,"{exponential uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Exponential","Continuous Aleatory Uncertain"},
		{"frechet_uncertain",0x19,6,15,0,1391,kw_206,0.,0.,0.,0,"{frechet uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Frechet","Continuous Aleatory Uncertain"},
		{"gamma_uncertain",0x19,6,13,0,1375,kw_207,0.,0.,0.,0,"{gamma uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gamma","Continuous Aleatory Uncertain"},
		{"geometric_uncertain",0x19,2,21,0,1441,kw_208,0.,0.,0.,0,"{geometric uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Geometric","Discrete Aleatory Uncertain"},
		{"gumbel_uncertain",0x19,6,14,0,1383,kw_209,0.,0.,0.,0,"{gumbel uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gumbel","Continuous Aleatory Uncertain"},
		{"histogram_bin_uncertain",0x19,10,17,0,1407,kw_210,0.,0.,0.,0,"{histogram bin uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram","Continuous Aleatory Uncertain"},
		{"histogram_point_uncertain",0x19,8,23,0,1457,kw_211,0.,0.,0.,0,"{histogram point uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Point_Histogram","Discrete Aleatory Uncertain"},
		{"hypergeometric_uncertain",0x19,4,22,0,1447,kw_212,0.,0.,0.,0,"{hypergeometric uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Hypergeometric","Discrete Aleatory Uncertain"},
		{"id_variables",11,0,1,0,1255,0,0.,0.,0.,0,"{Variables set identifier} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarSetId"},
		{"interval_uncertain",0x19,8,25,0,1469,kw_213,0.,0.,0.,0,"{interval uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCEUV_Interval","Epistemic Uncertain"},
		{"lognormal_uncertain",0x19,10,7,0,1313,kw_216,0.,0.,0.,0,"{lognormal uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal","Continuous Aleatory Uncertain"},
		{"loguniform_uncertain",0x19,6,9,0,1339,kw_217,0.,0.,0.,0,"{loguniform uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Loguniform","Continuous Aleatory Uncertain"},
		{"negative_binomial_uncertain",0x19,3,20,0,1433,kw_218,0.,0.,0.,0,"{negative binomial uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Negative_Binomial","Discrete Aleatory Uncertain"},
		{"normal_uncertain",0x19,10,6,0,1301,kw_219,0.,0.,0.,0,"{normal uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal","Continuous Aleatory Uncertain"},
		{"poisson_uncertain",0x19,2,18,0,1419,kw_220,0.,0.,0.,0,"{poisson uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Poisson","Discrete Aleatory Uncertain"},
		{"triangular_uncertain",0x19,8,10,0,1347,kw_221,0.,0.,0.,0,"{triangular uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Triangular","Continuous Aleatory Uncertain"},
		{"uncertain_correlation_matrix",14,0,24,0,1467,0,0.,0.,0.,0,"{correlations in aleatory uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarAUV_Correlations","Aleatory Uncertain Correlations"},
		{"uniform_uncertain",0x19,6,8,0,1331,kw_222,0.,0.,0.,0,"{uniform uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Uniform","Continuous Aleatory Uncertain"},
		{"weibull_uncertain",0x19,6,16,0,1399,kw_223,0.,0.,0.,0,"{weibull uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Weibull","Continuous Aleatory Uncertain"}
		},
	kw_225[6] = {
		{"interface",0x308,10,5,5,1519,kw_9,0.,0.,0.,0,"{Interface} An interface specifies how function evaluations will be performed in order to map a set of parameters into a set of responses. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/InterfCommands.html"},
		{"method",0x308,75,2,2,59,kw_143,0.,0.,0.,0,"{Method} A method specifies the name and controls of an iterative procedure, e.g., a sensitivity analysis, uncertainty quantification, or optimization method. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/MethodCommands.html"},
		{"model",8,6,3,3,1081,kw_166,0.,0.,0.,0,"{Model} A model consists of a model type and maps specified variables through an interface to generate responses. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/ModelCommands.html"},
		{"responses",0x308,15,6,6,1613,kw_183,0.,0.,0.,0,"{Responses} A responses object specifies the data that can be returned to DAKOTA through the interface after the completion of a function evaluation. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/RespCommands.html"},
		{"strategy",0x108,10,1,1,1,kw_194,0.,0.,0.,0,"{Strategy} The strategy specifies the top level technique which will govern the management of iterators and models in the solution of the problem of interest. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/StratCommands.html"},
		{"variables",0x308,29,4,4,1253,kw_224,0.,0.,0.,0,"{Variables} A variables object specifies the parameter set to be iterated by a particular method. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/VarCommands.html"}
		};

} // namespace Dakota

KeyWord Dakota_Keyword_Top = {"KeywordTop",0,6,0,0,Dakota::kw_225};

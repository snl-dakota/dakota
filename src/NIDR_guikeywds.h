
namespace Dakota {

/** 862 distinct keywords (plus 103 aliases) **/

static GuiKeyWord
	kw_1[3] = {
		{"active_set_vector",8,0,1,0,1567},
		{"evaluation_cache",8,0,2,0,1569},
		{"restart_file",8,0,3,0,1571}
		},
	kw_2[1] = {
		{"processors_per_analysis",9,0,1,0,1551,0,0.,0.,0.,0,"{Number of processors per analysis} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicDF"}
		},
	kw_3[4] = {
		{"abort",8,0,1,1,1557,0,0.,0.,0.,0,"@[CHOOSE failure mitigation]"},
		{"continuation",8,0,1,1,1563},
		{"recover",14,0,1,1,1561},
		{"retry",9,0,1,1,1559}
		},
	kw_4[2] = {
		{"copy",8,0,1,0,1545,0,0.,0.,0.,0,"{Copy template files} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"replace",8,0,2,0,1547,0,0.,0.,0.,0,"{Replace existing files} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"}
		},
	kw_5[7] = {
		{"dir_save",0,0,3,0,1538},
		{"dir_tag",0,0,2,0,1536},
		{"directory_save",8,0,3,0,1539,0,0.,0.,0.,0,"{Save work directory} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"directory_tag",8,0,2,0,1537,0,0.,0.,0.,0,"{Tag work directory} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"named",11,0,1,0,1535,0,0.,0.,0.,0,"{Name of work directory} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"template_directory",11,2,4,0,1541,kw_4,0.,0.,0.,0,"{Template directory} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"template_files",15,2,4,0,1543,kw_4,0.,0.,0.,0,"{Template files} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"}
		},
	kw_6[8] = {
		{"allow_existing_results",8,0,3,0,1523,0,0.,0.,0.,0,"{Allow existing results files} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"aprepro",8,0,5,0,1527,0,0.,0.,0.,0,"{Aprepro parameters file format} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"file_save",8,0,7,0,1531,0,0.,0.,0.,0,"{Parameters and results file saving} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"file_tag",8,0,6,0,1529,0,0.,0.,0.,0,"{Parameters and results file tagging} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"parameters_file",11,0,1,0,1519,0,0.,0.,0.,0,"{Parameters file name} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"results_file",11,0,2,0,1521,0,0.,0.,0.,0,"{Results file name} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"verbatim",8,0,4,0,1525,0,0.,0.,0.,0,"{Verbatim driver/filter invocation syntax} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"},
		{"work_directory",8,7,8,0,1533,kw_5,0.,0.,0.,0,"{Create work directory} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"}
		},
	kw_7[9] = {
		{"analysis_components",15,0,1,0,1509,0,0.,0.,0.,0,"{Additional identifiers for use by the analysis_drivers} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"deactivate",8,3,6,0,1565,kw_1,0.,0.,0.,0,"{Feature deactivation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"direct",8,1,4,1,1549,kw_2,0.,0.,0.,0,"[CHOOSE interface type]{Direct function interface } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicDF"},
		{"failure_capture",8,4,5,0,1555,kw_3,0.,0.,0.,0,"{Failure capturing} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"fork",8,8,4,1,1517,kw_6,0.,0.,0.,0,"@"},
		{"grid",8,0,4,1,1553,0,0.,0.,0.,0,"{Grid interface } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicG"},
		{"input_filter",11,0,2,0,1511,0,0.,0.,0.,0,"{Input filter} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"output_filter",11,0,3,0,1513,0,0.,0.,0.,0,"{Output filter} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"system",8,8,4,1,1515,kw_6,0.,0.,0.,0,"{System call interface } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplicSC"}
		},
	kw_8[4] = {
		{"analysis_concurrency",9,0,3,0,1581,0,0.,0.,0.,0,"{Asynchronous analysis concurrency} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"evaluation_concurrency",9,0,1,0,1575,0,0.,0.,0.,0,"{Asynchronous evaluation concurrency} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"local_evaluation_self_scheduling",8,0,2,0,1577,0,0.,0.,0.,0,"{Self-schedule local evals} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"local_evaluation_static_scheduling",8,0,2,0,1579,0,0.,0.,0.,0,"{Static-schedule local evals} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"}
		},
	kw_9[10] = {
		{"algebraic_mappings",11,0,2,0,1505,0,0.,0.,0.,0,"{Algebraic mappings file} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfAlgebraic"},
		{"analysis_drivers",15,9,3,0,1507,kw_7,0.,0.,0.,0,"{Analysis drivers} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfApplic"},
		{"analysis_self_scheduling",8,0,8,0,1591,0,0.,0.,0.,0,"[CHOOSE analysis sched.]{Self scheduling of analyses} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"analysis_servers",9,0,7,0,1589,0,0.,0.,0.,0,"{Number of analysis servers} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"analysis_static_scheduling",8,0,8,0,1593,0,0.,0.,0.,0,"{Static scheduling of analyses} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"asynchronous",8,4,4,0,1573,kw_8,0.,0.,0.,0,"{Asynchronous interface usage} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"evaluation_self_scheduling",8,0,6,0,1585,0,0.,0.,0.,0,"[CHOOSE evaluation sched.]{Self scheduling of evaluations} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"evaluation_servers",9,0,5,0,1583,0,0.,0.,0.,0,"{Number of evaluation servers} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"evaluation_static_scheduling",8,0,6,0,1587,0,0.,0.,0.,0,"{Static scheduling of evaluations} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"},
		{"id_interface",11,0,1,0,1503,0,0.,0.,0.,0,"{Interface set identifier} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/InterfCommands.html#InterfIndControl"}
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
		{"mt19937",8,0,1,1,799},
		{"rnum2",8,0,1,1,801}
		},
	kw_14[8] = {
		{"gpmsa",8,0,1,1,795},
		{"queso",8,0,1,1,793},
		{"rng",8,2,2,0,797,kw_13},
		{"samples",9,0,7,0,863,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,6,0,865,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"x_obs_data_file",11,0,3,0,803},
		{"y_obs_data_file",11,0,4,0,805},
		{"y_std_data_file",11,0,5,0,807}
		},
	kw_15[3] = {
		{"deltas_per_variable",5,0,2,2,1046},
		{"step_vector",14,0,1,1,1045,0,0.,0.,0.,0,"{Step vector} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSCPS"},
		{"steps_per_variable",13,0,2,2,1047,0,0.,0.,0.,0,"{Number of steps per variable} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSCPS"}
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
		{"drop_tolerance",10,0,1,0,831}
		},
	kw_33[14] = {
		{"box_behnken",8,0,1,1,821,0,0.,0.,0.,0,"[CHOOSE DACE type]"},
		{"central_composite",8,0,1,1,823},
		{"fixed_seed",8,0,5,0,833,0,0.,0.,0.,0,"{Fixed seed flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodDDACE"},
		{"grid",8,0,1,1,811},
		{"lhs",8,0,1,1,817},
		{"main_effects",8,0,2,0,825},
		{"oa_lhs",8,0,1,1,819},
		{"oas",8,0,1,1,815},
		{"quality_metrics",8,0,3,0,827,0,0.,0.,0.,0,"{Quality metrics} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodDDACE"},
		{"random",8,0,1,1,813},
		{"samples",9,0,8,0,863,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,7,0,865,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"symbols",9,0,6,0,835,0,0.,0.,0.,0,"{Number of symbols} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodDDACE"},
		{"variance_based_decomp",8,1,4,0,829,kw_32,0.,0.,0.,0,"{Variance based decomposition} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodDDACE"}
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
	kw_37[1] = {
		{"seed",9,0,7,0,865,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"}
		},
	kw_38[3] = {
		{"grid",8,0,1,1,851,0,0.,0.,0.,0,"[CHOOSE trial type]"},
		{"halton",8,0,1,1,853},
		{"random",8,0,1,1,855,0,0.,0.,0.,0,"@"}
		},
	kw_39[1] = {
		{"drop_tolerance",10,0,1,0,845}
		},
	kw_40[8] = {
		{"fixed_seed",8,0,4,0,847,0,0.,0.,0.,0,"{Fixed seed flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"latinize",8,0,1,0,839,0,0.,0.,0.,0,"{Latinization of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"num_trials",9,0,6,0,857,0,0.,0.,0.,0,"{Number of trials  } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"quality_metrics",8,0,2,0,841,0,0.,0.,0.,0,"{Quality metrics} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"samples",9,0,8,0,863,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,7,0,865,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"trial_type",8,3,5,0,849,kw_38,0.,0.,0.,0,"{Trial type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"variance_based_decomp",8,1,3,0,843,kw_39,0.,0.,0.,0,"{Variance based decomposition} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"}
		},
	kw_41[1] = {
		{"drop_tolerance",10,0,1,0,1019}
		},
	kw_42[10] = {
		{"fixed_sequence",8,0,6,0,1023,0,0.,0.,0.,0,"{Fixed sequence flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"halton",8,0,1,1,1009,0,0.,0.,0.,0,"[CHOOSE sequence type]"},
		{"hammersley",8,0,1,1,1011},
		{"latinize",8,0,2,0,1013,0,0.,0.,0.,0,"{Latinization of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"prime_base",13,0,9,0,1029,0,0.,0.,0.,0,"{Prime bases for sequences} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"quality_metrics",8,0,3,0,1015,0,0.,0.,0.,0,"{Quality metrics} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"samples",9,0,5,0,1021,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"sequence_leap",13,0,8,0,1027,0,0.,0.,0.,0,"{Sequence leaping indices} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"sequence_start",13,0,7,0,1025,0,0.,0.,0.,0,"{Sequence starting indices} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"},
		{"variance_based_decomp",8,1,4,0,1017,kw_41,0.,0.,0.,0,"{Variance based decomposition} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodFSUDACE"}
		},
	kw_43[2] = {
		{"complementary",8,0,1,1,763},
		{"cumulative",8,0,1,1,761}
		},
	kw_44[1] = {
		{"num_gen_reliability_levels",13,0,1,0,771,0,0.,0.,0.,0,"{Number of generalized reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"}
		},
	kw_45[1] = {
		{"num_probability_levels",13,0,1,0,767,0,0.,0.,0.,0,"{Number of probability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"}
		},
	kw_46[2] = {
		{"mt19937",8,0,1,1,775},
		{"rnum2",8,0,1,1,777}
		},
	kw_47[2] = {
		{"gen_reliabilities",8,0,1,1,757},
		{"probabilities",8,0,1,1,755}
		},
	kw_48[2] = {
		{"compute",8,2,2,0,753,kw_47},
		{"num_response_levels",13,0,1,0,751}
		},
	kw_49[9] = {
		{"distribution",8,2,5,0,759,kw_43,0.,0.,0.,0,"{Distribution type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"ego",8,0,1,0,747},
		{"gen_reliability_levels",14,1,7,0,769,kw_44,0.,0.,0.,0,"{Generalized reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"lhs",8,0,1,0,745},
		{"probability_levels",14,1,6,0,765,kw_45,0.,0.,0.,0,"{Probability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"response_levels",14,2,2,0,749,kw_48},
		{"rng",8,2,8,0,773,kw_46},
		{"samples",9,0,4,0,863,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,3,0,865,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"}
		},
	kw_50[2] = {
		{"mt19937",8,0,1,1,787},
		{"rnum2",8,0,1,1,789}
		},
	kw_51[5] = {
		{"ego",8,0,1,0,783},
		{"lhs",8,0,1,0,781},
		{"rng",8,2,2,0,785,kw_50,0.,0.,0.,0,"{Random seed generator} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDGlobalIntervalEst"},
		{"samples",9,0,4,0,863,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,3,0,865,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"}
		},
	kw_52[2] = {
		{"complementary",8,0,1,1,997},
		{"cumulative",8,0,1,1,995}
		},
	kw_53[1] = {
		{"num_gen_reliability_levels",13,0,1,0,1005}
		},
	kw_54[1] = {
		{"num_probability_levels",13,0,1,0,1001}
		},
	kw_55[2] = {
		{"gen_reliabilities",8,0,1,1,991},
		{"probabilities",8,0,1,1,989}
		},
	kw_56[2] = {
		{"compute",8,2,2,0,987,kw_55},
		{"num_response_levels",13,0,1,0,985}
		},
	kw_57[2] = {
		{"mt19937",8,0,1,1,979},
		{"rnum2",8,0,1,1,981}
		},
	kw_58[9] = {
		{"all_variables",8,0,2,0,973,0,0.,0.,0.,0,"{All variables flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDGlobalRel"},
		{"distribution",8,2,6,0,993,kw_52},
		{"gen_reliability_levels",14,1,8,0,1003,kw_53},
		{"probability_levels",14,1,7,0,999,kw_54},
		{"response_levels",14,2,5,0,983,kw_56},
		{"rng",8,2,4,0,977,kw_57},
		{"seed",9,0,3,0,975,0,0.,0.,0.,0,"{Random seed for initial GP construction} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDGlobalRel"},
		{"u_gaussian_process",8,0,1,1,971},
		{"x_gaussian_process",8,0,1,1,969}
		},
	kw_59[2] = {
		{"gen_reliabilities",8,0,1,1,741},
		{"probabilities",8,0,1,1,739}
		},
	kw_60[2] = {
		{"compute",8,2,2,0,737,kw_59},
		{"num_response_levels",13,0,1,0,735}
		},
	kw_61[7] = {
		{"distribution",8,2,4,0,759,kw_43,0.,0.,0.,0,"{Distribution type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"gen_reliability_levels",14,1,6,0,769,kw_44,0.,0.,0.,0,"{Generalized reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"probability_levels",14,1,5,0,765,kw_45,0.,0.,0.,0,"{Probability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"response_levels",14,2,1,0,733,kw_60},
		{"rng",8,2,7,0,773,kw_46},
		{"samples",9,0,3,0,863,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,2,0,865,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"}
		},
	kw_62[1] = {
		{"list_of_points",14,0,1,1,1041,0,0.,0.,0.,0,"{List of points to evaluate} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSLPS"}
		},
	kw_63[2] = {
		{"complementary",8,0,1,1,903},
		{"cumulative",8,0,1,1,901}
		},
	kw_64[1] = {
		{"num_gen_reliability_levels",13,0,1,0,897}
		},
	kw_65[1] = {
		{"num_probability_levels",13,0,1,0,893}
		},
	kw_66[2] = {
		{"gen_reliabilities",8,0,1,1,889},
		{"probabilities",8,0,1,1,887}
		},
	kw_67[2] = {
		{"compute",8,2,2,0,885,kw_66},
		{"num_response_levels",13,0,1,0,883}
		},
	kw_68[6] = {
		{"distribution",8,2,5,0,899,kw_63},
		{"gen_reliability_levels",14,1,4,0,895,kw_64},
		{"nip",8,0,1,0,879},
		{"probability_levels",14,1,3,0,891,kw_65},
		{"response_levels",14,2,2,0,881,kw_67},
		{"sqp",8,0,1,0,877}
		},
	kw_69[2] = {
		{"nip",8,0,1,0,909},
		{"sqp",8,0,1,0,907}
		},
	kw_70[5] = {
		{"adapt_import",8,0,1,1,943},
		{"import",8,0,1,1,941},
		{"mm_adapt_import",8,0,1,1,945},
		{"samples",9,0,2,0,947,0,0.,0.,0.,0,"{Refinement samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDLocalRel"},
		{"seed",9,0,3,0,949,0,0.,0.,0.,0,"{Refinement seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDLocalRel"}
		},
	kw_71[3] = {
		{"first_order",8,0,1,1,935},
		{"sample_refinement",8,5,2,0,939,kw_70},
		{"second_order",8,0,1,1,937}
		},
	kw_72[9] = {
		{"nip",8,0,2,0,931},
		{"no_approx",8,0,1,1,927},
		{"sqp",8,0,2,0,929},
		{"u_taylor_mean",8,0,1,1,917},
		{"u_taylor_mpp",8,0,1,1,921},
		{"u_two_point",8,0,1,1,925},
		{"x_taylor_mean",8,0,1,1,915},
		{"x_taylor_mpp",8,0,1,1,919},
		{"x_two_point",8,0,1,1,923}
		},
	kw_73[1] = {
		{"num_reliability_levels",13,0,1,0,965}
		},
	kw_74[3] = {
		{"gen_reliabilities",8,0,1,1,961},
		{"probabilities",8,0,1,1,957},
		{"reliabilities",8,0,1,1,959}
		},
	kw_75[2] = {
		{"compute",8,3,2,0,955,kw_74},
		{"num_response_levels",13,0,1,0,953}
		},
	kw_76[7] = {
		{"distribution",8,2,5,0,993,kw_52},
		{"gen_reliability_levels",14,1,7,0,1003,kw_53},
		{"integration",8,3,2,0,933,kw_71,0.,0.,0.,0,"{Integration method} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDLocalRel"},
		{"mpp_search",8,9,1,0,913,kw_72,0.,0.,0.,0,"{MPP search type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDLocalRel"},
		{"probability_levels",14,1,6,0,999,kw_54},
		{"reliability_levels",14,1,4,0,963,kw_73},
		{"response_levels",14,2,3,0,951,kw_75}
		},
	kw_77[2] = {
		{"num_offspring",0x19,0,2,0,369,0,0.,0.,0.,0,"{Number of offspring in random shuffle crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"num_parents",0x19,0,1,0,367,0,0.,0.,0.,0,"{Number of parents in random shuffle crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_78[5] = {
		{"crossover_rate",10,0,2,0,371,0,0.,0.,0.,0,"{Crossover rate} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"multi_point_binary",9,0,1,1,359,0,0.,0.,0.,0,"{Multi point binary crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"multi_point_parameterized_binary",9,0,1,1,361,0,0.,0.,0.,0,"{Multi point parameterized binary crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"multi_point_real",9,0,1,1,363,0,0.,0.,0.,0,"{Multi point real crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"shuffle_random",8,2,1,1,365,kw_77,0.,0.,0.,0,"{Random shuffle crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_79[3] = {
		{"flat_file",11,0,1,1,355},
		{"simple_random",8,0,1,1,351},
		{"unique_random",8,0,1,1,353}
		},
	kw_80[1] = {
		{"mutation_scale",10,0,1,0,385,0,0.,0.,0.,0,"{Mutation scale} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_81[6] = {
		{"bit_random",8,0,1,1,375},
		{"mutation_rate",10,0,2,0,387,0,0.,0.,0.,0,"{Mutation rate} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"offset_cauchy",8,1,1,1,381,kw_80},
		{"offset_normal",8,1,1,1,379,kw_80},
		{"offset_uniform",8,1,1,1,383,kw_80},
		{"replace_uniform",8,0,1,1,377}
		},
	kw_82[3] = {
		{"metric_tracker",8,0,1,1,301,0,0.,0.,0.,0,"{Convergence type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{"num_generations",0x29,0,3,0,305,0,0.,0.,0.,0,"{Number generations for metric_tracker converger} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{"percent_change",10,0,2,0,303,0,0.,0.,0.,0,"{Percent change limit for metric_tracker converger} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"}
		},
	kw_83[2] = {
		{"domination_count",8,0,1,1,279},
		{"layer_rank",8,0,1,1,277}
		},
	kw_84[2] = {
		{"distance",14,0,1,1,297},
		{"radial",14,0,1,1,295}
		},
	kw_85[1] = {
		{"orthogonal_distance",14,0,1,1,309,0,0.,0.,0.,0,"{Post_processor distance} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"}
		},
	kw_86[2] = {
		{"shrinkage_fraction",10,0,1,0,291},
		{"shrinkage_percentage",2,0,1,0,290}
		},
	kw_87[4] = {
		{"below_limit",10,2,1,1,289,kw_86,0.,0.,0.,0,"{Below limit selection} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"elitist",8,0,1,1,283},
		{"roulette_wheel",8,0,1,1,285},
		{"unique_roulette_wheel",8,0,1,1,287}
		},
	kw_88[21] = {
		{"convergence_type",8,3,4,0,299,kw_82},
		{"crossover_type",8,5,19,0,357,kw_78,0.,0.,0.,0,"{Crossover type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"fitness_type",8,2,1,0,275,kw_83,0.,0.,0.,0,"{Fitness type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{"initialization_type",8,3,18,0,349,kw_79,0.,0.,0.,0,"{Initialization type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
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
		{"mutation_type",8,6,20,0,373,kw_81,0.,0.,0.,0,"{Mutation type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"niching_type",8,2,3,0,293,kw_84,0.,0.,0.,0,"{Niche pressure type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{"population_size",9,0,15,0,343,0,0.,0.,0.,0,"{Number of population members} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"postprocessor_type",8,1,5,0,307,kw_85,0.,0.,0.,0,"{Post_processor type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{"print_each_pop",8,0,17,0,347,0,0.,0.,0.,0,"{Population output} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"replacement_type",8,4,2,0,281,kw_87,0.,0.,0.,0,"{Replacement type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"seed",9,0,21,0,389,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_89[1] = {
		{"partitions",13,0,1,1,1051,0,0.,0.,0.,0,"{Partitions per variable} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSMPS"}
		},
	kw_90[4] = {
		{"min_boxsize_limit",10,0,2,0,871,0,0.,0.,0.,0,"{Min boxsize limit} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNCSUDC"},
		{"solution_accuracy",2,0,1,0,868},
		{"solution_target",10,0,1,0,869,0,0.,0.,0.,0,"{Solution Target } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNCSUDC"},
		{"volume_boxsize_limit",10,0,3,0,873}
		},
	kw_91[9] = {
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
	kw_92[1] = {
		{"num_reliability_levels",13,0,1,0,717,0,0.,0.,0.,0,"{Number of reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"}
		},
	kw_93[3] = {
		{"gen_reliabilities",8,0,1,1,729},
		{"probabilities",8,0,1,1,725},
		{"reliabilities",8,0,1,1,727}
		},
	kw_94[2] = {
		{"compute",8,3,2,0,723,kw_93,0.,0.,0.,0,"{Target statistics for response levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"num_response_levels",13,0,1,0,721,0,0.,0.,0.,0,"{Number of response levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"}
		},
	kw_95[5] = {
		{"expansion_order",13,0,3,1,623,0,0.,0.,0.,0,"{Expansion order} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"expansion_terms",9,0,3,1,625,0,0.,0.,0.,0,"{Expansion terms} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"reuse_points",8,0,1,0,613},
		{"reuse_samples",0,0,1,0,612},
		{"use_derivatives",8,0,2,0,615}
		},
	kw_96[2] = {
		{"expansion_order",13,0,3,1,623,0,0.,0.,0.,0,"{Expansion order} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"expansion_terms",9,0,3,1,625,0,0.,0.,0.,0,"{Expansion terms} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"}
		},
	kw_97[3] = {
		{"expansion_order",13,0,2,1,623,0,0.,0.,0.,0,"{Expansion order} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"expansion_terms",9,0,2,1,625,0,0.,0.,0.,0,"{Expansion terms} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"incremental_lhs",8,0,1,0,619,0,0.,0.,0.,0,"{Incremental LHS flag for PCE coefficient estimation by expansion_samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"}
		},
	kw_98[3] = {
		{"decay",8,0,1,0,589},
		{"generalized",8,0,1,0,591},
		{"sobol",8,0,1,0,587}
		},
	kw_99[2] = {
		{"adaptive",8,3,1,1,585,kw_98},
		{"uniform",8,0,1,1,583}
		},
	kw_100[2] = {
		{"nested",8,0,1,0,603},
		{"non_nested",8,0,1,0,605}
		},
	kw_101[3] = {
		{"adapt_import",8,0,1,1,637},
		{"import",8,0,1,1,635},
		{"mm_adapt_import",8,0,1,1,639}
		},
	kw_102[2] = {
		{"lhs",8,0,1,1,643},
		{"random",8,0,1,1,645}
		},
	kw_103[3] = {
		{"dimension_preference",14,0,1,0,601,0,0.,0.,0.,0,"{Sparse grid dimension preference} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"nested",8,0,2,0,603},
		{"non_nested",8,0,2,0,605}
		},
	kw_104[2] = {
		{"drop_tolerance",10,0,2,0,631},
		{"univariate_effects",8,0,1,0,629}
		},
	kw_105[23] = {
		{"all_variables",8,0,15,0,711,0,0.,0.,0.,0,"{All variables flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"askey",8,0,2,0,593},
		{"collocation_points",9,5,3,1,609,kw_95,0.,0.,0.,0,"{Number of collocation points for PCE coefficient estimation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"collocation_ratio",10,5,3,1,611,kw_95,0.,0.,0.,0,"{Collocation point oversampling ratio for PCE coefficient estimation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"cubature_integrand",9,0,3,1,607,0,0.,0.,0.,0,"{Cubature integrand order for PCE coefficient estimation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"distribution",8,2,9,0,759,kw_43,0.,0.,0.,0,"{Distribution type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"expansion_import_file",11,2,3,1,621,kw_96,0.,0.,0.,0,"{File name for import of PCE coefficients} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"expansion_samples",9,3,3,1,617,kw_97,0.,0.,0.,0,"{Number of simulation samples for PCE coefficient estimation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"fixed_seed",8,0,16,0,713,0,0.,0.,0.,0,"{Fixed seed flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"gen_reliability_levels",14,1,11,0,769,kw_44,0.,0.,0.,0,"{Generalized reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"p_refinement",8,2,1,0,581,kw_99},
		{"probability_levels",14,1,10,0,765,kw_45,0.,0.,0.,0,"{Probability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"quadrature_order",13,2,3,1,597,kw_100,0.,0.,0.,0,"{Quadrature order for PCE coefficient estimation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"reliability_levels",14,1,13,0,715,kw_92,0.,0.,0.,0,"{Reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"response_levels",14,2,14,0,719,kw_94,0.,0.,0.,0,"{Response levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"rng",8,2,12,0,773,kw_46},
		{"sample_refinement",8,3,5,0,633,kw_101},
		{"sample_type",8,2,6,0,641,kw_102,0.,0.,0.,0,"{Sampling type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"samples",9,0,8,0,863,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,7,0,865,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"sparse_grid_level",9,3,3,1,599,kw_103,0.,0.,0.,0,"{Sparse grid level for PCE coefficient estimation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDPCE"},
		{"variance_based_decomp",8,2,4,0,627,kw_104,0.,0.,0.,0,"{Variance based decomposition} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"wiener",8,0,2,0,595}
		},
	kw_106[1] = {
		{"previous_samples",9,0,1,1,705,0,0.,0.,0.,0,"{Previous samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"}
		},
	kw_107[4] = {
		{"incremental_lhs",8,1,1,1,701,kw_106},
		{"incremental_random",8,1,1,1,703,kw_106},
		{"lhs",8,0,1,1,699},
		{"random",8,0,1,1,697}
		},
	kw_108[1] = {
		{"drop_tolerance",10,0,1,0,709}
		},
	kw_109[12] = {
		{"all_variables",8,0,11,0,711,0,0.,0.,0.,0,"{All variables flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"distribution",8,2,5,0,759,kw_43,0.,0.,0.,0,"{Distribution type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"fixed_seed",8,0,12,0,713,0,0.,0.,0.,0,"{Fixed seed flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"gen_reliability_levels",14,1,7,0,769,kw_44,0.,0.,0.,0,"{Generalized reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"probability_levels",14,1,6,0,765,kw_45,0.,0.,0.,0,"{Probability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"reliability_levels",14,1,9,0,715,kw_92,0.,0.,0.,0,"{Reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"response_levels",14,2,10,0,719,kw_94,0.,0.,0.,0,"{Response levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"rng",8,2,8,0,773,kw_46},
		{"sample_type",8,4,1,0,695,kw_107},
		{"samples",9,0,4,0,863,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,3,0,865,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"variance_based_decomp",8,1,2,0,707,kw_108}
		},
	kw_110[2] = {
		{"generalized",8,0,1,0,657},
		{"sobol",8,0,1,0,655}
		},
	kw_111[2] = {
		{"adaptive",8,2,1,1,653,kw_110},
		{"uniform",8,0,1,1,651}
		},
	kw_112[3] = {
		{"adapt_import",8,0,1,1,683},
		{"import",8,0,1,1,681},
		{"mm_adapt_import",8,0,1,1,685}
		},
	kw_113[2] = {
		{"lhs",8,0,1,1,689},
		{"random",8,0,1,1,691}
		},
	kw_114[1] = {
		{"dimension_preference",14,0,1,0,667,0,0.,0.,0.,0,"{Sparse grid dimension preference} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"}
		},
	kw_115[2] = {
		{"drop_tolerance",10,0,2,0,677},
		{"univariate_effects",8,0,1,0,675}
		},
	kw_116[20] = {
		{"all_variables",8,0,16,0,711,0,0.,0.,0.,0,"{All variables flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"askey",8,0,2,0,659},
		{"distribution",8,2,10,0,759,kw_43,0.,0.,0.,0,"{Distribution type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"fixed_seed",8,0,17,0,713,0,0.,0.,0.,0,"{Fixed seed flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"gen_reliability_levels",14,1,12,0,769,kw_44,0.,0.,0.,0,"{Generalized reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"nested",8,0,4,0,669},
		{"non_nested",8,0,4,0,671},
		{"p_refinement",8,2,1,0,649,kw_111},
		{"probability_levels",14,1,11,0,765,kw_45,0.,0.,0.,0,"{Probability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"quadrature_order",13,0,3,1,663,0,0.,0.,0.,0,"{Quadrature order for collocation points} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"},
		{"reliability_levels",14,1,14,0,715,kw_92,0.,0.,0.,0,"{Reliability levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"response_levels",14,2,15,0,719,kw_94,0.,0.,0.,0,"{Response levels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonD"},
		{"rng",8,2,13,0,773,kw_46},
		{"sample_refinement",8,3,6,0,679,kw_112},
		{"sample_type",8,2,7,0,687,kw_113,0.,0.,0.,0,"{Sampling type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"},
		{"samples",9,0,9,0,863,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,8,0,865,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"sparse_grid_level",9,1,3,1,665,kw_114,0.,0.,0.,0,"{Sparse grid level for collocation points} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDSC"},
		{"variance_based_decomp",8,2,5,0,673,kw_115},
		{"wiener",8,0,2,0,661}
		},
	kw_117[1] = {
		{"misc_options",15,0,1,0,567}
		},
	kw_118[12] = {
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
	kw_119[11] = {
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
	kw_120[10] = {
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
	kw_121[4] = {
		{"gradient_based_line_search",8,0,1,1,215,0,0.,0.,0.,0,"[CHOOSE line search type]"},
		{"tr_pds",8,0,1,1,219},
		{"trust_region",8,0,1,1,217},
		{"value_based_line_search",8,0,1,1,213}
		},
	kw_122[16] = {
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
		{"search_method",8,4,1,0,211,kw_121},
		{"steplength_to_boundary",10,0,4,0,225}
		},
	kw_123[4] = {
		{"debug",8,0,1,1,69,0,0.,0.,0.,0,"[CHOOSE output level]"},
		{"quiet",8,0,1,1,73},
		{"silent",8,0,1,1,75},
		{"verbose",8,0,1,1,71}
		},
	kw_124[3] = {
		{"partitions",13,0,1,0,861,0,0.,0.,0.,0,"{Number of partitions} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSUADE"},
		{"samples",9,0,3,0,863,0,0.,0.,0.,0,"{Number of samples} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"},
		{"seed",9,0,2,0,865,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNonDMC"}
		},
	kw_125[4] = {
		{"converge_order",8,0,1,1,1057},
		{"converge_qoi",8,0,1,1,1059},
		{"estimate_order",8,0,1,1,1055},
		{"refinement_rate",10,0,2,0,1061}
		},
	kw_126[2] = {
		{"num_generations",0x29,0,2,0,341},
		{"percent_change",10,0,1,0,339}
		},
	kw_127[2] = {
		{"num_generations",0x29,0,2,0,335,0,0.,0.,0.,0,"{Number of generations (for convergence test) } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{"percent_change",10,0,1,0,333,0,0.,0.,0.,0,"{Percent change in fitness} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"}
		},
	kw_128[2] = {
		{"average_fitness_tracker",8,2,1,1,337,kw_126},
		{"best_fitness_tracker",8,2,1,1,331,kw_127}
		},
	kw_129[2] = {
		{"constraint_penalty",10,0,2,0,317,0,0.,0.,0.,0,"{Constraint penalty in merit function} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{"merit_function",8,0,1,1,315}
		},
	kw_130[4] = {
		{"elitist",8,0,1,1,321},
		{"favor_feasible",8,0,1,1,323},
		{"roulette_wheel",8,0,1,1,325},
		{"unique_roulette_wheel",8,0,1,1,327}
		},
	kw_131[19] = {
		{"convergence_type",8,2,3,0,329,kw_128,0.,0.,0.,0,"{Convergence type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{"crossover_type",8,5,17,0,357,kw_78,0.,0.,0.,0,"{Crossover type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"fitness_type",8,2,1,0,313,kw_129,0.,0.,0.,0,"{Fitness type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{"initialization_type",8,3,16,0,349,kw_79,0.,0.,0.,0,"{Initialization type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
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
		{"mutation_type",8,6,18,0,373,kw_81,0.,0.,0.,0,"{Mutation type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"population_size",9,0,13,0,343,0,0.,0.,0.,0,"{Number of population members} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"print_each_pop",8,0,15,0,347,0,0.,0.,0.,0,"{Population output} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{"replacement_type",8,4,2,0,319,kw_130,0.,0.,0.,0,"{Replacement type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{"seed",9,0,19,0,389,0,0.,0.,0.,0,"{Random seed} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_132[14] = {
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
	kw_133[3] = {
		{"approx_method_name",11,0,1,1,571,0,0.,0.,0.,0,"[CHOOSE sub-method ref.]{Approximate sub-problem minimization method name} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBG"},
		{"approx_method_pointer",11,0,1,1,573,0,0.,0.,0.,0,"{Approximate sub-problem minimization method pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBG"},
		{"replace_points",8,0,2,0,575,0,0.,0.,0.,0,"{Replace points used in surrogate construction with best points from previous iteration} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBG"}
		},
	kw_134[2] = {
		{"filter",8,0,1,1,143,0,0.,0.,0.,0,"@[CHOOSE acceptance logic]"},
		{"tr_ratio",8,0,1,1,141}
		},
	kw_135[7] = {
		{"augmented_lagrangian_objective",8,0,1,1,119,0,0.,0.,0.,0,"[CHOOSE objective formulation]"},
		{"lagrangian_objective",8,0,1,1,121},
		{"linearized_constraints",8,0,2,2,125,0,0.,0.,0.,0,"[CHOOSE constraint formulation]"},
		{"no_constraints",8,0,2,2,127},
		{"original_constraints",8,0,2,2,123,0,0.,0.,0.,0,"@"},
		{"original_primary",8,0,1,1,115,0,0.,0.,0.,0,"@"},
		{"single_objective",8,0,1,1,117}
		},
	kw_136[1] = {
		{"homotopy",8,0,1,1,147}
		},
	kw_137[4] = {
		{"adaptive_penalty_merit",8,0,1,1,133,0,0.,0.,0.,0,"[CHOOSE merit function]"},
		{"augmented_lagrangian_merit",8,0,1,1,137,0,0.,0.,0.,0,"@"},
		{"lagrangian_merit",8,0,1,1,135},
		{"penalty_merit",8,0,1,1,131}
		},
	kw_138[6] = {
		{"contract_threshold",10,0,3,0,105,0,0.,0.,0.,0,"{Shrink trust region if trust region ratio is below this value} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"contraction_factor",10,0,5,0,109,0,0.,0.,0.,0,"{Trust region contraction factor} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"expand_threshold",10,0,4,0,107,0,0.,0.,0.,0,"{Expand trust region if trust region ratio is above this value} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"expansion_factor",10,0,6,0,111,0,0.,0.,0.,0,"{Trust region expansion factor} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"initial_size",10,0,1,0,101,0,0.,0.,0.,0,"{Trust region initial size (relative to bounds)} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"minimum_size",10,0,2,0,103,0,0.,0.,0.,0,"{Trust region minimum size} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"}
		},
	kw_139[18] = {
		{"acceptance_logic",8,2,7,0,139,kw_134,0.,0.,0.,0,"{SBL iterate acceptance logic} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"approx_method_name",11,0,1,1,91,0,0.,0.,0.,0,"[CHOOSE sub-method ref.]{Approximate sub-problem minimization method name} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"approx_method_pointer",11,0,1,1,93,0,0.,0.,0.,0,"{Approximate sub-problem minimization method pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"approx_subproblem",8,7,5,0,113,kw_135,0.,0.,0.,0,"{Approximate subproblem formulation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"constraint_relax",8,1,8,0,145,kw_136,0.,0.,0.,0,"{SBL constraint relaxation method for infeasible iterates} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"linear_equality_constraint_matrix",14,0,14,0,401,0,0.,0.,0.,0,"{Linear equality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scale_types",15,0,16,0,405,0,0.,0.,0.,0,"{Linear equality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_scales",14,0,17,0,407,0,0.,0.,0.,0,"{Linear equality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_equality_targets",14,0,15,0,403,0,0.,0.,0.,0,"{Linear equality targets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_constraint_matrix",14,0,9,0,391,0,0.,0.,0.,0,"{Linear inequality coefficient matrix} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_lower_bounds",14,0,10,0,393,0,0.,0.,0.,0,"{Linear inequality lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scale_types",15,0,12,0,397,0,0.,0.,0.,0,"{Linear inequality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_scales",14,0,13,0,399,0,0.,0.,0.,0,"{Linear inequality scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"linear_inequality_upper_bounds",14,0,11,0,395,0,0.,0.,0.,0,"{Linear inequality upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"merit_function",8,4,6,0,129,kw_137,0.,0.,0.,0,"{SBL merit function} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"soft_convergence_limit",9,0,2,0,95,0,0.,0.,0.,0,"{Soft convergence limit for SBL iterations} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"trust_region",8,6,4,0,99,kw_138,0.,0.,0.,0,"{Trust region group specification} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"},
		{"truth_surrogate_bypass",8,0,3,0,97,0,0.,0.,0.,0,"{Flag for bypassing lower level surrogates in truth verifications} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodSBL"}
		},
	kw_140[3] = {
		{"final_point",14,0,1,1,1033,0,0.,0.,0.,0,"[CHOOSE final pt or increment]{Termination point of vector} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSVPS"},
		{"num_steps",9,0,2,2,1037,0,0.,0.,0.,0,"{Number of steps along vector} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSVPS"},
		{"step_vector",14,0,1,1,1035,0,0.,0.,0.,0,"{Step vector} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSVPS"}
		},
	kw_141[74] = {
		{"asynch_pattern_search",8,18,10,1,237,kw_12,0.,0.,0.,0,0,"Optimization: Local, Derivative-free"},
		{"bayes_calibration",8,8,10,1,791,kw_14},
		{"centered_parameter_study",8,3,10,1,1043,kw_15,0.,0.,0.,0,"[CHOOSE PSTUDY method]{Centered parameter study} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSCPS","Parameter Studies"},
		{"coliny_apps",0,18,10,1,236,kw_12},
		{"coliny_cobyla",8,7,10,1,455,kw_16,0.,0.,0.,0,0,"Optimization: Local, Derivative-free"},
		{"coliny_direct",8,11,10,1,461,kw_18,0.,0.,0.,0,0,"Optimization: Global"},
		{"coliny_ea",8,14,10,1,479,kw_25,0.,0.,0.,0,0,"Optimization: Global"},
		{"coliny_pattern_search",8,17,10,1,409,kw_29,0.,0.,0.,0,0,"Optimization: Local, Derivative-free"},
		{"coliny_solis_wets",8,13,10,1,441,kw_30,0.,0.,0.,0,0,"Optimization: Local, Derivative-free"},
		{"conmin_frcg",8,9,10,1,177,kw_31,0.,0.,0.,0,"[CHOOSE OPT method]","Optimization: Local, Derivative-based"},
		{"conmin_mfd",8,9,10,1,179,kw_31,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"constraint_tolerance",10,0,8,0,85,0,0.,0.,0.,0,"{Constraint tolerance} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"convergence_tolerance",10,0,7,0,83,0,0.,0.,0.,0,"{Convergence tolerance} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"dace",8,14,10,1,809,kw_33,0.,0.,0.,0,0,"[CHOOSE method category]DACE"},
		{"dl_solver",11,9,10,1,181,kw_31,0.,0.,0.,0,0,"Optimization: Plug-in"},
		{"dot",8,15,10,1,159,kw_35},
		{"dot_bfgs",8,10,10,1,153,kw_36,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"dot_frcg",8,10,10,1,149,kw_36,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"dot_mmfd",8,10,10,1,151,kw_36,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"dot_slp",8,10,10,1,155,kw_36,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"dot_sqp",8,10,10,1,157,kw_36,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"efficient_global",8,1,10,1,577,kw_37,0.,0.,0.,0,0,"Surrogate-based Methods"},
		{"fsu_cvt",8,8,10,1,837,kw_40,0.,0.,0.,0,0,"DACE"},
		{"fsu_quasi_mc",8,10,10,1,1007,kw_42,0.,0.,0.,0,0,"DACE"},
		{"global_evidence",8,9,10,1,743,kw_49},
		{"global_interval_est",8,5,10,1,779,kw_51},
		{"global_reliability",8,9,10,1,967,kw_58},
		{"id_method",11,0,1,0,63,0,0.,0.,0.,0,"{Method set identifier} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"importance_sampling",8,7,10,1,731,kw_61},
		{"list_parameter_study",8,1,10,1,1039,kw_62,0.,0.,0.,0,"{List parameter study} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSLPS","Parameter Studies"},
		{"local_evidence",8,6,10,1,875,kw_68},
		{"local_interval_est",8,2,10,1,905,kw_69},
		{"local_reliability",8,7,10,1,911,kw_76},
		{"max_function_evaluations",9,0,5,0,79,0,0.,0.,0.,0,"{Maximum function evaluations} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"max_iterations",9,0,4,0,77,0,0.,0.,0.,0,"{Maximum iterations} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"model_pointer",11,0,2,0,65,0,0.,0.,0.,0,"{Model pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"moga",8,21,10,1,273,kw_88,0.,0.,0.,0,0,"Optimization: Global"},
		{"multidim_parameter_study",8,1,10,1,1049,kw_89,0.,0.,0.,0,"{Multidimensional parameter study} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSMPS","Parameter Studies"},
		{"ncsu_direct",8,4,10,1,867,kw_90,0.,0.,0.,0,0,"Optimization: Global"},
		{"nl2sol",8,9,10,1,545,kw_91,0.,0.,0.,0,"[CHOOSE LSQ method]","Nonlinear Least Squares"},
		{"nlpql_sqp",8,9,10,1,199,kw_31,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"nlssol_sqp",8,12,10,1,185,kw_118,0.,0.,0.,0,0,"Nonlinear Least Squares"},
		{"nond_bayes_calibration",0,8,10,1,790,kw_14},
		{"nond_global_evidence",0,9,10,1,742,kw_49},
		{"nond_global_interval_est",0,5,10,1,778,kw_51},
		{"nond_global_reliability",0,9,10,1,966,kw_58},
		{"nond_importance_sampling",0,7,10,1,730,kw_61},
		{"nond_local_evidence",0,6,10,1,874,kw_68},
		{"nond_local_interval_est",0,2,10,1,904,kw_69},
		{"nond_local_reliability",0,7,10,1,910,kw_76},
		{"nond_polynomial_chaos",0,23,10,1,578,kw_105},
		{"nond_sampling",0,12,10,1,692,kw_109},
		{"nond_stoch_collocation",0,20,10,1,646,kw_116},
		{"nonlinear_cg",8,1,10,1,565,kw_117,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"npsol_sqp",8,12,10,1,183,kw_118,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"optpp_cg",8,11,10,1,201,kw_119,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"optpp_fd_newton",8,16,10,1,205,kw_122,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"optpp_g_newton",8,16,10,1,207,kw_122,0.,0.,0.,0,0,"Nonlinear Least Squares"},
		{"optpp_newton",8,16,10,1,209,kw_122,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"optpp_pds",8,10,10,1,233,kw_120,0.,0.,0.,0,0,"Optimization: Local, Derivative-free"},
		{"optpp_q_newton",8,16,10,1,203,kw_122,0.,0.,0.,0,0,"Optimization: Local, Derivative-based"},
		{"output",8,4,3,0,67,kw_123,0.,0.,0.,0,"{Output verbosity} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"polynomial_chaos",8,23,10,1,579,kw_105},
		{"psuade_moat",8,3,10,1,859,kw_124,0.,0.,0.,0,"{PSUADE MOAT method} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSUADE","DACE"},
		{"richardson_extrap",8,4,10,1,1053,kw_125},
		{"sampling",8,12,10,1,693,kw_109},
		{"scaling",8,0,9,0,87,0,0.,0.,0.,0,"{Scaling flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"soga",8,19,10,1,311,kw_131,0.,0.,0.,0,0,"Optimization: Global"},
		{"speculative",8,0,6,0,81,0,0.,0.,0.,0,"{Speculative gradients and Hessians} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodIndControl"},
		{"stanford",8,14,10,1,187,kw_132},
		{"stoch_collocation",8,20,10,1,647,kw_116},
		{"surrogate_based_global",8,3,10,1,569,kw_133,0.,0.,0.,0,0,"Surrogate-based Methods"},
		{"surrogate_based_local",8,18,10,1,89,kw_139,0.,0.,0.,0,0,"Surrogate-based Methods"},
		{"vector_parameter_study",8,3,10,1,1031,kw_140,0.,0.,0.,0,"{Vector parameter study} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodPSVPS","Parameter Studies"}
		},
	kw_142[1] = {
		{"optional_interface_responses_pointer",11,0,1,0,1223,0,0.,0.,0.,0,"{Responses pointer for nested model optional interfaces} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelNested"}
		},
	kw_143[4] = {
		{"primary_response_mapping",14,0,3,0,1231,0,0.,0.,0.,0,"{Primary response mappings for nested models} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelNested"},
		{"primary_variable_mapping",15,0,1,0,1227,0,0.,0.,0.,0,"{Primary variable mappings for nested models} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelNested"},
		{"secondary_response_mapping",14,0,4,0,1233,0,0.,0.,0.,0,"{Secondary response mappings for nested models} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelNested"},
		{"secondary_variable_mapping",15,0,2,0,1229,0,0.,0.,0.,0,"{Secondary variable mappings for nested models} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelNested"}
		},
	kw_144[2] = {
		{"optional_interface_pointer",11,1,1,0,1221,kw_142,0.,0.,0.,0,"{Optional interface set pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelNested"},
		{"sub_method_pointer",11,4,2,1,1225,kw_143,0.,0.,0.,0,"{Sub-method pointer for nested models} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelNested"}
		},
	kw_145[1] = {
		{"interface_pointer",11,0,1,0,1073,0,0.,0.,0.,0,"{Interface set pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSingle"}
		},
	kw_146[6] = {
		{"additive",8,0,2,2,1181,0,0.,0.,0.,0,"[CHOOSE correction type]"},
		{"combined",8,0,2,2,1185},
		{"first_order",8,0,1,1,1177,0,0.,0.,0.,0,"[CHOOSE correction order]"},
		{"multiplicative",8,0,2,2,1183},
		{"second_order",8,0,1,1,1179},
		{"zeroth_order",8,0,1,1,1175}
		},
	kw_147[3] = {
		{"constant",8,0,1,1,1087,0,0.,0.,0.,0,"[CHOOSE trend type]"},
		{"linear",8,0,1,1,1089},
		{"reduced_quadratic",8,0,1,1,1091}
		},
	kw_148[2] = {
		{"point_selection",8,0,1,0,1083,0,0.,0.,0.,0,"{GP point selection} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"trend",8,3,2,0,1085,kw_147,0.,0.,0.,0,"{GP trend function} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_149[4] = {
		{"constant",8,0,1,1,1139},
		{"linear",8,0,1,1,1141},
		{"quadratic",8,0,1,1,1145},
		{"reduced_quadratic",8,0,1,1,1143}
		},
	kw_150[4] = {
		{"correlation_lengths",14,0,4,0,1151},
		{"max_trials",9,0,3,0,1149,0,0.,0.,0.,0,"{Kriging maximum trials} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"optimization_method",11,0,2,0,1147},
		{"trend",8,4,1,0,1137,kw_149}
		},
	kw_151[2] = {
		{"cubic",8,0,1,1,1101},
		{"linear",8,0,1,1,1099}
		},
	kw_152[2] = {
		{"interpolation",8,2,2,0,1097,kw_151,0.,0.,0.,0,"{MARS interpolation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"max_bases",9,0,1,0,1095,0,0.,0.,0.,0,"{MARS maximum bases} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_153[2] = {
		{"poly_order",9,0,1,0,1105,0,0.,0.,0.,0,"{MLS polynomial order} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"weight_function",9,0,2,0,1107,0,0.,0.,0.,0,"{MLS weight function} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_154[3] = {
		{"nodes",9,0,1,0,1111,0,0.,0.,0.,0,"{ANN number nodes} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"random_weight",9,0,3,0,1115,0,0.,0.,0.,0,"{ANN random weight} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"range",10,0,2,0,1113,0,0.,0.,0.,0,"{ANN range} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_155[3] = {
		{"cubic",8,0,1,1,1133,0,0.,0.,0.,0,"[CHOOSE polynomial order]"},
		{"linear",8,0,1,1,1129},
		{"quadratic",8,0,1,1,1131}
		},
	kw_156[4] = {
		{"bases",9,0,1,0,1119,0,0.,0.,0.,0,"{RBF number of bases} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"max_pts",9,0,2,0,1121,0,0.,0.,0.,0,"{RBF maximum points} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"max_subsets",9,0,4,0,1125},
		{"min_partition",9,0,3,0,1123,0,0.,0.,0.,0,"{RBF minimum partitions} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"}
		},
	kw_157[3] = {
		{"all",8,0,1,1,1163},
		{"none",8,0,1,1,1167},
		{"region",8,0,1,1,1165}
		},
	kw_158[18] = {
		{"correction",8,6,7,0,1173,kw_146,0.,0.,0.,0,"{Surrogate correction approach} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"dace_method_pointer",11,0,3,0,1159,0,0.,0.,0.,0,"{Design of experiments method pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"diagnostics",15,0,8,0,1187,0,0.,0.,0.,0,"{Print diagnostic metrics about the surrogate goodness of fit} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"gaussian_process",8,2,1,1,1081,kw_148,0.,0.,0.,0,"[CHOOSE surrogate type]{Gaussian process} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"kriging",8,4,1,1,1135,kw_150,0.,0.,0.,0,"{Kriging interpolation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"mars",8,2,1,1,1093,kw_152,0.,0.,0.,0,"{Multivariate adaptive regression splines} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"minimum_points",8,0,2,0,1155},
		{"moving_least_squares",8,2,1,1,1103,kw_153,0.,0.,0.,0,"{Moving least squares} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"neural_network",8,3,1,1,1109,kw_154,0.,0.,0.,0,"{Artificial neural network} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"points_file",11,0,5,0,1169},
		{"polynomial",8,3,1,1,1127,kw_155,0.,0.,0.,0,"{Polynomial} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"radial_basis",8,4,1,1,1117,kw_156},
		{"recommended_points",8,0,2,0,1157},
		{"reuse_points",8,3,4,0,1161,kw_157},
		{"reuse_samples",0,3,4,0,1160,kw_157},
		{"samples_file",3,0,5,0,1168},
		{"total_points",9,0,2,0,1153},
		{"use_derivatives",8,0,6,0,1171}
		},
	kw_159[6] = {
		{"additive",8,0,2,2,1213,0,0.,0.,0.,0,"[CHOOSE correction type]"},
		{"combined",8,0,2,2,1217},
		{"first_order",8,0,1,1,1209,0,0.,0.,0.,0,"[CHOOSE correction order]"},
		{"multiplicative",8,0,2,2,1215},
		{"second_order",8,0,1,1,1211},
		{"zeroth_order",8,0,1,1,1207}
		},
	kw_160[3] = {
		{"correction",8,6,3,3,1205,kw_159,0.,0.,0.,0,"{Surrogate correction approach} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrH"},
		{"high_fidelity_model_pointer",11,0,2,2,1203,0,0.,0.,0.,0,"{Pointer to the high fidelity model specification} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrH"},
		{"low_fidelity_model_pointer",11,0,1,1,1201,0,0.,0.,0.,0,"{Pointer to the low fidelity model specification} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrH"}
		},
	kw_161[2] = {
		{"actual_model_pointer",11,0,2,2,1197,0,0.,0.,0.,0,"{Pointer to the truth model specification} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrMP"},
		{"taylor_series",8,0,1,1,1195,0,0.,0.,0.,0,"{Taylor series local approximation } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrL"}
		},
	kw_162[2] = {
		{"actual_model_pointer",11,0,2,2,1197,0,0.,0.,0.,0,"{Pointer to the truth model specification} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrMP"},
		{"tana",8,0,1,1,1191,0,0.,0.,0.,0,"{Two-point adaptive nonlinear approximation } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrMP"}
		},
	kw_163[5] = {
		{"global",8,18,2,1,1079,kw_158,0.,0.,0.,0,"[CHOOSE surrogate category]{Global approximations } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrG"},
		{"hierarchical",8,3,2,1,1199,kw_160,0.,0.,0.,0,"{Hierarchical approximation } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrH"},
		{"id_surrogates",13,0,1,0,1077,0,0.,0.,0.,0,"{Surrogate response ids} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrogate"},
		{"local",8,2,2,1,1193,kw_161,0.,0.,0.,0,"{Local approximation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrL"},
		{"multipoint",8,2,2,1,1189,kw_162,0.,0.,0.,0,"{Multipoint approximation} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelSurrMP"}
		},
	kw_164[6] = {
		{"id_model",11,0,1,0,1065,0,0.,0.,0.,0,"{Model set identifier} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelIndControl"},
		{"nested",8,2,4,1,1219,kw_144,0.,0.,0.,0,"[CHOOSE model type]"},
		{"responses_pointer",11,0,3,0,1069,0,0.,0.,0.,0,"{Responses set pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelIndControl"},
		{"single",8,1,4,1,1071,kw_145,0.,0.,0.,0,"@"},
		{"surrogate",8,5,4,1,1075,kw_163},
		{"variables_pointer",11,0,2,0,1067,0,0.,0.,0.,0,"{Variables set pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/ModelCommands.html#ModelIndControl"}
		},
	kw_165[1] = {
		{"ignore_bounds",8,0,1,0,1673,0,0.,0.,0.,0,"{Ignore variable bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradMixed"}
		},
	kw_166[10] = {
		{"central",8,0,6,0,1681,0,0.,0.,0.,0,"[CHOOSE difference interval]"},
		{"dakota",8,1,4,0,1671,kw_165,0.,0.,0.,0,"@[CHOOSE gradient source]"},
		{"fd_gradient_step_size",0x406,0,7,0,1682,0,0.,0.,0.001},
		{"fd_step_size",0x40e,0,7,0,1683,0,0.,0.,0.001,0,"{Finite difference step size} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradMixed"},
		{"forward",8,0,6,0,1679,0,0.,0.,0.,0,"@"},
		{"id_analytic_gradients",13,0,2,2,1665,0,0.,0.,0.,0,"{Analytic derivatives function list} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradMixed"},
		{"id_numerical_gradients",13,0,1,1,1663,0,0.,0.,0.,0,"{Numerical derivatives function list} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradMixed"},
		{"interval_type",8,0,5,0,1677,0,0.,0.,0.,0,"{Interval type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradNum"},
		{"method_source",8,0,3,0,1669,0,0.,0.,0.,0,"{Method source} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradNum"},
		{"vendor",8,0,4,0,1675}
		},
	kw_167[2] = {
		{"fd_hessian_step_size",6,0,1,0,1708},
		{"fd_step_size",14,0,1,0,1709,0,0.,0.,0.,0,"{Finite difference step size} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessMixed"}
		},
	kw_168[1] = {
		{"damped",8,0,1,0,1719,0,0.,0.,0.,0,"{Numerical safeguarding of BFGS update} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessMixed"}
		},
	kw_169[2] = {
		{"bfgs",8,1,1,1,1717,kw_168,0.,0.,0.,0,"[CHOOSE Hessian approx.]"},
		{"sr1",8,0,1,1,1721}
		},
	kw_170[5] = {
		{"central",8,0,2,0,1713,0,0.,0.,0.,0,"[CHOOSE difference interval]"},
		{"forward",8,0,2,0,1711,0,0.,0.,0.,0,"@"},
		{"id_analytic_hessians",13,0,4,0,1723,0,0.,0.,0.,0,"{Analytic Hessians function list} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessMixed"},
		{"id_numerical_hessians",13,2,1,0,1707,kw_167,0.,0.,0.,0,"{Numerical Hessians function list} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessMixed"},
		{"id_quasi_hessians",13,2,3,0,1715,kw_169,0.,0.,0.,0,"{Quasi Hessians function list} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessMixed"}
		},
	kw_171[3] = {
		{"nonlinear_equality_scale_types",0x80f,0,2,0,1651,0,0.,0.,0.,0,"{Nonlinear equality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_nonlinear_equality_constraints"},
		{"nonlinear_equality_scales",0x80e,0,3,0,1653,0,0.,0.,0.,0,"{Nonlinear equality constraint scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_nonlinear_equality_constraints"},
		{"nonlinear_equality_targets",14,0,1,0,1649,0,0.,0.,0.,0,"{Nonlinear equality constraint targets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_nonlinear_equality_constraints"}
		},
	kw_172[4] = {
		{"nonlinear_inequality_lower_bounds",14,0,1,0,1639,0,0.,0.,0.,0,"{Nonlinear inequality constraint lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_nonlinear_inequality_constraints"},
		{"nonlinear_inequality_scale_types",0x80f,0,3,0,1643,0,0.,0.,0.,0,"{Nonlinear inequality scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_nonlinear_inequality_constraints"},
		{"nonlinear_inequality_scales",0x80e,0,4,0,1645,0,0.,0.,0.,0,"{Nonlinear inequality constraint scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_nonlinear_inequality_constraints"},
		{"nonlinear_inequality_upper_bounds",14,0,2,0,1641,0,0.,0.,0.,0,"{Nonlinear inequality constraint upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_nonlinear_inequality_constraints"}
		},
	kw_173[6] = {
		{"least_squares_data_file",11,0,1,0,1629,0,0.,0.,0.,0,"{Least squares data source file} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS"},
		{"least_squares_term_scale_types",0x80f,0,2,0,1631,0,0.,0.,0.,0,"{Least squares term scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_least_squares_terms"},
		{"least_squares_term_scales",0x80e,0,3,0,1633,0,0.,0.,0.,0,"{Least squares terms scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_least_squares_terms"},
		{"least_squares_weights",14,0,4,0,1635,0,0.,0.,0.,0,"{Least squares terms weightings} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS",0,"num_least_squares_terms"},
		{"num_nonlinear_equality_constraints",0x29,3,6,0,1647,kw_171,0.,0.,0.,0,"{Number of nonlinear equality constraints} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS"},
		{"num_nonlinear_inequality_constraints",0x29,4,5,0,1637,kw_172,0.,0.,0.,0,"{Number of nonlinear inequality constraints} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS"}
		},
	kw_174[3] = {
		{"nonlinear_equality_scale_types",0x80f,0,2,0,1623,0,0.,0.,0.,0,"{Nonlinear equality constraint scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_nonlinear_equality_constraints"},
		{"nonlinear_equality_scales",0x80e,0,3,0,1625,0,0.,0.,0.,0,"{Nonlinear equality constraint scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_nonlinear_equality_constraints"},
		{"nonlinear_equality_targets",14,0,1,0,1621,0,0.,0.,0.,0,"{Nonlinear equality constraint targets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_nonlinear_equality_constraints"}
		},
	kw_175[4] = {
		{"nonlinear_inequality_lower_bounds",14,0,1,0,1611,0,0.,0.,0.,0,"{Nonlinear inequality constraint lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_nonlinear_inequality_constraints"},
		{"nonlinear_inequality_scale_types",0x80f,0,3,0,1615,0,0.,0.,0.,0,"{Nonlinear inequality constraint scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_nonlinear_inequality_constraints"},
		{"nonlinear_inequality_scales",0x80e,0,4,0,1617,0,0.,0.,0.,0,"{Nonlinear inequality constraint scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_nonlinear_inequality_constraints"},
		{"nonlinear_inequality_upper_bounds",14,0,2,0,1613,0,0.,0.,0.,0,"{Nonlinear inequality constraint upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_nonlinear_inequality_constraints"}
		},
	kw_176[5] = {
		{"multi_objective_weights",14,0,3,0,1607,0,0.,0.,0.,0,"{Multiobjective weightings} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_objective_functions"},
		{"num_nonlinear_equality_constraints",0x29,3,5,0,1619,kw_174,0.,0.,0.,0,"{Number of nonlinear equality constraints} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_nonlinear_equality_constraints"},
		{"num_nonlinear_inequality_constraints",0x29,4,4,0,1609,kw_175,0.,0.,0.,0,"{Number of nonlinear inequality constraints} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt"},
		{"objective_function_scale_types",0x80f,0,1,0,1603,0,0.,0.,0.,0,"{Objective function scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_objective_functions"},
		{"objective_function_scales",0x80e,0,2,0,1605,0,0.,0.,0.,0,"{Objective function scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt",0,"num_objective_functions"}
		},
	kw_177[8] = {
		{"central",8,0,6,0,1681,0,0.,0.,0.,0,"[CHOOSE difference interval]"},
		{"dakota",8,1,4,0,1671,kw_165,0.,0.,0.,0,"@[CHOOSE gradient source]"},
		{"fd_gradient_step_size",0x406,0,7,0,1682,0,0.,0.,0.001},
		{"fd_step_size",0x40e,0,7,0,1683,0,0.,0.,0.001,0,"{Finite difference step size} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradMixed"},
		{"forward",8,0,6,0,1679,0,0.,0.,0.,0,"@"},
		{"interval_type",8,0,5,0,1677,0,0.,0.,0.,0,"{Interval type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradNum"},
		{"method_source",8,0,3,0,1669,0,0.,0.,0.,0,"{Method source} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradNum"},
		{"vendor",8,0,4,0,1675}
		},
	kw_178[4] = {
		{"central",8,0,2,0,1693,0,0.,0.,0.,0,"[CHOOSE difference interval]"},
		{"fd_hessian_step_size",6,0,1,0,1688},
		{"fd_step_size",14,0,1,0,1689,0,0.,0.,0.,0,"{Finite difference step size} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessNum"},
		{"forward",8,0,2,0,1691,0,0.,0.,0.,0,"@"}
		},
	kw_179[1] = {
		{"damped",8,0,1,0,1699,0,0.,0.,0.,0,"{Numerical safeguarding of BFGS update} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessQuasi"}
		},
	kw_180[2] = {
		{"bfgs",8,1,1,1,1697,kw_179,0.,0.,0.,0,"[CHOOSE Hessian approx.]"},
		{"sr1",8,0,1,1,1701}
		},
	kw_181[15] = {
		{"analytic_gradients",8,0,4,2,1659,0,0.,0.,0.,0,"[CHOOSE gradient type]"},
		{"analytic_hessians",8,0,5,3,1703,0,0.,0.,0.,0,"[CHOOSE Hessian type]"},
		{"descriptors",15,0,2,0,1599,0,0.,0.,0.,0,"{Response labels} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespLabels"},
		{"id_responses",11,0,1,0,1597,0,0.,0.,0.,0,"{Responses set identifier} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespSetId"},
		{"mixed_gradients",8,10,4,2,1661,kw_166,0.,0.,0.,0,"{Mixed gradients} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradMixed"},
		{"mixed_hessians",8,5,5,3,1705,kw_170,0.,0.,0.,0,"{Mixed Hessians} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessMixed"},
		{"no_gradients",8,0,4,2,1657,0,0.,0.,0.,0,"@"},
		{"no_hessians",8,0,5,3,1685,0,0.,0.,0.,0,"@"},
		{"num_least_squares_terms",0x29,6,3,1,1627,kw_173,0.,0.,0.,0,"[CHOOSE response type]{{Least squares (calibration)} Number of least squares terms} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnLS"},
		{"num_objective_functions",0x29,5,3,1,1601,kw_176,0.,0.,0.,0,"{{Optimization} Number of objective functions} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnOpt"},
		{"num_response_functions",0x29,0,3,1,1655,0,0.,0.,0.,0,"{{Generic responses} Number of response functions} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespFnGen"},
		{"numerical_gradients",8,8,4,2,1667,kw_177,0.,0.,0.,0,"{Numerical gradients} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespGradNum"},
		{"numerical_hessians",8,4,5,3,1687,kw_178,0.,0.,0.,0,"{Numerical Hessians} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessNum"},
		{"quasi_hessians",8,2,5,3,1695,kw_180,0.,0.,0.,0,"{Quasi Hessians} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/RespCommands.html#RespHessQuasi"},
		{"response_descriptors",7,0,2,0,1598}
		},
	kw_182[1] = {
		{"method_list",15,0,1,1,35,0,0.,0.,0.,0,"{List of methods} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"}
		},
	kw_183[3] = {
		{"global_method_pointer",11,0,1,1,27,0,0.,0.,0.,0,"{Pointer to the global method specification} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"local_method_pointer",11,0,2,2,29,0,0.,0.,0.,0,"{Pointer to the local method specification} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"local_search_probability",10,0,3,0,31,0,0.,0.,0.,0,"{Probability of executing local searches} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"}
		},
	kw_184[2] = {
		{"method_list",15,0,2,1,23,0,0.,0.,0.,0,"{List of methods} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"num_solutions_transferred",9,0,1,0,21,0,0.,0.,0.,0,"{Number of Solutions Transferred } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"}
		},
	kw_185[5] = {
		{"collaborative",8,1,1,1,33,kw_182,0.,0.,0.,0,"[CHOOSE hybrid type]{Collaborative hybrid} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"coupled",0,3,1,1,24,kw_183},
		{"embedded",8,3,1,1,25,kw_183,0.,0.,0.,0,"{Embedded hybrid} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"sequential",8,2,1,1,19,kw_184,0.,0.,0.,0,"{Sequential hybrid} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"uncoupled",0,2,1,1,18,kw_184}
		},
	kw_186[1] = {
		{"seed",9,0,1,0,43,0,0.,0.,0.,0,"{Seed for random starting points} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratMultiStart"}
		},
	kw_187[3] = {
		{"method_pointer",11,0,1,1,39,0,0.,0.,0.,0,"{Method pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratMultiStart"},
		{"random_starts",9,1,2,0,41,kw_186,0.,0.,0.,0,"{Number of random starting points} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratMultiStart"},
		{"starting_points",14,0,3,0,45,0,0.,0.,0.,0,"{List of user-specified starting points} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratMultiStart"}
		},
	kw_188[1] = {
		{"seed",9,0,1,0,53,0,0.,0.,0.,0,"{Seed for random weighting sets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratParetoSet"}
		},
	kw_189[5] = {
		{"method_pointer",11,0,1,1,49,0,0.,0.,0.,0,"{Optimization method pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratParetoSet"},
		{"multi_objective_weight_sets",6,0,3,0,54},
		{"opt_method_pointer",3,0,1,1,48},
		{"random_weight_sets",9,1,2,0,51,kw_188,0.,0.,0.,0,"{Number of random weighting sets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratParetoSet"},
		{"weight_sets",14,0,3,0,55,0,0.,0.,0.,0,"{List of user-specified weighting sets} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratParetoSet"}
		},
	kw_190[1] = {
		{"method_pointer",11,0,1,0,59,0,0.,0.,0.,0,"{Method pointer} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratSingle"}
		},
	kw_191[1] = {
		{"tabular_graphics_file",11,0,1,0,7,0,0.,0.,0.,0,"{File name for tabular graphics data} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratIndControl"}
		},
	kw_192[10] = {
		{"graphics",8,0,1,0,3,0,0.,0.,0.,0,"{Graphics flag} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratIndControl"},
		{"hybrid",8,5,7,1,17,kw_185,0.,0.,0.,0,"[CHOOSE strategy type]{Hybrid strategy} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratHybrid"},
		{"iterator_self_scheduling",8,0,5,0,13,0,0.,0.,0.,0,"{Self-scheduling of iterator jobs} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratIndControl"},
		{"iterator_servers",9,0,4,0,11,0,0.,0.,0.,0,"{Number of iterator servers} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratIndControl"},
		{"iterator_static_scheduling",8,0,6,0,15,0,0.,0.,0.,0,"{Static scheduling of iterator jobs} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratIndControl"},
		{"multi_start",8,3,7,1,37,kw_187,0.,0.,0.,0,"{Multi-start iteration strategy} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratMultiStart"},
		{"output_precision",0x29,0,3,0,9},
		{"pareto_set",8,5,7,1,47,kw_189,0.,0.,0.,0,"{Pareto set optimization strategy} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratParetoSet"},
		{"single_method",8,1,7,1,57,kw_190,0.,0.,0.,0,"@{Single method strategy} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratSingle"},
		{"tabular_graphics_data",8,1,2,0,5,kw_191,0.,0.,0.,0,"{Tabulation of graphics data} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/StratCommands.html#StratIndControl"}
		},
	kw_193[10] = {
		{"alphas",14,0,1,1,1347,0,0.,0.,0.,0,"{beta uncertain alphas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta",0,"beta_uncertain"},
		{"betas",14,0,2,2,1349,0,0.,0.,0.,0,"{beta uncertain betas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta",0,"beta_uncertain"},
		{"buv_alphas",6,0,1,1,1346,0,0.,0.,0.,0,0,0,"beta_uncertain"},
		{"buv_betas",6,0,2,2,1348,0,0.,0.,0.,0,0,0,"beta_uncertain"},
		{"buv_descriptors",7,0,5,0,1354,0,0.,0.,0.,0,0,0,"beta_uncertain"},
		{"buv_lower_bounds",6,0,3,3,1350,0,0.,0.,0.,0,0,0,"beta_uncertain"},
		{"buv_upper_bounds",6,0,4,4,1352,0,0.,0.,0.,0,0,0,"beta_uncertain"},
		{"descriptors",15,0,5,0,1355,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta",0,"beta_uncertain"},
		{"lower_bounds",14,0,3,3,1351,0,0.,0.,0.,0,"{Distribution lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta",0,"beta_uncertain"},
		{"upper_bounds",14,0,4,4,1353,0,0.,0.,0.,0,"{Distribution upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta",0,"beta_uncertain"}
		},
	kw_194[3] = {
		{"descriptors",15,0,3,0,1413,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Binomial",0,"binomial_uncertain"},
		{"num_trials",13,0,2,2,1411,0,0.,0.,0.,0,"{binomial uncertain num_trials} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Binomial",0,"binomial_uncertain"},
		{"prob_per_trial",14,0,1,1,1409,0,0.,0.,0.,0,"{binomial uncertain prob_per_trial} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Binomial",0,"binomial_uncertain"}
		},
	kw_195[12] = {
		{"cdv_descriptors",7,0,6,0,1250,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"cdv_initial_point",6,0,1,0,1240,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"cdv_lower_bounds",6,0,2,0,1242,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"cdv_scale_types",0x807,0,4,0,1246,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"cdv_scales",0x806,0,5,0,1248,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"cdv_upper_bounds",6,0,3,0,1244,0,0.,0.,0.,0,0,0,"continuous_design"},
		{"descriptors",15,0,6,0,1251,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"},
		{"initial_point",14,0,1,0,1241,0,0.,0.,0.,0,"{Initial point} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"},
		{"lower_bounds",14,0,2,0,1243,0,0.,0.,0.,0,"{Lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"},
		{"scale_types",0x80f,0,4,0,1247,0,0.,0.,0.,0,"{Scaling types} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"},
		{"scales",0x80e,0,5,0,1249,0,0.,0.,0.,0,"{Scales} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"},
		{"upper_bounds",14,0,3,0,1245,0,0.,0.,0.,0,"{Upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCDV",0,"continuous_design"}
		},
	kw_196[8] = {
		{"csv_descriptors",7,0,4,0,1468,0,0.,0.,0.,0,0,0,"continuous_state"},
		{"csv_initial_state",6,0,1,0,1462,0,0.,0.,0.,0,0,0,"continuous_state"},
		{"csv_lower_bounds",6,0,2,0,1464,0,0.,0.,0.,0,0,0,"continuous_state"},
		{"csv_upper_bounds",6,0,3,0,1466,0,0.,0.,0.,0,0,0,"continuous_state"},
		{"descriptors",15,0,4,0,1469,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCSV",0,"continuous_state"},
		{"initial_state",14,0,1,0,1463,0,0.,0.,0.,0,"{Initial states} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCSV",0,"continuous_state"},
		{"lower_bounds",14,0,2,0,1465,0,0.,0.,0.,0,"{Lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCSV",0,"continuous_state"},
		{"upper_bounds",14,0,3,0,1467,0,0.,0.,0.,0,"{Upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCSV",0,"continuous_state"}
		},
	kw_197[8] = {
		{"ddv_descriptors",7,0,4,0,1260,0,0.,0.,0.,0,0,0,"discrete_design_range"},
		{"ddv_initial_point",5,0,1,0,1254,0,0.,0.,0.,0,0,0,"discrete_design_range"},
		{"ddv_lower_bounds",5,0,2,0,1256,0,0.,0.,0.,0,0,0,"discrete_design_range"},
		{"ddv_upper_bounds",5,0,3,0,1258,0,0.,0.,0.,0,0,0,"discrete_design_range"},
		{"descriptors",15,0,4,0,1261,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDRIV",0,"discrete_design_range"},
		{"initial_point",13,0,1,0,1255,0,0.,0.,0.,0,"{Initial point} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDRIV",0,"discrete_design_range"},
		{"lower_bounds",13,0,2,0,1257,0,0.,0.,0.,0,"{Lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDRIV",0,"discrete_design_range"},
		{"upper_bounds",13,0,3,0,1259,0,0.,0.,0.,0,"{Upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDRIV",0,"discrete_design_range"}
		},
	kw_198[4] = {
		{"descriptors",15,0,4,0,1271,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSIV",0,"discrete_design_set_integer"},
		{"initial_point",13,0,1,0,1265,0,0.,0.,0.,0,"{Initial point} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSIV",0,"discrete_design_set_integer"},
		{"num_set_values",13,0,2,0,1267,0,0.,0.,0.,0,"{Number of values for each variable} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSIV",0,"discrete_design_set_integer"},
		{"set_values",13,0,3,1,1269,0,0.,0.,0.,0,"{Set values} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSIV"}
		},
	kw_199[4] = {
		{"descriptors",15,0,4,0,1281,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSRV",0,"discrete_design_set_real"},
		{"initial_point",14,0,1,0,1275,0,0.,0.,0.,0,"{Initial point} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSRV",0,"discrete_design_set_real"},
		{"num_set_values",13,0,2,0,1277,0,0.,0.,0.,0,"{Number of values for each variable} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSRV",0,"discrete_design_set_real"},
		{"set_values",14,0,3,1,1279,0,0.,0.,0.,0,"{Set values} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSRV"}
		},
	kw_200[8] = {
		{"descriptors",15,0,4,0,1479,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSRIV",0,"discrete_state_range"},
		{"dsv_descriptors",7,0,4,0,1478,0,0.,0.,0.,0,0,0,"discrete_state_range"},
		{"dsv_initial_state",5,0,1,0,1472,0,0.,0.,0.,0,0,0,"discrete_state_range"},
		{"dsv_lower_bounds",5,0,2,0,1474,0,0.,0.,0.,0,0,0,"discrete_state_range"},
		{"dsv_upper_bounds",5,0,3,0,1476,0,0.,0.,0.,0,0,0,"discrete_state_range"},
		{"initial_state",13,0,1,0,1473,0,0.,0.,0.,0,"{Initial states} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSRIV",0,"discrete_state_range"},
		{"lower_bounds",13,0,2,0,1475,0,0.,0.,0.,0,"{Lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSRIV",0,"discrete_state_range"},
		{"upper_bounds",13,0,3,0,1477,0,0.,0.,0.,0,"{Upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSRIV",0,"discrete_state_range"}
		},
	kw_201[4] = {
		{"descriptors",15,0,4,0,1489,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSIV",0,"discrete_state_set_integer"},
		{"initial_state",13,0,1,0,1483,0,0.,0.,0.,0,"{Initial state} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSIV",0,"discrete_state_set_integer"},
		{"num_set_values",13,0,2,0,1485,0,0.,0.,0.,0,"{Number of values for each variable} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSIV",0,"discrete_state_set_integer"},
		{"set_values",13,0,3,1,1487,0,0.,0.,0.,0,"{Set values} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSIV"}
		},
	kw_202[4] = {
		{"descriptors",15,0,4,0,1499,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSRV",0,"discrete_state_set_real"},
		{"initial_state",14,0,1,0,1493,0,0.,0.,0.,0,"{Initial state} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSRV",0,"discrete_state_set_real"},
		{"num_set_values",13,0,2,0,1495,0,0.,0.,0.,0,"{Number of values for each variable} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSRV",0,"discrete_state_set_real"},
		{"set_values",14,0,3,1,1497,0,0.,0.,0.,0,"{Set values} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSRV"}
		},
	kw_203[4] = {
		{"betas",14,0,1,1,1341,0,0.,0.,0.,0,"{exponential uncertain betas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Exponential",0,"exponential_uncertain"},
		{"descriptors",15,0,2,0,1343,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Exponential",0,"exponential_uncertain"},
		{"euv_betas",6,0,1,1,1340,0,0.,0.,0.,0,0,0,"exponential_uncertain"},
		{"euv_descriptors",7,0,2,0,1342,0,0.,0.,0.,0,0,0,"exponential_uncertain"}
		},
	kw_204[6] = {
		{"alphas",14,0,1,1,1375,0,0.,0.,0.,0,"{frechet uncertain alphas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Frechet",0,"frechet_uncertain"},
		{"betas",14,0,2,2,1377,0,0.,0.,0.,0,"{frechet uncertain betas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Frechet",0,"frechet_uncertain"},
		{"descriptors",15,0,3,0,1379,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Frechet",0,"frechet_uncertain"},
		{"fuv_alphas",6,0,1,1,1374,0,0.,0.,0.,0,0,0,"frechet_uncertain"},
		{"fuv_betas",6,0,2,2,1376,0,0.,0.,0.,0,0,0,"frechet_uncertain"},
		{"fuv_descriptors",7,0,3,0,1378,0,0.,0.,0.,0,0,0,"frechet_uncertain"}
		},
	kw_205[6] = {
		{"alphas",14,0,1,1,1359,0,0.,0.,0.,0,"{gamma uncertain alphas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gamma",0,"gamma_uncertain"},
		{"betas",14,0,2,2,1361,0,0.,0.,0.,0,"{gamma uncertain betas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gamma",0,"gamma_uncertain"},
		{"descriptors",15,0,3,0,1363,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gamma",0,"gamma_uncertain"},
		{"gauv_alphas",6,0,1,1,1358,0,0.,0.,0.,0,0,0,"gamma_uncertain"},
		{"gauv_betas",6,0,2,2,1360,0,0.,0.,0.,0,0,0,"gamma_uncertain"},
		{"gauv_descriptors",7,0,3,0,1362,0,0.,0.,0.,0,0,0,"gamma_uncertain"}
		},
	kw_206[2] = {
		{"descriptors",15,0,2,0,1427,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Geometric",0,"geometric_uncertain"},
		{"prob_per_trial",14,0,1,1,1425,0,0.,0.,0.,0,"{geometric uncertain prob_per_trial} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Geometric",0,"geometric_uncertain"}
		},
	kw_207[6] = {
		{"alphas",14,0,1,1,1367,0,0.,0.,0.,0,"{gumbel uncertain alphas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gumbel",0,"gumbel_uncertain"},
		{"betas",14,0,2,2,1369,0,0.,0.,0.,0,"{gumbel uncertain betas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gumbel",0,"gumbel_uncertain"},
		{"descriptors",15,0,3,0,1371,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gumbel",0,"gumbel_uncertain"},
		{"guuv_alphas",6,0,1,1,1366,0,0.,0.,0.,0,0,0,"gumbel_uncertain"},
		{"guuv_betas",6,0,2,2,1368,0,0.,0.,0.,0,0,0,"gumbel_uncertain"},
		{"guuv_descriptors",7,0,3,0,1370,0,0.,0.,0.,0,0,0,"gumbel_uncertain"}
		},
	kw_208[10] = {
		{"abscissas",14,0,2,1,1393,0,0.,0.,0.,0,"{sets of abscissas for bin-based histogram variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram"},
		{"counts",14,0,3,2,1397,0,0.,0.,0.,0,"{sets of counts for bin-based histogram variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram"},
		{"descriptors",15,0,4,0,1399,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram",0,"histogram_bin_uncertain"},
		{"huv_bin_abscissas",6,0,2,1,1392},
		{"huv_bin_counts",6,0,3,2,1396},
		{"huv_bin_descriptors",7,0,4,0,1398,0,0.,0.,0.,0,0,0,"histogram_bin_uncertain"},
		{"huv_bin_ordinates",6,0,3,2,1394},
		{"huv_num_bin_pairs",5,0,1,0,1390,0,0.,0.,0.,0,0,0,"histogram_bin_uncertain"},
		{"num_pairs",13,0,1,0,1391,0,0.,0.,0.,0,"{key to apportionment among bin-based histogram variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram",0,"histogram_bin_uncertain"},
		{"ordinates",14,0,3,2,1395,0,0.,0.,0.,0,"{sets of ordinates for bin-based histogram variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram"}
		},
	kw_209[8] = {
		{"abscissas",14,0,2,1,1443,0,0.,0.,0.,0,"{sets of abscissas for point-based histogram variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Point_Histogram"},
		{"counts",14,0,3,2,1445,0,0.,0.,0.,0,"{sets of counts for point-based histogram variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Point_Histogram"},
		{"descriptors",15,0,4,0,1447,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Point_Histogram",0,"histogram_point_uncertain"},
		{"huv_num_point_pairs",5,0,1,0,1440,0,0.,0.,0.,0,0,0,"histogram_point_uncertain"},
		{"huv_point_abscissas",6,0,2,1,1442},
		{"huv_point_counts",6,0,3,2,1444},
		{"huv_point_descriptors",7,0,4,0,1446,0,0.,0.,0.,0,0,0,"histogram_point_uncertain"},
		{"num_pairs",13,0,1,0,1441,0,0.,0.,0.,0,"{key to apportionment among point-based histogram variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Point_Histogram",0,"histogram_point_uncertain"}
		},
	kw_210[4] = {
		{"descriptors",15,0,4,0,1437,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Hypergeometric",0,"hypergeometric_uncertain"},
		{"num_drawn",13,0,3,3,1435,0,0.,0.,0.,0,"{hypergeometric uncertain num_drawn } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Hypergeometric",0,"hypergeometric_uncertain"},
		{"selected_population",13,0,2,2,1433,0,0.,0.,0.,0,"{hypergeometric uncertain selected_population} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Hypergeometric",0,"hypergeometric_uncertain"},
		{"total_population",13,0,1,1,1431,0,0.,0.,0.,0,"{hypergeometric uncertain total_population} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Hypergeometric",0,"hypergeometric_uncertain"}
		},
	kw_211[8] = {
		{"descriptors",15,0,4,0,1459,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCEUV_Interval",0,"interval_uncertain"},
		{"interval_bounds",14,0,3,2,1457,0,0.,0.,0.,0,"{bounds per interval} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCEUV_Interval"},
		{"interval_probs",14,0,2,1,1455,0,0.,0.,0.,0,"{basic probability assignments per interval} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCEUV_Interval"},
		{"iuv_descriptors",7,0,4,0,1458,0,0.,0.,0.,0,0,0,"interval_uncertain"},
		{"iuv_interval_bounds",6,0,3,2,1456},
		{"iuv_interval_probs",6,0,2,1,1454},
		{"iuv_num_intervals",5,0,1,0,1452,0,0.,0.,0.,0,0,0,"interval_uncertain"},
		{"num_intervals",13,0,1,0,1453,0,0.,0.,0.,0,"{number of intervals defined for each interval variable} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCEUV_Interval",0,"interval_uncertain"}
		},
	kw_212[2] = {
		{"lnuv_zetas",6,0,1,1,1298,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"zetas",14,0,1,1,1299,0,0.,0.,0.,0,"{lognormal uncertain zetas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"}
		},
	kw_213[4] = {
		{"error_factors",14,0,1,1,1305,0,0.,0.,0.,0,"[CHOOSE variance spec.]{lognormal uncertain error factors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"},
		{"lnuv_error_factors",6,0,1,1,1304,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lnuv_std_deviations",6,0,1,1,1302,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"std_deviations",14,0,1,1,1303,0,0.,0.,0.,0,"@{lognormal uncertain standard deviations} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"}
		},
	kw_214[10] = {
		{"descriptors",15,0,4,0,1311,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"},
		{"lambdas",14,2,1,1,1297,kw_212,0.,0.,0.,0,"[CHOOSE characterization]{lognormal uncertain lambdas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"},
		{"lnuv_descriptors",7,0,4,0,1310,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lnuv_lambdas",6,2,1,1,1296,kw_212,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lnuv_lower_bounds",6,0,2,0,1306,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lnuv_means",6,4,1,1,1300,kw_213,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lnuv_upper_bounds",6,0,3,0,1308,0,0.,0.,0.,0,0,0,"lognormal_uncertain"},
		{"lower_bounds",14,0,2,0,1307,0,0.,0.,0.,0,"{Distribution lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"},
		{"means",14,4,1,1,1301,kw_213,0.,0.,0.,0,"@{lognormal uncertain means} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"},
		{"upper_bounds",14,0,3,0,1309,0,0.,0.,0.,0,"{Distribution upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal",0,"lognormal_uncertain"}
		},
	kw_215[6] = {
		{"descriptors",15,0,3,0,1327,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Loguniform",0,"loguniform_uncertain"},
		{"lower_bounds",14,0,1,1,1323,0,0.,0.,0.,0,"{Distribution lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Loguniform",0,"loguniform_uncertain"},
		{"luuv_descriptors",7,0,3,0,1326,0,0.,0.,0.,0,0,0,"loguniform_uncertain"},
		{"luuv_lower_bounds",6,0,1,1,1322,0,0.,0.,0.,0,0,0,"loguniform_uncertain"},
		{"luuv_upper_bounds",6,0,2,2,1324,0,0.,0.,0.,0,0,0,"loguniform_uncertain"},
		{"upper_bounds",14,0,2,2,1325,0,0.,0.,0.,0,"{Distribution upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Loguniform",0,"loguniform_uncertain"}
		},
	kw_216[3] = {
		{"descriptors",15,0,3,0,1421,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Negative_Binomial",0,"negative_binomial_uncertain"},
		{"num_trials",13,0,2,2,1419,0,0.,0.,0.,0,"{negative binomial uncertain success num_trials} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Negative_Binomial",0,"negative_binomial_uncertain"},
		{"prob_per_trial",14,0,1,1,1417,0,0.,0.,0.,0,"{negative binomial uncertain success prob_per_trial} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Negative_Binomial",0,"negative_binomial_uncertain"}
		},
	kw_217[10] = {
		{"descriptors",15,0,5,0,1293,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal",0,"normal_uncertain"},
		{"lower_bounds",14,0,3,0,1289,0,0.,0.,0.,0,"{Distribution lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal",0,"normal_uncertain"},
		{"means",14,0,1,1,1285,0,0.,0.,0.,0,"{normal uncertain means} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal",0,"normal_uncertain"},
		{"nuv_descriptors",7,0,5,0,1292,0,0.,0.,0.,0,0,0,"normal_uncertain"},
		{"nuv_lower_bounds",6,0,3,0,1288,0,0.,0.,0.,0,0,0,"normal_uncertain"},
		{"nuv_means",6,0,1,1,1284,0,0.,0.,0.,0,0,0,"normal_uncertain"},
		{"nuv_std_deviations",6,0,2,2,1286,0,0.,0.,0.,0,0,0,"normal_uncertain"},
		{"nuv_upper_bounds",6,0,4,0,1290,0,0.,0.,0.,0,0,0,"normal_uncertain"},
		{"std_deviations",14,0,2,2,1287,0,0.,0.,0.,0,"{normal uncertain standard deviations} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal",0,"normal_uncertain"},
		{"upper_bounds",14,0,4,0,1291,0,0.,0.,0.,0,"{Distribution upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal",0,"normal_uncertain"}
		},
	kw_218[2] = {
		{"descriptors",15,0,2,0,1405,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Poisson",0,"poisson_uncertain"},
		{"lambdas",14,0,1,1,1403,0,0.,0.,0.,0,"{poisson uncertain lambdas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Poisson",0,"poisson_uncertain"}
		},
	kw_219[8] = {
		{"descriptors",15,0,4,0,1337,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Triangular",0,"triangular_uncertain"},
		{"lower_bounds",14,0,2,2,1333,0,0.,0.,0.,0,"{Distribution lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Triangular",0,"triangular_uncertain"},
		{"modes",14,0,1,1,1331,0,0.,0.,0.,0,"{triangular uncertain modes} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Triangular",0,"triangular_uncertain"},
		{"tuv_descriptors",7,0,4,0,1336,0,0.,0.,0.,0,0,0,"triangular_uncertain"},
		{"tuv_lower_bounds",6,0,2,2,1332,0,0.,0.,0.,0,0,0,"triangular_uncertain"},
		{"tuv_modes",6,0,1,1,1330,0,0.,0.,0.,0,0,0,"triangular_uncertain"},
		{"tuv_upper_bounds",6,0,3,3,1334,0,0.,0.,0.,0,0,0,"triangular_uncertain"},
		{"upper_bounds",14,0,3,3,1335,0,0.,0.,0.,0,"{Distribution upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Triangular",0,"triangular_uncertain"}
		},
	kw_220[6] = {
		{"descriptors",15,0,3,0,1319,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Uniform",0,"uniform_uncertain"},
		{"lower_bounds",14,0,1,1,1315,0,0.,0.,0.,0,"{Distribution lower bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Uniform",0,"uniform_uncertain"},
		{"upper_bounds",14,0,2,2,1317,0,0.,0.,0.,0,"{Distribution upper bounds} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Uniform",0,"uniform_uncertain"},
		{"uuv_descriptors",7,0,3,0,1318,0,0.,0.,0.,0,0,0,"uniform_uncertain"},
		{"uuv_lower_bounds",6,0,1,1,1314,0,0.,0.,0.,0,0,0,"uniform_uncertain"},
		{"uuv_upper_bounds",6,0,2,2,1316,0,0.,0.,0.,0,0,0,"uniform_uncertain"}
		},
	kw_221[6] = {
		{"alphas",14,0,1,1,1383,0,0.,0.,0.,0,"{weibull uncertain alphas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Weibull",0,"weibull_uncertain"},
		{"betas",14,0,2,2,1385,0,0.,0.,0.,0,"{weibull uncertain betas} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Weibull",0,"weibull_uncertain"},
		{"descriptors",15,0,3,0,1387,0,0.,0.,0.,0,"{Descriptors} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Weibull",0,"weibull_uncertain"},
		{"wuv_alphas",6,0,1,1,1382,0,0.,0.,0.,0,0,0,"weibull_uncertain"},
		{"wuv_betas",6,0,2,2,1384,0,0.,0.,0.,0,0,0,"weibull_uncertain"},
		{"wuv_descriptors",7,0,3,0,1386,0,0.,0.,0.,0,0,0,"weibull_uncertain"}
		},
	kw_222[29] = {
		{"beta_uncertain",0x19,10,12,0,1345,kw_193,0.,0.,0.,0,"{beta uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Beta","Continuous Aleatory Uncertain"},
		{"binomial_uncertain",0x19,3,19,0,1407,kw_194,0.,0.,0.,0,"{binomial uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Binomial","Discrete Aleatory Uncertain"},
		{"continuous_design",0x19,12,2,0,1239,kw_195,0.,0.,0.,0,"{Continuous design variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCDV","Design Variables"},
		{"continuous_state",0x19,8,26,0,1461,kw_196,0.,0.,0.,0,"{Continuous state variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCSV","State Variables"},
		{"discrete_design_range",0x19,8,3,0,1253,kw_197,0.,0.,0.,0,"{Discrete design range variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDRIV","Design Variables"},
		{"discrete_design_set_integer",0x19,4,4,0,1263,kw_198,0.,0.,0.,0,"{Discrete design set of integer variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSIV","Design Variables"},
		{"discrete_design_set_real",0x19,4,5,0,1273,kw_199,0.,0.,0.,0,"{Discrete design set of real variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDDSRV","Design Variables"},
		{"discrete_state_range",0x19,8,27,0,1471,kw_200,0.,0.,0.,0,"{Discrete state range variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSRIV","State Variables"},
		{"discrete_state_set_integer",0x19,4,28,0,1481,kw_201,0.,0.,0.,0,"{Discrete state set of integer variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSIV","State Variables"},
		{"discrete_state_set_real",0x19,4,29,0,1491,kw_202,0.,0.,0.,0,"{Discrete state set of real variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDSSRV","State Variables"},
		{"exponential_uncertain",0x19,4,11,0,1339,kw_203,0.,0.,0.,0,"{exponential uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Exponential","Continuous Aleatory Uncertain"},
		{"frechet_uncertain",0x19,6,15,0,1373,kw_204,0.,0.,0.,0,"{frechet uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Frechet","Continuous Aleatory Uncertain"},
		{"gamma_uncertain",0x19,6,13,0,1357,kw_205,0.,0.,0.,0,"{gamma uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gamma","Continuous Aleatory Uncertain"},
		{"geometric_uncertain",0x19,2,21,0,1423,kw_206,0.,0.,0.,0,"{geometric uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Geometric","Discrete Aleatory Uncertain"},
		{"gumbel_uncertain",0x19,6,14,0,1365,kw_207,0.,0.,0.,0,"{gumbel uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Gumbel","Continuous Aleatory Uncertain"},
		{"histogram_bin_uncertain",0x19,10,17,0,1389,kw_208,0.,0.,0.,0,"{histogram bin uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Bin_Histogram","Continuous Aleatory Uncertain"},
		{"histogram_point_uncertain",0x19,8,23,0,1439,kw_209,0.,0.,0.,0,"{histogram point uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Point_Histogram","Discrete Aleatory Uncertain"},
		{"hypergeometric_uncertain",0x19,4,22,0,1429,kw_210,0.,0.,0.,0,"{hypergeometric uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Hypergeometric","Discrete Aleatory Uncertain"},
		{"id_variables",11,0,1,0,1237,0,0.,0.,0.,0,"{Variables set identifier} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarSetId"},
		{"interval_uncertain",0x19,8,25,0,1451,kw_211,0.,0.,0.,0,"{interval uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCEUV_Interval","Epistemic Uncertain"},
		{"lognormal_uncertain",0x19,10,7,0,1295,kw_214,0.,0.,0.,0,"{lognormal uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Lognormal","Continuous Aleatory Uncertain"},
		{"loguniform_uncertain",0x19,6,9,0,1321,kw_215,0.,0.,0.,0,"{loguniform uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Loguniform","Continuous Aleatory Uncertain"},
		{"negative_binomial_uncertain",0x19,3,20,0,1415,kw_216,0.,0.,0.,0,"{negative binomial uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Negative_Binomial","Discrete Aleatory Uncertain"},
		{"normal_uncertain",0x19,10,6,0,1283,kw_217,0.,0.,0.,0,"{normal uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Normal","Continuous Aleatory Uncertain"},
		{"poisson_uncertain",0x19,2,18,0,1401,kw_218,0.,0.,0.,0,"{poisson uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarDAUV_Poisson","Discrete Aleatory Uncertain"},
		{"triangular_uncertain",0x19,8,10,0,1329,kw_219,0.,0.,0.,0,"{triangular uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Triangular","Continuous Aleatory Uncertain"},
		{"uncertain_correlation_matrix",14,0,24,0,1449,0,0.,0.,0.,0,"{correlations in aleatory uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarAUV_Correlations","Aleatory Uncertain Correlations"},
		{"uniform_uncertain",0x19,6,8,0,1313,kw_220,0.,0.,0.,0,"{uniform uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Uniform","Continuous Aleatory Uncertain"},
		{"weibull_uncertain",0x19,6,16,0,1381,kw_221,0.,0.,0.,0,"{weibull uncertain variables} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/VarCommands.html#VarCAUV_Weibull","Continuous Aleatory Uncertain"}
		},
	kw_223[6] = {
		{"interface",0x308,10,5,5,1501,kw_9,0.,0.,0.,0,"{Interface} An interface specifies how function evaluations will be performed in order to map a set of parameters into a set of responses. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/InterfCommands.html"},
		{"method",0x308,74,2,2,61,kw_141,0.,0.,0.,0,"{Method} A method specifies the name and controls of an iterative procedure, e.g., a sensitivity analysis, uncertainty quantification, or optimization method. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/MethodCommands.html"},
		{"model",8,6,3,3,1063,kw_164,0.,0.,0.,0,"{Model} A model consists of a model type and maps specified variables through an interface to generate responses. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/ModelCommands.html"},
		{"responses",0x308,15,6,6,1595,kw_181,0.,0.,0.,0,"{Responses} A responses object specifies the data that can be returned to DAKOTA through the interface after the completion of a function evaluation. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/RespCommands.html"},
		{"strategy",0x108,10,1,1,1,kw_192,0.,0.,0.,0,"{Strategy} The strategy specifies the top level technique which will govern the management of iterators and models in the solution of the problem of interest. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/StratCommands.html"},
		{"variables",0x308,29,4,4,1235,kw_222,0.,0.,0.,0,"{Variables} A variables object specifies the parameter set to be iterated by a particular method. http://www.cs.sandia.gov/DAKOTA/licensing/votd/html-ref/VarCommands.html"}
		};

} // namespace Dakota

KeyWord Dakota_Keyword_Top = {"KeywordTop",0,6,0,0,Dakota::kw_223};

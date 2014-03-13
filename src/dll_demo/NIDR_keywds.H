
namespace Dakota {

/** 742 distinct keywords (plus 101 aliases) **/

static KeyWord
	kw_1[3] = {
		{"active_set_vector",8,0,1,0,0,0.,0.,0,N_ifm(false,activeSetVectorFlag)},
		{"evaluation_cache",8,0,2,0,0,0.,0.,0,N_ifm(false,evalCacheFlag)},
		{"restart_file",8,0,3,0,0,0.,0.,0,N_ifm(false,restartFileFlag)}
		},
	kw_2[1] = {
		{"processors_per_analysis",9,0,1,0,0,0.,0.,0,N_ifm(int,procsPerAnalysis)}
		},
	kw_3[4] = {
		{"abort",8,0,1,1,0,0.,0.,0,N_ifm(lit,failAction_abort)},
		{"continuation",8,0,1,1,0,0.,0.,0,N_ifm(lit,failAction_continuation)},
		{"recover",14,0,1,1,0,0.,0.,0,N_ifm(Rlit,3failAction_recover)},
		{"retry",9,0,1,1,0,0.,0.,0,N_ifm(ilit,3failAction_retry)}
		},
	kw_4[2] = {
		{"copy",8,0,1,0,0,0.,0.,0,N_ifm(true,templateCopy)},
		{"replace",8,0,2,0,0,0.,0.,0,N_ifm(true,templateReplace)}
		},
	kw_5[7] = {
		{"dir_save",0,0,3,0,0,0.,0.,2,N_ifm(true,dirSave)},
		{"dir_tag",0,0,2,0,0,0.,0.,2,N_ifm(true,dirTag)},
		{"directory_save",8,0,3,0,0,0.,0.,0,N_ifm(true,dirSave)},
		{"directory_tag",8,0,2,0,0,0.,0.,0,N_ifm(true,dirTag)},
		{"named",11,0,1,0,0,0.,0.,0,N_ifm(str,workDir)},
		{"template_directory",11,2,4,0,kw_4,0.,0.,0,N_ifm(str,templateDir)},
		{"template_files",15,2,4,0,kw_4,0.,0.,0,N_ifm(strL,templateFiles)}
		},
	kw_6[7] = {
		{"aprepro",8,0,4,0,0,0.,0.,0,N_ifm(true,apreproFlag)},
		{"file_save",8,0,6,0,0,0.,0.,0,N_ifm(true,fileSaveFlag)},
		{"file_tag",8,0,5,0,0,0.,0.,0,N_ifm(true,fileTagFlag)},
		{"parameters_file",11,0,1,0,0,0.,0.,0,N_ifm(str,parametersFile)},
		{"results_file",11,0,2,0,0,0.,0.,0,N_ifm(str,resultsFile)},
		{"verbatim",8,0,3,0,0,0.,0.,0,N_ifm(true,verbatimFlag)},
		{"work_directory",8,7,7,0,kw_5,0.,0.,0,N_ifm(true,useWorkdir)}
		},
	kw_7[9] = {
		{"analysis_components",15,0,1,0,0,0.,0.,0,N_ifm(str2D,analysisComponents)},
		{"deactivate",8,3,6,0,kw_1},
		{"direct",8,1,4,1,kw_2,0.,0.,0,N_ifm(lit,interfaceType_direct)},
		{"failure_capture",8,4,5,0,kw_3},
		{"fork",8,7,4,1,kw_6,0.,0.,0,N_ifm(lit,interfaceType_fork)},
		{"grid",8,0,4,1,0,0.,0.,0,N_ifm(lit,interfaceType_grid)},
		{"input_filter",11,0,2,0,0,0.,0.,0,N_ifm(str,inputFilter)},
		{"output_filter",11,0,3,0,0,0.,0.,0,N_ifm(str,outputFilter)},
		{"system",8,7,4,1,kw_6,0.,0.,0,N_ifm(lit,interfaceType_system)}
		},
	kw_8[4] = {
		{"analysis_concurrency",9,0,3,0,0,0.,0.,0,N_ifm(int,asynchLocalAnalysisConcurrency)},
		{"evaluation_concurrency",9,0,1,0,0,0.,0.,0,N_ifm(int,asynchLocalEvalConcurrency)},
		{"local_evaluation_self_scheduling",8,0,2,0,0,0.,0.,0,N_ifm(lit,asynchLocalEvalScheduling_self)},
		{"local_evaluation_static_scheduling",8,0,2,0,0,0.,0.,0,N_ifm(lit,asynchLocalEvalScheduling_static)}
		},
	kw_9[10] = {
		{"algebraic_mappings",11,0,2,0,0,0.,0.,0,N_ifm(str,algebraicMappings)},
		{"analysis_drivers",15,9,3,0,kw_7,0.,0.,0,N_ifm(strL,analysisDrivers)},
		{"analysis_self_scheduling",8,0,8,0,0,0.,0.,0,N_ifm(lit,analysisScheduling_self)},
		{"analysis_servers",9,0,7,0,0,0.,0.,0,N_ifm(int,analysisServers)},
		{"analysis_static_scheduling",8,0,8,0,0,0.,0.,0,N_ifm(lit,analysisScheduling_static)},
		{"asynchronous",8,4,4,0,kw_8,0.,0.,0,N_ifm(lit,interfaceSynchronization_asynchronous)},
		{"evaluation_self_scheduling",8,0,6,0,0,0.,0.,0,N_ifm(lit,evalScheduling_self)},
		{"evaluation_servers",9,0,5,0,0,0.,0.,0,N_ifm(int,evalServers)},
		{"evaluation_static_scheduling",8,0,6,0,0,0.,0.,0,N_ifm(lit,evalScheduling_static)},
		{"id_interface",11,0,1,0,0,0.,0.,0,N_ifm(str,idInterface)}
		},
	kw_10[9] = {
		{"linear_equality_constraint_matrix",14,0,6,0,0,0.,0.,0,N_mdm(RealDL,linearEqConstraintCoeffs)},
		{"linear_equality_scale_types",15,0,8,0,0,0.,0.,0,N_mdm(strL,linearEqScaleTypes)},
		{"linear_equality_scales",14,0,9,0,0,0.,0.,0,N_mdm(RealDL,linearEqScales)},
		{"linear_equality_targets",14,0,7,0,0,0.,0.,0,N_mdm(RealDL,linearEqTargets)},
		{"linear_inequality_constraint_matrix",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,linearIneqConstraintCoeffs)},
		{"linear_inequality_lower_bounds",14,0,2,0,0,0.,0.,0,N_mdm(RealDL,linearIneqLowerBnds)},
		{"linear_inequality_scale_types",15,0,4,0,0,0.,0.,0,N_mdm(strL,linearIneqScaleTypes)},
		{"linear_inequality_scales",14,0,5,0,0,0.,0.,0,N_mdm(RealDL,linearIneqScales)},
		{"linear_inequality_upper_bounds",14,0,3,0,0,0.,0.,0,N_mdm(RealDL,linearIneqUpperBnds)}
		},
	kw_11[7] = {
		{"merit1",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit1)},
		{"merit1_smooth",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit1_smooth)},
		{"merit2",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit2)},
		{"merit2_smooth",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit2_smooth)},
		{"merit2_squared",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit2_squared)},
		{"merit_max",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit_max)},
		{"merit_max_smooth",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit_max_smooth)}
		},
	kw_12[2] = {
		{"blocking",8,0,1,1,0,0.,0.,0,N_mdm(lit,evalSynchronize_blocking)},
		{"nonblocking",8,0,1,1,0,0.,0.,0,N_mdm(lit,evalSynchronize_nonblocking)}
		},
	kw_13[10] = {
		{0,0,9,0,0,kw_10},
		{"constraint_penalty",10,0,7,0,0,0.,0.,0,N_mdm(Real,constrPenalty)},
		{"contraction_factor",10,0,2,0,0,0.,0.,0,N_mdm(Real,contractStepLength)},
		{"initial_delta",10,0,1,0,0,0.,0.,0,N_mdm(Real,initStepLength)},
		{"merit_function",8,7,6,0,kw_11},
		{"smoothing_factor",10,0,8,0,0,0.,0.,0,N_mdm(Real,smoothFactor)},
		{"solution_accuracy",2,0,4,0,0,0.,0.,1,N_mdm(Real,solnTarget)},
		{"solution_target",10,0,4,0,0,0.,0.,0,N_mdm(Real,solnTarget)},
		{"synchronization",8,2,5,0,kw_12},
		{"threshold_delta",10,0,3,0,0,0.,0.,0,N_mdm(Real,threshStepLength)}
		},
	kw_14[3] = {
		{"deltas_per_variable",5,0,2,2,0,0.,0.,2,N_mdm(intDL,stepsPerVariable)},
		{"step_vector",14,0,1,1,0,0.,0.,0,N_mdm(RealDL,stepVector)},
		{"steps_per_variable",13,0,2,2,0,0.,0.,0,N_mdm(intDL,stepsPerVariable)}
		},
	kw_15[5] = {
		{"misc_options",15,0,4,0,0,0.,0.,0,N_mdm(strL,miscOptions)},
		{"seed",9,0,2,0,0,0.,0.,0,N_mdm(pint,randomSeed)},
		{"show_misc_options",8,0,3,0,0,0.,0.,0,N_mdm(true,showMiscOptions)},
		{"solution_accuracy",2,0,1,0,0,0.,0.,1,N_mdm(Real,solnTarget)},
		{"solution_target",10,0,1,0,0,0.,0.,0,N_mdm(Real,solnTarget)}
		},
	kw_16[2] = {
		{"initial_delta",10,0,1,1,0,0.,0.,0,N_mdm(Real,initDelta)},
		{"threshold_delta",10,0,2,2,0,0.,0.,0,N_mdm(Real,threshDelta)}
		},
	kw_17[3] = {
		{0,0,5,0,0,kw_15},
		{0,0,2,0,0,kw_16},
		{""}
		},
	kw_18[2] = {
		{"all_dimensions",8,0,1,1,0,0.,0.,0,N_mdm(lit,boxDivision_all_dimensions)},
		{"major_dimension",8,0,1,1,0,0.,0.,0,N_mdm(lit,boxDivision_major_dimension)}
		},
	kw_19[7] = {
		{0,0,5,0,0,kw_15},
		{"constraint_penalty",10,0,6,0,0,0.,0.,0,N_mdm(Real,constraintPenalty)},
		{"division",8,2,1,0,kw_18},
		{"global_balance_parameter",10,0,2,0,0,0.,0.,0,N_mdm(Real,globalBalanceParam)},
		{"local_balance_parameter",10,0,3,0,0,0.,0.,0,N_mdm(Real,localBalanceParam)},
		{"max_boxsize_limit",10,0,4,0,0,0.,0.,0,N_mdm(Real,maxBoxSize)},
		{"min_boxsize_limit",10,0,5,0,0,0.,0.,0,N_mdm(Real,minBoxSize)}
		},
	kw_20[3] = {
		{"blend",8,0,1,1,0,0.,0.,0,N_mdm(lit,crossoverType_blend)},
		{"two_point",8,0,1,1,0,0.,0.,0,N_mdm(lit,crossoverType_two_point)},
		{"uniform",8,0,1,1,0,0.,0.,0,N_mdm(lit,crossoverType_uniform)}
		},
	kw_21[2] = {
		{"linear_rank",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_linear_rank)},
		{"merit_function",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_proportional)}
		},
	kw_22[3] = {
		{"flat_file",11,0,1,1,0,0.,0.,0,N_mdm(slit2,3initializationType_flat_file)},
		{"simple_random",8,0,1,1,0,0.,0.,0,N_mdm(lit,initializationType_random)},
		{"unique_random",8,0,1,1,0,0.,0.,0,N_mdm(lit,initializationType_unique_random)}
		},
	kw_23[2] = {
		{"mutation_range",9,0,2,0,0,0.,0.,0,N_mdm(int,mutationRange)},
		{"mutation_scale",10,0,1,0,0,0.,0.,0,N_mdm(Real,mutationScale)}
		},
	kw_24[5] = {
		{"non_adaptive",8,0,2,0,0,0.,0.,0,N_mdm(false,mutationAdaptive)},
		{"offset_cauchy",8,2,1,1,kw_23,0.,0.,0,N_mdm(lit,mutationType_offset_cauchy)},
		{"offset_normal",8,2,1,1,kw_23,0.,0.,0,N_mdm(lit,mutationType_offset_normal)},
		{"offset_uniform",8,2,1,1,kw_23,0.,0.,0,N_mdm(lit,mutationType_offset_uniform)},
		{"replace_uniform",8,0,1,1,0,0.,0.,0,N_mdm(lit,mutationType_replace_uniform)}
		},
	kw_25[4] = {
		{"chc",9,0,1,1,0,0.,0.,0,N_mdm(ilit2,3replacementType_chc)},
		{"elitist",9,0,1,1,0,0.,0.,0,N_mdm(ilit2,3replacementType_elitist)},
		{"new_solutions_generated",9,0,2,0,0,0.,0.,0,N_mdm(int,newSolnsGenerated)},
		{"random",9,0,1,1,0,0.,0.,0,N_mdm(ilit2,3replacementType_random)}
		},
	kw_26[10] = {
		{0,0,5,0,0,kw_15},
		{"constraint_penalty",10,0,9,0,0,0.,0.,0,N_mdm(Real,constraintPenalty)},
		{"crossover_rate",10,0,5,0,0,0.,0.,0,N_mdm(Real,crossoverRate)},
		{"crossover_type",8,3,6,0,kw_20},
		{"fitness_type",8,2,3,0,kw_21},
		{"initialization_type",8,3,2,0,kw_22},
		{"mutation_rate",10,0,7,0,0,0.,0.,0,N_mdm(Real,mutationRate)},
		{"mutation_type",8,5,8,0,kw_24},
		{"population_size",9,0,1,0,0,0.,0.,0,N_mdm(pint,populationSize)},
		{"replacement_type",8,4,4,0,kw_25}
		},
	kw_27[2] = {
		{"constraint_penalty",10,0,2,0,0,0.,0.,0,N_mdm(Real,constraintPenalty)},
		{"contraction_factor",10,0,1,0,0,0.,0.,0,N_mdm(Real,contractFactor)}
		},
	kw_28[3] = {
		{"adaptive_pattern",8,0,1,1,0,0.,0.,0,N_mdm(lit,exploratoryMoves_adaptive)},
		{"basic_pattern",8,0,1,1,0,0.,0.,0,N_mdm(lit,exploratoryMoves_simple)},
		{"multi_step",8,0,1,1,0,0.,0.,0,N_mdm(lit,exploratoryMoves_multi_step)}
		},
	kw_29[2] = {
		{"coordinate",8,0,1,1,0,0.,0.,0,N_mdm(lit,patternBasis_coordinate)},
		{"simplex",8,0,1,1,0,0.,0.,0,N_mdm(lit,patternBasis_simplex)}
		},
	kw_30[2] = {
		{"blocking",8,0,1,1,0,0.,0.,0,N_mdm(lit,evalSynchronization_blocking)},
		{"nonblocking",8,0,1,1,0,0.,0.,0,N_mdm(lit,evalSynchronization_nonblocking)}
		},
	kw_31[11] = {
		{0,0,5,0,0,kw_15},
		{0,0,2,0,0,kw_16},
		{0,0,2,0,0,kw_27},
		{"constant_penalty",8,0,1,0,0,0.,0.,0,N_mdm(true,constantPenalty)},
		{"expand_after_success",9,0,3,0,0,0.,0.,0,N_mdm(int,expandAfterSuccess)},
		{"exploratory_moves",8,3,7,0,kw_28},
		{"no_expansion",8,0,2,0,0,0.,0.,0,N_mdm(false,expansionFlag)},
		{"pattern_basis",8,2,4,0,kw_29},
		{"stochastic",8,0,5,0,0,0.,0.,0,N_mdm(true,randomizeOrderFlag)},
		{"synchronization",8,2,8,0,kw_30},
		{"total_pattern_size",9,0,6,0,0,0.,0.,0,N_mdm(int,totalPatternSize)}
		},
	kw_32[7] = {
		{0,0,5,0,0,kw_15},
		{0,0,2,0,0,kw_16},
		{0,0,2,0,0,kw_27},
		{"constant_penalty",8,0,4,0,0,0.,0.,0,N_mdm(true,constantPenalty)},
		{"contract_after_failure",9,0,1,0,0,0.,0.,0,N_mdm(int,contractAfterFail)},
		{"expand_after_success",9,0,3,0,0,0.,0.,0,N_mdm(int,expandAfterSuccess)},
		{"no_expansion",8,0,2,0,0,0.,0.,0,N_mdm(false,expansionFlag)}
		},
	kw_33[1] = {
		{"seed",9,0,1,0,0,0.,0.,0,N_mdm(pint,randomSeed)}
		},
	kw_34[1] = {
		{"samples",9,0,1,0,0,0.,0.,0,N_mdm(int,numSamples)}
		},
	kw_35[14] = {
		{0,0,1,0,0,kw_33},
		{0,0,1,0,0,kw_34},
		{"box_behnken",8,0,1,1,0,0.,0.,0,N_mdm(lit,daceMethod_box_behnken)},
		{"central_composite",8,0,1,1,0,0.,0.,0,N_mdm(lit,daceMethod_central_composite)},
		{"fixed_seed",8,0,5,0,0,0.,0.,0,N_mdm(true,fixedSeedFlag)},
		{"grid",8,0,1,1,0,0.,0.,0,N_mdm(lit,daceMethod_grid)},
		{"lhs",8,0,1,1,0,0.,0.,0,N_mdm(lit,daceMethod_lhs)},
		{"main_effects",8,0,2,0,0,0.,0.,0,N_mdm(true,mainEffectsFlag)},
		{"oa_lhs",8,0,1,1,0,0.,0.,0,N_mdm(lit,daceMethod_oa_lhs)},
		{"oas",8,0,1,1,0,0.,0.,0,N_mdm(lit,daceMethod_oas)},
		{"quality_metrics",8,0,3,0,0,0.,0.,0,N_mdm(true,volQualityFlag)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(lit,daceMethod_random)},
		{"symbols",9,0,6,0,0,0.,0.,0,N_mdm(int,numSymbols)},
		{"variance_based_decomp",8,0,4,0,0,0.,0.,0,N_mdm(true,varBasedDecompFlag)}
		},
	kw_36[3] = {
		{"grid",8,0,1,1,0,0.,0.,0,N_mdm(lit,trialType_grid)},
		{"halton",8,0,1,1,0,0.,0.,0,N_mdm(lit,trialType_halton)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(lit,trialType_random)}
		},
	kw_37[8] = {
		{0,0,1,0,0,kw_33},
		{0,0,1,0,0,kw_34},
		{"fixed_seed",8,0,4,0,0,0.,0.,0,N_mdm(true,fixedSeedFlag)},
		{"latinize",8,0,1,0,0,0.,0.,0,N_mdm(true,latinizeFlag)},
		{"num_trials",9,0,6,0,0,0.,0.,0,N_mdm(int,numTrials)},
		{"quality_metrics",8,0,2,0,0,0.,0.,0,N_mdm(true,volQualityFlag)},
		{"trial_type",8,3,5,0,kw_36},
		{"variance_based_decomp",8,0,3,0,0,0.,0.,0,N_mdm(true,varBasedDecompFlag)}
		},
	kw_38[10] = {
		{"fixed_sequence",8,0,6,0,0,0.,0.,0,N_mdm(true,fixedSequenceFlag)},
		{"halton",8,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_fsu_halton)},
		{"hammersley",8,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_fsu_hammersley)},
		{"latinize",8,0,2,0,0,0.,0.,0,N_mdm(true,latinizeFlag)},
		{"prime_base",13,0,9,0,0,0.,0.,0,N_mdm(intDL,primeBase)},
		{"quality_metrics",8,0,3,0,0,0.,0.,0,N_mdm(true,volQualityFlag)},
		{"samples",9,0,5,0,0,0.,0.,0,N_mdm(int,numSamples)},
		{"sequence_leap",13,0,8,0,0,0.,0.,0,N_mdm(intDL,sequenceLeap)},
		{"sequence_start",13,0,7,0,0,0.,0.,0,N_mdm(intDL,sequenceStart)},
		{"variance_based_decomp",8,0,4,0,0,0.,0.,0,N_mdm(true,varBasedDecompFlag)}
		},
	kw_39[1] = {
		{"list_of_points",14,0,1,1,0,0.,0.,0,N_mdm(RealDL,listOfPoints)}
		},
	kw_40[1] = {
		{"partitions",13,0,1,1,0,0.,0.,0,N_mdm(ushintL,varPartitions)}
		},
	kw_41[4] = {
		{"min_boxsize_limit",10,0,2,0,0,0.,0.,0,N_mdm(Real,minBoxSize)},
		{"solution_accuracy",2,0,1,0,0,0.,0.,1,N_mdm(Real,solnTarget)},
		{"solution_target",10,0,1,0,0,0.,0.,0,N_mdm(Real,solnTarget)},
		{"volume_boxsize_limit",10,0,3,0,0,0.,0.,0,N_mdm(Real,volBoxSize)}
		},
	kw_42[9] = {
		{"absolute_conv_tol",10,0,2,0,0,0.,0.,0,N_mdm(Real,absConvTol)},
		{"covariance",9,0,8,0,0,0.,0.,0,N_mdm(int,covarianceType)},
		{"false_conv_tol",10,0,6,0,0,0.,0.,0,N_mdm(Real,falseConvTol)},
		{"function_precision",10,0,1,0,0,0.,0.,0,N_mdm(Real,functionPrecision)},
		{"initial_trust_radius",10,0,7,0,0,0.,0.,0,N_mdm(Real,initTRRadius)},
		{"regression_diagnostics",8,0,9,0,0,0.,0.,0,N_mdm(true,regressDiag)},
		{"singular_conv_tol",10,0,4,0,0,0.,0.,0,N_mdm(Real,singConvTol)},
		{"singular_radius",10,0,5,0,0,0.,0.,0,N_mdm(Real,singRadius)},
		{"x_conv_tol",10,0,3,0,0,0.,0.,0,N_mdm(Real,xConvTol)}
		},
	kw_43[2] = {
		{"mt19937",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_mt19937)},
		{"rnum2",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_rnum2)}
		},
	kw_44[3] = {
		{0,0,1,0,0,kw_33},
		{0,0,1,0,0,kw_34},
		{"rng",8,2,1,0,kw_43}
		},
	kw_45[2] = {
		{"complementary",8,0,1,1,0,0.,0.,0,N_mdm(lit,distributionType_complementary)},
		{"cumulative",8,0,1,1,0,0.,0.,0,N_mdm(lit,distributionType_cumulative)}
		},
	kw_46[1] = {
		{"num_gen_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,genReliabilityLevels)}
		},
	kw_47[1] = {
		{"num_probability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,probabilityLevels)}
		},
	kw_48[2] = {
		{"mt19937",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_mt19937)},
		{"rnum2",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_rnum2)}
		},
	kw_49[4] = {
		{"distribution",8,2,1,0,kw_45},
		{"gen_reliability_levels",14,1,3,0,kw_46,0.,0.,0,N_mdm(resplevs,genReliabilityLevels)},
		{"probability_levels",14,1,2,0,kw_47,0.,0.,0,N_mdm(resplevs01,probabilityLevels)},
		{"rng",8,2,4,0,kw_48}
		},
	kw_50[2] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(lit,responseLevelMappingType_gen_reliabilities)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(lit,responseLevelMappingType_probabilities)}
		},
	kw_51[2] = {
		{"compute",8,2,2,0,kw_50},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_52[6] = {
		{0,0,1,0,0,kw_33},
		{0,0,1,0,0,kw_34},
		{0,0,4,0,0,kw_49},
		{"ego",8,0,1,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_ego)},
		{"lhs",8,0,1,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_lhs)},
		{"response_levels",14,2,2,0,kw_51,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_53[2] = {
		{"mt19937",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_mt19937)},
		{"rnum2",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_rnum2)}
		},
	kw_54[5] = {
		{0,0,1,0,0,kw_33},
		{0,0,1,0,0,kw_34},
		{"ego",8,0,1,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_ego)},
		{"lhs",8,0,1,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_lhs)},
		{"rng",8,2,2,0,kw_53}
		},
	kw_55[2] = {
		{"complementary",8,0,1,1,0,0.,0.,0,N_mdm(lit,distributionType_complementary)},
		{"cumulative",8,0,1,1,0,0.,0.,0,N_mdm(lit,distributionType_cumulative)}
		},
	kw_56[1] = {
		{"num_gen_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,genReliabilityLevels)}
		},
	kw_57[1] = {
		{"num_probability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,probabilityLevels)}
		},
	kw_58[3] = {
		{"distribution",8,2,1,0,kw_55},
		{"gen_reliability_levels",14,1,3,0,kw_56,0.,0.,0,N_mdm(resplevs,genReliabilityLevels)},
		{"probability_levels",14,1,2,0,kw_57,0.,0.,0,N_mdm(resplevs01,probabilityLevels)}
		},
	kw_59[2] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(lit,responseLevelMappingType_gen_reliabilities)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(lit,responseLevelMappingType_probabilities)}
		},
	kw_60[2] = {
		{"compute",8,2,2,0,kw_59},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_61[2] = {
		{"mt19937",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_mt19937)},
		{"rnum2",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_rnum2)}
		},
	kw_62[7] = {
		{0,0,3,0,0,kw_58},
		{"all_variables",8,0,2,0,0,0.,0.,0,N_mdm(true,allVarsFlag)},
		{"response_levels",14,2,5,0,kw_60,0.,0.,0,N_mdm(resplevs,responseLevels)},
		{"rng",8,2,4,0,kw_61},
		{"seed",9,0,3,0,0,0.,0.,0,N_mdm(pint,randomSeed)},
		{"u_gaussian_process",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilitySearchType_egra_u)},
		{"x_gaussian_process",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilitySearchType_egra_x)}
		},
	kw_63[2] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(lit,responseLevelMappingType_gen_reliabilities)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(lit,responseLevelMappingType_probabilities)}
		},
	kw_64[2] = {
		{"compute",8,2,2,0,kw_63},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_65[4] = {
		{0,0,1,0,0,kw_33},
		{0,0,1,0,0,kw_34},
		{0,0,4,0,0,kw_49},
		{"response_levels",14,2,1,0,kw_64,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_66[2] = {
		{"complementary",8,0,1,1,0,0.,0.,0,N_mdm(lit,distributionType_complementary)},
		{"cumulative",8,0,1,1,0,0.,0.,0,N_mdm(lit,distributionType_cumulative)}
		},
	kw_67[1] = {
		{"num_gen_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,genReliabilityLevels)}
		},
	kw_68[1] = {
		{"num_probability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,probabilityLevels)}
		},
	kw_69[2] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(lit,responseLevelMappingType_gen_reliabilities)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(lit,responseLevelMappingType_probabilities)}
		},
	kw_70[2] = {
		{"compute",8,2,2,0,kw_69},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_71[6] = {
		{"distribution",8,2,5,0,kw_66},
		{"gen_reliability_levels",14,1,4,0,kw_67,0.,0.,0,N_mdm(resplevs,genReliabilityLevels)},
		{"nip",8,0,1,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_nip)},
		{"probability_levels",14,1,3,0,kw_68,0.,0.,0,N_mdm(resplevs01,probabilityLevels)},
		{"response_levels",14,2,2,0,kw_70,0.,0.,0,N_mdm(resplevs,responseLevels)},
		{"sqp",8,0,1,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_sqp)}
		},
	kw_72[2] = {
		{"nip",8,0,1,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_nip)},
		{"sqp",8,0,1,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_sqp)}
		},
	kw_73[5] = {
		{"adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilityIntegrationRefine_ais)},
		{"import",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilityIntegrationRefine_is)},
		{"mm_adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilityIntegrationRefine_mmais)},
		{"samples",9,0,2,0,0,0.,0.,0,N_mdm(int,numSamples)},
		{"seed",9,0,3,0,0,0.,0.,0,N_mdm(pint,randomSeed)}
		},
	kw_74[3] = {
		{"first_order",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilityIntegration_first_order)},
		{"refinement",8,5,2,0,kw_73},
		{"second_order",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilityIntegration_second_order)}
		},
	kw_75[9] = {
		{"nip",8,0,2,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_nip)},
		{"no_approx",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilitySearchType_no_approx)},
		{"sqp",8,0,2,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_sqp)},
		{"u_taylor_mean",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilitySearchType_amv_u)},
		{"u_taylor_mpp",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilitySearchType_amv_plus_u)},
		{"u_two_point",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilitySearchType_tana_u)},
		{"x_taylor_mean",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilitySearchType_amv_x)},
		{"x_taylor_mpp",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilitySearchType_amv_plus_x)},
		{"x_two_point",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilitySearchType_tana_x)}
		},
	kw_76[1] = {
		{"num_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,reliabilityLevels)}
		},
	kw_77[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(lit,responseLevelMappingType_gen_reliabilities)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(lit,responseLevelMappingType_probabilities)},
		{"reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(lit,responseLevelMappingType_reliabilities)}
		},
	kw_78[2] = {
		{"compute",8,3,2,0,kw_77},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_79[5] = {
		{0,0,3,0,0,kw_58},
		{"integration",8,3,2,0,kw_74},
		{"mpp_search",8,9,1,0,kw_75},
		{"reliability_levels",14,1,4,0,kw_76,0.,0.,0,N_mdm(resplevs,reliabilityLevels)},
		{"response_levels",14,2,3,0,kw_78,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_80[1] = {
		{"num_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,reliabilityLevels)}
		},
	kw_81[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(lit,responseLevelMappingType_gen_reliabilities)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(lit,responseLevelMappingType_probabilities)},
		{"reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(lit,responseLevelMappingType_reliabilities)}
		},
	kw_82[2] = {
		{"compute",8,3,2,0,kw_81},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_83[2] = {
		{"reliability_levels",14,1,1,0,kw_80,0.,0.,0,N_mdm(resplevs,reliabilityLevels)},
		{"response_levels",14,2,2,0,kw_82,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_84[2] = {
		{"all_variables",8,0,1,0,0,0.,0.,0,N_mdm(true,allVarsFlag)},
		{"fixed_seed",8,0,2,0,0,0.,0.,0,N_mdm(true,fixedSeedFlag)}
		},
	kw_85[2] = {
		{"expansion_order",13,0,1,1,0,0.,0.,0,N_mdm(ushintL,expansionOrder)},
		{"expansion_terms",9,0,1,1,0,0.,0.,0,N_mdm(int,expansionTerms)}
		},
	kw_86[2] = {
		{0,0,2,0,0,kw_85},
		{"reuse_samples",8,0,1,0,0,0.,0.,0,N_mdm(lit,collocSampleReuse_all)}
		},
	kw_87[2] = {
		{0,0,2,0,0,kw_85},
		{"incremental_lhs",8,0,1,0,0,0.,0.,0,N_mdm(lit,expansionSampleType_incremental_lhs)}
		},
	kw_88[2] = {
		{"adaptive",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementType_ADAPTIVE_REFINEMENT)},
		{"uniform",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementType_UNIFORM_REFINEMENT)}
		},
	kw_89[2] = {
		{"lhs",8,0,1,1,0,0.,0.,0,N_mdm(lit,sampleType_lhs)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(lit,sampleType_random)}
		},
	kw_90[1] = {
		{"dimension_preference",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,sparseGridDimPref)}
		},
	kw_91[16] = {
		{0,0,1,0,0,kw_33},
		{0,0,1,0,0,kw_34},
		{0,0,4,0,0,kw_49},
		{0,0,2,0,0,kw_83},
		{0,0,2,0,0,kw_84},
		{"askey",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionType_ASKEY_U)},
		{"collocation_points",9,1,3,1,kw_86,0.,0.,0,N_mdm(int,collocationPoints)},
		{"collocation_ratio",10,1,3,1,kw_86,0.,0.,0,N_mdm(Realp,collocationRatio)},
		{"cubature_integrand",9,0,3,1,0,0.,0.,0,N_mdm(ushint,cubIntOrder)},
		{"expansion_import_file",11,2,3,1,kw_85,0.,0.,0,N_mdm(str,expansionImportFile)},
		{"expansion_samples",9,1,3,1,kw_87,0.,0.,0,N_mdm(int,expansionSamples)},
		{"quadrature_order",13,0,3,1,0,0.,0.,0,N_mdm(ushintL,quadratureOrder)},
		{"refinement",8,2,1,0,kw_88},
		{"sample_type",8,2,4,0,kw_89},
		{"sparse_grid_level",9,1,3,1,kw_90,0.,0.,0,N_mdm(ushint,sparseGridLevel)},
		{"wiener",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionType_STD_NORMAL_U)}
		},
	kw_92[1] = {
		{"previous_samples",9,0,1,1,0,0.,0.,0,N_mdm(int,previousSamples)}
		},
	kw_93[4] = {
		{"incremental_lhs",8,1,1,1,kw_92,0.,0.,0,N_mdm(lit,sampleType_incremental_lhs)},
		{"incremental_random",8,1,1,1,kw_92,0.,0.,0,N_mdm(lit,sampleType_incremental_random)},
		{"lhs",8,0,1,1,0,0.,0.,0,N_mdm(lit,sampleType_lhs)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(lit,sampleType_random)}
		},
	kw_94[7] = {
		{0,0,1,0,0,kw_33},
		{0,0,1,0,0,kw_34},
		{0,0,4,0,0,kw_49},
		{0,0,2,0,0,kw_83},
		{0,0,2,0,0,kw_84},
		{"sample_type",8,4,1,0,kw_93},
		{"variance_based_decomp",8,0,2,0,0,0.,0.,0,N_mdm(true,varBasedDecompFlag)}
		},
	kw_95[2] = {
		{"adaptive",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementType_ADAPTIVE_REFINEMENT)},
		{"uniform",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementType_UNIFORM_REFINEMENT)}
		},
	kw_96[2] = {
		{"lhs",8,0,1,1,0,0.,0.,0,N_mdm(lit,sampleType_lhs)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(lit,sampleType_random)}
		},
	kw_97[1] = {
		{"dimension_preference",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,sparseGridDimPref)}
		},
	kw_98[11] = {
		{0,0,1,0,0,kw_33},
		{0,0,1,0,0,kw_34},
		{0,0,4,0,0,kw_49},
		{0,0,2,0,0,kw_83},
		{0,0,2,0,0,kw_84},
		{"askey",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionType_ASKEY_U)},
		{"quadrature_order",13,0,3,1,0,0.,0.,0,N_mdm(ushintL,quadratureOrder)},
		{"refinement",8,2,1,0,kw_95},
		{"sample_type",8,2,4,0,kw_96},
		{"sparse_grid_level",9,1,3,1,kw_97,0.,0.,0,N_mdm(ushint,sparseGridLevel)},
		{"wiener",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionType_STD_NORMAL_U)}
		},
	kw_99[1] = {
		{"misc_options",15,0,1,0,0,0.,0.,0,N_mdm(strL,miscOptions)}
		},
	kw_100[2] = {
		{"gradient_tolerance",10,0,2,0,0,0.,0.,0,N_mdm(Real,gradientTolerance)},
		{"max_step",10,0,1,0,0,0.,0.,0,N_mdm(Real,maxStep)}
		},
	kw_101[3] = {
		{0,0,9,0,0,kw_10},
		{0,0,2,0,0,kw_100},
		{""}
		},
	kw_102[2] = {
		{0,0,9,0,0,kw_10},
		{"search_scheme_size",9,0,1,0,0,0.,0.,0,N_mdm(int,searchSchemeSize)}
		},
	kw_103[4] = {
		{"gradient_based_line_search",8,0,1,1,0,0.,0.,0,N_mdm(lit,searchMethod_gradient_based_line_search)},
		{"tr_pds",8,0,1,1,0,0.,0.,0,N_mdm(lit,searchMethod_tr_pds)},
		{"trust_region",8,0,1,1,0,0.,0.,0,N_mdm(lit,searchMethod_trust_region)},
		{"value_based_line_search",8,0,1,1,0,0.,0.,0,N_mdm(lit,searchMethod_value_based_line_search)}
		},
	kw_104[7] = {
		{0,0,9,0,0,kw_10},
		{0,0,2,0,0,kw_100},
		{"centering_parameter",10,0,5,0,0,0.,0.,0,N_mdm(Real,centeringParam)},
		{"central_path",11,0,3,0,0,0.,0.,0,N_mdm(meritFn,centralPath)},
		{"merit_function",11,0,2,0,0,0.,0.,0,N_mdm(meritFn,meritFn)},
		{"search_method",8,4,1,0,kw_103},
		{"steplength_to_boundary",10,0,4,0,0,0.,0.,0,N_mdm(Real,stepLenToBoundary)}
		},
	kw_105[4] = {
		{"debug",8,0,1,1,0,0.,0.,0,N_mdm(slit,methodOutput_DEBUG_OUTPUT)},
		{"quiet",8,0,1,1,0,0.,0.,0,N_mdm(slit,methodOutput_QUIET_OUTPUT)},
		{"silent",8,0,1,1,0,0.,0.,0,N_mdm(slit,methodOutput_SILENT_OUTPUT)},
		{"verbose",8,0,1,1,0,0.,0.,0,N_mdm(slit,methodOutput_VERBOSE_OUTPUT)}
		},
	kw_106[3] = {
		{0,0,1,0,0,kw_33},
		{0,0,1,0,0,kw_34},
		{"partitions",13,0,1,0,0,0.,0.,0,N_mdm(ushintL,varPartitions)}
		},
	kw_107[3] = {
		{"approx_method_name",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodName)},
		{"approx_method_pointer",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodPointer)},
		{"replace_points",8,0,2,0,0,0.,0.,0,N_mdm(true,surrBasedGlobalReplacePts)}
		},
	kw_108[2] = {
		{"filter",8,0,1,1,0,0.,0.,0,N_mdm(slit,surrBasedLocalAcceptLogic_FILTER)},
		{"tr_ratio",8,0,1,1,0,0.,0.,0,N_mdm(slit,surrBasedLocalAcceptLogic_TR_RATIO)}
		},
	kw_109[7] = {
		{"augmented_lagrangian_objective",8,0,1,1,0,0.,0.,0,N_mdm(slit,surrBasedLocalSubProbObj_AUGMENTED_LAGRANGIAN_OBJECTIVE)},
		{"lagrangian_objective",8,0,1,1,0,0.,0.,0,N_mdm(slit,surrBasedLocalSubProbObj_LAGRANGIAN_OBJECTIVE)},
		{"linearized_constraints",8,0,2,2,0,0.,0.,0,N_mdm(slit,surrBasedLocalSubProbCon_LINEARIZED_CONSTRAINTS)},
		{"no_constraints",8,0,2,2,0,0.,0.,0,N_mdm(slit,surrBasedLocalSubProbCon_NO_CONSTRAINTS)},
		{"original_constraints",8,0,2,2,0,0.,0.,0,N_mdm(slit,surrBasedLocalSubProbCon_ORIGINAL_CONSTRAINTS)},
		{"original_primary",8,0,1,1,0,0.,0.,0,N_mdm(slit,surrBasedLocalSubProbObj_ORIGINAL_PRIMARY)},
		{"single_objective",8,0,1,1,0,0.,0.,0,N_mdm(slit,surrBasedLocalSubProbObj_SINGLE_OBJECTIVE)}
		},
	kw_110[1] = {
		{"homotopy",8,0,1,1,0,0.,0.,0,N_mdm(slit,surrBasedLocalConstrRelax_HOMOTOPY)}
		},
	kw_111[4] = {
		{"adaptive_penalty_merit",8,0,1,1,0,0.,0.,0,N_mdm(slit,surrBasedLocalMeritFn_ADAPTIVE_PENALTY_MERIT)},
		{"augmented_lagrangian_merit",8,0,1,1,0,0.,0.,0,N_mdm(slit,surrBasedLocalMeritFn_AUGMENTED_LAGRANGIAN_MERIT)},
		{"lagrangian_merit",8,0,1,1,0,0.,0.,0,N_mdm(slit,surrBasedLocalMeritFn_LAGRANGIAN_MERIT)},
		{"penalty_merit",8,0,1,1,0,0.,0.,0,N_mdm(slit,surrBasedLocalMeritFn_PENALTY_MERIT)}
		},
	kw_112[6] = {
		{"contract_threshold",10,0,3,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRContractTrigger)},
		{"contraction_factor",10,0,5,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRContract)},
		{"expand_threshold",10,0,4,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRExpandTrigger)},
		{"expansion_factor",10,0,6,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRExpand)},
		{"initial_size",10,0,1,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRInitSize)},
		{"minimum_size",10,0,2,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRMinSize)}
		},
	kw_113[10] = {
		{0,0,9,0,0,kw_10},
		{"acceptance_logic",8,2,7,0,kw_108},
		{"approx_method_name",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodName)},
		{"approx_method_pointer",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodPointer)},
		{"approx_subproblem",8,7,5,0,kw_109},
		{"constraint_relax",8,1,8,0,kw_110},
		{"merit_function",8,4,6,0,kw_111},
		{"soft_convergence_limit",9,0,2,0,0,0.,0.,0,N_mdm(int,surrBasedLocalSoftConvLimit)},
		{"trust_region",8,6,4,0,kw_112,0.,0.,0,0,0,NIDRProblemDescDB::method_tr_final},
		{"truth_surrogate_bypass",8,0,3,0,0,0.,0.,0,N_mdm(true,surrBasedLocalLayerBypass)}
		},
	kw_114[3] = {
		{"final_point",14,0,1,1,0,0.,0.,0,N_mdm(RealDL,finalPoint)},
		{"num_steps",9,0,2,2,0,0.,0.,0,N_mdm(int,numSteps)},
		{"step_vector",14,0,1,1,0,0.,0.,0,N_mdm(RealDL,stepVector)}
		},
	kw_115[65] = {
		{"asynch_pattern_search",8,9,10,1,kw_13,0.,0.,0,N_mdm(lit,methodName_asynch_pattern_search)},
		{"centered_parameter_study",8,3,10,1,kw_14,0.,0.,0,N_mdm(lit,methodName_centered_parameter_study)},
		{"coliny_apps",0,9,10,1,kw_13,0.,0.,-2,N_mdm(lit,methodName_asynch_pattern_search)},
		{"coliny_cobyla",8,0,10,1,kw_17,0.,0.,0,N_mdm(lit,methodName_coliny_cobyla)},
		{"coliny_direct",8,6,10,1,kw_19,0.,0.,0,N_mdm(lit,methodName_coliny_direct)},
		{"coliny_ea",8,9,10,1,kw_26,0.,0.,0,NIDRProblemDescDB::method_coliny_ea},
		{"coliny_pattern_search",8,8,10,1,kw_31,0.,0.,0,N_mdm(lit,methodName_coliny_pattern_search)},
		{"coliny_solis_wets",8,4,10,1,kw_32,0.,0.,0,N_mdm(lit,methodName_coliny_solis_wets)},
		{"conmin_frcg",8,9,10,1,kw_10,0.,0.,0,N_mdm(lit,methodName_conmin_frcg)},
		{"conmin_mfd",8,9,10,1,kw_10,0.,0.,0,N_mdm(lit,methodName_conmin_mfd)},
		{"constraint_tolerance",10,0,8,0,0,0.,0.,0,N_mdm(Real,constraintTolerance)},
		{"convergence_tolerance",10,0,7,0,0,0.,0.,0,N_mdm(Real,convergenceTolerance)},
		{"dace",8,12,10,1,kw_35,0.,0.,0,N_mdm(lit,methodName_dace)},
		{"dl_solver",11,9,10,1,kw_10,0.,0.,0,N_mdm(slit2,3methodName_dl_solver)},
		{"dot",0x4008,9,10,1,kw_10,0.,0.,0,0,0,0,(void*)"dot.dll"},
		{"dot_bfgs",0x4000,9,10,1,kw_10,0.,0.,-1,0,0,0,(void*)"dot.dll"},
		{"dot_frcg",0x4000,9,10,1,kw_10,0.,0.,-2,0,0,0,(void*)"dot.dll"},
		{"dot_mmfd",0x4000,9,10,1,kw_10,0.,0.,-3,0,0,0,(void*)"dot.dll"},
		{"dot_slp",0x4000,9,10,1,kw_10,0.,0.,-4,0,0,0,(void*)"dot.dll"},
		{"dot_sqp",0x4000,9,10,1,kw_10,0.,0.,-5,0,0,0,(void*)"dot.dll"},
		{"efficient_global",8,1,10,1,kw_33,0.,0.,0,N_mdm(lit,methodName_efficient_global)},
		{"fsu_cvt",8,6,10,1,kw_37,0.,0.,0,N_mdm(lit,methodName_fsu_cvt)},
		{"fsu_quasi_mc",8,10,10,1,kw_38},
		{"id_method",11,0,1,0,0,0.,0.,0,N_mdm(str,idMethod)},
		{"jega",0x4008,9,10,1,kw_10,0.,0.,0,0,0,0,(void*)"jega.dll"},
		{"list_parameter_study",8,1,10,1,kw_39,0.,0.,0,N_mdm(lit,methodName_list_parameter_study)},
		{"max_function_evaluations",9,0,5,0,0,0.,0.,0,N_mdm(nnint,maxFunctionEvaluations)},
		{"max_iterations",9,0,4,0,0,0.,0.,0,N_mdm(nnint,maxIterations)},
		{"model_pointer",11,0,2,0,0,0.,0.,0,N_mdm(str,modelPointer)},
		{"moga",0x4000,9,10,1,kw_10,0.,0.,-5,0,0,0,(void*)"jega.dll"},
		{"multidim_parameter_study",8,1,10,1,kw_40,0.,0.,0,N_mdm(lit,methodName_multidim_parameter_study)},
		{"ncsu_direct",8,4,10,1,kw_41,0.,0.,0,N_mdm(lit,methodName_ncsu_direct)},
		{"nl2sol",8,9,10,1,kw_42,0.,0.,0,N_mdm(lit,methodName_nl2sol)},
		{"nlpql_sqp",0x4008,9,10,1,kw_10,0.,0.,0,0,0,0,(void*)"nlpql.dll"},
		{"nlssol",0x4000,9,10,1,kw_10,0.,0.,27,0,0,0,(void*)"npsol.dll"},
		{"nlssol_sqp",0x4000,9,10,1,kw_10,0.,0.,26,0,0,0,(void*)"npsol.dll"},
		{"nond_bayes_calib",8,1,10,1,kw_44,0.,0.,0,N_mdm(lit,methodName_nond_bayes_calib)},
		{"nond_global_evidence",8,3,10,1,kw_52,0.,0.,0,N_mdm(lit,methodName_nond_global_evidence)},
		{"nond_global_interval_est",8,3,10,1,kw_54,0.,0.,0,N_mdm(lit,methodName_nond_global_interval_est)},
		{"nond_global_reliability",8,6,10,1,kw_62,0.,0.,0,N_mdm(lit,methodName_nond_global_reliability)},
		{"nond_importance",8,1,10,1,kw_65,0.,0.,0,N_mdm(lit,methodName_nond_importance)},
		{"nond_local_evidence",8,6,10,1,kw_71,0.,0.,0,N_mdm(lit,methodName_nond_local_evidence)},
		{"nond_local_interval_est",8,2,10,1,kw_72,0.,0.,0,N_mdm(lit,methodName_nond_local_interval_est)},
		{"nond_local_reliability",8,4,10,1,kw_79,0.,0.,0,N_mdm(lit2,methodName_nond_local_reliability)},
		{"nond_polynomial_chaos",8,11,10,1,kw_91,0.,0.,0,N_mdm(lit,methodName_nond_polynomial_chaos)},
		{"nond_sampling",8,2,10,1,kw_94,0.,0.,0,N_mdm(lit,methodName_nond_sampling)},
		{"nond_stoch_collocation",8,6,10,1,kw_98,0.,0.,0,N_mdm(lit,methodName_nond_stoch_collocation)},
		{"nonlinear_cg",8,1,10,1,kw_99,0.,0.,0,N_mdm(lit,methodName_nonlinear_cg)},
		{"npsol",0x4000,9,10,1,kw_10,0.,0.,13,0,0,0,(void*)"npsol.dll"},
		{"npsol_sqp",0x4000,9,10,1,kw_10,0.,0.,12,0,0,0,(void*)"npsol.dll"},
		{"optpp_cg",8,0,10,1,kw_101,0.,0.,0,N_mdm(lit,methodName_optpp_cg)},
		{"optpp_fd_newton",8,5,10,1,kw_104,0.,0.,0,N_mdf(litpp,methodName_optpp_fd_newton)},
		{"optpp_g_newton",8,5,10,1,kw_104,0.,0.,0,N_mdf(litpp,methodName_optpp_g_newton)},
		{"optpp_newton",8,5,10,1,kw_104,0.,0.,0,N_mdf(litpp,methodName_optpp_newton)},
		{"optpp_pds",8,1,10,1,kw_102,0.,0.,0,N_mdm(lit,methodName_optpp_pds)},
		{"optpp_q_newton",8,5,10,1,kw_104,0.,0.,0,N_mdf(litpp,methodName_optpp_q_newton)},
		{"output",8,4,3,0,kw_105},
		{"psuade_moat",8,1,10,1,kw_106,0.,0.,0,N_mdm(lit,methodName_psuade_moat)},
		{"scaling",8,0,9,0,0,0.,0.,0,N_mdm(true,methodScaling)},
		{"soga",0x4000,9,10,1,kw_10,0.,0.,-35,0,0,0,(void*)"jega.dll"},
		{"speculative",8,0,6,0,0,0.,0.,0,N_mdm(true,speculativeFlag)},
		{"stanford",0x4008,9,10,1,kw_10,0.,0.,0,0,0,0,(void*)"npsol.dll"},
		{"surrogate_based_global",8,3,10,1,kw_107,0.,0.,0,N_mdm(lit,methodName_surrogate_based_global)},
		{"surrogate_based_local",8,9,10,1,kw_113,0.,0.,0,N_mdm(lit,methodName_surrogate_based_local)},
		{"vector_parameter_study",8,3,10,1,kw_114,0.,0.,0,N_mdm(lit,methodName_vector_parameter_study)}
		},
	kw_116[1] = {
		{"optional_interface_responses_pointer",11,0,1,0,0,0.,0.,0,N_mom(str,optionalInterfRespPointer)}
		},
	kw_117[4] = {
		{"primary_response_mapping",14,0,3,0,0,0.,0.,0,N_mom(RealDL,primaryRespCoeffs)},
		{"primary_variable_mapping",15,0,1,0,0,0.,0.,0,N_mom(strL,primaryVarMaps)},
		{"secondary_response_mapping",14,0,4,0,0,0.,0.,0,N_mom(RealDL,secondaryRespCoeffs)},
		{"secondary_variable_mapping",15,0,2,0,0,0.,0.,0,N_mom(strL,secondaryVarMaps)}
		},
	kw_118[2] = {
		{"optional_interface_pointer",11,1,1,0,kw_116,0.,0.,0,N_mom(str,interfacePointer)},
		{"sub_method_pointer",11,4,2,1,kw_117,0.,0.,0,N_mom(str,subMethodPointer)}
		},
	kw_119[1] = {
		{"interface_pointer",11,0,1,0,0,0.,0.,0,N_mom(str,interfacePointer)}
		},
	kw_120[6] = {
		{"additive",8,0,2,2,0,0.,0.,0,N_mom(lit,approxCorrectionType_additive)},
		{"combined",8,0,2,2,0,0.,0.,0,N_mom(lit,approxCorrectionType_combined)},
		{"first_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_1)},
		{"multiplicative",8,0,2,2,0,0.,0.,0,N_mom(lit,approxCorrectionType_multiplicative)},
		{"second_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_2)},
		{"zeroth_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_0)}
		},
	kw_121[3] = {
		{"constant",8,0,1,1,0,0.,0.,0,N_mom(order,trendOrder_0)},
		{"linear",8,0,1,1,0,0.,0.,0,N_mom(order,trendOrder_1)},
		{"quadratic",8,0,1,1,0,0.,0.,0,N_mom(order,trendOrder_2)}
		},
	kw_122[2] = {
		{"point_selection",8,0,1,0,0,0.,0.,0,N_mom(true,pointSelection)},
		{"trend",8,3,2,0,kw_121}
		},
	kw_123[5] = {
		{"conmin_seed",14,0,2,0,0,0.,0.,0,N_mom(RealDL,krigingConminSeed)},
		{"correlations",14,0,1,0,0,0.,0.,0,N_mom(RealDL,krigingCorrelations)},
		{"max_correlations",14,0,4,0,0,0.,0.,0,N_mom(RealDL,krigingMaxCorrelations)},
		{"max_trials",9,0,3,0,0,0.,0.,0,N_mom(shint,krigingMaxTrials)},
		{"min_correlations",14,0,5,0,0,0.,0.,0,N_mom(RealDL,krigingMinCorrelations)}
		},
	kw_124[2] = {
		{"cubic",8,0,1,1,0,0.,0.,0,N_mom(lit,marsInterpolation_cubic)},
		{"linear",8,0,1,1,0,0.,0.,0,N_mom(lit,marsInterpolation_linear)}
		},
	kw_125[2] = {
		{"interpolation",8,2,2,0,kw_124},
		{"max_bases",9,0,1,0,0,0.,0.,0,N_mom(shint,marsMaxBases)}
		},
	kw_126[2] = {
		{"poly_order",9,0,1,0,0,0.,0.,0,N_mom(shint,mlsPolyOrder)},
		{"weight_function",9,0,2,0,0,0.,0.,0,N_mom(shint,mlsWeightFunction)}
		},
	kw_127[3] = {
		{"nodes",9,0,1,0,0,0.,0.,0,N_mom(shint,annNodes)},
		{"random_weight",9,0,3,0,0,0.,0.,0,N_mom(shint,annRandomWeight)},
		{"range",10,0,2,0,0,0.,0.,0,N_mom(Real,annRange)}
		},
	kw_128[3] = {
		{"cubic",8,0,1,1,0,0.,0.,0,N_mom(order,polynomialOrder_3)},
		{"linear",8,0,1,1,0,0.,0.,0,N_mom(order,polynomialOrder_1)},
		{"quadratic",8,0,1,1,0,0.,0.,0,N_mom(order,polynomialOrder_2)}
		},
	kw_129[4] = {
		{"bases",9,0,1,0,0,0.,0.,0,N_mom(shint,rbfBases)},
		{"max_pts",9,0,2,0,0,0.,0.,0,N_mom(shint,rbfMaxPts)},
		{"max_subsets",9,0,4,0,0,0.,0.,0,N_mom(shint,rbfMaxSubsets)},
		{"min_partition",9,0,3,0,0,0.,0.,0,N_mom(shint,rbfMinPartition)}
		},
	kw_130[3] = {
		{"all",8,0,1,1,0,0.,0.,0,N_mom(lit,approxPointReuse_all)},
		{"none",8,0,1,1,0,0.,0.,0,N_mom(lit,approxPointReuse_none)},
		{"region",8,0,1,1,0,0.,0.,0,N_mom(lit,approxPointReuse_region)}
		},
	kw_131[18] = {
		{"correction",8,6,7,0,kw_120},
		{"dace_method_pointer",11,0,3,0,0,0.,0.,0,N_mom(str,subMethodPointer)},
		{"diagnostics",15,0,8,0,0,0.,0.,0,N_mom(strL,diagMetrics)},
		{"gaussian_process",8,2,1,1,kw_122,0.,0.,0,N_mom(lit,surrogateType_global_gaussian)},
		{"kriging",8,5,1,1,kw_123,0.,0.,0,N_mom(lit,surrogateType_global_kriging)},
		{"mars",8,2,1,1,kw_125,0.,0.,0,N_mom(lit,surrogateType_global_mars)},
		{"minimum_points",8,0,2,0,0,0.,0.,0,N_mom(true,pointsMinimum)},
		{"moving_least_squares",8,2,1,1,kw_126,0.,0.,0,N_mom(lit,surrogateType_global_moving_least_squares)},
		{"neural_network",8,3,1,1,kw_127,0.,0.,0,N_mom(lit,surrogateType_global_neural_network)},
		{"points_file",11,0,5,0,0,0.,0.,0,N_mom(str,approxPointReuseFile)},
		{"polynomial",8,3,1,1,kw_128,0.,0.,0,N_mom(lit,surrogateType_global_polynomial)},
		{"radial_basis",8,4,1,1,kw_129,0.,0.,0,N_mom(lit,surrogateType_global_radial_basis)},
		{"recommended_points",8,0,2,0,0,0.,0.,0,N_mom(true,pointsRecommended)},
		{"reuse_points",8,3,4,0,kw_130},
		{"reuse_samples",0,3,4,0,kw_130,0.,0.,-1},
		{"samples_file",3,0,5,0,0,0.,0.,-6,N_mom(str,approxPointReuseFile)},
		{"total_points",9,0,2,0,0,0.,0.,0,N_mom(int,pointsTotal)},
		{"use_gradients",8,0,6,0,0,0.,0.,0,N_mom(true,approxGradUsageFlag)}
		},
	kw_132[6] = {
		{"additive",8,0,2,2,0,0.,0.,0,N_mom(lit,approxCorrectionType_additive)},
		{"combined",8,0,2,2,0,0.,0.,0,N_mom(lit,approxCorrectionType_combined)},
		{"first_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_1)},
		{"multiplicative",8,0,2,2,0,0.,0.,0,N_mom(lit,approxCorrectionType_multiplicative)},
		{"second_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_2)},
		{"zeroth_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_0)}
		},
	kw_133[3] = {
		{"correction",8,6,3,3,kw_132},
		{"high_fidelity_model_pointer",11,0,2,2,0,0.,0.,0,N_mom(str,truthModelPointer)},
		{"low_fidelity_model_pointer",11,0,1,1,0,0.,0.,0,N_mom(str,lowFidelityModelPointer)}
		},
	kw_134[1] = {
		{"actual_model_pointer",11,0,1,1,0,0.,0.,0,N_mom(str,truthModelPointer)}
		},
	kw_135[2] = {
		{0,0,1,0,0,kw_134},
		{"taylor_series",8,0,1,1}
		},
	kw_136[2] = {
		{0,0,1,0,0,kw_134},
		{"tana",8,0,1,1}
		},
	kw_137[5] = {
		{"global",8,18,2,1,kw_131},
		{"hierarchical",8,3,2,1,kw_133,0.,0.,0,N_mom(lit,surrogateType_hierarchical)},
		{"id_surrogates",13,0,1,0,0,0.,0.,0,N_mom(intset,surrogateFnIndices)},
		{"local",8,1,2,1,kw_135,0.,0.,0,N_mom(lit,surrogateType_local_taylor)},
		{"multipoint",8,1,2,1,kw_136,0.,0.,0,N_mom(lit,surrogateType_multipoint_tana)}
		},
	kw_138[6] = {
		{"id_model",11,0,1,0,0,0.,0.,0,N_mom(str,idModel)},
		{"nested",8,2,4,1,kw_118,0.,0.,0,N_mom(lit,modelType_nested)},
		{"responses_pointer",11,0,3,0,0,0.,0.,0,N_mom(str,responsesPointer)},
		{"single",8,1,4,1,kw_119,0.,0.,0,N_mom(lit,modelType_single)},
		{"surrogate",8,5,4,1,kw_137,0.,0.,0,N_mom(lit,modelType_surrogate)},
		{"variables_pointer",11,0,2,0,0,0.,0.,0,N_mom(str,variablesPointer)}
		},
	kw_139[1] = {
		{"ignore_bounds",8,0,1,0,0,0.,0.,0,N_rem(true,ignoreBounds)}
		},
	kw_140[8] = {
		{"central",8,0,4,0,0,0.,0.,0,N_rem(lit,intervalType_central)},
		{"dakota",8,1,2,0,kw_139,0.,0.,0,N_rem(lit,methodSource_dakota)},
		{"fd_gradient_step_size",0x406,0,5,0,0,0.,0.,1,N_rem(RealL,fdGradStepSize)},
		{"fd_step_size",0x40e,0,5,0,0,0.,0.,0,N_rem(RealL,fdGradStepSize)},
		{"forward",8,0,4,0,0,0.,0.,0,N_rem(lit,intervalType_forward)},
		{"interval_type",8,0,3},
		{"method_source",8,0,1},
		{"vendor",8,0,2,0,0,0.,0.,0,N_rem(lit,methodSource_vendor)}
		},
	kw_141[3] = {
		{0,0,8,0,0,kw_140},
		{"id_analytic_gradients",13,0,2,2,0,0.,0.,0,N_rem(intL,idAnalyticGrads)},
		{"id_numerical_gradients",13,0,1,1,0,0.,0.,0,N_rem(intL,idNumericalGrads)}
		},
	kw_142[2] = {
		{"fd_hessian_step_size",6,0,1,0,0,0.,0.,1,N_rem(RealL,fdHessStepSize)},
		{"fd_step_size",14,0,1,0,0,0.,0.,0,N_rem(RealL,fdHessStepSize)}
		},
	kw_143[1] = {
		{"damped",8,0,1,0,0,0.,0.,0,N_rem(lit,quasiHessianType_damped_bfgs)}
		},
	kw_144[2] = {
		{"bfgs",8,1,1,1,kw_143,0.,0.,0,N_rem(lit,quasiHessianType_bfgs)},
		{"sr1",8,0,1,1,0,0.,0.,0,N_rem(lit,quasiHessianType_sr1)}
		},
	kw_145[5] = {
		{"central",8,0,2,0,0,0.,0.,0,N_rem(true,centralHess)},
		{"forward",8,0,2,0,0,0.,0.,0,N_rem(false,centralHess)},
		{"id_analytic_hessians",13,0,4,0,0,0.,0.,0,N_rem(intL,idAnalyticHessians)},
		{"id_numerical_hessians",13,2,1,0,kw_142,0.,0.,0,N_rem(intL,idNumericalHessians)},
		{"id_quasi_hessians",13,2,3,0,kw_144,0.,0.,0,N_rem(intL,idQuasiHessians)}
		},
	kw_146[3] = {
		{"nonlinear_equality_scale_types",0x80f,0,2,0,0,0.,0.,0,N_rem(strL,nonlinearEqScaleTypes)},
		{"nonlinear_equality_scales",0x80e,0,3,0,0,0.,0.,0,N_rem(RealDL,nonlinearEqScales)},
		{"nonlinear_equality_targets",14,0,1,0,0,0.,0.,0,N_rem(RealDL,nonlinearEqTargets)}
		},
	kw_147[4] = {
		{"nonlinear_inequality_lower_bounds",14,0,1,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqLowerBnds)},
		{"nonlinear_inequality_scale_types",0x80f,0,3,0,0,0.,0.,0,N_rem(strL,nonlinearIneqScaleTypes)},
		{"nonlinear_inequality_scales",0x80e,0,4,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqScales)},
		{"nonlinear_inequality_upper_bounds",14,0,2,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqUpperBnds)}
		},
	kw_148[6] = {
		{"least_squares_data_file",11,0,1,0,0,0.,0.,0,N_rem(str,leastSqDataFile)},
		{"least_squares_term_scale_types",0x80f,0,2,0,0,0.,0.,0,N_rem(strL,primaryRespFnScaleTypes)},
		{"least_squares_term_scales",0x80e,0,3,0,0,0.,0.,0,N_rem(RealDL,primaryRespFnScales)},
		{"least_squares_weights",14,0,4,0,0,0.,0.,0,N_rem(RealDL,primaryRespFnWeights)},
		{"num_nonlinear_equality_constraints",0x29,3,6,0,kw_146,0.,0.,0,N_rem(nnintz,numNonlinearEqConstraints)},
		{"num_nonlinear_inequality_constraints",0x29,4,5,0,kw_147,0.,0.,0,N_rem(nnintz,numNonlinearIneqConstraints)}
		},
	kw_149[3] = {
		{"nonlinear_equality_scale_types",0x80f,0,2,0,0,0.,0.,0,N_rem(strL,nonlinearEqScaleTypes)},
		{"nonlinear_equality_scales",0x80e,0,3,0,0,0.,0.,0,N_rem(RealDL,nonlinearEqScales)},
		{"nonlinear_equality_targets",14,0,1,0,0,0.,0.,0,N_rem(RealDL,nonlinearEqTargets)}
		},
	kw_150[4] = {
		{"nonlinear_inequality_lower_bounds",14,0,1,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqLowerBnds)},
		{"nonlinear_inequality_scale_types",0x80f,0,3,0,0,0.,0.,0,N_rem(strL,nonlinearIneqScaleTypes)},
		{"nonlinear_inequality_scales",0x80e,0,4,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqScales)},
		{"nonlinear_inequality_upper_bounds",14,0,2,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqUpperBnds)}
		},
	kw_151[5] = {
		{"multi_objective_weights",14,0,3,0,0,0.,0.,0,N_rem(RealDL,primaryRespFnWeights)},
		{"num_nonlinear_equality_constraints",0x29,3,5,0,kw_149,0.,0.,0,N_rem(nnintz,numNonlinearEqConstraints)},
		{"num_nonlinear_inequality_constraints",0x29,4,4,0,kw_150,0.,0.,0,N_rem(nnintz,numNonlinearIneqConstraints)},
		{"objective_function_scale_types",0x80f,0,1,0,0,0.,0.,0,N_rem(strL,primaryRespFnScaleTypes)},
		{"objective_function_scales",0x80e,0,2,0,0,0.,0.,0,N_rem(RealDL,primaryRespFnScales)}
		},
	kw_152[4] = {
		{"central",8,0,2,0,0,0.,0.,0,N_rem(true,centralHess)},
		{"fd_hessian_step_size",6,0,1,0,0,0.,0.,1,N_rem(RealL,fdHessStepSize)},
		{"fd_step_size",14,0,1,0,0,0.,0.,0,N_rem(RealL,fdHessStepSize)},
		{"forward",8,0,2,0,0,0.,0.,0,N_rem(false,centralHess)}
		},
	kw_153[1] = {
		{"damped",8,0,1,0,0,0.,0.,0,N_rem(lit,quasiHessianType_damped_bfgs)}
		},
	kw_154[2] = {
		{"bfgs",8,1,1,1,kw_153,0.,0.,0,N_rem(lit,quasiHessianType_bfgs)},
		{"sr1",8,0,1,1,0,0.,0.,0,N_rem(lit,quasiHessianType_sr1)}
		},
	kw_155[15] = {
		{"analytic_gradients",8,0,4,2,0,0.,0.,0,N_rem(lit,gradientType_analytic)},
		{"analytic_hessians",8,0,5,3,0,0.,0.,0,N_rem(lit,hessianType_analytic)},
		{"descriptors",15,0,2,0,0,0.,0.,0,N_rem(strL,responseLabels)},
		{"id_responses",11,0,1,0,0,0.,0.,0,N_rem(str,idResponses)},
		{"mixed_gradients",8,2,4,2,kw_141,0.,0.,0,N_rem(lit,gradientType_mixed)},
		{"mixed_hessians",8,5,5,3,kw_145,0.,0.,0,N_rem(lit,hessianType_mixed)},
		{"no_gradients",8,0,4,2,0,0.,0.,0,N_rem(lit,gradientType_none)},
		{"no_hessians",8,0,5,3,0,0.,0.,0,N_rem(lit,hessianType_none)},
		{"num_least_squares_terms",0x29,6,3,1,kw_148,0.,0.,0,N_rem(nnintz,numLeastSqTerms)},
		{"num_objective_functions",0x29,5,3,1,kw_151,0.,0.,0,N_rem(nnintz,numObjectiveFunctions)},
		{"num_response_functions",0x29,0,3,1,0,0.,0.,0,N_rem(nnintz,numResponseFunctions)},
		{"numerical_gradients",8,8,4,2,kw_140,0.,0.,0,N_rem(lit,gradientType_numerical)},
		{"numerical_hessians",8,4,5,3,kw_152,0.,0.,0,N_rem(lit,hessianType_numerical)},
		{"quasi_hessians",8,2,5,3,kw_154,0.,0.,0,N_rem(lit,hessianType_quasi)},
		{"response_descriptors",7,0,2,0,0,0.,0.,-12,N_rem(strL,responseLabels)}
		},
	kw_156[1] = {
		{"method_list",15,0,1,1,0,0.,0.,0,N_stm(strL,hybridMethodList)}
		},
	kw_157[3] = {
		{"global_method_pointer",11,0,1,1,0,0.,0.,0,N_stm(str,hybridGlobalMethodPointer)},
		{"local_method_pointer",11,0,2,2,0,0.,0.,0,N_stm(str,hybridLocalMethodPointer)},
		{"local_search_probability",10,0,3,0,0,0.,0.,0,N_stm(Real,hybridLSProb)}
		},
	kw_158[2] = {
		{"method_list",15,0,2,1,0,0.,0.,0,N_stm(strL,hybridMethodList)},
		{"num_solutions_transferred",9,0,1,0,0,0.,0.,0,N_stm(int,hybridNumSolnsTrans)}
		},
	kw_159[5] = {
		{"collaborative",8,1,1,1,kw_156,0.,0.,0,N_stm(lit,hybridType_collaborative)},
		{"coupled",0,3,1,1,kw_157,0.,0.,1,N_stm(lit,hybridType_embedded)},
		{"embedded",8,3,1,1,kw_157,0.,0.,0,N_stm(lit,hybridType_embedded)},
		{"sequential",8,2,1,1,kw_158,0.,0.,0,N_stm(lit,hybridType_sequential)},
		{"uncoupled",0,2,1,1,kw_158,0.,0.,-1,N_stm(lit,hybridType_sequential)}
		},
	kw_160[1] = {
		{"seed",9,0,1,0,0,0.,0.,0,N_stm(int,concurrentSeed)}
		},
	kw_161[3] = {
		{"method_pointer",11,0,1,1,0,0.,0.,0,N_stm(str,methodPointer)},
		{"random_starts",9,1,2,0,kw_160,0.,0.,0,N_stm(int,concurrentRandomJobs)},
		{"starting_points",14,0,3,0,0,0.,0.,0,N_stm(RealL,concurrentParameterSets)}
		},
	kw_162[1] = {
		{"seed",9,0,1,0,0,0.,0.,0,N_stm(int,concurrentSeed)}
		},
	kw_163[5] = {
		{"method_pointer",11,0,1,1,0,0.,0.,0,N_stm(str,methodPointer)},
		{"multi_objective_weight_sets",6,0,3,0,0,0.,0.,3,N_stm(RealL,concurrentParameterSets)},
		{"opt_method_pointer",3,0,1,1,0,0.,0.,-2,N_stm(str,methodPointer)},
		{"random_weight_sets",9,1,2,0,kw_162,0.,0.,0,N_stm(int,concurrentRandomJobs)},
		{"weight_sets",14,0,3,0,0,0.,0.,0,N_stm(RealL,concurrentParameterSets)}
		},
	kw_164[1] = {
		{"method_pointer",11,0,1,0,0,0.,0.,0,N_stm(str,methodPointer)}
		},
	kw_165[1] = {
		{"tabular_graphics_file",11,0,1,0,0,0.,0.,0,N_stm(str,tabularDataFile)}
		},
	kw_166[10] = {
		{"graphics",8,0,1,0,0,0.,0.,0,N_stm(true,graphicsFlag)},
		{"hybrid",8,5,7,1,kw_159,0.,0.,0,N_stm(lit,strategyType_hybrid)},
		{"iterator_self_scheduling",8,0,5,0,0,0.,0.,0,N_stm(lit,iteratorScheduling_self)},
		{"iterator_servers",9,0,4,0,0,0.,0.,0,N_stm(int,iteratorServers)},
		{"iterator_static_scheduling",8,0,6,0,0,0.,0.,0,N_stm(lit,iteratorScheduling_static)},
		{"multi_start",8,3,7,1,kw_161,0.,0.,0,N_stm(lit,strategyType_multi_start)},
		{"output_precision",0x29,0,3,0,0,0.,0.,0,N_stm(int,outputPrecision)},
		{"pareto_set",8,5,7,1,kw_163,0.,0.,0,N_stm(lit,strategyType_pareto_set)},
		{"single_method",8,1,7,1,kw_164,0.,0.,0,N_stm(lit,strategyType_single_method)},
		{"tabular_graphics_data",8,1,2,0,kw_165,0.,0.,0,N_stm(true,tabularDataFlag)}
		},
	kw_167[10] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,betaUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(RealLb,betaUncBetas)},
		{"buv_alphas",6,0,1,1,0,0.,0.,-2,N_vam(RealLb,betaUncAlphas)},
		{"buv_betas",6,0,2,2,0,0.,0.,-2,N_vam(RealLb,betaUncBetas)},
		{"buv_descriptors",7,0,5,0,0,0.,0.,3,N_vae(caulbl,CAUVar_beta)},
		{"buv_lower_bounds",6,0,3,3,0,0.,0.,3,N_vam(RealLd,betaUncLowerBnds)},
		{"buv_upper_bounds",6,0,4,4,0,0.,0.,3,N_vam(RealLd,betaUncUpperBnds)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(caulbl,CAUVar_beta)},
		{"lower_bounds",14,0,3,3,0,0.,0.,0,N_vam(RealLd,betaUncLowerBnds)},
		{"upper_bounds",14,0,4,4,0,0.,0.,0,N_vam(RealLd,betaUncUpperBnds)}
		},
	kw_168[3] = {
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(dailbl,DAUIVar_binomial)},
		{"num_trials",13,0,2,2,0,0.,0.,0,N_vam(intDL,binomialUncNumTrials)},
		{"prob_per_trial",14,0,1,1,0,0.,0.,0,N_vam(RealLd,binomialUncProbPerTrial)}
		},
	kw_169[12] = {
		{"cdv_descriptors",7,0,6,0,0,0.,0.,6,N_vam(strL,continuousDesignLabels)},
		{"cdv_initial_point",6,0,1,0,0,0.,0.,6,N_vam(RealLd,continuousDesignVars)},
		{"cdv_lower_bounds",6,0,2,0,0,0.,0.,6,N_vam(RealLd,continuousDesignLowerBnds)},
		{"cdv_scale_types",0x807,0,4,0,0,0.,0.,6,N_vam(strL,continuousDesignScaleTypes)},
		{"cdv_scales",0x806,0,5,0,0,0.,0.,6,N_vam(RealLd,continuousDesignScales)},
		{"cdv_upper_bounds",6,0,3,0,0,0.,0.,6,N_vam(RealLd,continuousDesignUpperBnds)},
		{"descriptors",15,0,6,0,0,0.,0.,0,N_vam(strL,continuousDesignLabels)},
		{"initial_point",14,0,1,0,0,0.,0.,0,N_vam(RealLd,continuousDesignVars)},
		{"lower_bounds",14,0,2,0,0,0.,0.,0,N_vam(RealLd,continuousDesignLowerBnds)},
		{"scale_types",0x80f,0,4,0,0,0.,0.,0,N_vam(strL,continuousDesignScaleTypes)},
		{"scales",0x80e,0,5,0,0,0.,0.,0,N_vam(RealLd,continuousDesignScales)},
		{"upper_bounds",14,0,3,0,0,0.,0.,0,N_vam(RealLd,continuousDesignUpperBnds)}
		},
	kw_170[8] = {
		{"csv_descriptors",7,0,4,0,0,0.,0.,4,N_vam(strL,continuousStateLabels)},
		{"csv_initial_state",6,0,1,0,0,0.,0.,4,N_vam(RealLd,continuousStateVars)},
		{"csv_lower_bounds",6,0,2,0,0,0.,0.,4,N_vam(RealLd,continuousStateLowerBnds)},
		{"csv_upper_bounds",6,0,3,0,0,0.,0.,4,N_vam(RealLd,continuousStateUpperBnds)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,continuousStateLabels)},
		{"initial_state",14,0,1,0,0,0.,0.,0,N_vam(RealLd,continuousStateVars)},
		{"lower_bounds",14,0,2,0,0,0.,0.,0,N_vam(RealLd,continuousStateLowerBnds)},
		{"upper_bounds",14,0,3,0,0,0.,0.,0,N_vam(RealLd,continuousStateUpperBnds)}
		},
	kw_171[8] = {
		{"ddv_descriptors",7,0,4,0,0,0.,0.,4,N_vam(strL,discreteDesignRangeLabels)},
		{"ddv_initial_point",5,0,1,0,0,0.,0.,4,N_vam(intDL,discreteDesignRangeVars)},
		{"ddv_lower_bounds",5,0,2,0,0,0.,0.,4,N_vam(intDL,discreteDesignRangeLowerBnds)},
		{"ddv_upper_bounds",5,0,3,0,0,0.,0.,4,N_vam(intDL,discreteDesignRangeUpperBnds)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteDesignRangeLabels)},
		{"initial_point",13,0,1,0,0,0.,0.,0,N_vam(intDL,discreteDesignRangeVars)},
		{"lower_bounds",13,0,2,0,0,0.,0.,0,N_vam(intDL,discreteDesignRangeLowerBnds)},
		{"upper_bounds",13,0,3,0,0,0.,0.,0,N_vam(intDL,discreteDesignRangeUpperBnds)}
		},
	kw_172[4] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteDesignSetIntLabels)},
		{"initial_point",13,0,1,0,0,0.,0.,0,N_vam(intDL,discreteDesignSetIntVars)},
		{"num_set_values",13,0,2,0,0,0.,0.,0,N_vam(vil,Var_Info_ndsvi)},
		{"set_values",13,0,3,1,0,0.,0.,0,N_vam(vil,Var_Info_dsvi)}
		},
	kw_173[4] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteDesignSetRealLabels)},
		{"initial_point",14,0,1,0,0,0.,0.,0,N_vam(RealLd,discreteDesignSetRealVars)},
		{"num_set_values",13,0,2,0,0,0.,0.,0,N_vam(vil,Var_Info_ndsvr)},
		{"set_values",14,0,3,1,0,0.,0.,0,N_vam(vrl,Var_Info_dsvr)}
		},
	kw_174[8] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteStateRangeLabels)},
		{"dsv_descriptors",7,0,4,0,0,0.,0.,-1,N_vam(strL,discreteStateRangeLabels)},
		{"dsv_initial_state",5,0,1,0,0,0.,0.,3,N_vam(intDL,discreteStateRangeVars)},
		{"dsv_lower_bounds",5,0,2,0,0,0.,0.,3,N_vam(intDL,discreteStateRangeLowerBnds)},
		{"dsv_upper_bounds",5,0,3,0,0,0.,0.,3,N_vam(intDL,discreteStateRangeUpperBnds)},
		{"initial_state",13,0,1,0,0,0.,0.,0,N_vam(intDL,discreteStateRangeVars)},
		{"lower_bounds",13,0,2,0,0,0.,0.,0,N_vam(intDL,discreteStateRangeLowerBnds)},
		{"upper_bounds",13,0,3,0,0,0.,0.,0,N_vam(intDL,discreteStateRangeUpperBnds)}
		},
	kw_175[4] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteStateSetIntLabels)},
		{"initial_state",13,0,1,0,0,0.,0.,0,N_vam(intDL,discreteStateSetIntVars)},
		{"num_set_values",13,0,2,0,0,0.,0.,0,N_vam(vil,Var_Info_nssvi)},
		{"set_values",13,0,3,1,0,0.,0.,0,N_vam(vil,Var_Info_ssvi)}
		},
	kw_176[4] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteStateSetRealLabels)},
		{"initial_state",14,0,1,0,0,0.,0.,0,N_vam(RealLd,discreteStateSetRealVars)},
		{"num_set_values",13,0,2,0,0,0.,0.,0,N_vam(vil,Var_Info_nssvr)},
		{"set_values",14,0,3,1,0,0.,0.,0,N_vam(vrl,Var_Info_ssvr)}
		},
	kw_177[4] = {
		{"betas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,exponentialUncBetas)},
		{"descriptors",15,0,2,0,0,0.,0.,0,N_vae(caulbl,CAUVar_exponential)},
		{"euv_betas",6,0,1,1,0,0.,0.,-2,N_vam(RealLb,exponentialUncBetas)},
		{"euv_descriptors",7,0,2,0,0,0.,0.,-2,N_vae(caulbl,CAUVar_exponential)}
		},
	kw_178[6] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,frechetUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(RealLd,frechetUncBetas)},
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(caulbl,CAUVar_frechet)},
		{"fuv_alphas",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,frechetUncAlphas)},
		{"fuv_betas",6,0,2,2,0,0.,0.,-3,N_vam(RealLd,frechetUncBetas)},
		{"fuv_descriptors",7,0,3,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_frechet)}
		},
	kw_179[6] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,gammaUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(RealLb,gammaUncBetas)},
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(caulbl,CAUVar_gamma)},
		{"gauv_alphas",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,gammaUncAlphas)},
		{"gauv_betas",6,0,2,2,0,0.,0.,-3,N_vam(RealLb,gammaUncBetas)},
		{"gauv_descriptors",7,0,3,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_gamma)}
		},
	kw_180[2] = {
		{"descriptors",15,0,2,0,0,0.,0.,0,N_vae(dailbl,DAUIVar_geometric)},
		{"prob_per_trial",14,0,1,1,0,0.,0.,0,N_vam(RealLd,geometricUncProbPerTrial)}
		},
	kw_181[6] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,gumbelUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(RealLd,gumbelUncBetas)},
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(caulbl,CAUVar_gumbel)},
		{"guuv_alphas",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,gumbelUncAlphas)},
		{"guuv_betas",6,0,2,2,0,0.,0.,-3,N_vam(RealLd,gumbelUncBetas)},
		{"guuv_descriptors",7,0,3,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_gumbel)}
		},
	kw_182[10] = {
		{"abscissas",14,0,2,1,0,0.,0.,0,N_vam(vrl,Var_Info_ba)},
		{"counts",14,0,3,2,0,0.,0.,0,N_vam(vrl,Var_Info_bc)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_histogram_bin)},
		{"huv_bin_abscissas",6,0,2,1,0,0.,0.,-3,N_vam(vrl,Var_Info_ba)},
		{"huv_bin_counts",6,0,3,2,0,0.,0.,-3,N_vam(vrl,Var_Info_bc)},
		{"huv_bin_descriptors",7,0,4,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_histogram_bin)},
		{"huv_bin_ordinates",6,0,3,2,0,0.,0.,3,N_vam(vrl,Var_Info_bo)},
		{"huv_num_bin_pairs",5,0,1,0,0,0.,0.,1,N_vam(vil,Var_Info_nbp)},
		{"num_pairs",13,0,1,0,0,0.,0.,0,N_vam(vil,Var_Info_nbp)},
		{"ordinates",14,0,3,2,0,0.,0.,0,N_vam(vrl,Var_Info_bo)}
		},
	kw_183[8] = {
		{"abscissas",14,0,2,1,0,0.,0.,0,N_vam(vrl,Var_Info_pa)},
		{"counts",14,0,3,2,0,0.,0.,0,N_vam(vrl,Var_Info_pc)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(darlbl,DAURVar_histogram_point)},
		{"huv_num_point_pairs",5,0,1,0,0,0.,0.,4,N_vam(vil,Var_Info_npp)},
		{"huv_point_abscissas",6,0,2,1,0,0.,0.,-4,N_vam(vrl,Var_Info_pa)},
		{"huv_point_counts",6,0,3,2,0,0.,0.,-4,N_vam(vrl,Var_Info_pc)},
		{"huv_point_descriptors",7,0,4,0,0,0.,0.,-4,N_vae(darlbl,DAURVar_histogram_point)},
		{"num_pairs",13,0,1,0,0,0.,0.,0,N_vam(vil,Var_Info_npp)}
		},
	kw_184[4] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(dailbl,DAUIVar_hypergeometric)},
		{"num_drawn",13,0,3,3,0,0.,0.,0,N_vam(intDL,hyperGeomUncNumDrawn)},
		{"selected_population",13,0,2,2,0,0.,0.,0,N_vam(intDL,hyperGeomUncSelectedPop)},
		{"total_population",13,0,1,1,0,0.,0.,0,N_vam(intDL,hyperGeomUncTotalPop)}
		},
	kw_185[8] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(ceulbl,CEUVar_interval)},
		{"interval_bounds",14,0,3,2,0,0.,0.,0,N_vam(vrl,Var_Info_Ivb)},
		{"interval_probs",14,0,2,1,0,0.,0.,0,N_vam(vrl,Var_Info_Ivp)},
		{"iuv_descriptors",7,0,4,0,0,0.,0.,-3,N_vae(ceulbl,CEUVar_interval)},
		{"iuv_interval_bounds",6,0,3,2,0,0.,0.,-3,N_vam(vrl,Var_Info_Ivb)},
		{"iuv_interval_probs",6,0,2,1,0,0.,0.,-3,N_vam(vrl,Var_Info_Ivp)},
		{"iuv_num_intervals",5,0,1,0,0,0.,0.,1,N_vam(vil,Var_Info_nIv)},
		{"num_intervals",13,0,1,0,0,0.,0.,0,N_vam(vil,Var_Info_nIv)}
		},
	kw_186[2] = {
		{"lnuv_zetas",6,0,1,1,0,0.,0.,1,N_vam(RealLb,lognormalUncZetas)},
		{"zetas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,lognormalUncZetas)}
		},
	kw_187[4] = {
		{"error_factors",14,0,1,1,0,0.,0.,0,N_vam(RealLb,lognormalUncErrFacts)},
		{"lnuv_error_factors",6,0,1,1,0,0.,0.,-1,N_vam(RealLb,lognormalUncErrFacts)},
		{"lnuv_std_deviations",6,0,1,1,0,0.,0.,1,N_vam(RealLb,lognormalUncStdDevs)},
		{"std_deviations",14,0,1,1,0,0.,0.,0,N_vam(RealLb,lognormalUncStdDevs)}
		},
	kw_188[10] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_lognormal)},
		{"lambdas",14,2,1,1,kw_186,0.,0.,0,N_vam(RealLd,lognormalUncLambdas)},
		{"lnuv_descriptors",7,0,4,0,0,0.,0.,-2,N_vae(caulbl,CAUVar_lognormal)},
		{"lnuv_lambdas",6,2,1,1,kw_186,0.,0.,-2,N_vam(RealLd,lognormalUncLambdas)},
		{"lnuv_lower_bounds",6,0,2,0,0,0.,0.,3,N_vam(RealLb,lognormalUncLowerBnds)},
		{"lnuv_means",6,4,1,1,kw_187,0.,0.,3,N_vam(RealLb,lognormalUncMeans)},
		{"lnuv_upper_bounds",6,0,3,0,0,0.,0.,3,N_vam(RealUb,lognormalUncUpperBnds)},
		{"lower_bounds",14,0,2,0,0,0.,0.,0,N_vam(RealLb,lognormalUncLowerBnds)},
		{"means",14,4,1,1,kw_187,0.,0.,0,N_vam(RealLb,lognormalUncMeans)},
		{"upper_bounds",14,0,3,0,0,0.,0.,0,N_vam(RealUb,lognormalUncUpperBnds)}
		},
	kw_189[6] = {
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(caulbl,CAUVar_loguniform)},
		{"lower_bounds",14,0,1,1,0,0.,0.,0,N_vam(RealLb,loguniformUncLowerBnds)},
		{"luuv_descriptors",7,0,3,0,0,0.,0.,-2,N_vae(caulbl,CAUVar_loguniform)},
		{"luuv_lower_bounds",6,0,1,1,0,0.,0.,-2,N_vam(RealLb,loguniformUncLowerBnds)},
		{"luuv_upper_bounds",6,0,2,2,0,0.,0.,1,N_vam(RealUb,loguniformUncUpperBnds)},
		{"upper_bounds",14,0,2,2,0,0.,0.,0,N_vam(RealUb,loguniformUncUpperBnds)}
		},
	kw_190[3] = {
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(dailbl,DAUIVar_negative_binomial)},
		{"num_trials",13,0,2,2,0,0.,0.,0,N_vam(intDL,negBinomialUncNumTrials)},
		{"prob_per_trial",14,0,1,1,0,0.,0.,0,N_vam(RealLd,negBinomialUncProbPerTrial)}
		},
	kw_191[10] = {
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(caulbl,CAUVar_normal)},
		{"lower_bounds",14,0,3,0,0,0.,0.,0,N_vam(RealLd,normalUncLowerBnds)},
		{"means",14,0,1,1,0,0.,0.,0,N_vam(RealLd,normalUncMeans)},
		{"nuv_descriptors",7,0,5,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_normal)},
		{"nuv_lower_bounds",6,0,3,0,0,0.,0.,-3,N_vam(RealLd,normalUncLowerBnds)},
		{"nuv_means",6,0,1,1,0,0.,0.,-3,N_vam(RealLd,normalUncMeans)},
		{"nuv_std_deviations",6,0,2,2,0,0.,0.,2,N_vam(RealLb,normalUncStdDevs)},
		{"nuv_upper_bounds",6,0,4,0,0,0.,0.,2,N_vam(RealLd,normalUncUpperBnds)},
		{"std_deviations",14,0,2,2,0,0.,0.,0,N_vam(RealLb,normalUncStdDevs)},
		{"upper_bounds",14,0,4,0,0,0.,0.,0,N_vam(RealLd,normalUncUpperBnds)}
		},
	kw_192[2] = {
		{"descriptors",15,0,2,0,0,0.,0.,0,N_vae(dailbl,DAUIVar_poisson)},
		{"lambdas",14,0,1,1,0,0.,0.,0,N_vam(RealLd,poissonUncLambdas)}
		},
	kw_193[8] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_triangular)},
		{"lower_bounds",14,0,2,2,0,0.,0.,0,N_vam(RealLb,triangularUncLowerBnds)},
		{"modes",14,0,1,1,0,0.,0.,0,N_vam(RealLd,triangularUncModes)},
		{"tuv_descriptors",7,0,4,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_triangular)},
		{"tuv_lower_bounds",6,0,2,2,0,0.,0.,-3,N_vam(RealLb,triangularUncLowerBnds)},
		{"tuv_modes",6,0,1,1,0,0.,0.,-3,N_vam(RealLd,triangularUncModes)},
		{"tuv_upper_bounds",6,0,3,3,0,0.,0.,1,N_vam(RealUb,triangularUncUpperBnds)},
		{"upper_bounds",14,0,3,3,0,0.,0.,0,N_vam(RealUb,triangularUncUpperBnds)}
		},
	kw_194[6] = {
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(caulbl,CAUVar_uniform)},
		{"lower_bounds",14,0,1,1,0,0.,0.,0,N_vam(RealLb,uniformUncLowerBnds)},
		{"upper_bounds",14,0,2,2,0,0.,0.,0,N_vam(RealUb,uniformUncUpperBnds)},
		{"uuv_descriptors",7,0,3,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_uniform)},
		{"uuv_lower_bounds",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,uniformUncLowerBnds)},
		{"uuv_upper_bounds",6,0,2,2,0,0.,0.,-3,N_vam(RealUb,uniformUncUpperBnds)}
		},
	kw_195[6] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,weibullUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(RealLb,weibullUncBetas)},
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(caulbl,CAUVar_weibull)},
		{"wuv_alphas",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,weibullUncAlphas)},
		{"wuv_betas",6,0,2,2,0,0.,0.,-3,N_vam(RealLb,weibullUncBetas)},
		{"wuv_descriptors",7,0,3,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_weibull)}
		},
	kw_196[29] = {
		{"beta_uncertain",0x19,10,12,0,kw_167,0.,0.,0,N_vam(intz,numBetaUncVars)},
		{"binomial_uncertain",0x19,3,19,0,kw_168,0.,0.,0,N_vam(intz,numBinomialUncVars)},
		{"continuous_design",0x19,12,2,0,kw_169,0.,0.,0,N_vam(intz,numContinuousDesVars)},
		{"continuous_state",0x19,8,26,0,kw_170,0.,0.,0,N_vam(intz,numContinuousStateVars)},
		{"discrete_design_range",0x19,8,3,0,kw_171,0.,0.,0,N_vam(intz,numDiscreteDesRangeVars)},
		{"discrete_design_set_integer",0x19,4,4,0,kw_172,0.,0.,0,N_vam(intz,numDiscreteDesSetIntVars)},
		{"discrete_design_set_real",0x19,4,5,0,kw_173,0.,0.,0,N_vam(intz,numDiscreteDesSetRealVars)},
		{"discrete_state_range",0x19,8,27,0,kw_174,0.,0.,0,N_vam(intz,numDiscreteStateRangeVars)},
		{"discrete_state_set_integer",0x19,4,28,0,kw_175,0.,0.,0,N_vam(intz,numDiscreteStateSetIntVars)},
		{"discrete_state_set_real",0x19,4,29,0,kw_176,0.,0.,0,N_vam(intz,numDiscreteStateSetRealVars)},
		{"exponential_uncertain",0x19,4,11,0,kw_177,0.,0.,0,N_vam(intz,numExponentialUncVars)},
		{"frechet_uncertain",0x19,6,15,0,kw_178,0.,0.,0,N_vam(intz,numFrechetUncVars)},
		{"gamma_uncertain",0x19,6,13,0,kw_179,0.,0.,0,N_vam(intz,numGammaUncVars)},
		{"geometric_uncertain",0x19,2,21,0,kw_180,0.,0.,0,N_vam(intz,numGeometricUncVars)},
		{"gumbel_uncertain",0x19,6,14,0,kw_181,0.,0.,0,N_vam(intz,numGumbelUncVars)},
		{"histogram_bin_uncertain",0x19,10,17,0,kw_182,0.,0.,0,N_vam(intz,numHistogramBinUncVars)},
		{"histogram_point_uncertain",0x19,8,23,0,kw_183,0.,0.,0,N_vam(intz,numHistogramPtUncVars)},
		{"hypergeometric_uncertain",0x19,4,22,0,kw_184,0.,0.,0,N_vam(intz,numHyperGeomUncVars)},
		{"id_variables",11,0,1,0,0,0.,0.,0,N_vam(str,idVariables)},
		{"interval_uncertain",0x19,8,25,0,kw_185,0.,0.,0,N_vam(intz,numIntervalUncVars)},
		{"lognormal_uncertain",0x19,10,7,0,kw_188,0.,0.,0,N_vam(intz,numLognormalUncVars)},
		{"loguniform_uncertain",0x19,6,9,0,kw_189,0.,0.,0,N_vam(intz,numLoguniformUncVars)},
		{"negative_binomial_uncertain",0x19,3,20,0,kw_190,0.,0.,0,N_vam(intz,numNegBinomialUncVars)},
		{"normal_uncertain",0x19,10,6,0,kw_191,0.,0.,0,N_vam(intz,numNormalUncVars)},
		{"poisson_uncertain",0x19,2,18,0,kw_192,0.,0.,0,N_vam(intz,numPoissonUncVars)},
		{"triangular_uncertain",0x19,8,10,0,kw_193,0.,0.,0,N_vam(intz,numTriangularUncVars)},
		{"uncertain_correlation_matrix",14,0,24,0,0,0.,0.,0,N_vam(vrl,Var_Info_ucm)},
		{"uniform_uncertain",0x19,6,8,0,kw_194,0.,0.,0,N_vam(intz,numUniformUncVars)},
		{"weibull_uncertain",0x19,6,16,0,kw_195,0.,0.,0,N_vam(intz,numWeibullUncVars)}
		},
	kw_197[6] = {
		{"interface",0x308,10,5,5,kw_9,0.,0.,0,N_ifm3(start,0,stop)},
		{"method",0x308,65,2,2,kw_115,0.,0.,0,N_mdm3(start,0,stop)},
		{"model",8,6,3,3,kw_138,0.,0.,0,N_mom3(start,0,stop)},
		{"responses",0x308,15,6,6,kw_155,0.,0.,0,N_rem3(start,0,stop)},
		{"strategy",0x108,10,1,1,kw_166,0.,0.,0,NIDRProblemDescDB::strategy_start},
		{"variables",0x308,29,4,4,kw_196,0.,0.,0,N_vam3(start,0,stop)}
		};

} // namespace Dakota

KeyWord Dakota_Keyword_Top = {"KeywordTop",0,6,0,0,Dakota::kw_197};

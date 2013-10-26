
namespace Dakota {

/** 1175 distinct keywords (plus 167 aliases) **/

static KeyWord
	kw_1[3] = {
		{"active_set_vector",8,0,1,0,0,0.,0.,0,N_ifm(false,activeSetVectorFlag)},
		{"evaluation_cache",8,0,2,0,0,0.,0.,0,N_ifm(false,evalCacheFlag)},
		{"restart_file",8,0,3,0,0,0.,0.,0,N_ifm(false,restartFileFlag)}
		},
	kw_2[1] = {
		{"processors_per_analysis",0x19,0,1,0,0,0.,0.,0,N_ifm(pint,procsPerAnalysis)}
		},
	kw_3[4] = {
		{"abort",8,0,1,1,0,0.,0.,0,N_ifm(lit,failAction_abort)},
		{"continuation",8,0,1,1,0,0.,0.,0,N_ifm(lit,failAction_continuation)},
		{"recover",14,0,1,1,0,0.,0.,0,N_ifm(Rlit,3failAction_recover)},
		{"retry",9,0,1,1,0,0.,0.,0,N_ifm(ilit,3failAction_retry)}
		},
	kw_4[1] = {
		{"numpy",8,0,1,0,0,0.,0.,0,N_ifm(true,numpyFlag)}
		},
	kw_5[2] = {
		{"copy",8,0,1,0,0,0.,0.,0,N_ifm(true,templateCopy)},
		{"replace",8,0,2,0,0,0.,0.,0,N_ifm(true,templateReplace)}
		},
	kw_6[7] = {
		{"dir_save",0,0,3,0,0,0.,0.,2,N_ifm(true,dirSave)},
		{"dir_tag",0,0,2,0,0,0.,0.,2,N_ifm(true,dirTag)},
		{"directory_save",8,0,3,0,0,0.,0.,0,N_ifm(true,dirSave)},
		{"directory_tag",8,0,2,0,0,0.,0.,0,N_ifm(true,dirTag)},
		{"named",11,0,1,0,0,0.,0.,0,N_ifm(str,workDir)},
		{"template_directory",11,2,4,0,kw_5,0.,0.,0,N_ifm(str,templateDir)},
		{"template_files",15,2,4,0,kw_5,0.,0.,0,N_ifm(strL,templateFiles)}
		},
	kw_7[8] = {
		{"allow_existing_results",8,0,3,0,0,0.,0.,0,N_ifm(true,allowExistingResultsFlag)},
		{"aprepro",8,0,5,0,0,0.,0.,0,N_ifm(true,apreproFlag)},
		{"file_save",8,0,7,0,0,0.,0.,0,N_ifm(true,fileSaveFlag)},
		{"file_tag",8,0,6,0,0,0.,0.,0,N_ifm(true,fileTagFlag)},
		{"parameters_file",11,0,1,0,0,0.,0.,0,N_ifm(str,parametersFile)},
		{"results_file",11,0,2,0,0,0.,0.,0,N_ifm(str,resultsFile)},
		{"verbatim",8,0,4,0,0,0.,0.,0,N_ifm(true,verbatimFlag)},
		{"work_directory",8,7,8,0,kw_6,0.,0.,0,N_ifm(true,useWorkdir)}
		},
	kw_8[12] = {
		{"analysis_components",15,0,1,0,0,0.,0.,0,N_ifm(str2D,analysisComponents)},
		{"deactivate",8,3,6,0,kw_1},
		{"direct",8,1,4,1,kw_2,0.,0.,0,N_ifm(lit,interfaceType_direct)},
		{"failure_capture",8,4,5,0,kw_3},
		{"fork",8,8,4,1,kw_7,0.,0.,0,N_ifm(lit,interfaceType_fork)},
		{"grid",8,0,4,1,0,0.,0.,0,N_ifm(lit,interfaceType_grid)},
		{"input_filter",11,0,2,0,0,0.,0.,0,N_ifm(str,inputFilter)},
		{"matlab",8,0,4,1,0,0.,0.,0,N_ifm(lit,interfaceType_matlab)},
		{"output_filter",11,0,3,0,0,0.,0.,0,N_ifm(str,outputFilter)},
		{"python",8,1,4,1,kw_4,0.,0.,0,N_ifm(lit,interfaceType_python)},
		{"scilab",8,0,4,1,0,0.,0.,0,N_ifm(lit,interfaceType_scilab)},
		{"system",8,8,4,1,kw_7,0.,0.,0,N_ifm(lit,interfaceType_system)}
		},
	kw_9[2] = {
		{"master",8,0,1,1,0,0.,0.,0,N_ifm(lit,analysisScheduling_master)},
		{"peer",8,0,1,1,0,0.,0.,0,N_ifm(lit,analysisScheduling_peer)}
		},
	kw_10[2] = {
		{"dynamic",8,0,1,1,0,0.,0.,0,N_ifm(lit,asynchLocalEvalScheduling_dynamic)},
		{"static",8,0,1,1,0,0.,0.,0,N_ifm(lit,asynchLocalEvalScheduling_static)}
		},
	kw_11[3] = {
		{"analysis_concurrency",0x19,0,3,0,0,0.,0.,0,N_ifm(pint,asynchLocalAnalysisConcurrency)},
		{"evaluation_concurrency",0x19,0,1,0,0,0.,0.,0,N_ifm(pint,asynchLocalEvalConcurrency)},
		{"local_evaluation_scheduling",8,2,2,0,kw_10}
		},
	kw_12[2] = {
		{"dynamic",8,0,1,1,0,0.,0.,0,N_ifm(lit,evalScheduling_peer_dynamic)},
		{"static",8,0,1,1,0,0.,0.,0,N_ifm(lit,evalScheduling_peer_static)}
		},
	kw_13[2] = {
		{"master",8,0,1,1,0,0.,0.,0,N_ifm(lit,evalScheduling_master)},
		{"peer",8,2,1,1,kw_12}
		},
	kw_14[8] = {
		{"algebraic_mappings",11,0,2,0,0,0.,0.,0,N_ifm(str,algebraicMappings)},
		{"analysis_drivers",15,12,3,0,kw_8,0.,0.,0,N_ifm(strL,analysisDrivers)},
		{"analysis_scheduling",8,2,8,0,kw_9},
		{"analysis_servers",0x19,0,7,0,0,0.,0.,0,N_ifm(pint,analysisServers)},
		{"asynchronous",8,3,4,0,kw_11,0.,0.,0,N_ifm(lit,interfaceSynchronization_asynchronous)},
		{"evaluation_scheduling",8,2,6,0,kw_13},
		{"evaluation_servers",0x19,0,5,0,0,0.,0.,0,N_ifm(pint,evalServers)},
		{"id_interface",11,0,1,0,0,0.,0.,0,N_ifm(str,idInterface)}
		},
	kw_15[1] = {
		{"seed",0x19,0,1,0,0,0.,0.,0,N_mdm(pint,randomSeed)}
		},
	kw_16[1] = {
		{"samples",9,0,1,0,0,0.,0.,0,N_mdm(int,numSamples)}
		},
	kw_17[2] = {
		{"complementary",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_COMPLEMENTARY)},
		{"cumulative",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_CUMULATIVE)}
		},
	kw_18[1] = {
		{"num_gen_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,genReliabilityLevels)}
		},
	kw_19[1] = {
		{"num_probability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,probabilityLevels)}
		},
	kw_20[2] = {
		{"mt19937",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_mt19937)},
		{"rnum2",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_rnum2)}
		},
	kw_21[4] = {
		{"distribution",8,2,1,0,kw_17},
		{"gen_reliability_levels",14,1,3,0,kw_18,0.,0.,0,N_mdm(resplevs,genReliabilityLevels)},
		{"probability_levels",14,1,2,0,kw_19,0.,0.,0,N_mdm(resplevs01,probabilityLevels)},
		{"rng",8,2,4,0,kw_20}
		},
	kw_22[4] = {
		{"constant_liar",8,0,1,1,0,0.,0.,0,N_mdm(lit,batchSelectionType_constant_liar)},
		{"distance_penalty",8,0,1,1,0,0.,0.,0,N_mdm(lit,batchSelectionType_distance_penalty)},
		{"naive",8,0,1,1,0,0.,0.,0,N_mdm(lit,batchSelectionType_naive)},
		{"topology",8,0,1,1,0,0.,0.,0,N_mdm(lit,batchSelectionType_topology)}
		},
	kw_23[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_24[3] = {
		{"distance",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessMetricType_distance)},
		{"gradient",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessMetricType_gradient)},
		{"predicted_variance",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessMetricType_predicted_variance)}
		},
	kw_25[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_26[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_27[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_26}
		},
	kw_28[2] = {
		{"compute",8,3,2,0,kw_27},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_29[11] = {
		{0,0,1,0,0,kw_15},
		{0,0,1,0,0,kw_16},
		{0,0,4,0,0,kw_21},
		{"batch_selection",8,4,3,0,kw_22},
		{"batch_size",9,0,4,0,0,0.,0.,0,N_mdm(int,batchSize)},
		{"emulator_samples",9,0,1,0,0,0.,0.,0,N_mdm(int,emulatorSamples)},
		{"export_points_file",11,2,6,0,kw_23,0.,0.,0,N_mdm(str,approxExportFile)},
		{"fitness_metric",8,3,2,0,kw_24},
		{"import_points_file",11,2,5,0,kw_25,0.,0.,0,N_mdm(str,approxImportFile)},
		{"misc_options",15,0,8,0,0,0.,0.,0,N_mdm(strL,miscOptions)},
		{"response_levels",14,2,7,0,kw_28,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_30[9] = {
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
	kw_31[7] = {
		{"merit1",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit1)},
		{"merit1_smooth",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit1_smooth)},
		{"merit2",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit2)},
		{"merit2_smooth",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit2_smooth)},
		{"merit2_squared",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit2_squared)},
		{"merit_max",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit_max)},
		{"merit_max_smooth",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit_max_smooth)}
		},
	kw_32[2] = {
		{"blocking",8,0,1,1,0,0.,0.,0,N_mdm(lit,evalSynchronize_blocking)},
		{"nonblocking",8,0,1,1,0,0.,0.,0,N_mdm(lit,evalSynchronize_nonblocking)}
		},
	kw_33[10] = {
		{0,0,9,0,0,kw_30},
		{"constraint_penalty",10,0,7,0,0,0.,0.,0,N_mdm(Real,constrPenalty)},
		{"contraction_factor",10,0,2,0,0,0.,0.,0,N_mdm(Real,contractStepLength)},
		{"initial_delta",10,0,1,0,0,0.,0.,0,N_mdm(Real,initStepLength)},
		{"merit_function",8,7,6,0,kw_31},
		{"smoothing_factor",10,0,8,0,0,0.,0.,0,N_mdm(Real,smoothFactor)},
		{"solution_accuracy",2,0,4,0,0,0.,0.,1,N_mdm(Real,solnTarget)},
		{"solution_target",10,0,4,0,0,0.,0.,0,N_mdm(Real,solnTarget)},
		{"synchronization",8,2,5,0,kw_32},
		{"threshold_delta",10,0,3,0,0,0.,0.,0,N_mdm(Real,threshStepLength)}
		},
	kw_34[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_35[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_36[5] = {
		{"dakota",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_GAUSSIAN_PROCESS)},
		{"emulator_samples",9,0,2,0,0,0.,0.,0,N_mdm(int,emulatorSamples)},
		{"export_points_file",11,2,4,0,kw_34,0.,0.,0,N_mdm(str,approxExportFile)},
		{"import_points_file",11,2,3,0,kw_35,0.,0.,0,N_mdm(str,approxImportFile)},
		{"surfpack",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING)}
		},
	kw_37[1] = {
		{"sparse_grid_level",13,0,1,0,0,0.,0.,0,N_mdm(usharray,sparseGridLevel)}
		},
	kw_38[1] = {
		{"sparse_grid_level",13,0,1,0,0,0.,0.,0,N_mdm(usharray,sparseGridLevel)}
		},
	kw_39[4] = {
		{"gaussian_process",8,5,1,1,kw_36},
		{"kriging",0,5,1,1,kw_36,0.,0.,-1},
		{"pce",8,1,1,1,kw_37,0.,0.,0,N_mdm(type,emulatorType_POLYNOMIAL_CHAOS)},
		{"sc",8,1,1,1,kw_38,0.,0.,0,N_mdm(type,emulatorType_STOCHASTIC_COLLOCATION)}
		},
	kw_40[6] = {
		{"chains",0x29,0,1,0,0,3.,0.,0,N_mdm(int,numChains)},
		{"crossover_chain_pairs",0x29,0,3,0,0,0.,0.,0,N_mdm(int,crossoverChainPairs)},
		{"emulator",8,4,6,0,kw_39},
		{"gr_threshold",0x1a,0,4,0,0,0.,0.,0,N_mdm(Real,grThreshold)},
		{"jump_step",0x29,0,5,0,0,0.,0.,0,N_mdm(int,jumpStep)},
		{"num_cr",0x29,0,2,0,0,1.,0.,0,N_mdm(int,numCR)}
		},
	kw_41[2] = {
		{"adaptive",8,0,1,1,0,0.,0.,0,N_mdm(lit,metropolisType_adaptive)},
		{"hastings",8,0,1,1,0,0.,0.,0,N_mdm(lit,metropolisType_hastings)}
		},
	kw_42[2] = {
		{"delayed",8,0,1,1,0,0.,0.,0,N_mdm(lit,rejectionType_delayed)},
		{"standard",8,0,1,1,0,0.,0.,0,N_mdm(lit,rejectionType_standard)}
		},
	kw_43[2] = {
		{"mt19937",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_mt19937)},
		{"rnum2",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_rnum2)}
		},
	kw_44[4] = {
		{"metropolis",8,2,2,0,kw_41},
		{"proposal_covariance_scale",14,0,4,0,0,0.,0.,0,N_mdm(RealDL,proposalCovScale)},
		{"rejection",8,2,1,0,kw_42},
		{"rng",8,2,3,0,kw_43}
		},
	kw_45[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_46[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_47[4] = {
		{0,0,4,0,0,kw_44},
		{"emulator_samples",9,0,1,1,0,0.,0.,0,N_mdm(int,emulatorSamples)},
		{"export_points_file",11,2,3,0,kw_45,0.,0.,0,N_mdm(str,approxExportFile)},
		{"import_points_file",11,2,2,0,kw_46,0.,0.,0,N_mdm(str,approxImportFile)}
		},
	kw_48[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_49[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_50[5] = {
		{"dakota",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_GAUSSIAN_PROCESS)},
		{"emulator_samples",9,0,2,0,0,0.,0.,0,N_mdm(int,emulatorSamples)},
		{"export_points_file",11,2,4,0,kw_48,0.,0.,0,N_mdm(str,approxExportFile)},
		{"import_points_file",11,2,3,0,kw_49,0.,0.,0,N_mdm(str,approxImportFile)},
		{"surfpack",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING)}
		},
	kw_51[1] = {
		{"sparse_grid_level",13,0,1,0,0,0.,0.,0,N_mdm(usharray,sparseGridLevel)}
		},
	kw_52[1] = {
		{"sparse_grid_level",13,0,1,0,0,0.,0.,0,N_mdm(usharray,sparseGridLevel)}
		},
	kw_53[4] = {
		{"gaussian_process",8,5,1,1,kw_50},
		{"kriging",0,5,1,1,kw_50,0.,0.,-1},
		{"pce",8,1,1,1,kw_51,0.,0.,0,N_mdm(type,emulatorType_POLYNOMIAL_CHAOS)},
		{"sc",8,1,1,1,kw_52,0.,0.,0,N_mdm(type,emulatorType_STOCHASTIC_COLLOCATION)}
		},
	kw_54[2] = {
		{0,0,4,0,0,kw_44},
		{"emulator",8,4,1,0,kw_53}
		},
	kw_55[8] = {
		{0,0,1,0,0,kw_15},
		{0,0,1,0,0,kw_16},
		{"calibrate_sigma",8,0,4,0,0,0.,0.,0,N_mdm(true,calibrateSigmaFlag)},
		{"dream",8,6,1,1,kw_40,0.,0.,0,N_mdm(lit,subMethodName_dream)},
		{"gpmsa",8,3,1,1,kw_47,0.,0.,0,N_mdm(lit,subMethodName_gpmsa)},
		{"likelihood_scale",10,0,3,0,0,0.,0.,0,N_mdm(Real,likelihoodScale)},
		{"queso",8,1,1,1,kw_54,0.,0.,0,N_mdm(lit,subMethodName_queso)},
		{"use_derivatives",8,0,2,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)}
		},
	kw_56[3] = {
		{"deltas_per_variable",5,0,2,2,0,0.,0.,2,N_mdm(ivec,stepsPerVariable)},
		{"step_vector",14,0,1,1,0,0.,0.,0,N_mdm(RealDL,stepVector)},
		{"steps_per_variable",13,0,2,2,0,0.,0.,0,N_mdm(ivec,stepsPerVariable)}
		},
	kw_57[5] = {
		{"misc_options",15,0,4,0,0,0.,0.,0,N_mdm(strL,miscOptions)},
		{"seed",0x19,0,2,0,0,0.,0.,0,N_mdm(pint,randomSeed)},
		{"show_misc_options",8,0,3,0,0,0.,0.,0,N_mdm(true,showMiscOptions)},
		{"solution_accuracy",2,0,1,0,0,0.,0.,1,N_mdm(Real,solnTarget)},
		{"solution_target",10,0,1,0,0,0.,0.,0,N_mdm(Real,solnTarget)}
		},
	kw_58[2] = {
		{0,0,5,0,0,kw_57},
		{"beta_solver_name",11,0,1,1,0,0.,0.,0,N_mdm(str,betaSolverName)}
		},
	kw_59[2] = {
		{"initial_delta",10,0,1,0,0,0.,0.,0,N_mdm(Real,initDelta)},
		{"threshold_delta",10,0,2,0,0,0.,0.,0,N_mdm(Real,threshDelta)}
		},
	kw_60[3] = {
		{0,0,5,0,0,kw_57},
		{0,0,2,0,0,kw_59},
		{""}
		},
	kw_61[2] = {
		{"all_dimensions",8,0,1,1,0,0.,0.,0,N_mdm(lit,boxDivision_all_dimensions)},
		{"major_dimension",8,0,1,1,0,0.,0.,0,N_mdm(lit,boxDivision_major_dimension)}
		},
	kw_62[7] = {
		{0,0,5,0,0,kw_57},
		{"constraint_penalty",10,0,6,0,0,0.,0.,0,N_mdm(Real,constraintPenalty)},
		{"division",8,2,1,0,kw_61},
		{"global_balance_parameter",10,0,2,0,0,0.,0.,0,N_mdm(Real,globalBalanceParam)},
		{"local_balance_parameter",10,0,3,0,0,0.,0.,0,N_mdm(Real,localBalanceParam)},
		{"max_boxsize_limit",10,0,4,0,0,0.,0.,0,N_mdm(Real,maxBoxSize)},
		{"min_boxsize_limit",10,0,5,0,0,0.,0.,0,N_mdm(Real,minBoxSize)}
		},
	kw_63[3] = {
		{"blend",8,0,1,1,0,0.,0.,0,N_mdm(lit,crossoverType_blend)},
		{"two_point",8,0,1,1,0,0.,0.,0,N_mdm(lit,crossoverType_two_point)},
		{"uniform",8,0,1,1,0,0.,0.,0,N_mdm(lit,crossoverType_uniform)}
		},
	kw_64[2] = {
		{"linear_rank",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_linear_rank)},
		{"merit_function",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_proportional)}
		},
	kw_65[3] = {
		{"flat_file",11,0,1,1,0,0.,0.,0,N_mdm(slit2,3initializationType_flat_file)},
		{"simple_random",8,0,1,1,0,0.,0.,0,N_mdm(lit,initializationType_random)},
		{"unique_random",8,0,1,1,0,0.,0.,0,N_mdm(lit,initializationType_unique_random)}
		},
	kw_66[2] = {
		{"mutation_range",9,0,2,0,0,0.,0.,0,N_mdm(int,mutationRange)},
		{"mutation_scale",10,0,1,0,0,0.,0.,0,N_mdm(Real,mutationScale)}
		},
	kw_67[5] = {
		{"non_adaptive",8,0,2,0,0,0.,0.,0,N_mdm(false,mutationAdaptive)},
		{"offset_cauchy",8,2,1,1,kw_66,0.,0.,0,N_mdm(lit,mutationType_offset_cauchy)},
		{"offset_normal",8,2,1,1,kw_66,0.,0.,0,N_mdm(lit,mutationType_offset_normal)},
		{"offset_uniform",8,2,1,1,kw_66,0.,0.,0,N_mdm(lit,mutationType_offset_uniform)},
		{"replace_uniform",8,0,1,1,0,0.,0.,0,N_mdm(lit,mutationType_replace_uniform)}
		},
	kw_68[4] = {
		{"chc",9,0,1,1,0,0.,0.,0,N_mdm(ilit2,3replacementType_chc)},
		{"elitist",9,0,1,1,0,0.,0.,0,N_mdm(ilit2,3replacementType_elitist)},
		{"new_solutions_generated",9,0,2,0,0,0.,0.,0,N_mdm(int,newSolnsGenerated)},
		{"random",9,0,1,1,0,0.,0.,0,N_mdm(ilit2,3replacementType_random)}
		},
	kw_69[10] = {
		{0,0,5,0,0,kw_57},
		{"constraint_penalty",10,0,9,0,0,0.,0.,0,N_mdm(Real,constraintPenalty)},
		{"crossover_rate",10,0,5,0,0,0.,0.,0,N_mdm(Real,crossoverRate)},
		{"crossover_type",8,3,6,0,kw_63},
		{"fitness_type",8,2,3,0,kw_64},
		{"initialization_type",8,3,2,0,kw_65},
		{"mutation_rate",10,0,7,0,0,0.,0.,0,N_mdm(Real,mutationRate)},
		{"mutation_type",8,5,8,0,kw_67},
		{"population_size",0x19,0,1,0,0,0.,0.,0,N_mdm(pint,populationSize)},
		{"replacement_type",8,4,4,0,kw_68}
		},
	kw_70[2] = {
		{"constraint_penalty",10,0,2,0,0,0.,0.,0,N_mdm(Real,constraintPenalty)},
		{"contraction_factor",10,0,1,0,0,0.,0.,0,N_mdm(Real,contractFactor)}
		},
	kw_71[3] = {
		{"adaptive_pattern",8,0,1,1,0,0.,0.,0,N_mdm(lit,exploratoryMoves_adaptive)},
		{"basic_pattern",8,0,1,1,0,0.,0.,0,N_mdm(lit,exploratoryMoves_simple)},
		{"multi_step",8,0,1,1,0,0.,0.,0,N_mdm(lit,exploratoryMoves_multi_step)}
		},
	kw_72[2] = {
		{"coordinate",8,0,1,1,0,0.,0.,0,N_mdm(lit,patternBasis_coordinate)},
		{"simplex",8,0,1,1,0,0.,0.,0,N_mdm(lit,patternBasis_simplex)}
		},
	kw_73[2] = {
		{"blocking",8,0,1,1,0,0.,0.,0,N_mdm(lit,evalSynchronize_blocking)},
		{"nonblocking",8,0,1,1,0,0.,0.,0,N_mdm(lit,evalSynchronize_nonblocking)}
		},
	kw_74[11] = {
		{0,0,5,0,0,kw_57},
		{0,0,2,0,0,kw_59},
		{0,0,2,0,0,kw_70},
		{"constant_penalty",8,0,1,0,0,0.,0.,0,N_mdm(true,constantPenalty)},
		{"expand_after_success",9,0,3,0,0,0.,0.,0,N_mdm(int,expandAfterSuccess)},
		{"exploratory_moves",8,3,7,0,kw_71},
		{"no_expansion",8,0,2,0,0,0.,0.,0,N_mdm(false,expansionFlag)},
		{"pattern_basis",8,2,4,0,kw_72},
		{"stochastic",8,0,5,0,0,0.,0.,0,N_mdm(true,randomizeOrderFlag)},
		{"synchronization",8,2,8,0,kw_73},
		{"total_pattern_size",9,0,6,0,0,0.,0.,0,N_mdm(int,totalPatternSize)}
		},
	kw_75[7] = {
		{0,0,5,0,0,kw_57},
		{0,0,2,0,0,kw_59},
		{0,0,2,0,0,kw_70},
		{"constant_penalty",8,0,4,0,0,0.,0.,0,N_mdm(true,constantPenalty)},
		{"contract_after_failure",9,0,1,0,0,0.,0.,0,N_mdm(int,contractAfterFail)},
		{"expand_after_success",9,0,3,0,0,0.,0.,0,N_mdm(int,expandAfterSuccess)},
		{"no_expansion",8,0,2,0,0,0.,0.,0,N_mdm(false,expansionFlag)}
		},
	kw_76[3] = {
		{0,0,9,0,0,kw_30},
		{"frcg",8,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_conmin_frcg)},
		{"mfd",8,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_conmin_mfd)}
		},
	kw_77[1] = {
		{"drop_tolerance",10,0,1,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)}
		},
	kw_78[14] = {
		{0,0,1,0,0,kw_15},
		{0,0,1,0,0,kw_16},
		{"box_behnken",8,0,1,1,0,0.,0.,0,N_mdm(lit,subMethodName_box_behnken)},
		{"central_composite",8,0,1,1,0,0.,0.,0,N_mdm(lit,subMethodName_central_composite)},
		{"fixed_seed",8,0,5,0,0,0.,0.,0,N_mdm(true,fixedSeedFlag)},
		{"grid",8,0,1,1,0,0.,0.,0,N_mdm(lit,subMethodName_grid)},
		{"lhs",8,0,1,1,0,0.,0.,0,N_mdm(lit,subMethodName_lhs)},
		{"main_effects",8,0,2,0,0,0.,0.,0,N_mdm(true,mainEffectsFlag)},
		{"oa_lhs",8,0,1,1,0,0.,0.,0,N_mdm(lit,subMethodName_oa_lhs)},
		{"oas",8,0,1,1,0,0.,0.,0,N_mdm(lit,subMethodName_oas)},
		{"quality_metrics",8,0,3,0,0,0.,0.,0,N_mdm(true,volQualityFlag)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(lit,subMethodName_random)},
		{"symbols",9,0,6,0,0,0.,0.,0,N_mdm(int,numSymbols)},
		{"variance_based_decomp",8,1,4,0,kw_77,0.,0.,0,N_mdm(true,vbdFlag)}
		},
	kw_79[6] = {
		{0,0,9,0,0,kw_30},
		{"bfgs",8,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_dot_bfgs)},
		{"frcg",8,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_dot_frcg)},
		{"mmfd",8,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_dot_mmfd)},
		{"slp",8,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_dot_slp)},
		{"sqp",8,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_dot_sqp)}
		},
	kw_80[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_81[2] = {
		{"dakota",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_GAUSSIAN_PROCESS)},
		{"surfpack",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING)}
		},
	kw_82[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_83[6] = {
		{0,0,1,0,0,kw_15},
		{"export_points_file",11,2,4,0,kw_80,0.,0.,0,N_mdm(str,approxExportFile)},
		{"gaussian_process",8,2,1,0,kw_81},
		{"import_points_file",11,2,3,0,kw_82,0.,0.,0,N_mdm(str,approxImportFile)},
		{"kriging",0,2,1,0,kw_81,0.,0.,-2},
		{"use_derivatives",8,0,2,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)}
		},
	kw_84[5] = {
		{0,0,1,0,0,kw_15},
		{0,0,1,0,0,kw_16},
		{0,0,4,0,0,kw_21},
		{"batch_size",9,0,2,0,0,0.,0.,0,N_mdm(int,batchSize)},
		{"emulator_samples",9,0,1,0,0,0.,0.,0,N_mdm(int,emulatorSamples)}
		},
	kw_85[3] = {
		{"grid",8,0,1,1,0,0.,0.,0,N_mdm(lit,trialType_grid)},
		{"halton",8,0,1,1,0,0.,0.,0,N_mdm(lit,trialType_halton)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(lit,trialType_random)}
		},
	kw_86[1] = {
		{"drop_tolerance",10,0,1,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)}
		},
	kw_87[8] = {
		{0,0,1,0,0,kw_15},
		{0,0,1,0,0,kw_16},
		{"fixed_seed",8,0,4,0,0,0.,0.,0,N_mdm(true,fixedSeedFlag)},
		{"latinize",8,0,1,0,0,0.,0.,0,N_mdm(true,latinizeFlag)},
		{"num_trials",9,0,6,0,0,0.,0.,0,N_mdm(int,numTrials)},
		{"quality_metrics",8,0,2,0,0,0.,0.,0,N_mdm(true,volQualityFlag)},
		{"trial_type",8,3,5,0,kw_85},
		{"variance_based_decomp",8,1,3,0,kw_86,0.,0.,0,N_mdm(true,vbdFlag)}
		},
	kw_88[1] = {
		{"drop_tolerance",10,0,1,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)}
		},
	kw_89[10] = {
		{"fixed_sequence",8,0,6,0,0,0.,0.,0,N_mdm(true,fixedSequenceFlag)},
		{"halton",8,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_fsu_halton)},
		{"hammersley",8,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_fsu_hammersley)},
		{"latinize",8,0,2,0,0,0.,0.,0,N_mdm(true,latinizeFlag)},
		{"prime_base",13,0,9,0,0,0.,0.,0,N_mdm(ivec,primeBase)},
		{"quality_metrics",8,0,3,0,0,0.,0.,0,N_mdm(true,volQualityFlag)},
		{"samples",9,0,5,0,0,0.,0.,0,N_mdm(int,numSamples)},
		{"sequence_leap",13,0,8,0,0,0.,0.,0,N_mdm(ivec,sequenceLeap)},
		{"sequence_start",13,0,7,0,0,0.,0.,0,N_mdm(ivec,sequenceStart)},
		{"variance_based_decomp",8,1,4,0,kw_88,0.,0.,0,N_mdm(true,vbdFlag)}
		},
	kw_90[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_91[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_92[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_93[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_92}
		},
	kw_94[2] = {
		{"compute",8,3,2,0,kw_93},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_95[7] = {
		{0,0,1,0,0,kw_15},
		{0,0,1,0,0,kw_16},
		{0,0,4,0,0,kw_21},
		{"emulator_samples",9,0,1,0,0,0.,0.,0,N_mdm(int,emulatorSamples)},
		{"export_points_file",11,2,3,0,kw_90,0.,0.,0,N_mdm(str,approxExportFile)},
		{"import_points_file",11,2,2,0,kw_91,0.,0.,0,N_mdm(str,approxImportFile)},
		{"response_levels",14,2,4,0,kw_94,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_96[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_97[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_96}
		},
	kw_98[2] = {
		{"compute",8,3,2,0,kw_97},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_99[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_100[2] = {
		{"dakota",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_GAUSSIAN_PROCESS)},
		{"surfpack",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING)}
		},
	kw_101[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_102[5] = {
		{"export_points_file",11,2,4,0,kw_99,0.,0.,0,N_mdm(str,approxExportFile)},
		{"gaussian_process",8,2,1,0,kw_100},
		{"import_points_file",11,2,3,0,kw_101,0.,0.,0,N_mdm(str,approxImportFile)},
		{"kriging",0,2,1,0,kw_100,0.,0.,-2},
		{"use_derivatives",8,0,2,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)}
		},
	kw_103[8] = {
		{0,0,1,0,0,kw_15},
		{0,0,1,0,0,kw_16},
		{0,0,4,0,0,kw_21},
		{"ea",8,0,1,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_ea)},
		{"ego",8,5,1,0,kw_102,0.,0.,0,N_mdm(lit,nondOptAlgorithm_ego)},
		{"lhs",8,0,1,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_lhs)},
		{"response_levels",14,2,2,0,kw_98,0.,0.,0,N_mdm(resplevs,responseLevels)},
		{"sbo",8,5,1,0,kw_102,0.,0.,0,N_mdm(lit,nondOptAlgorithm_sbo)}
		},
	kw_104[2] = {
		{"mt19937",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_mt19937)},
		{"rnum2",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_rnum2)}
		},
	kw_105[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_106[2] = {
		{"dakota",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_GAUSSIAN_PROCESS)},
		{"surfpack",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING)}
		},
	kw_107[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_108[5] = {
		{"export_points_file",11,2,4,0,kw_105,0.,0.,0,N_mdm(str,approxExportFile)},
		{"gaussian_process",8,2,1,0,kw_106},
		{"import_points_file",11,2,3,0,kw_107,0.,0.,0,N_mdm(str,approxImportFile)},
		{"kriging",0,2,1,0,kw_106,0.,0.,-2},
		{"use_derivatives",8,0,2,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)}
		},
	kw_109[7] = {
		{0,0,1,0,0,kw_15},
		{0,0,1,0,0,kw_16},
		{"ea",8,0,1,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_ea)},
		{"ego",8,5,1,0,kw_108,0.,0.,0,N_mdm(lit,nondOptAlgorithm_ego)},
		{"lhs",8,0,1,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_lhs)},
		{"rng",8,2,2,0,kw_104},
		{"sbo",8,5,1,0,kw_108,0.,0.,0,N_mdm(lit,nondOptAlgorithm_sbo)}
		},
	kw_110[2] = {
		{"complementary",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_COMPLEMENTARY)},
		{"cumulative",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_CUMULATIVE)}
		},
	kw_111[1] = {
		{"num_gen_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,genReliabilityLevels)}
		},
	kw_112[1] = {
		{"num_probability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,probabilityLevels)}
		},
	kw_113[3] = {
		{"distribution",8,2,1,0,kw_110},
		{"gen_reliability_levels",14,1,3,0,kw_111,0.,0.,0,N_mdm(resplevs,genReliabilityLevels)},
		{"probability_levels",14,1,2,0,kw_112,0.,0.,0,N_mdm(resplevs01,probabilityLevels)}
		},
	kw_114[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_115[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_116[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_117[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_116}
		},
	kw_118[2] = {
		{"compute",8,3,2,0,kw_117},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_119[2] = {
		{"mt19937",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_mt19937)},
		{"rnum2",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_rnum2)}
		},
	kw_120[2] = {
		{"dakota",8,0,1,0,0,0.,0.,0,N_mdm(type,emulatorType_GAUSSIAN_PROCESS)},
		{"surfpack",8,0,1,0,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING)}
		},
	kw_121[11] = {
		{0,0,3,0,0,kw_113},
		{"export_points_file",11,2,3,0,kw_114,0.,0.,0,N_mdm(str,approxExportFile)},
		{"import_points_file",11,2,2,0,kw_115,0.,0.,0,N_mdm(str,approxImportFile)},
		{"response_levels",14,2,7,0,kw_118,0.,0.,0,N_mdm(resplevs,responseLevels)},
		{"rng",8,2,6,0,kw_119},
		{"seed",0x19,0,5,0,0,0.,0.,0,N_mdm(pint,randomSeed)},
		{"u_gaussian_process",8,2,1,1,kw_120,0.,0.,0,N_mdm(lit,reliabilitySearchType_egra_u)},
		{"u_kriging",0,0,1,1,0,0.,0.,-1,N_mdm(lit,reliabilitySearchType_egra_u)},
		{"use_derivatives",8,0,4,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)},
		{"x_gaussian_process",8,2,1,1,kw_120,0.,0.,0,N_mdm(lit,reliabilitySearchType_egra_x)},
		{"x_kriging",0,2,1,1,kw_120,0.,0.,-1,N_mdm(lit,reliabilitySearchType_egra_x)}
		},
	kw_122[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_123[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_122}
		},
	kw_124[2] = {
		{"compute",8,3,2,0,kw_123},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_125[7] = {
		{0,0,1,0,0,kw_15},
		{0,0,1,0,0,kw_16},
		{0,0,4,0,0,kw_21},
		{"adapt_import",8,0,1,0,0,0.,0.,0,N_mdm(lit,integrationRefine_ais)},
		{"import",8,0,1,0,0,0.,0.,0,N_mdm(lit,integrationRefine_is)},
		{"mm_adapt_import",8,0,1,0,0,0.,0.,0,N_mdm(lit,integrationRefine_mmais)},
		{"response_levels",14,2,2,0,kw_124,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_126[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,pstudyFileAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,pstudyFileAnnotated)}
		},
	kw_127[2] = {
		{"import_points_file",11,2,1,1,kw_126,0.,0.,0,N_mdm(str,pstudyFilename)},
		{"list_of_points",14,0,1,1,0,0.,0.,0,N_mdm(RealDL,listOfPoints)}
		},
	kw_128[2] = {
		{"complementary",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_COMPLEMENTARY)},
		{"cumulative",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_CUMULATIVE)}
		},
	kw_129[1] = {
		{"num_gen_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,genReliabilityLevels)}
		},
	kw_130[1] = {
		{"num_probability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,probabilityLevels)}
		},
	kw_131[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_132[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_131}
		},
	kw_133[2] = {
		{"compute",8,3,2,0,kw_132},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_134[6] = {
		{"distribution",8,2,5,0,kw_128},
		{"gen_reliability_levels",14,1,4,0,kw_129,0.,0.,0,N_mdm(resplevs,genReliabilityLevels)},
		{"nip",8,0,1,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_nip)},
		{"probability_levels",14,1,3,0,kw_130,0.,0.,0,N_mdm(resplevs01,probabilityLevels)},
		{"response_levels",14,2,2,0,kw_133,0.,0.,0,N_mdm(resplevs,responseLevels)},
		{"sqp",8,0,1,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_sqp)}
		},
	kw_135[2] = {
		{"nip",8,0,1,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_nip)},
		{"sqp",8,0,1,0,0,0.,0.,0,N_mdm(lit,nondOptAlgorithm_sqp)}
		},
	kw_136[5] = {
		{"adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(lit,integrationRefine_ais)},
		{"import",8,0,1,1,0,0.,0.,0,N_mdm(lit,integrationRefine_is)},
		{"mm_adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(lit,integrationRefine_mmais)},
		{"samples",9,0,2,0,0,0.,0.,0,N_mdm(int,numSamples)},
		{"seed",0x19,0,3,0,0,0.,0.,0,N_mdm(pint,randomSeed)}
		},
	kw_137[3] = {
		{"first_order",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilityIntegration_first_order)},
		{"sample_refinement",8,5,2,0,kw_136},
		{"second_order",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilityIntegration_second_order)}
		},
	kw_138[10] = {
		{"integration",8,3,3,0,kw_137},
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
	kw_139[1] = {
		{"num_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,reliabilityLevels)}
		},
	kw_140[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_141[4] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_RELIABILITIES)},
		{"system",8,2,2,0,kw_140}
		},
	kw_142[2] = {
		{"compute",8,4,2,0,kw_141},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_143[4] = {
		{0,0,3,0,0,kw_113},
		{"mpp_search",8,10,1,0,kw_138},
		{"reliability_levels",14,1,3,0,kw_139,0.,0.,0,N_mdm(resplevs,reliabilityLevels)},
		{"response_levels",14,2,2,0,kw_142,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_144[7] = {
		{0,0,9,0,0,kw_30},
		{"display_all_evaluations",8,0,6,0,0,0.,0.,0,N_mdm(true,showAllEval)},
		{"display_format",11,0,4,0,0,0.,0.,0,N_mdm(str,displayFormat)},
		{"function_precision",10,0,1,0,0,0.,0.,0,N_mdm(Real,functionPrecision)},
		{"history_file",11,0,3,0,0,0.,0.,0,N_mdm(str,historyFile)},
		{"seed",0x19,0,2,0,0,0.,0.,0,N_mdm(pint,randomSeed)},
		{"variable_neighborhood_search",10,0,5,0,0,0.,0.,0,N_mdm(Real,vns)}
		},
	kw_145[2] = {
		{"num_offspring",0x19,0,2,0,0,0.,0.,0,N_mdm(pintz,numOffspring)},
		{"num_parents",0x19,0,1,0,0,0.,0.,0,N_mdm(pintz,numParents)}
		},
	kw_146[5] = {
		{"crossover_rate",10,0,2,0,0,0.,0.,0,N_mdm(litz,3crossoverType_null_crossover)},
		{"multi_point_binary",9,0,1,1,0,0.,0.,0,N_mdm(ilit2p,3crossoverType_multi_point_binary)},
		{"multi_point_parameterized_binary",9,0,1,1,0,0.,0.,0,N_mdm(ilit2p,3crossoverType_multi_point_parameterized_binary)},
		{"multi_point_real",9,0,1,1,0,0.,0.,0,N_mdm(ilit2p,3crossoverType_multi_point_real)},
		{"shuffle_random",8,2,1,1,kw_145,0.,0.,0,N_mdm(litc,3crossoverType_shuffle_random)}
		},
	kw_147[3] = {
		{"flat_file",11,0,1,1,0,0.,0.,0,N_mdm(slit2,3initializationType_flat_file)},
		{"simple_random",8,0,1,1,0,0.,0.,0,N_mdm(lit,initializationType_random)},
		{"unique_random",8,0,1,1,0,0.,0.,0,N_mdm(lit,initializationType_unique_random)}
		},
	kw_148[1] = {
		{"mutation_scale",10,0,1,0,0,0.,0.,0,N_mdm(Real01,mutationScale)}
		},
	kw_149[6] = {
		{"bit_random",8,0,1,1,0,0.,0.,0,N_mdm(lit,mutationType_bit_random)},
		{"mutation_rate",10,0,2,0,0,0.,0.,0,N_mdm(litz,3mutationType_null_mutation)},
		{"offset_cauchy",8,1,1,1,kw_148,0.,0.,0,N_mdm(litc,3mutationType_offset_cauchy)},
		{"offset_normal",8,1,1,1,kw_148,0.,0.,0,N_mdm(litc,3mutationType_offset_normal)},
		{"offset_uniform",8,1,1,1,kw_148,0.,0.,0,N_mdm(litc,3mutationType_offset_uniform)},
		{"replace_uniform",8,0,1,1,0,0.,0.,0,N_mdm(lit,mutationType_replace_uniform)}
		},
	kw_150[7] = {
		{"crossover_type",8,5,5,0,kw_146},
		{"initialization_type",8,3,4,0,kw_147},
		{"log_file",11,0,2,0,0,0.,0.,0,N_mdm(str,logFile)},
		{"mutation_type",8,6,6,0,kw_149},
		{"population_size",0x29,0,1,0,0,0.,0.,0,N_mdm(nnint,populationSize)},
		{"print_each_pop",8,0,3,0,0,0.,0.,0,N_mdm(true,printPopFlag)},
		{"seed",0x19,0,7,0,0,0.,0.,0,N_mdm(pint,randomSeed)}
		},
	kw_151[3] = {
		{"metric_tracker",8,0,1,1,0,0.,0.,0,N_mdm(lit,convergenceType_metric_tracker)},
		{"num_generations",0x29,0,3,0,0,0.,0.,0,N_mdm(sizet,numGenerations)},
		{"percent_change",10,0,2,0,0,0.,0.,0,N_mdm(Realz,convergenceTolerance)}
		},
	kw_152[2] = {
		{"domination_count",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_domination_count)},
		{"layer_rank",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_layer_rank)}
		},
	kw_153[1] = {
		{"num_designs",0x29,0,1,0,0,2.,0.,0,N_mdm(pintz,numDesigns)}
		},
	kw_154[3] = {
		{"distance",14,0,1,1,0,0.,0.,0,N_mdm(RealLlit,3nichingType_distance)},
		{"max_designs",14,1,1,1,kw_153,0.,0.,0,N_mdm(RealLlit,3nichingType_max_designs)},
		{"radial",14,0,1,1,0,0.,0.,0,N_mdm(RealLlit,3nichingType_radial)}
		},
	kw_155[1] = {
		{"orthogonal_distance",14,0,1,1,0,0.,0.,0,N_mdm(RealLlit,3postProcessorType_distance_postprocessor)}
		},
	kw_156[2] = {
		{"shrinkage_fraction",10,0,1,0,0,0.,0.,0,N_mdm(Real01,shrinkagePercent)},
		{"shrinkage_percentage",2,0,1,0,0,0.,0.,-1,N_mdm(Real01,shrinkagePercent)}
		},
	kw_157[4] = {
		{"below_limit",10,2,1,1,kw_156,0.,0.,0,N_mdm(litp,3replacementType_below_limit)},
		{"elitist",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_elitist)},
		{"roulette_wheel",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_roulette_wheel)},
		{"unique_roulette_wheel",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_unique_roulette_wheel)}
		},
	kw_158[7] = {
		{0,0,9,0,0,kw_30},
		{0,0,7,0,0,kw_150},
		{"convergence_type",8,3,4,0,kw_151},
		{"fitness_type",8,2,1,0,kw_152},
		{"niching_type",8,3,3,0,kw_154},
		{"postprocessor_type",8,1,5,0,kw_155},
		{"replacement_type",8,4,2,0,kw_157}
		},
	kw_159[1] = {
		{"partitions",13,0,1,1,0,0.,0.,0,N_mdm(usharray,varPartitions)}
		},
	kw_160[4] = {
		{"min_boxsize_limit",10,0,2,0,0,0.,0.,0,N_mdm(Real,minBoxSize)},
		{"solution_accuracy",2,0,1,0,0,0.,0.,1,N_mdm(Real,solnTarget)},
		{"solution_target",10,0,1,0,0,0.,0.,0,N_mdm(Real,solnTarget)},
		{"volume_boxsize_limit",10,0,3,0,0,0.,0.,0,N_mdm(Real,volBoxSize)}
		},
	kw_161[9] = {
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
	kw_162[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_163[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_162}
		},
	kw_164[2] = {
		{"compute",8,3,2,0,kw_163},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_165[4] = {
		{0,0,1,0,0,kw_15},
		{0,0,1,0,0,kw_16},
		{0,0,4,0,0,kw_21},
		{"response_levels",14,2,1,0,kw_164,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_166[1] = {
		{"num_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,reliabilityLevels)}
		},
	kw_167[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_168[4] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_RELIABILITIES)},
		{"system",8,2,2,0,kw_167}
		},
	kw_169[2] = {
		{"compute",8,4,2,0,kw_168},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_170[2] = {
		{"reliability_levels",14,1,1,0,kw_166,0.,0.,0,N_mdm(resplevs,reliabilityLevels)},
		{"response_levels",14,2,2,0,kw_169,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_171[1] = {
		{"fixed_seed",8,0,1,0,0,0.,0.,0,N_mdm(true,fixedSeedFlag)}
		},
	kw_172[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_173[1] = {
		{"import_points_file",11,2,1,0,kw_172,0.,0.,0,N_mdm(str,approxImportFile)}
		},
	kw_174[1] = {
		{"noise_tolerance",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,regressionNoiseTol)}
		},
	kw_175[1] = {
		{"noise_tolerance",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,regressionNoiseTol)}
		},
	kw_176[2] = {
		{"l2_penalty",10,0,2,0,0,0.,0.,0,N_mdm(Real,regressionL2Penalty)},
		{"noise_tolerance",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,regressionNoiseTol)}
		},
	kw_177[2] = {
		{"equality_constrained",8,0,1,0,0,0.,0.,0,N_mdm(type,lsRegressionType_EQ_CON_LS)},
		{"svd",8,0,1,0,0,0.,0.,0,N_mdm(type,lsRegressionType_SVD_LS)}
		},
	kw_178[1] = {
		{"noise_tolerance",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,regressionNoiseTol)}
		},
	kw_179[18] = {
		{0,0,1,0,0,kw_173},
		{"basis_pursuit",8,0,2,0,0,0.,0.,0,N_mdm(type,regressionType_BASIS_PURSUIT)},
		{"basis_pursuit_denoising",8,1,2,0,kw_174,0.,0.,0,N_mdm(type,regressionType_BASIS_PURSUIT_DENOISING)},
		{"bp",0,0,2,0,0,0.,0.,-2,N_mdm(type,regressionType_BASIS_PURSUIT)},
		{"bpdn",0,1,2,0,kw_174,0.,0.,-2,N_mdm(type,regressionType_BASIS_PURSUIT_DENOISING)},
		{"cross_validation",8,0,3,0,0,0.,0.,0,N_mdm(true,crossValidation)},
		{"lars",0,1,2,0,kw_175,0.,0.,3,N_mdm(type,regressionType_LEAST_ANGLE_REGRESSION)},
		{"lasso",0,2,2,0,kw_176,0.,0.,1,N_mdm(type,regressionType_LASSO_REGRESSION)},
		{"least_absolute_shrinkage",8,2,2,0,kw_176,0.,0.,0,N_mdm(type,regressionType_LASSO_REGRESSION)},
		{"least_angle_regression",8,1,2,0,kw_175,0.,0.,0,N_mdm(type,regressionType_LEAST_ANGLE_REGRESSION)},
		{"least_squares",8,2,2,0,kw_177,0.,0.,0,N_mdm(type,regressionType_DEFAULT_LEAST_SQ_REGRESSION)},
		{"omp",0,1,2,0,kw_178,0.,0.,1,N_mdm(type,regressionType_ORTHOG_MATCH_PURSUIT)},
		{"orthogonal_matching_pursuit",8,1,2,0,kw_178,0.,0.,0,N_mdm(type,regressionType_ORTHOG_MATCH_PURSUIT)},
		{"ratio_order",10,0,1,0,0,0.,0.,0,N_mdm(Realp,collocRatioTermsOrder)},
		{"reuse_points",8,0,6,0,0,0.,0.,0,N_mdm(lit,pointReuse_all)},
		{"reuse_samples",0,0,6,0,0,0.,0.,-1,N_mdm(lit,pointReuse_all)},
		{"tensor_grid",8,0,5,0,0,0.,0.,0,N_mdm(true,tensorGridFlag)},
		{"use_derivatives",8,0,4,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)}
		},
	kw_180[4] = {
		{0,0,1,0,0,kw_173},
		{"incremental_lhs",8,0,2,0,0,0.,0.,0,N_mdm(lit,expansionSampleType_incremental_lhs)},
		{"reuse_points",8,0,1,0,0,0.,0.,0,N_mdm(lit,pointReuse_all)},
		{"reuse_samples",0,0,1,0,0,0.,0.,-1,N_mdm(lit,pointReuse_all)}
		},
	kw_181[5] = {
		{"collocation_points",13,17,2,1,kw_179,0.,0.,0,N_mdm(szarray,collocationPoints)},
		{"collocation_ratio",10,17,2,1,kw_179,0.,0.,0,N_mdm(Realp,collocationRatio)},
		{"dimension_preference",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,anisoDimPref)},
		{"expansion_import_file",11,0,2,1,0,0.,0.,0,N_mdm(str,expansionImportFile)},
		{"expansion_samples",13,3,2,1,kw_180,0.,0.,0,N_mdm(szarray,expansionSamples)}
		},
	kw_182[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_183[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_184[6] = {
		{"collocation_points",13,0,1,1,0,0.,0.,0,N_mdm(szarray,collocationPoints)},
		{"cross_validation",8,0,2,0,0,0.,0.,0,N_mdm(true,crossValidation)},
		{"import_points_file",11,2,5,0,kw_183,0.,0.,0,N_mdm(str,approxImportFile)},
		{"reuse_points",8,0,4,0,0,0.,0.,0,N_mdm(lit,pointReuse_all)},
		{"reuse_samples",0,0,4,0,0,0.,0.,-1,N_mdm(lit,pointReuse_all)},
		{"tensor_grid",13,0,3,0,0,0.,0.,0,N_mdm(usharray,tensorGridOrder)}
		},
	kw_185[3] = {
		{"decay",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_DECAY)},
		{"generalized",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_GENERALIZED)},
		{"sobol",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_SOBOL)}
		},
	kw_186[2] = {
		{"dimension_adaptive",8,3,1,1,kw_185},
		{"uniform",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_UNIFORM_CONTROL)}
		},
	kw_187[3] = {
		{"dimension_preference",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,anisoDimPref)},
		{"nested",8,0,2,0,0,0.,0.,0,N_mdm(type,nestingOverride_NESTED)},
		{"non_nested",8,0,2,0,0,0.,0.,0,N_mdm(type,nestingOverride_NON_NESTED)}
		},
	kw_188[3] = {
		{"adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(lit,integrationRefine_ais)},
		{"import",8,0,1,1,0,0.,0.,0,N_mdm(lit,integrationRefine_is)},
		{"mm_adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(lit,integrationRefine_mmais)}
		},
	kw_189[2] = {
		{"lhs",8,0,1,1,0,0.,0.,0,N_mdm(lit,sampleType_lhs)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(lit,sampleType_random)}
		},
	kw_190[3] = {
		{0,0,3,0,0,kw_187},
		{"restricted",8,0,1,0,0,0.,0.,0,N_mdm(type,growthOverride_RESTRICTED)},
		{"unrestricted",8,0,1,0,0,0.,0.,0,N_mdm(type,growthOverride_UNRESTRICTED)}
		},
	kw_191[2] = {
		{"drop_tolerance",10,0,2,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)},
		{"interaction_order",0x19,0,1,0,0,0.,0.,0,N_mdm(ushint,vbdOrder)}
		},
	kw_192[22] = {
		{0,0,1,0,0,kw_15},
		{0,0,1,0,0,kw_16},
		{0,0,4,0,0,kw_21},
		{0,0,2,0,0,kw_170},
		{0,0,1,0,0,kw_171},
		{"askey",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionType_ASKEY_U)},
		{"cubature_integrand",9,0,3,1,0,0.,0.,0,N_mdm(ushint,cubIntOrder)},
		{"diagonal_covariance",8,0,5,0,0,0.,0.,0,N_mdm(type,covarianceControl_DIAGONAL_COVARIANCE)},
		{"expansion_order",13,5,3,1,kw_181,0.,0.,0,N_mdm(usharray,expansionOrder)},
		{"export_points_file",11,2,9,0,kw_182,0.,0.,0,N_mdm(str,approxExportFile)},
		{"full_covariance",8,0,5,0,0,0.,0.,0,N_mdm(type,covarianceControl_FULL_COVARIANCE)},
		{"least_interpolation",0,6,3,1,kw_184,0.,0.,3,N_mdm(type,regressionType_ORTHOG_LEAST_INTERPOLATION)},
		{"normalized",8,0,6,0,0,0.,0.,0,N_mdm(true,normalizedCoeffs)},
		{"oli",0,6,3,1,kw_184,0.,0.,1,N_mdm(type,regressionType_ORTHOG_LEAST_INTERPOLATION)},
		{"orthogonal_least_interpolation",8,6,3,1,kw_184,0.,0.,0,N_mdm(type,regressionType_ORTHOG_LEAST_INTERPOLATION)},
		{"p_refinement",8,2,1,0,kw_186,0.,0.,0,N_mdm(type,refinementType_P_REFINEMENT)},
		{"quadrature_order",13,3,3,1,kw_187,0.,0.,0,N_mdm(usharray,quadratureOrder)},
		{"sample_refinement",8,3,8,0,kw_188},
		{"sample_type",8,2,7,0,kw_189},
		{"sparse_grid_level",13,2,3,1,kw_190,0.,0.,0,N_mdm(usharray,sparseGridLevel)},
		{"variance_based_decomp",8,2,4,0,kw_191,0.,0.,0,N_mdm(true,vbdFlag)},
		{"wiener",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionType_STD_NORMAL_U)}
		},
	kw_193[1] = {
		{"previous_samples",9,0,1,1,0,0.,0.,0,N_mdm(int,previousSamples)}
		},
	kw_194[4] = {
		{"incremental_lhs",8,1,1,1,kw_193,0.,0.,0,N_mdm(lit,sampleType_incremental_lhs)},
		{"incremental_random",8,1,1,1,kw_193,0.,0.,0,N_mdm(lit,sampleType_incremental_random)},
		{"lhs",8,0,1,1,0,0.,0.,0,N_mdm(lit,sampleType_lhs)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(lit,sampleType_random)}
		},
	kw_195[1] = {
		{"drop_tolerance",10,0,1,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)}
		},
	kw_196[7] = {
		{0,0,1,0,0,kw_15},
		{0,0,1,0,0,kw_16},
		{0,0,4,0,0,kw_21},
		{0,0,2,0,0,kw_170},
		{0,0,1,0,0,kw_171},
		{"sample_type",8,4,1,0,kw_194},
		{"variance_based_decomp",8,1,2,0,kw_195,0.,0.,0,N_mdm(true,vbdFlag)}
		},
	kw_197[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_198[2] = {
		{"generalized",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_GENERALIZED)},
		{"sobol",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_SOBOL)}
		},
	kw_199[3] = {
		{"dimension_adaptive",8,2,1,1,kw_198},
		{"local_adaptive",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_LOCAL_ADAPTIVE_CONTROL)},
		{"uniform",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_UNIFORM_CONTROL)}
		},
	kw_200[2] = {
		{"generalized",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_GENERALIZED)},
		{"sobol",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_SOBOL)}
		},
	kw_201[2] = {
		{"dimension_adaptive",8,2,1,1,kw_200},
		{"uniform",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_UNIFORM_CONTROL)}
		},
	kw_202[3] = {
		{"adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(lit,integrationRefine_ais)},
		{"import",8,0,1,1,0,0.,0.,0,N_mdm(lit,integrationRefine_is)},
		{"mm_adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(lit,integrationRefine_mmais)}
		},
	kw_203[2] = {
		{"lhs",8,0,1,1,0,0.,0.,0,N_mdm(lit,sampleType_lhs)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(lit,sampleType_random)}
		},
	kw_204[4] = {
		{"hierarchical",8,0,2,0,0,0.,0.,0,N_mdm(type,sparseGridBasisType_HIERARCHICAL_INTERPOLANT)},
		{"nodal",8,0,2,0,0,0.,0.,0,N_mdm(type,sparseGridBasisType_NODAL_INTERPOLANT)},
		{"restricted",8,0,1,0,0,0.,0.,0,N_mdm(type,growthOverride_RESTRICTED)},
		{"unrestricted",8,0,1,0,0,0.,0.,0,N_mdm(type,growthOverride_UNRESTRICTED)}
		},
	kw_205[2] = {
		{"drop_tolerance",10,0,2,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)},
		{"interaction_order",0x19,0,1,0,0,0.,0.,0,N_mdm(ushint,vbdOrder)}
		},
	kw_206[22] = {
		{0,0,1,0,0,kw_15},
		{0,0,1,0,0,kw_16},
		{0,0,4,0,0,kw_21},
		{0,0,2,0,0,kw_170},
		{0,0,1,0,0,kw_171},
		{"askey",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionType_ASKEY_U)},
		{"diagonal_covariance",8,0,8,0,0,0.,0.,0,N_mdm(type,covarianceControl_DIAGONAL_COVARIANCE)},
		{"dimension_preference",14,0,4,0,0,0.,0.,0,N_mdm(RealDL,anisoDimPref)},
		{"export_points_file",11,2,11,0,kw_197,0.,0.,0,N_mdm(str,approxExportFile)},
		{"full_covariance",8,0,8,0,0,0.,0.,0,N_mdm(type,covarianceControl_FULL_COVARIANCE)},
		{"h_refinement",8,3,1,0,kw_199,0.,0.,0,N_mdm(type,refinementType_H_REFINEMENT)},
		{"nested",8,0,6,0,0,0.,0.,0,N_mdm(type,nestingOverride_NESTED)},
		{"non_nested",8,0,6,0,0,0.,0.,0,N_mdm(type,nestingOverride_NON_NESTED)},
		{"p_refinement",8,2,1,0,kw_201,0.,0.,0,N_mdm(type,refinementType_P_REFINEMENT)},
		{"piecewise",8,0,2,0,0,0.,0.,0,NIDRProblemDescDB::method_piecewise},
		{"quadrature_order",13,0,3,1,0,0.,0.,0,N_mdm(usharray,quadratureOrder)},
		{"sample_refinement",8,3,10,0,kw_202},
		{"sample_type",8,2,9,0,kw_203},
		{"sparse_grid_level",13,4,3,1,kw_204,0.,0.,0,N_mdm(usharray,sparseGridLevel)},
		{"use_derivatives",8,0,5,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)},
		{"variance_based_decomp",8,2,7,0,kw_205,0.,0.,0,N_mdm(true,vbdFlag)},
		{"wiener",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionType_STD_NORMAL_U)}
		},
	kw_207[1] = {
		{"misc_options",15,0,1,0,0,0.,0.,0,N_mdm(strL,miscOptions)}
		},
	kw_208[3] = {
		{"function_precision",10,0,2,0,0,0.,0.,0,N_mdm(Real,functionPrecision)},
		{"linesearch_tolerance",10,0,3,0,0,0.,0.,0,N_mdm(Real,lineSearchTolerance)},
		{"verify_level",9,0,1,0,0,0.,0.,0,N_mdm(int,verifyLevel)}
		},
	kw_209[3] = {
		{0,0,9,0,0,kw_30},
		{0,0,3,0,0,kw_208},
		{""}
		},
	kw_210[2] = {
		{"gradient_tolerance",10,0,2,0,0,0.,0.,0,N_mdm(Real,gradientTolerance)},
		{"max_step",10,0,1,0,0,0.,0.,0,N_mdm(Real,maxStep)}
		},
	kw_211[3] = {
		{0,0,9,0,0,kw_30},
		{0,0,2,0,0,kw_210},
		{""}
		},
	kw_212[2] = {
		{0,0,9,0,0,kw_30},
		{"search_scheme_size",9,0,1,0,0,0.,0.,0,N_mdm(int,searchSchemeSize)}
		},
	kw_213[3] = {
		{"argaez_tapia",8,0,1,1,0,0.,0.,0,N_mdm(type,meritFn_ArgaezTapia)},
		{"el_bakry",8,0,1,1,0,0.,0.,0,N_mdm(type,meritFn_NormFmu)},
		{"van_shanno",8,0,1,1,0,0.,0.,0,N_mdm(type,meritFn_VanShanno)}
		},
	kw_214[4] = {
		{"gradient_based_line_search",8,0,1,1,0,0.,0.,0,N_mdm(lit,searchMethod_gradient_based_line_search)},
		{"tr_pds",8,0,1,1,0,0.,0.,0,N_mdm(lit,searchMethod_tr_pds)},
		{"trust_region",8,0,1,1,0,0.,0.,0,N_mdm(lit,searchMethod_trust_region)},
		{"value_based_line_search",8,0,1,1,0,0.,0.,0,N_mdm(lit,searchMethod_value_based_line_search)}
		},
	kw_215[6] = {
		{0,0,9,0,0,kw_30},
		{0,0,2,0,0,kw_210},
		{"centering_parameter",10,0,4,0,0,0.,0.,0,N_mdm(Real,centeringParam)},
		{"merit_function",8,3,2,0,kw_213},
		{"search_method",8,4,1,0,kw_214},
		{"steplength_to_boundary",10,0,3,0,0,0.,0.,0,N_mdm(Real,stepLenToBoundary)}
		},
	kw_216[5] = {
		{"debug",8,0,1,1,0,0.,0.,0,N_mdm(type,methodOutput_DEBUG_OUTPUT)},
		{"normal",8,0,1,1,0,0.,0.,0,N_mdm(type,methodOutput_NORMAL_OUTPUT)},
		{"quiet",8,0,1,1,0,0.,0.,0,N_mdm(type,methodOutput_QUIET_OUTPUT)},
		{"silent",8,0,1,1,0,0.,0.,0,N_mdm(type,methodOutput_SILENT_OUTPUT)},
		{"verbose",8,0,1,1,0,0.,0.,0,N_mdm(type,methodOutput_VERBOSE_OUTPUT)}
		},
	kw_217[3] = {
		{0,0,1,0,0,kw_15},
		{0,0,1,0,0,kw_16},
		{"partitions",13,0,1,0,0,0.,0.,0,N_mdm(usharray,varPartitions)}
		},
	kw_218[4] = {
		{"converge_order",8,0,1,1,0,0.,0.,0,N_mdm(lit,subMethodName_converge_order)},
		{"converge_qoi",8,0,1,1,0,0.,0.,0,N_mdm(lit,subMethodName_converge_qoi)},
		{"estimate_order",8,0,1,1,0,0.,0.,0,N_mdm(lit,subMethodName_estimate_order)},
		{"refinement_rate",10,0,2,0,0,0.,0.,0,N_mdm(Real,refinementRate)}
		},
	kw_219[2] = {
		{"num_generations",0x29,0,2,0,0,0.,0.,0,N_mdm(sizet,numGenerations)},
		{"percent_change",10,0,1,0,0,0.,0.,0,N_mdm(Realz,convergenceTolerance)}
		},
	kw_220[2] = {
		{"num_generations",0x29,0,2,0,0,0.,0.,0,N_mdm(sizet,numGenerations)},
		{"percent_change",10,0,1,0,0,0.,0.,0,N_mdm(Realz,convergenceTolerance)}
		},
	kw_221[2] = {
		{"average_fitness_tracker",8,2,1,1,kw_219,0.,0.,0,N_mdm(lit,convergenceType_average_fitness_tracker)},
		{"best_fitness_tracker",8,2,1,1,kw_220,0.,0.,0,N_mdm(lit,convergenceType_best_fitness_tracker)}
		},
	kw_222[2] = {
		{"constraint_penalty",10,0,2,0,0,0.,0.,0,N_mdm(Realp,constraintTolerance)},
		{"merit_function",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_merit_function)}
		},
	kw_223[4] = {
		{"elitist",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_elitist)},
		{"favor_feasible",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_favor_feasible)},
		{"roulette_wheel",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_roulette_wheel)},
		{"unique_roulette_wheel",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_unique_roulette_wheel)}
		},
	kw_224[5] = {
		{0,0,9,0,0,kw_30},
		{0,0,7,0,0,kw_150},
		{"convergence_type",8,2,3,0,kw_221},
		{"fitness_type",8,2,1,0,kw_222},
		{"replacement_type",8,4,2,0,kw_223}
		},
	kw_225[4] = {
		{0,0,9,0,0,kw_30},
		{0,0,3,0,0,kw_208},
		{"nlssol",8,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_nlssol_sqp)},
		{"npsol",8,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_npsol_sqp)}
		},
	kw_226[3] = {
		{"approx_method_name",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodName)},
		{"approx_method_pointer",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodPointer)},
		{"replace_points",8,0,2,0,0,0.,0.,0,N_mdm(true,surrBasedGlobalReplacePts)}
		},
	kw_227[2] = {
		{"filter",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalAcceptLogic_FILTER)},
		{"tr_ratio",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalAcceptLogic_TR_RATIO)}
		},
	kw_228[7] = {
		{"augmented_lagrangian_objective",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbObj_AUGMENTED_LAGRANGIAN_OBJECTIVE)},
		{"lagrangian_objective",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbObj_LAGRANGIAN_OBJECTIVE)},
		{"linearized_constraints",8,0,2,2,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbCon_LINEARIZED_CONSTRAINTS)},
		{"no_constraints",8,0,2,2,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbCon_NO_CONSTRAINTS)},
		{"original_constraints",8,0,2,2,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbCon_ORIGINAL_CONSTRAINTS)},
		{"original_primary",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbObj_ORIGINAL_PRIMARY)},
		{"single_objective",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbObj_SINGLE_OBJECTIVE)}
		},
	kw_229[1] = {
		{"homotopy",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalConstrRelax_HOMOTOPY)}
		},
	kw_230[4] = {
		{"adaptive_penalty_merit",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalMeritFn_ADAPTIVE_PENALTY_MERIT)},
		{"augmented_lagrangian_merit",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalMeritFn_AUGMENTED_LAGRANGIAN_MERIT)},
		{"lagrangian_merit",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalMeritFn_LAGRANGIAN_MERIT)},
		{"penalty_merit",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalMeritFn_PENALTY_MERIT)}
		},
	kw_231[6] = {
		{"contract_threshold",10,0,3,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRContractTrigger)},
		{"contraction_factor",10,0,5,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRContract)},
		{"expand_threshold",10,0,4,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRExpandTrigger)},
		{"expansion_factor",10,0,6,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRExpand)},
		{"initial_size",10,0,1,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRInitSize)},
		{"minimum_size",10,0,2,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRMinSize)}
		},
	kw_232[10] = {
		{0,0,9,0,0,kw_30},
		{"acceptance_logic",8,2,7,0,kw_227},
		{"approx_method_name",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodName)},
		{"approx_method_pointer",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodPointer)},
		{"approx_subproblem",8,7,5,0,kw_228},
		{"constraint_relax",8,1,8,0,kw_229},
		{"merit_function",8,4,6,0,kw_230},
		{"soft_convergence_limit",9,0,2,0,0,0.,0.,0,N_mdm(int,surrBasedLocalSoftConvLimit)},
		{"trust_region",8,6,4,0,kw_231,0.,0.,0,0,0,NIDRProblemDescDB::method_tr_final},
		{"truth_surrogate_bypass",8,0,3,0,0,0.,0.,0,N_mdm(true,surrBasedLocalLayerBypass)}
		},
	kw_233[3] = {
		{"final_point",14,0,1,1,0,0.,0.,0,N_mdm(RealDL,finalPoint)},
		{"num_steps",9,0,2,2,0,0.,0.,0,N_mdm(int,numSteps)},
		{"step_vector",14,0,1,1,0,0.,0.,0,N_mdm(RealDL,stepVector)}
		},
	kw_234[86] = {
		{"adaptive_sampling",8,8,11,1,kw_29,0.,0.,0,N_mdm(lit,methodName_nond_adaptive_sampling)},
		{"asynch_pattern_search",8,9,11,1,kw_33,0.,0.,0,N_mdm(lit,methodName_asynch_pattern_search)},
		{"bayes_calibration",8,6,11,1,kw_55,0.,0.,0,N_mdm(lit,methodName_nond_bayes_calibration)},
		{"centered_parameter_study",8,3,11,1,kw_56,0.,0.,0,N_mdm(lit,methodName_centered_parameter_study)},
		{"coliny_apps",0,9,11,1,kw_33,0.,0.,-3,N_mdm(lit,methodName_asynch_pattern_search)},
		{"coliny_beta",8,1,11,1,kw_58,0.,0.,0,N_mdm(lit,methodName_coliny_beta)},
		{"coliny_cobyla",8,0,11,1,kw_60,0.,0.,0,N_mdm(lit,methodName_coliny_cobyla)},
		{"coliny_direct",8,6,11,1,kw_62,0.,0.,0,N_mdm(lit,methodName_coliny_direct)},
		{"coliny_ea",8,9,11,1,kw_69,0.,0.,0,N_mdm(lit,methodName_coliny_ea)},
		{"coliny_pattern_search",8,8,11,1,kw_74,0.,0.,0,N_mdm(lit,methodName_coliny_pattern_search)},
		{"coliny_solis_wets",8,4,11,1,kw_75,0.,0.,0,N_mdm(lit,methodName_coliny_solis_wets)},
		{"conmin",8,2,11,1,kw_76},
		{"conmin_frcg",8,9,11,1,kw_30,0.,0.,0,N_mdm(lit,methodName_conmin_frcg)},
		{"conmin_mfd",8,9,11,1,kw_30,0.,0.,0,N_mdm(lit,methodName_conmin_mfd)},
		{"constraint_tolerance",10,0,8,0,0,0.,0.,0,N_mdm(Real,constraintTolerance)},
		{"convergence_tolerance",10,0,7,0,0,0.,0.,0,N_mdm(Real,convergenceTolerance)},
		{"dace",8,12,11,1,kw_78,0.,0.,0,N_mdm(lit,methodName_dace)},
		{"dl_solver",11,9,11,1,kw_30,0.,0.,0,N_mdm(slit2,3methodName_dl_solver)},
		{"dot",8,5,11,1,kw_79},
		{"dot_bfgs",8,9,11,1,kw_30,0.,0.,0,N_mdm(lit,methodName_dot_bfgs)},
		{"dot_frcg",8,9,11,1,kw_30,0.,0.,0,N_mdm(lit,methodName_dot_frcg)},
		{"dot_mmfd",8,9,11,1,kw_30,0.,0.,0,N_mdm(lit,methodName_dot_mmfd)},
		{"dot_slp",8,9,11,1,kw_30,0.,0.,0,N_mdm(lit,methodName_dot_slp)},
		{"dot_sqp",8,9,11,1,kw_30,0.,0.,0,N_mdm(lit,methodName_dot_sqp)},
		{"efficient_global",8,5,11,1,kw_83,0.,0.,0,N_mdm(lit,methodName_efficient_global)},
		{"efficient_subspace",8,2,11,1,kw_84,0.,0.,0,N_mdm(lit,methodName_nond_efficient_subspace)},
		{"final_solutions",0x29,0,10,0,0,0.,0.,0,N_mdm(sizet,numFinalSolutions)},
		{"fsu_cvt",8,6,11,1,kw_87,0.,0.,0,N_mdm(lit,methodName_fsu_cvt)},
		{"fsu_quasi_mc",8,10,11,1,kw_89},
		{"gaussian_process_adaptive_importance_sampling",0,4,11,1,kw_95,0.,0.,4,N_mdm(lit,methodName_nond_gpais)},
		{"global_evidence",8,5,11,1,kw_103,0.,0.,0,N_mdm(lit,methodName_nond_global_evidence)},
		{"global_interval_est",8,5,11,1,kw_109,0.,0.,0,N_mdm(lit,methodName_nond_global_interval_est)},
		{"global_reliability",8,10,11,1,kw_121,0.,0.,0,N_mdm(lit,methodName_nond_global_reliability)},
		{"gpais",8,4,11,1,kw_95,0.,0.,0,N_mdm(lit,methodName_nond_gpais)},
		{"id_method",11,0,1,0,0,0.,0.,0,N_mdm(str,idMethod)},
		{"importance_sampling",8,4,11,1,kw_125,0.,0.,0,N_mdm(lit,methodName_nond_importance_sampling)},
		{"list_parameter_study",8,2,11,1,kw_127,0.,0.,0,N_mdm(lit,methodName_list_parameter_study)},
		{"local_evidence",8,6,11,1,kw_134,0.,0.,0,N_mdm(lit,methodName_nond_local_evidence)},
		{"local_interval_est",8,2,11,1,kw_135,0.,0.,0,N_mdm(lit,methodName_nond_local_interval_est)},
		{"local_reliability",8,3,11,1,kw_143,0.,0.,0,N_mdm(lit2,methodName_nond_local_reliability)},
		{"max_function_evaluations",0x29,0,5,0,0,0.,0.,0,N_mdm(nnint,maxFunctionEvaluations)},
		{"max_iterations",0x29,0,4,0,0,0.,0.,0,N_mdm(nnint,maxIterations)},
		{"mesh_adaptive_search",8,6,11,1,kw_144,0.,0.,0,N_mdm(lit,methodName_mesh_adaptive_search)},
		{"model_pointer",11,0,2,0,0,0.,0.,0,N_mdm(str,modelPointer)},
		{"moga",8,5,11,1,kw_158,0.,0.,0,N_mdm(lit,methodName_moga)},
		{"multidim_parameter_study",8,1,11,1,kw_159,0.,0.,0,N_mdm(lit,methodName_multidim_parameter_study)},
		{"ncsu_direct",8,4,11,1,kw_160,0.,0.,0,N_mdm(lit,methodName_ncsu_direct)},
		{"nl2sol",8,9,11,1,kw_161,0.,0.,0,N_mdm(lit,methodName_nl2sol)},
		{"nlpql_sqp",8,9,11,1,kw_30,0.,0.,0,N_mdm(lit,methodName_nlpql_sqp)},
		{"nlssol_sqp",8,0,11,1,kw_209,0.,0.,0,N_mdm(lit,methodName_nlssol_sqp)},
		{"nond_adaptive_sampling",0,8,11,1,kw_29,0.,0.,-50,N_mdm(lit,methodName_nond_adaptive_sampling)},
		{"nond_bayes_calibration",0,6,11,1,kw_55,0.,0.,-49,N_mdm(lit,methodName_nond_bayes_calibration)},
		{"nond_efficient_subspace",0,2,11,1,kw_84,0.,0.,-27,N_mdm(lit,methodName_nond_efficient_subspace)},
		{"nond_global_evidence",0,5,11,1,kw_103,0.,0.,-23,N_mdm(lit,methodName_nond_global_evidence)},
		{"nond_global_interval_est",0,5,11,1,kw_109,0.,0.,-23,N_mdm(lit,methodName_nond_global_interval_est)},
		{"nond_global_reliability",0,10,11,1,kw_121,0.,0.,-23,N_mdm(lit,methodName_nond_global_reliability)},
		{"nond_importance_sampling",0,4,11,1,kw_125,0.,0.,-21,N_mdm(lit,methodName_nond_importance_sampling)},
		{"nond_local_evidence",0,6,11,1,kw_134,0.,0.,-20,N_mdm(lit,methodName_nond_local_evidence)},
		{"nond_local_interval_est",0,2,11,1,kw_135,0.,0.,-20,N_mdm(lit,methodName_nond_local_interval_est)},
		{"nond_local_reliability",0,3,11,1,kw_143,0.,0.,-20,N_mdm(lit2,methodName_nond_local_reliability)},
		{"nond_pof_darts",0,1,11,1,kw_165,0.,0.,13,N_mdm(lit,methodName_nond_pof_darts)},
		{"nond_polynomial_chaos",0,17,11,1,kw_192,0.,0.,13,N_mdm(lit,methodName_nond_polynomial_chaos)},
		{"nond_sampling",0,2,11,1,kw_196,0.,0.,15,N_mdm(lit,methodName_nond_sampling)},
		{"nond_stoch_collocation",0,17,11,1,kw_206,0.,0.,19,N_mdm(lit,methodName_nond_stoch_collocation)},
		{"nonlinear_cg",8,1,11,1,kw_207,0.,0.,0,N_mdm(lit,methodName_nonlinear_cg)},
		{"npsol_sqp",8,0,11,1,kw_209,0.,0.,0,N_mdm(lit,methodName_npsol_sqp)},
		{"optpp_cg",8,0,11,1,kw_211,0.,0.,0,N_mdm(lit,methodName_optpp_cg)},
		{"optpp_fd_newton",8,4,11,1,kw_215,0.,0.,0,N_mdm(lit,methodName_optpp_fd_newton)},
		{"optpp_g_newton",8,4,11,1,kw_215,0.,0.,0,N_mdm(lit,methodName_optpp_g_newton)},
		{"optpp_newton",8,4,11,1,kw_215,0.,0.,0,N_mdm(lit,methodName_optpp_newton)},
		{"optpp_pds",8,1,11,1,kw_212,0.,0.,0,N_mdm(lit,methodName_optpp_pds)},
		{"optpp_q_newton",8,4,11,1,kw_215,0.,0.,0,N_mdm(lit,methodName_optpp_q_newton)},
		{"output",8,5,3,0,kw_216},
		{"pof_darts",8,1,11,1,kw_165,0.,0.,0,N_mdm(lit,methodName_nond_pof_darts)},
		{"polynomial_chaos",8,17,11,1,kw_192,0.,0.,0,N_mdm(lit,methodName_nond_polynomial_chaos)},
		{"psuade_moat",8,1,11,1,kw_217,0.,0.,0,N_mdm(lit,methodName_psuade_moat)},
		{"richardson_extrap",8,4,11,1,kw_218,0.,0.,0,N_mdm(lit,methodName_richardson_extrap)},
		{"sampling",8,2,11,1,kw_196,0.,0.,0,N_mdm(lit,methodName_nond_sampling)},
		{"scaling",8,0,9,0,0,0.,0.,0,N_mdm(true,methodScaling)},
		{"soga",8,3,11,1,kw_224,0.,0.,0,N_mdm(lit,methodName_soga)},
		{"speculative",8,0,6,0,0,0.,0.,0,N_mdm(true,speculativeFlag)},
		{"stanford",8,2,11,1,kw_225},
		{"stoch_collocation",8,17,11,1,kw_206,0.,0.,0,N_mdm(lit,methodName_nond_stoch_collocation)},
		{"surrogate_based_global",8,3,11,1,kw_226,0.,0.,0,N_mdm(lit,methodName_surrogate_based_global)},
		{"surrogate_based_local",8,9,11,1,kw_232,0.,0.,0,N_mdm(lit,methodName_surrogate_based_local)},
		{"vector_parameter_study",8,3,11,1,kw_233,0.,0.,0,N_mdm(lit,methodName_vector_parameter_study)}
		},
	kw_235[1] = {
		{"optional_interface_responses_pointer",11,0,1,0,0,0.,0.,0,N_mom(str,optionalInterfRespPointer)}
		},
	kw_236[4] = {
		{"primary_response_mapping",14,0,3,0,0,0.,0.,0,N_mom(RealDL,primaryRespCoeffs)},
		{"primary_variable_mapping",15,0,1,0,0,0.,0.,0,N_mom(strL,primaryVarMaps)},
		{"secondary_response_mapping",14,0,4,0,0,0.,0.,0,N_mom(RealDL,secondaryRespCoeffs)},
		{"secondary_variable_mapping",15,0,2,0,0,0.,0.,0,N_mom(strL,secondaryVarMaps)}
		},
	kw_237[2] = {
		{"optional_interface_pointer",11,1,1,0,kw_235,0.,0.,0,N_mom(str,interfacePointer)},
		{"sub_method_pointer",11,4,2,1,kw_236,0.,0.,0,N_mom(str,subMethodPointer)}
		},
	kw_238[1] = {
		{"interface_pointer",11,0,1,0,0,0.,0.,0,N_mom(str,interfacePointer)}
		},
	kw_239[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mom(true,approxChallengeAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mom(false,approxChallengeAnnotated)}
		},
	kw_240[6] = {
		{"additive",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_ADDITIVE_CORRECTION)},
		{"combined",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_COMBINED_CORRECTION)},
		{"first_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_1)},
		{"multiplicative",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_MULTIPLICATIVE_CORRECTION)},
		{"second_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_2)},
		{"zeroth_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_0)}
		},
	kw_241[2] = {
		{"folds",9,0,1,0,0,0.,0.,0,N_mom(int,numFolds)},
		{"percent",10,0,1,0,0,0.,0.,0,N_mom(Real,percentFold)}
		},
	kw_242[2] = {
		{"cross_validate",8,2,1,0,kw_241,0.,0.,0,N_mom(true,crossValidateFlag)},
		{"press",8,0,2,0,0,0.,0.,0,N_mom(true,pressFlag)}
		},
	kw_243[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mom(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mom(false,approxExportAnnotated)}
		},
	kw_244[3] = {
		{"constant",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_constant)},
		{"linear",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_linear)},
		{"reduced_quadratic",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_reduced_quadratic)}
		},
	kw_245[2] = {
		{"point_selection",8,0,1,0,0,0.,0.,0,N_mom(true,pointSelection)},
		{"trend",8,3,2,0,kw_244}
		},
	kw_246[4] = {
		{"constant",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_constant)},
		{"linear",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_linear)},
		{"quadratic",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_quadratic)},
		{"reduced_quadratic",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_reduced_quadratic)}
		},
	kw_247[7] = {
		{"correlation_lengths",14,0,5,0,0,0.,0.,0,N_mom(RealDL,krigingCorrelations)},
		{"export_model_file",11,0,6,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"find_nugget",9,0,4,0,0,0.,0.,0,N_mom(shint,krigingFindNugget)},
		{"max_trials",0x19,0,3,0,0,0.,0.,0,N_mom(shint,krigingMaxTrials)},
		{"nugget",0x1a,0,4,0,0,0.,0.,0,N_mom(Real,krigingNugget)},
		{"optimization_method",11,0,2,0,0,0.,0.,0,N_mom(str,krigingOptMethod)},
		{"trend",8,4,1,0,kw_246}
		},
	kw_248[2] = {
		{"dakota",8,2,1,1,kw_245,0.,0.,0,N_mom(lit,surrogateType_global_gaussian)},
		{"surfpack",8,7,1,1,kw_247,0.,0.,0,N_mom(lit,surrogateType_global_kriging)}
		},
	kw_249[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mom(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mom(false,approxImportAnnotated)}
		},
	kw_250[2] = {
		{"cubic",8,0,1,1,0,0.,0.,0,N_mom(lit,marsInterpolation_cubic)},
		{"linear",8,0,1,1,0,0.,0.,0,N_mom(lit,marsInterpolation_linear)}
		},
	kw_251[3] = {
		{"export_model_file",11,0,3,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"interpolation",8,2,2,0,kw_250},
		{"max_bases",9,0,1,0,0,0.,0.,0,N_mom(shint,marsMaxBases)}
		},
	kw_252[3] = {
		{"export_model_file",11,0,3,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"poly_order",9,0,1,0,0,0.,0.,0,N_mom(shint,mlsPolyOrder)},
		{"weight_function",9,0,2,0,0,0.,0.,0,N_mom(shint,mlsWeightFunction)}
		},
	kw_253[4] = {
		{"export_model_file",11,0,4,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"nodes",9,0,1,0,0,0.,0.,0,N_mom(shint,annNodes)},
		{"random_weight",9,0,3,0,0,0.,0.,0,N_mom(shint,annRandomWeight)},
		{"range",10,0,2,0,0,0.,0.,0,N_mom(Real,annRange)}
		},
	kw_254[4] = {
		{"cubic",8,0,1,1,0,0.,0.,0,N_mom(order,polynomialOrder_3)},
		{"export_model_file",11,0,2,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"linear",8,0,1,1,0,0.,0.,0,N_mom(order,polynomialOrder_1)},
		{"quadratic",8,0,1,1,0,0.,0.,0,N_mom(order,polynomialOrder_2)}
		},
	kw_255[5] = {
		{"bases",9,0,1,0,0,0.,0.,0,N_mom(shint,rbfBases)},
		{"export_model_file",11,0,5,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"max_pts",9,0,2,0,0,0.,0.,0,N_mom(shint,rbfMaxPts)},
		{"max_subsets",9,0,4,0,0,0.,0.,0,N_mom(shint,rbfMaxSubsets)},
		{"min_partition",9,0,3,0,0,0.,0.,0,N_mom(shint,rbfMinPartition)}
		},
	kw_256[3] = {
		{"all",8,0,1,1,0,0.,0.,0,N_mom(lit,approxPointReuse_all)},
		{"none",8,0,1,1,0,0.,0.,0,N_mom(lit,approxPointReuse_none)},
		{"region",8,0,1,1,0,0.,0.,0,N_mom(lit,approxPointReuse_region)}
		},
	kw_257[21] = {
		{"challenge_points_file",11,2,10,0,kw_239,0.,0.,0,N_mom(str,approxChallengeFile)},
		{"correction",8,6,8,0,kw_240},
		{"dace_method_pointer",11,0,3,0,0,0.,0.,0,N_mom(str,subMethodPointer)},
		{"diagnostics",7,2,9,0,kw_242,0.,0.,6,N_mom(strL,diagMetrics)},
		{"export_points_file",11,2,6,0,kw_243,0.,0.,0,N_mom(str,approxExportFile)},
		{"gaussian_process",8,2,1,1,kw_248},
		{"import_points_file",11,2,5,0,kw_249,0.,0.,0,N_mom(str,approxImportFile)},
		{"kriging",0,2,1,1,kw_248,0.,0.,-2},
		{"mars",8,3,1,1,kw_251,0.,0.,0,N_mom(lit,surrogateType_global_mars)},
		{"metrics",15,2,9,0,kw_242,0.,0.,0,N_mom(strL,diagMetrics)},
		{"minimum_points",8,0,2,0,0,0.,0.,0,N_mom(type,pointsManagement_MINIMUM_POINTS)},
		{"moving_least_squares",8,3,1,1,kw_252,0.,0.,0,N_mom(lit,surrogateType_global_moving_least_squares)},
		{"neural_network",8,4,1,1,kw_253,0.,0.,0,N_mom(lit,surrogateType_global_neural_network)},
		{"polynomial",8,4,1,1,kw_254,0.,0.,0,N_mom(lit,surrogateType_global_polynomial)},
		{"radial_basis",8,5,1,1,kw_255,0.,0.,0,N_mom(lit,surrogateType_global_radial_basis)},
		{"recommended_points",8,0,2,0,0,0.,0.,0,N_mom(type,pointsManagement_RECOMMENDED_POINTS)},
		{"reuse_points",8,3,4,0,kw_256},
		{"reuse_samples",0,3,4,0,kw_256,0.,0.,-1},
		{"samples_file",3,2,5,0,kw_249,0.,0.,-12,N_mom(str,approxImportFile)},
		{"total_points",9,0,2,0,0,0.,0.,0,N_mom(int,pointsTotal)},
		{"use_derivatives",8,0,7,0,0,0.,0.,0,N_mom(true,modelUseDerivsFlag)}
		},
	kw_258[6] = {
		{"additive",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_ADDITIVE_CORRECTION)},
		{"combined",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_COMBINED_CORRECTION)},
		{"first_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_1)},
		{"multiplicative",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_MULTIPLICATIVE_CORRECTION)},
		{"second_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_2)},
		{"zeroth_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_0)}
		},
	kw_259[3] = {
		{"correction",8,6,3,3,kw_258},
		{"high_fidelity_model_pointer",11,0,2,2,0,0.,0.,0,N_mom(str,truthModelPointer)},
		{"low_fidelity_model_pointer",11,0,1,1,0,0.,0.,0,N_mom(str,lowFidelityModelPointer)}
		},
	kw_260[1] = {
		{"actual_model_pointer",11,0,1,1,0,0.,0.,0,N_mom(str,truthModelPointer)}
		},
	kw_261[2] = {
		{0,0,1,0,0,kw_260},
		{"taylor_series",8,0,1,1}
		},
	kw_262[2] = {
		{0,0,1,0,0,kw_260},
		{"tana",8,0,1,1}
		},
	kw_263[5] = {
		{"global",8,21,2,1,kw_257},
		{"hierarchical",8,3,2,1,kw_259,0.,0.,0,N_mom(lit,surrogateType_hierarchical)},
		{"id_surrogates",13,0,1,0,0,0.,0.,0,N_mom(intsetm1,surrogateFnIndices)},
		{"local",8,1,2,1,kw_261,0.,0.,0,N_mom(lit,surrogateType_local_taylor)},
		{"multipoint",8,1,2,1,kw_262,0.,0.,0,N_mom(lit,surrogateType_multipoint_tana)}
		},
	kw_264[7] = {
		{"hierarchical_tagging",8,0,4,0,0,0.,0.,0,N_mom(true,hierarchicalTags)},
		{"id_model",11,0,1,0,0,0.,0.,0,N_mom(str,idModel)},
		{"nested",8,2,5,1,kw_237,0.,0.,0,N_mom(lit,modelType_nested)},
		{"responses_pointer",11,0,3,0,0,0.,0.,0,N_mom(str,responsesPointer)},
		{"single",8,1,5,1,kw_238,0.,0.,0,N_mom(lit,modelType_single)},
		{"surrogate",8,5,5,1,kw_263,0.,0.,0,N_mom(lit,modelType_surrogate)},
		{"variables_pointer",11,0,2,0,0,0.,0.,0,N_mom(str,variablesPointer)}
		},
	kw_265[6] = {
		{"annotated",8,0,3,0,0,0.,0.,0,N_rem(true,expDataFileAnnotated)},
		{"freeform",8,0,3,0,0,0.,0.,0,N_rem(false,expDataFileAnnotated)},
		{"num_config_variables",0x29,0,4,0,0,0.,0.,0,N_rem(sizet,numExpConfigVars)},
		{"num_experiments",0x29,0,1,0,0,0.,0.,0,N_rem(sizet,numExperiments)},
		{"num_replicates",13,0,2,0,0,0.,0.,0,N_rem(ivec,numReplicates)},
		{"num_std_deviations",0x29,0,5,0,0,0.,0.,0,N_rem(sizet,numExpStdDeviations)}
		},
	kw_266[6] = {
		{"nonlinear_equality_scale_types",0x807,0,2,0,0,0.,0.,3,N_rem(strL,nonlinearEqScaleTypes)},
		{"nonlinear_equality_scales",0x806,0,3,0,0,0.,0.,3,N_rem(RealDL,nonlinearEqScales)},
		{"nonlinear_equality_targets",6,0,1,0,0,0.,0.,3,N_rem(RealDL,nonlinearEqTargets)},
		{"scale_types",0x80f,0,2,0,0,0.,0.,0,N_rem(strL,nonlinearEqScaleTypes)},
		{"scales",0x80e,0,3,0,0,0.,0.,0,N_rem(RealDL,nonlinearEqScales)},
		{"targets",14,0,1,0,0,0.,0.,0,N_rem(RealDL,nonlinearEqTargets)}
		},
	kw_267[8] = {
		{"lower_bounds",14,0,1,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqLowerBnds)},
		{"nonlinear_inequality_lower_bounds",6,0,1,0,0,0.,0.,-1,N_rem(RealDL,nonlinearIneqLowerBnds)},
		{"nonlinear_inequality_scale_types",0x807,0,3,0,0,0.,0.,3,N_rem(strL,nonlinearIneqScaleTypes)},
		{"nonlinear_inequality_scales",0x806,0,4,0,0,0.,0.,3,N_rem(RealDL,nonlinearIneqScales)},
		{"nonlinear_inequality_upper_bounds",6,0,2,0,0,0.,0.,3,N_rem(RealDL,nonlinearIneqUpperBnds)},
		{"scale_types",0x80f,0,3,0,0,0.,0.,0,N_rem(strL,nonlinearIneqScaleTypes)},
		{"scales",0x80e,0,4,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqScales)},
		{"upper_bounds",14,0,2,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqUpperBnds)}
		},
	kw_268[15] = {
		{"calibration_data_file",11,6,4,0,kw_265,0.,0.,0,N_rem(str,expDataFileName)},
		{"calibration_term_scale_types",0x807,0,1,0,0,0.,0.,11,N_rem(strL,primaryRespFnScaleTypes)},
		{"calibration_term_scales",0x806,0,2,0,0,0.,0.,11,N_rem(RealDL,primaryRespFnScales)},
		{"calibration_weights",6,0,3,0,0,0.,0.,11,N_rem(RealDL,primaryRespFnWeights)},
		{"least_squares_data_file",3,6,4,0,kw_265,0.,0.,-4,N_rem(str,expDataFileName)},
		{"least_squares_term_scale_types",0x807,0,1,0,0,0.,0.,7,N_rem(strL,primaryRespFnScaleTypes)},
		{"least_squares_term_scales",0x806,0,2,0,0,0.,0.,7,N_rem(RealDL,primaryRespFnScales)},
		{"least_squares_weights",6,0,3,0,0,0.,0.,7,N_rem(RealDL,primaryRespFnWeights)},
		{"nonlinear_equality_constraints",0x29,6,6,0,kw_266,0.,0.,0,N_rem(sizet,numNonlinearEqConstraints)},
		{"nonlinear_inequality_constraints",0x29,8,5,0,kw_267,0.,0.,0,N_rem(sizet,numNonlinearIneqConstraints)},
		{"num_nonlinear_equality_constraints",0x21,6,6,0,kw_266,0.,0.,-2,N_rem(sizet,numNonlinearEqConstraints)},
		{"num_nonlinear_inequality_constraints",0x21,8,5,0,kw_267,0.,0.,-2,N_rem(sizet,numNonlinearIneqConstraints)},
		{"primary_scale_types",0x80f,0,1,0,0,0.,0.,0,N_rem(strL,primaryRespFnScaleTypes)},
		{"primary_scales",0x80e,0,2,0,0,0.,0.,0,N_rem(RealDL,primaryRespFnScales)},
		{"weights",14,0,3,0,0,0.,0.,0,N_rem(RealDL,primaryRespFnWeights)}
		},
	kw_269[4] = {
		{"absolute",8,0,2,0,0,0.,0.,0,N_rem(lit,fdGradStepType_absolute)},
		{"bounds",8,0,2,0,0,0.,0.,0,N_rem(lit,fdGradStepType_bounds)},
		{"ignore_bounds",8,0,1,0,0,0.,0.,0,N_rem(true,ignoreBounds)},
		{"relative",8,0,2,0,0,0.,0.,0,N_rem(lit,fdGradStepType_relative)}
		},
	kw_270[8] = {
		{"central",8,0,4,0,0,0.,0.,0,N_rem(lit,intervalType_central)},
		{"dakota",8,4,2,0,kw_269,0.,0.,0,N_rem(lit,methodSource_dakota)},
		{"fd_gradient_step_size",0x406,0,5,0,0,0.,0.,1,N_rem(RealL,fdGradStepSize)},
		{"fd_step_size",0x40e,0,5,0,0,0.,0.,0,N_rem(RealL,fdGradStepSize)},
		{"forward",8,0,4,0,0,0.,0.,0,N_rem(lit,intervalType_forward)},
		{"interval_type",8,0,3},
		{"method_source",8,0,1},
		{"vendor",8,0,2,0,0,0.,0.,0,N_rem(lit,methodSource_vendor)}
		},
	kw_271[3] = {
		{0,0,8,0,0,kw_270},
		{"id_analytic_gradients",13,0,2,2,0,0.,0.,0,N_rem(intset,idAnalyticGrads)},
		{"id_numerical_gradients",13,0,1,1,0,0.,0.,0,N_rem(intset,idNumericalGrads)}
		},
	kw_272[2] = {
		{"fd_hessian_step_size",6,0,1,0,0,0.,0.,1,N_rem(RealL,fdHessStepSize)},
		{"fd_step_size",14,0,1,0,0,0.,0.,0,N_rem(RealL,fdHessStepSize)}
		},
	kw_273[1] = {
		{"damped",8,0,1,0,0,0.,0.,0,N_rem(lit,quasiHessianType_damped_bfgs)}
		},
	kw_274[2] = {
		{"bfgs",8,1,1,1,kw_273,0.,0.,0,N_rem(lit,quasiHessianType_bfgs)},
		{"sr1",8,0,1,1,0,0.,0.,0,N_rem(lit,quasiHessianType_sr1)}
		},
	kw_275[8] = {
		{"absolute",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_absolute)},
		{"bounds",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_bounds)},
		{"central",8,0,3,0,0,0.,0.,0,N_rem(true,centralHess)},
		{"forward",8,0,3,0,0,0.,0.,0,N_rem(false,centralHess)},
		{"id_analytic_hessians",13,0,5,0,0,0.,0.,0,N_rem(intset,idAnalyticHessians)},
		{"id_numerical_hessians",13,2,1,0,kw_272,0.,0.,0,N_rem(intset,idNumericalHessians)},
		{"id_quasi_hessians",13,2,4,0,kw_274,0.,0.,0,N_rem(intset,idQuasiHessians)},
		{"relative",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_relative)}
		},
	kw_276[6] = {
		{"nonlinear_equality_scale_types",0x807,0,2,0,0,0.,0.,3,N_rem(strL,nonlinearEqScaleTypes)},
		{"nonlinear_equality_scales",0x806,0,3,0,0,0.,0.,3,N_rem(RealDL,nonlinearEqScales)},
		{"nonlinear_equality_targets",6,0,1,0,0,0.,0.,3,N_rem(RealDL,nonlinearEqTargets)},
		{"scale_types",0x80f,0,2,0,0,0.,0.,0,N_rem(strL,nonlinearEqScaleTypes)},
		{"scales",0x80e,0,3,0,0,0.,0.,0,N_rem(RealDL,nonlinearEqScales)},
		{"targets",14,0,1,0,0,0.,0.,0,N_rem(RealDL,nonlinearEqTargets)}
		},
	kw_277[8] = {
		{"lower_bounds",14,0,1,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqLowerBnds)},
		{"nonlinear_inequality_lower_bounds",6,0,1,0,0,0.,0.,-1,N_rem(RealDL,nonlinearIneqLowerBnds)},
		{"nonlinear_inequality_scale_types",0x807,0,3,0,0,0.,0.,3,N_rem(strL,nonlinearIneqScaleTypes)},
		{"nonlinear_inequality_scales",0x806,0,4,0,0,0.,0.,3,N_rem(RealDL,nonlinearIneqScales)},
		{"nonlinear_inequality_upper_bounds",6,0,2,0,0,0.,0.,3,N_rem(RealDL,nonlinearIneqUpperBnds)},
		{"scale_types",0x80f,0,3,0,0,0.,0.,0,N_rem(strL,nonlinearIneqScaleTypes)},
		{"scales",0x80e,0,4,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqScales)},
		{"upper_bounds",14,0,2,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqUpperBnds)}
		},
	kw_278[11] = {
		{"multi_objective_weights",6,0,4,0,0,0.,0.,10,N_rem(RealDL,primaryRespFnWeights)},
		{"nonlinear_equality_constraints",0x29,6,6,0,kw_276,0.,0.,0,N_rem(sizet,numNonlinearEqConstraints)},
		{"nonlinear_inequality_constraints",0x29,8,5,0,kw_277,0.,0.,0,N_rem(sizet,numNonlinearIneqConstraints)},
		{"num_nonlinear_equality_constraints",0x21,6,6,0,kw_276,0.,0.,-2,N_rem(sizet,numNonlinearEqConstraints)},
		{"num_nonlinear_inequality_constraints",0x21,8,5,0,kw_277,0.,0.,-2,N_rem(sizet,numNonlinearIneqConstraints)},
		{"objective_function_scale_types",0x807,0,2,0,0,0.,0.,2,N_rem(strL,primaryRespFnScaleTypes)},
		{"objective_function_scales",0x806,0,3,0,0,0.,0.,2,N_rem(RealDL,primaryRespFnScales)},
		{"primary_scale_types",0x80f,0,2,0,0,0.,0.,0,N_rem(strL,primaryRespFnScaleTypes)},
		{"primary_scales",0x80e,0,3,0,0,0.,0.,0,N_rem(RealDL,primaryRespFnScales)},
		{"sense",0x80f,0,1,0,0,0.,0.,0,N_rem(strL,primaryRespFnSense)},
		{"weights",14,0,4,0,0,0.,0.,0,N_rem(RealDL,primaryRespFnWeights)}
		},
	kw_279[7] = {
		{"absolute",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_absolute)},
		{"bounds",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_bounds)},
		{"central",8,0,3,0,0,0.,0.,0,N_rem(true,centralHess)},
		{"fd_hessian_step_size",6,0,1,0,0,0.,0.,1,N_rem(RealL,fdHessStepSize)},
		{"fd_step_size",14,0,1,0,0,0.,0.,0,N_rem(RealL,fdHessStepSize)},
		{"forward",8,0,3,0,0,0.,0.,0,N_rem(false,centralHess)},
		{"relative",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_relative)}
		},
	kw_280[1] = {
		{"damped",8,0,1,0,0,0.,0.,0,N_rem(lit,quasiHessianType_damped_bfgs)}
		},
	kw_281[2] = {
		{"bfgs",8,1,1,1,kw_280,0.,0.,0,N_rem(lit,quasiHessianType_bfgs)},
		{"sr1",8,0,1,1,0,0.,0.,0,N_rem(lit,quasiHessianType_sr1)}
		},
	kw_282[19] = {
		{"analytic_gradients",8,0,4,2,0,0.,0.,0,N_rem(lit,gradientType_analytic)},
		{"analytic_hessians",8,0,5,3,0,0.,0.,0,N_rem(lit,hessianType_analytic)},
		{"calibration_terms",0x29,15,3,1,kw_268,0.,0.,0,N_rem(sizet,numLeastSqTerms)},
		{"descriptors",15,0,2,0,0,0.,0.,0,N_rem(strL,responseLabels)},
		{"id_responses",11,0,1,0,0,0.,0.,0,N_rem(str,idResponses)},
		{"least_squares_terms",0x21,15,3,1,kw_268,0.,0.,-3,N_rem(sizet,numLeastSqTerms)},
		{"mixed_gradients",8,2,4,2,kw_271,0.,0.,0,N_rem(lit,gradientType_mixed)},
		{"mixed_hessians",8,8,5,3,kw_275,0.,0.,0,N_rem(lit,hessianType_mixed)},
		{"no_gradients",8,0,4,2,0,0.,0.,0,N_rem(lit,gradientType_none)},
		{"no_hessians",8,0,5,3,0,0.,0.,0,N_rem(lit,hessianType_none)},
		{"num_least_squares_terms",0x21,15,3,1,kw_268,0.,0.,-8,N_rem(sizet,numLeastSqTerms)},
		{"num_objective_functions",0x21,11,3,1,kw_278,0.,0.,4,N_rem(sizet,numObjectiveFunctions)},
		{"num_response_functions",0x21,0,3,1,0,0.,0.,6,N_rem(sizet,numResponseFunctions)},
		{"numerical_gradients",8,8,4,2,kw_270,0.,0.,0,N_rem(lit,gradientType_numerical)},
		{"numerical_hessians",8,7,5,3,kw_279,0.,0.,0,N_rem(lit,hessianType_numerical)},
		{"objective_functions",0x29,11,3,1,kw_278,0.,0.,0,N_rem(sizet,numObjectiveFunctions)},
		{"quasi_hessians",8,2,5,3,kw_281,0.,0.,0,N_rem(lit,hessianType_quasi)},
		{"response_descriptors",7,0,2,0,0,0.,0.,-14,N_rem(strL,responseLabels)},
		{"response_functions",0x29,0,3,1,0,0.,0.,0,N_rem(sizet,numResponseFunctions)}
		},
	kw_283[1] = {
		{"method_list",15,0,1,1,0,0.,0.,0,N_stm(strL,hybridMethodList)}
		},
	kw_284[3] = {
		{"global_method_pointer",11,0,1,1,0,0.,0.,0,N_stm(str,hybridGlobalMethodPointer)},
		{"local_method_pointer",11,0,2,2,0,0.,0.,0,N_stm(str,hybridLocalMethodPointer)},
		{"local_search_probability",10,0,3,0,0,0.,0.,0,N_stm(Real,hybridLSProb)}
		},
	kw_285[1] = {
		{"method_list",15,0,1,1,0,0.,0.,0,N_stm(strL,hybridMethodList)}
		},
	kw_286[5] = {
		{"collaborative",8,1,1,1,kw_283,0.,0.,0,N_stm(lit,hybridType_collaborative)},
		{"coupled",0,3,1,1,kw_284,0.,0.,1,N_stm(lit,hybridType_embedded)},
		{"embedded",8,3,1,1,kw_284,0.,0.,0,N_stm(lit,hybridType_embedded)},
		{"sequential",8,1,1,1,kw_285,0.,0.,0,N_stm(lit,hybridType_sequential)},
		{"uncoupled",0,1,1,1,kw_285,0.,0.,-1,N_stm(lit,hybridType_sequential)}
		},
	kw_287[2] = {
		{"master",8,0,1,1,0,0.,0.,0,N_stm(lit,iteratorScheduling_master)},
		{"peer",8,0,1,1,0,0.,0.,0,N_stm(lit,iteratorScheduling_peer)}
		},
	kw_288[1] = {
		{"seed",9,0,1,0,0,0.,0.,0,N_stm(int,concurrentSeed)}
		},
	kw_289[3] = {
		{"method_pointer",11,0,1,1,0,0.,0.,0,N_stm(str,methodPointer)},
		{"random_starts",9,1,2,0,kw_288,0.,0.,0,N_stm(int,concurrentRandomJobs)},
		{"starting_points",14,0,3,0,0,0.,0.,0,N_stm(RealL,concurrentParameterSets)}
		},
	kw_290[1] = {
		{"seed",9,0,1,0,0,0.,0.,0,N_stm(int,concurrentSeed)}
		},
	kw_291[5] = {
		{"method_pointer",11,0,1,1,0,0.,0.,0,N_stm(str,methodPointer)},
		{"multi_objective_weight_sets",6,0,3,0,0,0.,0.,3,N_stm(RealL,concurrentParameterSets)},
		{"opt_method_pointer",3,0,1,1,0,0.,0.,-2,N_stm(str,methodPointer)},
		{"random_weight_sets",9,1,2,0,kw_290,0.,0.,0,N_stm(int,concurrentRandomJobs)},
		{"weight_sets",14,0,3,0,0,0.,0.,0,N_stm(RealL,concurrentParameterSets)}
		},
	kw_292[1] = {
		{"results_output_file",11,0,1,0,0,0.,0.,0,N_stm(str,resultsOutputFile)}
		},
	kw_293[1] = {
		{"method_pointer",11,0,1,0,0,0.,0.,0,N_stm(str,methodPointer)}
		},
	kw_294[1] = {
		{"tabular_graphics_file",11,0,1,0,0,0.,0.,0,N_stm(str,tabularDataFile)}
		},
	kw_295[10] = {
		{"graphics",8,0,1,0,0,0.,0.,0,N_stm(true,graphicsFlag)},
		{"hybrid",8,5,7,1,kw_286,0.,0.,0,N_stm(lit,strategyType_hybrid)},
		{"iterator_scheduling",8,2,6,0,kw_287},
		{"iterator_servers",9,0,5,0,0,0.,0.,0,N_stm(int,iteratorServers)},
		{"multi_start",8,3,7,1,kw_289,0.,0.,0,N_stm(lit,strategyType_multi_start)},
		{"output_precision",0x29,0,3,0,0,0.,0.,0,N_stm(int,outputPrecision)},
		{"pareto_set",8,5,7,1,kw_291,0.,0.,0,N_stm(lit,strategyType_pareto_set)},
		{"results_output",8,1,4,0,kw_292,0.,0.,0,N_stm(true,resultsOutputFlag)},
		{"single_method",8,1,7,1,kw_293,0.,0.,0,N_stm(lit,strategyType_single_method)},
		{"tabular_graphics_data",8,1,2,0,kw_294,0.,0.,0,N_stm(true,tabularDataFlag)}
		},
	kw_296[6] = {
		{"aleatory",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_ALEATORY_UNCERTAIN_VIEW)},
		{"all",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_ALL_VIEW)},
		{"design",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_DESIGN_VIEW)},
		{"epistemic",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_EPISTEMIC_UNCERTAIN_VIEW)},
		{"state",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_STATE_VIEW)},
		{"uncertain",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_UNCERTAIN_VIEW)}
		},
	kw_297[10] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,betaUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(RealLb,betaUncBetas)},
		{"buv_alphas",6,0,1,1,0,0.,0.,-2,N_vam(RealLb,betaUncAlphas)},
		{"buv_betas",6,0,2,2,0,0.,0.,-2,N_vam(RealLb,betaUncBetas)},
		{"buv_descriptors",7,0,5,0,0,0.,0.,3,N_vae(caulbl,CAUVar_beta)},
		{"buv_lower_bounds",6,0,3,3,0,0.,0.,3,N_vam(rvec,betaUncLowerBnds)},
		{"buv_upper_bounds",6,0,4,4,0,0.,0.,3,N_vam(rvec,betaUncUpperBnds)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(caulbl,CAUVar_beta)},
		{"lower_bounds",14,0,3,3,0,0.,0.,0,N_vam(rvec,betaUncLowerBnds)},
		{"upper_bounds",14,0,4,4,0,0.,0.,0,N_vam(rvec,betaUncUpperBnds)}
		},
	kw_298[4] = {
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(dauilbl,DAUIVar_binomial)},
		{"num_trials",13,0,2,2,0,0.,0.,0,N_vam(ivec,binomialUncNumTrials)},
		{"prob_per_trial",6,0,1,1,0,0.,0.,1,N_vam(rvec,binomialUncProbPerTrial)},
		{"probability_per_trial",14,0,1,1,0,0.,0.,0,N_vam(rvec,binomialUncProbPerTrial)}
		},
	kw_299[12] = {
		{"cdv_descriptors",7,0,6,0,0,0.,0.,6,N_vam(strL,continuousDesignLabels)},
		{"cdv_initial_point",6,0,1,0,0,0.,0.,6,N_vam(rvec,continuousDesignVars)},
		{"cdv_lower_bounds",6,0,2,0,0,0.,0.,6,N_vam(rvec,continuousDesignLowerBnds)},
		{"cdv_scale_types",0x807,0,4,0,0,0.,0.,6,N_vam(strL,continuousDesignScaleTypes)},
		{"cdv_scales",0x806,0,5,0,0,0.,0.,6,N_vam(rvec,continuousDesignScales)},
		{"cdv_upper_bounds",6,0,3,0,0,0.,0.,6,N_vam(rvec,continuousDesignUpperBnds)},
		{"descriptors",15,0,6,0,0,0.,0.,0,N_vam(strL,continuousDesignLabels)},
		{"initial_point",14,0,1,0,0,0.,0.,0,N_vam(rvec,continuousDesignVars)},
		{"lower_bounds",14,0,2,0,0,0.,0.,0,N_vam(rvec,continuousDesignLowerBnds)},
		{"scale_types",0x80f,0,4,0,0,0.,0.,0,N_vam(strL,continuousDesignScaleTypes)},
		{"scales",0x80e,0,5,0,0,0.,0.,0,N_vam(rvec,continuousDesignScales)},
		{"upper_bounds",14,0,3,0,0,0.,0.,0,N_vam(rvec,continuousDesignUpperBnds)}
		},
	kw_300[9] = {
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(ceulbl,CEUVar_interval)},
		{"interval_probabilities",14,0,2,0,0,0.,0.,0,N_vam(newrvec,Var_Info_CIp)},
		{"interval_probs",6,0,2,0,0,0.,0.,-1,N_vam(newrvec,Var_Info_CIp)},
		{"iuv_descriptors",7,0,5,0,0,0.,0.,-3,N_vae(ceulbl,CEUVar_interval)},
		{"iuv_interval_probs",6,0,2,0,0,0.,0.,-3,N_vam(newrvec,Var_Info_CIp)},
		{"iuv_num_intervals",5,0,1,0,0,0.,0.,2,N_vam(newiarray,Var_Info_nCI)},
		{"lower_bounds",14,0,3,1,0,0.,0.,0,N_vam(newrvec,Var_Info_CIlb)},
		{"num_intervals",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nCI)},
		{"upper_bounds",14,0,4,2,0,0.,0.,0,N_vam(newrvec,Var_Info_CIub)}
		},
	kw_301[8] = {
		{"csv_descriptors",7,0,4,0,0,0.,0.,4,N_vam(strL,continuousStateLabels)},
		{"csv_initial_state",6,0,1,0,0,0.,0.,4,N_vam(rvec,continuousStateVars)},
		{"csv_lower_bounds",6,0,2,0,0,0.,0.,4,N_vam(rvec,continuousStateLowerBnds)},
		{"csv_upper_bounds",6,0,3,0,0,0.,0.,4,N_vam(rvec,continuousStateUpperBnds)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,continuousStateLabels)},
		{"initial_state",14,0,1,0,0,0.,0.,0,N_vam(rvec,continuousStateVars)},
		{"lower_bounds",14,0,2,0,0,0.,0.,0,N_vam(rvec,continuousStateLowerBnds)},
		{"upper_bounds",14,0,3,0,0,0.,0.,0,N_vam(rvec,continuousStateUpperBnds)}
		},
	kw_302[8] = {
		{"ddv_descriptors",7,0,4,0,0,0.,0.,4,N_vam(strL,discreteDesignRangeLabels)},
		{"ddv_initial_point",5,0,1,0,0,0.,0.,4,N_vam(ivec,discreteDesignRangeVars)},
		{"ddv_lower_bounds",5,0,2,0,0,0.,0.,4,N_vam(ivec,discreteDesignRangeLowerBnds)},
		{"ddv_upper_bounds",5,0,3,0,0,0.,0.,4,N_vam(ivec,discreteDesignRangeUpperBnds)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteDesignRangeLabels)},
		{"initial_point",13,0,1,0,0,0.,0.,0,N_vam(ivec,discreteDesignRangeVars)},
		{"lower_bounds",13,0,2,0,0,0.,0.,0,N_vam(ivec,discreteDesignRangeLowerBnds)},
		{"upper_bounds",13,0,3,0,0,0.,0.,0,N_vam(ivec,discreteDesignRangeUpperBnds)}
		},
	kw_303[4] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteDesignSetIntLabels)},
		{"initial_point",13,0,1,0,0,0.,0.,0,N_vam(ivec,discreteDesignSetIntVars)},
		{"num_set_values",13,0,2,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nddsi)},
		{"set_values",13,0,3,1,0,0.,0.,0,N_vam(newivec,Var_Info_ddsi)}
		},
	kw_304[4] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteDesignSetRealLabels)},
		{"initial_point",14,0,1,0,0,0.,0.,0,N_vam(rvec,discreteDesignSetRealVars)},
		{"num_set_values",13,0,2,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nddsr)},
		{"set_values",14,0,3,1,0,0.,0.,0,N_vam(newrvec,Var_Info_ddsr)}
		},
	kw_305[8] = {
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(deuilbl,DEUIVar_interval)},
		{"interval_probabilities",14,0,2,0,0,0.,0.,0,N_vam(newrvec,Var_Info_DIp)},
		{"interval_probs",6,0,2,0,0,0.,0.,-1,N_vam(newrvec,Var_Info_DIp)},
		{"lower_bounds",13,0,3,1,0,0.,0.,0,N_vam(newivec,Var_Info_DIlb)},
		{"num_intervals",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nDI)},
		{"range_probabilities",6,0,2,0,0,0.,0.,-4,N_vam(newrvec,Var_Info_DIp)},
		{"range_probs",6,0,2,0,0,0.,0.,-5,N_vam(newrvec,Var_Info_DIp)},
		{"upper_bounds",13,0,4,2,0,0.,0.,0,N_vam(newivec,Var_Info_DIub)}
		},
	kw_306[8] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteStateRangeLabels)},
		{"dsv_descriptors",7,0,4,0,0,0.,0.,-1,N_vam(strL,discreteStateRangeLabels)},
		{"dsv_initial_state",5,0,1,0,0,0.,0.,3,N_vam(ivec,discreteStateRangeVars)},
		{"dsv_lower_bounds",5,0,2,0,0,0.,0.,3,N_vam(ivec,discreteStateRangeLowerBnds)},
		{"dsv_upper_bounds",5,0,3,0,0,0.,0.,3,N_vam(ivec,discreteStateRangeUpperBnds)},
		{"initial_state",13,0,1,0,0,0.,0.,0,N_vam(ivec,discreteStateRangeVars)},
		{"lower_bounds",13,0,2,0,0,0.,0.,0,N_vam(ivec,discreteStateRangeLowerBnds)},
		{"upper_bounds",13,0,3,0,0,0.,0.,0,N_vam(ivec,discreteStateRangeUpperBnds)}
		},
	kw_307[4] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteStateSetIntLabels)},
		{"initial_state",13,0,1,0,0,0.,0.,0,N_vam(ivec,discreteStateSetIntVars)},
		{"num_set_values",13,0,2,0,0,0.,0.,0,N_vam(newiarray,Var_Info_ndssi)},
		{"set_values",13,0,3,1,0,0.,0.,0,N_vam(newivec,Var_Info_dssi)}
		},
	kw_308[4] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteStateSetRealLabels)},
		{"initial_state",14,0,1,0,0,0.,0.,0,N_vam(rvec,discreteStateSetRealVars)},
		{"num_set_values",13,0,2,0,0,0.,0.,0,N_vam(newiarray,Var_Info_ndssr)},
		{"set_values",14,0,3,1,0,0.,0.,0,N_vam(newrvec,Var_Info_dssr)}
		},
	kw_309[5] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(deuilbl,DEUIVar_set_int)},
		{"num_set_values",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_ndusi)},
		{"set_probabilities",14,0,3,0,0,0.,0.,0,N_vam(newrvec,Var_Info_DSIp)},
		{"set_probs",6,0,3,0,0,0.,0.,-1,N_vam(newrvec,Var_Info_DSIp)},
		{"set_values",13,0,2,1,0,0.,0.,0,N_vam(newivec,Var_Info_dusi)}
		},
	kw_310[5] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(deurlbl,DEURVar_set_real)},
		{"num_set_values",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_ndusr)},
		{"set_probabilities",14,0,3,0,0,0.,0.,0,N_vam(newrvec,Var_Info_DSRp)},
		{"set_probs",6,0,3,0,0,0.,0.,-1,N_vam(newrvec,Var_Info_DSRp)},
		{"set_values",14,0,2,1,0,0.,0.,0,N_vam(newrvec,Var_Info_dusr)}
		},
	kw_311[4] = {
		{"betas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,exponentialUncBetas)},
		{"descriptors",15,0,2,0,0,0.,0.,0,N_vae(caulbl,CAUVar_exponential)},
		{"euv_betas",6,0,1,1,0,0.,0.,-2,N_vam(RealLb,exponentialUncBetas)},
		{"euv_descriptors",7,0,2,0,0,0.,0.,-2,N_vae(caulbl,CAUVar_exponential)}
		},
	kw_312[6] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,frechetUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(rvec,frechetUncBetas)},
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(caulbl,CAUVar_frechet)},
		{"fuv_alphas",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,frechetUncAlphas)},
		{"fuv_betas",6,0,2,2,0,0.,0.,-3,N_vam(rvec,frechetUncBetas)},
		{"fuv_descriptors",7,0,3,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_frechet)}
		},
	kw_313[6] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,gammaUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(RealLb,gammaUncBetas)},
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(caulbl,CAUVar_gamma)},
		{"gauv_alphas",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,gammaUncAlphas)},
		{"gauv_betas",6,0,2,2,0,0.,0.,-3,N_vam(RealLb,gammaUncBetas)},
		{"gauv_descriptors",7,0,3,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_gamma)}
		},
	kw_314[3] = {
		{"descriptors",15,0,2,0,0,0.,0.,0,N_vae(dauilbl,DAUIVar_geometric)},
		{"prob_per_trial",6,0,1,1,0,0.,0.,1,N_vam(rvec,geometricUncProbPerTrial)},
		{"probability_per_trial",14,0,1,1,0,0.,0.,0,N_vam(rvec,geometricUncProbPerTrial)}
		},
	kw_315[6] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,gumbelUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(rvec,gumbelUncBetas)},
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(caulbl,CAUVar_gumbel)},
		{"guuv_alphas",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,gumbelUncAlphas)},
		{"guuv_betas",6,0,2,2,0,0.,0.,-3,N_vam(rvec,gumbelUncBetas)},
		{"guuv_descriptors",7,0,3,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_gumbel)}
		},
	kw_316[10] = {
		{"abscissas",14,0,2,1,0,0.,0.,0,N_vam(newrvec,Var_Info_hba)},
		{"counts",14,0,3,2,0,0.,0.,0,N_vam(newrvec,Var_Info_hbc)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_histogram_bin)},
		{"huv_bin_abscissas",6,0,2,1,0,0.,0.,-3,N_vam(newrvec,Var_Info_hba)},
		{"huv_bin_counts",6,0,3,2,0,0.,0.,-3,N_vam(newrvec,Var_Info_hbc)},
		{"huv_bin_descriptors",7,0,4,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_histogram_bin)},
		{"huv_bin_ordinates",6,0,3,2,0,0.,0.,3,N_vam(newrvec,Var_Info_hbo)},
		{"huv_num_bin_pairs",5,0,1,0,0,0.,0.,1,N_vam(newiarray,Var_Info_nhbp)},
		{"num_pairs",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nhbp)},
		{"ordinates",14,0,3,2,0,0.,0.,0,N_vam(newrvec,Var_Info_hbo)}
		},
	kw_317[8] = {
		{"abscissas",14,0,2,1,0,0.,0.,0,N_vam(newrvec,Var_Info_hpa)},
		{"counts",14,0,3,2,0,0.,0.,0,N_vam(newrvec,Var_Info_hpc)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(daurlbl,DAURVar_histogram_point)},
		{"huv_num_point_pairs",5,0,1,0,0,0.,0.,4,N_vam(newiarray,Var_Info_nhpp)},
		{"huv_point_abscissas",6,0,2,1,0,0.,0.,-4,N_vam(newrvec,Var_Info_hpa)},
		{"huv_point_counts",6,0,3,2,0,0.,0.,-4,N_vam(newrvec,Var_Info_hpc)},
		{"huv_point_descriptors",7,0,4,0,0,0.,0.,-4,N_vae(daurlbl,DAURVar_histogram_point)},
		{"num_pairs",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nhpp)}
		},
	kw_318[4] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(dauilbl,DAUIVar_hypergeometric)},
		{"num_drawn",13,0,3,3,0,0.,0.,0,N_vam(ivec,hyperGeomUncNumDrawn)},
		{"selected_population",13,0,2,2,0,0.,0.,0,N_vam(ivec,hyperGeomUncSelectedPop)},
		{"total_population",13,0,1,1,0,0.,0.,0,N_vam(ivec,hyperGeomUncTotalPop)}
		},
	kw_319[2] = {
		{"lnuv_zetas",6,0,1,1,0,0.,0.,1,N_vam(RealLb,lognormalUncZetas)},
		{"zetas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,lognormalUncZetas)}
		},
	kw_320[4] = {
		{"error_factors",14,0,1,1,0,0.,0.,0,N_vam(RealLb,lognormalUncErrFacts)},
		{"lnuv_error_factors",6,0,1,1,0,0.,0.,-1,N_vam(RealLb,lognormalUncErrFacts)},
		{"lnuv_std_deviations",6,0,1,1,0,0.,0.,1,N_vam(RealLb,lognormalUncStdDevs)},
		{"std_deviations",14,0,1,1,0,0.,0.,0,N_vam(RealLb,lognormalUncStdDevs)}
		},
	kw_321[10] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_lognormal)},
		{"lambdas",14,2,1,1,kw_319,0.,0.,0,N_vam(rvec,lognormalUncLambdas)},
		{"lnuv_descriptors",7,0,4,0,0,0.,0.,-2,N_vae(caulbl,CAUVar_lognormal)},
		{"lnuv_lambdas",6,2,1,1,kw_319,0.,0.,-2,N_vam(rvec,lognormalUncLambdas)},
		{"lnuv_lower_bounds",6,0,2,0,0,0.,0.,3,N_vam(RealLb,lognormalUncLowerBnds)},
		{"lnuv_means",6,4,1,1,kw_320,0.,0.,3,N_vam(RealLb,lognormalUncMeans)},
		{"lnuv_upper_bounds",6,0,3,0,0,0.,0.,3,N_vam(RealUb,lognormalUncUpperBnds)},
		{"lower_bounds",14,0,2,0,0,0.,0.,0,N_vam(RealLb,lognormalUncLowerBnds)},
		{"means",14,4,1,1,kw_320,0.,0.,0,N_vam(RealLb,lognormalUncMeans)},
		{"upper_bounds",14,0,3,0,0,0.,0.,0,N_vam(RealUb,lognormalUncUpperBnds)}
		},
	kw_322[6] = {
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(caulbl,CAUVar_loguniform)},
		{"lower_bounds",14,0,1,1,0,0.,0.,0,N_vam(RealLb,loguniformUncLowerBnds)},
		{"luuv_descriptors",7,0,3,0,0,0.,0.,-2,N_vae(caulbl,CAUVar_loguniform)},
		{"luuv_lower_bounds",6,0,1,1,0,0.,0.,-2,N_vam(RealLb,loguniformUncLowerBnds)},
		{"luuv_upper_bounds",6,0,2,2,0,0.,0.,1,N_vam(RealUb,loguniformUncUpperBnds)},
		{"upper_bounds",14,0,2,2,0,0.,0.,0,N_vam(RealUb,loguniformUncUpperBnds)}
		},
	kw_323[4] = {
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(dauilbl,DAUIVar_negative_binomial)},
		{"num_trials",13,0,2,2,0,0.,0.,0,N_vam(ivec,negBinomialUncNumTrials)},
		{"prob_per_trial",6,0,1,1,0,0.,0.,1,N_vam(rvec,negBinomialUncProbPerTrial)},
		{"probability_per_trial",14,0,1,1,0,0.,0.,0,N_vam(rvec,negBinomialUncProbPerTrial)}
		},
	kw_324[10] = {
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(caulbl,CAUVar_normal)},
		{"lower_bounds",14,0,3,0,0,0.,0.,0,N_vam(rvec,normalUncLowerBnds)},
		{"means",14,0,1,1,0,0.,0.,0,N_vam(rvec,normalUncMeans)},
		{"nuv_descriptors",7,0,5,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_normal)},
		{"nuv_lower_bounds",6,0,3,0,0,0.,0.,-3,N_vam(rvec,normalUncLowerBnds)},
		{"nuv_means",6,0,1,1,0,0.,0.,-3,N_vam(rvec,normalUncMeans)},
		{"nuv_std_deviations",6,0,2,2,0,0.,0.,2,N_vam(RealLb,normalUncStdDevs)},
		{"nuv_upper_bounds",6,0,4,0,0,0.,0.,2,N_vam(rvec,normalUncUpperBnds)},
		{"std_deviations",14,0,2,2,0,0.,0.,0,N_vam(RealLb,normalUncStdDevs)},
		{"upper_bounds",14,0,4,0,0,0.,0.,0,N_vam(rvec,normalUncUpperBnds)}
		},
	kw_325[2] = {
		{"descriptors",15,0,2,0,0,0.,0.,0,N_vae(dauilbl,DAUIVar_poisson)},
		{"lambdas",14,0,1,1,0,0.,0.,0,N_vam(rvec,poissonUncLambdas)}
		},
	kw_326[8] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_triangular)},
		{"lower_bounds",14,0,2,2,0,0.,0.,0,N_vam(RealLb,triangularUncLowerBnds)},
		{"modes",14,0,1,1,0,0.,0.,0,N_vam(rvec,triangularUncModes)},
		{"tuv_descriptors",7,0,4,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_triangular)},
		{"tuv_lower_bounds",6,0,2,2,0,0.,0.,-3,N_vam(RealLb,triangularUncLowerBnds)},
		{"tuv_modes",6,0,1,1,0,0.,0.,-3,N_vam(rvec,triangularUncModes)},
		{"tuv_upper_bounds",6,0,3,3,0,0.,0.,1,N_vam(RealUb,triangularUncUpperBnds)},
		{"upper_bounds",14,0,3,3,0,0.,0.,0,N_vam(RealUb,triangularUncUpperBnds)}
		},
	kw_327[6] = {
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(caulbl,CAUVar_uniform)},
		{"lower_bounds",14,0,1,1,0,0.,0.,0,N_vam(RealLb,uniformUncLowerBnds)},
		{"upper_bounds",14,0,2,2,0,0.,0.,0,N_vam(RealUb,uniformUncUpperBnds)},
		{"uuv_descriptors",7,0,3,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_uniform)},
		{"uuv_lower_bounds",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,uniformUncLowerBnds)},
		{"uuv_upper_bounds",6,0,2,2,0,0.,0.,-3,N_vam(RealUb,uniformUncUpperBnds)}
		},
	kw_328[6] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,weibullUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(RealLb,weibullUncBetas)},
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(caulbl,CAUVar_weibull)},
		{"wuv_alphas",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,weibullUncAlphas)},
		{"wuv_betas",6,0,2,2,0,0.,0.,-3,N_vam(RealLb,weibullUncBetas)},
		{"wuv_descriptors",7,0,3,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_weibull)}
		},
	kw_329[37] = {
		{"active",8,6,2,0,kw_296},
		{"beta_uncertain",0x19,10,14,0,kw_297,0.,0.,0,N_vam(pintz,numBetaUncVars)},
		{"binomial_uncertain",0x19,4,21,0,kw_298,0.,0.,0,N_vam(pintz,numBinomialUncVars)},
		{"continuous_design",0x19,12,4,0,kw_299,0.,0.,0,N_vam(pintz,numContinuousDesVars)},
		{"continuous_interval_uncertain",0x19,9,27,0,kw_300,0.,0.,0,N_vam(pintz,numContinuousIntervalUncVars)},
		{"continuous_state",0x19,8,31,0,kw_301,0.,0.,0,N_vam(pintz,numContinuousStateVars)},
		{"discrete_design_range",0x19,8,5,0,kw_302,0.,0.,0,N_vam(pintz,numDiscreteDesRangeVars)},
		{"discrete_design_set_integer",0x19,4,6,0,kw_303,0.,0.,0,N_vam(pintz,numDiscreteDesSetIntVars)},
		{"discrete_design_set_real",0x19,4,7,0,kw_304,0.,0.,0,N_vam(pintz,numDiscreteDesSetRealVars)},
		{"discrete_interval_uncertain",0x19,8,28,0,kw_305,0.,0.,0,N_vam(pintz,numDiscreteIntervalUncVars)},
		{"discrete_state_range",0x19,8,32,0,kw_306,0.,0.,0,N_vam(pintz,numDiscreteStateRangeVars)},
		{"discrete_state_set_integer",0x19,4,33,0,kw_307,0.,0.,0,N_vam(pintz,numDiscreteStateSetIntVars)},
		{"discrete_state_set_real",0x19,4,34,0,kw_308,0.,0.,0,N_vam(pintz,numDiscreteStateSetRealVars)},
		{"discrete_uncertain_range",0x11,8,28,0,kw_305,0.,0.,-4,N_vam(pintz,numDiscreteIntervalUncVars)},
		{"discrete_uncertain_set_integer",0x19,5,29,0,kw_309,0.,0.,0,N_vam(pintz,numDiscreteUncSetIntVars)},
		{"discrete_uncertain_set_real",0x19,5,30,0,kw_310,0.,0.,0,N_vam(pintz,numDiscreteUncSetRealVars)},
		{"exponential_uncertain",0x19,4,13,0,kw_311,0.,0.,0,N_vam(pintz,numExponentialUncVars)},
		{"frechet_uncertain",0x19,6,17,0,kw_312,0.,0.,0,N_vam(pintz,numFrechetUncVars)},
		{"gamma_uncertain",0x19,6,15,0,kw_313,0.,0.,0,N_vam(pintz,numGammaUncVars)},
		{"geometric_uncertain",0x19,3,23,0,kw_314,0.,0.,0,N_vam(pintz,numGeometricUncVars)},
		{"gumbel_uncertain",0x19,6,16,0,kw_315,0.,0.,0,N_vam(pintz,numGumbelUncVars)},
		{"histogram_bin_uncertain",0x19,10,19,0,kw_316,0.,0.,0,N_vam(pintz,numHistogramBinUncVars)},
		{"histogram_point_uncertain",0x19,8,25,0,kw_317,0.,0.,0,N_vam(pintz,numHistogramPtUncVars)},
		{"hypergeometric_uncertain",0x19,4,24,0,kw_318,0.,0.,0,N_vam(pintz,numHyperGeomUncVars)},
		{"id_variables",11,0,1,0,0,0.,0.,0,N_vam(str,idVariables)},
		{"interval_uncertain",0x11,9,27,0,kw_300,0.,0.,-21,N_vam(pintz,numContinuousIntervalUncVars)},
		{"lognormal_uncertain",0x19,10,9,0,kw_321,0.,0.,0,N_vam(pintz,numLognormalUncVars)},
		{"loguniform_uncertain",0x19,6,11,0,kw_322,0.,0.,0,N_vam(pintz,numLoguniformUncVars)},
		{"mixed",8,0,3,0,0,0.,0.,0,N_vam(type,varsDomain_MIXED_DOMAIN)},
		{"negative_binomial_uncertain",0x19,4,22,0,kw_323,0.,0.,0,N_vam(pintz,numNegBinomialUncVars)},
		{"normal_uncertain",0x19,10,8,0,kw_324,0.,0.,0,N_vam(pintz,numNormalUncVars)},
		{"poisson_uncertain",0x19,2,20,0,kw_325,0.,0.,0,N_vam(pintz,numPoissonUncVars)},
		{"relaxed",8,0,3,0,0,0.,0.,0,N_vam(type,varsDomain_RELAXED_DOMAIN)},
		{"triangular_uncertain",0x19,8,12,0,kw_326,0.,0.,0,N_vam(pintz,numTriangularUncVars)},
		{"uncertain_correlation_matrix",14,0,26,0,0,0.,0.,0,N_vam(newrvec,Var_Info_ucm)},
		{"uniform_uncertain",0x19,6,10,0,kw_327,0.,0.,0,N_vam(pintz,numUniformUncVars)},
		{"weibull_uncertain",0x19,6,18,0,kw_328,0.,0.,0,N_vam(pintz,numWeibullUncVars)}
		},
	kw_330[6] = {
		{"interface",0x308,8,5,5,kw_14,0.,0.,0,N_ifm3(start,0,stop)},
		{"method",0x308,86,2,2,kw_234,0.,0.,0,N_mdm3(start,0,stop)},
		{"model",8,7,3,3,kw_264,0.,0.,0,N_mom3(start,0,stop)},
		{"responses",0x308,19,6,6,kw_282,0.,0.,0,N_rem3(start,0,stop)},
		{"strategy",0x108,10,1,1,kw_295,0.,0.,0,NIDRProblemDescDB::strategy_start},
		{"variables",0x308,37,4,4,kw_329,0.,0.,0,N_vam3(start,0,stop)}
		};

} // namespace Dakota

#ifdef __cplusplus
extern "C" {
#endif
KeyWord Dakota_Keyword_Top = {"KeywordTop",0,6,0,0,Dakota::kw_330};
#ifdef __cplusplus
}
#endif

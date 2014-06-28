
namespace Dakota {

/** 1289 distinct keywords (plus 200 aliases) **/

static KeyWord
	kw_1[2] = {
		{"input",11,0,1,0,0,0.,0.,0,N_stm(str,postRunInput)},
		{"output",11,0,2,0,0,0.,0.,0,N_stm(str,postRunOutput)}
		},
	kw_2[2] = {
		{"input",11,0,1,0,0,0.,0.,0,N_stm(str,preRunInput)},
		{"output",11,0,2,0,0,0.,0.,0,N_stm(str,preRunOutput)}
		},
	kw_3[1] = {
		{"stop_restart",0x29,0,1,0,0,0.,0.,0,N_stm(int,stopRestart)}
		},
	kw_4[1] = {
		{"results_output_file",11,0,1,0,0,0.,0.,0,N_stm(str,resultsOutputFile)}
		},
	kw_5[2] = {
		{"input",11,0,1,0,0,0.,0.,0,N_stm(str,runInput)},
		{"output",11,0,2,0,0,0.,0.,0,N_stm(str,runOutput)}
		},
	kw_6[2] = {
		{"tabular_data_file",11,0,1,0,0,0.,0.,0,N_stm(str,tabularDataFile)},
		{"tabular_graphics_file",3,0,1,0,0,0.,0.,-1,N_stm(str,tabularDataFile)}
		},
	kw_7[15] = {
		{"check",8,0,1,0,0,0.,0.,0,N_stm(true,checkFlag)},
		{"error_file",11,0,3,0,0,0.,0.,0,N_stm(str,errorFile)},
		{"graphics",8,0,9,0,0,0.,0.,0,N_stm(true,graphicsFlag)},
		{"method_pointer",3,0,13,0,0,0.,0.,10,N_stm(str,topMethodPointer)},
		{"output_file",11,0,2,0,0,0.,0.,0,N_stm(str,outputFile)},
		{"output_precision",0x29,0,11,0,0,0.,0.,0,N_stm(int,outputPrecision)},
		{"post_run",8,2,8,0,kw_1,0.,0.,0,N_stm(true,postRunFlag)},
		{"pre_run",8,2,6,0,kw_2,0.,0.,0,N_stm(true,preRunFlag)},
		{"read_restart",11,1,4,0,kw_3,0.,0.,0,N_stm(str,readRestart)},
		{"results_output",8,1,12,0,kw_4,0.,0.,0,N_stm(true,resultsOutputFlag)},
		{"run",8,2,7,0,kw_5,0.,0.,0,N_stm(true,runFlag)},
		{"tabular_data",8,2,10,0,kw_6,0.,0.,0,N_stm(true,tabularDataFlag)},
		{"tabular_graphics_data",0,2,10,0,kw_6,0.,0.,-1,N_stm(true,tabularDataFlag)},
		{"top_method_pointer",11,0,13,0,0,0.,0.,0,N_stm(str,topMethodPointer)},
		{"write_restart",11,0,5,0,0,0.,0.,0,N_stm(str,writeRestart)}
		},
	kw_8[1] = {
		{"cache_tolerance",10,0,1,0,0,0.,0.,0,N_ifm(Real,nearbyEvalCacheTol)}
		},
	kw_9[4] = {
		{"active_set_vector",8,0,1,0,0,0.,0.,0,N_ifm(false,activeSetVectorFlag)},
		{"evaluation_cache",8,0,2,0,0,0.,0.,0,N_ifm(false,evalCacheFlag)},
		{"restart_file",8,0,4,0,0,0.,0.,0,N_ifm(false,restartFileFlag)},
		{"strict_cache_equality",8,1,3,0,kw_8,0.,0.,0,N_ifm(true,nearbyEvalCacheFlag)}
		},
	kw_10[1] = {
		{"processors_per_analysis",0x19,0,1,0,0,0.,0.,0,N_ifm(pint,procsPerAnalysis)}
		},
	kw_11[4] = {
		{"abort",8,0,1,1,0,0.,0.,0,N_ifm(lit,failAction_abort)},
		{"continuation",8,0,1,1,0,0.,0.,0,N_ifm(lit,failAction_continuation)},
		{"recover",14,0,1,1,0,0.,0.,0,N_ifm(Rlit,TYPE_DATA_failAction_recover)},
		{"retry",9,0,1,1,0,0.,0.,0,N_ifm(ilit,TYPE_DATA_failAction_retry)}
		},
	kw_12[1] = {
		{"numpy",8,0,1,0,0,0.,0.,0,N_ifm(true,numpyFlag)}
		},
	kw_13[2] = {
		{"copy",8,0,1,0,0,0.,0.,0,N_ifm(true,templateCopy)},
		{"replace",8,0,2,0,0,0.,0.,0,N_ifm(true,templateReplace)}
		},
	kw_14[7] = {
		{"dir_save",0,0,3,0,0,0.,0.,2,N_ifm(true,dirSave)},
		{"dir_tag",0,0,2,0,0,0.,0.,2,N_ifm(true,dirTag)},
		{"directory_save",8,0,3,0,0,0.,0.,0,N_ifm(true,dirSave)},
		{"directory_tag",8,0,2,0,0,0.,0.,0,N_ifm(true,dirTag)},
		{"named",11,0,1,0,0,0.,0.,0,N_ifm(str,workDir)},
		{"template_directory",11,2,4,0,kw_13,0.,0.,0,N_ifm(str,templateDir)},
		{"template_files",15,2,4,0,kw_13,0.,0.,0,N_ifm(strL,templateFiles)}
		},
	kw_15[9] = {
		{"allow_existing_results",8,0,3,0,0,0.,0.,0,N_ifm(true,allowExistingResultsFlag)},
		{"aprepro",8,0,5,0,0,0.,0.,0,N_ifm(true,apreproFlag)},
		{"dprepro",0,0,5,0,0,0.,0.,-1,N_ifm(true,apreproFlag)},
		{"file_save",8,0,7,0,0,0.,0.,0,N_ifm(true,fileSaveFlag)},
		{"file_tag",8,0,6,0,0,0.,0.,0,N_ifm(true,fileTagFlag)},
		{"parameters_file",11,0,1,0,0,0.,0.,0,N_ifm(str,parametersFile)},
		{"results_file",11,0,2,0,0,0.,0.,0,N_ifm(str,resultsFile)},
		{"verbatim",8,0,4,0,0,0.,0.,0,N_ifm(true,verbatimFlag)},
		{"work_directory",8,7,8,0,kw_14,0.,0.,0,N_ifm(true,useWorkdir)}
		},
	kw_16[12] = {
		{"analysis_components",15,0,1,0,0,0.,0.,0,N_ifm(str2D,analysisComponents)},
		{"deactivate",8,4,6,0,kw_9},
		{"direct",8,1,4,1,kw_10,0.,0.,0,N_ifm(lit,interfaceType_direct)},
		{"failure_capture",8,4,5,0,kw_11},
		{"fork",8,9,4,1,kw_15,0.,0.,0,N_ifm(lit,interfaceType_fork)},
		{"grid",8,0,4,1,0,0.,0.,0,N_ifm(lit,interfaceType_grid)},
		{"input_filter",11,0,2,0,0,0.,0.,0,N_ifm(str,inputFilter)},
		{"matlab",8,0,4,1,0,0.,0.,0,N_ifm(lit,interfaceType_matlab)},
		{"output_filter",11,0,3,0,0,0.,0.,0,N_ifm(str,outputFilter)},
		{"python",8,1,4,1,kw_12,0.,0.,0,N_ifm(lit,interfaceType_python)},
		{"scilab",8,0,4,1,0,0.,0.,0,N_ifm(lit,interfaceType_scilab)},
		{"system",8,9,4,1,kw_15,0.,0.,0,N_ifm(lit,interfaceType_system)}
		},
	kw_17[2] = {
		{"master",8,0,1,1,0,0.,0.,0,N_ifm(type,analysisScheduling_MASTER_SCHEDULING)},
		{"peer",8,0,1,1,0,0.,0.,0,N_ifm(type,analysisScheduling_PEER_SCHEDULING)}
		},
	kw_18[2] = {
		{"dynamic",8,0,1,1,0,0.,0.,0,N_ifm(type,asynchLocalEvalScheduling_DYNAMIC_SCHEDULING)},
		{"static",8,0,1,1,0,0.,0.,0,N_ifm(type,asynchLocalEvalScheduling_STATIC_SCHEDULING)}
		},
	kw_19[3] = {
		{"analysis_concurrency",0x19,0,3,0,0,0.,0.,0,N_ifm(pint,asynchLocalAnalysisConcurrency)},
		{"evaluation_concurrency",0x19,0,1,0,0,0.,0.,0,N_ifm(pint,asynchLocalEvalConcurrency)},
		{"local_evaluation_scheduling",8,2,2,0,kw_18}
		},
	kw_20[2] = {
		{"dynamic",8,0,1,1,0,0.,0.,0,N_ifm(type,evalScheduling_PEER_DYNAMIC_SCHEDULING)},
		{"static",8,0,1,1,0,0.,0.,0,N_ifm(type,evalScheduling_PEER_STATIC_SCHEDULING)}
		},
	kw_21[2] = {
		{"master",8,0,1,1,0,0.,0.,0,N_ifm(type,evalScheduling_MASTER_SCHEDULING)},
		{"peer",8,2,1,1,kw_20}
		},
	kw_22[9] = {
		{"algebraic_mappings",11,0,2,0,0,0.,0.,0,N_ifm(str,algebraicMappings)},
		{"analysis_drivers",15,12,3,0,kw_16,0.,0.,0,N_ifm(strL,analysisDrivers)},
		{"analysis_scheduling",8,2,9,0,kw_17},
		{"analysis_servers",0x19,0,8,0,0,0.,0.,0,N_ifm(pint,analysisServers)},
		{"asynchronous",8,3,4,0,kw_19,0.,0.,0,N_ifm(lit,interfaceSynchronization_asynchronous)},
		{"evaluation_scheduling",8,2,6,0,kw_21},
		{"evaluation_servers",0x19,0,5,0,0,0.,0.,0,N_ifm(pint,evalServers)},
		{"id_interface",11,0,1,0,0,0.,0.,0,N_ifm(str,idInterface)},
		{"processors_per_evaluation",0x19,0,7,0,0,0.,0.,0,N_ifm(pint,procsPerEval)}
		},
	kw_23[1] = {
		{"model_pointer",11,0,1,0,0,0.,0.,0,N_mdm(str,modelPointer)}
		},
	kw_24[2] = {
		{"samples",9,0,1,0,0,0.,0.,0,N_mdm(int,numSamples)},
		{"seed",0x19,0,2,0,0,0.,0.,0,N_mdm(pint,randomSeed)}
		},
	kw_25[2] = {
		{"complementary",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_COMPLEMENTARY)},
		{"cumulative",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_CUMULATIVE)}
		},
	kw_26[1] = {
		{"num_gen_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,genReliabilityLevels)}
		},
	kw_27[1] = {
		{"num_probability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,probabilityLevels)}
		},
	kw_28[2] = {
		{"mt19937",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_mt19937)},
		{"rnum2",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_rnum2)}
		},
	kw_29[4] = {
		{"distribution",8,2,1,0,kw_25},
		{"gen_reliability_levels",14,1,3,0,kw_26,0.,0.,0,N_mdm(resplevs,genReliabilityLevels)},
		{"probability_levels",14,1,2,0,kw_27,0.,0.,0,N_mdm(resplevs01,probabilityLevels)},
		{"rng",8,2,4,0,kw_28}
		},
	kw_30[4] = {
		{"constant_liar",8,0,1,1,0,0.,0.,0,N_mdm(lit,batchSelectionType_constant_liar)},
		{"distance_penalty",8,0,1,1,0,0.,0.,0,N_mdm(lit,batchSelectionType_distance_penalty)},
		{"naive",8,0,1,1,0,0.,0.,0,N_mdm(lit,batchSelectionType_naive)},
		{"topology",8,0,1,1,0,0.,0.,0,N_mdm(lit,batchSelectionType_topology)}
		},
	kw_31[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_32[3] = {
		{"distance",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessMetricType_distance)},
		{"gradient",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessMetricType_gradient)},
		{"predicted_variance",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessMetricType_predicted_variance)}
		},
	kw_33[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_34[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_35[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_34}
		},
	kw_36[2] = {
		{"compute",8,3,2,0,kw_35},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_37[11] = {
		{0,0,1,0,0,kw_23},
		{0,0,2,0,0,kw_24},
		{0,0,4,0,0,kw_29},
		{"batch_selection",8,4,3,0,kw_30},
		{"batch_size",9,0,4,0,0,0.,0.,0,N_mdm(int,batchSize)},
		{"emulator_samples",9,0,1,0,0,0.,0.,0,N_mdm(int,emulatorSamples)},
		{"export_points_file",11,2,6,0,kw_31,0.,0.,0,N_mdm(str,approxExportFile)},
		{"fitness_metric",8,3,2,0,kw_32},
		{"import_points_file",11,2,5,0,kw_33,0.,0.,0,N_mdm(str,approxImportFile)},
		{"misc_options",15,0,8,0,0,0.,0.,0,N_mdm(strL,miscOptions)},
		{"response_levels",14,2,7,0,kw_36,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_38[9] = {
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
	kw_39[7] = {
		{"merit1",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit1)},
		{"merit1_smooth",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit1_smooth)},
		{"merit2",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit2)},
		{"merit2_smooth",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit2_smooth)},
		{"merit2_squared",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit2_squared)},
		{"merit_max",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit_max)},
		{"merit_max_smooth",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit_max_smooth)}
		},
	kw_40[2] = {
		{"blocking",8,0,1,1,0,0.,0.,0,N_mdm(lit,evalSynchronize_blocking)},
		{"nonblocking",8,0,1,1,0,0.,0.,0,N_mdm(lit,evalSynchronize_nonblocking)}
		},
	kw_41[11] = {
		{0,0,1,0,0,kw_23},
		{0,0,9,0,0,kw_38},
		{"constraint_penalty",10,0,7,0,0,0.,0.,0,N_mdm(Real,constrPenalty)},
		{"contraction_factor",10,0,2,0,0,0.,0.,0,N_mdm(Real,contractStepLength)},
		{"initial_delta",10,0,1,0,0,0.,0.,0,N_mdm(Real,initStepLength)},
		{"merit_function",8,7,6,0,kw_39},
		{"smoothing_factor",10,0,8,0,0,0.,0.,0,N_mdm(Real,smoothFactor)},
		{"solution_accuracy",2,0,4,0,0,0.,0.,1,N_mdm(Real,solnTarget)},
		{"solution_target",10,0,4,0,0,0.,0.,0,N_mdm(Real,solnTarget)},
		{"synchronization",8,2,5,0,kw_40},
		{"threshold_delta",10,0,3,0,0,0.,0.,0,N_mdm(Real,threshStepLength)}
		},
	kw_42[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_43[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_44[5] = {
		{"dakota",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_GP_EMULATOR)},
		{"emulator_samples",9,0,2,0,0,0.,0.,0,N_mdm(int,emulatorSamples)},
		{"export_points_file",11,2,4,0,kw_42,0.,0.,0,N_mdm(str,approxExportFile)},
		{"import_points_file",11,2,3,0,kw_43,0.,0.,0,N_mdm(str,approxImportFile)},
		{"surfpack",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING_EMULATOR)}
		},
	kw_45[1] = {
		{"sparse_grid_level",13,0,1,0,0,0.,0.,0,N_mdm(usharray,sparseGridLevel)}
		},
	kw_46[1] = {
		{"sparse_grid_level",13,0,1,0,0,0.,0.,0,N_mdm(usharray,sparseGridLevel)}
		},
	kw_47[4] = {
		{"gaussian_process",8,5,1,1,kw_44},
		{"kriging",0,5,1,1,kw_44,0.,0.,-1},
		{"pce",8,1,1,1,kw_45,0.,0.,0,N_mdm(type,emulatorType_PCE_EMULATOR)},
		{"sc",8,1,1,1,kw_46,0.,0.,0,N_mdm(type,emulatorType_SC_EMULATOR)}
		},
	kw_48[6] = {
		{"chains",0x29,0,1,0,0,3.,0.,0,N_mdm(int,numChains)},
		{"crossover_chain_pairs",0x29,0,3,0,0,0.,0.,0,N_mdm(int,crossoverChainPairs)},
		{"emulator",8,4,6,0,kw_47},
		{"gr_threshold",0x1a,0,4,0,0,0.,0.,0,N_mdm(Real,grThreshold)},
		{"jump_step",0x29,0,5,0,0,0.,0.,0,N_mdm(int,jumpStep)},
		{"num_cr",0x29,0,2,0,0,1.,0.,0,N_mdm(int,numCR)}
		},
	kw_49[2] = {
		{"adaptive",8,0,1,1,0,0.,0.,0,N_mdm(lit,metropolisType_adaptive)},
		{"hastings",8,0,1,1,0,0.,0.,0,N_mdm(lit,metropolisType_hastings)}
		},
	kw_50[2] = {
		{"delayed",8,0,1,1,0,0.,0.,0,N_mdm(lit,rejectionType_delayed)},
		{"standard",8,0,1,1,0,0.,0.,0,N_mdm(lit,rejectionType_standard)}
		},
	kw_51[2] = {
		{"metropolis",8,2,2,0,kw_49},
		{"rejection",8,2,1,0,kw_50}
		},
	kw_52[2] = {
		{"dram",8,2,1,1,kw_51,0.,0.,0,N_mdm(lit,mcmcType_dram)},
		{"multilevel",8,0,1,1,0,0.,0.,0,N_mdm(lit,mcmcType_multilevel)}
		},
	kw_53[2] = {
		{"mt19937",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_mt19937)},
		{"rnum2",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_rnum2)}
		},
	kw_54[3] = {
		{"mcmc_type",8,2,1,0,kw_52},
		{"proposal_covariance_scale",14,0,3,0,0,0.,0.,0,N_mdm(RealDL,proposalCovScale)},
		{"rng",8,2,2,0,kw_53}
		},
	kw_55[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_56[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_57[4] = {
		{0,0,3,0,0,kw_54},
		{"emulator_samples",9,0,1,1,0,0.,0.,0,N_mdm(int,emulatorSamples)},
		{"export_points_file",11,2,3,0,kw_55,0.,0.,0,N_mdm(str,approxExportFile)},
		{"import_points_file",11,2,2,0,kw_56,0.,0.,0,N_mdm(str,approxImportFile)}
		},
	kw_58[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_59[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_60[5] = {
		{"dakota",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_GP_EMULATOR)},
		{"emulator_samples",9,0,2,0,0,0.,0.,0,N_mdm(int,emulatorSamples)},
		{"export_points_file",11,2,4,0,kw_58,0.,0.,0,N_mdm(str,approxExportFile)},
		{"import_points_file",11,2,3,0,kw_59,0.,0.,0,N_mdm(str,approxImportFile)},
		{"surfpack",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING_EMULATOR)}
		},
	kw_61[1] = {
		{"sparse_grid_level",13,0,1,0,0,0.,0.,0,N_mdm(usharray,sparseGridLevel)}
		},
	kw_62[1] = {
		{"sparse_grid_level",13,0,1,0,0,0.,0.,0,N_mdm(usharray,sparseGridLevel)}
		},
	kw_63[4] = {
		{"gaussian_process",8,5,1,1,kw_60},
		{"kriging",0,5,1,1,kw_60,0.,0.,-1},
		{"pce",8,1,1,1,kw_61,0.,0.,0,N_mdm(type,emulatorType_PCE_EMULATOR)},
		{"sc",8,1,1,1,kw_62,0.,0.,0,N_mdm(type,emulatorType_SC_EMULATOR)}
		},
	kw_64[2] = {
		{0,0,3,0,0,kw_54},
		{"emulator",8,4,1,0,kw_63}
		},
	kw_65[8] = {
		{0,0,1,0,0,kw_23},
		{0,0,2,0,0,kw_24},
		{"calibrate_sigma",8,0,4,0,0,0.,0.,0,N_mdm(true,calibrateSigmaFlag)},
		{"dream",8,6,1,1,kw_48,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_DREAM)},
		{"gpmsa",8,3,1,1,kw_57,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_GPMSA)},
		{"likelihood_scale",10,0,3,0,0,0.,0.,0,N_mdm(Real,likelihoodScale)},
		{"queso",8,1,1,1,kw_64,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_QUESO)},
		{"use_derivatives",8,0,2,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)}
		},
	kw_66[4] = {
		{0,0,1,0,0,kw_23},
		{"deltas_per_variable",5,0,2,2,0,0.,0.,2,N_mdm(ivec,stepsPerVariable)},
		{"step_vector",14,0,1,1,0,0.,0.,0,N_mdm(RealDL,stepVector)},
		{"steps_per_variable",13,0,2,2,0,0.,0.,0,N_mdm(ivec,stepsPerVariable)}
		},
	kw_67[5] = {
		{"misc_options",15,0,4,0,0,0.,0.,0,N_mdm(strL,miscOptions)},
		{"seed",0x19,0,2,0,0,0.,0.,0,N_mdm(pint,randomSeed)},
		{"show_misc_options",8,0,3,0,0,0.,0.,0,N_mdm(true,showMiscOptions)},
		{"solution_accuracy",2,0,1,0,0,0.,0.,1,N_mdm(Real,solnTarget)},
		{"solution_target",10,0,1,0,0,0.,0.,0,N_mdm(Real,solnTarget)}
		},
	kw_68[3] = {
		{0,0,1,0,0,kw_23},
		{0,0,5,0,0,kw_67},
		{"beta_solver_name",11,0,1,1,0,0.,0.,0,N_mdm(str,betaSolverName)}
		},
	kw_69[2] = {
		{"initial_delta",10,0,1,0,0,0.,0.,0,N_mdm(Real,initDelta)},
		{"threshold_delta",10,0,2,0,0,0.,0.,0,N_mdm(Real,threshDelta)}
		},
	kw_70[4] = {
		{0,0,1,0,0,kw_23},
		{0,0,5,0,0,kw_67},
		{0,0,2,0,0,kw_69},
		{""}
		},
	kw_71[2] = {
		{"all_dimensions",8,0,1,1,0,0.,0.,0,N_mdm(lit,boxDivision_all_dimensions)},
		{"major_dimension",8,0,1,1,0,0.,0.,0,N_mdm(lit,boxDivision_major_dimension)}
		},
	kw_72[8] = {
		{0,0,1,0,0,kw_23},
		{0,0,5,0,0,kw_67},
		{"constraint_penalty",10,0,6,0,0,0.,0.,0,N_mdm(Real,constraintPenalty)},
		{"division",8,2,1,0,kw_71},
		{"global_balance_parameter",10,0,2,0,0,0.,0.,0,N_mdm(Real,globalBalanceParam)},
		{"local_balance_parameter",10,0,3,0,0,0.,0.,0,N_mdm(Real,localBalanceParam)},
		{"max_boxsize_limit",10,0,4,0,0,0.,0.,0,N_mdm(Real,maxBoxSize)},
		{"min_boxsize_limit",10,0,5,0,0,0.,0.,0,N_mdm(Real,minBoxSize)}
		},
	kw_73[3] = {
		{"blend",8,0,1,1,0,0.,0.,0,N_mdm(lit,crossoverType_blend)},
		{"two_point",8,0,1,1,0,0.,0.,0,N_mdm(lit,crossoverType_two_point)},
		{"uniform",8,0,1,1,0,0.,0.,0,N_mdm(lit,crossoverType_uniform)}
		},
	kw_74[2] = {
		{"linear_rank",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_linear_rank)},
		{"merit_function",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_proportional)}
		},
	kw_75[3] = {
		{"flat_file",11,0,1,1,0,0.,0.,0,N_mdm(slit2,TYPE_DATA_initializationType_flat_file)},
		{"simple_random",8,0,1,1,0,0.,0.,0,N_mdm(lit,initializationType_random)},
		{"unique_random",8,0,1,1,0,0.,0.,0,N_mdm(lit,initializationType_unique_random)}
		},
	kw_76[2] = {
		{"mutation_range",9,0,2,0,0,0.,0.,0,N_mdm(int,mutationRange)},
		{"mutation_scale",10,0,1,0,0,0.,0.,0,N_mdm(Real,mutationScale)}
		},
	kw_77[5] = {
		{"non_adaptive",8,0,2,0,0,0.,0.,0,N_mdm(false,mutationAdaptive)},
		{"offset_cauchy",8,2,1,1,kw_76,0.,0.,0,N_mdm(lit,mutationType_offset_cauchy)},
		{"offset_normal",8,2,1,1,kw_76,0.,0.,0,N_mdm(lit,mutationType_offset_normal)},
		{"offset_uniform",8,2,1,1,kw_76,0.,0.,0,N_mdm(lit,mutationType_offset_uniform)},
		{"replace_uniform",8,0,1,1,0,0.,0.,0,N_mdm(lit,mutationType_replace_uniform)}
		},
	kw_78[4] = {
		{"chc",9,0,1,1,0,0.,0.,0,N_mdm(ilit2,TYPE_DATA_replacementType_chc)},
		{"elitist",9,0,1,1,0,0.,0.,0,N_mdm(ilit2,TYPE_DATA_replacementType_elitist)},
		{"new_solutions_generated",9,0,2,0,0,0.,0.,0,N_mdm(int,newSolnsGenerated)},
		{"random",9,0,1,1,0,0.,0.,0,N_mdm(ilit2,TYPE_DATA_replacementType_random)}
		},
	kw_79[11] = {
		{0,0,1,0,0,kw_23},
		{0,0,5,0,0,kw_67},
		{"constraint_penalty",10,0,9,0,0,0.,0.,0,N_mdm(Real,constraintPenalty)},
		{"crossover_rate",10,0,5,0,0,0.,0.,0,N_mdm(Real,crossoverRate)},
		{"crossover_type",8,3,6,0,kw_73},
		{"fitness_type",8,2,3,0,kw_74},
		{"initialization_type",8,3,2,0,kw_75},
		{"mutation_rate",10,0,7,0,0,0.,0.,0,N_mdm(Real,mutationRate)},
		{"mutation_type",8,5,8,0,kw_77},
		{"population_size",0x19,0,1,0,0,0.,0.,0,N_mdm(pint,populationSize)},
		{"replacement_type",8,4,4,0,kw_78}
		},
	kw_80[2] = {
		{"constraint_penalty",10,0,2,0,0,0.,0.,0,N_mdm(Real,constraintPenalty)},
		{"contraction_factor",10,0,1,0,0,0.,0.,0,N_mdm(Real,contractFactor)}
		},
	kw_81[3] = {
		{"adaptive_pattern",8,0,1,1,0,0.,0.,0,N_mdm(lit,exploratoryMoves_adaptive)},
		{"basic_pattern",8,0,1,1,0,0.,0.,0,N_mdm(lit,exploratoryMoves_simple)},
		{"multi_step",8,0,1,1,0,0.,0.,0,N_mdm(lit,exploratoryMoves_multi_step)}
		},
	kw_82[2] = {
		{"coordinate",8,0,1,1,0,0.,0.,0,N_mdm(lit,patternBasis_coordinate)},
		{"simplex",8,0,1,1,0,0.,0.,0,N_mdm(lit,patternBasis_simplex)}
		},
	kw_83[2] = {
		{"blocking",8,0,1,1,0,0.,0.,0,N_mdm(lit,evalSynchronize_blocking)},
		{"nonblocking",8,0,1,1,0,0.,0.,0,N_mdm(lit,evalSynchronize_nonblocking)}
		},
	kw_84[12] = {
		{0,0,1,0,0,kw_23},
		{0,0,5,0,0,kw_67},
		{0,0,2,0,0,kw_69},
		{0,0,2,0,0,kw_80},
		{"constant_penalty",8,0,1,0,0,0.,0.,0,N_mdm(true,constantPenalty)},
		{"expand_after_success",9,0,3,0,0,0.,0.,0,N_mdm(int,expandAfterSuccess)},
		{"exploratory_moves",8,3,7,0,kw_81},
		{"no_expansion",8,0,2,0,0,0.,0.,0,N_mdm(false,expansionFlag)},
		{"pattern_basis",8,2,4,0,kw_82},
		{"stochastic",8,0,5,0,0,0.,0.,0,N_mdm(true,randomizeOrderFlag)},
		{"synchronization",8,2,8,0,kw_83},
		{"total_pattern_size",9,0,6,0,0,0.,0.,0,N_mdm(int,totalPatternSize)}
		},
	kw_85[8] = {
		{0,0,1,0,0,kw_23},
		{0,0,5,0,0,kw_67},
		{0,0,2,0,0,kw_69},
		{0,0,2,0,0,kw_80},
		{"constant_penalty",8,0,4,0,0,0.,0.,0,N_mdm(true,constantPenalty)},
		{"contract_after_failure",9,0,1,0,0,0.,0.,0,N_mdm(int,contractAfterFail)},
		{"expand_after_success",9,0,3,0,0,0.,0.,0,N_mdm(int,expandAfterSuccess)},
		{"no_expansion",8,0,2,0,0,0.,0.,0,N_mdm(false,expansionFlag)}
		},
	kw_86[4] = {
		{0,0,1,0,0,kw_23},
		{0,0,9,0,0,kw_38},
		{"frcg",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_CONMIN_FRCG)},
		{"mfd",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_CONMIN_MFD)}
		},
	kw_87[3] = {
		{0,0,1,0,0,kw_23},
		{0,0,9,0,0,kw_38},
		{""}
		},
	kw_88[1] = {
		{"drop_tolerance",10,0,1,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)}
		},
	kw_89[14] = {
		{0,0,1,0,0,kw_23},
		{0,0,2,0,0,kw_24},
		{"box_behnken",8,0,1,1,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_BOX_BEHNKEN)},
		{"central_composite",8,0,1,1,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_CENTRAL_COMPOSITE)},
		{"fixed_seed",8,0,5,0,0,0.,0.,0,N_mdm(true,fixedSeedFlag)},
		{"grid",8,0,1,1,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_GRID)},
		{"lhs",8,0,1,1,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_LHS)},
		{"main_effects",8,0,2,0,0,0.,0.,0,N_mdm(true,mainEffectsFlag)},
		{"oa_lhs",8,0,1,1,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_OA_LHS)},
		{"oas",8,0,1,1,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_OAS)},
		{"quality_metrics",8,0,3,0,0,0.,0.,0,N_mdm(true,volQualityFlag)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_RANDOM)},
		{"symbols",9,0,6,0,0,0.,0.,0,N_mdm(int,numSymbols)},
		{"variance_based_decomp",8,1,4,0,kw_88,0.,0.,0,N_mdm(true,vbdFlag)}
		},
	kw_90[7] = {
		{0,0,1,0,0,kw_23},
		{0,0,9,0,0,kw_38},
		{"bfgs",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_DOT_BFGS)},
		{"frcg",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_DOT_FRCG)},
		{"mmfd",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_DOT_MMFD)},
		{"slp",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_DOT_SLP)},
		{"sqp",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_DOT_SQP)}
		},
	kw_91[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_92[2] = {
		{"dakota",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_GP_EMULATOR)},
		{"surfpack",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING_EMULATOR)}
		},
	kw_93[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_94[7] = {
		{0,0,1,0,0,kw_23},
		{"export_points_file",11,2,4,0,kw_91,0.,0.,0,N_mdm(str,approxExportFile)},
		{"gaussian_process",8,2,1,0,kw_92},
		{"import_points_file",11,2,3,0,kw_93,0.,0.,0,N_mdm(str,approxImportFile)},
		{"kriging",0,2,1,0,kw_92,0.,0.,-2},
		{"seed",0x19,0,5,0,0,0.,0.,0,N_mdm(pint,randomSeed)},
		{"use_derivatives",8,0,2,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)}
		},
	kw_95[5] = {
		{0,0,1,0,0,kw_23},
		{0,0,2,0,0,kw_24},
		{0,0,4,0,0,kw_29},
		{"batch_size",9,0,2,0,0,0.,0.,0,N_mdm(int,batchSize)},
		{"emulator_samples",9,0,1,0,0,0.,0.,0,N_mdm(int,emulatorSamples)}
		},
	kw_96[3] = {
		{"grid",8,0,1,1,0,0.,0.,0,N_mdm(lit,trialType_grid)},
		{"halton",8,0,1,1,0,0.,0.,0,N_mdm(lit,trialType_halton)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(lit,trialType_random)}
		},
	kw_97[1] = {
		{"drop_tolerance",10,0,1,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)}
		},
	kw_98[8] = {
		{0,0,1,0,0,kw_23},
		{0,0,2,0,0,kw_24},
		{"fixed_seed",8,0,4,0,0,0.,0.,0,N_mdm(true,fixedSeedFlag)},
		{"latinize",8,0,1,0,0,0.,0.,0,N_mdm(true,latinizeFlag)},
		{"num_trials",9,0,6,0,0,0.,0.,0,N_mdm(int,numTrials)},
		{"quality_metrics",8,0,2,0,0,0.,0.,0,N_mdm(true,volQualityFlag)},
		{"trial_type",8,3,5,0,kw_96},
		{"variance_based_decomp",8,1,3,0,kw_97,0.,0.,0,N_mdm(true,vbdFlag)}
		},
	kw_99[1] = {
		{"drop_tolerance",10,0,1,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)}
		},
	kw_100[11] = {
		{0,0,1,0,0,kw_23},
		{"fixed_sequence",8,0,6,0,0,0.,0.,0,N_mdm(true,fixedSequenceFlag)},
		{"halton",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_FSU_HALTON)},
		{"hammersley",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_FSU_HAMMERSLEY)},
		{"latinize",8,0,2,0,0,0.,0.,0,N_mdm(true,latinizeFlag)},
		{"prime_base",13,0,9,0,0,0.,0.,0,N_mdm(ivec,primeBase)},
		{"quality_metrics",8,0,3,0,0,0.,0.,0,N_mdm(true,volQualityFlag)},
		{"samples",9,0,5,0,0,0.,0.,0,N_mdm(int,numSamples)},
		{"sequence_leap",13,0,8,0,0,0.,0.,0,N_mdm(ivec,sequenceLeap)},
		{"sequence_start",13,0,7,0,0,0.,0.,0,N_mdm(ivec,sequenceStart)},
		{"variance_based_decomp",8,1,4,0,kw_99,0.,0.,0,N_mdm(true,vbdFlag)}
		},
	kw_101[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_102[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_103[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_104[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_103}
		},
	kw_105[2] = {
		{"compute",8,3,2,0,kw_104},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_106[7] = {
		{0,0,1,0,0,kw_23},
		{0,0,2,0,0,kw_24},
		{0,0,4,0,0,kw_29},
		{"emulator_samples",9,0,1,0,0,0.,0.,0,N_mdm(int,emulatorSamples)},
		{"export_points_file",11,2,3,0,kw_101,0.,0.,0,N_mdm(str,approxExportFile)},
		{"import_points_file",11,2,2,0,kw_102,0.,0.,0,N_mdm(str,approxImportFile)},
		{"response_levels",14,2,4,0,kw_105,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_107[2] = {
		{0,0,1,0,0,kw_23},
		{"seed",0x19,0,1,0,0,0.,0.,0,N_mdm(pint,randomSeed)}
		},
	kw_108[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_109[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_108}
		},
	kw_110[2] = {
		{"compute",8,3,2,0,kw_109},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_111[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_112[2] = {
		{"dakota",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_GP_EMULATOR)},
		{"surfpack",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING_EMULATOR)}
		},
	kw_113[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_114[5] = {
		{"export_points_file",11,2,4,0,kw_111,0.,0.,0,N_mdm(str,approxExportFile)},
		{"gaussian_process",8,2,1,0,kw_112},
		{"import_points_file",11,2,3,0,kw_113,0.,0.,0,N_mdm(str,approxImportFile)},
		{"kriging",0,2,1,0,kw_112,0.,0.,-2},
		{"use_derivatives",8,0,2,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)}
		},
	kw_115[8] = {
		{0,0,1,0,0,kw_23},
		{0,0,2,0,0,kw_24},
		{0,0,4,0,0,kw_29},
		{"ea",8,0,1,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_EA)},
		{"ego",8,5,1,0,kw_114,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_EGO)},
		{"lhs",8,0,1,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_LHS)},
		{"response_levels",14,2,2,0,kw_110,0.,0.,0,N_mdm(resplevs,responseLevels)},
		{"sbo",8,5,1,0,kw_114,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_SBO)}
		},
	kw_116[2] = {
		{"mt19937",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_mt19937)},
		{"rnum2",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_rnum2)}
		},
	kw_117[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_118[2] = {
		{"dakota",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_GP_EMULATOR)},
		{"surfpack",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING_EMULATOR)}
		},
	kw_119[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_120[5] = {
		{"export_points_file",11,2,4,0,kw_117,0.,0.,0,N_mdm(str,approxExportFile)},
		{"gaussian_process",8,2,1,0,kw_118},
		{"import_points_file",11,2,3,0,kw_119,0.,0.,0,N_mdm(str,approxImportFile)},
		{"kriging",0,2,1,0,kw_118,0.,0.,-2},
		{"use_derivatives",8,0,2,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)}
		},
	kw_121[7] = {
		{0,0,1,0,0,kw_23},
		{0,0,2,0,0,kw_24},
		{"ea",8,0,1,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_EA)},
		{"ego",8,5,1,0,kw_120,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_EGO)},
		{"lhs",8,0,1,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_LHS)},
		{"rng",8,2,2,0,kw_116},
		{"sbo",8,5,1,0,kw_120,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_SBO)}
		},
	kw_122[2] = {
		{"complementary",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_COMPLEMENTARY)},
		{"cumulative",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_CUMULATIVE)}
		},
	kw_123[1] = {
		{"num_gen_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,genReliabilityLevels)}
		},
	kw_124[1] = {
		{"num_probability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,probabilityLevels)}
		},
	kw_125[3] = {
		{"distribution",8,2,1,0,kw_122},
		{"gen_reliability_levels",14,1,3,0,kw_123,0.,0.,0,N_mdm(resplevs,genReliabilityLevels)},
		{"probability_levels",14,1,2,0,kw_124,0.,0.,0,N_mdm(resplevs01,probabilityLevels)}
		},
	kw_126[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_127[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_128[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_129[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_128}
		},
	kw_130[2] = {
		{"compute",8,3,2,0,kw_129},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_131[2] = {
		{"mt19937",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_mt19937)},
		{"rnum2",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_rnum2)}
		},
	kw_132[14] = {
		{0,0,1,0,0,kw_23},
		{0,0,3,0,0,kw_125},
		{"dakota",8,0,2,0,0,0.,0.,0,N_mdm(type,emulatorType_GP_EMULATOR)},
		{"export_points_file",11,2,4,0,kw_126,0.,0.,0,N_mdm(str,approxExportFile)},
		{"import_points_file",11,2,3,0,kw_127,0.,0.,0,N_mdm(str,approxImportFile)},
		{"response_levels",14,2,8,0,kw_130,0.,0.,0,N_mdm(resplevs,responseLevels)},
		{"rng",8,2,7,0,kw_131},
		{"seed",0x19,0,6,0,0,0.,0.,0,N_mdm(pint,randomSeed)},
		{"surfpack",8,0,2,0,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING_EMULATOR)},
		{"u_gaussian_process",8,0,1,1,0,0.,0.,0,N_mdm(utype,reliabilitySearchType_EGRA_U)},
		{"u_kriging",0,0,1,1,0,0.,0.,-1,N_mdm(utype,reliabilitySearchType_EGRA_U)},
		{"use_derivatives",8,0,5,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)},
		{"x_gaussian_process",8,0,1,1,0,0.,0.,0,N_mdm(utype,reliabilitySearchType_EGRA_X)},
		{"x_kriging",0,0,1,1,0,0.,0.,-1,N_mdm(utype,reliabilitySearchType_EGRA_X)}
		},
	kw_133[2] = {
		{"master",8,0,1,1,0,0.,0.,0,N_mdm(type,iteratorScheduling_MASTER_SCHEDULING)},
		{"peer",8,0,1,1,0,0.,0.,0,N_mdm(type,iteratorScheduling_PEER_SCHEDULING)}
		},
	kw_134[3] = {
		{"iterator_scheduling",8,2,2,0,kw_133},
		{"iterator_servers",0x19,0,1,0,0,0.,0.,0,N_mdm(pint,iteratorServers)},
		{"processors_per_iterator",0x19,0,3,0,0,0.,0.,0,N_mdm(pint,procsPerIterator)}
		},
	kw_135[1] = {
		{"model_pointer_list",11,0,1,0,0,0.,0.,0,N_mdm(strL,hybridModelPointers)}
		},
	kw_136[2] = {
		{"method_name_list",15,1,1,1,kw_135,0.,0.,0,N_mdm(strL,hybridMethodNames)},
		{"method_pointer_list",15,0,1,1,0,0.,0.,0,N_mdm(strL,hybridMethodPointers)}
		},
	kw_137[1] = {
		{"global_model_pointer",11,0,1,0,0,0.,0.,0,N_mdm(str,hybridGlobalModelPointer)}
		},
	kw_138[1] = {
		{"local_model_pointer",11,0,1,0,0,0.,0.,0,N_mdm(str,hybridLocalModelPointer)}
		},
	kw_139[5] = {
		{"global_method_name",11,1,1,1,kw_137,0.,0.,0,N_mdm(str,hybridGlobalMethodName)},
		{"global_method_pointer",11,0,1,1,0,0.,0.,0,N_mdm(str,hybridGlobalMethodPointer)},
		{"local_method_name",11,1,2,2,kw_138,0.,0.,0,N_mdm(str,hybridLocalMethodName)},
		{"local_method_pointer",11,0,2,2,0,0.,0.,0,N_mdm(str,hybridLocalMethodPointer)},
		{"local_search_probability",10,0,3,0,0,0.,0.,0,N_mdm(Real,hybridLSProb)}
		},
	kw_140[1] = {
		{"model_pointer_list",11,0,1,0,0,0.,0.,0,N_mdm(strL,hybridModelPointers)}
		},
	kw_141[2] = {
		{"method_name_list",15,1,1,1,kw_140,0.,0.,0,N_mdm(strL,hybridMethodNames)},
		{"method_pointer_list",15,0,1,1,0,0.,0.,0,N_mdm(strL,hybridMethodPointers)}
		},
	kw_142[6] = {
		{0,0,3,0,0,kw_134},
		{"collaborative",8,2,1,1,kw_136,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_COLLABORATIVE)},
		{"coupled",0,5,1,1,kw_139,0.,0.,1,N_mdm(utype,subMethod_SUBMETHOD_EMBEDDED)},
		{"embedded",8,5,1,1,kw_139,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_EMBEDDED)},
		{"sequential",8,2,1,1,kw_141,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_SEQUENTIAL)},
		{"uncoupled",0,2,1,1,kw_141,0.,0.,-1,N_mdm(utype,subMethod_SUBMETHOD_SEQUENTIAL)}
		},
	kw_143[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_144[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_143}
		},
	kw_145[2] = {
		{"compute",8,3,2,0,kw_144},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_146[8] = {
		{0,0,1,0,0,kw_23},
		{0,0,2,0,0,kw_24},
		{0,0,4,0,0,kw_29},
		{"adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_AIS)},
		{"import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_IS)},
		{"mm_adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_MMAIS)},
		{"refinement_samples",9,0,2,0,0,0.,0.,0,N_mdm(int,refineSamples)},
		{"response_levels",14,2,3,0,kw_145,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_147[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,pstudyFileAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,pstudyFileAnnotated)}
		},
	kw_148[3] = {
		{0,0,1,0,0,kw_23},
		{"import_points_file",11,2,1,1,kw_147,0.,0.,0,N_mdm(str,pstudyFilename)},
		{"list_of_points",14,0,1,1,0,0.,0.,0,N_mdm(RealDL,listOfPoints)}
		},
	kw_149[2] = {
		{"complementary",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_COMPLEMENTARY)},
		{"cumulative",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_CUMULATIVE)}
		},
	kw_150[1] = {
		{"num_gen_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,genReliabilityLevels)}
		},
	kw_151[1] = {
		{"num_probability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,probabilityLevels)}
		},
	kw_152[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_153[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_152}
		},
	kw_154[2] = {
		{"compute",8,3,2,0,kw_153},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_155[7] = {
		{0,0,1,0,0,kw_23},
		{"distribution",8,2,5,0,kw_149},
		{"gen_reliability_levels",14,1,4,0,kw_150,0.,0.,0,N_mdm(resplevs,genReliabilityLevels)},
		{"nip",8,0,1,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_NIP)},
		{"probability_levels",14,1,3,0,kw_151,0.,0.,0,N_mdm(resplevs01,probabilityLevels)},
		{"response_levels",14,2,2,0,kw_154,0.,0.,0,N_mdm(resplevs,responseLevels)},
		{"sqp",8,0,1,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_SQP)}
		},
	kw_156[3] = {
		{0,0,1,0,0,kw_23},
		{"nip",8,0,1,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_NIP)},
		{"sqp",8,0,1,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_SQP)}
		},
	kw_157[5] = {
		{"adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_AIS)},
		{"import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_IS)},
		{"mm_adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_MMAIS)},
		{"refinement_samples",9,0,2,0,0,0.,0.,0,N_mdm(int,refineSamples)},
		{"seed",0x19,0,3,0,0,0.,0.,0,N_mdm(pint,randomSeed)}
		},
	kw_158[4] = {
		{"first_order",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilityIntegration_first_order)},
		{"probability_refinement",8,5,2,0,kw_157},
		{"sample_refinement",0,5,2,0,kw_157,0.,0.,-1},
		{"second_order",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilityIntegration_second_order)}
		},
	kw_159[10] = {
		{"integration",8,4,3,0,kw_158},
		{"nip",8,0,2,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_NIP)},
		{"no_approx",8,0,1,1,0,0.,0.,0,N_mdm(utype,reliabilitySearchType_NO_APPROX)},
		{"sqp",8,0,2,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_SQP)},
		{"u_taylor_mean",8,0,1,1,0,0.,0.,0,N_mdm(utype,reliabilitySearchType_AMV_U)},
		{"u_taylor_mpp",8,0,1,1,0,0.,0.,0,N_mdm(utype,reliabilitySearchType_AMV_PLUS_U)},
		{"u_two_point",8,0,1,1,0,0.,0.,0,N_mdm(utype,reliabilitySearchType_TANA_U)},
		{"x_taylor_mean",8,0,1,1,0,0.,0.,0,N_mdm(utype,reliabilitySearchType_AMV_X)},
		{"x_taylor_mpp",8,0,1,1,0,0.,0.,0,N_mdm(utype,reliabilitySearchType_AMV_PLUS_X)},
		{"x_two_point",8,0,1,1,0,0.,0.,0,N_mdm(utype,reliabilitySearchType_TANA_X)}
		},
	kw_160[1] = {
		{"num_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,reliabilityLevels)}
		},
	kw_161[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_162[4] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_RELIABILITIES)},
		{"system",8,2,2,0,kw_161}
		},
	kw_163[2] = {
		{"compute",8,4,2,0,kw_162},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_164[5] = {
		{0,0,1,0,0,kw_23},
		{0,0,3,0,0,kw_125},
		{"mpp_search",8,10,1,0,kw_159},
		{"reliability_levels",14,1,3,0,kw_160,0.,0.,0,N_mdm(resplevs,reliabilityLevels)},
		{"response_levels",14,2,2,0,kw_163,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_165[8] = {
		{0,0,1,0,0,kw_23},
		{0,0,9,0,0,kw_38},
		{"display_all_evaluations",8,0,6,0,0,0.,0.,0,N_mdm(true,showAllEval)},
		{"display_format",11,0,4,0,0,0.,0.,0,N_mdm(str,displayFormat)},
		{"function_precision",10,0,1,0,0,0.,0.,0,N_mdm(Real,functionPrecision)},
		{"history_file",11,0,3,0,0,0.,0.,0,N_mdm(str,historyFile)},
		{"seed",0x19,0,2,0,0,0.,0.,0,N_mdm(pint,randomSeed)},
		{"variable_neighborhood_search",10,0,5,0,0,0.,0.,0,N_mdm(Real,vns)}
		},
	kw_166[2] = {
		{"num_offspring",0x19,0,2,0,0,0.,0.,0,N_mdm(pintz,numOffspring)},
		{"num_parents",0x19,0,1,0,0,0.,0.,0,N_mdm(pintz,numParents)}
		},
	kw_167[5] = {
		{"crossover_rate",10,0,2,0,0,0.,0.,0,N_mdm(litz,TYPE_DATA_crossoverType_null_crossover)},
		{"multi_point_binary",9,0,1,1,0,0.,0.,0,N_mdm(ilit2p,TYPE_DATA_crossoverType_multi_point_binary)},
		{"multi_point_parameterized_binary",9,0,1,1,0,0.,0.,0,N_mdm(ilit2p,TYPE_DATA_crossoverType_multi_point_parameterized_binary)},
		{"multi_point_real",9,0,1,1,0,0.,0.,0,N_mdm(ilit2p,TYPE_DATA_crossoverType_multi_point_real)},
		{"shuffle_random",8,2,1,1,kw_166,0.,0.,0,N_mdm(litc,TYPE_DATA_crossoverType_shuffle_random)}
		},
	kw_168[3] = {
		{"flat_file",11,0,1,1,0,0.,0.,0,N_mdm(slit2,TYPE_DATA_initializationType_flat_file)},
		{"simple_random",8,0,1,1,0,0.,0.,0,N_mdm(lit,initializationType_random)},
		{"unique_random",8,0,1,1,0,0.,0.,0,N_mdm(lit,initializationType_unique_random)}
		},
	kw_169[1] = {
		{"mutation_scale",10,0,1,0,0,0.,0.,0,N_mdm(Real01,mutationScale)}
		},
	kw_170[6] = {
		{"bit_random",8,0,1,1,0,0.,0.,0,N_mdm(lit,mutationType_bit_random)},
		{"mutation_rate",10,0,2,0,0,0.,0.,0,N_mdm(litz,TYPE_DATA_mutationType_null_mutation)},
		{"offset_cauchy",8,1,1,1,kw_169,0.,0.,0,N_mdm(litc,TYPE_DATA_mutationType_offset_cauchy)},
		{"offset_normal",8,1,1,1,kw_169,0.,0.,0,N_mdm(litc,TYPE_DATA_mutationType_offset_normal)},
		{"offset_uniform",8,1,1,1,kw_169,0.,0.,0,N_mdm(litc,TYPE_DATA_mutationType_offset_uniform)},
		{"replace_uniform",8,0,1,1,0,0.,0.,0,N_mdm(lit,mutationType_replace_uniform)}
		},
	kw_171[7] = {
		{"crossover_type",8,5,5,0,kw_167},
		{"initialization_type",8,3,4,0,kw_168},
		{"log_file",11,0,2,0,0,0.,0.,0,N_mdm(str,logFile)},
		{"mutation_type",8,6,6,0,kw_170},
		{"population_size",0x29,0,1,0,0,0.,0.,0,N_mdm(nnint,populationSize)},
		{"print_each_pop",8,0,3,0,0,0.,0.,0,N_mdm(true,printPopFlag)},
		{"seed",0x19,0,7,0,0,0.,0.,0,N_mdm(pint,randomSeed)}
		},
	kw_172[3] = {
		{"metric_tracker",8,0,1,1,0,0.,0.,0,N_mdm(lit,convergenceType_metric_tracker)},
		{"num_generations",0x29,0,3,0,0,0.,0.,0,N_mdm(sizet,numGenerations)},
		{"percent_change",10,0,2,0,0,0.,0.,0,N_mdm(Realz,convergenceTolerance)}
		},
	kw_173[2] = {
		{"domination_count",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_domination_count)},
		{"layer_rank",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_layer_rank)}
		},
	kw_174[1] = {
		{"num_designs",0x29,0,1,0,0,2.,0.,0,N_mdm(pintz,numDesigns)}
		},
	kw_175[3] = {
		{"distance",14,0,1,1,0,0.,0.,0,N_mdm(RealLlit,TYPE_DATA_nichingType_distance)},
		{"max_designs",14,1,1,1,kw_174,0.,0.,0,N_mdm(RealLlit,TYPE_DATA_nichingType_max_designs)},
		{"radial",14,0,1,1,0,0.,0.,0,N_mdm(RealLlit,TYPE_DATA_nichingType_radial)}
		},
	kw_176[1] = {
		{"orthogonal_distance",14,0,1,1,0,0.,0.,0,N_mdm(RealLlit,TYPE_DATA_postProcessorType_distance_postprocessor)}
		},
	kw_177[2] = {
		{"shrinkage_fraction",10,0,1,0,0,0.,0.,0,N_mdm(Real01,shrinkagePercent)},
		{"shrinkage_percentage",2,0,1,0,0,0.,0.,-1,N_mdm(Real01,shrinkagePercent)}
		},
	kw_178[4] = {
		{"below_limit",10,2,1,1,kw_177,0.,0.,0,N_mdm(litp,TYPE_DATA_replacementType_below_limit)},
		{"elitist",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_elitist)},
		{"roulette_wheel",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_roulette_wheel)},
		{"unique_roulette_wheel",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_unique_roulette_wheel)}
		},
	kw_179[8] = {
		{0,0,1,0,0,kw_23},
		{0,0,9,0,0,kw_38},
		{0,0,7,0,0,kw_171},
		{"convergence_type",8,3,4,0,kw_172},
		{"fitness_type",8,2,1,0,kw_173},
		{"niching_type",8,3,3,0,kw_175},
		{"postprocessor_type",8,1,5,0,kw_176},
		{"replacement_type",8,4,2,0,kw_178}
		},
	kw_180[1] = {
		{"model_pointer",11,0,1,0,0,0.,0.,0,N_mdm(str,subModelPointer)}
		},
	kw_181[1] = {
		{"seed",9,0,1,0,0,0.,0.,0,N_mdm(int,randomSeed)}
		},
	kw_182[5] = {
		{0,0,3,0,0,kw_134},
		{"method_name",11,1,1,1,kw_180,0.,0.,0,N_mdm(str,subMethodName)},
		{"method_pointer",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodPointer)},
		{"random_starts",9,1,2,0,kw_181,0.,0.,0,N_mdm(int,concurrentRandomJobs)},
		{"starting_points",14,0,3,0,0,0.,0.,0,N_mdm(RealDL,concurrentParameterSets)}
		},
	kw_183[2] = {
		{0,0,1,0,0,kw_23},
		{"partitions",13,0,1,1,0,0.,0.,0,N_mdm(usharray,varPartitions)}
		},
	kw_184[5] = {
		{0,0,1,0,0,kw_23},
		{"min_boxsize_limit",10,0,2,0,0,0.,0.,0,N_mdm(Real,minBoxSize)},
		{"solution_accuracy",2,0,1,0,0,0.,0.,1,N_mdm(Real,solnTarget)},
		{"solution_target",10,0,1,0,0,0.,0.,0,N_mdm(Real,solnTarget)},
		{"volume_boxsize_limit",10,0,3,0,0,0.,0.,0,N_mdm(Real,volBoxSize)}
		},
	kw_185[10] = {
		{0,0,1,0,0,kw_23},
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
	kw_186[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_187[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_186}
		},
	kw_188[2] = {
		{"compute",8,3,2,0,kw_187},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_189[4] = {
		{0,0,1,0,0,kw_23},
		{0,0,2,0,0,kw_24},
		{0,0,4,0,0,kw_29},
		{"response_levels",14,2,1,0,kw_188,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_190[1] = {
		{"num_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,reliabilityLevels)}
		},
	kw_191[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_192[4] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_RELIABILITIES)},
		{"system",8,2,2,0,kw_191}
		},
	kw_193[2] = {
		{"compute",8,4,2,0,kw_192},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_194[2] = {
		{"reliability_levels",14,1,1,0,kw_190,0.,0.,0,N_mdm(resplevs,reliabilityLevels)},
		{"response_levels",14,2,2,0,kw_193,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_195[1] = {
		{"fixed_seed",8,0,1,0,0,0.,0.,0,N_mdm(true,fixedSeedFlag)}
		},
	kw_196[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxImportAnnotated)}
		},
	kw_197[1] = {
		{"import_points_file",11,2,1,0,kw_196,0.,0.,0,N_mdm(str,approxImportFile)}
		},
	kw_198[1] = {
		{"advancements",9,0,1,0,0,0.,0.,0,N_mdm(ushint,adaptedBasisAdvancements)}
		},
	kw_199[1] = {
		{"initial_level",9,0,1,0,0,0.,0.,0,N_mdm(ushint,adaptedBasisInitLevel)}
		},
	kw_200[3] = {
		{"expanding_front",8,1,1,1,kw_198,0.,0.,0,N_mdm(type,expansionBasisType_ADAPTED_BASIS_EXPANDING_FRONT)},
		{"generalized",8,1,1,1,kw_199,0.,0.,0,N_mdm(type,expansionBasisType_ADAPTED_BASIS_GENERALIZED)},
		{"soft_convergence_limit",9,0,2,0,0,0.,0.,0,N_mdm(ushint,softConvLimit)}
		},
	kw_201[3] = {
		{"adapted",8,3,1,1,kw_200},
		{"tensor_product",8,0,1,1,0,0.,0.,0,N_mdm(type,expansionBasisType_TENSOR_PRODUCT_BASIS)},
		{"total_order",8,0,1,1,0,0.,0.,0,N_mdm(type,expansionBasisType_TOTAL_ORDER_BASIS)}
		},
	kw_202[1] = {
		{"noise_tolerance",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,regressionNoiseTol)}
		},
	kw_203[1] = {
		{"noise_tolerance",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,regressionNoiseTol)}
		},
	kw_204[2] = {
		{"l2_penalty",10,0,2,0,0,0.,0.,0,N_mdm(Real,regressionL2Penalty)},
		{"noise_tolerance",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,regressionNoiseTol)}
		},
	kw_205[2] = {
		{"equality_constrained",8,0,1,0,0,0.,0.,0,N_mdm(type,lsRegressionType_EQ_CON_LS)},
		{"svd",8,0,1,0,0,0.,0.,0,N_mdm(type,lsRegressionType_SVD_LS)}
		},
	kw_206[1] = {
		{"noise_tolerance",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,regressionNoiseTol)}
		},
	kw_207[17] = {
		{"basis_pursuit",8,0,2,0,0,0.,0.,0,N_mdm(type,regressionType_BASIS_PURSUIT)},
		{"basis_pursuit_denoising",8,1,2,0,kw_202,0.,0.,0,N_mdm(type,regressionType_BASIS_PURSUIT_DENOISING)},
		{"bp",0,0,2,0,0,0.,0.,-2,N_mdm(type,regressionType_BASIS_PURSUIT)},
		{"bpdn",0,1,2,0,kw_202,0.,0.,-2,N_mdm(type,regressionType_BASIS_PURSUIT_DENOISING)},
		{"cross_validation",8,0,3,0,0,0.,0.,0,N_mdm(true,crossValidation)},
		{"lars",0,1,2,0,kw_203,0.,0.,3,N_mdm(type,regressionType_LEAST_ANGLE_REGRESSION)},
		{"lasso",0,2,2,0,kw_204,0.,0.,1,N_mdm(type,regressionType_LASSO_REGRESSION)},
		{"least_absolute_shrinkage",8,2,2,0,kw_204,0.,0.,0,N_mdm(type,regressionType_LASSO_REGRESSION)},
		{"least_angle_regression",8,1,2,0,kw_203,0.,0.,0,N_mdm(type,regressionType_LEAST_ANGLE_REGRESSION)},
		{"least_squares",8,2,2,0,kw_205,0.,0.,0,N_mdm(type,regressionType_DEFAULT_LEAST_SQ_REGRESSION)},
		{"omp",0,1,2,0,kw_206,0.,0.,1,N_mdm(type,regressionType_ORTHOG_MATCH_PURSUIT)},
		{"orthogonal_matching_pursuit",8,1,2,0,kw_206,0.,0.,0,N_mdm(type,regressionType_ORTHOG_MATCH_PURSUIT)},
		{"ratio_order",10,0,1,0,0,0.,0.,0,N_mdm(Realp,collocRatioTermsOrder)},
		{"reuse_points",8,0,6,0,0,0.,0.,0,N_mdm(lit,pointReuse_all)},
		{"reuse_samples",0,0,6,0,0,0.,0.,-1,N_mdm(lit,pointReuse_all)},
		{"tensor_grid",8,0,5,0,0,0.,0.,0,N_mdm(true,tensorGridFlag)},
		{"use_derivatives",8,0,4,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)}
		},
	kw_208[3] = {
		{"incremental_lhs",8,0,2,0,0,0.,0.,0,N_mdm(lit,expansionSampleType_incremental_lhs)},
		{"reuse_points",8,0,1,0,0,0.,0.,0,N_mdm(lit,pointReuse_all)},
		{"reuse_samples",0,0,1,0,0,0.,0.,-1,N_mdm(lit,pointReuse_all)}
		},
	kw_209[6] = {
		{0,0,1,0,0,kw_197},
		{"basis_type",8,3,2,0,kw_201},
		{"collocation_points",13,17,3,1,kw_207,0.,0.,0,N_mdm(szarray,collocationPoints)},
		{"collocation_ratio",10,17,3,1,kw_207,0.,0.,0,N_mdm(Realp,collocationRatio)},
		{"dimension_preference",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,anisoDimPref)},
		{"expansion_samples",13,3,3,1,kw_208,0.,0.,0,N_mdm(szarray,expansionSamples)}
		},
	kw_210[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_211[6] = {
		{0,0,1,0,0,kw_197},
		{"collocation_points",13,0,1,1,0,0.,0.,0,N_mdm(szarray,collocationPoints)},
		{"cross_validation",8,0,2,0,0,0.,0.,0,N_mdm(true,crossValidation)},
		{"reuse_points",8,0,4,0,0,0.,0.,0,N_mdm(lit,pointReuse_all)},
		{"reuse_samples",0,0,4,0,0,0.,0.,-1,N_mdm(lit,pointReuse_all)},
		{"tensor_grid",13,0,3,0,0,0.,0.,0,N_mdm(usharray,tensorGridOrder)}
		},
	kw_212[3] = {
		{"decay",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_DECAY)},
		{"generalized",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_GENERALIZED)},
		{"sobol",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_SOBOL)}
		},
	kw_213[2] = {
		{"dimension_adaptive",8,3,1,1,kw_212},
		{"uniform",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_UNIFORM_CONTROL)}
		},
	kw_214[4] = {
		{"adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_AIS)},
		{"import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_IS)},
		{"mm_adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_MMAIS)},
		{"refinement_samples",9,0,2,0,0,0.,0.,0,N_mdm(int,refineSamples)}
		},
	kw_215[3] = {
		{"dimension_preference",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,anisoDimPref)},
		{"nested",8,0,2,0,0,0.,0.,0,N_mdm(type,nestingOverride_NESTED)},
		{"non_nested",8,0,2,0,0,0.,0.,0,N_mdm(type,nestingOverride_NON_NESTED)}
		},
	kw_216[2] = {
		{"lhs",8,0,1,1,0,0.,0.,0,N_mdm(utype,sampleType_SUBMETHOD_LHS)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(utype,sampleType_SUBMETHOD_RANDOM)}
		},
	kw_217[3] = {
		{0,0,3,0,0,kw_215},
		{"restricted",8,0,1,0,0,0.,0.,0,N_mdm(type,growthOverride_RESTRICTED)},
		{"unrestricted",8,0,1,0,0,0.,0.,0,N_mdm(type,growthOverride_UNRESTRICTED)}
		},
	kw_218[2] = {
		{"drop_tolerance",10,0,2,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)},
		{"interaction_order",0x19,0,1,0,0,0.,0.,0,N_mdm(ushint,vbdOrder)}
		},
	kw_219[25] = {
		{0,0,1,0,0,kw_23},
		{0,0,2,0,0,kw_24},
		{0,0,4,0,0,kw_29},
		{0,0,2,0,0,kw_194},
		{0,0,1,0,0,kw_195},
		{"askey",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionType_ASKEY_U)},
		{"cubature_integrand",9,0,3,1,0,0.,0.,0,N_mdm(ushint,cubIntOrder)},
		{"diagonal_covariance",8,0,5,0,0,0.,0.,0,N_mdm(type,covarianceControl_DIAGONAL_COVARIANCE)},
		{"expansion_order",13,5,3,1,kw_209,0.,0.,0,N_mdm(usharray,expansionOrder)},
		{"export_expansion_file",11,0,10,0,0,0.,0.,0,N_mdm(str,expansionExportFile)},
		{"export_points_file",11,2,9,0,kw_210,0.,0.,0,N_mdm(str,approxExportFile)},
		{"full_covariance",8,0,5,0,0,0.,0.,0,N_mdm(type,covarianceControl_FULL_COVARIANCE)},
		{"import_expansion_file",11,0,3,1,0,0.,0.,0,N_mdm(str,expansionImportFile)},
		{"least_interpolation",0,5,3,1,kw_211,0.,0.,3,N_mdm(type,regressionType_ORTHOG_LEAST_INTERPOLATION)},
		{"normalized",8,0,6,0,0,0.,0.,0,N_mdm(true,normalizedCoeffs)},
		{"oli",0,5,3,1,kw_211,0.,0.,1,N_mdm(type,regressionType_ORTHOG_LEAST_INTERPOLATION)},
		{"orthogonal_least_interpolation",8,5,3,1,kw_211,0.,0.,0,N_mdm(type,regressionType_ORTHOG_LEAST_INTERPOLATION)},
		{"p_refinement",8,2,1,0,kw_213,0.,0.,0,N_mdm(type,refinementType_P_REFINEMENT)},
		{"probability_refinement",8,4,8,0,kw_214},
		{"quadrature_order",13,3,3,1,kw_215,0.,0.,0,N_mdm(usharray,quadratureOrder)},
		{"sample_refinement",0,4,8,0,kw_214,0.,0.,-2},
		{"sample_type",8,2,7,0,kw_216},
		{"sparse_grid_level",13,2,3,1,kw_217,0.,0.,0,N_mdm(usharray,sparseGridLevel)},
		{"variance_based_decomp",8,2,4,0,kw_218,0.,0.,0,N_mdm(true,vbdFlag)},
		{"wiener",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionType_STD_NORMAL_U)}
		},
	kw_220[1] = {
		{"previous_samples",9,0,1,1,0,0.,0.,0,N_mdm(int,previousSamples)}
		},
	kw_221[4] = {
		{"incremental_lhs",8,1,1,1,kw_220,0.,0.,0,N_mdm(utype,sampleType_SUBMETHOD_INCREMENTAL_LHS)},
		{"incremental_random",8,1,1,1,kw_220,0.,0.,0,N_mdm(utype,sampleType_SUBMETHOD_INCREMENTAL_RANDOM)},
		{"lhs",8,0,1,1,0,0.,0.,0,N_mdm(utype,sampleType_SUBMETHOD_LHS)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(utype,sampleType_SUBMETHOD_RANDOM)}
		},
	kw_222[1] = {
		{"drop_tolerance",10,0,1,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)}
		},
	kw_223[7] = {
		{0,0,1,0,0,kw_23},
		{0,0,2,0,0,kw_24},
		{0,0,4,0,0,kw_29},
		{0,0,2,0,0,kw_194},
		{0,0,1,0,0,kw_195},
		{"sample_type",8,4,1,0,kw_221},
		{"variance_based_decomp",8,1,2,0,kw_222,0.,0.,0,N_mdm(true,vbdFlag)}
		},
	kw_224[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(false,approxExportAnnotated)}
		},
	kw_225[2] = {
		{"generalized",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_GENERALIZED)},
		{"sobol",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_SOBOL)}
		},
	kw_226[3] = {
		{"dimension_adaptive",8,2,1,1,kw_225},
		{"local_adaptive",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_LOCAL_ADAPTIVE_CONTROL)},
		{"uniform",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_UNIFORM_CONTROL)}
		},
	kw_227[2] = {
		{"generalized",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_GENERALIZED)},
		{"sobol",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_SOBOL)}
		},
	kw_228[2] = {
		{"dimension_adaptive",8,2,1,1,kw_227},
		{"uniform",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_UNIFORM_CONTROL)}
		},
	kw_229[4] = {
		{"adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_AIS)},
		{"import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_IS)},
		{"mm_adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_MMAIS)},
		{"refinement_samples",9,0,2,0,0,0.,0.,0,N_mdm(int,refineSamples)}
		},
	kw_230[2] = {
		{"lhs",8,0,1,1,0,0.,0.,0,N_mdm(utype,sampleType_SUBMETHOD_LHS)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(utype,sampleType_SUBMETHOD_RANDOM)}
		},
	kw_231[4] = {
		{"hierarchical",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionBasisType_HIERARCHICAL_INTERPOLANT)},
		{"nodal",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionBasisType_NODAL_INTERPOLANT)},
		{"restricted",8,0,1,0,0,0.,0.,0,N_mdm(type,growthOverride_RESTRICTED)},
		{"unrestricted",8,0,1,0,0,0.,0.,0,N_mdm(type,growthOverride_UNRESTRICTED)}
		},
	kw_232[2] = {
		{"drop_tolerance",10,0,2,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)},
		{"interaction_order",0x19,0,1,0,0,0.,0.,0,N_mdm(ushint,vbdOrder)}
		},
	kw_233[23] = {
		{0,0,1,0,0,kw_23},
		{0,0,2,0,0,kw_24},
		{0,0,4,0,0,kw_29},
		{0,0,2,0,0,kw_194},
		{0,0,1,0,0,kw_195},
		{"askey",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionType_ASKEY_U)},
		{"diagonal_covariance",8,0,8,0,0,0.,0.,0,N_mdm(type,covarianceControl_DIAGONAL_COVARIANCE)},
		{"dimension_preference",14,0,4,0,0,0.,0.,0,N_mdm(RealDL,anisoDimPref)},
		{"export_points_file",11,2,11,0,kw_224,0.,0.,0,N_mdm(str,approxExportFile)},
		{"full_covariance",8,0,8,0,0,0.,0.,0,N_mdm(type,covarianceControl_FULL_COVARIANCE)},
		{"h_refinement",8,3,1,0,kw_226,0.,0.,0,N_mdm(type,refinementType_H_REFINEMENT)},
		{"nested",8,0,6,0,0,0.,0.,0,N_mdm(type,nestingOverride_NESTED)},
		{"non_nested",8,0,6,0,0,0.,0.,0,N_mdm(type,nestingOverride_NON_NESTED)},
		{"p_refinement",8,2,1,0,kw_228,0.,0.,0,N_mdm(type,refinementType_P_REFINEMENT)},
		{"piecewise",8,0,2,0,0,0.,0.,0,NIDRProblemDescDB::method_piecewise},
		{"probability_refinement",8,4,10,0,kw_229},
		{"quadrature_order",13,0,3,1,0,0.,0.,0,N_mdm(usharray,quadratureOrder)},
		{"sample_refinement",0,4,10,0,kw_229,0.,0.,-2},
		{"sample_type",8,2,9,0,kw_230},
		{"sparse_grid_level",13,4,3,1,kw_231,0.,0.,0,N_mdm(usharray,sparseGridLevel)},
		{"use_derivatives",8,0,5,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)},
		{"variance_based_decomp",8,2,7,0,kw_232,0.,0.,0,N_mdm(true,vbdFlag)},
		{"wiener",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionType_STD_NORMAL_U)}
		},
	kw_234[2] = {
		{0,0,1,0,0,kw_23},
		{"misc_options",15,0,1,0,0,0.,0.,0,N_mdm(strL,miscOptions)}
		},
	kw_235[3] = {
		{"function_precision",10,0,2,0,0,0.,0.,0,N_mdm(Real,functionPrecision)},
		{"linesearch_tolerance",10,0,3,0,0,0.,0.,0,N_mdm(Real,lineSearchTolerance)},
		{"verify_level",9,0,1,0,0,0.,0.,0,N_mdm(int,verifyLevel)}
		},
	kw_236[4] = {
		{0,0,1,0,0,kw_23},
		{0,0,9,0,0,kw_38},
		{0,0,3,0,0,kw_235},
		{""}
		},
	kw_237[2] = {
		{"gradient_tolerance",10,0,2,0,0,0.,0.,0,N_mdm(Real,gradientTolerance)},
		{"max_step",10,0,1,0,0,0.,0.,0,N_mdm(Real,maxStep)}
		},
	kw_238[4] = {
		{0,0,1,0,0,kw_23},
		{0,0,9,0,0,kw_38},
		{0,0,2,0,0,kw_237},
		{""}
		},
	kw_239[3] = {
		{0,0,1,0,0,kw_23},
		{0,0,9,0,0,kw_38},
		{"search_scheme_size",9,0,1,0,0,0.,0.,0,N_mdm(int,searchSchemeSize)}
		},
	kw_240[3] = {
		{"argaez_tapia",8,0,1,1,0,0.,0.,0,N_mdm(type,meritFn_ArgaezTapia)},
		{"el_bakry",8,0,1,1,0,0.,0.,0,N_mdm(type,meritFn_NormFmu)},
		{"van_shanno",8,0,1,1,0,0.,0.,0,N_mdm(type,meritFn_VanShanno)}
		},
	kw_241[4] = {
		{"gradient_based_line_search",8,0,1,1,0,0.,0.,0,N_mdm(lit,searchMethod_gradient_based_line_search)},
		{"tr_pds",8,0,1,1,0,0.,0.,0,N_mdm(lit,searchMethod_tr_pds)},
		{"trust_region",8,0,1,1,0,0.,0.,0,N_mdm(lit,searchMethod_trust_region)},
		{"value_based_line_search",8,0,1,1,0,0.,0.,0,N_mdm(lit,searchMethod_value_based_line_search)}
		},
	kw_242[7] = {
		{0,0,1,0,0,kw_23},
		{0,0,9,0,0,kw_38},
		{0,0,2,0,0,kw_237},
		{"centering_parameter",10,0,4,0,0,0.,0.,0,N_mdm(Real,centeringParam)},
		{"merit_function",8,3,2,0,kw_240},
		{"search_method",8,4,1,0,kw_241},
		{"steplength_to_boundary",10,0,3,0,0,0.,0.,0,N_mdm(Real,stepLenToBoundary)}
		},
	kw_243[5] = {
		{"debug",8,0,1,1,0,0.,0.,0,N_mdm(type,methodOutput_DEBUG_OUTPUT)},
		{"normal",8,0,1,1,0,0.,0.,0,N_mdm(type,methodOutput_NORMAL_OUTPUT)},
		{"quiet",8,0,1,1,0,0.,0.,0,N_mdm(type,methodOutput_QUIET_OUTPUT)},
		{"silent",8,0,1,1,0,0.,0.,0,N_mdm(type,methodOutput_SILENT_OUTPUT)},
		{"verbose",8,0,1,1,0,0.,0.,0,N_mdm(type,methodOutput_VERBOSE_OUTPUT)}
		},
	kw_244[2] = {
		{"model_pointer",11,0,1,0,0,0.,0.,0,N_mdm(str,subModelPointer)},
		{"opt_model_pointer",3,0,1,0,0,0.,0.,-1,N_mdm(str,subModelPointer)}
		},
	kw_245[1] = {
		{"seed",9,0,1,0,0,0.,0.,0,N_mdm(int,randomSeed)}
		},
	kw_246[8] = {
		{0,0,3,0,0,kw_134},
		{"method_name",11,2,1,1,kw_244,0.,0.,0,N_mdm(str,subMethodName)},
		{"method_pointer",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodPointer)},
		{"multi_objective_weight_sets",6,0,3,0,0,0.,0.,4,N_mdm(RealDL,concurrentParameterSets)},
		{"opt_method_name",3,2,1,1,kw_244,0.,0.,-3,N_mdm(str,subMethodName)},
		{"opt_method_pointer",3,0,1,1,0,0.,0.,-3,N_mdm(str,subMethodPointer)},
		{"random_weight_sets",9,1,2,0,kw_245,0.,0.,0,N_mdm(int,concurrentRandomJobs)},
		{"weight_sets",14,0,3,0,0,0.,0.,0,N_mdm(RealDL,concurrentParameterSets)}
		},
	kw_247[3] = {
		{0,0,1,0,0,kw_23},
		{0,0,2,0,0,kw_24},
		{"partitions",13,0,1,0,0,0.,0.,0,N_mdm(usharray,varPartitions)}
		},
	kw_248[5] = {
		{0,0,1,0,0,kw_23},
		{"converge_order",8,0,1,1,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_CONVERGE_ORDER)},
		{"converge_qoi",8,0,1,1,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_CONVERGE_QOI)},
		{"estimate_order",8,0,1,1,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_ESTIMATE_ORDER)},
		{"refinement_rate",10,0,2,0,0,0.,0.,0,N_mdm(Real,refinementRate)}
		},
	kw_249[2] = {
		{"num_generations",0x29,0,2,0,0,0.,0.,0,N_mdm(sizet,numGenerations)},
		{"percent_change",10,0,1,0,0,0.,0.,0,N_mdm(Realz,convergenceTolerance)}
		},
	kw_250[2] = {
		{"num_generations",0x29,0,2,0,0,0.,0.,0,N_mdm(sizet,numGenerations)},
		{"percent_change",10,0,1,0,0,0.,0.,0,N_mdm(Realz,convergenceTolerance)}
		},
	kw_251[2] = {
		{"average_fitness_tracker",8,2,1,1,kw_249,0.,0.,0,N_mdm(lit,convergenceType_average_fitness_tracker)},
		{"best_fitness_tracker",8,2,1,1,kw_250,0.,0.,0,N_mdm(lit,convergenceType_best_fitness_tracker)}
		},
	kw_252[2] = {
		{"constraint_penalty",10,0,2,0,0,0.,0.,0,N_mdm(Realp,constraintTolerance)},
		{"merit_function",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_merit_function)}
		},
	kw_253[4] = {
		{"elitist",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_elitist)},
		{"favor_feasible",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_favor_feasible)},
		{"roulette_wheel",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_roulette_wheel)},
		{"unique_roulette_wheel",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_unique_roulette_wheel)}
		},
	kw_254[6] = {
		{0,0,1,0,0,kw_23},
		{0,0,9,0,0,kw_38},
		{0,0,7,0,0,kw_171},
		{"convergence_type",8,2,3,0,kw_251},
		{"fitness_type",8,2,1,0,kw_252},
		{"replacement_type",8,4,2,0,kw_253}
		},
	kw_255[5] = {
		{0,0,1,0,0,kw_23},
		{0,0,9,0,0,kw_38},
		{0,0,3,0,0,kw_235},
		{"nlssol",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_NLSSOL_SQP)},
		{"npsol",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_NPSOL_SQP)}
		},
	kw_256[7] = {
		{"approx_method_name",3,0,1,1,0,0.,0.,3,N_mdm(str,subMethodName)},
		{"approx_method_pointer",3,0,1,1,0,0.,0.,3,N_mdm(str,subMethodPointer)},
		{"approx_model_pointer",3,0,2,2,0,0.,0.,3,N_mdm(str,modelPointer)},
		{"method_name",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodName)},
		{"method_pointer",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodPointer)},
		{"model_pointer",11,0,2,2,0,0.,0.,0,N_mdm(str,modelPointer)},
		{"replace_points",8,0,3,0,0,0.,0.,0,N_mdm(true,surrBasedGlobalReplacePts)}
		},
	kw_257[2] = {
		{"filter",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalAcceptLogic_FILTER)},
		{"tr_ratio",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalAcceptLogic_TR_RATIO)}
		},
	kw_258[7] = {
		{"augmented_lagrangian_objective",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbObj_AUGMENTED_LAGRANGIAN_OBJECTIVE)},
		{"lagrangian_objective",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbObj_LAGRANGIAN_OBJECTIVE)},
		{"linearized_constraints",8,0,2,2,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbCon_LINEARIZED_CONSTRAINTS)},
		{"no_constraints",8,0,2,2,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbCon_NO_CONSTRAINTS)},
		{"original_constraints",8,0,2,2,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbCon_ORIGINAL_CONSTRAINTS)},
		{"original_primary",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbObj_ORIGINAL_PRIMARY)},
		{"single_objective",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbObj_SINGLE_OBJECTIVE)}
		},
	kw_259[1] = {
		{"homotopy",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalConstrRelax_HOMOTOPY)}
		},
	kw_260[4] = {
		{"adaptive_penalty_merit",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalMeritFn_ADAPTIVE_PENALTY_MERIT)},
		{"augmented_lagrangian_merit",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalMeritFn_AUGMENTED_LAGRANGIAN_MERIT)},
		{"lagrangian_merit",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalMeritFn_LAGRANGIAN_MERIT)},
		{"penalty_merit",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalMeritFn_PENALTY_MERIT)}
		},
	kw_261[6] = {
		{"contract_threshold",10,0,3,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRContractTrigger)},
		{"contraction_factor",10,0,5,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRContract)},
		{"expand_threshold",10,0,4,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRExpandTrigger)},
		{"expansion_factor",10,0,6,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRExpand)},
		{"initial_size",10,0,1,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRInitSize)},
		{"minimum_size",10,0,2,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRMinSize)}
		},
	kw_262[13] = {
		{"acceptance_logic",8,2,8,0,kw_257},
		{"approx_method_name",3,0,1,1,0,0.,0.,6,N_mdm(str,subMethodName)},
		{"approx_method_pointer",3,0,1,1,0,0.,0.,6,N_mdm(str,subMethodPointer)},
		{"approx_model_pointer",3,0,2,2,0,0.,0.,6,N_mdm(str,modelPointer)},
		{"approx_subproblem",8,7,6,0,kw_258},
		{"constraint_relax",8,1,9,0,kw_259},
		{"merit_function",8,4,7,0,kw_260},
		{"method_name",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodName)},
		{"method_pointer",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodPointer)},
		{"model_pointer",11,0,2,2,0,0.,0.,0,N_mdm(str,modelPointer)},
		{"soft_convergence_limit",9,0,3,0,0,0.,0.,0,N_mdm(ushint,softConvLimit)},
		{"trust_region",8,6,5,0,kw_261,0.,0.,0,0,0,NIDRProblemDescDB::method_tr_final},
		{"truth_surrogate_bypass",8,0,4,0,0,0.,0.,0,N_mdm(true,surrBasedLocalLayerBypass)}
		},
	kw_263[4] = {
		{0,0,1,0,0,kw_23},
		{"final_point",14,0,1,1,0,0.,0.,0,N_mdm(RealDL,finalPoint)},
		{"num_steps",9,0,2,2,0,0.,0.,0,N_mdm(int,numSteps)},
		{"step_vector",14,0,1,1,0,0.,0.,0,N_mdm(RealDL,stepVector)}
		},
	kw_264[90] = {
		{"adaptive_sampling",8,8,10,1,kw_37,0.,0.,0,N_mdm(utype,methodName_ADAPTIVE_SAMPLING)},
		{"asynch_pattern_search",8,9,10,1,kw_41,0.,0.,0,N_mdm(utype,methodName_ASYNCH_PATTERN_SEARCH)},
		{"bayes_calibration",8,6,10,1,kw_65,0.,0.,0,N_mdm(utype,methodName_BAYES_CALIBRATION)},
		{"centered_parameter_study",8,3,10,1,kw_66,0.,0.,0,N_mdm(utype,methodName_CENTERED_PARAMETER_STUDY)},
		{"coliny_apps",0,9,10,1,kw_41,0.,0.,-3,N_mdm(utype,methodName_ASYNCH_PATTERN_SEARCH)},
		{"coliny_beta",8,1,10,1,kw_68,0.,0.,0,N_mdm(utype,methodName_COLINY_BETA)},
		{"coliny_cobyla",8,0,10,1,kw_70,0.,0.,0,N_mdm(utype,methodName_COLINY_COBYLA)},
		{"coliny_direct",8,6,10,1,kw_72,0.,0.,0,N_mdm(utype,methodName_COLINY_DIRECT)},
		{"coliny_ea",8,9,10,1,kw_79,0.,0.,0,N_mdm(utype,methodName_COLINY_EA)},
		{"coliny_pattern_search",8,8,10,1,kw_84,0.,0.,0,N_mdm(utype,methodName_COLINY_PATTERN_SEARCH)},
		{"coliny_solis_wets",8,4,10,1,kw_85,0.,0.,0,N_mdm(utype,methodName_COLINY_SOLIS_WETS)},
		{"conmin",8,2,10,1,kw_86},
		{"conmin_frcg",8,0,10,1,kw_87,0.,0.,0,N_mdm(utype,methodName_CONMIN_FRCG)},
		{"conmin_mfd",8,0,10,1,kw_87,0.,0.,0,N_mdm(utype,methodName_CONMIN_MFD)},
		{"constraint_tolerance",10,0,7,0,0,0.,0.,0,N_mdm(Real,constraintTolerance)},
		{"convergence_tolerance",10,0,6,0,0,0.,0.,0,N_mdm(Real,convergenceTolerance)},
		{"dace",8,12,10,1,kw_89,0.,0.,0,N_mdm(utype,methodName_DACE)},
		{"dl_solver",11,0,10,1,kw_87,0.,0.,0,N_mdm(utype_lit,TYPE_DATA_methodName_DL_SOLVER)},
		{"dot",8,5,10,1,kw_90},
		{"dot_bfgs",8,0,10,1,kw_87,0.,0.,0,N_mdm(utype,methodName_DOT_BFGS)},
		{"dot_frcg",8,0,10,1,kw_87,0.,0.,0,N_mdm(utype,methodName_DOT_FRCG)},
		{"dot_mmfd",8,0,10,1,kw_87,0.,0.,0,N_mdm(utype,methodName_DOT_MMFD)},
		{"dot_slp",8,0,10,1,kw_87,0.,0.,0,N_mdm(utype,methodName_DOT_SLP)},
		{"dot_sqp",8,0,10,1,kw_87,0.,0.,0,N_mdm(utype,methodName_DOT_SQP)},
		{"efficient_global",8,6,10,1,kw_94,0.,0.,0,N_mdm(utype,methodName_EFFICIENT_GLOBAL)},
		{"efficient_subspace",8,2,10,1,kw_95,0.,0.,0,N_mdm(utype,methodName_EFFICIENT_SUBSPACE)},
		{"final_solutions",0x29,0,9,0,0,0.,0.,0,N_mdm(sizet,numFinalSolutions)},
		{"fsu_cvt",8,6,10,1,kw_98,0.,0.,0,N_mdm(utype,methodName_FSU_CVT)},
		{"fsu_quasi_mc",8,10,10,1,kw_100},
		{"gaussian_process_adaptive_importance_sampling",0,4,10,1,kw_106,0.,0.,6,N_mdm(utype,methodName_GPAIS)},
		{"genie_direct",8,1,10,1,kw_107,0.,0.,0,N_mdm(utype,methodName_GENIE_DIRECT)},
		{"genie_opt_darts",8,1,10,1,kw_107,0.,0.,0,N_mdm(utype,methodName_GENIE_OPT_DARTS)},
		{"global_evidence",8,5,10,1,kw_115,0.,0.,0,N_mdm(utype,methodName_GLOBAL_EVIDENCE)},
		{"global_interval_est",8,5,10,1,kw_121,0.,0.,0,N_mdm(utype,methodName_GLOBAL_INTERVAL_EST)},
		{"global_reliability",8,12,10,1,kw_132,0.,0.,0,N_mdm(utype,methodName_GLOBAL_RELIABILITY)},
		{"gpais",8,4,10,1,kw_106,0.,0.,0,N_mdm(utype,methodName_GPAIS)},
		{"hybrid",8,5,10,1,kw_142,0.,0.,0,N_mdm(utype,methodName_HYBRID)},
		{"id_method",11,0,1,0,0,0.,0.,0,N_mdm(str,idMethod)},
		{"importance_sampling",8,5,10,1,kw_146,0.,0.,0,N_mdm(utype,methodName_IMPORTANCE_SAMPLING)},
		{"list_parameter_study",8,2,10,1,kw_148,0.,0.,0,N_mdm(utype,methodName_LIST_PARAMETER_STUDY)},
		{"local_evidence",8,6,10,1,kw_155,0.,0.,0,N_mdm(utype,methodName_LOCAL_EVIDENCE)},
		{"local_interval_est",8,2,10,1,kw_156,0.,0.,0,N_mdm(utype,methodName_LOCAL_INTERVAL_EST)},
		{"local_reliability",8,3,10,1,kw_164,0.,0.,0,N_mdm(utype,methodName_LOCAL_RELIABILITY)},
		{"max_function_evaluations",0x29,0,4,0,0,0.,0.,0,N_mdm(nnint,maxFunctionEvaluations)},
		{"max_iterations",0x29,0,3,0,0,0.,0.,0,N_mdm(nnint,maxIterations)},
		{"mesh_adaptive_search",8,6,10,1,kw_165,0.,0.,0,N_mdm(utype,methodName_MESH_ADAPTIVE_SEARCH)},
		{"moga",8,5,10,1,kw_179,0.,0.,0,N_mdm(utype,methodName_MOGA)},
		{"multi_start",8,4,10,1,kw_182,0.,0.,0,N_mdm(utype,methodName_MULTI_START)},
		{"multidim_parameter_study",8,1,10,1,kw_183,0.,0.,0,N_mdm(utype,methodName_MULTIDIM_PARAMETER_STUDY)},
		{"ncsu_direct",8,4,10,1,kw_184,0.,0.,0,N_mdm(utype,methodName_NCSU_DIRECT)},
		{"nl2sol",8,9,10,1,kw_185,0.,0.,0,N_mdm(utype,methodName_NL2SOL)},
		{"nlpql_sqp",8,0,10,1,kw_87,0.,0.,0,N_mdm(utype,methodName_NLPQL_SQP)},
		{"nlssol_sqp",8,0,10,1,kw_236,0.,0.,0,N_mdm(utype,methodName_NLSSOL_SQP)},
		{"nond_adaptive_sampling",0,8,10,1,kw_37,0.,0.,-53,N_mdm(utype,methodName_ADAPTIVE_SAMPLING)},
		{"nond_bayes_calibration",0,6,10,1,kw_65,0.,0.,-52,N_mdm(utype,methodName_BAYES_CALIBRATION)},
		{"nond_efficient_subspace",0,2,10,1,kw_95,0.,0.,-30,N_mdm(utype,methodName_EFFICIENT_SUBSPACE)},
		{"nond_global_evidence",0,5,10,1,kw_115,0.,0.,-24,N_mdm(utype,methodName_GLOBAL_EVIDENCE)},
		{"nond_global_interval_est",0,5,10,1,kw_121,0.,0.,-24,N_mdm(utype,methodName_GLOBAL_INTERVAL_EST)},
		{"nond_global_reliability",0,12,10,1,kw_132,0.,0.,-24,N_mdm(utype,methodName_GLOBAL_RELIABILITY)},
		{"nond_importance_sampling",0,5,10,1,kw_146,0.,0.,-21,N_mdm(utype,methodName_IMPORTANCE_SAMPLING)},
		{"nond_local_evidence",0,6,10,1,kw_155,0.,0.,-20,N_mdm(utype,methodName_LOCAL_EVIDENCE)},
		{"nond_local_interval_est",0,2,10,1,kw_156,0.,0.,-20,N_mdm(utype,methodName_LOCAL_INTERVAL_EST)},
		{"nond_local_reliability",0,3,10,1,kw_164,0.,0.,-20,N_mdm(utype,methodName_LOCAL_RELIABILITY)},
		{"nond_pof_darts",0,1,10,1,kw_189,0.,0.,14,N_mdm(utype,methodName_POF_DARTS)},
		{"nond_polynomial_chaos",0,20,10,1,kw_219,0.,0.,14,N_mdm(utype,methodName_POLYNOMIAL_CHAOS)},
		{"nond_sampling",0,2,10,1,kw_223,0.,0.,16,N_mdm(utype,methodName_RANDOM_SAMPLING)},
		{"nond_stoch_collocation",0,18,10,1,kw_233,0.,0.,20,N_mdm(utype,methodName_STOCH_COLLOCATION)},
		{"nonlinear_cg",8,1,10,1,kw_234,0.,0.,0,N_mdm(utype,methodName_NONLINEAR_CG)},
		{"npsol_sqp",8,0,10,1,kw_236,0.,0.,0,N_mdm(utype,methodName_NPSOL_SQP)},
		{"optpp_cg",8,0,10,1,kw_238,0.,0.,0,N_mdm(utype,methodName_OPTPP_CG)},
		{"optpp_fd_newton",8,4,10,1,kw_242,0.,0.,0,N_mdm(utype,methodName_OPTPP_FD_NEWTON)},
		{"optpp_g_newton",8,4,10,1,kw_242,0.,0.,0,N_mdm(utype,methodName_OPTPP_G_NEWTON)},
		{"optpp_newton",8,4,10,1,kw_242,0.,0.,0,N_mdm(utype,methodName_OPTPP_NEWTON)},
		{"optpp_pds",8,1,10,1,kw_239,0.,0.,0,N_mdm(utype,methodName_OPTPP_PDS)},
		{"optpp_q_newton",8,4,10,1,kw_242,0.,0.,0,N_mdm(utype,methodName_OPTPP_Q_NEWTON)},
		{"output",8,5,2,0,kw_243},
		{"pareto_set",8,7,10,1,kw_246,0.,0.,0,N_mdm(utype,methodName_PARETO_SET)},
		{"pof_darts",8,1,10,1,kw_189,0.,0.,0,N_mdm(utype,methodName_POF_DARTS)},
		{"polynomial_chaos",8,20,10,1,kw_219,0.,0.,0,N_mdm(utype,methodName_POLYNOMIAL_CHAOS)},
		{"psuade_moat",8,1,10,1,kw_247,0.,0.,0,N_mdm(utype,methodName_PSUADE_MOAT)},
		{"richardson_extrap",8,4,10,1,kw_248,0.,0.,0,N_mdm(utype,methodName_RICHARDSON_EXTRAP)},
		{"sampling",8,2,10,1,kw_223,0.,0.,0,N_mdm(utype,methodName_RANDOM_SAMPLING)},
		{"scaling",8,0,8,0,0,0.,0.,0,N_mdm(true,methodScaling)},
		{"soga",8,3,10,1,kw_254,0.,0.,0,N_mdm(utype,methodName_SOGA)},
		{"speculative",8,0,5,0,0,0.,0.,0,N_mdm(true,speculativeFlag)},
		{"stanford",8,2,10,1,kw_255},
		{"stoch_collocation",8,18,10,1,kw_233,0.,0.,0,N_mdm(utype,methodName_STOCH_COLLOCATION)},
		{"surrogate_based_global",8,7,10,1,kw_256,0.,0.,0,N_mdm(utype,methodName_SURROGATE_BASED_GLOBAL)},
		{"surrogate_based_local",8,13,10,1,kw_262,0.,0.,0,N_mdm(utype,methodName_SURROGATE_BASED_LOCAL)},
		{"vector_parameter_study",8,3,10,1,kw_263,0.,0.,0,N_mdm(utype,methodName_VECTOR_PARAMETER_STUDY)}
		},
	kw_265[1] = {
		{"optional_interface_responses_pointer",11,0,1,0,0,0.,0.,0,N_mom(str,optionalInterfRespPointer)}
		},
	kw_266[2] = {
		{"master",8,0,1,1,0,0.,0.,0,N_mom(type,subMethodScheduling_MASTER_SCHEDULING)},
		{"peer",8,0,1,1,0,0.,0.,0,N_mom(type,subMethodScheduling_PEER_SCHEDULING)}
		},
	kw_267[7] = {
		{"iterator_scheduling",8,2,2,0,kw_266},
		{"iterator_servers",0x19,0,1,0,0,0.,0.,0,N_mom(pint,subMethodServers)},
		{"primary_response_mapping",14,0,6,0,0,0.,0.,0,N_mom(RealDL,primaryRespCoeffs)},
		{"primary_variable_mapping",15,0,4,0,0,0.,0.,0,N_mom(strL,primaryVarMaps)},
		{"processors_per_iterator",0x19,0,3,0,0,0.,0.,0,N_mom(pint,subMethodProcs)},
		{"secondary_response_mapping",14,0,7,0,0,0.,0.,0,N_mom(RealDL,secondaryRespCoeffs)},
		{"secondary_variable_mapping",15,0,5,0,0,0.,0.,0,N_mom(strL,secondaryVarMaps)}
		},
	kw_268[2] = {
		{"optional_interface_pointer",11,1,1,0,kw_265,0.,0.,0,N_mom(str,interfacePointer)},
		{"sub_method_pointer",11,7,2,1,kw_267,0.,0.,0,N_mom(str,subMethodPointer)}
		},
	kw_269[1] = {
		{"interface_pointer",11,0,1,0,0,0.,0.,0,N_mom(str,interfacePointer)}
		},
	kw_270[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mom(true,approxChallengeAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mom(false,approxChallengeAnnotated)}
		},
	kw_271[6] = {
		{"additive",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_ADDITIVE_CORRECTION)},
		{"combined",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_COMBINED_CORRECTION)},
		{"first_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_1)},
		{"multiplicative",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_MULTIPLICATIVE_CORRECTION)},
		{"second_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_2)},
		{"zeroth_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_0)}
		},
	kw_272[2] = {
		{"folds",9,0,1,0,0,0.,0.,0,N_mom(int,numFolds)},
		{"percent",10,0,1,0,0,0.,0.,0,N_mom(Real,percentFold)}
		},
	kw_273[2] = {
		{"cross_validation",8,2,1,0,kw_272,0.,0.,0,N_mom(true,crossValidateFlag)},
		{"press",8,0,2,0,0,0.,0.,0,N_mom(true,pressFlag)}
		},
	kw_274[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mom(true,approxExportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mom(false,approxExportAnnotated)}
		},
	kw_275[3] = {
		{"constant",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_constant)},
		{"linear",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_linear)},
		{"reduced_quadratic",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_reduced_quadratic)}
		},
	kw_276[2] = {
		{"point_selection",8,0,1,0,0,0.,0.,0,N_mom(true,pointSelection)},
		{"trend",8,3,2,0,kw_275}
		},
	kw_277[4] = {
		{"constant",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_constant)},
		{"linear",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_linear)},
		{"quadratic",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_quadratic)},
		{"reduced_quadratic",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_reduced_quadratic)}
		},
	kw_278[7] = {
		{"correlation_lengths",14,0,5,0,0,0.,0.,0,N_mom(RealDL,krigingCorrelations)},
		{"export_model_file",11,0,6,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"find_nugget",9,0,4,0,0,0.,0.,0,N_mom(shint,krigingFindNugget)},
		{"max_trials",0x19,0,3,0,0,0.,0.,0,N_mom(shint,krigingMaxTrials)},
		{"nugget",0x1a,0,4,0,0,0.,0.,0,N_mom(Real,krigingNugget)},
		{"optimization_method",11,0,2,0,0,0.,0.,0,N_mom(str,krigingOptMethod)},
		{"trend",8,4,1,0,kw_277}
		},
	kw_279[2] = {
		{"dakota",8,2,1,1,kw_276,0.,0.,0,N_mom(lit,surrogateType_global_gaussian)},
		{"surfpack",8,7,1,1,kw_278,0.,0.,0,N_mom(lit,surrogateType_global_kriging)}
		},
	kw_280[2] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mom(true,approxImportAnnotated)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mom(false,approxImportAnnotated)}
		},
	kw_281[2] = {
		{"cubic",8,0,1,1,0,0.,0.,0,N_mom(lit,marsInterpolation_cubic)},
		{"linear",8,0,1,1,0,0.,0.,0,N_mom(lit,marsInterpolation_linear)}
		},
	kw_282[3] = {
		{"export_model_file",11,0,3,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"interpolation",8,2,2,0,kw_281},
		{"max_bases",9,0,1,0,0,0.,0.,0,N_mom(shint,marsMaxBases)}
		},
	kw_283[3] = {
		{"export_model_file",11,0,3,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"poly_order",9,0,1,0,0,0.,0.,0,N_mom(shint,mlsPolyOrder)},
		{"weight_function",9,0,2,0,0,0.,0.,0,N_mom(shint,mlsWeightFunction)}
		},
	kw_284[4] = {
		{"export_model_file",11,0,4,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"nodes",9,0,1,0,0,0.,0.,0,N_mom(shint,annNodes)},
		{"random_weight",9,0,3,0,0,0.,0.,0,N_mom(shint,annRandomWeight)},
		{"range",10,0,2,0,0,0.,0.,0,N_mom(Real,annRange)}
		},
	kw_285[4] = {
		{"cubic",8,0,1,1,0,0.,0.,0,N_mom(order,polynomialOrder_3)},
		{"export_model_file",11,0,2,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"linear",8,0,1,1,0,0.,0.,0,N_mom(order,polynomialOrder_1)},
		{"quadratic",8,0,1,1,0,0.,0.,0,N_mom(order,polynomialOrder_2)}
		},
	kw_286[5] = {
		{"bases",9,0,1,0,0,0.,0.,0,N_mom(shint,rbfBases)},
		{"export_model_file",11,0,5,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"max_pts",9,0,2,0,0,0.,0.,0,N_mom(shint,rbfMaxPts)},
		{"max_subsets",9,0,4,0,0,0.,0.,0,N_mom(shint,rbfMaxSubsets)},
		{"min_partition",9,0,3,0,0,0.,0.,0,N_mom(shint,rbfMinPartition)}
		},
	kw_287[3] = {
		{"all",8,0,1,1,0,0.,0.,0,N_mom(lit,approxPointReuse_all)},
		{"none",8,0,1,1,0,0.,0.,0,N_mom(lit,approxPointReuse_none)},
		{"region",8,0,1,1,0,0.,0.,0,N_mom(lit,approxPointReuse_region)}
		},
	kw_288[21] = {
		{"challenge_points_file",11,2,10,0,kw_270,0.,0.,0,N_mom(str,approxChallengeFile)},
		{"correction",8,6,8,0,kw_271},
		{"dace_method_pointer",11,0,3,0,0,0.,0.,0,N_mom(str,subMethodPointer)},
		{"diagnostics",7,2,9,0,kw_273,0.,0.,6,N_mom(strL,diagMetrics)},
		{"export_points_file",11,2,6,0,kw_274,0.,0.,0,N_mom(str,approxExportFile)},
		{"gaussian_process",8,2,1,1,kw_279},
		{"import_points_file",11,2,5,0,kw_280,0.,0.,0,N_mom(str,approxImportFile)},
		{"kriging",0,2,1,1,kw_279,0.,0.,-2},
		{"mars",8,3,1,1,kw_282,0.,0.,0,N_mom(lit,surrogateType_global_mars)},
		{"metrics",15,2,9,0,kw_273,0.,0.,0,N_mom(strL,diagMetrics)},
		{"minimum_points",8,0,2,0,0,0.,0.,0,N_mom(type,pointsManagement_MINIMUM_POINTS)},
		{"moving_least_squares",8,3,1,1,kw_283,0.,0.,0,N_mom(lit,surrogateType_global_moving_least_squares)},
		{"neural_network",8,4,1,1,kw_284,0.,0.,0,N_mom(lit,surrogateType_global_neural_network)},
		{"polynomial",8,4,1,1,kw_285,0.,0.,0,N_mom(lit,surrogateType_global_polynomial)},
		{"radial_basis",8,5,1,1,kw_286,0.,0.,0,N_mom(lit,surrogateType_global_radial_basis)},
		{"recommended_points",8,0,2,0,0,0.,0.,0,N_mom(type,pointsManagement_RECOMMENDED_POINTS)},
		{"reuse_points",8,3,4,0,kw_287},
		{"reuse_samples",0,3,4,0,kw_287,0.,0.,-1},
		{"samples_file",3,2,5,0,kw_280,0.,0.,-12,N_mom(str,approxImportFile)},
		{"total_points",9,0,2,0,0,0.,0.,0,N_mom(int,pointsTotal)},
		{"use_derivatives",8,0,7,0,0,0.,0.,0,N_mom(true,modelUseDerivsFlag)}
		},
	kw_289[6] = {
		{"additive",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_ADDITIVE_CORRECTION)},
		{"combined",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_COMBINED_CORRECTION)},
		{"first_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_1)},
		{"multiplicative",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_MULTIPLICATIVE_CORRECTION)},
		{"second_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_2)},
		{"zeroth_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_0)}
		},
	kw_290[3] = {
		{"correction",8,6,3,3,kw_289},
		{"high_fidelity_model_pointer",11,0,2,2,0,0.,0.,0,N_mom(str,truthModelPointer)},
		{"low_fidelity_model_pointer",11,0,1,1,0,0.,0.,0,N_mom(str,lowFidelityModelPointer)}
		},
	kw_291[1] = {
		{"actual_model_pointer",11,0,1,1,0,0.,0.,0,N_mom(str,truthModelPointer)}
		},
	kw_292[2] = {
		{0,0,1,0,0,kw_291},
		{"taylor_series",8,0,1,1}
		},
	kw_293[2] = {
		{0,0,1,0,0,kw_291},
		{"tana",8,0,1,1}
		},
	kw_294[5] = {
		{"global",8,21,2,1,kw_288},
		{"hierarchical",8,3,2,1,kw_290,0.,0.,0,N_mom(lit,surrogateType_hierarchical)},
		{"id_surrogates",13,0,1,0,0,0.,0.,0,N_mom(intsetm1,surrogateFnIndices)},
		{"local",8,1,2,1,kw_292,0.,0.,0,N_mom(lit,surrogateType_local_taylor)},
		{"multipoint",8,1,2,1,kw_293,0.,0.,0,N_mom(lit,surrogateType_multipoint_tana)}
		},
	kw_295[7] = {
		{"hierarchical_tagging",8,0,4,0,0,0.,0.,0,N_mom(true,hierarchicalTags)},
		{"id_model",11,0,1,0,0,0.,0.,0,N_mom(str,idModel)},
		{"nested",8,2,5,1,kw_268,0.,0.,0,N_mom(lit,modelType_nested)},
		{"responses_pointer",11,0,3,0,0,0.,0.,0,N_mom(str,responsesPointer)},
		{"single",8,1,5,1,kw_269,0.,0.,0,N_mom(lit,modelType_single)},
		{"surrogate",8,5,5,1,kw_294,0.,0.,0,N_mom(lit,modelType_surrogate)},
		{"variables_pointer",11,0,2,0,0,0.,0.,0,N_mom(str,variablesPointer)}
		},
	kw_296[6] = {
		{"annotated",8,0,3,0,0,0.,0.,0,N_rem(true,expDataFileAnnotated)},
		{"freeform",8,0,3,0,0,0.,0.,0,N_rem(false,expDataFileAnnotated)},
		{"num_config_variables",0x29,0,4,0,0,0.,0.,0,N_rem(sizet,numExpConfigVars)},
		{"num_experiments",0x29,0,1,0,0,0.,0.,0,N_rem(sizet,numExperiments)},
		{"num_replicates",13,0,2,0,0,0.,0.,0,N_rem(ivec,numReplicates)},
		{"num_std_deviations",0x29,0,5,0,0,0.,0.,0,N_rem(sizet,numExpStdDeviations)}
		},
	kw_297[6] = {
		{"nonlinear_equality_scale_types",0x807,0,2,0,0,0.,0.,3,N_rem(strL,nonlinearEqScaleTypes)},
		{"nonlinear_equality_scales",0x806,0,3,0,0,0.,0.,3,N_rem(RealDL,nonlinearEqScales)},
		{"nonlinear_equality_targets",6,0,1,0,0,0.,0.,3,N_rem(RealDL,nonlinearEqTargets)},
		{"scale_types",0x80f,0,2,0,0,0.,0.,0,N_rem(strL,nonlinearEqScaleTypes)},
		{"scales",0x80e,0,3,0,0,0.,0.,0,N_rem(RealDL,nonlinearEqScales)},
		{"targets",14,0,1,0,0,0.,0.,0,N_rem(RealDL,nonlinearEqTargets)}
		},
	kw_298[8] = {
		{"lower_bounds",14,0,1,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqLowerBnds)},
		{"nonlinear_inequality_lower_bounds",6,0,1,0,0,0.,0.,-1,N_rem(RealDL,nonlinearIneqLowerBnds)},
		{"nonlinear_inequality_scale_types",0x807,0,3,0,0,0.,0.,3,N_rem(strL,nonlinearIneqScaleTypes)},
		{"nonlinear_inequality_scales",0x806,0,4,0,0,0.,0.,3,N_rem(RealDL,nonlinearIneqScales)},
		{"nonlinear_inequality_upper_bounds",6,0,2,0,0,0.,0.,3,N_rem(RealDL,nonlinearIneqUpperBnds)},
		{"scale_types",0x80f,0,3,0,0,0.,0.,0,N_rem(strL,nonlinearIneqScaleTypes)},
		{"scales",0x80e,0,4,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqScales)},
		{"upper_bounds",14,0,2,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqUpperBnds)}
		},
	kw_299[15] = {
		{"calibration_data_file",11,6,4,0,kw_296,0.,0.,0,N_rem(str,expDataFileName)},
		{"calibration_term_scale_types",0x807,0,1,0,0,0.,0.,11,N_rem(strL,primaryRespFnScaleTypes)},
		{"calibration_term_scales",0x806,0,2,0,0,0.,0.,11,N_rem(RealDL,primaryRespFnScales)},
		{"calibration_weights",6,0,3,0,0,0.,0.,11,N_rem(RealDL,primaryRespFnWeights)},
		{"least_squares_data_file",3,6,4,0,kw_296,0.,0.,-4,N_rem(str,expDataFileName)},
		{"least_squares_term_scale_types",0x807,0,1,0,0,0.,0.,7,N_rem(strL,primaryRespFnScaleTypes)},
		{"least_squares_term_scales",0x806,0,2,0,0,0.,0.,7,N_rem(RealDL,primaryRespFnScales)},
		{"least_squares_weights",6,0,3,0,0,0.,0.,7,N_rem(RealDL,primaryRespFnWeights)},
		{"nonlinear_equality_constraints",0x29,6,6,0,kw_297,0.,0.,0,N_rem(sizet,numNonlinearEqConstraints)},
		{"nonlinear_inequality_constraints",0x29,8,5,0,kw_298,0.,0.,0,N_rem(sizet,numNonlinearIneqConstraints)},
		{"num_nonlinear_equality_constraints",0x21,6,6,0,kw_297,0.,0.,-2,N_rem(sizet,numNonlinearEqConstraints)},
		{"num_nonlinear_inequality_constraints",0x21,8,5,0,kw_298,0.,0.,-2,N_rem(sizet,numNonlinearIneqConstraints)},
		{"primary_scale_types",0x80f,0,1,0,0,0.,0.,0,N_rem(strL,primaryRespFnScaleTypes)},
		{"primary_scales",0x80e,0,2,0,0,0.,0.,0,N_rem(RealDL,primaryRespFnScales)},
		{"weights",14,0,3,0,0,0.,0.,0,N_rem(RealDL,primaryRespFnWeights)}
		},
	kw_300[4] = {
		{"absolute",8,0,2,0,0,0.,0.,0,N_rem(lit,fdGradStepType_absolute)},
		{"bounds",8,0,2,0,0,0.,0.,0,N_rem(lit,fdGradStepType_bounds)},
		{"ignore_bounds",8,0,1,0,0,0.,0.,0,N_rem(true,ignoreBounds)},
		{"relative",8,0,2,0,0,0.,0.,0,N_rem(lit,fdGradStepType_relative)}
		},
	kw_301[8] = {
		{"central",8,0,4,0,0,0.,0.,0,N_rem(lit,intervalType_central)},
		{"dakota",8,4,2,0,kw_300,0.,0.,0,N_rem(lit,methodSource_dakota)},
		{"fd_gradient_step_size",6,0,5,0,0,0.,0.,1,N_rem(RealL,fdGradStepSize)},
		{"fd_step_size",14,0,5,0,0,0.,0.,0,N_rem(RealL,fdGradStepSize)},
		{"forward",8,0,4,0,0,0.,0.,0,N_rem(lit,intervalType_forward)},
		{"interval_type",8,0,3},
		{"method_source",8,0,1},
		{"vendor",8,0,2,0,0,0.,0.,0,N_rem(lit,methodSource_vendor)}
		},
	kw_302[3] = {
		{0,0,8,0,0,kw_301},
		{"id_analytic_gradients",13,0,2,2,0,0.,0.,0,N_rem(intset,idAnalyticGrads)},
		{"id_numerical_gradients",13,0,1,1,0,0.,0.,0,N_rem(intset,idNumericalGrads)}
		},
	kw_303[2] = {
		{"fd_hessian_step_size",6,0,1,0,0,0.,0.,1,N_rem(RealL,fdHessStepSize)},
		{"fd_step_size",14,0,1,0,0,0.,0.,0,N_rem(RealL,fdHessStepSize)}
		},
	kw_304[1] = {
		{"damped",8,0,1,0,0,0.,0.,0,N_rem(lit,quasiHessianType_damped_bfgs)}
		},
	kw_305[2] = {
		{"bfgs",8,1,1,1,kw_304,0.,0.,0,N_rem(lit,quasiHessianType_bfgs)},
		{"sr1",8,0,1,1,0,0.,0.,0,N_rem(lit,quasiHessianType_sr1)}
		},
	kw_306[8] = {
		{"absolute",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_absolute)},
		{"bounds",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_bounds)},
		{"central",8,0,3,0,0,0.,0.,0,N_rem(true,centralHess)},
		{"forward",8,0,3,0,0,0.,0.,0,N_rem(false,centralHess)},
		{"id_analytic_hessians",13,0,5,0,0,0.,0.,0,N_rem(intset,idAnalyticHessians)},
		{"id_numerical_hessians",13,2,1,0,kw_303,0.,0.,0,N_rem(intset,idNumericalHessians)},
		{"id_quasi_hessians",13,2,4,0,kw_305,0.,0.,0,N_rem(intset,idQuasiHessians)},
		{"relative",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_relative)}
		},
	kw_307[6] = {
		{"nonlinear_equality_scale_types",0x807,0,2,0,0,0.,0.,3,N_rem(strL,nonlinearEqScaleTypes)},
		{"nonlinear_equality_scales",0x806,0,3,0,0,0.,0.,3,N_rem(RealDL,nonlinearEqScales)},
		{"nonlinear_equality_targets",6,0,1,0,0,0.,0.,3,N_rem(RealDL,nonlinearEqTargets)},
		{"scale_types",0x80f,0,2,0,0,0.,0.,0,N_rem(strL,nonlinearEqScaleTypes)},
		{"scales",0x80e,0,3,0,0,0.,0.,0,N_rem(RealDL,nonlinearEqScales)},
		{"targets",14,0,1,0,0,0.,0.,0,N_rem(RealDL,nonlinearEqTargets)}
		},
	kw_308[8] = {
		{"lower_bounds",14,0,1,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqLowerBnds)},
		{"nonlinear_inequality_lower_bounds",6,0,1,0,0,0.,0.,-1,N_rem(RealDL,nonlinearIneqLowerBnds)},
		{"nonlinear_inequality_scale_types",0x807,0,3,0,0,0.,0.,3,N_rem(strL,nonlinearIneqScaleTypes)},
		{"nonlinear_inequality_scales",0x806,0,4,0,0,0.,0.,3,N_rem(RealDL,nonlinearIneqScales)},
		{"nonlinear_inequality_upper_bounds",6,0,2,0,0,0.,0.,3,N_rem(RealDL,nonlinearIneqUpperBnds)},
		{"scale_types",0x80f,0,3,0,0,0.,0.,0,N_rem(strL,nonlinearIneqScaleTypes)},
		{"scales",0x80e,0,4,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqScales)},
		{"upper_bounds",14,0,2,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqUpperBnds)}
		},
	kw_309[11] = {
		{"multi_objective_weights",6,0,4,0,0,0.,0.,10,N_rem(RealDL,primaryRespFnWeights)},
		{"nonlinear_equality_constraints",0x29,6,6,0,kw_307,0.,0.,0,N_rem(sizet,numNonlinearEqConstraints)},
		{"nonlinear_inequality_constraints",0x29,8,5,0,kw_308,0.,0.,0,N_rem(sizet,numNonlinearIneqConstraints)},
		{"num_nonlinear_equality_constraints",0x21,6,6,0,kw_307,0.,0.,-2,N_rem(sizet,numNonlinearEqConstraints)},
		{"num_nonlinear_inequality_constraints",0x21,8,5,0,kw_308,0.,0.,-2,N_rem(sizet,numNonlinearIneqConstraints)},
		{"objective_function_scale_types",0x807,0,2,0,0,0.,0.,2,N_rem(strL,primaryRespFnScaleTypes)},
		{"objective_function_scales",0x806,0,3,0,0,0.,0.,2,N_rem(RealDL,primaryRespFnScales)},
		{"primary_scale_types",0x80f,0,2,0,0,0.,0.,0,N_rem(strL,primaryRespFnScaleTypes)},
		{"primary_scales",0x80e,0,3,0,0,0.,0.,0,N_rem(RealDL,primaryRespFnScales)},
		{"sense",0x80f,0,1,0,0,0.,0.,0,N_rem(strL,primaryRespFnSense)},
		{"weights",14,0,4,0,0,0.,0.,0,N_rem(RealDL,primaryRespFnWeights)}
		},
	kw_310[7] = {
		{"absolute",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_absolute)},
		{"bounds",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_bounds)},
		{"central",8,0,3,0,0,0.,0.,0,N_rem(true,centralHess)},
		{"fd_hessian_step_size",6,0,1,0,0,0.,0.,1,N_rem(RealL,fdHessStepSize)},
		{"fd_step_size",14,0,1,0,0,0.,0.,0,N_rem(RealL,fdHessStepSize)},
		{"forward",8,0,3,0,0,0.,0.,0,N_rem(false,centralHess)},
		{"relative",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_relative)}
		},
	kw_311[1] = {
		{"damped",8,0,1,0,0,0.,0.,0,N_rem(lit,quasiHessianType_damped_bfgs)}
		},
	kw_312[2] = {
		{"bfgs",8,1,1,1,kw_311,0.,0.,0,N_rem(lit,quasiHessianType_bfgs)},
		{"sr1",8,0,1,1,0,0.,0.,0,N_rem(lit,quasiHessianType_sr1)}
		},
	kw_313[19] = {
		{"analytic_gradients",8,0,4,2,0,0.,0.,0,N_rem(lit,gradientType_analytic)},
		{"analytic_hessians",8,0,5,3,0,0.,0.,0,N_rem(lit,hessianType_analytic)},
		{"calibration_terms",0x29,15,3,1,kw_299,0.,0.,0,N_rem(sizet,numLeastSqTerms)},
		{"descriptors",15,0,2,0,0,0.,0.,0,N_rem(strL,responseLabels)},
		{"id_responses",11,0,1,0,0,0.,0.,0,N_rem(str,idResponses)},
		{"least_squares_terms",0x21,15,3,1,kw_299,0.,0.,-3,N_rem(sizet,numLeastSqTerms)},
		{"mixed_gradients",8,2,4,2,kw_302,0.,0.,0,N_rem(lit,gradientType_mixed)},
		{"mixed_hessians",8,8,5,3,kw_306,0.,0.,0,N_rem(lit,hessianType_mixed)},
		{"no_gradients",8,0,4,2,0,0.,0.,0,N_rem(lit,gradientType_none)},
		{"no_hessians",8,0,5,3,0,0.,0.,0,N_rem(lit,hessianType_none)},
		{"num_least_squares_terms",0x21,15,3,1,kw_299,0.,0.,-8,N_rem(sizet,numLeastSqTerms)},
		{"num_objective_functions",0x21,11,3,1,kw_309,0.,0.,4,N_rem(sizet,numObjectiveFunctions)},
		{"num_response_functions",0x21,0,3,1,0,0.,0.,6,N_rem(sizet,numResponseFunctions)},
		{"numerical_gradients",8,8,4,2,kw_301,0.,0.,0,N_rem(lit,gradientType_numerical)},
		{"numerical_hessians",8,7,5,3,kw_310,0.,0.,0,N_rem(lit,hessianType_numerical)},
		{"objective_functions",0x29,11,3,1,kw_309,0.,0.,0,N_rem(sizet,numObjectiveFunctions)},
		{"quasi_hessians",8,2,5,3,kw_312,0.,0.,0,N_rem(lit,hessianType_quasi)},
		{"response_descriptors",7,0,2,0,0,0.,0.,-14,N_rem(strL,responseLabels)},
		{"response_functions",0x29,0,3,1,0,0.,0.,0,N_rem(sizet,numResponseFunctions)}
		},
	kw_314[6] = {
		{"aleatory",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_ALEATORY_UNCERTAIN_VIEW)},
		{"all",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_ALL_VIEW)},
		{"design",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_DESIGN_VIEW)},
		{"epistemic",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_EPISTEMIC_UNCERTAIN_VIEW)},
		{"state",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_STATE_VIEW)},
		{"uncertain",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_UNCERTAIN_VIEW)}
		},
	kw_315[11] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,betaUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(RealLb,betaUncBetas)},
		{"buv_alphas",6,0,1,1,0,0.,0.,-2,N_vam(RealLb,betaUncAlphas)},
		{"buv_betas",6,0,2,2,0,0.,0.,-2,N_vam(RealLb,betaUncBetas)},
		{"buv_descriptors",7,0,6,0,0,0.,0.,3,N_vae(caulbl,CAUVar_beta)},
		{"buv_lower_bounds",6,0,3,3,0,0.,0.,4,N_vam(rvec,betaUncLowerBnds)},
		{"buv_upper_bounds",6,0,4,4,0,0.,0.,4,N_vam(rvec,betaUncUpperBnds)},
		{"descriptors",15,0,6,0,0,0.,0.,0,N_vae(caulbl,CAUVar_beta)},
		{"initial_point",14,0,5,0,0,0.,0.,0,N_vam(rvec,betaUncVars)},
		{"lower_bounds",14,0,3,3,0,0.,0.,0,N_vam(rvec,betaUncLowerBnds)},
		{"upper_bounds",14,0,4,4,0,0.,0.,0,N_vam(rvec,betaUncUpperBnds)}
		},
	kw_316[5] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(dauilbl,DAUIVar_binomial)},
		{"initial_point",13,0,3,0,0,0.,0.,0,N_vam(IntLb,binomialUncVars)},
		{"num_trials",13,0,2,2,0,0.,0.,0,N_vam(IntLb,binomialUncNumTrials)},
		{"prob_per_trial",6,0,1,1,0,0.,0.,1,N_vam(rvec,binomialUncProbPerTrial)},
		{"probability_per_trial",14,0,1,1,0,0.,0.,0,N_vam(rvec,binomialUncProbPerTrial)}
		},
	kw_317[12] = {
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
	kw_318[10] = {
		{"descriptors",15,0,6,0,0,0.,0.,0,N_vae(ceulbl,CEUVar_interval)},
		{"initial_point",14,0,5,0,0,0.,0.,0,N_vam(rvec,continuousIntervalUncVars)},
		{"interval_probabilities",14,0,2,0,0,0.,0.,0,N_vam(newrvec,Var_Info_CIp)},
		{"interval_probs",6,0,2,0,0,0.,0.,-1,N_vam(newrvec,Var_Info_CIp)},
		{"iuv_descriptors",7,0,6,0,0,0.,0.,-4,N_vae(ceulbl,CEUVar_interval)},
		{"iuv_interval_probs",6,0,2,0,0,0.,0.,-3,N_vam(newrvec,Var_Info_CIp)},
		{"iuv_num_intervals",5,0,1,0,0,0.,0.,2,N_vam(newiarray,Var_Info_nCI)},
		{"lower_bounds",14,0,3,1,0,0.,0.,0,N_vam(newrvec,Var_Info_CIlb)},
		{"num_intervals",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nCI)},
		{"upper_bounds",14,0,4,2,0,0.,0.,0,N_vam(newrvec,Var_Info_CIub)}
		},
	kw_319[8] = {
		{"csv_descriptors",7,0,4,0,0,0.,0.,4,N_vam(strL,continuousStateLabels)},
		{"csv_initial_state",6,0,1,0,0,0.,0.,4,N_vam(rvec,continuousStateVars)},
		{"csv_lower_bounds",6,0,2,0,0,0.,0.,4,N_vam(rvec,continuousStateLowerBnds)},
		{"csv_upper_bounds",6,0,3,0,0,0.,0.,4,N_vam(rvec,continuousStateUpperBnds)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,continuousStateLabels)},
		{"initial_state",14,0,1,0,0,0.,0.,0,N_vam(rvec,continuousStateVars)},
		{"lower_bounds",14,0,2,0,0,0.,0.,0,N_vam(rvec,continuousStateLowerBnds)},
		{"upper_bounds",14,0,3,0,0,0.,0.,0,N_vam(rvec,continuousStateUpperBnds)}
		},
	kw_320[8] = {
		{"ddv_descriptors",7,0,4,0,0,0.,0.,4,N_vam(strL,discreteDesignRangeLabels)},
		{"ddv_initial_point",5,0,1,0,0,0.,0.,4,N_vam(ivec,discreteDesignRangeVars)},
		{"ddv_lower_bounds",5,0,2,0,0,0.,0.,4,N_vam(ivec,discreteDesignRangeLowerBnds)},
		{"ddv_upper_bounds",5,0,3,0,0,0.,0.,4,N_vam(ivec,discreteDesignRangeUpperBnds)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteDesignRangeLabels)},
		{"initial_point",13,0,1,0,0,0.,0.,0,N_vam(ivec,discreteDesignRangeVars)},
		{"lower_bounds",13,0,2,0,0,0.,0.,0,N_vam(ivec,discreteDesignRangeLowerBnds)},
		{"upper_bounds",13,0,3,0,0,0.,0.,0,N_vam(ivec,discreteDesignRangeUpperBnds)}
		},
	kw_321[7] = {
		{"categorical",15,0,3,0,0,0.,0.,0,N_vam(categorical,discreteDesignSetIntCat)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vam(strL,discreteDesignSetIntLabels)},
		{"elements",13,0,2,1,0,0.,0.,0,N_vam(newivec,Var_Info_ddsi)},
		{"elements_per_variable",0x80d,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nddsi)},
		{"initial_point",13,0,4,0,0,0.,0.,0,N_vam(ivec,discreteDesignSetIntVars)},
		{"num_set_values",0x805,0,1,0,0,0.,0.,-2,N_vam(newiarray,Var_Info_nddsi)},
		{"set_values",5,0,2,1,0,0.,0.,-4,N_vam(newivec,Var_Info_ddsi)}
		},
	kw_322[7] = {
		{"categorical",15,0,3,0,0,0.,0.,0,N_vam(categorical,discreteDesignSetRealCat)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vam(strL,discreteDesignSetRealLabels)},
		{"elements",14,0,2,1,0,0.,0.,0,N_vam(newrvec,Var_Info_ddsr)},
		{"elements_per_variable",0x80d,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nddsr)},
		{"initial_point",14,0,4,0,0,0.,0.,0,N_vam(rvec,discreteDesignSetRealVars)},
		{"num_set_values",0x805,0,1,0,0,0.,0.,-2,N_vam(newiarray,Var_Info_nddsr)},
		{"set_values",6,0,2,1,0,0.,0.,-4,N_vam(newrvec,Var_Info_ddsr)}
		},
	kw_323[6] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteDesignSetStrLabels)},
		{"elements",15,0,2,1,0,0.,0.,0,N_vam(newsarray,Var_Info_ddss)},
		{"elements_per_variable",0x80d,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nddss)},
		{"initial_point",15,0,3,0,0,0.,0.,0,N_vam(strL,discreteDesignSetStrVars)},
		{"num_set_values",0x805,0,1,0,0,0.,0.,-2,N_vam(newiarray,Var_Info_nddss)},
		{"set_values",7,0,2,1,0,0.,0.,-4,N_vam(newsarray,Var_Info_ddss)}
		},
	kw_324[3] = {
		{"integer",0x19,7,1,0,kw_321,0.,0.,0,N_vam(pintz,numDiscreteDesSetIntVars)},
		{"real",0x19,7,3,0,kw_322,0.,0.,0,N_vam(pintz,numDiscreteDesSetRealVars)},
		{"string",0x19,6,2,0,kw_323,0.,0.,0,N_vam(pintz,numDiscreteDesSetStrVars)}
		},
	kw_325[9] = {
		{"descriptors",15,0,6,0,0,0.,0.,0,N_vae(deuilbl,DEUIVar_interval)},
		{"initial_point",13,0,5,0,0,0.,0.,0,N_vam(ivec,discreteIntervalUncVars)},
		{"interval_probabilities",14,0,2,0,0,0.,0.,0,N_vam(newrvec,Var_Info_DIp)},
		{"interval_probs",6,0,2,0,0,0.,0.,-1,N_vam(newrvec,Var_Info_DIp)},
		{"lower_bounds",13,0,3,1,0,0.,0.,0,N_vam(newivec,Var_Info_DIlb)},
		{"num_intervals",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nDI)},
		{"range_probabilities",6,0,2,0,0,0.,0.,-4,N_vam(newrvec,Var_Info_DIp)},
		{"range_probs",6,0,2,0,0,0.,0.,-5,N_vam(newrvec,Var_Info_DIp)},
		{"upper_bounds",13,0,4,2,0,0.,0.,0,N_vam(newivec,Var_Info_DIub)}
		},
	kw_326[8] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteStateRangeLabels)},
		{"dsv_descriptors",7,0,4,0,0,0.,0.,-1,N_vam(strL,discreteStateRangeLabels)},
		{"dsv_initial_state",5,0,1,0,0,0.,0.,3,N_vam(ivec,discreteStateRangeVars)},
		{"dsv_lower_bounds",5,0,2,0,0,0.,0.,3,N_vam(ivec,discreteStateRangeLowerBnds)},
		{"dsv_upper_bounds",5,0,3,0,0,0.,0.,3,N_vam(ivec,discreteStateRangeUpperBnds)},
		{"initial_state",13,0,1,0,0,0.,0.,0,N_vam(ivec,discreteStateRangeVars)},
		{"lower_bounds",13,0,2,0,0,0.,0.,0,N_vam(ivec,discreteStateRangeLowerBnds)},
		{"upper_bounds",13,0,3,0,0,0.,0.,0,N_vam(ivec,discreteStateRangeUpperBnds)}
		},
	kw_327[7] = {
		{"categorical",15,0,3,0,0,0.,0.,0,N_vam(categorical,discreteStateSetIntCat)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vam(strL,discreteStateSetIntLabels)},
		{"elements",13,0,2,1,0,0.,0.,0,N_vam(newivec,Var_Info_dssi)},
		{"elements_per_variable",0x80d,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_ndssi)},
		{"initial_state",13,0,4,0,0,0.,0.,0,N_vam(ivec,discreteStateSetIntVars)},
		{"num_set_values",0x805,0,1,0,0,0.,0.,-2,N_vam(newiarray,Var_Info_ndssi)},
		{"set_values",5,0,2,1,0,0.,0.,-4,N_vam(newivec,Var_Info_dssi)}
		},
	kw_328[7] = {
		{"categorical",15,0,3,0,0,0.,0.,0,N_vam(categorical,discreteStateSetRealCat)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vam(strL,discreteStateSetRealLabels)},
		{"elements",14,0,2,1,0,0.,0.,0,N_vam(newrvec,Var_Info_dssr)},
		{"elements_per_variable",0x80d,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_ndssr)},
		{"initial_state",14,0,4,0,0,0.,0.,0,N_vam(rvec,discreteStateSetRealVars)},
		{"num_set_values",0x805,0,1,0,0,0.,0.,-2,N_vam(newiarray,Var_Info_ndssr)},
		{"set_values",6,0,2,1,0,0.,0.,-4,N_vam(newrvec,Var_Info_dssr)}
		},
	kw_329[6] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteStateSetStrLabels)},
		{"elements",15,0,2,1,0,0.,0.,0,N_vam(newsarray,Var_Info_dsss)},
		{"elements_per_variable",0x80d,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_ndsss)},
		{"initial_state",15,0,3,0,0,0.,0.,0,N_vam(strL,discreteStateSetStrVars)},
		{"num_set_values",0x805,0,1,0,0,0.,0.,-2,N_vam(newiarray,Var_Info_ndsss)},
		{"set_values",7,0,2,1,0,0.,0.,-4,N_vam(newsarray,Var_Info_dsss)}
		},
	kw_330[3] = {
		{"integer",0x19,7,1,0,kw_327,0.,0.,0,N_vam(pintz,numDiscreteStateSetIntVars)},
		{"real",0x19,7,3,0,kw_328,0.,0.,0,N_vam(pintz,numDiscreteStateSetRealVars)},
		{"string",0x19,6,2,0,kw_329,0.,0.,0,N_vam(pintz,numDiscreteStateSetStrVars)}
		},
	kw_331[9] = {
		{"categorical",15,0,4,0,0,0.,0.,0,N_vam(categorical,discreteUncSetIntCat)},
		{"descriptors",15,0,6,0,0,0.,0.,0,N_vae(deuilbl,DEUIVar_set_int)},
		{"elements",13,0,2,1,0,0.,0.,0,N_vam(newivec,Var_Info_dusi)},
		{"elements_per_variable",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_ndusi)},
		{"initial_point",13,0,5,0,0,0.,0.,0,N_vam(ivec,discreteUncSetIntVars)},
		{"num_set_values",5,0,1,0,0,0.,0.,-2,N_vam(newiarray,Var_Info_ndusi)},
		{"set_probabilities",14,0,3,0,0,0.,0.,0,N_vam(newrvec,Var_Info_DSIp)},
		{"set_probs",6,0,3,0,0,0.,0.,-1,N_vam(newrvec,Var_Info_DSIp)},
		{"set_values",5,0,2,1,0,0.,0.,-6,N_vam(newivec,Var_Info_dusi)}
		},
	kw_332[9] = {
		{"categorical",15,0,4,0,0,0.,0.,0,N_vam(categorical,discreteUncSetRealCat)},
		{"descriptors",15,0,6,0,0,0.,0.,0,N_vae(deurlbl,DEURVar_set_real)},
		{"elements",14,0,2,1,0,0.,0.,0,N_vam(newrvec,Var_Info_dusr)},
		{"elements_per_variable",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_ndusr)},
		{"initial_point",14,0,5,0,0,0.,0.,0,N_vam(rvec,discreteUncSetRealVars)},
		{"num_set_values",5,0,1,0,0,0.,0.,-2,N_vam(newiarray,Var_Info_ndusr)},
		{"set_probabilities",14,0,3,0,0,0.,0.,0,N_vam(newrvec,Var_Info_DSRp)},
		{"set_probs",6,0,3,0,0,0.,0.,-1,N_vam(newrvec,Var_Info_DSRp)},
		{"set_values",6,0,2,1,0,0.,0.,-6,N_vam(newrvec,Var_Info_dusr)}
		},
	kw_333[8] = {
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(deuslbl,DEUSVar_set_str)},
		{"elements",15,0,2,1,0,0.,0.,0,N_vam(newsarray,Var_Info_duss)},
		{"elements_per_variable",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nduss)},
		{"initial_point",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteUncSetStrVars)},
		{"num_set_values",5,0,1,0,0,0.,0.,-2,N_vam(newiarray,Var_Info_nduss)},
		{"set_probabilities",14,0,3,0,0,0.,0.,0,N_vam(newrvec,Var_Info_DSSp)},
		{"set_probs",6,0,3,0,0,0.,0.,-1,N_vam(newrvec,Var_Info_DSSp)},
		{"set_values",7,0,2,1,0,0.,0.,-6,N_vam(newsarray,Var_Info_duss)}
		},
	kw_334[3] = {
		{"integer",0x19,9,1,0,kw_331,0.,0.,0,N_vam(pintz,numDiscreteUncSetIntVars)},
		{"real",0x19,9,3,0,kw_332,0.,0.,0,N_vam(pintz,numDiscreteUncSetRealVars)},
		{"string",0x19,8,2,0,kw_333,0.,0.,0,N_vam(pintz,numDiscreteUncSetStrVars)}
		},
	kw_335[5] = {
		{"betas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,exponentialUncBetas)},
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(caulbl,CAUVar_exponential)},
		{"euv_betas",6,0,1,1,0,0.,0.,-2,N_vam(RealLb,exponentialUncBetas)},
		{"euv_descriptors",7,0,3,0,0,0.,0.,-2,N_vae(caulbl,CAUVar_exponential)},
		{"initial_point",14,0,2,0,0,0.,0.,0,N_vam(RealLb,exponentialUncVars)}
		},
	kw_336[7] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,frechetUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(rvec,frechetUncBetas)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_frechet)},
		{"fuv_alphas",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,frechetUncAlphas)},
		{"fuv_betas",6,0,2,2,0,0.,0.,-3,N_vam(rvec,frechetUncBetas)},
		{"fuv_descriptors",7,0,4,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_frechet)},
		{"initial_point",14,0,3,0,0,0.,0.,0,N_vam(rvec,frechetUncVars)}
		},
	kw_337[7] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,gammaUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(RealLb,gammaUncBetas)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_gamma)},
		{"gauv_alphas",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,gammaUncAlphas)},
		{"gauv_betas",6,0,2,2,0,0.,0.,-3,N_vam(RealLb,gammaUncBetas)},
		{"gauv_descriptors",7,0,4,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_gamma)},
		{"initial_point",14,0,3,0,0,0.,0.,0,N_vam(RealLb,gammaUncVars)}
		},
	kw_338[4] = {
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(dauilbl,DAUIVar_geometric)},
		{"initial_point",13,0,2,0,0,0.,0.,0,N_vam(IntLb,geometricUncVars)},
		{"prob_per_trial",6,0,1,1,0,0.,0.,1,N_vam(rvec,geometricUncProbPerTrial)},
		{"probability_per_trial",14,0,1,1,0,0.,0.,0,N_vam(rvec,geometricUncProbPerTrial)}
		},
	kw_339[7] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,gumbelUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(rvec,gumbelUncBetas)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_gumbel)},
		{"guuv_alphas",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,gumbelUncAlphas)},
		{"guuv_betas",6,0,2,2,0,0.,0.,-3,N_vam(rvec,gumbelUncBetas)},
		{"guuv_descriptors",7,0,4,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_gumbel)},
		{"initial_point",14,0,3,0,0,0.,0.,0,N_vam(rvec,gumbelUncVars)}
		},
	kw_340[11] = {
		{"abscissas",14,0,2,1,0,0.,0.,0,N_vam(newrvec,Var_Info_hba)},
		{"counts",14,0,3,2,0,0.,0.,0,N_vam(newrvec,Var_Info_hbc)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(caulbl,CAUVar_histogram_bin)},
		{"huv_bin_abscissas",6,0,2,1,0,0.,0.,-3,N_vam(newrvec,Var_Info_hba)},
		{"huv_bin_counts",6,0,3,2,0,0.,0.,-3,N_vam(newrvec,Var_Info_hbc)},
		{"huv_bin_descriptors",7,0,5,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_histogram_bin)},
		{"huv_bin_ordinates",6,0,3,2,0,0.,0.,4,N_vam(newrvec,Var_Info_hbo)},
		{"huv_num_bin_pairs",5,0,1,0,0,0.,0.,2,N_vam(newiarray,Var_Info_nhbp)},
		{"initial_point",14,0,4,0,0,0.,0.,0,N_vam(rvec,histogramBinUncVars)},
		{"num_pairs",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nhbp)},
		{"ordinates",14,0,3,2,0,0.,0.,0,N_vam(newrvec,Var_Info_hbo)}
		},
	kw_341[6] = {
		{"abscissas",13,0,2,1,0,0.,0.,0,N_vam(newivec,Var_Info_hpia)},
		{"counts",14,0,3,2,0,0.,0.,0,N_vam(newrvec,Var_Info_hpic)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(dauilbl,DAUIVar_histogram_point_int)},
		{"initial_point",13,0,4,0,0,0.,0.,0,N_vam(ivec,histogramPointIntUncVars)},
		{"num_pairs",5,0,1,0,0,0.,0.,1,N_vam(newiarray,Var_Info_nhpip)},
		{"pairs_per_variable",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nhpip)}
		},
	kw_342[6] = {
		{"abscissas",14,0,2,1,0,0.,0.,0,N_vam(newrvec,Var_Info_hpra)},
		{"counts",14,0,3,2,0,0.,0.,0,N_vam(newrvec,Var_Info_hprc)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(daurlbl,DAURVar_histogram_point_real)},
		{"initial_point",14,0,4,0,0,0.,0.,0,N_vam(rvec,histogramPointRealUncVars)},
		{"num_pairs",5,0,1,0,0,0.,0.,1,N_vam(newiarray,Var_Info_nhprp)},
		{"pairs_per_variable",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nhprp)}
		},
	kw_343[6] = {
		{"abscissas",15,0,2,1,0,0.,0.,0,N_vam(newsarray,Var_Info_hpsa)},
		{"counts",14,0,3,2,0,0.,0.,0,N_vam(newrvec,Var_Info_hpsc)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(dauslbl,DAUSVar_histogram_point_str)},
		{"initial_point",15,0,4,0,0,0.,0.,0,N_vam(strL,histogramPointStrUncVars)},
		{"num_pairs",5,0,1,0,0,0.,0.,1,N_vam(newiarray,Var_Info_nhpsp)},
		{"pairs_per_variable",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nhpsp)}
		},
	kw_344[3] = {
		{"integer",0x19,6,1,0,kw_341,0.,0.,0,N_vam(pintz,numHistogramPtIntUncVars)},
		{"real",0x19,6,3,0,kw_342,0.,0.,0,N_vam(pintz,numHistogramPtRealUncVars)},
		{"string",0x19,6,2,0,kw_343,0.,0.,0,N_vam(pintz,numHistogramPtStrUncVars)}
		},
	kw_345[5] = {
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(dauilbl,DAUIVar_hypergeometric)},
		{"initial_point",13,0,4,0,0,0.,0.,0,N_vam(IntLb,hyperGeomUncVars)},
		{"num_drawn",13,0,3,3,0,0.,0.,0,N_vam(IntLb,hyperGeomUncNumDrawn)},
		{"selected_population",13,0,2,2,0,0.,0.,0,N_vam(IntLb,hyperGeomUncSelectedPop)},
		{"total_population",13,0,1,1,0,0.,0.,0,N_vam(IntLb,hyperGeomUncTotalPop)}
		},
	kw_346[2] = {
		{"lnuv_zetas",6,0,1,1,0,0.,0.,1,N_vam(RealLb,lognormalUncZetas)},
		{"zetas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,lognormalUncZetas)}
		},
	kw_347[4] = {
		{"error_factors",14,0,1,1,0,0.,0.,0,N_vam(RealLb,lognormalUncErrFacts)},
		{"lnuv_error_factors",6,0,1,1,0,0.,0.,-1,N_vam(RealLb,lognormalUncErrFacts)},
		{"lnuv_std_deviations",6,0,1,1,0,0.,0.,1,N_vam(RealLb,lognormalUncStdDevs)},
		{"std_deviations",14,0,1,1,0,0.,0.,0,N_vam(RealLb,lognormalUncStdDevs)}
		},
	kw_348[11] = {
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(caulbl,CAUVar_lognormal)},
		{"initial_point",14,0,4,0,0,0.,0.,0,N_vam(RealLb,lognormalUncVars)},
		{"lambdas",14,2,1,1,kw_346,0.,0.,0,N_vam(rvec,lognormalUncLambdas)},
		{"lnuv_descriptors",7,0,5,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_lognormal)},
		{"lnuv_lambdas",6,2,1,1,kw_346,0.,0.,-2,N_vam(rvec,lognormalUncLambdas)},
		{"lnuv_lower_bounds",6,0,2,0,0,0.,0.,3,N_vam(RealLb,lognormalUncLowerBnds)},
		{"lnuv_means",6,4,1,1,kw_347,0.,0.,3,N_vam(RealLb,lognormalUncMeans)},
		{"lnuv_upper_bounds",6,0,3,0,0,0.,0.,3,N_vam(RealUb,lognormalUncUpperBnds)},
		{"lower_bounds",14,0,2,0,0,0.,0.,0,N_vam(RealLb,lognormalUncLowerBnds)},
		{"means",14,4,1,1,kw_347,0.,0.,0,N_vam(RealLb,lognormalUncMeans)},
		{"upper_bounds",14,0,3,0,0,0.,0.,0,N_vam(RealUb,lognormalUncUpperBnds)}
		},
	kw_349[7] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_loguniform)},
		{"initial_point",14,0,3,0,0,0.,0.,0,N_vam(RealLb,loguniformUncVars)},
		{"lower_bounds",14,0,1,1,0,0.,0.,0,N_vam(RealLb,loguniformUncLowerBnds)},
		{"luuv_descriptors",7,0,4,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_loguniform)},
		{"luuv_lower_bounds",6,0,1,1,0,0.,0.,-2,N_vam(RealLb,loguniformUncLowerBnds)},
		{"luuv_upper_bounds",6,0,2,2,0,0.,0.,1,N_vam(RealUb,loguniformUncUpperBnds)},
		{"upper_bounds",14,0,2,2,0,0.,0.,0,N_vam(RealUb,loguniformUncUpperBnds)}
		},
	kw_350[5] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(dauilbl,DAUIVar_negative_binomial)},
		{"initial_point",13,0,3,0,0,0.,0.,0,N_vam(IntLb,negBinomialUncVars)},
		{"num_trials",13,0,2,2,0,0.,0.,0,N_vam(IntLb,negBinomialUncNumTrials)},
		{"prob_per_trial",6,0,1,1,0,0.,0.,1,N_vam(rvec,negBinomialUncProbPerTrial)},
		{"probability_per_trial",14,0,1,1,0,0.,0.,0,N_vam(rvec,negBinomialUncProbPerTrial)}
		},
	kw_351[11] = {
		{"descriptors",15,0,6,0,0,0.,0.,0,N_vae(caulbl,CAUVar_normal)},
		{"initial_point",14,0,5,0,0,0.,0.,0,N_vam(rvec,normalUncVars)},
		{"lower_bounds",14,0,3,0,0,0.,0.,0,N_vam(rvec,normalUncLowerBnds)},
		{"means",14,0,1,1,0,0.,0.,0,N_vam(rvec,normalUncMeans)},
		{"nuv_descriptors",7,0,6,0,0,0.,0.,-4,N_vae(caulbl,CAUVar_normal)},
		{"nuv_lower_bounds",6,0,3,0,0,0.,0.,-3,N_vam(rvec,normalUncLowerBnds)},
		{"nuv_means",6,0,1,1,0,0.,0.,-3,N_vam(rvec,normalUncMeans)},
		{"nuv_std_deviations",6,0,2,2,0,0.,0.,2,N_vam(RealLb,normalUncStdDevs)},
		{"nuv_upper_bounds",6,0,4,0,0,0.,0.,2,N_vam(rvec,normalUncUpperBnds)},
		{"std_deviations",14,0,2,2,0,0.,0.,0,N_vam(RealLb,normalUncStdDevs)},
		{"upper_bounds",14,0,4,0,0,0.,0.,0,N_vam(rvec,normalUncUpperBnds)}
		},
	kw_352[3] = {
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(dauilbl,DAUIVar_poisson)},
		{"initial_point",13,0,2,0,0,0.,0.,0,N_vam(IntLb,poissonUncVars)},
		{"lambdas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,poissonUncLambdas)}
		},
	kw_353[9] = {
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(caulbl,CAUVar_triangular)},
		{"initial_point",14,0,4,0,0,0.,0.,0,N_vam(rvec,triangularUncVars)},
		{"lower_bounds",14,0,2,2,0,0.,0.,0,N_vam(RealLb,triangularUncLowerBnds)},
		{"modes",14,0,1,1,0,0.,0.,0,N_vam(rvec,triangularUncModes)},
		{"tuv_descriptors",7,0,5,0,0,0.,0.,-4,N_vae(caulbl,CAUVar_triangular)},
		{"tuv_lower_bounds",6,0,2,2,0,0.,0.,-3,N_vam(RealLb,triangularUncLowerBnds)},
		{"tuv_modes",6,0,1,1,0,0.,0.,-3,N_vam(rvec,triangularUncModes)},
		{"tuv_upper_bounds",6,0,3,3,0,0.,0.,1,N_vam(RealUb,triangularUncUpperBnds)},
		{"upper_bounds",14,0,3,3,0,0.,0.,0,N_vam(RealUb,triangularUncUpperBnds)}
		},
	kw_354[7] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_uniform)},
		{"initial_point",14,0,3,0,0,0.,0.,0,N_vam(rvec,uniformUncVars)},
		{"lower_bounds",14,0,1,1,0,0.,0.,0,N_vam(RealLb,uniformUncLowerBnds)},
		{"upper_bounds",14,0,2,2,0,0.,0.,0,N_vam(RealUb,uniformUncUpperBnds)},
		{"uuv_descriptors",7,0,4,0,0,0.,0.,-4,N_vae(caulbl,CAUVar_uniform)},
		{"uuv_lower_bounds",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,uniformUncLowerBnds)},
		{"uuv_upper_bounds",6,0,2,2,0,0.,0.,-3,N_vam(RealUb,uniformUncUpperBnds)}
		},
	kw_355[7] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,weibullUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(RealLb,weibullUncBetas)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_weibull)},
		{"initial_point",14,0,3,0,0,0.,0.,0,N_vam(RealLb,weibullUncVars)},
		{"wuv_alphas",6,0,1,1,0,0.,0.,-4,N_vam(RealLb,weibullUncAlphas)},
		{"wuv_betas",6,0,2,2,0,0.,0.,-4,N_vam(RealLb,weibullUncBetas)},
		{"wuv_descriptors",7,0,4,0,0,0.,0.,-4,N_vae(caulbl,CAUVar_weibull)}
		},
	kw_356[34] = {
		{"active",8,6,2,0,kw_314},
		{"beta_uncertain",0x19,11,13,0,kw_315,0.,0.,0,N_vam(pintz,numBetaUncVars)},
		{"binomial_uncertain",0x19,5,20,0,kw_316,0.,0.,0,N_vam(pintz,numBinomialUncVars)},
		{"continuous_design",0x19,12,4,0,kw_317,0.,0.,0,N_vam(pintz,numContinuousDesVars)},
		{"continuous_interval_uncertain",0x19,10,26,0,kw_318,0.,0.,0,N_vam(pintz,numContinuousIntervalUncVars)},
		{"continuous_state",0x19,8,29,0,kw_319,0.,0.,0,N_vam(pintz,numContinuousStateVars)},
		{"discrete_design_range",0x19,8,5,0,kw_320,0.,0.,0,N_vam(pintz,numDiscreteDesRangeVars)},
		{"discrete_design_set",8,3,6,0,kw_324},
		{"discrete_interval_uncertain",0x19,9,27,0,kw_325,0.,0.,0,N_vam(pintz,numDiscreteIntervalUncVars)},
		{"discrete_state_range",0x19,8,30,0,kw_326,0.,0.,0,N_vam(pintz,numDiscreteStateRangeVars)},
		{"discrete_state_set",8,3,31,0,kw_330},
		{"discrete_uncertain_range",0x11,9,27,0,kw_325,0.,0.,-3,N_vam(pintz,numDiscreteIntervalUncVars)},
		{"discrete_uncertain_set",8,3,28,0,kw_334},
		{"exponential_uncertain",0x19,5,12,0,kw_335,0.,0.,0,N_vam(pintz,numExponentialUncVars)},
		{"frechet_uncertain",0x19,7,16,0,kw_336,0.,0.,0,N_vam(pintz,numFrechetUncVars)},
		{"gamma_uncertain",0x19,7,14,0,kw_337,0.,0.,0,N_vam(pintz,numGammaUncVars)},
		{"geometric_uncertain",0x19,4,22,0,kw_338,0.,0.,0,N_vam(pintz,numGeometricUncVars)},
		{"gumbel_uncertain",0x19,7,15,0,kw_339,0.,0.,0,N_vam(pintz,numGumbelUncVars)},
		{"histogram_bin_uncertain",0x19,11,18,0,kw_340,0.,0.,0,N_vam(pintz,numHistogramBinUncVars)},
		{"histogram_point_uncertain",8,3,24,0,kw_344},
		{"hypergeometric_uncertain",0x19,5,23,0,kw_345,0.,0.,0,N_vam(pintz,numHyperGeomUncVars)},
		{"id_variables",11,0,1,0,0,0.,0.,0,N_vam(str,idVariables)},
		{"interval_uncertain",0x11,10,26,0,kw_318,0.,0.,-18,N_vam(pintz,numContinuousIntervalUncVars)},
		{"lognormal_uncertain",0x19,11,8,0,kw_348,0.,0.,0,N_vam(pintz,numLognormalUncVars)},
		{"loguniform_uncertain",0x19,7,10,0,kw_349,0.,0.,0,N_vam(pintz,numLoguniformUncVars)},
		{"mixed",8,0,3,0,0,0.,0.,0,N_vam(type,varsDomain_MIXED_DOMAIN)},
		{"negative_binomial_uncertain",0x19,5,21,0,kw_350,0.,0.,0,N_vam(pintz,numNegBinomialUncVars)},
		{"normal_uncertain",0x19,11,7,0,kw_351,0.,0.,0,N_vam(pintz,numNormalUncVars)},
		{"poisson_uncertain",0x19,3,19,0,kw_352,0.,0.,0,N_vam(pintz,numPoissonUncVars)},
		{"relaxed",8,0,3,0,0,0.,0.,0,N_vam(type,varsDomain_RELAXED_DOMAIN)},
		{"triangular_uncertain",0x19,9,11,0,kw_353,0.,0.,0,N_vam(pintz,numTriangularUncVars)},
		{"uncertain_correlation_matrix",14,0,25,0,0,0.,0.,0,N_vam(newrvec,Var_Info_ucm)},
		{"uniform_uncertain",0x19,7,9,0,kw_354,0.,0.,0,N_vam(pintz,numUniformUncVars)},
		{"weibull_uncertain",0x19,7,17,0,kw_355,0.,0.,0,N_vam(pintz,numWeibullUncVars)}
		},
	kw_357[6] = {
		{"environment",0x108,15,1,1,kw_7,0.,0.,0,NIDRProblemDescDB::env_start},
		{"interface",0x308,9,5,5,kw_22,0.,0.,0,N_ifm3(start,0,stop)},
		{"method",0x308,90,2,2,kw_264,0.,0.,0,N_mdm3(start,0,stop)},
		{"model",8,7,3,3,kw_295,0.,0.,0,N_mom3(start,0,stop)},
		{"responses",0x308,19,6,6,kw_313,0.,0.,0,N_rem3(start,0,stop)},
		{"variables",0x308,34,4,4,kw_356,0.,0.,0,N_vam3(start,0,stop)}
		};

} // namespace Dakota

#ifdef __cplusplus
extern "C" {
#endif
KeyWord Dakota_Keyword_Top = {"KeywordTop",0,6,0,0,Dakota::kw_357};
#ifdef __cplusplus
}
#endif

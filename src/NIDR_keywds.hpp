
namespace Dakota {

/** 1546 distinct keywords (plus 226 aliases) **/

static KeyWord
	kw_1[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_stm(augment_utype,postRunInputFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_stm(augment_utype,postRunInputFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_stm(augment_utype,postRunInputFormat_TABULAR_IFACE_ID)}
		},
	kw_2[3] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_stm(utype,postRunInputFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_1,0.,0.,0,N_stm(utype,postRunInputFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_stm(utype,postRunInputFormat_TABULAR_NONE)}
		},
	kw_3[2] = {
		{"input",11,3,1,0,kw_2,0.,0.,0,N_stm(str,postRunInput)},
		{"output",11,0,2,0,0,0.,0.,0,N_stm(str,postRunOutput)}
		},
	kw_4[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_stm(augment_utype,preRunOutputFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_stm(augment_utype,preRunOutputFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_stm(augment_utype,preRunOutputFormat_TABULAR_IFACE_ID)}
		},
	kw_5[3] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_stm(utype,preRunOutputFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_4,0.,0.,0,N_stm(utype,preRunOutputFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_stm(utype,preRunOutputFormat_TABULAR_NONE)}
		},
	kw_6[2] = {
		{"input",11,0,1,0,0,0.,0.,0,N_stm(str,preRunInput)},
		{"output",11,3,2,0,kw_5,0.,0.,0,N_stm(str,preRunOutput)}
		},
	kw_7[1] = {
		{"stop_restart",0x29,0,1,0,0,0.,0.,0,N_stm(int,stopRestart)}
		},
	kw_8[1] = {
		{"results_output_file",11,0,1,0,0,0.,0.,0,N_stm(str,resultsOutputFile)}
		},
	kw_9[2] = {
		{"input",11,0,1,0,0,0.,0.,0,N_stm(str,runInput)},
		{"output",11,0,2,0,0,0.,0.,0,N_stm(str,runOutput)}
		},
	kw_10[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_stm(augment_utype,tabularFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_stm(augment_utype,tabularFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_stm(augment_utype,tabularFormat_TABULAR_IFACE_ID)}
		},
	kw_11[5] = {
		{"annotated",8,0,2,0,0,0.,0.,0,N_stm(utype,tabularFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,2,0,kw_10,0.,0.,0,N_stm(utype,tabularFormat_TABULAR_NONE)},
		{"freeform",8,0,2,0,0,0.,0.,0,N_stm(utype,tabularFormat_TABULAR_NONE)},
		{"tabular_data_file",11,0,1,0,0,0.,0.,0,N_stm(str,tabularDataFile)},
		{"tabular_graphics_file",3,0,1,0,0,0.,0.,-1,N_stm(str,tabularDataFile)}
		},
	kw_12[15] = {
		{"check",8,0,1,0,0,0.,0.,0,N_stm(true,checkFlag)},
		{"error_file",11,0,3,0,0,0.,0.,0,N_stm(str,errorFile)},
		{"graphics",8,0,9,0,0,0.,0.,0,N_stm(true,graphicsFlag)},
		{"method_pointer",3,0,13,0,0,0.,0.,10,N_stm(str,topMethodPointer)},
		{"output_file",11,0,2,0,0,0.,0.,0,N_stm(str,outputFile)},
		{"output_precision",0x29,0,11,0,0,0.,0.,0,N_stm(int,outputPrecision)},
		{"post_run",8,2,8,0,kw_3,0.,0.,0,N_stm(true,postRunFlag)},
		{"pre_run",8,2,6,0,kw_6,0.,0.,0,N_stm(true,preRunFlag)},
		{"read_restart",11,1,4,0,kw_7,0.,0.,0,N_stm(str,readRestart)},
		{"results_output",8,1,12,0,kw_8,0.,0.,0,N_stm(true,resultsOutputFlag)},
		{"run",8,2,7,0,kw_9,0.,0.,0,N_stm(true,runFlag)},
		{"tabular_data",8,5,10,0,kw_11,0.,0.,0,N_stm(true,tabularDataFlag)},
		{"tabular_graphics_data",0,5,10,0,kw_11,0.,0.,-1,N_stm(true,tabularDataFlag)},
		{"top_method_pointer",11,0,13,0,0,0.,0.,0,N_stm(str,topMethodPointer)},
		{"write_restart",11,0,5,0,0,0.,0.,0,N_stm(str,writeRestart)}
		},
	kw_13[1] = {
		{"cache_tolerance",10,0,1,0,0,0.,0.,0,N_ifm(Real,nearbyEvalCacheTol)}
		},
	kw_14[4] = {
		{"active_set_vector",8,0,1,0,0,0.,0.,0,N_ifm(false,activeSetVectorFlag)},
		{"evaluation_cache",8,0,2,0,0,0.,0.,0,N_ifm(false,evalCacheFlag)},
		{"restart_file",8,0,4,0,0,0.,0.,0,N_ifm(false,restartFileFlag)},
		{"strict_cache_equality",8,1,3,0,kw_13,0.,0.,0,N_ifm(true,nearbyEvalCacheFlag)}
		},
	kw_15[1] = {
		{"processors_per_analysis",0x19,0,1,0,0,0.,0.,0,N_ifm(pint,procsPerAnalysis)}
		},
	kw_16[4] = {
		{"abort",8,0,1,1,0,0.,0.,0,N_ifm(lit,failAction_abort)},
		{"continuation",8,0,1,1,0,0.,0.,0,N_ifm(lit,failAction_continuation)},
		{"recover",14,0,1,1,0,0.,0.,0,N_ifm(Rlit,TYPE_DATA_failAction_recover)},
		{"retry",9,0,1,1,0,0.,0.,0,N_ifm(ilit,TYPE_DATA_failAction_retry)}
		},
	kw_17[1] = {
		{"numpy",8,0,1,0,0,0.,0.,0,N_ifm(true,numpyFlag)}
		},
	kw_18[8] = {
		{"copy_files",15,0,5,0,0,0.,0.,0,N_ifm(strL,copyFiles)},
		{"dir_save",0,0,3,0,0,0.,0.,2,N_ifm(true,dirSave)},
		{"dir_tag",0,0,2,0,0,0.,0.,2,N_ifm(true,dirTag)},
		{"directory_save",8,0,3,0,0,0.,0.,0,N_ifm(true,dirSave)},
		{"directory_tag",8,0,2,0,0,0.,0.,0,N_ifm(true,dirTag)},
		{"link_files",15,0,4,0,0,0.,0.,0,N_ifm(strL,linkFiles)},
		{"named",11,0,1,0,0,0.,0.,0,N_ifm(str,workDir)},
		{"replace",8,0,6,0,0,0.,0.,0,N_ifm(true,templateReplace)}
		},
	kw_19[9] = {
		{"allow_existing_results",8,0,3,0,0,0.,0.,0,N_ifm(true,allowExistingResultsFlag)},
		{"aprepro",8,0,5,0,0,0.,0.,0,N_ifm(true,apreproFlag)},
		{"dprepro",0,0,5,0,0,0.,0.,-1,N_ifm(true,apreproFlag)},
		{"file_save",8,0,7,0,0,0.,0.,0,N_ifm(true,fileSaveFlag)},
		{"file_tag",8,0,6,0,0,0.,0.,0,N_ifm(true,fileTagFlag)},
		{"parameters_file",11,0,1,0,0,0.,0.,0,N_ifm(str,parametersFile)},
		{"results_file",11,0,2,0,0,0.,0.,0,N_ifm(str,resultsFile)},
		{"verbatim",8,0,4,0,0,0.,0.,0,N_ifm(true,verbatimFlag)},
		{"work_directory",8,8,8,0,kw_18,0.,0.,0,N_ifm(true,useWorkdir)}
		},
	kw_20[12] = {
		{"analysis_components",15,0,1,0,0,0.,0.,0,N_ifm(str2D,analysisComponents)},
		{"deactivate",8,4,6,0,kw_14},
		{"direct",8,1,4,1,kw_15,0.,0.,0,N_ifm(type,interfaceType_TEST_INTERFACE)},
		{"failure_capture",8,4,5,0,kw_16},
		{"fork",8,9,4,1,kw_19,0.,0.,0,N_ifm(type,interfaceType_FORK_INTERFACE)},
		{"grid",8,0,4,1,0,0.,0.,0,N_ifm(type,interfaceType_GRID_INTERFACE)},
		{"input_filter",11,0,2,0,0,0.,0.,0,N_ifm(str,inputFilter)},
		{"matlab",8,0,4,1,0,0.,0.,0,N_ifm(type,interfaceType_MATLAB_INTERFACE)},
		{"output_filter",11,0,3,0,0,0.,0.,0,N_ifm(str,outputFilter)},
		{"python",8,1,4,1,kw_17,0.,0.,0,N_ifm(type,interfaceType_PYTHON_INTERFACE)},
		{"scilab",8,0,4,1,0,0.,0.,0,N_ifm(type,interfaceType_SCILAB_INTERFACE)},
		{"system",8,9,4,1,kw_19,0.,0.,0,N_ifm(type,interfaceType_SYSTEM_INTERFACE)}
		},
	kw_21[2] = {
		{"master",8,0,1,1,0,0.,0.,0,N_ifm(type,analysisScheduling_MASTER_SCHEDULING)},
		{"peer",8,0,1,1,0,0.,0.,0,N_ifm(type,analysisScheduling_PEER_SCHEDULING)}
		},
	kw_22[2] = {
		{"dynamic",8,0,1,1,0,0.,0.,0,N_ifm(type,asynchLocalEvalScheduling_DYNAMIC_SCHEDULING)},
		{"static",8,0,1,1,0,0.,0.,0,N_ifm(type,asynchLocalEvalScheduling_STATIC_SCHEDULING)}
		},
	kw_23[3] = {
		{"analysis_concurrency",0x19,0,3,0,0,0.,0.,0,N_ifm(pint,asynchLocalAnalysisConcurrency)},
		{"evaluation_concurrency",0x19,0,1,0,0,0.,0.,0,N_ifm(pint,asynchLocalEvalConcurrency)},
		{"local_evaluation_scheduling",8,2,2,0,kw_22}
		},
	kw_24[2] = {
		{"dynamic",8,0,1,1,0,0.,0.,0,N_ifm(type,evalScheduling_PEER_DYNAMIC_SCHEDULING)},
		{"static",8,0,1,1,0,0.,0.,0,N_ifm(type,evalScheduling_PEER_STATIC_SCHEDULING)}
		},
	kw_25[2] = {
		{"master",8,0,1,1,0,0.,0.,0,N_ifm(type,evalScheduling_MASTER_SCHEDULING)},
		{"peer",8,2,1,1,kw_24}
		},
	kw_26[9] = {
		{"algebraic_mappings",11,0,2,0,0,0.,0.,0,N_ifm(str,algebraicMappings)},
		{"analysis_drivers",15,12,3,0,kw_20,0.,0.,0,N_ifm(strL,analysisDrivers)},
		{"analysis_scheduling",8,2,9,0,kw_21},
		{"analysis_servers",0x19,0,8,0,0,0.,0.,0,N_ifm(pint,analysisServers)},
		{"asynchronous",8,3,4,0,kw_23,0.,0.,0,N_ifm(type,interfaceSynchronization_ASYNCHRONOUS_INTERFACE)},
		{"evaluation_scheduling",8,2,6,0,kw_25},
		{"evaluation_servers",0x19,0,5,0,0,0.,0.,0,N_ifm(pint,evalServers)},
		{"id_interface",11,0,1,0,0,0.,0.,0,N_ifm(str,idInterface)},
		{"processors_per_evaluation",0x19,0,7,0,0,0.,0.,0,N_ifm(pint,procsPerEval)}
		},
	kw_27[1] = {
		{"model_pointer",11,0,1,0,0,0.,0.,0,N_mdm(str,modelPointer)}
		},
	kw_28[2] = {
		{"samples",9,0,1,0,0,0.,0.,0,N_mdm(int,numSamples)},
		{"seed",0x19,0,2,0,0,0.,0.,0,N_mdm(pint,randomSeed)}
		},
	kw_29[2] = {
		{"complementary",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_COMPLEMENTARY)},
		{"cumulative",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_CUMULATIVE)}
		},
	kw_30[1] = {
		{"num_gen_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,genReliabilityLevels)}
		},
	kw_31[1] = {
		{"num_probability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,probabilityLevels)}
		},
	kw_32[2] = {
		{"mt19937",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_mt19937)},
		{"rnum2",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_rnum2)}
		},
	kw_33[4] = {
		{"distribution",8,2,1,0,kw_29},
		{"gen_reliability_levels",14,1,3,0,kw_30,0.,0.,0,N_mdm(resplevs,genReliabilityLevels)},
		{"probability_levels",14,1,2,0,kw_31,0.,0.,0,N_mdm(resplevs01,probabilityLevels)},
		{"rng",8,2,4,0,kw_32}
		},
	kw_34[4] = {
		{"constant_liar",8,0,1,1,0,0.,0.,0,N_mdm(lit,batchSelectionType_constant_liar)},
		{"distance_penalty",8,0,1,1,0,0.,0.,0,N_mdm(lit,batchSelectionType_distance_penalty)},
		{"naive",8,0,1,1,0,0.,0.,0,N_mdm(lit,batchSelectionType_naive)},
		{"topology",8,0,1,1,0,0.,0.,0,N_mdm(lit,batchSelectionType_topology)}
		},
	kw_35[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_IFACE_ID)}
		},
	kw_36[3] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_35,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_NONE)}
		},
	kw_37[3] = {
		{"distance",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessMetricType_distance)},
		{"gradient",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessMetricType_gradient)},
		{"predicted_variance",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessMetricType_predicted_variance)}
		},
	kw_38[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_39[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_38,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)}
		},
	kw_40[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_41[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_40}
		},
	kw_42[2] = {
		{"compute",8,3,2,0,kw_41},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_43[12] = {
		{0,0,1,0,0,kw_27},
		{0,0,2,0,0,kw_28},
		{0,0,4,0,0,kw_33},
		{"batch_selection",8,4,3,0,kw_34},
		{"batch_size",9,0,4,0,0,0.,0.,0,N_mdm(int,batchSize)},
		{"emulator_samples",9,0,1,0,0,0.,0.,0,N_mdm(int,emulatorSamples)},
		{"export_points_file",11,3,6,0,kw_36,0.,0.,0,N_mdm(str,approxExportFile)},
		{"fitness_metric",8,3,2,0,kw_37},
		{"import_build_points_file",11,4,5,0,kw_39,0.,0.,0,N_mdm(str,approxImportFile)},
		{"import_points_file",3,4,5,0,kw_39,0.,0.,-1,N_mdm(str,approxImportFile)},
		{"misc_options",15,0,8,0,0,0.,0.,0,N_mdm(strL,miscOptions)},
		{"response_levels",14,2,7,0,kw_42,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_44[9] = {
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
	kw_45[7] = {
		{"merit1",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit1)},
		{"merit1_smooth",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit1_smooth)},
		{"merit2",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit2)},
		{"merit2_smooth",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit2_smooth)},
		{"merit2_squared",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit2_squared)},
		{"merit_max",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit_max)},
		{"merit_max_smooth",8,0,1,1,0,0.,0.,0,N_mdm(lit,meritFunction_merit_max_smooth)}
		},
	kw_46[2] = {
		{"blocking",8,0,1,1,0,0.,0.,0,N_mdm(lit,evalSynchronize_blocking)},
		{"nonblocking",8,0,1,1,0,0.,0.,0,N_mdm(lit,evalSynchronize_nonblocking)}
		},
	kw_47[11] = {
		{0,0,1,0,0,kw_27},
		{0,0,9,0,0,kw_44},
		{"constraint_penalty",10,0,7,0,0,0.,0.,0,N_mdm(Real,constrPenalty)},
		{"contraction_factor",10,0,2,0,0,0.,0.,0,N_mdm(Real,contractStepLength)},
		{"initial_delta",10,0,1,0,0,0.,0.,0,N_mdm(Real,initStepLength)},
		{"merit_function",8,7,6,0,kw_45},
		{"smoothing_factor",10,0,8,0,0,0.,0.,0,N_mdm(Real,smoothFactor)},
		{"solution_accuracy",2,0,4,0,0,0.,0.,1,N_mdm(Real,solnTarget)},
		{"solution_target",10,0,4,0,0,0.,0.,0,N_mdm(Real,solnTarget)},
		{"synchronization",8,2,5,0,kw_46},
		{"threshold_delta",10,0,3,0,0,0.,0.,0,N_mdm(Real,threshStepLength)}
		},
	kw_48[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_49[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_48,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)}
		},
	kw_50[6] = {
		{"dakota",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_GP_EMULATOR)},
		{"emulator_samples",9,0,2,0,0,0.,0.,0,N_mdm(int,emulatorSamples)},
		{"import_build_points_file",11,4,4,0,kw_49,0.,0.,0,N_mdm(str,approxImportFile)},
		{"import_points_file",3,4,4,0,kw_49,0.,0.,-1,N_mdm(str,approxImportFile)},
		{"posterior_adaptive",8,0,3,0,0,0.,0.,0,N_mdm(true,adaptPosteriorRefine)},
		{"surfpack",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING_EMULATOR)}
		},
	kw_51[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_52[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_51,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)}
		},
	kw_53[3] = {
		{"import_build_points_file",11,4,2,0,kw_52,0.,0.,0,N_mdm(str,approxImportFile)},
		{"import_points_file",3,4,2,0,kw_52,0.,0.,-1,N_mdm(str,approxImportFile)},
		{"posterior_adaptive",8,0,1,0,0,0.,0.,0,N_mdm(true,adaptPosteriorRefine)}
		},
	kw_54[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_55[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_54,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)}
		},
	kw_56[4] = {
		{"collocation_ratio",10,0,1,1,0,0.,0.,0,N_mdm(Realp,collocationRatio)},
		{"import_build_points_file",11,4,3,0,kw_55,0.,0.,0,N_mdm(str,approxImportFile)},
		{"import_points_file",3,4,3,0,kw_55,0.,0.,-1,N_mdm(str,approxImportFile)},
		{"posterior_adaptive",8,0,2,0,0,0.,0.,0,N_mdm(true,adaptPosteriorRefine)}
		},
	kw_57[3] = {
		{"collocation_points",13,3,1,1,kw_53,0.,0.,0,N_mdm(szarray,collocationPoints)},
		{"expansion_order",13,4,1,1,kw_56,0.,0.,0,N_mdm(usharray,expansionOrder)},
		{"sparse_grid_level",13,0,1,1,0,0.,0.,0,N_mdm(usharray,sparseGridLevel)}
		},
	kw_58[1] = {
		{"sparse_grid_level",13,0,1,1,0,0.,0.,0,N_mdm(usharray,sparseGridLevel)}
		},
	kw_59[5] = {
		{"gaussian_process",8,6,1,1,kw_50},
		{"kriging",0,6,1,1,kw_50,0.,0.,-1},
		{"pce",8,3,1,1,kw_57,0.,0.,0,N_mdm(type,emulatorType_PCE_EMULATOR)},
		{"sc",8,1,1,1,kw_58,0.,0.,0,N_mdm(type,emulatorType_SC_EMULATOR)},
		{"use_derivatives",8,0,2,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)}
		},
	kw_60[6] = {
		{"chains",0x29,0,1,0,0,3.,0.,0,N_mdm(int,numChains)},
		{"crossover_chain_pairs",0x29,0,3,0,0,0.,0.,0,N_mdm(int,crossoverChainPairs)},
		{"emulator",8,5,6,0,kw_59},
		{"gr_threshold",0x1a,0,4,0,0,0.,0.,0,N_mdm(Real,grThreshold)},
		{"jump_step",0x29,0,5,0,0,0.,0.,0,N_mdm(int,jumpStep)},
		{"num_cr",0x29,0,2,0,0,1.,0.,0,N_mdm(int,numCR)}
		},
	kw_61[2] = {
		{"nip",8,0,1,1,0,0.,0.,0,N_mdm(utype,preSolveMethod_SUBMETHOD_NIP)},
		{"sqp",8,0,1,1,0,0.,0.,0,N_mdm(utype,preSolveMethod_SUBMETHOD_SQP)}
		},
	kw_62[1] = {
		{"proposal_updates",9,0,1,0,0,0.,0.,0,N_mdm(int,proposalCovUpdates)}
		},
	kw_63[2] = {
		{"diagonal",8,0,1,1,0,0.,0.,0,N_mdm(lit,proposalCovInputType_diagonal)},
		{"matrix",8,0,1,1,0,0.,0.,0,N_mdm(lit,proposalCovInputType_matrix)}
		},
	kw_64[2] = {
		{"diagonal",8,0,1,1,0,0.,0.,0,N_mdm(lit,proposalCovInputType_diagonal)},
		{"matrix",8,0,1,1,0,0.,0.,0,N_mdm(lit,proposalCovInputType_matrix)}
		},
	kw_65[4] = {
		{"derivatives",8,1,1,1,kw_62,0.,0.,0,N_mdm(lit,proposalCovType_derivatives)},
		{"filename",11,2,1,1,kw_63,0.,0.,0,N_mdm(str,proposalCovFile)},
		{"prior",8,0,1,1,0,0.,0.,0,N_mdm(lit,proposalCovType_prior)},
		{"values",14,2,1,1,kw_64,0.,0.,0,N_mdm(RealDL,proposalCovData)}
		},
	kw_66[2] = {
		{"mt19937",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_mt19937)},
		{"rnum2",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_rnum2)}
		},
	kw_67[8] = {
		{"adaptive_metropolis",8,0,1,0,0,0.,0.,0,N_mdm(lit,mcmcType_adaptive_metropolis)},
		{"delayed_rejection",8,0,1,0,0,0.,0.,0,N_mdm(lit,mcmcType_delayed_rejection)},
		{"dram",8,0,1,0,0,0.,0.,0,N_mdm(lit,mcmcType_dram)},
		{"metropolis_hastings",8,0,1,0,0,0.,0.,0,N_mdm(lit,mcmcType_metropolis_hastings)},
		{"multilevel",8,0,1,0,0,0.,0.,0,N_mdm(lit,mcmcType_multilevel)},
		{"pre_solve",8,2,3,0,kw_61},
		{"proposal_covariance",8,4,4,0,kw_65,0.,0.,0,N_mdm(lit,proposalCovType_user)},
		{"rng",8,2,2,0,kw_66}
		},
	kw_68[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_69[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_68,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)}
		},
	kw_70[4] = {
		{0,0,8,0,0,kw_67},
		{"emulator_samples",9,0,1,1,0,0.,0.,0,N_mdm(int,emulatorSamples)},
		{"import_build_points_file",11,4,2,0,kw_69,0.,0.,0,N_mdm(str,approxImportFile)},
		{"import_points_file",3,4,2,0,kw_69,0.,0.,-1,N_mdm(str,approxImportFile)}
		},
	kw_71[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_72[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_71,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)}
		},
	kw_73[6] = {
		{"dakota",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_GP_EMULATOR)},
		{"emulator_samples",9,0,2,0,0,0.,0.,0,N_mdm(int,emulatorSamples)},
		{"import_build_points_file",11,4,4,0,kw_72,0.,0.,0,N_mdm(str,approxImportFile)},
		{"import_points_file",3,4,4,0,kw_72,0.,0.,-1,N_mdm(str,approxImportFile)},
		{"posterior_adaptive",8,0,3,0,0,0.,0.,0,N_mdm(true,adaptPosteriorRefine)},
		{"surfpack",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING_EMULATOR)}
		},
	kw_74[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_75[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_74,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)}
		},
	kw_76[3] = {
		{"import_build_points_file",11,4,2,0,kw_75,0.,0.,0,N_mdm(str,approxImportFile)},
		{"import_points_file",3,4,2,0,kw_75,0.,0.,-1,N_mdm(str,approxImportFile)},
		{"posterior_adaptive",8,0,1,0,0,0.,0.,0,N_mdm(true,adaptPosteriorRefine)}
		},
	kw_77[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_78[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_77,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)}
		},
	kw_79[4] = {
		{"collocation_ratio",10,0,1,1,0,0.,0.,0,N_mdm(Realp,collocationRatio)},
		{"import_build_points_file",11,4,3,0,kw_78,0.,0.,0,N_mdm(str,approxImportFile)},
		{"import_points_file",3,4,3,0,kw_78,0.,0.,-1,N_mdm(str,approxImportFile)},
		{"posterior_adaptive",8,0,2,0,0,0.,0.,0,N_mdm(true,adaptPosteriorRefine)}
		},
	kw_80[3] = {
		{"collocation_points",13,3,1,1,kw_76,0.,0.,0,N_mdm(szarray,collocationPoints)},
		{"expansion_order",13,4,1,1,kw_79,0.,0.,0,N_mdm(usharray,expansionOrder)},
		{"sparse_grid_level",13,0,1,1,0,0.,0.,0,N_mdm(usharray,sparseGridLevel)}
		},
	kw_81[1] = {
		{"sparse_grid_level",13,0,1,1,0,0.,0.,0,N_mdm(usharray,sparseGridLevel)}
		},
	kw_82[5] = {
		{"gaussian_process",8,6,1,1,kw_73},
		{"kriging",0,6,1,1,kw_73,0.,0.,-1},
		{"pce",8,3,1,1,kw_80,0.,0.,0,N_mdm(type,emulatorType_PCE_EMULATOR)},
		{"sc",8,1,1,1,kw_81,0.,0.,0,N_mdm(type,emulatorType_SC_EMULATOR)},
		{"use_derivatives",8,0,2,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)}
		},
	kw_83[3] = {
		{0,0,8,0,0,kw_67},
		{"emulator",8,5,1,0,kw_82},
		{"logit_transform",8,0,2,0,0,0.,0.,0,N_mdm(true,logitTransform)}
		},
	kw_84[2] = {
		{"diagonal",8,0,1,1,0,0.,0.,0,N_mdm(lit,dataDistCovInputType_diagonal)},
		{"matrix",8,0,1,1,0,0.,0.,0,N_mdm(lit,dataDistCovInputType_matrix)}
		},
	kw_85[2] = {
		{"covariance",14,2,2,2,kw_84,0.,0.,0,N_mdm(RealDL,dataDistCovariance)},
		{"means",14,0,1,1,0,0.,0.,0,N_mdm(RealDL,dataDistMeans)}
		},
	kw_86[2] = {
		{"gaussian",8,2,1,1,kw_85},
		{"obs_data_filename",11,0,1,1,0,0.,0.,0,N_mdm(str,dataDistFile)}
		},
	kw_87[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_88[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_87,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)}
		},
	kw_89[6] = {
		{"dakota",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_GP_EMULATOR)},
		{"emulator_samples",9,0,2,0,0,0.,0.,0,N_mdm(int,emulatorSamples)},
		{"import_build_points_file",11,4,4,0,kw_88,0.,0.,0,N_mdm(str,approxImportFile)},
		{"import_points_file",3,4,4,0,kw_88,0.,0.,-1,N_mdm(str,approxImportFile)},
		{"posterior_adaptive",8,0,3,0,0,0.,0.,0,N_mdm(true,adaptPosteriorRefine)},
		{"surfpack",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING_EMULATOR)}
		},
	kw_90[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_91[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_90,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)}
		},
	kw_92[3] = {
		{"import_build_points_file",11,4,2,0,kw_91,0.,0.,0,N_mdm(str,approxImportFile)},
		{"import_points_file",3,4,2,0,kw_91,0.,0.,-1,N_mdm(str,approxImportFile)},
		{"posterior_adaptive",8,0,1,0,0,0.,0.,0,N_mdm(true,adaptPosteriorRefine)}
		},
	kw_93[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_94[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_93,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)}
		},
	kw_95[4] = {
		{"collocation_ratio",10,0,1,1,0,0.,0.,0,N_mdm(Realp,collocationRatio)},
		{"import_build_points_file",11,4,3,0,kw_94,0.,0.,0,N_mdm(str,approxImportFile)},
		{"import_points_file",3,4,3,0,kw_94,0.,0.,-1,N_mdm(str,approxImportFile)},
		{"posterior_adaptive",8,0,2,0,0,0.,0.,0,N_mdm(true,adaptPosteriorRefine)}
		},
	kw_96[3] = {
		{"collocation_points",13,3,1,1,kw_92,0.,0.,0,N_mdm(szarray,collocationPoints)},
		{"expansion_order",13,4,1,1,kw_95,0.,0.,0,N_mdm(usharray,expansionOrder)},
		{"sparse_grid_level",13,0,1,1,0,0.,0.,0,N_mdm(usharray,sparseGridLevel)}
		},
	kw_97[1] = {
		{"sparse_grid_level",13,0,1,1,0,0.,0.,0,N_mdm(usharray,sparseGridLevel)}
		},
	kw_98[5] = {
		{"gaussian_process",8,6,1,1,kw_89},
		{"kriging",0,6,1,1,kw_89,0.,0.,-1},
		{"pce",8,3,1,1,kw_96,0.,0.,0,N_mdm(type,emulatorType_PCE_EMULATOR)},
		{"sc",8,1,1,1,kw_97,0.,0.,0,N_mdm(type,emulatorType_SC_EMULATOR)},
		{"use_derivatives",8,0,2,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)}
		},
	kw_99[2] = {
		{"data_distribution",8,2,2,1,kw_86},
		{"emulator",8,5,1,0,kw_98}
		},
	kw_100[9] = {
		{0,0,1,0,0,kw_27},
		{0,0,2,0,0,kw_28},
		{"calibrate_sigma",8,0,4,0,0,0.,0.,0,N_mdm(true,calibrateSigmaFlag)},
		{"dream",8,6,1,1,kw_60,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_DREAM)},
		{"gpmsa",8,3,1,1,kw_70,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_GPMSA)},
		{"likelihood_scale",10,0,3,0,0,0.,0.,0,N_mdm(Real,likelihoodScale)},
		{"queso",8,2,1,1,kw_83,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_QUESO)},
		{"standardized_space",8,0,2,0,0,0.,0.,0,N_mdm(true,standardizedSpace)},
		{"wasabi",8,2,1,1,kw_99,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_WASABI)}
		},
	kw_101[5] = {
		{"misc_options",15,0,4,0,0,0.,0.,0,N_mdm(strL,miscOptions)},
		{"seed",0x19,0,2,0,0,0.,0.,0,N_mdm(pint,randomSeed)},
		{"show_misc_options",8,0,3,0,0,0.,0.,0,N_mdm(true,showMiscOptions)},
		{"solution_accuracy",2,0,1,0,0,0.,0.,1,N_mdm(Real,solnTarget)},
		{"solution_target",10,0,1,0,0,0.,0.,0,N_mdm(Real,solnTarget)}
		},
	kw_102[3] = {
		{0,0,1,0,0,kw_27},
		{0,0,5,0,0,kw_101},
		{""}
		},
	kw_103[4] = {
		{0,0,1,0,0,kw_27},
		{"deltas_per_variable",5,0,2,2,0,0.,0.,2,N_mdm(ivec,stepsPerVariable)},
		{"step_vector",14,0,1,1,0,0.,0.,0,N_mdm(RealDL,stepVector)},
		{"steps_per_variable",13,0,2,2,0,0.,0.,0,N_mdm(ivec,stepsPerVariable)}
		},
	kw_104[3] = {
		{0,0,1,0,0,kw_27},
		{0,0,5,0,0,kw_101},
		{"beta_solver_name",11,0,1,1,0,0.,0.,0,N_mdm(str,betaSolverName)}
		},
	kw_105[2] = {
		{"initial_delta",10,0,1,0,0,0.,0.,0,N_mdm(Real,initDelta)},
		{"threshold_delta",10,0,2,0,0,0.,0.,0,N_mdm(Real,threshDelta)}
		},
	kw_106[4] = {
		{0,0,1,0,0,kw_27},
		{0,0,5,0,0,kw_101},
		{0,0,2,0,0,kw_105},
		{""}
		},
	kw_107[2] = {
		{"all_dimensions",8,0,1,1,0,0.,0.,0,N_mdm(lit,boxDivision_all_dimensions)},
		{"major_dimension",8,0,1,1,0,0.,0.,0,N_mdm(lit,boxDivision_major_dimension)}
		},
	kw_108[8] = {
		{0,0,1,0,0,kw_27},
		{0,0,5,0,0,kw_101},
		{"constraint_penalty",10,0,6,0,0,0.,0.,0,N_mdm(Real,constraintPenalty)},
		{"division",8,2,1,0,kw_107},
		{"global_balance_parameter",10,0,2,0,0,0.,0.,0,N_mdm(Real,globalBalanceParam)},
		{"local_balance_parameter",10,0,3,0,0,0.,0.,0,N_mdm(Real,localBalanceParam)},
		{"max_boxsize_limit",10,0,4,0,0,0.,0.,0,N_mdm(Real,maxBoxSize)},
		{"min_boxsize_limit",10,0,5,0,0,0.,0.,0,N_mdm(Real,minBoxSize)}
		},
	kw_109[3] = {
		{"blend",8,0,1,1,0,0.,0.,0,N_mdm(lit,crossoverType_blend)},
		{"two_point",8,0,1,1,0,0.,0.,0,N_mdm(lit,crossoverType_two_point)},
		{"uniform",8,0,1,1,0,0.,0.,0,N_mdm(lit,crossoverType_uniform)}
		},
	kw_110[2] = {
		{"linear_rank",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_linear_rank)},
		{"merit_function",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_proportional)}
		},
	kw_111[3] = {
		{"flat_file",11,0,1,1,0,0.,0.,0,N_mdm(slit2,TYPE_DATA_initializationType_flat_file)},
		{"simple_random",8,0,1,1,0,0.,0.,0,N_mdm(lit,initializationType_random)},
		{"unique_random",8,0,1,1,0,0.,0.,0,N_mdm(lit,initializationType_unique_random)}
		},
	kw_112[2] = {
		{"mutation_range",9,0,2,0,0,0.,0.,0,N_mdm(int,mutationRange)},
		{"mutation_scale",10,0,1,0,0,0.,0.,0,N_mdm(Real,mutationScale)}
		},
	kw_113[5] = {
		{"non_adaptive",8,0,2,0,0,0.,0.,0,N_mdm(false,mutationAdaptive)},
		{"offset_cauchy",8,2,1,1,kw_112,0.,0.,0,N_mdm(lit,mutationType_offset_cauchy)},
		{"offset_normal",8,2,1,1,kw_112,0.,0.,0,N_mdm(lit,mutationType_offset_normal)},
		{"offset_uniform",8,2,1,1,kw_112,0.,0.,0,N_mdm(lit,mutationType_offset_uniform)},
		{"replace_uniform",8,0,1,1,0,0.,0.,0,N_mdm(lit,mutationType_replace_uniform)}
		},
	kw_114[4] = {
		{"chc",9,0,1,1,0,0.,0.,0,N_mdm(ilit2,TYPE_DATA_replacementType_chc)},
		{"elitist",9,0,1,1,0,0.,0.,0,N_mdm(ilit2,TYPE_DATA_replacementType_elitist)},
		{"new_solutions_generated",9,0,2,0,0,0.,0.,0,N_mdm(int,newSolnsGenerated)},
		{"random",9,0,1,1,0,0.,0.,0,N_mdm(ilit2,TYPE_DATA_replacementType_random)}
		},
	kw_115[11] = {
		{0,0,1,0,0,kw_27},
		{0,0,5,0,0,kw_101},
		{"constraint_penalty",10,0,9,0,0,0.,0.,0,N_mdm(Real,constraintPenalty)},
		{"crossover_rate",10,0,5,0,0,0.,0.,0,N_mdm(Real,crossoverRate)},
		{"crossover_type",8,3,6,0,kw_109},
		{"fitness_type",8,2,3,0,kw_110},
		{"initialization_type",8,3,2,0,kw_111},
		{"mutation_rate",10,0,7,0,0,0.,0.,0,N_mdm(Real,mutationRate)},
		{"mutation_type",8,5,8,0,kw_113},
		{"population_size",0x19,0,1,0,0,0.,0.,0,N_mdm(pint,populationSize)},
		{"replacement_type",8,4,4,0,kw_114}
		},
	kw_116[2] = {
		{"constraint_penalty",10,0,2,0,0,0.,0.,0,N_mdm(Real,constraintPenalty)},
		{"contraction_factor",10,0,1,0,0,0.,0.,0,N_mdm(Real,contractFactor)}
		},
	kw_117[3] = {
		{"adaptive_pattern",8,0,1,1,0,0.,0.,0,N_mdm(lit,exploratoryMoves_adaptive)},
		{"basic_pattern",8,0,1,1,0,0.,0.,0,N_mdm(lit,exploratoryMoves_simple)},
		{"multi_step",8,0,1,1,0,0.,0.,0,N_mdm(lit,exploratoryMoves_multi_step)}
		},
	kw_118[2] = {
		{"coordinate",8,0,1,1,0,0.,0.,0,N_mdm(lit,patternBasis_coordinate)},
		{"simplex",8,0,1,1,0,0.,0.,0,N_mdm(lit,patternBasis_simplex)}
		},
	kw_119[2] = {
		{"blocking",8,0,1,1,0,0.,0.,0,N_mdm(lit,evalSynchronize_blocking)},
		{"nonblocking",8,0,1,1,0,0.,0.,0,N_mdm(lit,evalSynchronize_nonblocking)}
		},
	kw_120[12] = {
		{0,0,1,0,0,kw_27},
		{0,0,5,0,0,kw_101},
		{0,0,2,0,0,kw_105},
		{0,0,2,0,0,kw_116},
		{"constant_penalty",8,0,1,0,0,0.,0.,0,N_mdm(true,constantPenalty)},
		{"expand_after_success",9,0,3,0,0,0.,0.,0,N_mdm(int,expandAfterSuccess)},
		{"exploratory_moves",8,3,7,0,kw_117},
		{"no_expansion",8,0,2,0,0,0.,0.,0,N_mdm(false,expansionFlag)},
		{"pattern_basis",8,2,4,0,kw_118},
		{"stochastic",8,0,5,0,0,0.,0.,0,N_mdm(true,randomizeOrderFlag)},
		{"synchronization",8,2,8,0,kw_119},
		{"total_pattern_size",9,0,6,0,0,0.,0.,0,N_mdm(int,totalPatternSize)}
		},
	kw_121[8] = {
		{0,0,1,0,0,kw_27},
		{0,0,5,0,0,kw_101},
		{0,0,2,0,0,kw_105},
		{0,0,2,0,0,kw_116},
		{"constant_penalty",8,0,4,0,0,0.,0.,0,N_mdm(true,constantPenalty)},
		{"contract_after_failure",9,0,1,0,0,0.,0.,0,N_mdm(int,contractAfterFail)},
		{"expand_after_success",9,0,3,0,0,0.,0.,0,N_mdm(int,expandAfterSuccess)},
		{"no_expansion",8,0,2,0,0,0.,0.,0,N_mdm(false,expansionFlag)}
		},
	kw_122[4] = {
		{0,0,1,0,0,kw_27},
		{0,0,9,0,0,kw_44},
		{"frcg",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_CONMIN_FRCG)},
		{"mfd",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_CONMIN_MFD)}
		},
	kw_123[3] = {
		{0,0,1,0,0,kw_27},
		{0,0,9,0,0,kw_44},
		{""}
		},
	kw_124[1] = {
		{"drop_tolerance",10,0,1,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)}
		},
	kw_125[14] = {
		{0,0,1,0,0,kw_27},
		{0,0,2,0,0,kw_28},
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
		{"variance_based_decomp",8,1,4,0,kw_124,0.,0.,0,N_mdm(true,vbdFlag)}
		},
	kw_126[7] = {
		{0,0,1,0,0,kw_27},
		{0,0,9,0,0,kw_44},
		{"bfgs",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_DOT_BFGS)},
		{"frcg",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_DOT_FRCG)},
		{"mmfd",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_DOT_MMFD)},
		{"slp",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_DOT_SLP)},
		{"sqp",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_DOT_SQP)}
		},
	kw_127[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_IFACE_ID)}
		},
	kw_128[3] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_127,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_NONE)}
		},
	kw_129[2] = {
		{"dakota",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_GP_EMULATOR)},
		{"surfpack",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING_EMULATOR)}
		},
	kw_130[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_131[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_130,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)}
		},
	kw_132[8] = {
		{0,0,1,0,0,kw_27},
		{"export_points_file",11,3,4,0,kw_128,0.,0.,0,N_mdm(str,approxExportFile)},
		{"gaussian_process",8,2,1,0,kw_129},
		{"import_build_points_file",11,4,3,0,kw_131,0.,0.,0,N_mdm(str,approxImportFile)},
		{"import_points_file",3,4,3,0,kw_131,0.,0.,-1,N_mdm(str,approxImportFile)},
		{"kriging",0,2,1,0,kw_129,0.,0.,-3},
		{"seed",0x19,0,5,0,0,0.,0.,0,N_mdm(pint,randomSeed)},
		{"use_derivatives",8,0,2,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)}
		},
	kw_133[5] = {
		{0,0,1,0,0,kw_27},
		{0,0,2,0,0,kw_28},
		{0,0,4,0,0,kw_33},
		{"batch_size",9,0,2,0,0,0.,0.,0,N_mdm(int,batchSize)},
		{"emulator_samples",9,0,1,0,0,0.,0.,0,N_mdm(int,emulatorSamples)}
		},
	kw_134[3] = {
		{"grid",8,0,1,1,0,0.,0.,0,N_mdm(lit,trialType_grid)},
		{"halton",8,0,1,1,0,0.,0.,0,N_mdm(lit,trialType_halton)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(lit,trialType_random)}
		},
	kw_135[1] = {
		{"drop_tolerance",10,0,1,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)}
		},
	kw_136[8] = {
		{0,0,1,0,0,kw_27},
		{0,0,2,0,0,kw_28},
		{"fixed_seed",8,0,4,0,0,0.,0.,0,N_mdm(true,fixedSeedFlag)},
		{"latinize",8,0,1,0,0,0.,0.,0,N_mdm(true,latinizeFlag)},
		{"num_trials",9,0,6,0,0,0.,0.,0,N_mdm(int,numTrials)},
		{"quality_metrics",8,0,2,0,0,0.,0.,0,N_mdm(true,volQualityFlag)},
		{"trial_type",8,3,5,0,kw_134},
		{"variance_based_decomp",8,1,3,0,kw_135,0.,0.,0,N_mdm(true,vbdFlag)}
		},
	kw_137[1] = {
		{"drop_tolerance",10,0,1,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)}
		},
	kw_138[11] = {
		{0,0,1,0,0,kw_27},
		{"fixed_sequence",8,0,6,0,0,0.,0.,0,N_mdm(true,fixedSequenceFlag)},
		{"halton",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_FSU_HALTON)},
		{"hammersley",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_FSU_HAMMERSLEY)},
		{"latinize",8,0,2,0,0,0.,0.,0,N_mdm(true,latinizeFlag)},
		{"prime_base",13,0,9,0,0,0.,0.,0,N_mdm(ivec,primeBase)},
		{"quality_metrics",8,0,3,0,0,0.,0.,0,N_mdm(true,volQualityFlag)},
		{"samples",9,0,5,0,0,0.,0.,0,N_mdm(int,numSamples)},
		{"sequence_leap",13,0,8,0,0,0.,0.,0,N_mdm(ivec,sequenceLeap)},
		{"sequence_start",13,0,7,0,0,0.,0.,0,N_mdm(ivec,sequenceStart)},
		{"variance_based_decomp",8,1,4,0,kw_137,0.,0.,0,N_mdm(true,vbdFlag)}
		},
	kw_139[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_IFACE_ID)}
		},
	kw_140[3] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_139,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_NONE)}
		},
	kw_141[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_142[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_141,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)}
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
		{0,0,1,0,0,kw_27},
		{0,0,2,0,0,kw_28},
		{0,0,4,0,0,kw_33},
		{"emulator_samples",9,0,1,0,0,0.,0.,0,N_mdm(int,emulatorSamples)},
		{"export_points_file",11,3,3,0,kw_140,0.,0.,0,N_mdm(str,approxExportFile)},
		{"import_build_points_file",11,4,2,0,kw_142,0.,0.,0,N_mdm(str,approxImportFile)},
		{"import_points_file",3,4,2,0,kw_142,0.,0.,-1,N_mdm(str,approxImportFile)},
		{"response_levels",14,2,4,0,kw_145,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_147[2] = {
		{0,0,1,0,0,kw_27},
		{"seed",0x19,0,1,0,0,0.,0.,0,N_mdm(pint,randomSeed)}
		},
	kw_148[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_149[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_148}
		},
	kw_150[2] = {
		{"compute",8,3,2,0,kw_149},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_151[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_IFACE_ID)}
		},
	kw_152[3] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_151,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_NONE)}
		},
	kw_153[2] = {
		{"dakota",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_GP_EMULATOR)},
		{"surfpack",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING_EMULATOR)}
		},
	kw_154[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_155[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_154,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)}
		},
	kw_156[6] = {
		{"export_points_file",11,3,4,0,kw_152,0.,0.,0,N_mdm(str,approxExportFile)},
		{"gaussian_process",8,2,1,0,kw_153},
		{"import_build_points_file",11,4,3,0,kw_155,0.,0.,0,N_mdm(str,approxImportFile)},
		{"import_points_file",3,4,3,0,kw_155,0.,0.,-1,N_mdm(str,approxImportFile)},
		{"kriging",0,2,1,0,kw_153,0.,0.,-3},
		{"use_derivatives",8,0,2,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)}
		},
	kw_157[8] = {
		{0,0,1,0,0,kw_27},
		{0,0,2,0,0,kw_28},
		{0,0,4,0,0,kw_33},
		{"ea",8,0,1,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_EA)},
		{"ego",8,6,1,0,kw_156,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_EGO)},
		{"lhs",8,0,1,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_LHS)},
		{"response_levels",14,2,2,0,kw_150,0.,0.,0,N_mdm(resplevs,responseLevels)},
		{"sbo",8,6,1,0,kw_156,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_SBO)}
		},
	kw_158[2] = {
		{"mt19937",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_mt19937)},
		{"rnum2",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_rnum2)}
		},
	kw_159[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_IFACE_ID)}
		},
	kw_160[3] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_159,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_NONE)}
		},
	kw_161[2] = {
		{"dakota",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_GP_EMULATOR)},
		{"surfpack",8,0,1,1,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING_EMULATOR)}
		},
	kw_162[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_163[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_162,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)}
		},
	kw_164[6] = {
		{"export_points_file",11,3,4,0,kw_160,0.,0.,0,N_mdm(str,approxExportFile)},
		{"gaussian_process",8,2,1,0,kw_161},
		{"import_build_points_file",11,4,3,0,kw_163,0.,0.,0,N_mdm(str,approxImportFile)},
		{"import_points_file",3,4,3,0,kw_163,0.,0.,-1,N_mdm(str,approxImportFile)},
		{"kriging",0,2,1,0,kw_161,0.,0.,-3},
		{"use_derivatives",8,0,2,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)}
		},
	kw_165[7] = {
		{0,0,1,0,0,kw_27},
		{0,0,2,0,0,kw_28},
		{"ea",8,0,1,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_EA)},
		{"ego",8,6,1,0,kw_164,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_EGO)},
		{"lhs",8,0,1,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_LHS)},
		{"rng",8,2,2,0,kw_158},
		{"sbo",8,6,1,0,kw_164,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_SBO)}
		},
	kw_166[2] = {
		{"complementary",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_COMPLEMENTARY)},
		{"cumulative",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_CUMULATIVE)}
		},
	kw_167[1] = {
		{"num_gen_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,genReliabilityLevels)}
		},
	kw_168[1] = {
		{"num_probability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,probabilityLevels)}
		},
	kw_169[3] = {
		{"distribution",8,2,1,0,kw_166},
		{"gen_reliability_levels",14,1,3,0,kw_167,0.,0.,0,N_mdm(resplevs,genReliabilityLevels)},
		{"probability_levels",14,1,2,0,kw_168,0.,0.,0,N_mdm(resplevs01,probabilityLevels)}
		},
	kw_170[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_IFACE_ID)}
		},
	kw_171[3] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_170,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_NONE)}
		},
	kw_172[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_173[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_172,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)}
		},
	kw_174[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_175[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_174}
		},
	kw_176[2] = {
		{"compute",8,3,2,0,kw_175},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_177[2] = {
		{"mt19937",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_mt19937)},
		{"rnum2",8,0,1,1,0,0.,0.,0,N_mdm(lit,rngName_rnum2)}
		},
	kw_178[15] = {
		{0,0,1,0,0,kw_27},
		{0,0,3,0,0,kw_169},
		{"dakota",8,0,2,0,0,0.,0.,0,N_mdm(type,emulatorType_GP_EMULATOR)},
		{"export_points_file",11,3,4,0,kw_171,0.,0.,0,N_mdm(str,approxExportFile)},
		{"import_build_points_file",11,4,3,0,kw_173,0.,0.,0,N_mdm(str,approxImportFile)},
		{"import_points_file",3,4,3,0,kw_173,0.,0.,-1,N_mdm(str,approxImportFile)},
		{"response_levels",14,2,8,0,kw_176,0.,0.,0,N_mdm(resplevs,responseLevels)},
		{"rng",8,2,7,0,kw_177},
		{"seed",0x19,0,6,0,0,0.,0.,0,N_mdm(pint,randomSeed)},
		{"surfpack",8,0,2,0,0,0.,0.,0,N_mdm(type,emulatorType_KRIGING_EMULATOR)},
		{"u_gaussian_process",8,0,1,1,0,0.,0.,0,N_mdm(utype,reliabilitySearchType_EGRA_U)},
		{"u_kriging",0,0,1,1,0,0.,0.,-1,N_mdm(utype,reliabilitySearchType_EGRA_U)},
		{"use_derivatives",8,0,5,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)},
		{"x_gaussian_process",8,0,1,1,0,0.,0.,0,N_mdm(utype,reliabilitySearchType_EGRA_X)},
		{"x_kriging",0,0,1,1,0,0.,0.,-1,N_mdm(utype,reliabilitySearchType_EGRA_X)}
		},
	kw_179[2] = {
		{"master",8,0,1,1,0,0.,0.,0,N_mdm(type,iteratorScheduling_MASTER_SCHEDULING)},
		{"peer",8,0,1,1,0,0.,0.,0,N_mdm(type,iteratorScheduling_PEER_SCHEDULING)}
		},
	kw_180[3] = {
		{"iterator_scheduling",8,2,2,0,kw_179},
		{"iterator_servers",0x19,0,1,0,0,0.,0.,0,N_mdm(pint,iteratorServers)},
		{"processors_per_iterator",0x19,0,3,0,0,0.,0.,0,N_mdm(pint,procsPerIterator)}
		},
	kw_181[1] = {
		{"model_pointer_list",11,0,1,0,0,0.,0.,0,N_mdm(strL,hybridModelPointers)}
		},
	kw_182[2] = {
		{"method_name_list",15,1,1,1,kw_181,0.,0.,0,N_mdm(strL,hybridMethodNames)},
		{"method_pointer_list",15,0,1,1,0,0.,0.,0,N_mdm(strL,hybridMethodPointers)}
		},
	kw_183[1] = {
		{"global_model_pointer",11,0,1,0,0,0.,0.,0,N_mdm(str,hybridGlobalModelPointer)}
		},
	kw_184[1] = {
		{"local_model_pointer",11,0,1,0,0,0.,0.,0,N_mdm(str,hybridLocalModelPointer)}
		},
	kw_185[5] = {
		{"global_method_name",11,1,1,1,kw_183,0.,0.,0,N_mdm(str,hybridGlobalMethodName)},
		{"global_method_pointer",11,0,1,1,0,0.,0.,0,N_mdm(str,hybridGlobalMethodPointer)},
		{"local_method_name",11,1,2,2,kw_184,0.,0.,0,N_mdm(str,hybridLocalMethodName)},
		{"local_method_pointer",11,0,2,2,0,0.,0.,0,N_mdm(str,hybridLocalMethodPointer)},
		{"local_search_probability",10,0,3,0,0,0.,0.,0,N_mdm(Real,hybridLSProb)}
		},
	kw_186[1] = {
		{"model_pointer_list",11,0,1,0,0,0.,0.,0,N_mdm(strL,hybridModelPointers)}
		},
	kw_187[2] = {
		{"method_name_list",15,1,1,1,kw_186,0.,0.,0,N_mdm(strL,hybridMethodNames)},
		{"method_pointer_list",15,0,1,1,0,0.,0.,0,N_mdm(strL,hybridMethodPointers)}
		},
	kw_188[6] = {
		{0,0,3,0,0,kw_180},
		{"collaborative",8,2,1,1,kw_182,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_COLLABORATIVE)},
		{"coupled",0,5,1,1,kw_185,0.,0.,1,N_mdm(utype,subMethod_SUBMETHOD_EMBEDDED)},
		{"embedded",8,5,1,1,kw_185,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_EMBEDDED)},
		{"sequential",8,2,1,1,kw_187,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_SEQUENTIAL)},
		{"uncoupled",0,2,1,1,kw_187,0.,0.,-1,N_mdm(utype,subMethod_SUBMETHOD_SEQUENTIAL)}
		},
	kw_189[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_190[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_189}
		},
	kw_191[2] = {
		{"compute",8,3,2,0,kw_190},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_192[8] = {
		{0,0,1,0,0,kw_27},
		{0,0,2,0,0,kw_28},
		{0,0,4,0,0,kw_33},
		{"adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_AIS)},
		{"import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_IS)},
		{"mm_adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_MMAIS)},
		{"refinement_samples",9,0,2,0,0,0.,0.,0,N_mdm(int,refineSamples)},
		{"response_levels",14,2,3,0,kw_191,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_193[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,pstudyFileFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,pstudyFileFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,pstudyFileFormat_TABULAR_IFACE_ID)}
		},
	kw_194[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,pstudyFileActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,pstudyFileFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_193,0.,0.,0,N_mdm(utype,pstudyFileFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,pstudyFileFormat_TABULAR_NONE)}
		},
	kw_195[3] = {
		{0,0,1,0,0,kw_27},
		{"import_points_file",11,4,1,1,kw_194,0.,0.,0,N_mdm(str,pstudyFilename)},
		{"list_of_points",14,0,1,1,0,0.,0.,0,N_mdm(RealDL,listOfPoints)}
		},
	kw_196[2] = {
		{"complementary",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_COMPLEMENTARY)},
		{"cumulative",8,0,1,1,0,0.,0.,0,N_mdm(type,distributionType_CUMULATIVE)}
		},
	kw_197[1] = {
		{"num_gen_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,genReliabilityLevels)}
		},
	kw_198[1] = {
		{"num_probability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,probabilityLevels)}
		},
	kw_199[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_200[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_199}
		},
	kw_201[2] = {
		{"compute",8,3,2,0,kw_200},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_202[7] = {
		{0,0,1,0,0,kw_27},
		{"distribution",8,2,5,0,kw_196},
		{"gen_reliability_levels",14,1,4,0,kw_197,0.,0.,0,N_mdm(resplevs,genReliabilityLevels)},
		{"nip",8,0,1,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_NIP)},
		{"probability_levels",14,1,3,0,kw_198,0.,0.,0,N_mdm(resplevs01,probabilityLevels)},
		{"response_levels",14,2,2,0,kw_201,0.,0.,0,N_mdm(resplevs,responseLevels)},
		{"sqp",8,0,1,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_SQP)}
		},
	kw_203[3] = {
		{0,0,1,0,0,kw_27},
		{"nip",8,0,1,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_NIP)},
		{"sqp",8,0,1,0,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_SQP)}
		},
	kw_204[5] = {
		{"adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_AIS)},
		{"import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_IS)},
		{"mm_adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_MMAIS)},
		{"refinement_samples",9,0,2,0,0,0.,0.,0,N_mdm(int,refineSamples)},
		{"seed",0x19,0,3,0,0,0.,0.,0,N_mdm(pint,randomSeed)}
		},
	kw_205[4] = {
		{"first_order",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilityIntegration_first_order)},
		{"probability_refinement",8,5,2,0,kw_204},
		{"sample_refinement",0,5,2,0,kw_204,0.,0.,-1},
		{"second_order",8,0,1,1,0,0.,0.,0,N_mdm(lit,reliabilityIntegration_second_order)}
		},
	kw_206[10] = {
		{"integration",8,4,3,0,kw_205},
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
	kw_207[1] = {
		{"num_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,reliabilityLevels)}
		},
	kw_208[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_209[4] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_RELIABILITIES)},
		{"system",8,2,2,0,kw_208}
		},
	kw_210[2] = {
		{"compute",8,4,2,0,kw_209},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_211[5] = {
		{0,0,1,0,0,kw_27},
		{0,0,3,0,0,kw_169},
		{"mpp_search",8,10,1,0,kw_206},
		{"reliability_levels",14,1,3,0,kw_207,0.,0.,0,N_mdm(resplevs,reliabilityLevels)},
		{"response_levels",14,2,2,0,kw_210,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_212[9] = {
		{0,0,1,0,0,kw_27},
		{0,0,9,0,0,kw_44},
		{"display_all_evaluations",8,0,7,0,0,0.,0.,0,N_mdm(true,showAllEval)},
		{"display_format",11,0,4,0,0,0.,0.,0,N_mdm(str,displayFormat)},
		{"function_precision",10,0,1,0,0,0.,0.,0,N_mdm(Real,functionPrecision)},
		{"history_file",11,0,3,0,0,0.,0.,0,N_mdm(str,historyFile)},
		{"neighbor_order",0x19,0,6,0,0,0.,0.,0,N_mdm(pint,neighborOrder)},
		{"seed",0x19,0,2,0,0,0.,0.,0,N_mdm(pint,randomSeed)},
		{"variable_neighborhood_search",10,0,5,0,0,0.,0.,0,N_mdm(Real,vns)}
		},
	kw_213[2] = {
		{"num_offspring",0x19,0,2,0,0,0.,0.,0,N_mdm(pintz,numOffspring)},
		{"num_parents",0x19,0,1,0,0,0.,0.,0,N_mdm(pintz,numParents)}
		},
	kw_214[5] = {
		{"crossover_rate",10,0,2,0,0,0.,0.,0,N_mdm(litz,TYPE_DATA_crossoverType_null_crossover)},
		{"multi_point_binary",9,0,1,1,0,0.,0.,0,N_mdm(ilit2p,TYPE_DATA_crossoverType_multi_point_binary)},
		{"multi_point_parameterized_binary",9,0,1,1,0,0.,0.,0,N_mdm(ilit2p,TYPE_DATA_crossoverType_multi_point_parameterized_binary)},
		{"multi_point_real",9,0,1,1,0,0.,0.,0,N_mdm(ilit2p,TYPE_DATA_crossoverType_multi_point_real)},
		{"shuffle_random",8,2,1,1,kw_213,0.,0.,0,N_mdm(litc,TYPE_DATA_crossoverType_shuffle_random)}
		},
	kw_215[3] = {
		{"flat_file",11,0,1,1,0,0.,0.,0,N_mdm(slit2,TYPE_DATA_initializationType_flat_file)},
		{"simple_random",8,0,1,1,0,0.,0.,0,N_mdm(lit,initializationType_random)},
		{"unique_random",8,0,1,1,0,0.,0.,0,N_mdm(lit,initializationType_unique_random)}
		},
	kw_216[1] = {
		{"mutation_scale",10,0,1,0,0,0.,0.,0,N_mdm(Real01,mutationScale)}
		},
	kw_217[6] = {
		{"bit_random",8,0,1,1,0,0.,0.,0,N_mdm(lit,mutationType_bit_random)},
		{"mutation_rate",10,0,2,0,0,0.,0.,0,N_mdm(litz,TYPE_DATA_mutationType_null_mutation)},
		{"offset_cauchy",8,1,1,1,kw_216,0.,0.,0,N_mdm(litc,TYPE_DATA_mutationType_offset_cauchy)},
		{"offset_normal",8,1,1,1,kw_216,0.,0.,0,N_mdm(litc,TYPE_DATA_mutationType_offset_normal)},
		{"offset_uniform",8,1,1,1,kw_216,0.,0.,0,N_mdm(litc,TYPE_DATA_mutationType_offset_uniform)},
		{"replace_uniform",8,0,1,1,0,0.,0.,0,N_mdm(lit,mutationType_replace_uniform)}
		},
	kw_218[7] = {
		{"crossover_type",8,5,5,0,kw_214},
		{"initialization_type",8,3,4,0,kw_215},
		{"log_file",11,0,2,0,0,0.,0.,0,N_mdm(str,logFile)},
		{"mutation_type",8,6,6,0,kw_217},
		{"population_size",0x29,0,1,0,0,0.,0.,0,N_mdm(nnint,populationSize)},
		{"print_each_pop",8,0,3,0,0,0.,0.,0,N_mdm(true,printPopFlag)},
		{"seed",0x19,0,7,0,0,0.,0.,0,N_mdm(pint,randomSeed)}
		},
	kw_219[3] = {
		{"metric_tracker",8,0,1,1,0,0.,0.,0,N_mdm(lit,convergenceType_metric_tracker)},
		{"num_generations",0x29,0,3,0,0,0.,0.,0,N_mdm(sizet,numGenerations)},
		{"percent_change",10,0,2,0,0,0.,0.,0,N_mdm(Realz,convergenceTolerance)}
		},
	kw_220[2] = {
		{"domination_count",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_domination_count)},
		{"layer_rank",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_layer_rank)}
		},
	kw_221[1] = {
		{"num_designs",0x29,0,1,0,0,2.,0.,0,N_mdm(pintz,numDesigns)}
		},
	kw_222[3] = {
		{"distance",14,0,1,1,0,0.,0.,0,N_mdm(RealLlit,TYPE_DATA_nichingType_distance)},
		{"max_designs",14,1,1,1,kw_221,0.,0.,0,N_mdm(RealLlit,TYPE_DATA_nichingType_max_designs)},
		{"radial",14,0,1,1,0,0.,0.,0,N_mdm(RealLlit,TYPE_DATA_nichingType_radial)}
		},
	kw_223[1] = {
		{"orthogonal_distance",14,0,1,1,0,0.,0.,0,N_mdm(RealLlit,TYPE_DATA_postProcessorType_distance_postprocessor)}
		},
	kw_224[2] = {
		{"shrinkage_fraction",10,0,1,0,0,0.,0.,0,N_mdm(Real01,shrinkagePercent)},
		{"shrinkage_percentage",2,0,1,0,0,0.,0.,-1,N_mdm(Real01,shrinkagePercent)}
		},
	kw_225[4] = {
		{"below_limit",10,2,1,1,kw_224,0.,0.,0,N_mdm(litp,TYPE_DATA_replacementType_below_limit)},
		{"elitist",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_elitist)},
		{"roulette_wheel",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_roulette_wheel)},
		{"unique_roulette_wheel",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_unique_roulette_wheel)}
		},
	kw_226[8] = {
		{0,0,1,0,0,kw_27},
		{0,0,9,0,0,kw_44},
		{0,0,7,0,0,kw_218},
		{"convergence_type",8,3,4,0,kw_219},
		{"fitness_type",8,2,1,0,kw_220},
		{"niching_type",8,3,3,0,kw_222},
		{"postprocessor_type",8,1,5,0,kw_223},
		{"replacement_type",8,4,2,0,kw_225}
		},
	kw_227[1] = {
		{"model_pointer",11,0,1,0,0,0.,0.,0,N_mdm(str,subModelPointer)}
		},
	kw_228[1] = {
		{"seed",9,0,1,0,0,0.,0.,0,N_mdm(int,randomSeed)}
		},
	kw_229[5] = {
		{0,0,3,0,0,kw_180},
		{"method_name",11,1,1,1,kw_227,0.,0.,0,N_mdm(str,subMethodName)},
		{"method_pointer",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodPointer)},
		{"random_starts",9,1,2,0,kw_228,0.,0.,0,N_mdm(int,concurrentRandomJobs)},
		{"starting_points",14,0,3,0,0,0.,0.,0,N_mdm(RealDL,concurrentParameterSets)}
		},
	kw_230[2] = {
		{0,0,1,0,0,kw_27},
		{"partitions",13,0,1,1,0,0.,0.,0,N_mdm(usharray,varPartitions)}
		},
	kw_231[5] = {
		{0,0,1,0,0,kw_27},
		{"min_boxsize_limit",10,0,2,0,0,0.,0.,0,N_mdm(Real,minBoxSize)},
		{"solution_accuracy",2,0,1,0,0,0.,0.,1,N_mdm(Real,solnTarget)},
		{"solution_target",10,0,1,0,0,0.,0.,0,N_mdm(Real,solnTarget)},
		{"volume_boxsize_limit",10,0,3,0,0,0.,0.,0,N_mdm(Real,volBoxSize)}
		},
	kw_232[10] = {
		{0,0,1,0,0,kw_27},
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
	kw_233[2] = {
		{"global",8,0,1,1,0,0.,0.,0,N_mdm(lit,lipschitzType_global)},
		{"local",8,0,1,1,0,0.,0.,0,N_mdm(lit,lipschitzType_local)}
		},
	kw_234[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_235[3] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"system",8,2,2,0,kw_234}
		},
	kw_236[2] = {
		{"compute",8,3,2,0,kw_235},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_237[6] = {
		{0,0,1,0,0,kw_27},
		{0,0,2,0,0,kw_28},
		{0,0,4,0,0,kw_33},
		{"emulator_samples",9,0,2,0,0,0.,0.,0,N_mdm(int,emulatorSamples)},
		{"lipschitz",8,2,1,0,kw_233},
		{"response_levels",14,2,3,0,kw_236,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_238[1] = {
		{"num_reliability_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,reliabilityLevels)}
		},
	kw_239[2] = {
		{"parallel",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_PARALLEL)},
		{"series",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTargetReduce_SYSTEM_SERIES)}
		},
	kw_240[4] = {
		{"gen_reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_GEN_RELIABILITIES)},
		{"probabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_PROBABILITIES)},
		{"reliabilities",8,0,1,1,0,0.,0.,0,N_mdm(type,responseLevelTarget_RELIABILITIES)},
		{"system",8,2,2,0,kw_239}
		},
	kw_241[2] = {
		{"compute",8,4,2,0,kw_240},
		{"num_response_levels",13,0,1,0,0,0.,0.,0,N_mdm(num_resplevs,responseLevels)}
		},
	kw_242[2] = {
		{"reliability_levels",14,1,1,0,kw_238,0.,0.,0,N_mdm(resplevs,reliabilityLevels)},
		{"response_levels",14,2,2,0,kw_241,0.,0.,0,N_mdm(resplevs,responseLevels)}
		},
	kw_243[1] = {
		{"fixed_seed",8,0,1,0,0,0.,0.,0,N_mdm(true,fixedSeedFlag)}
		},
	kw_244[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_245[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_244,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxImportFormat_TABULAR_NONE)}
		},
	kw_246[2] = {
		{"import_build_points_file",11,4,1,0,kw_245,0.,0.,0,N_mdm(str,approxImportFile)},
		{"import_points_file",3,4,1,0,kw_245,0.,0.,-1,N_mdm(str,approxImportFile)}
		},
	kw_247[2] = {
		{"advancements",9,0,1,0,0,0.,0.,0,N_mdm(ushint,adaptedBasisAdvancements)},
		{"soft_convergence_limit",9,0,2,0,0,0.,0.,0,N_mdm(ushint,softConvLimit)}
		},
	kw_248[3] = {
		{"adapted",8,2,1,1,kw_247,0.,0.,0,N_mdm(type,expansionBasisType_ADAPTED_BASIS_EXPANDING_FRONT)},
		{"tensor_product",8,0,1,1,0,0.,0.,0,N_mdm(type,expansionBasisType_TENSOR_PRODUCT_BASIS)},
		{"total_order",8,0,1,1,0,0.,0.,0,N_mdm(type,expansionBasisType_TOTAL_ORDER_BASIS)}
		},
	kw_249[1] = {
		{"noise_tolerance",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,regressionNoiseTol)}
		},
	kw_250[1] = {
		{"noise_tolerance",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,regressionNoiseTol)}
		},
	kw_251[2] = {
		{"l2_penalty",10,0,2,0,0,0.,0.,0,N_mdm(Real,regressionL2Penalty)},
		{"noise_tolerance",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,regressionNoiseTol)}
		},
	kw_252[2] = {
		{"equality_constrained",8,0,1,0,0,0.,0.,0,N_mdm(type,lsRegressionType_EQ_CON_LS)},
		{"svd",8,0,1,0,0,0.,0.,0,N_mdm(type,lsRegressionType_SVD_LS)}
		},
	kw_253[1] = {
		{"noise_tolerance",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,regressionNoiseTol)}
		},
	kw_254[17] = {
		{"basis_pursuit",8,0,2,0,0,0.,0.,0,N_mdm(type,regressionType_BASIS_PURSUIT)},
		{"basis_pursuit_denoising",8,1,2,0,kw_249,0.,0.,0,N_mdm(type,regressionType_BASIS_PURSUIT_DENOISING)},
		{"bp",0,0,2,0,0,0.,0.,-2,N_mdm(type,regressionType_BASIS_PURSUIT)},
		{"bpdn",0,1,2,0,kw_249,0.,0.,-2,N_mdm(type,regressionType_BASIS_PURSUIT_DENOISING)},
		{"cross_validation",8,0,3,0,0,0.,0.,0,N_mdm(true,crossValidation)},
		{"lars",0,1,2,0,kw_250,0.,0.,3,N_mdm(type,regressionType_LEAST_ANGLE_REGRESSION)},
		{"lasso",0,2,2,0,kw_251,0.,0.,1,N_mdm(type,regressionType_LASSO_REGRESSION)},
		{"least_absolute_shrinkage",8,2,2,0,kw_251,0.,0.,0,N_mdm(type,regressionType_LASSO_REGRESSION)},
		{"least_angle_regression",8,1,2,0,kw_250,0.,0.,0,N_mdm(type,regressionType_LEAST_ANGLE_REGRESSION)},
		{"least_squares",8,2,2,0,kw_252,0.,0.,0,N_mdm(type,regressionType_DEFAULT_LEAST_SQ_REGRESSION)},
		{"omp",0,1,2,0,kw_253,0.,0.,1,N_mdm(type,regressionType_ORTHOG_MATCH_PURSUIT)},
		{"orthogonal_matching_pursuit",8,1,2,0,kw_253,0.,0.,0,N_mdm(type,regressionType_ORTHOG_MATCH_PURSUIT)},
		{"ratio_order",10,0,1,0,0,0.,0.,0,N_mdm(Realp,collocRatioTermsOrder)},
		{"reuse_points",8,0,6,0,0,0.,0.,0,N_mdm(lit,pointReuse_all)},
		{"reuse_samples",0,0,6,0,0,0.,0.,-1,N_mdm(lit,pointReuse_all)},
		{"tensor_grid",8,0,5,0,0,0.,0.,0,N_mdm(true,tensorGridFlag)},
		{"use_derivatives",8,0,4,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)}
		},
	kw_255[3] = {
		{"incremental_lhs",8,0,2,0,0,0.,0.,0,N_mdm(lit,expansionSampleType_incremental_lhs)},
		{"reuse_points",8,0,1,0,0,0.,0.,0,N_mdm(lit,pointReuse_all)},
		{"reuse_samples",0,0,1,0,0,0.,0.,-1,N_mdm(lit,pointReuse_all)}
		},
	kw_256[6] = {
		{0,0,2,0,0,kw_246},
		{"basis_type",8,3,2,0,kw_248},
		{"collocation_points",13,17,3,1,kw_254,0.,0.,0,N_mdm(szarray,collocationPoints)},
		{"collocation_ratio",10,17,3,1,kw_254,0.,0.,0,N_mdm(Realp,collocationRatio)},
		{"dimension_preference",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,anisoDimPref)},
		{"expansion_samples",13,3,3,1,kw_255,0.,0.,0,N_mdm(szarray,expansionSamples)}
		},
	kw_257[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_IFACE_ID)}
		},
	kw_258[3] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_257,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_NONE)}
		},
	kw_259[6] = {
		{0,0,2,0,0,kw_246},
		{"collocation_points",13,0,1,1,0,0.,0.,0,N_mdm(szarray,collocationPoints)},
		{"cross_validation",8,0,2,0,0,0.,0.,0,N_mdm(true,crossValidation)},
		{"reuse_points",8,0,4,0,0,0.,0.,0,N_mdm(lit,pointReuse_all)},
		{"reuse_samples",0,0,4,0,0,0.,0.,-1,N_mdm(lit,pointReuse_all)},
		{"tensor_grid",13,0,3,0,0,0.,0.,0,N_mdm(usharray,tensorGridOrder)}
		},
	kw_260[3] = {
		{"decay",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_DECAY)},
		{"generalized",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_GENERALIZED)},
		{"sobol",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_SOBOL)}
		},
	kw_261[2] = {
		{"dimension_adaptive",8,3,1,1,kw_260},
		{"uniform",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_UNIFORM_CONTROL)}
		},
	kw_262[4] = {
		{"adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_AIS)},
		{"import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_IS)},
		{"mm_adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_MMAIS)},
		{"refinement_samples",9,0,2,0,0,0.,0.,0,N_mdm(int,refineSamples)}
		},
	kw_263[3] = {
		{"dimension_preference",14,0,1,0,0,0.,0.,0,N_mdm(RealDL,anisoDimPref)},
		{"nested",8,0,2,0,0,0.,0.,0,N_mdm(type,nestingOverride_NESTED)},
		{"non_nested",8,0,2,0,0,0.,0.,0,N_mdm(type,nestingOverride_NON_NESTED)}
		},
	kw_264[2] = {
		{"lhs",8,0,1,1,0,0.,0.,0,N_mdm(utype,sampleType_SUBMETHOD_LHS)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(utype,sampleType_SUBMETHOD_RANDOM)}
		},
	kw_265[3] = {
		{0,0,3,0,0,kw_263},
		{"restricted",8,0,1,0,0,0.,0.,0,N_mdm(type,growthOverride_RESTRICTED)},
		{"unrestricted",8,0,1,0,0,0.,0.,0,N_mdm(type,growthOverride_UNRESTRICTED)}
		},
	kw_266[2] = {
		{"drop_tolerance",10,0,2,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)},
		{"interaction_order",0x19,0,1,0,0,0.,0.,0,N_mdm(ushint,vbdOrder)}
		},
	kw_267[25] = {
		{0,0,1,0,0,kw_27},
		{0,0,2,0,0,kw_28},
		{0,0,4,0,0,kw_33},
		{0,0,2,0,0,kw_242},
		{0,0,1,0,0,kw_243},
		{"askey",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionType_ASKEY_U)},
		{"cubature_integrand",9,0,3,1,0,0.,0.,0,N_mdm(ushint,cubIntOrder)},
		{"diagonal_covariance",8,0,5,0,0,0.,0.,0,N_mdm(type,covarianceControl_DIAGONAL_COVARIANCE)},
		{"expansion_order",13,5,3,1,kw_256,0.,0.,0,N_mdm(usharray,expansionOrder)},
		{"export_expansion_file",11,0,10,0,0,0.,0.,0,N_mdm(str,expansionExportFile)},
		{"export_points_file",11,3,9,0,kw_258,0.,0.,0,N_mdm(str,approxExportFile)},
		{"full_covariance",8,0,5,0,0,0.,0.,0,N_mdm(type,covarianceControl_FULL_COVARIANCE)},
		{"import_expansion_file",11,0,3,1,0,0.,0.,0,N_mdm(str,expansionImportFile)},
		{"least_interpolation",0,5,3,1,kw_259,0.,0.,3,N_mdm(type,regressionType_ORTHOG_LEAST_INTERPOLATION)},
		{"normalized",8,0,6,0,0,0.,0.,0,N_mdm(true,normalizedCoeffs)},
		{"oli",0,5,3,1,kw_259,0.,0.,1,N_mdm(type,regressionType_ORTHOG_LEAST_INTERPOLATION)},
		{"orthogonal_least_interpolation",8,5,3,1,kw_259,0.,0.,0,N_mdm(type,regressionType_ORTHOG_LEAST_INTERPOLATION)},
		{"p_refinement",8,2,1,0,kw_261,0.,0.,0,N_mdm(type,refinementType_P_REFINEMENT)},
		{"probability_refinement",8,4,8,0,kw_262},
		{"quadrature_order",13,3,3,1,kw_263,0.,0.,0,N_mdm(usharray,quadratureOrder)},
		{"sample_refinement",0,4,8,0,kw_262,0.,0.,-2},
		{"sample_type",8,2,7,0,kw_264},
		{"sparse_grid_level",13,2,3,1,kw_265,0.,0.,0,N_mdm(usharray,sparseGridLevel)},
		{"variance_based_decomp",8,2,4,0,kw_266,0.,0.,0,N_mdm(true,vbdFlag)},
		{"wiener",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionType_STD_NORMAL_U)}
		},
	kw_268[1] = {
		{"percent_variance_explained",10,0,1,0,0,0.,0.,0,N_mdm(Real,percentVarianceExplained)}
		},
	kw_269[1] = {
		{"previous_samples",9,0,1,1,0,0.,0.,0,N_mdm(int,previousSamples)}
		},
	kw_270[4] = {
		{"incremental_lhs",8,1,1,1,kw_269,0.,0.,0,N_mdm(utype,sampleType_SUBMETHOD_INCREMENTAL_LHS)},
		{"incremental_random",8,1,1,1,kw_269,0.,0.,0,N_mdm(utype,sampleType_SUBMETHOD_INCREMENTAL_RANDOM)},
		{"lhs",8,0,1,1,0,0.,0.,0,N_mdm(utype,sampleType_SUBMETHOD_LHS)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(utype,sampleType_SUBMETHOD_RANDOM)}
		},
	kw_271[1] = {
		{"drop_tolerance",10,0,1,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)}
		},
	kw_272[9] = {
		{0,0,1,0,0,kw_27},
		{0,0,2,0,0,kw_28},
		{0,0,4,0,0,kw_33},
		{0,0,2,0,0,kw_242},
		{0,0,1,0,0,kw_243},
		{"backfill",8,0,3,0,0,0.,0.,0,N_mdm(true,backfillFlag)},
		{"principal_components",8,1,4,0,kw_268,0.,0.,0,N_mdm(true,pcaFlag)},
		{"sample_type",8,4,1,0,kw_270},
		{"variance_based_decomp",8,1,2,0,kw_271,0.,0.,0,N_mdm(true,vbdFlag)}
		},
	kw_273[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mdm(augment_utype,approxExportFormat_TABULAR_IFACE_ID)}
		},
	kw_274[3] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_273,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mdm(utype,approxExportFormat_TABULAR_NONE)}
		},
	kw_275[2] = {
		{"generalized",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_GENERALIZED)},
		{"sobol",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_SOBOL)}
		},
	kw_276[3] = {
		{"dimension_adaptive",8,2,1,1,kw_275},
		{"local_adaptive",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_LOCAL_ADAPTIVE_CONTROL)},
		{"uniform",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_UNIFORM_CONTROL)}
		},
	kw_277[2] = {
		{"generalized",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_GENERALIZED)},
		{"sobol",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_DIMENSION_ADAPTIVE_CONTROL_SOBOL)}
		},
	kw_278[2] = {
		{"dimension_adaptive",8,2,1,1,kw_277},
		{"uniform",8,0,1,1,0,0.,0.,0,N_mdm(type,refinementControl_UNIFORM_CONTROL)}
		},
	kw_279[4] = {
		{"adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_AIS)},
		{"import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_IS)},
		{"mm_adapt_import",8,0,1,1,0,0.,0.,0,N_mdm(utype,integrationRefine_MMAIS)},
		{"refinement_samples",9,0,2,0,0,0.,0.,0,N_mdm(int,refineSamples)}
		},
	kw_280[2] = {
		{"lhs",8,0,1,1,0,0.,0.,0,N_mdm(utype,sampleType_SUBMETHOD_LHS)},
		{"random",8,0,1,1,0,0.,0.,0,N_mdm(utype,sampleType_SUBMETHOD_RANDOM)}
		},
	kw_281[4] = {
		{"hierarchical",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionBasisType_HIERARCHICAL_INTERPOLANT)},
		{"nodal",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionBasisType_NODAL_INTERPOLANT)},
		{"restricted",8,0,1,0,0,0.,0.,0,N_mdm(type,growthOverride_RESTRICTED)},
		{"unrestricted",8,0,1,0,0,0.,0.,0,N_mdm(type,growthOverride_UNRESTRICTED)}
		},
	kw_282[2] = {
		{"drop_tolerance",10,0,2,0,0,0.,0.,0,N_mdm(Real,vbdDropTolerance)},
		{"interaction_order",0x19,0,1,0,0,0.,0.,0,N_mdm(ushint,vbdOrder)}
		},
	kw_283[23] = {
		{0,0,1,0,0,kw_27},
		{0,0,2,0,0,kw_28},
		{0,0,4,0,0,kw_33},
		{0,0,2,0,0,kw_242},
		{0,0,1,0,0,kw_243},
		{"askey",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionType_ASKEY_U)},
		{"diagonal_covariance",8,0,8,0,0,0.,0.,0,N_mdm(type,covarianceControl_DIAGONAL_COVARIANCE)},
		{"dimension_preference",14,0,4,0,0,0.,0.,0,N_mdm(RealDL,anisoDimPref)},
		{"export_points_file",11,3,11,0,kw_274,0.,0.,0,N_mdm(str,approxExportFile)},
		{"full_covariance",8,0,8,0,0,0.,0.,0,N_mdm(type,covarianceControl_FULL_COVARIANCE)},
		{"h_refinement",8,3,1,0,kw_276,0.,0.,0,N_mdm(type,refinementType_H_REFINEMENT)},
		{"nested",8,0,6,0,0,0.,0.,0,N_mdm(type,nestingOverride_NESTED)},
		{"non_nested",8,0,6,0,0,0.,0.,0,N_mdm(type,nestingOverride_NON_NESTED)},
		{"p_refinement",8,2,1,0,kw_278,0.,0.,0,N_mdm(type,refinementType_P_REFINEMENT)},
		{"piecewise",8,0,2,0,0,0.,0.,0,NIDRProblemDescDB::method_piecewise},
		{"probability_refinement",8,4,10,0,kw_279},
		{"quadrature_order",13,0,3,1,0,0.,0.,0,N_mdm(usharray,quadratureOrder)},
		{"sample_refinement",0,4,10,0,kw_279,0.,0.,-2},
		{"sample_type",8,2,9,0,kw_280},
		{"sparse_grid_level",13,4,3,1,kw_281,0.,0.,0,N_mdm(usharray,sparseGridLevel)},
		{"use_derivatives",8,0,5,0,0,0.,0.,0,N_mdm(true,methodUseDerivsFlag)},
		{"variance_based_decomp",8,2,7,0,kw_282,0.,0.,0,N_mdm(true,vbdFlag)},
		{"wiener",8,0,2,0,0,0.,0.,0,N_mdm(type,expansionType_STD_NORMAL_U)}
		},
	kw_284[2] = {
		{0,0,1,0,0,kw_27},
		{"misc_options",15,0,1,0,0,0.,0.,0,N_mdm(strL,miscOptions)}
		},
	kw_285[3] = {
		{"function_precision",10,0,2,0,0,0.,0.,0,N_mdm(Real,functionPrecision)},
		{"linesearch_tolerance",10,0,3,0,0,0.,0.,0,N_mdm(Real,lineSearchTolerance)},
		{"verify_level",9,0,1,0,0,0.,0.,0,N_mdm(int,verifyLevel)}
		},
	kw_286[4] = {
		{0,0,1,0,0,kw_27},
		{0,0,9,0,0,kw_44},
		{0,0,3,0,0,kw_285},
		{""}
		},
	kw_287[2] = {
		{"gradient_tolerance",10,0,2,0,0,0.,0.,0,N_mdm(Real,gradientTolerance)},
		{"max_step",10,0,1,0,0,0.,0.,0,N_mdm(Real,maxStep)}
		},
	kw_288[4] = {
		{0,0,1,0,0,kw_27},
		{0,0,9,0,0,kw_44},
		{0,0,2,0,0,kw_287},
		{""}
		},
	kw_289[3] = {
		{0,0,1,0,0,kw_27},
		{0,0,9,0,0,kw_44},
		{"search_scheme_size",9,0,1,0,0,0.,0.,0,N_mdm(int,searchSchemeSize)}
		},
	kw_290[3] = {
		{"argaez_tapia",8,0,1,1,0,0.,0.,0,N_mdm(type,meritFn_ArgaezTapia)},
		{"el_bakry",8,0,1,1,0,0.,0.,0,N_mdm(type,meritFn_NormFmu)},
		{"van_shanno",8,0,1,1,0,0.,0.,0,N_mdm(type,meritFn_VanShanno)}
		},
	kw_291[4] = {
		{"gradient_based_line_search",8,0,1,1,0,0.,0.,0,N_mdm(lit,searchMethod_gradient_based_line_search)},
		{"tr_pds",8,0,1,1,0,0.,0.,0,N_mdm(lit,searchMethod_tr_pds)},
		{"trust_region",8,0,1,1,0,0.,0.,0,N_mdm(lit,searchMethod_trust_region)},
		{"value_based_line_search",8,0,1,1,0,0.,0.,0,N_mdm(lit,searchMethod_value_based_line_search)}
		},
	kw_292[7] = {
		{0,0,1,0,0,kw_27},
		{0,0,9,0,0,kw_44},
		{0,0,2,0,0,kw_287},
		{"centering_parameter",10,0,4,0,0,0.,0.,0,N_mdm(Real,centeringParam)},
		{"merit_function",8,3,2,0,kw_290},
		{"search_method",8,4,1,0,kw_291},
		{"steplength_to_boundary",10,0,3,0,0,0.,0.,0,N_mdm(Real,stepLenToBoundary)}
		},
	kw_293[5] = {
		{"debug",8,0,1,1,0,0.,0.,0,N_mdm(type,methodOutput_DEBUG_OUTPUT)},
		{"normal",8,0,1,1,0,0.,0.,0,N_mdm(type,methodOutput_NORMAL_OUTPUT)},
		{"quiet",8,0,1,1,0,0.,0.,0,N_mdm(type,methodOutput_QUIET_OUTPUT)},
		{"silent",8,0,1,1,0,0.,0.,0,N_mdm(type,methodOutput_SILENT_OUTPUT)},
		{"verbose",8,0,1,1,0,0.,0.,0,N_mdm(type,methodOutput_VERBOSE_OUTPUT)}
		},
	kw_294[2] = {
		{"model_pointer",11,0,1,0,0,0.,0.,0,N_mdm(str,subModelPointer)},
		{"opt_model_pointer",3,0,1,0,0,0.,0.,-1,N_mdm(str,subModelPointer)}
		},
	kw_295[1] = {
		{"seed",9,0,1,0,0,0.,0.,0,N_mdm(int,randomSeed)}
		},
	kw_296[8] = {
		{0,0,3,0,0,kw_180},
		{"method_name",11,2,1,1,kw_294,0.,0.,0,N_mdm(str,subMethodName)},
		{"method_pointer",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodPointer)},
		{"multi_objective_weight_sets",6,0,3,0,0,0.,0.,4,N_mdm(RealDL,concurrentParameterSets)},
		{"opt_method_name",3,2,1,1,kw_294,0.,0.,-3,N_mdm(str,subMethodName)},
		{"opt_method_pointer",3,0,1,1,0,0.,0.,-3,N_mdm(str,subMethodPointer)},
		{"random_weight_sets",9,1,2,0,kw_295,0.,0.,0,N_mdm(int,concurrentRandomJobs)},
		{"weight_sets",14,0,3,0,0,0.,0.,0,N_mdm(RealDL,concurrentParameterSets)}
		},
	kw_297[3] = {
		{0,0,1,0,0,kw_27},
		{0,0,2,0,0,kw_28},
		{"partitions",13,0,1,0,0,0.,0.,0,N_mdm(usharray,varPartitions)}
		},
	kw_298[5] = {
		{0,0,1,0,0,kw_27},
		{"converge_order",8,0,1,1,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_CONVERGE_ORDER)},
		{"converge_qoi",8,0,1,1,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_CONVERGE_QOI)},
		{"estimate_order",8,0,1,1,0,0.,0.,0,N_mdm(utype,subMethod_SUBMETHOD_ESTIMATE_ORDER)},
		{"refinement_rate",10,0,2,0,0,0.,0.,0,N_mdm(Real,refinementRate)}
		},
	kw_299[2] = {
		{"num_generations",0x29,0,2,0,0,0.,0.,0,N_mdm(sizet,numGenerations)},
		{"percent_change",10,0,1,0,0,0.,0.,0,N_mdm(Realz,convergenceTolerance)}
		},
	kw_300[2] = {
		{"num_generations",0x29,0,2,0,0,0.,0.,0,N_mdm(sizet,numGenerations)},
		{"percent_change",10,0,1,0,0,0.,0.,0,N_mdm(Realz,convergenceTolerance)}
		},
	kw_301[2] = {
		{"average_fitness_tracker",8,2,1,1,kw_299,0.,0.,0,N_mdm(lit,convergenceType_average_fitness_tracker)},
		{"best_fitness_tracker",8,2,1,1,kw_300,0.,0.,0,N_mdm(lit,convergenceType_best_fitness_tracker)}
		},
	kw_302[2] = {
		{"constraint_penalty",10,0,2,0,0,0.,0.,0,N_mdm(Realp,constraintTolerance)},
		{"merit_function",8,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_merit_function)}
		},
	kw_303[4] = {
		{"elitist",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_elitist)},
		{"favor_feasible",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_favor_feasible)},
		{"roulette_wheel",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_roulette_wheel)},
		{"unique_roulette_wheel",8,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_unique_roulette_wheel)}
		},
	kw_304[6] = {
		{0,0,1,0,0,kw_27},
		{0,0,9,0,0,kw_44},
		{0,0,7,0,0,kw_218},
		{"convergence_type",8,2,3,0,kw_301},
		{"fitness_type",8,2,1,0,kw_302},
		{"replacement_type",8,4,2,0,kw_303}
		},
	kw_305[5] = {
		{0,0,1,0,0,kw_27},
		{0,0,9,0,0,kw_44},
		{0,0,3,0,0,kw_285},
		{"nlssol",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_NLSSOL_SQP)},
		{"npsol",8,0,1,1,0,0.,0.,0,N_mdm(utype,methodName_NPSOL_SQP)}
		},
	kw_306[7] = {
		{"approx_method_name",3,0,1,1,0,0.,0.,3,N_mdm(str,subMethodName)},
		{"approx_method_pointer",3,0,1,1,0,0.,0.,3,N_mdm(str,subMethodPointer)},
		{"approx_model_pointer",3,0,2,2,0,0.,0.,3,N_mdm(str,modelPointer)},
		{"method_name",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodName)},
		{"method_pointer",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodPointer)},
		{"model_pointer",11,0,2,2,0,0.,0.,0,N_mdm(str,modelPointer)},
		{"replace_points",8,0,3,0,0,0.,0.,0,N_mdm(true,surrBasedGlobalReplacePts)}
		},
	kw_307[2] = {
		{"filter",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalAcceptLogic_FILTER)},
		{"tr_ratio",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalAcceptLogic_TR_RATIO)}
		},
	kw_308[7] = {
		{"augmented_lagrangian_objective",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbObj_AUGMENTED_LAGRANGIAN_OBJECTIVE)},
		{"lagrangian_objective",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbObj_LAGRANGIAN_OBJECTIVE)},
		{"linearized_constraints",8,0,2,2,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbCon_LINEARIZED_CONSTRAINTS)},
		{"no_constraints",8,0,2,2,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbCon_NO_CONSTRAINTS)},
		{"original_constraints",8,0,2,2,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbCon_ORIGINAL_CONSTRAINTS)},
		{"original_primary",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbObj_ORIGINAL_PRIMARY)},
		{"single_objective",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalSubProbObj_SINGLE_OBJECTIVE)}
		},
	kw_309[1] = {
		{"homotopy",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalConstrRelax_HOMOTOPY)}
		},
	kw_310[4] = {
		{"adaptive_penalty_merit",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalMeritFn_ADAPTIVE_PENALTY_MERIT)},
		{"augmented_lagrangian_merit",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalMeritFn_AUGMENTED_LAGRANGIAN_MERIT)},
		{"lagrangian_merit",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalMeritFn_LAGRANGIAN_MERIT)},
		{"penalty_merit",8,0,1,1,0,0.,0.,0,N_mdm(type,surrBasedLocalMeritFn_PENALTY_MERIT)}
		},
	kw_311[6] = {
		{"contract_threshold",10,0,3,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRContractTrigger)},
		{"contraction_factor",10,0,5,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRContract)},
		{"expand_threshold",10,0,4,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRExpandTrigger)},
		{"expansion_factor",10,0,6,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRExpand)},
		{"initial_size",10,0,1,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRInitSize)},
		{"minimum_size",10,0,2,0,0,0.,0.,0,N_mdm(Real,surrBasedLocalTRMinSize)}
		},
	kw_312[13] = {
		{"acceptance_logic",8,2,8,0,kw_307},
		{"approx_method_name",3,0,1,1,0,0.,0.,6,N_mdm(str,subMethodName)},
		{"approx_method_pointer",3,0,1,1,0,0.,0.,6,N_mdm(str,subMethodPointer)},
		{"approx_model_pointer",3,0,2,2,0,0.,0.,6,N_mdm(str,modelPointer)},
		{"approx_subproblem",8,7,6,0,kw_308},
		{"constraint_relax",8,1,9,0,kw_309},
		{"merit_function",8,4,7,0,kw_310},
		{"method_name",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodName)},
		{"method_pointer",11,0,1,1,0,0.,0.,0,N_mdm(str,subMethodPointer)},
		{"model_pointer",11,0,2,2,0,0.,0.,0,N_mdm(str,modelPointer)},
		{"soft_convergence_limit",9,0,3,0,0,0.,0.,0,N_mdm(ushint,softConvLimit)},
		{"trust_region",8,6,5,0,kw_311,0.,0.,0,0,0,NIDRProblemDescDB::method_tr_final},
		{"truth_surrogate_bypass",8,0,4,0,0,0.,0.,0,N_mdm(true,surrBasedLocalLayerBypass)}
		},
	kw_313[4] = {
		{0,0,1,0,0,kw_27},
		{"final_point",14,0,1,1,0,0.,0.,0,N_mdm(RealDL,finalPoint)},
		{"num_steps",9,0,2,2,0,0.,0.,0,N_mdm(int,numSteps)},
		{"step_vector",14,0,1,1,0,0.,0.,0,N_mdm(RealDL,stepVector)}
		},
	kw_314[91] = {
		{"adaptive_sampling",8,9,10,1,kw_43,0.,0.,0,N_mdm(utype,methodName_ADAPTIVE_SAMPLING)},
		{"asynch_pattern_search",8,9,10,1,kw_47,0.,0.,0,N_mdm(utype,methodName_ASYNCH_PATTERN_SEARCH)},
		{"bayes_calibration",8,7,10,1,kw_100,0.,0.,0,N_mdm(utype,methodName_BAYES_CALIBRATION)},
		{"branch_and_bound",8,0,10,1,kw_102,0.,0.,0,N_mdm(utype,methodName_BRANCH_AND_BOUND)},
		{"centered_parameter_study",8,3,10,1,kw_103,0.,0.,0,N_mdm(utype,methodName_CENTERED_PARAMETER_STUDY)},
		{"coliny_apps",0,9,10,1,kw_47,0.,0.,-4,N_mdm(utype,methodName_ASYNCH_PATTERN_SEARCH)},
		{"coliny_beta",8,1,10,1,kw_104,0.,0.,0,N_mdm(utype,methodName_COLINY_BETA)},
		{"coliny_cobyla",8,0,10,1,kw_106,0.,0.,0,N_mdm(utype,methodName_COLINY_COBYLA)},
		{"coliny_direct",8,6,10,1,kw_108,0.,0.,0,N_mdm(utype,methodName_COLINY_DIRECT)},
		{"coliny_ea",8,9,10,1,kw_115,0.,0.,0,N_mdm(utype,methodName_COLINY_EA)},
		{"coliny_pattern_search",8,8,10,1,kw_120,0.,0.,0,N_mdm(utype,methodName_COLINY_PATTERN_SEARCH)},
		{"coliny_solis_wets",8,4,10,1,kw_121,0.,0.,0,N_mdm(utype,methodName_COLINY_SOLIS_WETS)},
		{"conmin",8,2,10,1,kw_122},
		{"conmin_frcg",8,0,10,1,kw_123,0.,0.,0,N_mdm(utype,methodName_CONMIN_FRCG)},
		{"conmin_mfd",8,0,10,1,kw_123,0.,0.,0,N_mdm(utype,methodName_CONMIN_MFD)},
		{"constraint_tolerance",10,0,7,0,0,0.,0.,0,N_mdm(Real,constraintTolerance)},
		{"convergence_tolerance",10,0,6,0,0,0.,0.,0,N_mdm(Real,convergenceTolerance)},
		{"dace",8,12,10,1,kw_125,0.,0.,0,N_mdm(utype,methodName_DACE)},
		{"dl_solver",11,0,10,1,kw_123,0.,0.,0,N_mdm(utype_lit,TYPE_DATA_methodName_DL_SOLVER)},
		{"dot",8,5,10,1,kw_126},
		{"dot_bfgs",8,0,10,1,kw_123,0.,0.,0,N_mdm(utype,methodName_DOT_BFGS)},
		{"dot_frcg",8,0,10,1,kw_123,0.,0.,0,N_mdm(utype,methodName_DOT_FRCG)},
		{"dot_mmfd",8,0,10,1,kw_123,0.,0.,0,N_mdm(utype,methodName_DOT_MMFD)},
		{"dot_slp",8,0,10,1,kw_123,0.,0.,0,N_mdm(utype,methodName_DOT_SLP)},
		{"dot_sqp",8,0,10,1,kw_123,0.,0.,0,N_mdm(utype,methodName_DOT_SQP)},
		{"efficient_global",8,7,10,1,kw_132,0.,0.,0,N_mdm(utype,methodName_EFFICIENT_GLOBAL)},
		{"efficient_subspace",8,2,10,1,kw_133,0.,0.,0,N_mdm(utype,methodName_EFFICIENT_SUBSPACE)},
		{"final_solutions",0x29,0,9,0,0,0.,0.,0,N_mdm(sizet,numFinalSolutions)},
		{"fsu_cvt",8,6,10,1,kw_136,0.,0.,0,N_mdm(utype,methodName_FSU_CVT)},
		{"fsu_quasi_mc",8,10,10,1,kw_138},
		{"gaussian_process_adaptive_importance_sampling",0,5,10,1,kw_146,0.,0.,6,N_mdm(utype,methodName_GPAIS)},
		{"genie_direct",8,1,10,1,kw_147,0.,0.,0,N_mdm(utype,methodName_GENIE_DIRECT)},
		{"genie_opt_darts",8,1,10,1,kw_147,0.,0.,0,N_mdm(utype,methodName_GENIE_OPT_DARTS)},
		{"global_evidence",8,5,10,1,kw_157,0.,0.,0,N_mdm(utype,methodName_GLOBAL_EVIDENCE)},
		{"global_interval_est",8,5,10,1,kw_165,0.,0.,0,N_mdm(utype,methodName_GLOBAL_INTERVAL_EST)},
		{"global_reliability",8,13,10,1,kw_178,0.,0.,0,N_mdm(utype,methodName_GLOBAL_RELIABILITY)},
		{"gpais",8,5,10,1,kw_146,0.,0.,0,N_mdm(utype,methodName_GPAIS)},
		{"hybrid",8,5,10,1,kw_188,0.,0.,0,N_mdm(utype,methodName_HYBRID)},
		{"id_method",11,0,1,0,0,0.,0.,0,N_mdm(str,idMethod)},
		{"importance_sampling",8,5,10,1,kw_192,0.,0.,0,N_mdm(utype,methodName_IMPORTANCE_SAMPLING)},
		{"list_parameter_study",8,2,10,1,kw_195,0.,0.,0,N_mdm(utype,methodName_LIST_PARAMETER_STUDY)},
		{"local_evidence",8,6,10,1,kw_202,0.,0.,0,N_mdm(utype,methodName_LOCAL_EVIDENCE)},
		{"local_interval_est",8,2,10,1,kw_203,0.,0.,0,N_mdm(utype,methodName_LOCAL_INTERVAL_EST)},
		{"local_reliability",8,3,10,1,kw_211,0.,0.,0,N_mdm(utype,methodName_LOCAL_RELIABILITY)},
		{"max_function_evaluations",0x29,0,4,0,0,0.,0.,0,N_mdm(nnint,maxFunctionEvaluations)},
		{"max_iterations",0x29,0,3,0,0,0.,0.,0,N_mdm(nnint,maxIterations)},
		{"mesh_adaptive_search",8,7,10,1,kw_212,0.,0.,0,N_mdm(utype,methodName_MESH_ADAPTIVE_SEARCH)},
		{"moga",8,5,10,1,kw_226,0.,0.,0,N_mdm(utype,methodName_MOGA)},
		{"multi_start",8,4,10,1,kw_229,0.,0.,0,N_mdm(utype,methodName_MULTI_START)},
		{"multidim_parameter_study",8,1,10,1,kw_230,0.,0.,0,N_mdm(utype,methodName_MULTIDIM_PARAMETER_STUDY)},
		{"ncsu_direct",8,4,10,1,kw_231,0.,0.,0,N_mdm(utype,methodName_NCSU_DIRECT)},
		{"nl2sol",8,9,10,1,kw_232,0.,0.,0,N_mdm(utype,methodName_NL2SOL)},
		{"nlpql_sqp",8,0,10,1,kw_123,0.,0.,0,N_mdm(utype,methodName_NLPQL_SQP)},
		{"nlssol_sqp",8,0,10,1,kw_286,0.,0.,0,N_mdm(utype,methodName_NLSSOL_SQP)},
		{"nond_adaptive_sampling",0,9,10,1,kw_43,0.,0.,-54,N_mdm(utype,methodName_ADAPTIVE_SAMPLING)},
		{"nond_bayes_calibration",0,7,10,1,kw_100,0.,0.,-53,N_mdm(utype,methodName_BAYES_CALIBRATION)},
		{"nond_efficient_subspace",0,2,10,1,kw_133,0.,0.,-30,N_mdm(utype,methodName_EFFICIENT_SUBSPACE)},
		{"nond_global_evidence",0,5,10,1,kw_157,0.,0.,-24,N_mdm(utype,methodName_GLOBAL_EVIDENCE)},
		{"nond_global_interval_est",0,5,10,1,kw_165,0.,0.,-24,N_mdm(utype,methodName_GLOBAL_INTERVAL_EST)},
		{"nond_global_reliability",0,13,10,1,kw_178,0.,0.,-24,N_mdm(utype,methodName_GLOBAL_RELIABILITY)},
		{"nond_importance_sampling",0,5,10,1,kw_192,0.,0.,-21,N_mdm(utype,methodName_IMPORTANCE_SAMPLING)},
		{"nond_local_evidence",0,6,10,1,kw_202,0.,0.,-20,N_mdm(utype,methodName_LOCAL_EVIDENCE)},
		{"nond_local_interval_est",0,2,10,1,kw_203,0.,0.,-20,N_mdm(utype,methodName_LOCAL_INTERVAL_EST)},
		{"nond_local_reliability",0,3,10,1,kw_211,0.,0.,-20,N_mdm(utype,methodName_LOCAL_RELIABILITY)},
		{"nond_pof_darts",0,3,10,1,kw_237,0.,0.,14,N_mdm(utype,methodName_POF_DARTS)},
		{"nond_polynomial_chaos",0,20,10,1,kw_267,0.,0.,14,N_mdm(utype,methodName_POLYNOMIAL_CHAOS)},
		{"nond_sampling",0,4,10,1,kw_272,0.,0.,16,N_mdm(utype,methodName_RANDOM_SAMPLING)},
		{"nond_stoch_collocation",0,18,10,1,kw_283,0.,0.,20,N_mdm(utype,methodName_STOCH_COLLOCATION)},
		{"nonlinear_cg",8,1,10,1,kw_284,0.,0.,0,N_mdm(utype,methodName_NONLINEAR_CG)},
		{"npsol_sqp",8,0,10,1,kw_286,0.,0.,0,N_mdm(utype,methodName_NPSOL_SQP)},
		{"optpp_cg",8,0,10,1,kw_288,0.,0.,0,N_mdm(utype,methodName_OPTPP_CG)},
		{"optpp_fd_newton",8,4,10,1,kw_292,0.,0.,0,N_mdm(utype,methodName_OPTPP_FD_NEWTON)},
		{"optpp_g_newton",8,4,10,1,kw_292,0.,0.,0,N_mdm(utype,methodName_OPTPP_G_NEWTON)},
		{"optpp_newton",8,4,10,1,kw_292,0.,0.,0,N_mdm(utype,methodName_OPTPP_NEWTON)},
		{"optpp_pds",8,1,10,1,kw_289,0.,0.,0,N_mdm(utype,methodName_OPTPP_PDS)},
		{"optpp_q_newton",8,4,10,1,kw_292,0.,0.,0,N_mdm(utype,methodName_OPTPP_Q_NEWTON)},
		{"output",8,5,2,0,kw_293},
		{"pareto_set",8,7,10,1,kw_296,0.,0.,0,N_mdm(utype,methodName_PARETO_SET)},
		{"pof_darts",8,3,10,1,kw_237,0.,0.,0,N_mdm(utype,methodName_POF_DARTS)},
		{"polynomial_chaos",8,20,10,1,kw_267,0.,0.,0,N_mdm(utype,methodName_POLYNOMIAL_CHAOS)},
		{"psuade_moat",8,1,10,1,kw_297,0.,0.,0,N_mdm(utype,methodName_PSUADE_MOAT)},
		{"richardson_extrap",8,4,10,1,kw_298,0.,0.,0,N_mdm(utype,methodName_RICHARDSON_EXTRAP)},
		{"sampling",8,4,10,1,kw_272,0.,0.,0,N_mdm(utype,methodName_RANDOM_SAMPLING)},
		{"scaling",8,0,8,0,0,0.,0.,0,N_mdm(true,methodScaling)},
		{"soga",8,3,10,1,kw_304,0.,0.,0,N_mdm(utype,methodName_SOGA)},
		{"speculative",8,0,5,0,0,0.,0.,0,N_mdm(true,speculativeFlag)},
		{"stanford",8,2,10,1,kw_305},
		{"stoch_collocation",8,18,10,1,kw_283,0.,0.,0,N_mdm(utype,methodName_STOCH_COLLOCATION)},
		{"surrogate_based_global",8,7,10,1,kw_306,0.,0.,0,N_mdm(utype,methodName_SURROGATE_BASED_GLOBAL)},
		{"surrogate_based_local",8,13,10,1,kw_312,0.,0.,0,N_mdm(utype,methodName_SURROGATE_BASED_LOCAL)},
		{"vector_parameter_study",8,3,10,1,kw_313,0.,0.,0,N_mdm(utype,methodName_VECTOR_PARAMETER_STUDY)}
		},
	kw_315[1] = {
		{"optional_interface_responses_pointer",11,0,1,0,0,0.,0.,0,N_mom(str,optionalInterfRespPointer)}
		},
	kw_316[2] = {
		{"master",8,0,1,1,0,0.,0.,0,N_mom(type,subMethodScheduling_MASTER_SCHEDULING)},
		{"peer",8,0,1,1,0,0.,0.,0,N_mom(type,subMethodScheduling_PEER_SCHEDULING)}
		},
	kw_317[7] = {
		{"iterator_scheduling",8,2,2,0,kw_316},
		{"iterator_servers",0x19,0,1,0,0,0.,0.,0,N_mom(pint,subMethodServers)},
		{"primary_response_mapping",14,0,6,0,0,0.,0.,0,N_mom(RealDL,primaryRespCoeffs)},
		{"primary_variable_mapping",15,0,4,0,0,0.,0.,0,N_mom(strL,primaryVarMaps)},
		{"processors_per_iterator",0x19,0,3,0,0,0.,0.,0,N_mom(pint,subMethodProcs)},
		{"secondary_response_mapping",14,0,7,0,0,0.,0.,0,N_mom(RealDL,secondaryRespCoeffs)},
		{"secondary_variable_mapping",15,0,5,0,0,0.,0.,0,N_mom(strL,secondaryVarMaps)}
		},
	kw_318[2] = {
		{"optional_interface_pointer",11,1,1,0,kw_315,0.,0.,0,N_mom(str,interfacePointer)},
		{"sub_method_pointer",11,7,2,1,kw_317,0.,0.,0,N_mom(str,subMethodPointer)}
		},
	kw_319[1] = {
		{"interface_pointer",11,0,1,0,0,0.,0.,0,N_mom(str,interfacePointer)}
		},
	kw_320[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mom(augment_utype,approxChallengeFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mom(augment_utype,approxChallengeFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mom(augment_utype,approxChallengeFormat_TABULAR_IFACE_ID)}
		},
	kw_321[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mom(true,approxChallengeActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mom(utype,approxChallengeFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_320,0.,0.,0,N_mom(utype,approxChallengeFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mom(utype,approxChallengeFormat_TABULAR_NONE)}
		},
	kw_322[6] = {
		{"additive",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_ADDITIVE_CORRECTION)},
		{"combined",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_COMBINED_CORRECTION)},
		{"first_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_1)},
		{"multiplicative",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_MULTIPLICATIVE_CORRECTION)},
		{"second_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_2)},
		{"zeroth_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_0)}
		},
	kw_323[2] = {
		{"folds",9,0,1,0,0,0.,0.,0,N_mom(int,numFolds)},
		{"percent",10,0,1,0,0,0.,0.,0,N_mom(Real,percentFold)}
		},
	kw_324[2] = {
		{"cross_validation",8,2,1,0,kw_323,0.,0.,0,N_mom(true,crossValidateFlag)},
		{"press",8,0,2,0,0,0.,0.,0,N_mom(true,pressFlag)}
		},
	kw_325[2] = {
		{"gradient_threshold",10,0,1,1,0,0.,0.,0,N_mom(Real,discontGradThresh)},
		{"jump_threshold",10,0,1,1,0,0.,0.,0,N_mom(Real,discontJumpThresh)}
		},
	kw_326[3] = {
		{"cell_type",11,0,1,0,0,0.,0.,0,N_mom(str,decompCellType)},
		{"discontinuity_detection",8,2,3,0,kw_325,0.,0.,0,N_mom(true,decompDiscontDetect)},
		{"support_layers",9,0,2,0,0,0.,0.,0,N_mom(int,decompSupportLayers)}
		},
	kw_327[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mom(augment_utype,approxExportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mom(augment_utype,approxExportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mom(augment_utype,approxExportFormat_TABULAR_IFACE_ID)}
		},
	kw_328[3] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_mom(utype,approxExportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_327,0.,0.,0,N_mom(utype,approxExportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mom(utype,approxExportFormat_TABULAR_NONE)}
		},
	kw_329[3] = {
		{"constant",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_constant)},
		{"linear",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_linear)},
		{"reduced_quadratic",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_reduced_quadratic)}
		},
	kw_330[2] = {
		{"point_selection",8,0,1,0,0,0.,0.,0,N_mom(true,pointSelection)},
		{"trend",8,3,2,0,kw_329}
		},
	kw_331[4] = {
		{"constant",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_constant)},
		{"linear",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_linear)},
		{"quadratic",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_quadratic)},
		{"reduced_quadratic",8,0,1,1,0,0.,0.,0,N_mom(lit,trendOrder_reduced_quadratic)}
		},
	kw_332[7] = {
		{"correlation_lengths",14,0,5,0,0,0.,0.,0,N_mom(RealDL,krigingCorrelations)},
		{"export_model_file",11,0,6,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"find_nugget",9,0,4,0,0,0.,0.,0,N_mom(shint,krigingFindNugget)},
		{"max_trials",0x19,0,3,0,0,0.,0.,0,N_mom(shint,krigingMaxTrials)},
		{"nugget",0x1a,0,4,0,0,0.,0.,0,N_mom(Real,krigingNugget)},
		{"optimization_method",11,0,2,0,0,0.,0.,0,N_mom(str,krigingOptMethod)},
		{"trend",8,4,1,0,kw_331}
		},
	kw_333[2] = {
		{"dakota",8,2,1,1,kw_330,0.,0.,0,N_mom(lit,surrogateType_global_gaussian)},
		{"surfpack",8,7,1,1,kw_332,0.,0.,0,N_mom(lit,surrogateType_global_kriging)}
		},
	kw_334[3] = {
		{"eval_id",8,0,2,0,0,0.,0.,0,N_mom(augment_utype,approxImportFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_mom(augment_utype,approxImportFormat_TABULAR_HEADER)},
		{"interface_id",8,0,3,0,0,0.,0.,0,N_mom(augment_utype,approxImportFormat_TABULAR_IFACE_ID)}
		},
	kw_335[4] = {
		{"active_only",8,0,2,0,0,0.,0.,0,N_mdm(true,approxImportActive)},
		{"annotated",8,0,1,0,0,0.,0.,0,N_mom(utype,approxImportFormat_TABULAR_ANNOTATED)},
		{"custom_annotated",8,3,1,0,kw_334,0.,0.,0,N_mom(utype,approxImportFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_mom(utype,approxImportFormat_TABULAR_NONE)}
		},
	kw_336[2] = {
		{"cubic",8,0,1,1,0,0.,0.,0,N_mom(lit,marsInterpolation_cubic)},
		{"linear",8,0,1,1,0,0.,0.,0,N_mom(lit,marsInterpolation_linear)}
		},
	kw_337[3] = {
		{"export_model_file",11,0,3,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"interpolation",8,2,2,0,kw_336},
		{"max_bases",9,0,1,0,0,0.,0.,0,N_mom(shint,marsMaxBases)}
		},
	kw_338[4] = {
		{"basis_order",0x29,0,1,0,0,0.,0.,0,N_mom(shint,polynomialOrder)},
		{"export_model_file",11,0,3,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"poly_order",0x21,0,1,0,0,0.,0.,-2,N_mom(shint,polynomialOrder)},
		{"weight_function",9,0,2,0,0,0.,0.,0,N_mom(shint,mlsWeightFunction)}
		},
	kw_339[5] = {
		{"export_model_file",11,0,4,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"max_nodes",9,0,1,0,0,0.,0.,0,N_mom(shint,annNodes)},
		{"nodes",1,0,1,0,0,0.,0.,-1,N_mom(shint,annNodes)},
		{"random_weight",9,0,3,0,0,0.,0.,0,N_mom(shint,annRandomWeight)},
		{"range",10,0,2,0,0,0.,0.,0,N_mom(Real,annRange)}
		},
	kw_340[5] = {
		{"basis_order",0x29,0,1,1,0,0.,0.,0,N_mom(shint,polynomialOrder)},
		{"cubic",8,0,1,1,0,0.,0.,0,N_mom(order,polynomialOrder_3)},
		{"export_model_file",11,0,2,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"linear",8,0,1,1,0,0.,0.,0,N_mom(order,polynomialOrder_1)},
		{"quadratic",8,0,1,1,0,0.,0.,0,N_mom(order,polynomialOrder_2)}
		},
	kw_341[5] = {
		{"bases",9,0,1,0,0,0.,0.,0,N_mom(shint,rbfBases)},
		{"export_model_file",11,0,5,0,0,0.,0.,0,N_mom(str, approxExportModelFile)},
		{"max_pts",9,0,2,0,0,0.,0.,0,N_mom(shint,rbfMaxPts)},
		{"max_subsets",9,0,4,0,0,0.,0.,0,N_mom(shint,rbfMaxSubsets)},
		{"min_partition",9,0,3,0,0,0.,0.,0,N_mom(shint,rbfMinPartition)}
		},
	kw_342[3] = {
		{"all",8,0,1,1,0,0.,0.,0,N_mom(lit,approxPointReuse_all)},
		{"none",8,0,1,1,0,0.,0.,0,N_mom(lit,approxPointReuse_none)},
		{"region",8,0,1,1,0,0.,0.,0,N_mom(lit,approxPointReuse_region)}
		},
	kw_343[25] = {
		{"actual_model_pointer",11,0,4,0,0,0.,0.,0,N_mom(str,truthModelPointer)},
		{"challenge_points_file",3,4,11,0,kw_321,0.,0.,8,N_mom(str,approxChallengeFile)},
		{"correction",8,6,9,0,kw_322},
		{"dace_method_pointer",11,0,4,0,0,0.,0.,0,N_mom(str,subMethodPointer)},
		{"diagnostics",7,2,10,0,kw_324,0.,0.,9,N_mom(strL,diagMetrics)},
		{"domain_decomposition",8,3,2,0,kw_326,0.,0.,0,N_mom(true,domainDecomp)},
		{"export_points_file",11,3,7,0,kw_328,0.,0.,0,N_mom(str,approxExportFile)},
		{"gaussian_process",8,2,1,1,kw_333},
		{"import_build_points_file",11,4,6,0,kw_335,0.,0.,0,N_mom(str,approxImportFile)},
		{"import_challenge_points_file",11,4,11,0,kw_321,0.,0.,0,N_mom(str,approxChallengeFile)},
		{"import_points_file",3,4,6,0,kw_335,0.,0.,-2,N_mom(str,approxImportFile)},
		{"kriging",0,2,1,1,kw_333,0.,0.,-4},
		{"mars",8,3,1,1,kw_337,0.,0.,0,N_mom(lit,surrogateType_global_mars)},
		{"metrics",15,2,10,0,kw_324,0.,0.,0,N_mom(strL,diagMetrics)},
		{"minimum_points",8,0,3,0,0,0.,0.,0,N_mom(type,pointsManagement_MINIMUM_POINTS)},
		{"moving_least_squares",8,4,1,1,kw_338,0.,0.,0,N_mom(lit,surrogateType_global_moving_least_squares)},
		{"neural_network",8,5,1,1,kw_339,0.,0.,0,N_mom(lit,surrogateType_global_neural_network)},
		{"polynomial",8,5,1,1,kw_340,0.,0.,0,N_mom(lit,surrogateType_global_polynomial)},
		{"radial_basis",8,5,1,1,kw_341,0.,0.,0,N_mom(lit,surrogateType_global_radial_basis)},
		{"recommended_points",8,0,3,0,0,0.,0.,0,N_mom(type,pointsManagement_RECOMMENDED_POINTS)},
		{"reuse_points",8,3,5,0,kw_342},
		{"reuse_samples",0,3,5,0,kw_342,0.,0.,-1},
		{"samples_file",3,4,6,0,kw_335,0.,0.,-14,N_mom(str,approxImportFile)},
		{"total_points",9,0,3,0,0,0.,0.,0,N_mom(int,pointsTotal)},
		{"use_derivatives",8,0,8,0,0,0.,0.,0,N_mom(true,modelUseDerivsFlag)}
		},
	kw_344[6] = {
		{"additive",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_ADDITIVE_CORRECTION)},
		{"combined",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_COMBINED_CORRECTION)},
		{"first_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_1)},
		{"multiplicative",8,0,2,2,0,0.,0.,0,N_mom(type,approxCorrectionType_MULTIPLICATIVE_CORRECTION)},
		{"second_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_2)},
		{"zeroth_order",8,0,1,1,0,0.,0.,0,N_mom(order,approxCorrectionOrder_0)}
		},
	kw_345[3] = {
		{"correction",8,6,3,3,kw_344},
		{"high_fidelity_model_pointer",11,0,2,2,0,0.,0.,0,N_mom(str,truthModelPointer)},
		{"low_fidelity_model_pointer",11,0,1,1,0,0.,0.,0,N_mom(str,lowFidelityModelPointer)}
		},
	kw_346[1] = {
		{"actual_model_pointer",11,0,1,1,0,0.,0.,0,N_mom(str,truthModelPointer)}
		},
	kw_347[2] = {
		{0,0,1,0,0,kw_346},
		{"taylor_series",8,0,1,1}
		},
	kw_348[2] = {
		{0,0,1,0,0,kw_346},
		{"tana",8,0,1,1}
		},
	kw_349[5] = {
		{"global",8,25,2,1,kw_343},
		{"hierarchical",8,3,2,1,kw_345,0.,0.,0,N_mom(lit,surrogateType_hierarchical)},
		{"id_surrogates",13,0,1,0,0,0.,0.,0,N_mom(intsetm1,surrogateFnIndices)},
		{"local",8,1,2,1,kw_347,0.,0.,0,N_mom(lit,surrogateType_local_taylor)},
		{"multipoint",8,1,2,1,kw_348,0.,0.,0,N_mom(lit,surrogateType_multipoint_tana)}
		},
	kw_350[7] = {
		{"hierarchical_tagging",8,0,4,0,0,0.,0.,0,N_mom(true,hierarchicalTags)},
		{"id_model",11,0,1,0,0,0.,0.,0,N_mom(str,idModel)},
		{"nested",8,2,5,1,kw_318,0.,0.,0,N_mom(lit,modelType_nested)},
		{"responses_pointer",11,0,3,0,0,0.,0.,0,N_mom(str,responsesPointer)},
		{"single",8,1,5,1,kw_319,0.,0.,0,N_mom(lit,modelType_single)},
		{"surrogate",8,5,5,1,kw_349,0.,0.,0,N_mom(lit,modelType_surrogate)},
		{"variables_pointer",11,0,2,0,0,0.,0.,0,N_mom(str,variablesPointer)}
		},
	kw_351[2] = {
		{"exp_id",8,0,2,0,0,0.,0.,0,N_rem(augment_utype,scalarDataFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_rem(augment_utype,scalarDataFormat_TABULAR_HEADER)}
		},
	kw_352[3] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_rem(utype,scalarDataFormat_TABULAR_EXPER_ANNOT)},
		{"custom_annotated",8,2,1,0,kw_351,0.,0.,0,N_rem(utype,scalarDataFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_rem(utype,scalarDataFormat_TABULAR_NONE)}
		},
	kw_353[5] = {
		{"interpolate",8,0,5,0,0,0.,0.,0,N_rem(true,interpolateFlag)},
		{"num_config_variables",0x29,0,2,0,0,0.,0.,0,N_rem(sizet,numExpConfigVars)},
		{"num_experiments",0x29,0,1,0,0,0.,0.,0,N_rem(sizet,numExperiments)},
		{"scalar_data_file",11,3,4,0,kw_352,0.,0.,0,N_rem(str,scalarDataFileName)},
		{"variance_type",0x80f,0,3,0,0,0.,0.,0,N_rem(strL,varianceType)}
		},
	kw_354[2] = {
		{"exp_id",8,0,2,0,0,0.,0.,0,N_rem(augment_utype,scalarDataFormat_TABULAR_EVAL_ID)},
		{"header",8,0,1,0,0,0.,0.,0,N_rem(augment_utype,scalarDataFormat_TABULAR_HEADER)}
		},
	kw_355[6] = {
		{"annotated",8,0,1,0,0,0.,0.,0,N_rem(utype,scalarDataFormat_TABULAR_EXPER_ANNOT)},
		{"custom_annotated",8,2,1,0,kw_354,0.,0.,0,N_rem(utype,scalarDataFormat_TABULAR_NONE)},
		{"freeform",8,0,1,0,0,0.,0.,0,N_rem(utype,scalarDataFormat_TABULAR_NONE)},
		{"num_config_variables",0x29,0,3,0,0,0.,0.,0,N_rem(sizet,numExpConfigVars)},
		{"num_experiments",0x29,0,2,0,0,0.,0.,0,N_rem(sizet,numExperiments)},
		{"variance_type",0x80f,0,4,0,0,0.,0.,0,N_rem(strL,varianceType)}
		},
	kw_356[3] = {
		{"lengths",13,0,1,1,0,0.,0.,0,N_rem(ivec,fieldLengths)},
		{"num_coordinates_per_field",13,0,2,0,0,0.,0.,0,N_rem(ivec,numCoordsPerField)},
		{"read_field_coordinates",8,0,3,0,0,0.,0.,0,N_rem(true,readFieldCoords)}
		},
	kw_357[6] = {
		{"nonlinear_equality_scale_types",0x807,0,2,0,0,0.,0.,3,N_rem(strL,nonlinearEqScaleTypes)},
		{"nonlinear_equality_scales",0x806,0,3,0,0,0.,0.,3,N_rem(RealDL,nonlinearEqScales)},
		{"nonlinear_equality_targets",6,0,1,0,0,0.,0.,3,N_rem(RealDL,nonlinearEqTargets)},
		{"scale_types",0x80f,0,2,0,0,0.,0.,0,N_rem(strL,nonlinearEqScaleTypes)},
		{"scales",0x80e,0,3,0,0,0.,0.,0,N_rem(RealDL,nonlinearEqScales)},
		{"targets",14,0,1,0,0,0.,0.,0,N_rem(RealDL,nonlinearEqTargets)}
		},
	kw_358[8] = {
		{"lower_bounds",14,0,1,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqLowerBnds)},
		{"nonlinear_inequality_lower_bounds",6,0,1,0,0,0.,0.,-1,N_rem(RealDL,nonlinearIneqLowerBnds)},
		{"nonlinear_inequality_scale_types",0x807,0,3,0,0,0.,0.,3,N_rem(strL,nonlinearIneqScaleTypes)},
		{"nonlinear_inequality_scales",0x806,0,4,0,0,0.,0.,3,N_rem(RealDL,nonlinearIneqScales)},
		{"nonlinear_inequality_upper_bounds",6,0,2,0,0,0.,0.,3,N_rem(RealDL,nonlinearIneqUpperBnds)},
		{"scale_types",0x80f,0,3,0,0,0.,0.,0,N_rem(strL,nonlinearIneqScaleTypes)},
		{"scales",0x80e,0,4,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqScales)},
		{"upper_bounds",14,0,2,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqUpperBnds)}
		},
	kw_359[18] = {
		{"calibration_data",8,5,6,0,kw_353,0.,0.,0,N_rem(true,calibrationDataFlag)},
		{"calibration_data_file",11,6,6,0,kw_355,0.,0.,0,N_rem(str,scalarDataFileName)},
		{"calibration_term_scale_types",0x807,0,3,0,0,0.,0.,12,N_rem(strL,primaryRespFnScaleTypes)},
		{"calibration_term_scales",0x806,0,4,0,0,0.,0.,12,N_rem(RealDL,primaryRespFnScales)},
		{"calibration_weights",6,0,5,0,0,0.,0.,13,N_rem(RealDL,primaryRespFnWeights)},
		{"field_calibration_terms",0x29,3,2,0,kw_356,0.,0.,0,N_rem(sizet,numFieldLeastSqTerms)},
		{"least_squares_data_file",3,6,6,0,kw_355,0.,0.,-5,N_rem(str,scalarDataFileName)},
		{"least_squares_term_scale_types",0x807,0,3,0,0,0.,0.,7,N_rem(strL,primaryRespFnScaleTypes)},
		{"least_squares_term_scales",0x806,0,4,0,0,0.,0.,7,N_rem(RealDL,primaryRespFnScales)},
		{"least_squares_weights",6,0,5,0,0,0.,0.,8,N_rem(RealDL,primaryRespFnWeights)},
		{"nonlinear_equality_constraints",0x29,6,8,0,kw_357,0.,0.,0,N_rem(sizet,numNonlinearEqConstraints)},
		{"nonlinear_inequality_constraints",0x29,8,7,0,kw_358,0.,0.,0,N_rem(sizet,numNonlinearIneqConstraints)},
		{"num_nonlinear_equality_constraints",0x21,6,8,0,kw_357,0.,0.,-2,N_rem(sizet,numNonlinearEqConstraints)},
		{"num_nonlinear_inequality_constraints",0x21,8,7,0,kw_358,0.,0.,-2,N_rem(sizet,numNonlinearIneqConstraints)},
		{"primary_scale_types",0x80f,0,3,0,0,0.,0.,0,N_rem(strL,primaryRespFnScaleTypes)},
		{"primary_scales",0x80e,0,4,0,0,0.,0.,0,N_rem(RealDL,primaryRespFnScales)},
		{"scalar_calibration_terms",0x29,0,1,0,0,0.,0.,0,N_rem(sizet,numScalarLeastSqTerms)},
		{"weights",14,0,5,0,0,0.,0.,0,N_rem(RealDL,primaryRespFnWeights)}
		},
	kw_360[4] = {
		{"absolute",8,0,2,0,0,0.,0.,0,N_rem(lit,fdGradStepType_absolute)},
		{"bounds",8,0,2,0,0,0.,0.,0,N_rem(lit,fdGradStepType_bounds)},
		{"ignore_bounds",8,0,1,0,0,0.,0.,0,N_rem(true,ignoreBounds)},
		{"relative",8,0,2,0,0,0.,0.,0,N_rem(lit,fdGradStepType_relative)}
		},
	kw_361[8] = {
		{"central",8,0,4,0,0,0.,0.,0,N_rem(lit,intervalType_central)},
		{"dakota",8,4,2,0,kw_360,0.,0.,0,N_rem(lit,methodSource_dakota)},
		{"fd_gradient_step_size",6,0,5,0,0,0.,0.,1,N_rem(RealL,fdGradStepSize)},
		{"fd_step_size",14,0,5,0,0,0.,0.,0,N_rem(RealL,fdGradStepSize)},
		{"forward",8,0,4,0,0,0.,0.,0,N_rem(lit,intervalType_forward)},
		{"interval_type",8,0,3},
		{"method_source",8,0,1},
		{"vendor",8,0,2,0,0,0.,0.,0,N_rem(lit,methodSource_vendor)}
		},
	kw_362[3] = {
		{0,0,8,0,0,kw_361},
		{"id_analytic_gradients",13,0,2,2,0,0.,0.,0,N_rem(intset,idAnalyticGrads)},
		{"id_numerical_gradients",13,0,1,1,0,0.,0.,0,N_rem(intset,idNumericalGrads)}
		},
	kw_363[2] = {
		{"fd_hessian_step_size",6,0,1,0,0,0.,0.,1,N_rem(RealL,fdHessStepSize)},
		{"fd_step_size",14,0,1,0,0,0.,0.,0,N_rem(RealL,fdHessStepSize)}
		},
	kw_364[1] = {
		{"damped",8,0,1,0,0,0.,0.,0,N_rem(lit,quasiHessianType_damped_bfgs)}
		},
	kw_365[2] = {
		{"bfgs",8,1,1,1,kw_364,0.,0.,0,N_rem(lit,quasiHessianType_bfgs)},
		{"sr1",8,0,1,1,0,0.,0.,0,N_rem(lit,quasiHessianType_sr1)}
		},
	kw_366[8] = {
		{"absolute",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_absolute)},
		{"bounds",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_bounds)},
		{"central",8,0,3,0,0,0.,0.,0,N_rem(true,centralHess)},
		{"forward",8,0,3,0,0,0.,0.,0,N_rem(false,centralHess)},
		{"id_analytic_hessians",13,0,5,0,0,0.,0.,0,N_rem(intset,idAnalyticHessians)},
		{"id_numerical_hessians",13,2,1,0,kw_363,0.,0.,0,N_rem(intset,idNumericalHessians)},
		{"id_quasi_hessians",13,2,4,0,kw_365,0.,0.,0,N_rem(intset,idQuasiHessians)},
		{"relative",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_relative)}
		},
	kw_367[3] = {
		{"lengths",13,0,1,1,0,0.,0.,0,N_rem(ivec,fieldLengths)},
		{"num_coordinates_per_field",13,0,2,0,0,0.,0.,0,N_rem(ivec,numCoordsPerField)},
		{"read_field_coordinates",8,0,3,0,0,0.,0.,0,N_rem(true,readFieldCoords)}
		},
	kw_368[6] = {
		{"nonlinear_equality_scale_types",0x807,0,2,0,0,0.,0.,3,N_rem(strL,nonlinearEqScaleTypes)},
		{"nonlinear_equality_scales",0x806,0,3,0,0,0.,0.,3,N_rem(RealDL,nonlinearEqScales)},
		{"nonlinear_equality_targets",6,0,1,0,0,0.,0.,3,N_rem(RealDL,nonlinearEqTargets)},
		{"scale_types",0x80f,0,2,0,0,0.,0.,0,N_rem(strL,nonlinearEqScaleTypes)},
		{"scales",0x80e,0,3,0,0,0.,0.,0,N_rem(RealDL,nonlinearEqScales)},
		{"targets",14,0,1,0,0,0.,0.,0,N_rem(RealDL,nonlinearEqTargets)}
		},
	kw_369[8] = {
		{"lower_bounds",14,0,1,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqLowerBnds)},
		{"nonlinear_inequality_lower_bounds",6,0,1,0,0,0.,0.,-1,N_rem(RealDL,nonlinearIneqLowerBnds)},
		{"nonlinear_inequality_scale_types",0x807,0,3,0,0,0.,0.,3,N_rem(strL,nonlinearIneqScaleTypes)},
		{"nonlinear_inequality_scales",0x806,0,4,0,0,0.,0.,3,N_rem(RealDL,nonlinearIneqScales)},
		{"nonlinear_inequality_upper_bounds",6,0,2,0,0,0.,0.,3,N_rem(RealDL,nonlinearIneqUpperBnds)},
		{"scale_types",0x80f,0,3,0,0,0.,0.,0,N_rem(strL,nonlinearIneqScaleTypes)},
		{"scales",0x80e,0,4,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqScales)},
		{"upper_bounds",14,0,2,0,0,0.,0.,0,N_rem(RealDL,nonlinearIneqUpperBnds)}
		},
	kw_370[15] = {
		{"field_objectives",0x29,3,8,0,kw_367,0.,0.,0,N_rem(sizet,numFieldObjectiveFunctions)},
		{"multi_objective_weights",6,0,4,0,0,0.,0.,13,N_rem(RealDL,primaryRespFnWeights)},
		{"nonlinear_equality_constraints",0x29,6,6,0,kw_368,0.,0.,0,N_rem(sizet,numNonlinearEqConstraints)},
		{"nonlinear_inequality_constraints",0x29,8,5,0,kw_369,0.,0.,0,N_rem(sizet,numNonlinearIneqConstraints)},
		{"num_field_objectives",0x21,3,8,0,kw_367,0.,0.,-4,N_rem(sizet,numFieldObjectiveFunctions)},
		{"num_nonlinear_equality_constraints",0x21,6,6,0,kw_368,0.,0.,-3,N_rem(sizet,numNonlinearEqConstraints)},
		{"num_nonlinear_inequality_constraints",0x21,8,5,0,kw_369,0.,0.,-3,N_rem(sizet,numNonlinearIneqConstraints)},
		{"num_scalar_objectives",0x21,0,7,0,0,0.,0.,5,N_rem(sizet,numScalarObjectiveFunctions)},
		{"objective_function_scale_types",0x807,0,2,0,0,0.,0.,2,N_rem(strL,primaryRespFnScaleTypes)},
		{"objective_function_scales",0x806,0,3,0,0,0.,0.,2,N_rem(RealDL,primaryRespFnScales)},
		{"primary_scale_types",0x80f,0,2,0,0,0.,0.,0,N_rem(strL,primaryRespFnScaleTypes)},
		{"primary_scales",0x80e,0,3,0,0,0.,0.,0,N_rem(RealDL,primaryRespFnScales)},
		{"scalar_objectives",0x29,0,7,0,0,0.,0.,0,N_rem(sizet,numScalarObjectiveFunctions)},
		{"sense",0x80f,0,1,0,0,0.,0.,0,N_rem(strL,primaryRespFnSense)},
		{"weights",14,0,4,0,0,0.,0.,0,N_rem(RealDL,primaryRespFnWeights)}
		},
	kw_371[3] = {
		{"lengths",13,0,1,1,0,0.,0.,0,N_rem(ivec,fieldLengths)},
		{"num_coordinates_per_field",13,0,2,0,0,0.,0.,0,N_rem(ivec,numCoordsPerField)},
		{"read_field_coordinates",8,0,3,0,0,0.,0.,0,N_rem(true,readFieldCoords)}
		},
	kw_372[4] = {
		{"field_responses",0x29,3,2,0,kw_371,0.,0.,0,N_rem(sizet,numFieldResponseFunctions)},
		{"num_field_responses",0x21,3,2,0,kw_371,0.,0.,-1,N_rem(sizet,numFieldResponseFunctions)},
		{"num_scalar_responses",0x21,0,1,0,0,0.,0.,1,N_rem(sizet,numScalarResponseFunctions)},
		{"scalar_responses",0x29,0,1,0,0,0.,0.,0,N_rem(sizet,numScalarResponseFunctions)}
		},
	kw_373[7] = {
		{"absolute",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_absolute)},
		{"bounds",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_bounds)},
		{"central",8,0,3,0,0,0.,0.,0,N_rem(true,centralHess)},
		{"fd_hessian_step_size",6,0,1,0,0,0.,0.,1,N_rem(RealL,fdHessStepSize)},
		{"fd_step_size",14,0,1,0,0,0.,0.,0,N_rem(RealL,fdHessStepSize)},
		{"forward",8,0,3,0,0,0.,0.,0,N_rem(false,centralHess)},
		{"relative",8,0,2,0,0,0.,0.,0,N_rem(lit,fdHessStepType_relative)}
		},
	kw_374[1] = {
		{"damped",8,0,1,0,0,0.,0.,0,N_rem(lit,quasiHessianType_damped_bfgs)}
		},
	kw_375[2] = {
		{"bfgs",8,1,1,1,kw_374,0.,0.,0,N_rem(lit,quasiHessianType_bfgs)},
		{"sr1",8,0,1,1,0,0.,0.,0,N_rem(lit,quasiHessianType_sr1)}
		},
	kw_376[19] = {
		{"analytic_gradients",8,0,4,2,0,0.,0.,0,N_rem(lit,gradientType_analytic)},
		{"analytic_hessians",8,0,5,3,0,0.,0.,0,N_rem(lit,hessianType_analytic)},
		{"calibration_terms",0x29,18,3,1,kw_359,0.,0.,0,N_rem(sizet,numLeastSqTerms)},
		{"descriptors",15,0,2,0,0,0.,0.,0,N_rem(strL,responseLabels)},
		{"id_responses",11,0,1,0,0,0.,0.,0,N_rem(str,idResponses)},
		{"least_squares_terms",0x21,18,3,1,kw_359,0.,0.,-3,N_rem(sizet,numLeastSqTerms)},
		{"mixed_gradients",8,2,4,2,kw_362,0.,0.,0,N_rem(lit,gradientType_mixed)},
		{"mixed_hessians",8,8,5,3,kw_366,0.,0.,0,N_rem(lit,hessianType_mixed)},
		{"no_gradients",8,0,4,2,0,0.,0.,0,N_rem(lit,gradientType_none)},
		{"no_hessians",8,0,5,3,0,0.,0.,0,N_rem(lit,hessianType_none)},
		{"num_least_squares_terms",0x21,18,3,1,kw_359,0.,0.,-8,N_rem(sizet,numLeastSqTerms)},
		{"num_objective_functions",0x21,15,3,1,kw_370,0.,0.,4,N_rem(sizet,numObjectiveFunctions)},
		{"num_response_functions",0x21,4,3,1,kw_372,0.,0.,6,N_rem(sizet,numResponseFunctions)},
		{"numerical_gradients",8,8,4,2,kw_361,0.,0.,0,N_rem(lit,gradientType_numerical)},
		{"numerical_hessians",8,7,5,3,kw_373,0.,0.,0,N_rem(lit,hessianType_numerical)},
		{"objective_functions",0x29,15,3,1,kw_370,0.,0.,0,N_rem(sizet,numObjectiveFunctions)},
		{"quasi_hessians",8,2,5,3,kw_375,0.,0.,0,N_rem(lit,hessianType_quasi)},
		{"response_descriptors",7,0,2,0,0,0.,0.,-14,N_rem(strL,responseLabels)},
		{"response_functions",0x29,4,3,1,kw_372,0.,0.,0,N_rem(sizet,numResponseFunctions)}
		},
	kw_377[6] = {
		{"aleatory",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_ALEATORY_UNCERTAIN_VIEW)},
		{"all",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_ALL_VIEW)},
		{"design",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_DESIGN_VIEW)},
		{"epistemic",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_EPISTEMIC_UNCERTAIN_VIEW)},
		{"state",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_STATE_VIEW)},
		{"uncertain",8,0,1,1,0,0.,0.,0,N_vam(type,varsView_UNCERTAIN_VIEW)}
		},
	kw_378[11] = {
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
	kw_379[5] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(dauilbl,DAUIVar_binomial)},
		{"initial_point",13,0,3,0,0,0.,0.,0,N_vam(IntLb,binomialUncVars)},
		{"num_trials",13,0,2,2,0,0.,0.,0,N_vam(IntLb,binomialUncNumTrials)},
		{"prob_per_trial",6,0,1,1,0,0.,0.,1,N_vam(rvec,binomialUncProbPerTrial)},
		{"probability_per_trial",14,0,1,1,0,0.,0.,0,N_vam(rvec,binomialUncProbPerTrial)}
		},
	kw_380[12] = {
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
	kw_381[10] = {
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
	kw_382[8] = {
		{"csv_descriptors",7,0,4,0,0,0.,0.,4,N_vam(strL,continuousStateLabels)},
		{"csv_initial_state",6,0,1,0,0,0.,0.,4,N_vam(rvec,continuousStateVars)},
		{"csv_lower_bounds",6,0,2,0,0,0.,0.,4,N_vam(rvec,continuousStateLowerBnds)},
		{"csv_upper_bounds",6,0,3,0,0,0.,0.,4,N_vam(rvec,continuousStateUpperBnds)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,continuousStateLabels)},
		{"initial_state",14,0,1,0,0,0.,0.,0,N_vam(rvec,continuousStateVars)},
		{"lower_bounds",14,0,2,0,0,0.,0.,0,N_vam(rvec,continuousStateLowerBnds)},
		{"upper_bounds",14,0,3,0,0,0.,0.,0,N_vam(rvec,continuousStateUpperBnds)}
		},
	kw_383[8] = {
		{"ddv_descriptors",7,0,4,0,0,0.,0.,4,N_vam(strL,discreteDesignRangeLabels)},
		{"ddv_initial_point",5,0,1,0,0,0.,0.,4,N_vam(ivec,discreteDesignRangeVars)},
		{"ddv_lower_bounds",5,0,2,0,0,0.,0.,4,N_vam(ivec,discreteDesignRangeLowerBnds)},
		{"ddv_upper_bounds",5,0,3,0,0,0.,0.,4,N_vam(ivec,discreteDesignRangeUpperBnds)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteDesignRangeLabels)},
		{"initial_point",13,0,1,0,0,0.,0.,0,N_vam(ivec,discreteDesignRangeVars)},
		{"lower_bounds",13,0,2,0,0,0.,0.,0,N_vam(ivec,discreteDesignRangeLowerBnds)},
		{"upper_bounds",13,0,3,0,0,0.,0.,0,N_vam(ivec,discreteDesignRangeUpperBnds)}
		},
	kw_384[1] = {
		{"adjacency_matrix",13,0,1,0,0,0.,0.,0,N_vam(newivec,Var_Info_ddsia)}
		},
	kw_385[7] = {
		{"categorical",15,1,3,0,kw_384,0.,0.,0,N_vam(categorical,discreteDesignSetIntCat)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vam(strL,discreteDesignSetIntLabels)},
		{"elements",13,0,2,1,0,0.,0.,0,N_vam(newivec,Var_Info_ddsi)},
		{"elements_per_variable",0x80d,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nddsi)},
		{"initial_point",13,0,4,0,0,0.,0.,0,N_vam(ivec,discreteDesignSetIntVars)},
		{"num_set_values",0x805,0,1,0,0,0.,0.,-2,N_vam(newiarray,Var_Info_nddsi)},
		{"set_values",5,0,2,1,0,0.,0.,-4,N_vam(newivec,Var_Info_ddsi)}
		},
	kw_386[1] = {
		{"adjacency_matrix",13,0,1,0,0,0.,0.,0,N_vam(newivec,Var_Info_ddsra)}
		},
	kw_387[7] = {
		{"categorical",15,1,3,0,kw_386,0.,0.,0,N_vam(categorical,discreteDesignSetRealCat)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vam(strL,discreteDesignSetRealLabels)},
		{"elements",14,0,2,1,0,0.,0.,0,N_vam(newrvec,Var_Info_ddsr)},
		{"elements_per_variable",0x80d,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nddsr)},
		{"initial_point",14,0,4,0,0,0.,0.,0,N_vam(rvec,discreteDesignSetRealVars)},
		{"num_set_values",0x805,0,1,0,0,0.,0.,-2,N_vam(newiarray,Var_Info_nddsr)},
		{"set_values",6,0,2,1,0,0.,0.,-4,N_vam(newrvec,Var_Info_ddsr)}
		},
	kw_388[7] = {
		{"adjacency_matrix",13,0,3,0,0,0.,0.,0,N_vam(newivec,Var_Info_ddssa)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vam(strL,discreteDesignSetStrLabels)},
		{"elements",15,0,2,1,0,0.,0.,0,N_vam(newsarray,Var_Info_ddss)},
		{"elements_per_variable",0x80d,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nddss)},
		{"initial_point",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteDesignSetStrVars)},
		{"num_set_values",0x805,0,1,0,0,0.,0.,-2,N_vam(newiarray,Var_Info_nddss)},
		{"set_values",7,0,2,1,0,0.,0.,-4,N_vam(newsarray,Var_Info_ddss)}
		},
	kw_389[3] = {
		{"integer",0x19,7,1,0,kw_385,0.,0.,0,N_vam(pintz,numDiscreteDesSetIntVars)},
		{"real",0x19,7,3,0,kw_387,0.,0.,0,N_vam(pintz,numDiscreteDesSetRealVars)},
		{"string",0x19,7,2,0,kw_388,0.,0.,0,N_vam(pintz,numDiscreteDesSetStrVars)}
		},
	kw_390[9] = {
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
	kw_391[8] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteStateRangeLabels)},
		{"dsv_descriptors",7,0,4,0,0,0.,0.,-1,N_vam(strL,discreteStateRangeLabels)},
		{"dsv_initial_state",5,0,1,0,0,0.,0.,3,N_vam(ivec,discreteStateRangeVars)},
		{"dsv_lower_bounds",5,0,2,0,0,0.,0.,3,N_vam(ivec,discreteStateRangeLowerBnds)},
		{"dsv_upper_bounds",5,0,3,0,0,0.,0.,3,N_vam(ivec,discreteStateRangeUpperBnds)},
		{"initial_state",13,0,1,0,0,0.,0.,0,N_vam(ivec,discreteStateRangeVars)},
		{"lower_bounds",13,0,2,0,0,0.,0.,0,N_vam(ivec,discreteStateRangeLowerBnds)},
		{"upper_bounds",13,0,3,0,0,0.,0.,0,N_vam(ivec,discreteStateRangeUpperBnds)}
		},
	kw_392[7] = {
		{"categorical",15,0,3,0,0,0.,0.,0,N_vam(categorical,discreteStateSetIntCat)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vam(strL,discreteStateSetIntLabels)},
		{"elements",13,0,2,1,0,0.,0.,0,N_vam(newivec,Var_Info_dssi)},
		{"elements_per_variable",0x80d,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_ndssi)},
		{"initial_state",13,0,4,0,0,0.,0.,0,N_vam(ivec,discreteStateSetIntVars)},
		{"num_set_values",0x805,0,1,0,0,0.,0.,-2,N_vam(newiarray,Var_Info_ndssi)},
		{"set_values",5,0,2,1,0,0.,0.,-4,N_vam(newivec,Var_Info_dssi)}
		},
	kw_393[7] = {
		{"categorical",15,0,3,0,0,0.,0.,0,N_vam(categorical,discreteStateSetRealCat)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vam(strL,discreteStateSetRealLabels)},
		{"elements",14,0,2,1,0,0.,0.,0,N_vam(newrvec,Var_Info_dssr)},
		{"elements_per_variable",0x80d,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_ndssr)},
		{"initial_state",14,0,4,0,0,0.,0.,0,N_vam(rvec,discreteStateSetRealVars)},
		{"num_set_values",0x805,0,1,0,0,0.,0.,-2,N_vam(newiarray,Var_Info_ndssr)},
		{"set_values",6,0,2,1,0,0.,0.,-4,N_vam(newrvec,Var_Info_dssr)}
		},
	kw_394[6] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteStateSetStrLabels)},
		{"elements",15,0,2,1,0,0.,0.,0,N_vam(newsarray,Var_Info_dsss)},
		{"elements_per_variable",0x80d,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_ndsss)},
		{"initial_state",15,0,3,0,0,0.,0.,0,N_vam(strL,discreteStateSetStrVars)},
		{"num_set_values",0x805,0,1,0,0,0.,0.,-2,N_vam(newiarray,Var_Info_ndsss)},
		{"set_values",7,0,2,1,0,0.,0.,-4,N_vam(newsarray,Var_Info_dsss)}
		},
	kw_395[3] = {
		{"integer",0x19,7,1,0,kw_392,0.,0.,0,N_vam(pintz,numDiscreteStateSetIntVars)},
		{"real",0x19,7,3,0,kw_393,0.,0.,0,N_vam(pintz,numDiscreteStateSetRealVars)},
		{"string",0x19,6,2,0,kw_394,0.,0.,0,N_vam(pintz,numDiscreteStateSetStrVars)}
		},
	kw_396[9] = {
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
	kw_397[9] = {
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
	kw_398[8] = {
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(deuslbl,DEUSVar_set_str)},
		{"elements",15,0,2,1,0,0.,0.,0,N_vam(newsarray,Var_Info_duss)},
		{"elements_per_variable",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nduss)},
		{"initial_point",15,0,4,0,0,0.,0.,0,N_vam(strL,discreteUncSetStrVars)},
		{"num_set_values",5,0,1,0,0,0.,0.,-2,N_vam(newiarray,Var_Info_nduss)},
		{"set_probabilities",14,0,3,0,0,0.,0.,0,N_vam(newrvec,Var_Info_DSSp)},
		{"set_probs",6,0,3,0,0,0.,0.,-1,N_vam(newrvec,Var_Info_DSSp)},
		{"set_values",7,0,2,1,0,0.,0.,-6,N_vam(newsarray,Var_Info_duss)}
		},
	kw_399[3] = {
		{"integer",0x19,9,1,0,kw_396,0.,0.,0,N_vam(pintz,numDiscreteUncSetIntVars)},
		{"real",0x19,9,3,0,kw_397,0.,0.,0,N_vam(pintz,numDiscreteUncSetRealVars)},
		{"string",0x19,8,2,0,kw_398,0.,0.,0,N_vam(pintz,numDiscreteUncSetStrVars)}
		},
	kw_400[5] = {
		{"betas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,exponentialUncBetas)},
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(caulbl,CAUVar_exponential)},
		{"euv_betas",6,0,1,1,0,0.,0.,-2,N_vam(RealLb,exponentialUncBetas)},
		{"euv_descriptors",7,0,3,0,0,0.,0.,-2,N_vae(caulbl,CAUVar_exponential)},
		{"initial_point",14,0,2,0,0,0.,0.,0,N_vam(RealLb,exponentialUncVars)}
		},
	kw_401[7] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,frechetUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(rvec,frechetUncBetas)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_frechet)},
		{"fuv_alphas",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,frechetUncAlphas)},
		{"fuv_betas",6,0,2,2,0,0.,0.,-3,N_vam(rvec,frechetUncBetas)},
		{"fuv_descriptors",7,0,4,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_frechet)},
		{"initial_point",14,0,3,0,0,0.,0.,0,N_vam(rvec,frechetUncVars)}
		},
	kw_402[7] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,gammaUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(RealLb,gammaUncBetas)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_gamma)},
		{"gauv_alphas",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,gammaUncAlphas)},
		{"gauv_betas",6,0,2,2,0,0.,0.,-3,N_vam(RealLb,gammaUncBetas)},
		{"gauv_descriptors",7,0,4,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_gamma)},
		{"initial_point",14,0,3,0,0,0.,0.,0,N_vam(RealLb,gammaUncVars)}
		},
	kw_403[4] = {
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(dauilbl,DAUIVar_geometric)},
		{"initial_point",13,0,2,0,0,0.,0.,0,N_vam(IntLb,geometricUncVars)},
		{"prob_per_trial",6,0,1,1,0,0.,0.,1,N_vam(rvec,geometricUncProbPerTrial)},
		{"probability_per_trial",14,0,1,1,0,0.,0.,0,N_vam(rvec,geometricUncProbPerTrial)}
		},
	kw_404[7] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,gumbelUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(rvec,gumbelUncBetas)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_gumbel)},
		{"guuv_alphas",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,gumbelUncAlphas)},
		{"guuv_betas",6,0,2,2,0,0.,0.,-3,N_vam(rvec,gumbelUncBetas)},
		{"guuv_descriptors",7,0,4,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_gumbel)},
		{"initial_point",14,0,3,0,0,0.,0.,0,N_vam(rvec,gumbelUncVars)}
		},
	kw_405[11] = {
		{"abscissas",14,0,2,1,0,0.,0.,0,N_vam(newrvec,Var_Info_hba)},
		{"counts",14,0,3,2,0,0.,0.,0,N_vam(newrvec,Var_Info_hbc)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(caulbl,CAUVar_histogram_bin)},
		{"huv_bin_abscissas",6,0,2,1,0,0.,0.,-3,N_vam(newrvec,Var_Info_hba)},
		{"huv_bin_counts",6,0,3,2,0,0.,0.,-3,N_vam(newrvec,Var_Info_hbc)},
		{"huv_bin_descriptors",7,0,5,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_histogram_bin)},
		{"huv_bin_ordinates",6,0,3,2,0,0.,0.,3,N_vam(newrvec,Var_Info_hbo)},
		{"initial_point",14,0,4,0,0,0.,0.,0,N_vam(rvec,histogramBinUncVars)},
		{"num_pairs",5,0,1,0,0,0.,0.,2,N_vam(newiarray,Var_Info_nhbp)},
		{"ordinates",14,0,3,2,0,0.,0.,0,N_vam(newrvec,Var_Info_hbo)},
		{"pairs_per_variable",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nhbp)}
		},
	kw_406[6] = {
		{"abscissas",13,0,2,1,0,0.,0.,0,N_vam(newivec,Var_Info_hpia)},
		{"counts",14,0,3,2,0,0.,0.,0,N_vam(newrvec,Var_Info_hpic)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(dauilbl,DAUIVar_histogram_point_int)},
		{"initial_point",13,0,4,0,0,0.,0.,0,N_vam(ivec,histogramPointIntUncVars)},
		{"num_pairs",5,0,1,0,0,0.,0.,1,N_vam(newiarray,Var_Info_nhpip)},
		{"pairs_per_variable",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nhpip)}
		},
	kw_407[6] = {
		{"abscissas",14,0,2,1,0,0.,0.,0,N_vam(newrvec,Var_Info_hpra)},
		{"counts",14,0,3,2,0,0.,0.,0,N_vam(newrvec,Var_Info_hprc)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(daurlbl,DAURVar_histogram_point_real)},
		{"initial_point",14,0,4,0,0,0.,0.,0,N_vam(rvec,histogramPointRealUncVars)},
		{"num_pairs",5,0,1,0,0,0.,0.,1,N_vam(newiarray,Var_Info_nhprp)},
		{"pairs_per_variable",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nhprp)}
		},
	kw_408[6] = {
		{"abscissas",15,0,2,1,0,0.,0.,0,N_vam(newsarray,Var_Info_hpsa)},
		{"counts",14,0,3,2,0,0.,0.,0,N_vam(newrvec,Var_Info_hpsc)},
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(dauslbl,DAUSVar_histogram_point_str)},
		{"initial_point",15,0,4,0,0,0.,0.,0,N_vam(strL,histogramPointStrUncVars)},
		{"num_pairs",5,0,1,0,0,0.,0.,1,N_vam(newiarray,Var_Info_nhpsp)},
		{"pairs_per_variable",13,0,1,0,0,0.,0.,0,N_vam(newiarray,Var_Info_nhpsp)}
		},
	kw_409[3] = {
		{"integer",0x19,6,1,0,kw_406,0.,0.,0,N_vam(pintz,numHistogramPtIntUncVars)},
		{"real",0x19,6,3,0,kw_407,0.,0.,0,N_vam(pintz,numHistogramPtRealUncVars)},
		{"string",0x19,6,2,0,kw_408,0.,0.,0,N_vam(pintz,numHistogramPtStrUncVars)}
		},
	kw_410[5] = {
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(dauilbl,DAUIVar_hypergeometric)},
		{"initial_point",13,0,4,0,0,0.,0.,0,N_vam(IntLb,hyperGeomUncVars)},
		{"num_drawn",13,0,3,3,0,0.,0.,0,N_vam(IntLb,hyperGeomUncNumDrawn)},
		{"selected_population",13,0,2,2,0,0.,0.,0,N_vam(IntLb,hyperGeomUncSelectedPop)},
		{"total_population",13,0,1,1,0,0.,0.,0,N_vam(IntLb,hyperGeomUncTotalPop)}
		},
	kw_411[2] = {
		{"lnuv_zetas",6,0,1,1,0,0.,0.,1,N_vam(RealLb,lognormalUncZetas)},
		{"zetas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,lognormalUncZetas)}
		},
	kw_412[4] = {
		{"error_factors",14,0,1,1,0,0.,0.,0,N_vam(RealLb,lognormalUncErrFacts)},
		{"lnuv_error_factors",6,0,1,1,0,0.,0.,-1,N_vam(RealLb,lognormalUncErrFacts)},
		{"lnuv_std_deviations",6,0,1,1,0,0.,0.,1,N_vam(RealLb,lognormalUncStdDevs)},
		{"std_deviations",14,0,1,1,0,0.,0.,0,N_vam(RealLb,lognormalUncStdDevs)}
		},
	kw_413[11] = {
		{"descriptors",15,0,5,0,0,0.,0.,0,N_vae(caulbl,CAUVar_lognormal)},
		{"initial_point",14,0,4,0,0,0.,0.,0,N_vam(RealLb,lognormalUncVars)},
		{"lambdas",14,2,1,1,kw_411,0.,0.,0,N_vam(rvec,lognormalUncLambdas)},
		{"lnuv_descriptors",7,0,5,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_lognormal)},
		{"lnuv_lambdas",6,2,1,1,kw_411,0.,0.,-2,N_vam(rvec,lognormalUncLambdas)},
		{"lnuv_lower_bounds",6,0,2,0,0,0.,0.,3,N_vam(RealLb,lognormalUncLowerBnds)},
		{"lnuv_means",6,4,1,1,kw_412,0.,0.,3,N_vam(RealLb,lognormalUncMeans)},
		{"lnuv_upper_bounds",6,0,3,0,0,0.,0.,3,N_vam(RealUb,lognormalUncUpperBnds)},
		{"lower_bounds",14,0,2,0,0,0.,0.,0,N_vam(RealLb,lognormalUncLowerBnds)},
		{"means",14,4,1,1,kw_412,0.,0.,0,N_vam(RealLb,lognormalUncMeans)},
		{"upper_bounds",14,0,3,0,0,0.,0.,0,N_vam(RealUb,lognormalUncUpperBnds)}
		},
	kw_414[7] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_loguniform)},
		{"initial_point",14,0,3,0,0,0.,0.,0,N_vam(RealLb,loguniformUncVars)},
		{"lower_bounds",14,0,1,1,0,0.,0.,0,N_vam(RealLb,loguniformUncLowerBnds)},
		{"luuv_descriptors",7,0,4,0,0,0.,0.,-3,N_vae(caulbl,CAUVar_loguniform)},
		{"luuv_lower_bounds",6,0,1,1,0,0.,0.,-2,N_vam(RealLb,loguniformUncLowerBnds)},
		{"luuv_upper_bounds",6,0,2,2,0,0.,0.,1,N_vam(RealUb,loguniformUncUpperBnds)},
		{"upper_bounds",14,0,2,2,0,0.,0.,0,N_vam(RealUb,loguniformUncUpperBnds)}
		},
	kw_415[5] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(dauilbl,DAUIVar_negative_binomial)},
		{"initial_point",13,0,3,0,0,0.,0.,0,N_vam(IntLb,negBinomialUncVars)},
		{"num_trials",13,0,2,2,0,0.,0.,0,N_vam(IntLb,negBinomialUncNumTrials)},
		{"prob_per_trial",6,0,1,1,0,0.,0.,1,N_vam(rvec,negBinomialUncProbPerTrial)},
		{"probability_per_trial",14,0,1,1,0,0.,0.,0,N_vam(rvec,negBinomialUncProbPerTrial)}
		},
	kw_416[11] = {
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
	kw_417[3] = {
		{"descriptors",15,0,3,0,0,0.,0.,0,N_vae(dauilbl,DAUIVar_poisson)},
		{"initial_point",13,0,2,0,0,0.,0.,0,N_vam(IntLb,poissonUncVars)},
		{"lambdas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,poissonUncLambdas)}
		},
	kw_418[9] = {
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
	kw_419[7] = {
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_uniform)},
		{"initial_point",14,0,3,0,0,0.,0.,0,N_vam(rvec,uniformUncVars)},
		{"lower_bounds",14,0,1,1,0,0.,0.,0,N_vam(RealLb,uniformUncLowerBnds)},
		{"upper_bounds",14,0,2,2,0,0.,0.,0,N_vam(RealUb,uniformUncUpperBnds)},
		{"uuv_descriptors",7,0,4,0,0,0.,0.,-4,N_vae(caulbl,CAUVar_uniform)},
		{"uuv_lower_bounds",6,0,1,1,0,0.,0.,-3,N_vam(RealLb,uniformUncLowerBnds)},
		{"uuv_upper_bounds",6,0,2,2,0,0.,0.,-3,N_vam(RealUb,uniformUncUpperBnds)}
		},
	kw_420[7] = {
		{"alphas",14,0,1,1,0,0.,0.,0,N_vam(RealLb,weibullUncAlphas)},
		{"betas",14,0,2,2,0,0.,0.,0,N_vam(RealLb,weibullUncBetas)},
		{"descriptors",15,0,4,0,0,0.,0.,0,N_vae(caulbl,CAUVar_weibull)},
		{"initial_point",14,0,3,0,0,0.,0.,0,N_vam(RealLb,weibullUncVars)},
		{"wuv_alphas",6,0,1,1,0,0.,0.,-4,N_vam(RealLb,weibullUncAlphas)},
		{"wuv_betas",6,0,2,2,0,0.,0.,-4,N_vam(RealLb,weibullUncBetas)},
		{"wuv_descriptors",7,0,4,0,0,0.,0.,-4,N_vae(caulbl,CAUVar_weibull)}
		},
	kw_421[34] = {
		{"active",8,6,2,0,kw_377},
		{"beta_uncertain",0x19,11,13,0,kw_378,0.,0.,0,N_vam(pintz,numBetaUncVars)},
		{"binomial_uncertain",0x19,5,20,0,kw_379,0.,0.,0,N_vam(pintz,numBinomialUncVars)},
		{"continuous_design",0x19,12,4,0,kw_380,0.,0.,0,N_vam(pintz,numContinuousDesVars)},
		{"continuous_interval_uncertain",0x19,10,26,0,kw_381,0.,0.,0,N_vam(pintz,numContinuousIntervalUncVars)},
		{"continuous_state",0x19,8,29,0,kw_382,0.,0.,0,N_vam(pintz,numContinuousStateVars)},
		{"discrete_design_range",0x19,8,5,0,kw_383,0.,0.,0,N_vam(pintz,numDiscreteDesRangeVars)},
		{"discrete_design_set",8,3,6,0,kw_389},
		{"discrete_interval_uncertain",0x19,9,27,0,kw_390,0.,0.,0,N_vam(pintz,numDiscreteIntervalUncVars)},
		{"discrete_state_range",0x19,8,30,0,kw_391,0.,0.,0,N_vam(pintz,numDiscreteStateRangeVars)},
		{"discrete_state_set",8,3,31,0,kw_395},
		{"discrete_uncertain_range",0x11,9,27,0,kw_390,0.,0.,-3,N_vam(pintz,numDiscreteIntervalUncVars)},
		{"discrete_uncertain_set",8,3,28,0,kw_399},
		{"exponential_uncertain",0x19,5,12,0,kw_400,0.,0.,0,N_vam(pintz,numExponentialUncVars)},
		{"frechet_uncertain",0x19,7,16,0,kw_401,0.,0.,0,N_vam(pintz,numFrechetUncVars)},
		{"gamma_uncertain",0x19,7,14,0,kw_402,0.,0.,0,N_vam(pintz,numGammaUncVars)},
		{"geometric_uncertain",0x19,4,22,0,kw_403,0.,0.,0,N_vam(pintz,numGeometricUncVars)},
		{"gumbel_uncertain",0x19,7,15,0,kw_404,0.,0.,0,N_vam(pintz,numGumbelUncVars)},
		{"histogram_bin_uncertain",0x19,11,18,0,kw_405,0.,0.,0,N_vam(pintz,numHistogramBinUncVars)},
		{"histogram_point_uncertain",8,3,24,0,kw_409},
		{"hypergeometric_uncertain",0x19,5,23,0,kw_410,0.,0.,0,N_vam(pintz,numHyperGeomUncVars)},
		{"id_variables",11,0,1,0,0,0.,0.,0,N_vam(str,idVariables)},
		{"interval_uncertain",0x11,10,26,0,kw_381,0.,0.,-18,N_vam(pintz,numContinuousIntervalUncVars)},
		{"lognormal_uncertain",0x19,11,8,0,kw_413,0.,0.,0,N_vam(pintz,numLognormalUncVars)},
		{"loguniform_uncertain",0x19,7,10,0,kw_414,0.,0.,0,N_vam(pintz,numLoguniformUncVars)},
		{"mixed",8,0,3,0,0,0.,0.,0,N_vam(type,varsDomain_MIXED_DOMAIN)},
		{"negative_binomial_uncertain",0x19,5,21,0,kw_415,0.,0.,0,N_vam(pintz,numNegBinomialUncVars)},
		{"normal_uncertain",0x19,11,7,0,kw_416,0.,0.,0,N_vam(pintz,numNormalUncVars)},
		{"poisson_uncertain",0x19,3,19,0,kw_417,0.,0.,0,N_vam(pintz,numPoissonUncVars)},
		{"relaxed",8,0,3,0,0,0.,0.,0,N_vam(type,varsDomain_RELAXED_DOMAIN)},
		{"triangular_uncertain",0x19,9,11,0,kw_418,0.,0.,0,N_vam(pintz,numTriangularUncVars)},
		{"uncertain_correlation_matrix",14,0,25,0,0,0.,0.,0,N_vam(newrvec,Var_Info_ucm)},
		{"uniform_uncertain",0x19,7,9,0,kw_419,0.,0.,0,N_vam(pintz,numUniformUncVars)},
		{"weibull_uncertain",0x19,7,17,0,kw_420,0.,0.,0,N_vam(pintz,numWeibullUncVars)}
		},
	kw_422[6] = {
		{"environment",0x108,15,1,1,kw_12,0.,0.,0,NIDRProblemDescDB::env_start},
		{"interface",0x308,9,5,5,kw_26,0.,0.,0,N_ifm3(start,0,stop)},
		{"method",0x308,91,2,2,kw_314,0.,0.,0,N_mdm3(start,0,stop)},
		{"model",8,7,3,3,kw_350,0.,0.,0,N_mom3(start,0,stop)},
		{"responses",0x308,19,6,6,kw_376,0.,0.,0,N_rem3(start,0,stop)},
		{"variables",0x308,34,4,4,kw_421,0.,0.,0,N_vam3(start,0,stop)}
		};

} // namespace Dakota

#ifdef __cplusplus
extern "C" {
#endif
KeyWord Dakota_Keyword_Top = {"KeywordTop",0,6,0,0,Dakota::kw_422};
#ifdef __cplusplus
}
#endif

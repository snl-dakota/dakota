#include "nidr.h"

#include "jega.inc"

/** 60 distinct keywords (plus 1 aliases) **/

static KeyWordx
	kw_1[2] = {
		{{"num_offspring",0x20019,0,2,0,0,0.,0.,0,N_mdm(pintz,numOffspring)},51,99,"N_mdm(pintz,numOffspring)","{Number of offspring in random shuffle crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{{"num_parents",0x20019,0,1,0,0,0.,0.,0,N_mdm(pintz,numParents)},50,97,"N_mdm(pintz,numParents)","{Number of parents in random shuffle crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_2[5] = {
		{{"crossover_rate",0x2000a,0,2,0,0,0.,0.,0,N_mdm(litz,3crossoverType_null_crossover)},52,101,"N_mdm(litz,3crossoverType_null_crossover)","{Crossover rate} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{{"multi_point_binary",0x20009,0,1,1,0,0.,0.,0,N_mdm(ilit2p,3crossoverType_multi_point_binary)},46,89,"N_mdm(ilit2p,3crossoverType_multi_point_binary)","{Multi point binary crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{{"multi_point_parameterized_binary",0x20009,0,1,1,0,0.,0.,0,N_mdm(ilit2p,3crossoverType_multi_point_parameterized_binary)},47,91,"N_mdm(ilit2p,3crossoverType_multi_point_parameterized_binary)","{Multi point parameterized binary crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{{"multi_point_real",0x20009,0,1,1,0,0.,0.,0,N_mdm(ilit2p,3crossoverType_multi_point_real)},48,93,"N_mdm(ilit2p,3crossoverType_multi_point_real)","{Multi point real crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{{"shuffle_random",0x20008,2,1,1,(KeyWord*)kw_1,0.,0.,0,N_mdm(litc,3crossoverType_shuffle_random)},49,95,"N_mdm(litc,3crossoverType_shuffle_random)","{Random shuffle crossover} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_3[3] = {
		{{"flat_file",0x2000b,0,1,1,0,0.,0.,0,N_mdm(slit2,3initializationType_flat_file)},44,85,"N_mdm(slit2,3initializationType_flat_file)"},
		{{"simple_random",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,initializationType_random)},42,81,"N_mdm(lit,initializationType_random)"},
		{{"unique_random",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,initializationType_unique_random)},43,83,"N_mdm(lit,initializationType_unique_random)"}
		},
	kw_4[3] = {
		{{"metric_tracker",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,convergenceType_metric_tracker)},17,31,"N_mdm(lit,convergenceType_metric_tracker)","{Convergence type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{{"num_generations",0x20029,0,3,0,0,0.,0.,0,N_mdm(nnintz,numGenerations)},19,35,"N_mdm(nnintz,numGenerations)","{Number generations for metric_tracker converger} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{{"percent_change",0x2000a,0,2,0,0,0.,0.,0,N_mdm(Realz,convergenceTolerance)},18,33,"N_mdm(Realz,convergenceTolerance)","{Percent change limit for metric_tracker converger} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"}
		},
	kw_5[2] = {
		{{"domination_count",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_domination_count)},5,9,"N_mdm(lit,fitnessType_domination_count)"},
		{{"layer_rank",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_layer_rank)},4,7,"N_mdm(lit,fitnessType_layer_rank)"}
		},
	kw_6[2] = {
		{{"distance",0x2000e,0,1,1,0,0.,0.,0,N_mdm(RealLlit,3nichingType_distance)},15,27,"N_mdm(RealLlit,3nichingType_distance)"},
		{{"radial",0x2000e,0,1,1,0,0.,0.,0,N_mdm(RealLlit,3nichingType_radial)},14,25,"N_mdm(RealLlit,3nichingType_radial)"}
		},
	kw_7[1] = {
		{{"orthogonal_distance",0x2000e,0,1,1,0,0.,0.,0,N_mdm(RealLlit,3postProcessorType_distance_postprocessor)},21,39,"N_mdm(RealLlit,3postProcessorType_distance_postprocessor)","{Post_processor distance} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"}
		},
	kw_8[2] = {
		{{"shrinkage_fraction",0x2000a,0,1,0,0,0.,0.,0,N_mdm(Real01,shrinkagePercent)},11,21,"N_mdm(Real01,shrinkagePercent)"},
		{{"shrinkage_percentage",0x20002,0,1,0,0,0.,0.,-1,N_mdm(Real01,shrinkagePercent)},12,20,"N_mdm(Real01,shrinkagePercent)"}
		},
	kw_9[4] = {
		{{"below_limit",0x2000a,2,1,1,(KeyWord*)kw_8,0.,0.,0,N_mdm(litp,3replacementType_below_limit)},10,19,"N_mdm(litp,3replacementType_below_limit)","{Below limit selection} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{{"elitist",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_elitist)},7,13,"N_mdm(lit,replacementType_elitist)"},
		{{"roulette_wheel",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_roulette_wheel)},8,15,"N_mdm(lit,replacementType_roulette_wheel)"},
		{{"unique_roulette_wheel",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_unique_roulette_wheel)},9,17,"N_mdm(lit,replacementType_unique_roulette_wheel)"}
		},
	kw_10[5] = {
		{{"convergence_type",0x20008,3,4,0,(KeyWord*)kw_4},16,29,0,"{Convergence type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{{"fitness_type",0x20008,2,1,0,(KeyWord*)kw_5},3,5,0,"{Fitness type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{{"niching_type",0x20008,2,3,0,(KeyWord*)kw_6},13,23,0,"{Niche pressure type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{{"postprocessor_type",0x20008,1,5,0,(KeyWord*)kw_7},20,37,0,"{Post_processor type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGAMOGA"},
		{{"replacement_type",0x20008,4,2,0,(KeyWord*)kw_9},6,11,0,"{Replacement type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_11[1] = {
		{{"mutation_scale",0x2000a,0,1,0,0,0.,0.,0,N_mdm(Real01,mutationScale)},59,115,"N_mdm(Real01,mutationScale)","{Mutation scale} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"}
		},
	kw_12[6] = {
		{{"bit_random",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,mutationType_bit_random)},54,105,"N_mdm(lit,mutationType_bit_random)"},
		{{"mutation_rate",0x2000a,0,2,0,0,0.,0.,0,N_mdm(litz,3mutationType_null_mutation)},60,117,"N_mdm(litz,3mutationType_null_mutation)","{Mutation rate} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{{"offset_cauchy",0x20008,1,1,1,(KeyWord*)kw_11,0.,0.,0,N_mdm(litc,3mutationType_offset_cauchy)},57,111,"N_mdm(litc,3mutationType_offset_cauchy)"},
		{{"offset_normal",0x20008,1,1,1,(KeyWord*)kw_11,0.,0.,0,N_mdm(litc,3mutationType_offset_normal)},56,109,"N_mdm(litc,3mutationType_offset_normal)"},
		{{"offset_uniform",0x20008,1,1,1,(KeyWord*)kw_11,0.,0.,0,N_mdm(litc,3mutationType_offset_uniform)},58,113,"N_mdm(litc,3mutationType_offset_uniform)"},
		{{"replace_uniform",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,mutationType_replace_uniform)},55,107,"N_mdm(lit,mutationType_replace_uniform)"}
		},
	kw_13[2] = {
		{{"num_generations",0x20029,0,2,0,0,0.,0.,0,N_mdm(nnintz,numGenerations)},37,71,"N_mdm(nnintz,numGenerations)"},
		{{"percent_change",0x2000a,0,1,0,0,0.,0.,0,N_mdm(Realz,convergenceTolerance)},36,69,"N_mdm(Realz,convergenceTolerance)"}
		},
	kw_14[2] = {
		{{"num_generations",0x20029,0,2,0,0,0.,0.,0,N_mdm(nnintz,numGenerations)},34,65,"N_mdm(nnintz,numGenerations)","{Number of generations (for convergence test) } http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{{"percent_change",0x2000a,0,1,0,0,0.,0.,0,N_mdm(Realz,convergenceTolerance)},33,63,"N_mdm(Realz,convergenceTolerance)","{Percent change in fitness} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"}
		},
	kw_15[2] = {
		{{"average_fitness_tracker",0x20008,2,1,1,(KeyWord*)kw_13,0.,0.,0,N_mdm(lit,convergenceType_average_fitness_tracker)},35,67,"N_mdm(lit,convergenceType_average_fitness_tracker)"},
		{{"best_fitness_tracker",0x20008,2,1,1,(KeyWord*)kw_14,0.,0.,0,N_mdm(lit,convergenceType_best_fitness_tracker)},32,61,"N_mdm(lit,convergenceType_best_fitness_tracker)"}
		},
	kw_16[2] = {
		{{"constraint_penalty",0x2000a,0,2,0,0,0.,0.,0,N_mdm(Realp,constraintTolerance)},25,47,"N_mdm(Realp,constraintTolerance)","{Constraint penalty in merit function} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{{"merit_function",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,fitnessType_merit_function)},24,45,"N_mdm(lit,fitnessType_merit_function)"}
		},
	kw_17[4] = {
		{{"elitist",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_elitist)},27,51,"N_mdm(lit,replacementType_elitist)"},
		{{"favor_feasible",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_favor_feasible)},28,53,"N_mdm(lit,replacementType_favor_feasible)"},
		{{"roulette_wheel",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_roulette_wheel)},29,55,"N_mdm(lit,replacementType_roulette_wheel)"},
		{{"unique_roulette_wheel",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,replacementType_unique_roulette_wheel)},30,57,"N_mdm(lit,replacementType_unique_roulette_wheel)"}
		},
	kw_18[3] = {
		{{"convergence_type",0x20008,2,3,0,(KeyWord*)kw_15},31,59},
		{{"fitness_type",0x20008,2,1,0,(KeyWord*)kw_16},23,43,0,"{Fitness type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"},
		{{"replacement_type",0x20008,4,2,0,(KeyWord*)kw_17},26,49,0,"{Replacement type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGASOGA"}
		},
	kw_19[9] = {
		{{"crossover_type",0x20008,5,6,0,(KeyWord*)kw_2},45,87,0,"{Crossover type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{{"initialization_type",0x20008,3,5,0,(KeyWord*)kw_3},41,79,0,"{Initialization type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{{"log_file",0x2000b,0,3,0,0,0.,0.,0,N_mdm(str,logFile)},39,75,"N_mdm(str,logFile)","{Log file} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{{"moga",0x20008,5,1,1,(KeyWord*)kw_10,0.,0.,0,N_mdm3(moga_begin,0,moga_final)},2,3,"N_mdm3(moga_begin,0,moga_final)",0,"Optimization: Global"},
		{{"mutation_type",0x20008,6,7,0,(KeyWord*)kw_12},53,103,0,"{Mutation type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{{"population_size",0x20009,0,2,0,0,0.,0.,0,N_mdm(nnint,populationSize)},38,73,"N_mdm(nnint,populationSize)","{Number of population members} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{{"print_each_pop",0x20008,0,4,0,0,0.,0.,0,N_mdm(true,printPopFlag)},40,77,"N_mdm(true,printPopFlag)","{Population output} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodJEGADC"},
		{{"seed",0x20009,0,8,0,0,0.,0.,0,N_mdm(pint,randomSeed)},61,119,"N_mdm(pint,randomSeed)"},
		{{"soga",0x20008,3,1,1,(KeyWord*)kw_18,0.,0.,0,N_mdm3(soga_begin,0,soga_final)},22,41,"N_mdm3(soga_begin,0,soga_final)",0,"Optimization: Global"}
		},
	kw_20[1] = {
		{{"jega",0x20008,9,1,1,(KeyWord*)kw_19,0.,0.,0,jega_start},1,1,"jega_start"}
		};


#ifdef __cplusplus
extern "C" {
KeyWord *keyword_add(void);
}
#endif

 KeyWord*
keyword_add(void) {
	return &kw_20[0].kw;
	}
}

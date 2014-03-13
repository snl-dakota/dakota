#include "NIDRProblemDescDB.H"
#include "JEGAOptimizer.H"

namespace Dakota {
#define MP_(x) DataMethodRep::* method_mp_##x = &DataMethodRep::x
#define MP2(x,y) method_mp_##x##_##y = {&DataMethodRep::x,#y}
#define MP3(x,y,z) method_mp_3##x##_##z = {&DataMethodRep::x,&DataMethodRep::y,#z}

 struct
Method_mp_lit {
	String DataMethodRep::* sp;
	const char *lit;
	};

 struct
Method_mp_litrv {
	String DataMethodRep::* sp;
	RealVector DataMethodRep::* rp;
	const char *lit;
	};

 struct
Method_mp_ilit2z {
	String DataMethodRep::* sp;
	size_t DataMethodRep::* ip;
	const char *lit;
	};

 struct
Method_mp_litc {
	String DataMethodRep::* sp;
	Real DataMethodRep::* rp;
	const char *lit;
	};

 struct
Method_mp_slit2 {
	String DataMethodRep::* sp;
	String DataMethodRep::* sp2;
	const char *lit;
	};

static bool
	MP_(printPopFlag);

static Real
	MP_(constraintTolerance),
	MP_(convergenceTolerance),
	MP_(mutationScale),
	MP_(shrinkagePercent);	// should be called shrinkageFraction

static Method_mp_ilit2z
	MP3(crossoverType,numCrossPoints,multi_point_binary),
	MP3(crossoverType,numCrossPoints,multi_point_parameterized_binary),
	MP3(crossoverType,numCrossPoints,multi_point_real);

static Method_mp_lit
	MP2(convergenceType,average_fitness_tracker),
	MP2(convergenceType,best_fitness_tracker),
	MP2(convergenceType,metric_tracker),
	MP2(crossoverType,blend),
	MP2(fitnessType,domination_count),
	MP2(fitnessType,layer_rank),
	MP2(fitnessType,merit_function),
	MP2(initializationType,random),
	MP2(initializationType,unique_random),
	MP2(mutationType,bit_random),
	MP2(mutationType,offset_cauchy),
	MP2(mutationType,offset_normal),
	MP2(mutationType,offset_uniform),
	MP2(mutationType,replace_uniform),
	MP2(replacementType,elitist),
	MP2(replacementType,favor_feasible),
	MP2(replacementType,roulette_wheel),
	MP2(replacementType,unique_roulette_wheel);

static Method_mp_litc
	MP3(crossoverType,crossoverRate,shuffle_random),
	MP3(crossoverType,crossoverRate,null_crossover),
	MP3(mutationType,mutationRate,null_mutation),
	MP3(mutationType,mutationRate,offset_cauchy),
	MP3(mutationType,mutationRate,offset_normal),
	MP3(mutationType,mutationRate,offset_uniform),
	MP3(replacementType,fitnessLimit,below_limit);

static Method_mp_litrv
	MP3(nichingType,nicheVector,distance),
	MP3(nichingType,nicheVector,radial),
	MP3(postProcessorType,distanceVector,distance_postprocessor);

static int
	MP_(populationSize),
	MP_(randomSeed);

static Method_mp_slit2
	MP3(initializationType,flatFile,flat_file);

static size_t
	MP_(numGenerations),
	MP_(numOffspring),
	MP_(numParents);

static String
	MP_(logFile);

#define N_mdm(x,y)	NIDRProblemDescDB::method_##x,&method_mp_##y
#define N_mdm3(x,y,z)	NIDRProblemDescDB::method_##x,y,NIDRProblemDescDB::method_##z

extern JEGAOptimizer *(*new_JEGAOptimizer)(Model& model);

 static JEGAOptimizer *
my_new_JEGAOptimizer(Model& model)
{
	return new JEGAOptimizer(model);
	}

 static void
jega_start(const char *keyname, Values *val, void **g, void *v)
{
	new_JEGAOptimizer = my_new_JEGAOptimizer;
	}

/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */
//
// pebblParams.cpp
//

#include <acro_config.h>
#include <pebbl/pebblParams.h>
#include <pebbl/pebblBase.h>
#include <pebbl/gRandom.h>

namespace pebbl {

using utilib::ParameterLowerBound;
using utilib::ParameterBounds;
using utilib::ParameterNonnegative;

// this refers to the global random number seed, which is 
// currently declared in pebbl/gRandom.h. 
extern size_t randomSeed;

pebblParams::pebblParams()
  : statusPrintCount(100000),
    statusPrintSeconds(10.0),
    depthFirst(false),
    breadthFirst(false),
    initialDive(false),
    integralityDive(true),
    lazyBounding(false),
    eagerBounding(false),
    relTolerance(1e-7),
    absTolerance(0.0),
    earlyOutputMinutes(0.0),
    startIncumbent(0.0),
    validateLog(false),    
    heurLog(false),
    loadLogSeconds(0),
    loadLogWriteSeconds(0),
    maxSPBounds(0),
    maxCPUMinutes(0.0),
    maxWallMinutes(0.0),
    haltOnIncumbent(false),
    printAbortMessage(true),
    printIntMeasure(false),    
    printDepth(false),
    debugPrecision(0),
    suppressWarnings(false),
    loadMeasureDegree(1),
    enumRelTol(-1.0),
    enumAbsTol(-1.0),
    enumCutoff(-MAXDOUBLE),
    enumCount(-1),
    enumHashSize(1024),
    debug_solver_params(false),
    printSpTimes(0),
#ifdef ACRO_VALIDATING
    use_abort(true)
#else
    use_abort(false)
#endif
{
/// GENERAL

  create_categorized_parameter("loadMeasureDegree",loadMeasureDegree, 
		"<int>","1",
		"Measure 'weight' of a subproblem by its difference\n\t"
                "from the incumbent to this power",
		"Search",
		utilib::ParameterBounds<int>(0,pebblBase::maxLoadDegree));

  // this is a slightly strange parameter. the global seed is stored in
  // pebbl/gRandom, but is referenced here for purposes of parameter-driven
  // initialization. thus far, this is the only beast of its kind.
  randomSeed=1;
  create_categorized_parameter("randomSeed",randomSeed,
		"<size_t>","1",
		"Global seed for random number generation.",
		"General",
		utilib::ParameterNonnegative<int>());
  alias_parameter("randomSeed","seed");

/// ACRO_VALIDATING

  version_flag=false;
  create_categorized_parameter("version",version_flag,
                "<bool>","false",
                "If true, print version info (must be first parameter)",
		"General");

  create_categorized_parameter("debug-solver-params",debug_solver_params,
                "<bool>","false",
                "If true, print solver parameters",
		"Debugging");

  create_categorized_parameter("useAbort",use_abort,
                "<bool>",
                #ifdef ACRO_VALIDATING
                "true",
                #else
                "false",
                #endif
                "If true, then force an abort when an error occurs.",
                "Termination");

  create_categorized_parameter("printIntMeasure",printIntMeasure,
	      	"<bool>","false",
	      	"Include subproblem integrality measures in debug output",
		"Debugging");

  create_categorized_parameter("printDepth",printDepth,
		"<bool>", "false",
		"Include subproblem depth in debug output",
		"Debugging");

  create_categorized_parameter("debugPrecision",debugPrecision,
		"<int>","0",
		"Debug output objective value precision",
		"Debugging",
		ParameterBounds<int>(0,20));

  create_categorized_parameter("suppressWarnings",suppressWarnings,
		"<bool>","false",
		"Suppress warning messages.",
		"Output");

  create_categorized_parameter("output",solFileName,
		"<string>","",
		"Filename used to store the final solution.  The default is <name>.sol.txt",
		"Output");

  create_categorized_parameter("debug",debug,
		"<int>","0",
		"Debugging output level",
		"Debugging",
		ParameterNonnegative<int>());

  create_categorized_parameter("statusPrintCount",statusPrintCount,
		"<int>","100000",
		"Number of subproblems bounded between status printouts",
		"Output",
		ParameterNonnegative<int>());

  create_categorized_parameter("statusPrintSeconds",statusPrintSeconds,
		"<double>","10.0",
		"Seconds elapsing between status printouts",
		"Output",
		ParameterNonnegative<double>());

  create_categorized_parameter("earlyOutputMinutes",earlyOutputMinutes,
		"<double>","0",
		"If this much time elapses, make sure current incumbent\n\t"
		"is output to a file in case of crash etc.",
		"Output",
		ParameterNonnegative<double>());

  create_categorized_parameter("validateLog",validateLog,
		"<bool>","false",
		"Output validation log files val*.log for logAnalyze",
		"Debugging");

  create_categorized_parameter("loadLogSeconds",loadLogSeconds,
		"<double>","0",
		"Seconds between load log records (0 means no load log)",
		"Debugging",
		ParameterNonnegative<double>());

  create_categorized_parameter("loadLogWriteSeconds",loadLogWriteSeconds,
		"<double>","0",
		"Seconds between writes to load log file\n\t"
		"0 means write load log only at end of run\n\t"
                "If nonzero in parallel, all processors must have\n\t"
                "direct access to file system",
		"Debugging",
		ParameterNonnegative<double>());


/// SEARCH

  create_categorized_parameter("depthFirst",depthFirst,
		"<bool>","false",
		"Use depth-first search",
		"Search");

  create_categorized_parameter("breadthFirst",breadthFirst,
		"<bool>","false",
		"Use breadth-first search",
		"Search");

  create_categorized_parameter("initialDive",initialDive,
		"<bool>","false",
		"Use depth-first-like 'dive' until first incumbent found",
		"Search");

  create_categorized_parameter("integralityDive",integralityDive,
		"<bool>","true",
		"Use subproblem integrality measure to guide initial\n\t"
                "diving; otherwise, just use depth",
		"Search");

  create_categorized_parameter("lazyBounding",lazyBounding,
		"<bool>","false",
		"Delay bounding subproblems as long as possible",
		"Search");

  create_categorized_parameter("eagerBounding",eagerBounding,
		"<bool>","false",
		"Bound problems as soon as possible",
		"Search");

/// TERMINATION

  create_categorized_parameter("relTolerance",relTolerance,
		"<double>","1e-7",
		"Relative tolerance for optimal objective value",
		"Termination",
		ParameterNonnegative<double>());

  create_categorized_parameter("absTolerance",absTolerance,
		"<double>","0.0",
		"Absolute tolerance for optimal objective value",
		"Termination",
		ParameterNonnegative<double>());

  create_categorized_parameter("integerTolerance",pebblBase::integerTolerance,
		"<double>","1e-5",
		"Tolerance to determine whether values are integral",
		"Termination",
		ParameterBounds<double>(0.0,1.0));

  create_categorized_parameter("maxSPBounds",maxSPBounds,
		"<int>","0",
		"Maximum subproblem bounds computed before abort\n\t"
		"(0 means no limit)",
		"Termination",
		ParameterNonnegative<int>());

  create_categorized_parameter("maxCPUMinutes",maxCPUMinutes,
		"<double>","0.0",
		"CPU minutes (per processor) allowed before abort\n\t"
		"(0 means no limit)",
		"Termination",
		ParameterNonnegative<double>());

  create_categorized_parameter("maxWallMinutes",maxWallMinutes,
		"<double>","0.0",
		"Wall clock minutes allowed before abort (0 means no limit)",
		"Termination",
		ParameterNonnegative<double>());

  create_categorized_parameter("haltOnIncumbent",haltOnIncumbent,
		"<bool>","false",
		"Halt search once an incumbent is found",
                "Termination");

  create_categorized_parameter("printAbortMessage",printAbortMessage,
		"<bool>","true",
		"Print an alert message and statistics when aborting",
		"Termination");

/// INCUMBENT MANAGEMENT

  create_categorized_parameter("startIncumbent",startIncumbent,
		"<double>","0.0",
		"Value of some known feasible solution",
		"Incumbent");

  create_categorized_parameter("heurLog",heurLog,
		"<bool>","false",
		"Output log for debugging incumbent heuristics",
		"Debugging");

/// ENUMERATION

  create_categorized_parameter("enumRelTol",
		enumRelTol, "<double>","-1.0",
                "Relative tolerance for enumeration.  Find solutions\n\t"
                "that are within this multiplicative factor of being\n\t"
                "optimal.  For example, a value of 0.1 requests solutions\n\t"
                "within 10% of optimality.",
		"Enumeration",
		utilib::ParameterLowerBound<double>(-1.0));

  create_categorized_parameter("enumAbsTol",
                enumAbsTol, "<double>","-1.0",
		"Absolute tolerance for enumeration.  Find solutions\n\t"
                "that are within this additive distance of optimality.",
		"Enumeration",
		utilib::ParameterLowerBound<double>(-1.0));

  create_categorized_parameter("enumCutoff",
                enumCutoff, "<double>","-MAXDOUBLE",
		"Absolute cutoff value for enumeration.  Only enumerate\n\t"
                "solutions better than this value.",
		"Enumeration"),

  create_categorized_parameter("enumCount",enumCount, "<int>","0",
            "If positive, the limit on the number of enumerated solutions.\n\t"
            "If enumRelTolerance or enumAbsTolerance are set, return an\n\t"
            "arbitrary set of up to enumCount solutions meeting the\n\t"
            "tolerance criteria.  If neither enumeration tolerance\n\t"
            "is set, return a set of enumCount solutions with the\n\t"
	    "best acheivable objective values.",
	    "Enumeration",
	     utilib::ParameterNonnegative<int>());

  create_categorized_parameter("enumHashSize",enumHashSize,"<int>","1024",
	    "Size of hash table used to check for duplicate solutions.",
	    "Enumeration",
	    utilib::ParameterPositive<int>());



  printFullSolution=false;
  create_categorized_parameter("printFullSolution",printFullSolution,
		  "<bool>","false",
		  "Print full solution to standard output as well\n\t"
		  "as writing it to a file","Output");

  create_categorized_parameter("printSpTimes",printSpTimes, 
		"<int>","0",
		"Print individual subproblem timings\n\t"
                "1=print statistics only, 2=print for each subproblems",
		"Search",
		utilib::ParameterBounds<int>(0,2));

}

} // namespace pebbl

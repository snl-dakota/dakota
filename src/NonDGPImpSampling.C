/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGPImpSampling
//- Description: Implementation code for NonDGPImpSampling class
//- Owner:       Laura Swiler and Keith Dalbey
//- Checked by:
//- Version:

#include "NonDGPImpSampling.H"
#include "system_defs.h"
#include "data_types.h"
#include "data_io.h"
#include "DakotaModel.H"
#include "DakotaResponse.H"
#include "NonDLHSSampling.H"
#include "ProblemDescDB.H"
#include "DataFitSurrModel.H"
#include "pecos_data_types.hpp"
#include "pecos_stat_util.hpp"
#include "DakotaApproximation.H"
#include <boost/lexical_cast.hpp>

static const char rcsId[]="@(#) $Id: NonDGPImpSampling.C 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDGPImpSampling::NonDGPImpSampling(Model& model): NonDSampling(model)
{ 
  samplingVarsMode = ACTIVE_UNIFORM;
  String sample_reuse, approx_type("global_gaussian");/*("global_kriging");*/
  UShortArray approx_order; // not used by GP/kriging
  short corr_order = -1, data_order = 1, corr_type = NO_CORRECTION;
  if (probDescDB.get_bool("method.derivative_usage")) {
    if (gradientType != "none") data_order |= 2;
    if (hessianType  != "none") data_order |= 4;
  }
  String sample_type("lhs"); // hard-wired for now
  bool vary_pattern = false; // for consistency across outer loop invocations
  construct_lhs(gpBuild, iteratedModel, sample_type, numSamples, randomSeed,
		rngName, vary_pattern);
  gpModel.assign_rep(new DataFitSurrModel(gpBuild, iteratedModel, approx_type,
    approx_order, corr_type, corr_order, data_order, sample_reuse), false);
  vary_pattern = true; // allow seed to run among multiple approx sample sets
  construct_lhs(gpEval, gpModel, sample_type, numEmulEval, randomSeed,
		rngName, vary_pattern);

  //gpModel.init_communicators(gpBuild.maximum_concurrency());
  gpModel.init_communicators(gpEval.maximum_concurrency());
}

NonDGPImpSampling::~NonDGPImpSampling()
{
  //gpModel.free_communicators(gpBuild.maximum_concurrency());
  gpModel.free_communicators(gpEval.maximum_concurrency());
}
 

//NonDGPImpSampling::
//NonDGPImpSampling(Model& model, const String& sample_type, int samples,
//                     int seed, const String& rng,
//                     short sampling_vars_mode = ACTIVE,
//                     const RealVector& lower_bnds, const RealVector& upper_bnds):
//  NonDSampling(NoDBBaseConstructor(), model, sample_type, samples, seed, rng)
//{
//
//}


/** Calculate the failure probabilities for specified probability levels 
    using Gaussian process based importance sampling. */
void NonDGPImpSampling::quantify_uncertainty()
{

  numPtsAdd = 400; // need to include this in the input spec
  numPtsTotal = numSamples + numPtsAdd;
  numEmulEval = 10; // perhaps add to input spect?
 
  // Build initial GP model using truth model evals.  This will be
  // over the initial LHS sample set defined in the constructor.
  gpModel.build_approximation(); // runs gpBuild on iteratedModel

  // Evaluate approximate samples on the GP
  gpEval.run_iterator(Cout);

  //KEITH:  Currently I have gpCvars, gpVar, and gpMeans as variables 
  //holding the GP surrogate results.  We should discuss if these 
  //should be class variables or local variables.
 
  const RealMatrix&  all_samples = gpEval.all_samples();
  RealVectorArray  gpCvars(numEmulEval);
  RealVector temp_cvars;
  RealVectorArray gpVar(numEmulEval);
  RealVectorArray gpMeans(numEmulEval);
  const IntResponseMap& all_resp = gpEval.all_responses();
  int num_problem_vars=iteratedModel.acv();
  int i,j;
 
// KEITH:  You will probably not need to use this.  This piece of 
// code prints out the build points, not the approximation points.
  const Pecos::SurrogateData& gp_data = gpModel.approximation_data(0);
  for (j = 0; j < numSamples; j++) {
    Cout << " Surrogate Vars " << gp_data.continuous_variables(j) << '\n';
    Cout << " Surrogate Response " << gp_data.response_function(j); 
  }

// The following two loops obtain the variables, the variances, and the 
// means of the GP prediction.  Probably the best way to handle this is 
// to calculate these and hand them to an expected indicator function. 
//
  for (i = 0; i< numEmulEval; i++) {
    temp_cvars = RealVector(Teuchos::View, (Real*)all_samples[i], num_problem_vars);
    gpCvars[i]=temp_cvars;
    Cout << "input is " << gpCvars[i] << '\n'; 
    gpVar[i] = gpModel.approximation_variances(temp_cvars);
    Cout << "variance is " << gpVar[i];
  }

  IntRespMCIter resp_it = all_resp.begin();
  for (j=0, resp_it=all_resp.begin(); j<numEmulEval; ++j, ++resp_it) {
    RealVector temp_resp(numFunctions);
    for (i=0; i<numFunctions; i++) 
       temp_resp(i) = resp_it->second.function_value(i);
     gpMeans[j]=temp_resp;
     Cout << "output is " << gpMeans[j] << '\n';
  }

// We have built the initial GP.  Now we need to go through, per response function// and response level and calculate the failure probability. 
// We will need to add error handling:  we will only be calculating 
// results per response level, not probability level or reliability index.
   int respFnCount, levelCount;

   for (respFnCount=0; respFnCount<numFunctions; respFnCount++) {
     size_t num_levels = requestedRespLevels[respFnCount].length();
     for (levelCount=0; levelCount<num_levels; levelCount++) {
       // Here we will have a loop over the number of points added, 
       // where each time you calculate the expected indicator function 
       // and add to the approximation. 
       // it should be something like: 
       // for (k = 0; k < numPtsAdd; k++) { 
       //   calculate expected indicator function;
       //   calculate rhoDraw, normalization constants, etc. 
       //   calculate importance density and probability of hitting failure reg
       //   calculate mixture model estimate
       //   NOTE:  when you need to append to the gpModel, use: 
       //   gpModel.append_approximation(new_vars, resp_truth, true), 
       //   where you need to run the simulation model at your suggested point 
       //   using something like: 
       //   iteratedModel.continuous_variables(new_vars);
       //   iteratedModel.compute_response();
       //   IntResponsePair resp_truth(iteratedModel.evaluation_id(),
       //                            iteratedModel.current_response());
       //                                                        
       //
     }
   }
}


} // namespace Dakota

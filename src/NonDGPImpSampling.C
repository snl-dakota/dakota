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
  String sample_reuse, approx_type("global_kriging");/*("global_kriging");*/
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
  // need to add to input spec
  numEmulEval = 100;
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

  numPtsAdd = 200; // need to include this in the input spec
  numPtsTotal = numSamples + numPtsAdd;
  numEmulEval = 100; // perhaps add to input spect?
 
  // Build initial GP model.  This will be built over the initial LHS sample set
  // defined in the constructor.
  gpModel.build_approximation();
  
  gpCvars.resize(numEmulEval);
  RealVector temp_cvars;
  gpVar.resize(numEmulEval);
  gpMeans.resize(numEmulEval);
  indicator.resize(numPtsTotal);
  expIndicator.resize(numEmulEval);
  normConst.resize(numPtsAdd);
  //RealVector rhoEmul0(numEmulEval);
  //RealVector rhoEmul2(numEmulEval);
  int num_problem_vars=iteratedModel.acv();

  int i,j,k;
 
// KEITH:  You will probably not need to use this.  This piece of 
// code prints out the build points, not the approximation points.
  const Pecos::SurrogateData& gp_data = gpModel.approximation_data(0);
//  for (j = 0; j < numSamples; j++) {
//    Cout << " Surrogate Vars " << gp_data.continuous_variables(j) << '\n';
//    Cout << " Surrogate Response " << gp_data.response_function(j); 
//  }

// We have built the initial GP.  Now we need to go through, per response function
// and response level and calculate the failure probability. 
// We will need to add error handling:  we will only be calculating 
// results per response level, not probability level or reliability index.
   
   int respFnCount, levelCount, iter;

   for (respFnCount=0; respFnCount<numFunctions; respFnCount++) {
     size_t num_levels = requestedRespLevels[respFnCount].length();
     for (levelCount=0; levelCount<num_levels; levelCount++) {
       Cout << "Starting calculations for response function " << respFnCount+1 << '\n';
       Cout << "Starting calculations for level  " << levelCount+1 << '\n';
       Cout << "Threshold level is " << requestedRespLevels[respFnCount][levelCount] << '\n';
       Real z = requestedRespLevels[respFnCount][levelCount];
       // Calculate indicator over the true function evaluations
       for (j = 0; j < numSamples; j++) {
         if (gp_data.response_function(j)<z) 
	   indicator(j)=1;
         else indicator(j)=0;
         Cout << "indicator j " << indicator(j) << '\n';
       }
       
       // Here we loop over the number of points added, 
       // where each time we calculate the expected indicator function 
       // and add to the approximation. 
       for (k = 0; k < numPtsAdd; k++) { 
         // generate new set of emulator samples.  Note this will have a different seed 
         // each time.
         gpEval.run_iterator(Cout);
         // obtain results 
         const RealMatrix&  all_samples = gpEval.all_samples();
         const IntResponseMap& all_resp  = gpEval.all_responses();
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
          
       // calculate expected indicator function;
         expIndicator = calcExpIndicator(respFnCount,z);
       // calculate distribution pdfs required to calculate the draw distribution
       // distribution_pdf(rhoEmul2);
       // distribution_pdf(rhoEmul0);
       // FOR NOW, we assume distribution 0 and 2 are the same, 
       // but in general rhoDrawThis = expIndicator * rhoEmul0/rhoEmul2;
         rhoDrawThis = expIndicator; 
      	
       // calculate the normalization constant
         normConst(k)=0.0;
         for (j = 0; j < numEmulEval; j++) 
	   normConst(k)+=expIndicator(j);
         normConst(k)/=numEmulEval;
         Real temp_norm_const = normConst(k);
       // calculate the draw distribution
         rhoDrawThis.resize(0);
         xDrawThis.resize(0);
         expIndThis.resize(0);
         calcRhoDraw();
          
         iter = 1;
         while ((iter<3) && (normConst(k)*numEmulEval<16)) {
	   iter = iter+1;
           gpEval.run_iterator(Cout);
           // obtain results 
           const RealMatrix&  this_samples = gpEval.all_samples();
           const IntResponseMap& this_resp  = gpEval.all_responses();
           for (i = 0; i< numEmulEval; i++) {
             temp_cvars = RealVector(Teuchos::View, (Real*)this_samples[i], num_problem_vars);
             gpCvars[i]=temp_cvars;
             Cout << "input is " << gpCvars[i] << '\n';
             gpVar[i] = gpModel.approximation_variances(temp_cvars);
             Cout << "variance is " << gpVar[i];
           }

           resp_it = this_resp.begin();
           for (j=0, resp_it=this_resp.begin(); j<numEmulEval; ++j, ++resp_it) {
             RealVector temp_resp(numFunctions);
             for (i=0; i<numFunctions; i++)
               temp_resp(i) = resp_it->second.function_value(i);
               gpMeans[j]=temp_resp;
              Cout << "output is " << gpMeans[j] << '\n';
           }
          
       // calculate expected indicator function;
           expIndicator = calcExpIndicator(respFnCount,z);
           Real temp_norm_this=0.0;
           for (j = 0; j < numEmulEval; j++) 
	     temp_norm_this+=expIndicator(j);
           temp_norm_this/=numEmulEval;
           temp_norm_const = temp_norm_const+temp_norm_this;
           calcRhoDraw();

         }     
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
       }
     }
   }
}


void NonDGPImpSampling::calcRhoDraw()
{ 
  int i, templength;
  templength = xDrawThis.size();

  for (i = 0; i< numEmulEval; i++) {
    if (expIndicator(i)!=0.0) {
      xDrawThis.resize(templength+1);
      expIndThis.resize(templength+1);
      rhoDrawThis.resize(templength+1);
      xDrawThis[templength]=gpCvars[i];
      expIndThis(templength)=expIndicator(i);
      rhoDrawThis(templength)=expIndicator(i);
      templength=templength+1;
    }
  }    
  
  for (i = 0; i< templength; i++) {
    Cout << "xDrawThis  " << i << xDrawThis[i] << '\n';
    Cout << "rhoDrawThis  "  << rhoDrawThis[i] << '\n';
  }
}

RealVector NonDGPImpSampling::calcExpIndicator(const int respFnCount, const Real respThresh)
{
  int i, j;
  RealVector ei(numEmulEval);

  Real cdf,snv,stdv;
  for (i = 0; i< numEmulEval; i++) {
    //Cout << "GPmean  " << gpMeans[i][respFnCount];
    //Cout << "GPvar  " << gpVar[i][respFnCount];
    snv = respThresh-gpMeans[i][respFnCount];
    stdv = std::sqrt(gpVar[i][respFnCount]); 
    if(std::fabs(snv)>=std::fabs(stdv)*50.0) {
    //this will trap the denominator=0.0 case even if numerator=0.0
      cdf=(snv>0.0)?1.0:0.0;
    }
    else{
      snv/=stdv;
      cdf = Pecos::Phi(snv);
    }
    ei(i)=cdf;
    Cout << "EI " << ei(i) << " respThresh= " << respThresh << " mu= " << gpMeans[i][respFnCount] << " stdv= " << stdv << '\n';
  }    
  return ei;

}

} // namespace Dakota

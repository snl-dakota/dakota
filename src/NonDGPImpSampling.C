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
		rngName, vary_pattern); //these are being drawn from 
  //distribution 1 which is the distribution that the initial set of samples
  //used to build the initial GP are drawn from this should "ALWAYS" be 
  //uniform in the input of the GP (even if the nominal distribution is not
  //uniform) because it is a set of samples to build a good GP and nothing
  //else.  Rho 0 is the nonminal distribution of the input variable

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

  numPtsAdd = 20; // need to include this in the input spec
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
  rhoMix.resize(numPtsTotal);
  //RealVector rhoEmul0(numEmulEval);
  //RealVector rhoEmul2(numEmulEval);
  int num_problem_vars=iteratedModel.acv();

  int i,j,k;
 
// KEITH:  You will probably not need to use this.  This piece of 
// code prints out the build points, not the approximation points.
// const Pecos::SurrogateData& gp_data = gpModel.approximation_data(0);
//  for (j = 0; j < numSamples; j++) {
//    Cout << " Surrogate Vars " << gp_data.continuous_variables(j) << '\n';
//    Cout << " Surrogate Response " << gp_data.response_function(j); 
//  }

// We have built the initial GP.  Now we need to go through, per response function
// and response level and calculate the failure probability. 
// We will need to add error handling:  we will only be calculating 
// results per response level, not probability level or reliability index.
   
  int respFnCount, levelCount, iter;
  RealVector new_X;
 
  for (respFnCount=0; respFnCount<numFunctions; respFnCount++) {
    size_t num_levels = requestedRespLevels[respFnCount].length();
    const Pecos::SurrogateData& gp_data = gpModel.approximation_data(respFnCount);
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
        const IntResponseMap& all_resp = gpEval.all_responses();
        for (i = 0; i< numEmulEval; i++) {
          temp_cvars = Teuchos::getCol(Teuchos::View,
	    const_cast<RealMatrix&>(all_samples), i);
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
        while ((iter<4) && (temp_norm_const*numEmulEval<16)) {
	  iter = iter+1;
          gpEval.run_iterator(Cout);
           // obtain results 
          const RealMatrix&  this_samples = gpEval.all_samples();
          const IntResponseMap& this_resp = gpEval.all_responses();
          for (i = 0; i< numEmulEval; i++) {
	    temp_cvars = Teuchos::getCol(Teuchos::View,
	       const_cast<RealMatrix&>(this_samples), i);
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

       // xDrawThis, rhoDrawThis, and expIndThis should be populated now
        normConst(k)=temp_norm_const/iter;
        Cout << "NormConstk " << normConst(k) << '\n';
 
        int num_eval_kept = xDrawThis.size();  
        Real est_prob_hit_failregion; 
 
        if (num_eval_kept==0) {
	  normConst(k)=0.;
          /// need to determine how to sample x_new 
	  /// KRD says draw from distribution 1 in this case
          est_prob_hit_failregion = 0.;
        }
        else 
          new_X = drawNewX(k);
         
         // add new_X to the build points and append approximation
        iteratedModel.continuous_variables(new_X);
        iteratedModel.compute_response();
        IntResponsePair resp_truth(iteratedModel.evaluation_id(),
                                   iteratedModel.current_response());
        gpModel.append_approximation(iteratedModel.current_variables(), resp_truth, true);
        if (gp_data.response_function(numSamples+k-1)<z) 
	  indicator(numSamples+k-1)=1;
        else indicator(numSamples+k-1)=0;
        Cout << "Done with iteration k "; 
      }
      RealVectorArray gp_final_data(numPtsTotal);
      for (j = 0; j < numPtsTotal; j++) 
        gp_final_data[j]=gp_data.continuous_variables(j);
      Cout << "GP final data size " <<  gp_final_data.size() << '\n'; 
//
//This is where we need some re-architecting.  I want to evaluate the GPmodel at a 
//set of pre-defined points.  We will need to use a parameter list study. 
//something like the following
//Iterator listStudy;
//The list of points will be the full X data, consisting of the original plus added points
//We have this stored in the Surrogate data
//VariablesArray list_points;
//list_points.resize(numPtsTotal);
//for (j = 0; j < numSamples; j++) {
//    list_points[j]=gp_data.continuous_variables(j);
//}
//
//listStudy.assign_rep(new ParamStudy(gpModel,"LIST", list_points));
//
      rhoOne.resize(numPtsTotal);
//not sure if I have this correct 
//for now, since we are assuming rho0=rho1=rho2=all uniform, just set rho1 to 1.
      for (j = 0; j < numPtsTotal; j++) 
        rhoOne(j)=1; //for uniform this should be 1.0/prod(xmax-xmin) across all dimensions... ok now I see this isn't rhoOne it's rhoOne/rhoZero... if rho0=rho1=rho2 this should be ok
  
      for (j = 0; j < numPtsTotal; j++)
        rhoMix(j)=rhoOne(j)*numSamples;
         
      for (j = 0; j < numPtsAdd; j++){
        if (normConst(j)==0) {
          for (k = 0; k < numPtsTotal; k++) 
            rhoMix(k)=rhoMix(k)+rhoOne(k);
        }
        else {
          RealVector this_mean;
          RealVector this_var;
          RealVector exp_ind_this(numPtsTotal);
          for (k = 0; k < numPtsTotal; k++){ 
            gpModel.continuous_variables(gp_final_data[k]);
            gpModel.compute_response();
            this_mean = gpModel.current_response().function_values();
            this_var = gpModel.approximation_variances(gp_final_data[k]);
            exp_ind_this(k) = calcExpIndPoint(respFnCount,z,this_mean,this_var);
          }
          for (k = 0; k < numPtsTotal; k++) 
            rhoMix(k)=rhoMix(k)+exp_ind_this(k)*1.0/normConst(j);
	  //the 1.0 here is reall rhoZero/rhoZero (ok for rho0=rho1=rho2)
          gpModel.pop_approximation(false);
          Cout << "Size of build data set " << gp_data.size() << '\n';
        } 
//Since we need to evaluate the SUCCESSIVE SEQUENCES of GPs built using numSamples-->numPtsTotal
//it might be most efficient to "pop" the data and go backward: 
//listStudy.run_iterator();
//obtain results and expected indicator functions
//gpModel.pop_approximation();
//
      }            
      Cout << "rhoMix " << rhoMix << '\n';
      Cout << "indicator " << indicator << '\n'; 
      for (j = 0; j < numPtsTotal; j++)
        rhoMix(j)/=numPtsTotal;
      Real prob_mix=0.0;
      for (j = 0; j < numPtsTotal; j++) 
        prob_mix+=1.0*indicator(j)/rhoMix(j);
      //the 1.0 here is reall rhoZero/rhoZero (ok for rho0=rho1=rho2)
      prob_mix/=numPtsTotal;
      Cout << "Prob Mix IS " << prob_mix << '\n'; 
 
      Real fract_fail_mix = 0.0;
      for (j = 0; j < numPtsTotal; j++) 
        fract_fail_mix+=indicator(j);
      fract_fail_mix/=numPtsTotal;
      Cout << "Fraction Fail IS " << fract_fail_mix << '\n'; 
        
    }
  }
}


RealVector NonDGPImpSampling::drawNewX(int this_k)
{
  int i,j,templength;
  templength = xDrawThis.size();
  RealVector binEnds;
  binEnds.size(templength);
  Real cum_sum = 0.; 
  Real est_prob_hit_failregion = 0;
  
  Cout << "templength " << templength << '\n'; 
  for (i=0; i<templength; i++) {
    rhoDrawThis(i)/=normConst(this_k);
    if (i==0)
      binEnds(i)=rhoDrawThis(i);
    else
      binEnds(i)=rhoDrawThis(i)+rhoDrawThis(i-1);
  }
  cum_sum = binEnds(templength-1);
  Cout << "Cum Sum"  << cum_sum << '\n';
  Cout << "BinEnds " << binEnds << '\n';
  Cout << "RhoDrawThis " << rhoDrawThis << '\n';
  for (i=0; i<templength; i++) {
    binEnds(i)=binEnds(i)/cum_sum;
  }
  //std::srand(randomSeed);
  double rand_cdf = (double)std::rand()/RAND_MAX;
  Cout << "randcdf " << rand_cdf << '\n';
  bool found_cdf=false; 
  i=0; 
  while ((i<templength) && !found_cdf) {
    if (binEnds(i) > rand_cdf)
      found_cdf = true;
    else 
      i = i+1;
  }
  for (j=0; j<templength; j++) {
    est_prob_hit_failregion+=expIndThis(j)*rhoDrawThis(j)/cum_sum;
  } 
  Cout << "Estimated prob of hitting failure region " << est_prob_hit_failregion << '\n';
  return xDrawThis[i];
}

void NonDGPImpSampling::calcRhoDraw()
{ 
  int i, templength;
  templength = xDrawThis.size();

  for (i = 0; i<numEmulEval; i++) {
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
      ei(i)=(snv>=0.0)?1.0:0.0;
    }
    else{
      snv/=stdv;
      cdf = Pecos::Phi(snv);
      ei(i)=cdf;
    }

    Cout << "EI " << ei(i) << " respThresh= " << respThresh << " mu= " << gpMeans[i][respFnCount] << " stdv= " << stdv << '\n';
  }    
  return ei;
}

Real NonDGPImpSampling::calcExpIndPoint(const int respFnCount, const Real respThresh, const RealVector this_mean, const RealVector this_var)
{
  int i, j;
  Real ei;

  Real cdf,snv,stdv;
  //  Cout << "GPmean  " << this_mean(respFnCount);
  //  Cout << "GPvar  " << this_var(respFnCount);
  snv = respThresh-this_mean(respFnCount);
  stdv = std::sqrt(this_var(respFnCount)); 
  if(std::fabs(snv)>=std::fabs(stdv)*50.0) {
    //this will trap the denominator=0.0 case even if numerator=0.0
    ei=(snv>=0.0)?1.0:0.0;
  }
  else{
    snv/=stdv;
    cdf = Pecos::Phi(snv);
    ei=cdf;
  }

  Cout << "EI " << ei << " respThresh= " << respThresh << " mu= " << this_mean(respFnCount) << " stdv= " << stdv << '\n';
      
  return ei;
}

} // namespace Dakota

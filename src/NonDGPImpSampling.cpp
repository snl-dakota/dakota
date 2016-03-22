/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGPImpSampling
//- Description: Implementation code for NonDGPImpSampling class
//- Owner:       Laura Swiler and Keith Dalbey
//- Checked by:
//- Version:

#include "NonDGPImpSampling.hpp"
#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "dakota_data_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDLHSSampling.hpp"
#include "ProblemDescDB.hpp"
#include "DataFitSurrModel.hpp"
#include "pecos_data_types.hpp"
#include "pecos_stat_util.hpp"
#include "DakotaApproximation.hpp"
#include <boost/lexical_cast.hpp>

static const char rcsId[]="@(#) $Id: NonDGPImpSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDGPImpSampling::NonDGPImpSampling(ProblemDescDB& problem_db, Model& model):
  NonDSampling(problem_db, model)
{
  // sampleType default in DataMethod.cpp is SUBMETHOD_DEFAULT (0).
  // Enforce an LHS default for this method.
  if (!sampleType)
    sampleType = SUBMETHOD_LHS;

  samplingVarsMode = ACTIVE_UNIFORM;
  String sample_reuse, approx_type("global_kriging");/*("global_kriging");*/
  UShortArray approx_order; // not used by GP/kriging
  short corr_order = -1, data_order = 1, corr_type = NO_CORRECTION;
  if (probDescDB.get_bool("method.derivative_usage")) {
    if (iteratedModel.gradient_type() != "none") data_order |= 2;
    if (iteratedModel.hessian_type()  != "none") data_order |= 4;
  }
  unsigned short sample_type = SUBMETHOD_DEFAULT;
  statsFlag = true; //print computed probability levels at end
  bool vary_pattern = false; // for consistency across outer loop invocations
  // get point samples file
  const String& import_pts_file
    = probDescDB.get_string("method.import_build_points_file");
  // BMA: This was previously using numSamples = initial_samples from base class
  numSamples = probDescDB.get_int("method.build_samples");
  int samples = numSamples;
  if (!import_pts_file.empty())
    { samples = 0; sample_reuse = "all"; }

  gpBuild.assign_rep(new NonDLHSSampling(iteratedModel, sample_type,
     samples, randomSeed, rngName, varyPattern, ACTIVE_UNIFORM), false);
  //distribution 1 which is the distribution that the initial set of samples
  //used to build the initial GP are drawn from this should "ALWAYS" be 
  //uniform in the input of the GP (even if the nominal distribution is not
  //uniform) because it is a set of samples to build a good GP and nothing
  //else.  Rho 0 is the nonminal distribution of the input variable

  ActiveSet gp_set = iteratedModel.current_response().active_set(); // copy
  gp_set.request_values(1); // no surr deriv evals, but GP may be grad-enhanced
  gpModel.assign_rep(new DataFitSurrModel(gpBuild, iteratedModel,
    gp_set, approx_type, approx_order, corr_type, corr_order, data_order,
    outputLevel, sample_reuse, import_pts_file,
    probDescDB.get_ushort("method.import_build_format"),
    probDescDB.get_bool("method.import_build_active_only"),
    probDescDB.get_string("method.export_approx_points_file"),
    probDescDB.get_ushort("method.export_approx_format")), false);
  vary_pattern = true; // allow seed to run among multiple approx sample sets
  // need to add to input spec
  numEmulEval = probDescDB.get_int("method.nond.samples_on_emulator");
  if (numEmulEval==0)
    numEmulEval = 10000;
  construct_lhs(gpEval, gpModel, sample_type, numEmulEval, randomSeed,
		rngName, vary_pattern);
  if (maxIterations < 0) 
    numPtsAdd = 150;
  else
    numPtsAdd = maxIterations;

  //construct sampler to generate one draw from rhoOne distribution, with 
  //seed varying between invocations
  construct_lhs(sampleRhoOne, iteratedModel, sample_type, 1, randomSeed,
		rngName, vary_pattern); 
}


NonDGPImpSampling::~NonDGPImpSampling()
{ }


bool NonDGPImpSampling::resize()
{
  bool parent_reinit_comms = NonDSampling::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void NonDGPImpSampling::derived_init_communicators(ParLevLIter pl_iter)
{
  iteratedModel.init_communicators(pl_iter, maxEvalConcurrency);

  // gpBuild and gpEval use NoDBBaseConstructor, so no need to
  // manage DB list nodes at this level
  //gpBuild.init_communicators(pl_iter);
  gpEval.init_communicators(pl_iter);
} 


void NonDGPImpSampling::derived_set_communicators(ParLevLIter pl_iter)
{
  NonD::derived_set_communicators(pl_iter);

  // gpBuild and gpEval use NoDBBaseConstructor, so no need to
  // manage DB list nodes at this level
  //gpBuild.set_communicators(pl_iter);
  gpEval.set_communicators(pl_iter);
} 


void NonDGPImpSampling::derived_free_communicators(ParLevLIter pl_iter)
{
  gpEval.free_communicators(pl_iter);
  //gpBuild.free_communicators(pl_iter);

  iteratedModel.free_communicators(pl_iter, maxEvalConcurrency);
}


/** Calculate the failure probabilities for specified probability levels 
    using Gaussian process based importance sampling. */
void NonDGPImpSampling::core_run()
{
  numPtsTotal = numSamples + numPtsAdd;
 
  // Build initial GP model.  This will be built over the initial LHS sample set
  // defined in the constructor.
  gpModel.build_approximation();
  
  gpCvars.resize(numEmulEval);
  RealVector temp_cvars;
  gpVar.resize(numEmulEval);
  gpMeans.resize(numEmulEval);
  indicator.resize(numPtsTotal);
  expIndicator.resize(numEmulEval);
  rhoDraw.resize(numEmulEval);
  normConst.resize(numPtsAdd);
  rhoMix.resize(numPtsTotal);
  //RealVector rhoEmul0(numEmulEval);
  //RealVector rhoEmul2(numEmulEval);
  int num_problem_vars=iteratedModel.acv();
  RealVector c_upper = iteratedModel.continuous_upper_bounds(), 
             c_lower = iteratedModel.continuous_lower_bounds();

  int i,j,k;
 
// This piece of code prints out the build points, not the approximation points.
// const Pecos::SurrogateData& gp_data = gpModel.approximation_data(0);
//  for (j = 0; j < numSamples; j++) {
//    Cout << " Surrogate Vars " << gp_data.continuous_variables(j) << '\n';
//    Cout << " Surrogate Response " << gp_data.response_function(j); 
//  }

// We have built the initial GP.  Now we need to go through, per response function
// and response level and calculate the failure probability. 
// We will need to add error handling:  we will only be calculating 
// results per response level, not probability level or reliability index.
   
  size_t resp_fn_count, level_count, iter;
  RealVector new_X;
  initialize_level_mappings();
  ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);

  for (resp_fn_count=0; resp_fn_count<numFunctions; resp_fn_count++) {
    size_t num_levels = requestedRespLevels[resp_fn_count].length();
    const Pecos::SurrogateData& gp_data = gpModel.approximation_data(resp_fn_count);
    for (level_count=0; level_count<num_levels; level_count++) {
      Cout << "Starting calculations for response function " << resp_fn_count+1 << '\n';
      Cout << "Starting calculations for level  " << level_count+1 << '\n';
      Cout << "Threshold level is " << requestedRespLevels[resp_fn_count][level_count] << '\n';
      Real z = requestedRespLevels[resp_fn_count][level_count];
       // Calculate indicator over the true function evaluations
      double cdfMult = (cdfFlag?1.0:-1.0);
      for (j = 0; j < numSamples; j++) {
        indicator(j) = static_cast<double>((z-gp_data.response_function(j))*cdfMult>0.0); 
        if (outputLevel > NORMAL_OUTPUT)
          Cout << "indicator(" << j << ")=" << indicator(j) << '\n';
      }
      
      // rho0 is the original PDF, rho1 is the compon importance pdf, rho2 is the 
      // distribution used to generate candidate points for the emulator
      // For the initial implementation, we assume rho1 is uniform, and 
      // rho0 and rho2 are uniform also with the same distribution.  
      double rho0const, rho1const, rho2const;
      rho0const = 1.0;
      for (i = 0; i < num_problem_vars; i++) 
        rho0const = rho0const/(c_upper[i]-c_lower[i]);
      rho1const = rho0const;
      rho2const = rho0const;
 
       // Here we loop over the number of points added, 
       // where each time we calculate the expected indicator function 
       // and add to the approximation. 
      for (k = 0; k < numPtsAdd; k++) { 
	// generate new set of emulator samples.
	// Note this will have a different seed each time.
        gpEval.run(pl_iter);
         // obtain results 
        const RealMatrix&  all_samples = gpEval.all_samples();
        const IntResponseMap& all_resp = gpEval.all_responses();
        for (i = 0; i< numEmulEval; i++) {
          temp_cvars = Teuchos::getCol(Teuchos::View,
	    const_cast<RealMatrix&>(all_samples), i);
          gpCvars[i] = temp_cvars;
          //Cout << "input is " << gpCvars[i] << '\n';
	  // update gpModel currentVariables for use in approx_variances()
	  gpModel.continuous_variables(temp_cvars);
          gpVar[i]
	    = gpModel.approximation_variances(gpModel.current_variables());
          //Cout << "variance is " << gpVar[i];
        }

        IntRespMCIter resp_it = all_resp.begin();
        for (j=0, resp_it=all_resp.begin(); j<numEmulEval; ++j, ++resp_it) {
          RealVector temp_resp(numFunctions);
            for (i=0; i<numFunctions; i++)
              temp_resp(i) = resp_it->second.function_value(i);
            gpMeans[j]=temp_resp;
            //Cout << "output is " << gpMeans[j] << '\n';
        }
          
       // calculate expected indicator function;
        expIndicator = calcExpIndicator(resp_fn_count,z);
       // calculate distribution pdfs required to calculate the draw distribution
       // distribution_pdf(rhoEmul2);
       // distribution_pdf(rhoEmul0);
       // FOR NOW, we assume distribution 0 and 2 are the same, 
       // but in general rhoDrawThis = expIndicator * rhoEmul0/rhoEmul2;
        for (i = 0; i< numEmulEval; i++) 
          rhoDraw(i) = expIndicator(i)*rho0const/rho2const; 
      	
       // calculate the normalization constant
        Cout << "numEmulEval " << numEmulEval << '\n';
        Cout << "size exp indicator " << expIndicator.length() << '\n';
	double temp_sum_nc=0.0;
        normConst(k)=0.0;
        for (j = 0; j < numEmulEval; j++) {
	  temp_sum_nc+=rhoDraw(j);
        }
	temp_sum_nc/=numEmulEval;
        normConst(k)=temp_sum_nc;
        Real temp_norm_const = normConst(k);
        Cout << "norm const " << k << "=" << temp_sum_nc << "=" << normConst(k) <<  "\n";
       // calculate the draw distribution
        rhoDrawThis.resize(0);
        xDrawThis.resize(0);
        expIndThis.resize(0);
        calcRhoDraw();
          
        iter = 1;
        while ((iter<20) && (temp_norm_const*numEmulEval<25)) {
	  iter = iter+1;
          gpEval.run(pl_iter);
           // obtain results 
          const RealMatrix&  this_samples = gpEval.all_samples();
          const IntResponseMap& this_resp = gpEval.all_responses();
          for (i = 0; i< numEmulEval; i++) {
	    temp_cvars = Teuchos::getCol(Teuchos::View,
	       const_cast<RealMatrix&>(this_samples), i);
            gpCvars[i] = temp_cvars;
            //Cout << "input is " << gpCvars[i] << '\n';
	    // update gpModel currentVariables for use in approx_variances()
	    gpModel.continuous_variables(temp_cvars);
            gpVar[i]
	      = gpModel.approximation_variances(gpModel.current_variables());
            //Cout << "variance is " << gpVar[i];
           }

          resp_it = this_resp.begin();
          for (j=0, resp_it=this_resp.begin(); j<numEmulEval; ++j, ++resp_it) {
            RealVector temp_resp(numFunctions);
            for (i=0; i<numFunctions; i++)
              temp_resp(i) = resp_it->second.function_value(i);
            gpMeans[j]=temp_resp;
            //Cout << "output is " << gpMeans[j] << '\n';
          }
          
       // calculate expected indicator function;
          expIndicator = calcExpIndicator(resp_fn_count,z);
          for (j = 0; j < numEmulEval; j++) 
            rhoDraw(j)=expIndicator(j)*rho0const/rho2const;
          Real temp_norm_this=0.0;
          for (j = 0; j < numEmulEval; j++) 
	    temp_norm_this+=rhoDraw(j);
          temp_norm_this/=numEmulEval;
          temp_norm_const = temp_norm_const+temp_norm_this;
          calcRhoDraw();
        }     

       // xDrawThis, rhoDrawThis, and expIndThis should be populated now
        normConst(k)=temp_norm_const/iter;
        if (outputLevel > NORMAL_OUTPUT) 
          Cout << "NormConst " << k << " =  " << normConst(k) << '\n';
 
        int num_eval_kept = xDrawThis.size();  
        Real est_prob_hit_failregion; 
 
        if (num_eval_kept==0) {
	  normConst(k)=0.0;
          sampleRhoOne.run(pl_iter);
         // obtain results 
          const RealMatrix&  rho1_samples = sampleRhoOne.all_samples();
          // For now, we always only draw one sample
            new_X = Teuchos::getCol(Teuchos::View,
	      const_cast<RealMatrix&>(rho1_samples), 0);
            Cout << "Draw from Rho One is " << new_X << '\n';
        }  
        else 
          new_X = drawNewX(k);
         
         // add new_X to the build points and append approximation
        iteratedModel.continuous_variables(new_X);
        iteratedModel.evaluate();
        IntResponsePair resp_truth(iteratedModel.evaluation_id(),
                                   iteratedModel.current_response());
        gpModel.append_approximation(iteratedModel.current_variables(), resp_truth, true);
	indicator(numSamples+k) = static_cast<double>((z-gp_data.response_function(numSamples+k))*cdfMult>0.0); 
        //if (gp_data.response_function(numSamples+k-1)<z) 
	//indicator(numSamples+k-1)=1;
        //else indicator(numSamples+k-1)=0;
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
        rhoOne(j)=rho1const; //for uniform this should be 1.0/prod(xmax-xmin) across all dimensions... ok now I see this isn't rhoOne it's rhoOne/rhoZero... if rho0=rho1=rho2 this should be ok
  
      for (j = 0; j < numPtsTotal; j++)
        rhoMix(j)=rhoOne(j)*numSamples;
         
      for (j = numPtsAdd-1; j>=0; j--){
        if (normConst(j)==0.0) {
          for (k = 0; k < numPtsTotal; k++) 
            rhoMix(k)=rhoMix(k)+rhoOne(k);
        }
        else {
          RealVector this_mean;
          RealVector this_var;
          RealVector exp_ind_this(numPtsTotal);
          for (k = 0; k < numPtsTotal; k++){ 
            gpModel.continuous_variables(gp_final_data[k]);
            gpModel.evaluate();
            this_mean = gpModel.current_response().function_values();
            this_var
	      = gpModel.approximation_variances(gpModel.current_variables());
            exp_ind_this(k) = calcExpIndPoint(resp_fn_count,z,this_mean,this_var);
            if (outputLevel > NORMAL_OUTPUT) 
              Cout << "exp_ind_final " << k << " " <<  exp_ind_this(k) << '\n';
          }
          for (k = 0; k < numPtsTotal; k++) 
            rhoMix(k)=rhoMix(k)+exp_ind_this(k)*rho0const/normConst(j);
	  //the 1.0 here is reall rhoZero/rhoZero (ok for rho0=rho1=rho2)
          gpModel.pop_approximation(false, true);
          //gpModel.update_approximation(true);
          Cout << "Size of build data set " << gp_data.points() << '\n';
        } 
//Since we need to evaluate the SUCCESSIVE SEQUENCES of GPs built using numSamples-->numPtsTotal
//it might be most efficient to "pop" the data and go backward: 
//listStudy.run(pl_iter);
//obtain results and expected indicator functions
//gpModel.pop_approximation();
//
      }            
      if (outputLevel > NORMAL_OUTPUT) {
        Cout << "rhoMix " << rhoMix << '\n';
        Cout << "indicator " << indicator << '\n'; 
      }
      for (j = 0; j < numPtsTotal; j++) {
	Real yada=rhoMix(j);
        rhoMix(j)=yada/numPtsTotal;
      }
      Real prob_mix=0.0;
      for (j = 0; j < numPtsTotal; j++) 
        prob_mix+=rho0const*indicator(j)/rhoMix(j);
      //the 1.0 here is reall rhoZero/rhoZero (ok for rho0=rho1=rho2)
      prob_mix/=numPtsTotal;
      Cout << "Prob Mix IS " << prob_mix << '\n'; 
 
      Real fract_fail_mix = 0.0;
      for (j = 0; j < numPtsTotal; j++) 
        fract_fail_mix+=indicator(j);
      fract_fail_mix/=numPtsTotal;
      Cout << "Fraction Fail IS " << fract_fail_mix << '\n'; 
      finalProb = prob_mix; 
      computedProbLevels[resp_fn_count][level_count]=finalProb;  
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
  Real yada2;
  //Cout << "templength " << templength << '\n'; 
  for (i=0; i<templength; i++) {
    Real yada=rhoDrawThis(i);
    rhoDrawThis(i)=yada/normConst(this_k);
    if (i==0)
      binEnds(i)=rhoDrawThis(i);
    else{
      yada=rhoDrawThis(i);
      yada2=binEnds(i-1);
      binEnds(i)=yada+yada2;
    }
  }
  cum_sum = binEnds(templength-1);
  //Cout << "Cum Sum"  << cum_sum << '\n';
  //Cout << "BinEnds " << binEnds << '\n';
  //Cout << "RhoDrawThis " << rhoDrawThis << '\n';
  for (i=0; i<templength; i++) {
    yada2=binEnds(i);
    binEnds(i)=yada2/cum_sum;
  }
  //std::srand(randomSeed);
  double rand_cdf = (double)std::rand()/RAND_MAX;
  //Cout << "randcdf " << rand_cdf << '\n';
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
      // for now this is OK because rho0const = rho2const, will need to change this
      rhoDrawThis(templength)=expIndicator(i);
      templength=templength+1;
    }
  }    
  
  //for (i = 0; i< templength; i++) {
  //  Cout << "xDrawThis  " << i << xDrawThis[i] << '\n';
  //  Cout << "rhoDrawThis  "  << rhoDrawThis[i] << '\n';
  //}
}

RealVector NonDGPImpSampling::calcExpIndicator(const int resp_fn_count, const Real respThresh)
{
  int i, j;
  RealVector ei(numEmulEval);

  Real cdf,snv,stdv;
  for (i = 0; i< numEmulEval; i++) {
    //Cout << "GPmean  " << gpMeans[i][resp_fn_count];
    //Cout << "GPvar  " << gpVar[i][resp_fn_count];
    snv = (respThresh-gpMeans[i][resp_fn_count])*(cdfFlag?1.0:-1.0);
    //this conditional sign maps the problem to the case where the mean being
    //"below" the threshold (i.e. snv > 0) indicates "mostly failure" and the
    //mean being "above" the threshold (i.e. snv < 0) indicates "mostly not
    //failure" this allows the mapped problem to ALWAYS use the cdf (instead
    //of complimentary cdf)

    stdv = std::sqrt(gpVar[i][resp_fn_count]); 
    if(std::fabs(snv)>=std::fabs(stdv)*50.0) {
      //this will trap the denominator=0.0 case even if numerator=0.0
      ei(i)=(snv>=0.0)?1.0:0.0;
      //the mean being exactly at the threshold when variance=0.0 is 
      //considered to indicate failure
    }
    else{
      snv/=stdv;
      ei(i)= Pecos::NormalRandomVariable::std_cdf(snv);
      //the expected indicator is the fraction of the mapped problem's cdf
      //that fails, the simple mapping is at most a change in sign of the
      //snv and might be the identity mapping (not even a change in sign)
    }

    //Cout << "EI " << ei(i) << " respThresh= " << respThresh << " mu= " << gpMeans[i][resp_fn_count] << " stdv= " << stdv << '\n';
  }    
  return ei;
}

Real NonDGPImpSampling::calcExpIndPoint(const int resp_fn_count, const Real respThresh, const RealVector this_mean, const RealVector this_var)
{
  int i, j;
  Real ei;

  Real cdf,snv,stdv;
  //  Cout << "GPmean  " << this_mean(resp_fn_count);
  //  Cout << "GPvar  " << this_var(resp_fn_count);
  snv = (respThresh-this_mean(resp_fn_count))*(cdfFlag?1.0:-1.0);
    //this conditional sign maps the problem to the case where the mean being
    //"below" the threshold (i.e. snv > 0) indicates "mostly failure" and the
    //mean being "above" the threshold (i.e. snv < 0) indicates "mostly not
    //failure" this allows the mapped problem to ALWAYS use the cdf (instead
    //of complimentary cdf)
  stdv = std::sqrt(this_var(resp_fn_count)); 
  if(std::fabs(snv)>=std::fabs(stdv)*50.0) {
    //this will trap the denominator=0.0 case even if numerator=0.0
    ei=(snv>=0.0)?1.0:0.0;
    //the mean being exactly at the threshold when variance=0.0 is 
    //considered to indicate failure
  }
  else{
    snv/=stdv;
    ei= Pecos::NormalRandomVariable::std_cdf(snv);
    //the expected indicator is the fraction of the mapped problem's cdf
    //that fails, the simple mapping is at most a change in sign of the
    //snv and might be the identity mapping (not even a change in sign)
  }

  //Cout << "EI " << ei << " respThresh= " << respThresh << " mu= " << this_mean(resp_fn_count) << " stdv= " << stdv << '\n';
      
  return ei;
}

void NonDGPImpSampling::print_results(std::ostream& s)
{
  if (statsFlag) {
    s << "\nStatistics based on the importance sampling calculations:\n";
    print_level_mappings(s);
  }
}

} // namespace Dakota

/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDIncremLHSSampling
//- Description: Implementation code for the incremental version of LHS class
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDIncremLHSSampling.hpp"
#include "dakota_system_defs.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
#include "DakotaVariables.hpp"
//#ifdef HAVE_BOOST
#include "pecos_data_types.hpp"
/*
#elif HAVE_GSL
#include "gsl/gsl_cdf.h"
#endif
*/
#include "pecos_stat_util.hpp"
#include <algorithm>

//#define DEBUG

static const char rcsId[]="@(#) $Id: NonDIncremLHSSampling.cpp 6715 2010-04-02 21:58:15Z wjbohnh $";

namespace Dakota {
  extern PRPCache data_pairs;

RealArray NonDIncremLHSSampling::rawData;


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDIncremLHSSampling::
NonDIncremLHSSampling(ProblemDescDB& problem_db, Model& model):
  NonDSampling(problem_db, model),
  varBasedDecompFlag(probDescDB.get_bool("method.variance_based_decomp")),
  previousSamples(probDescDB.get_int("method.previous_samples"))
{ }


NonDIncremLHSSampling::~NonDIncremLHSSampling()
{ }


bool NonDIncremLHSSampling::rank_sort(const int& x, const int& y)
{ return rawData[x]<rawData[y]; }


/** Generate incremental samples.  Loop over the set of samples and 
    compute responses.  Compute statistics on the set of responses 
    if statsFlag is set. */
void NonDIncremLHSSampling::quantify_uncertainty()
{
  // Note: incremental LHS does not support variance based decomposition

#ifdef DEBUG
  Cout << "lhs1 test sample type " << submethod_enum_to_string(sampleType)
       << '\n';
#endif //DEBUG

  if (sampleType == SUBMETHOD_INCREMENTAL_LHS &&
      previousSamples*2 != numSamples) {
    Cout << "Error: For incremental LHS sampling, the number of samples "
 	 << "must double each time.  The number of samples must be 2 times " 
         << "the number of previous samples." << std::endl;
    abort_handler(-1);
  }

  if (numGumbelVars > 0 || numFrechetVars > 0) {
    Cout << "Error: The Gumbel and Frechet distributions are not "
 	 << "supported in incremental LHS sampling. " << std::endl;
    abort_handler(-1);
  }

  if (numPoissonVars > 0 || numBinomialVars > 0 || numNegBinomialVars > 0 ||
      numGeometricVars > 0 || numHyperGeomVars > 0) {
    Cout << "Error: Discrete distribution types are not "
 	 << "supported in incremental LHS sampling.  " << std::endl;
    abort_handler(-1);
  }
  
  if (numHistogramBinVars > 0 || numHistogramPtIntVars > 0 ||
      numHistogramPtStringVars > 0 || numHistogramPtRealVars > 0) {
    Cout << "Error: Histogram variables, including point and bin, are not "
 	 << "supported in incremental LHS sampling. " << std::endl;
    abort_handler(-1);
  }
  
  numSamples = previousSamples;
  sampleRanksMode = GET_RANKS;

  //NOW:  instead of getting the original parameter sets by calling 
  //LHS with the seed, we read it from the restart file.

  // Now read in variable and response data
  size_t num_evals = data_pairs.size();
  if (num_evals > numSamples){
    Cout << "Warning: The number of function evaluations in the restart " 
         << "file does not equal the number of previous samples.  "  
         << "Only the previous number of samples will be taken. \n";
  }          
  else if (num_evals < numSamples) {
    Cerr << "Error: Number of function evaluations in the restart file "
	 << "is less than the number of previous samples.  Incremental " 
	 << "sampling cannot proceed."  << std::endl;
    abort_handler(-1);
  }

  size_t i, j, k;
  RealVectorArray sample_values_first(numSamples); // views OK
  PRPCacheCIter prp_iter;
  for (i=0, prp_iter = data_pairs.begin(); i<numSamples; ++i, ++prp_iter)
    sample_values_first[i] = prp_iter->variables().continuous_variables();
#ifdef DEBUG
  Cout << "\nsample1\n" << sample_values_first << '\n';
#endif

  if (sampleType == SUBMETHOD_INCREMENTAL_RANDOM) {
    // for random sampling, obtain a new set of samples of the full size
    numSamples = samplesRef;
    varyPattern = true; 
    get_parameter_sets(iteratedModel);
  }
  else if (sampleType == SUBMETHOD_INCREMENTAL_LHS) {
    // for LHS, determine rank of current sample, rank of second "fill in" 
    // sample, and ranks of combined sample
    IntArray rank_col(numSamples), final_rank(numSamples);
    rawData.resize(numSamples);
    RealMatrix sample_ranks_first(numContinuousVars, numSamples, false);
    
    // store sample values and ranks returned by 1st get_parameter_sets() call
    for (i=0; i<numContinuousVars; ++i) {
      for (size_t rank_count = 0; rank_count < numSamples; rank_count++){
        rank_col[rank_count] = rank_count;
        rawData[rank_count] = sample_values_first[rank_count][i];
      }
      std::sort(rank_col.begin(),rank_col.end(),rank_sort);
      for (j=0; j<numSamples; ++j)
        final_rank[rank_col[j]] = j;
      for (j=0; j<numSamples; ++j)
        sample_ranks_first(i,j) = (Real)final_rank[j] + 1.;
    }

#ifdef DEBUG
    Cout << "rank1\n" << sample_ranks_first << '\n';
#endif //DEBUG

    // distribution specific calculations to get rank values of samples 
    double temp_x, mean_x, sigma_x, x, scaled_x, lwr, mode, upr, alpha, beta;
    Pecos::AleatoryDistParams& adp
      = iteratedModel.aleatory_distribution_parameters();
    const Pecos::RealVector& n_means    = adp.normal_means();
    const Pecos::RealVector& n_std_devs = adp.normal_std_deviations();
    const Pecos::RealVector& n_l_bnds   = adp.normal_lower_bounds();
    const Pecos::RealVector& n_u_bnds   = adp.normal_upper_bounds();
    const Pecos::RealVector& ln_means   = adp.lognormal_means();
    const Pecos::RealVector& ln_std_devs  = adp.lognormal_std_deviations();
    const Pecos::RealVector& ln_lambdas   = adp.lognormal_means();
    const Pecos::RealVector& ln_zetas     = adp.lognormal_zetas();
    const Pecos::RealVector& ln_err_facts = adp.lognormal_error_factors();
    const Pecos::RealVector& ln_l_bnds  = adp.lognormal_lower_bounds();
    const Pecos::RealVector& ln_u_bnds  = adp.lognormal_upper_bounds();
    const Pecos::RealVector& u_low      = adp.uniform_lower_bounds();
    const Pecos::RealVector& u_up       = adp.uniform_upper_bounds();
    const Pecos::RealVector& lu_low     = adp.loguniform_lower_bounds();
    const Pecos::RealVector& lu_up      = adp.loguniform_upper_bounds();
    const Pecos::RealVector& tri_low    = adp.triangular_lower_bounds();
    const Pecos::RealVector& tri_mode   = adp.triangular_modes();
    const Pecos::RealVector& tri_up     = adp.triangular_upper_bounds();
    const Pecos::RealVector& exp_beta   = adp.exponential_betas();
    const Pecos::RealVector& beta_alpha = adp.beta_alphas();
    const Pecos::RealVector& beta_beta  = adp.beta_betas();
    const Pecos::RealVector& beta_low   = adp.beta_lower_bounds();
    const Pecos::RealVector& beta_up    = adp.beta_upper_bounds();
    const Pecos::RealVector& gamma_alpha   = adp.gamma_alphas();
    const Pecos::RealVector& gamma_beta    = adp.gamma_betas();
    const Pecos::RealVector& weibull_alpha = adp.weibull_alphas();
    const Pecos::RealVector& weibull_beta  = adp.weibull_betas();

    // initialize the matrix which identifies whether the sample 2 rank should
    // be switched or not
    BoolDequeArray switch_ranks(numContinuousVars);
    for (i=0; i<numContinuousVars; i++)
      switch_ranks[i] = BoolDeque(numSamples, false);

    size_t cntr = 0;

    //NORMAL
    for (k=0; k<numNormalVars; k++, cntr++) {
      mean_x = n_means[k];
      sigma_x = n_std_devs[k];
      Pecos::normal_dist norm(mean_x,sigma_x);

      for (i=0; i<numSamples; i++) {
        temp_x = sample_values_first[i][cntr];
        if (n_l_bnds[k] > -DBL_MAX || n_u_bnds[k] < DBL_MAX) {
      	  Real Phi_l = (n_l_bnds[k] > -DBL_MAX) ? 
	    Pecos::Phi((n_l_bnds[k]-mean_x)/sigma_x): 0.;
      	  Real Phi_u = (n_u_bnds[k] < DBL_MAX) ? 
	    Pecos::Phi((n_u_bnds[k]-mean_x)/sigma_x) : 1.;
          Real Phi_x = Pecos::Phi((temp_x-mean_x)/sigma_x);
          Real cdf = (Phi_x-Phi_l)/(Phi_u - Phi_l);
	  if (cdf < (2*sample_ranks_first(cntr,i)-1)/(numSamples*2) ) {
	    int index_temp = static_cast<int>(sample_ranks_first(cntr,i))-1;
	    switch_ranks[cntr][index_temp] = true;
          }
        }
//#ifdef HAVE_BOOST
        else if ( bmth::cdf(norm, temp_x) <
		  (2*sample_ranks_first(cntr,i)-1)/(numSamples*2) ) {
	  int index_temp = static_cast<int>(sample_ranks_first(cntr,i))-1;
	  switch_ranks[cntr][index_temp] = true;
        }
/*
#elif HAVE_GSL
        else if ( gsl_cdf_gaussian_P(temp_x,sigma_x) <
		  (2*sample_ranks_first(cntr,i)-1)/(numSamples*2) ) {
	  int index_temp = static_cast<int>(sample_ranks_first(cntr,i))-1;
	  switch_ranks[cntr][index_temp] = true;
        }
#endif // HAVE_GSL
*/
      }
    }

    //LOGNORMAL
    for (k=0; k<numLognormalVars; k++, cntr++) {
      Real lambda, zeta;
      Pecos::params_from_lognormal_spec(ln_means, ln_std_devs, ln_lambdas,
					ln_zetas, ln_err_facts, k, lambda,zeta);
      Pecos::normal_dist norm(lambda,zeta);

      for (i=0; i<numSamples; i++) {
        temp_x = log(sample_values_first[i][cntr]);
        if (ln_l_bnds[k] > 0 || ln_u_bnds[k] < DBL_MAX) {
      	  Real Phi_l = (ln_l_bnds[k] > 0 ) ? 
	    Pecos::Phi((log(ln_l_bnds[k])-lambda)/zeta): 0.;
      	  Real Phi_u = (ln_u_bnds[k] < DBL_MAX) ? 
	    Pecos::Phi((log(ln_u_bnds[k])-lambda)/zeta) : 1.;
          Real Phi_x = Pecos::Phi((temp_x-lambda)/(zeta));
          Real cdf = (Phi_x - Phi_l)/(Phi_u - Phi_l);
	  if (cdf < (2*sample_ranks_first(cntr,i)-1)/(numSamples*2) ) {
	    int index_temp = static_cast<int>(sample_ranks_first(cntr,i))-1;
	    switch_ranks[cntr][index_temp] = true;
          }
        }
//#ifdef HAVE_BOOST
	else if ( bmth::cdf(norm, temp_x) <
		  (2*sample_ranks_first(cntr,i)-1)/(numSamples*2) ) {
	  int index_temp = static_cast<int>(sample_ranks_first(cntr,i))-1;
	  switch_ranks[cntr][index_temp] = true;
        }
/*
#elif HAVE_GSL
	else if ( gsl_cdf_gaussian_P(temp_x,zeta) <
		  (2*sample_ranks_first(cntr,i)-1)/(numSamples*2) ) {
	  int index_temp = static_cast<int>(sample_ranks_first(cntr,i))-1;
	  switch_ranks[cntr][index_temp] = true;
        }
#endif
*/
      }
    }

    //UNIFORM
    for (k=0; k<numUniformVars; k++, cntr++) {
      for (i=0; i<numSamples; i++) {
        temp_x = (sample_values_first[i][cntr] - u_low[k])/(u_up[k]-u_low[k]);
        if (temp_x < (2*sample_ranks_first(cntr,i)-1)/(numSamples*2)){
	  int index_temp = static_cast<int>(sample_ranks_first(cntr,i))-1;
	  switch_ranks[cntr][index_temp] = true;
        }
      }
    }

    //LOGUNIFORM
    for (k=0; k<numLoguniformVars; k++, cntr++) {
      for (i=0; i<numSamples; i++) {
        temp_x = (log(sample_values_first[i][cntr])-log(lu_low[k]))
	  / (log(lu_up[k])-log(lu_low[k]));
        if (temp_x < (2*sample_ranks_first(cntr,i)-1)/(numSamples*2)){
	  int index_temp = static_cast<int>(sample_ranks_first(cntr,i))-1;
	  switch_ranks[cntr][index_temp] = true;
        }
      }
    }

    //TRIANGULAR
    for (k=0; k<numTriangularVars; k++, cntr++) {
      lwr = tri_low[k];
      mode = tri_mode[k];
      upr = tri_up[k];
      for (i=0; i<numSamples; i++) {
        x = sample_values_first[i][cntr];
        Real tri_cdf = (x < mode) ? std::pow(x-lwr,2.)/(upr-lwr)/(mode-lwr) :
          ((mode-lwr) - (x+mode-2*upr)*(x-mode)/(upr-mode))/(upr-lwr);
        if (tri_cdf < (2*sample_ranks_first(cntr,i)-1)/(numSamples*2)){
	  int index_temp = static_cast<int>(sample_ranks_first(cntr,i))-1;
	  switch_ranks[cntr][index_temp] = true;
        }
      }
    }	      

    //EXPONENTIAL
    for (k=0; k<numExponentialVars; k++, cntr++) {
      beta = exp_beta[k];
      for (i=0; i<numSamples; i++) {
//#ifdef HAVE_BOOST
        Pecos::exponential_dist exp1(beta);
        if ( bmth::cdf(exp1,sample_values_first[i][cntr]) <
	     (2*sample_ranks_first(cntr,i)-1)/(numSamples*2)){
	  int index_temp = static_cast<int>(sample_ranks_first(cntr,i))-1;
	  switch_ranks[cntr][index_temp] = true;
        }
/*
#elif HAVE_GSL
        if ( gsl_cdf_exponential_P(sample_values_first[i][cntr], beta) <
	     (2*sample_ranks_first(cntr,i)-1)/(numSamples*2)){
	  int index_temp = static_cast<int>(sample_ranks_first(cntr,i))-1;
	  switch_ranks[cntr][index_temp] = true;
        }
#endif
*/
      }
    }	      

    //BETA
    for (k=0; k<numBetaVars; k++, cntr++) {
      alpha = beta_alpha[k];
      beta = beta_beta[k];
      lwr = beta_low[k];
      upr = beta_up[k];
      for (i=0; i<numSamples; i++) {
        scaled_x = (sample_values_first[i][cntr]-lwr)/(upr - lwr);
//#ifdef HAVE_BOOST
        Pecos::beta_dist beta1(alpha,beta);
        if (bmth::cdf(beta1,scaled_x) <
	     (2*sample_ranks_first(cntr,i)-1)/(numSamples*2) ) {
	  int index_temp = static_cast<int>(sample_ranks_first(cntr,i))-1;
	  switch_ranks[cntr][index_temp] = true;
        }
/*
#elif HAVE_GSL
	// GSL beta passes alpha followed by beta
        if ( gsl_cdf_beta_P(scaled_x, alpha, beta) <
	     (2*sample_ranks_first(cntr,i)-1)/(numSamples*2) ) {
	  int index_temp = static_cast<int>(sample_ranks_first(cntr,i))-1;
	  switch_ranks[cntr][index_temp] = true;
        }
#endif
*/
      }
    }	      

    //GAMMA
    for (k=0; k<numGammaVars; k++, cntr++) {
      alpha = gamma_alpha[k];
      beta = gamma_beta[k];
      for (i=0; i<numSamples; i++) {
//#ifdef HAVE_BOOST
        Pecos::gamma_dist gamma1(alpha,beta);
        if ( bmth::cdf(gamma1,sample_values_first[i][cntr]) <
	     (2*sample_ranks_first(cntr,i)-1)/(numSamples*2)){
	  int index_temp = static_cast<int>(sample_ranks_first(cntr,i))-1;
	  switch_ranks[cntr][index_temp] = true;
         }
/*
#elif HAVE_GSL
        if ( gsl_cdf_gamma_P(sample_values_first[i][cntr], alpha, beta) <
	     (2*sample_ranks_first(cntr,i)-1)/(numSamples*2)){
	  int index_temp = static_cast<int>(sample_ranks_first(cntr,i))-1;
	  switch_ranks[cntr][index_temp] = true;
        }
#endif
*/
      }
    }	      

    //WEIBULL
    for (k=0; k<numWeibullVars; k++, cntr++) {
      alpha = weibull_alpha[k];
      beta = weibull_beta[k];
      for (i=0; i<numSamples; i++) {
//#ifdef HAVE_BOOST
        Pecos::weibull_dist weibull1(alpha,beta);
        if ( bmth::cdf(weibull1,sample_values_first[i][cntr]) <
	     (2*sample_ranks_first(cntr,i)-1)/(numSamples*2) ) {
	  int index_temp = static_cast<int>(sample_ranks_first(cntr,i))-1;
	  switch_ranks[cntr][index_temp] = true;
        }
/*
#elif HAVE_GSL
        if ( gsl_cdf_weibull_P(sample_values_first[i][cntr],beta,alpha) <
	     (2*sample_ranks_first(cntr,i)-1)/(numSamples*2) ) {
	  int index_temp = static_cast<int>(sample_ranks_first(cntr,i))-1;
	  switch_ranks[cntr][index_temp] = true;
        }
#endif
*/
      }
    }	      

#ifdef DEBUG
    //Cout << "switch rank1\n" << switch_ranks << '\n';
#endif //DEBUG

    varyPattern = true;
    //numSamples = previousSamples;
    //sampleRanksMode = GET_RANKS;
    get_parameter_sets(iteratedModel);
    get_parameter_sets(iteratedModel);
    // store sample ranks returned by 2nd get_parameter_sets() call
    RealMatrix sample_ranks_second = sampleRanks;

#ifdef DEBUG
    Cout << "lhs2 test\nsample2\n";
    write_data(Cout, allSamples, false, true, true);
    Cout << "rank2\n" << sample_ranks_second;
#endif // DEBUG

    // calculate the combined ranks  
    sampleRanks.shapeUninitialized(numContinuousVars, samplesRef);
    for (i=0; i<samplesRef; i++) {
      for (j=0; j<numContinuousVars; j++) {
        if (i < numSamples) {
	  int index_temp = static_cast<int>(sample_ranks_first(j,i))-1;
	  sampleRanks(j,i) = (switch_ranks[j][index_temp]) ?
	    2*sample_ranks_first(j,i)-1 : 2*sample_ranks_first(j,i);
        }
        else {
	  int r2_index = i - numSamples;
	  int index_temp = static_cast<int>(sample_ranks_second(j,r2_index))-1;
	  sampleRanks(j,i) = (switch_ranks[j][index_temp]) ?
	    2*sample_ranks_second(j,r2_index) :
	    2*sample_ranks_second(j,r2_index)-1;
        }
      }
    } 

#ifdef DEBUG
    Cout << "\ncombined ranks\n" << sampleRanks;
#endif //DEBUG

    // send LHS the full sampleRanks matrix for the combined set
    // and get back a sample that maintains the structure   
    numSamples = samplesRef;
#ifdef DEBUG
    sampleRanksMode = SET_GET_RANKS;
#else
    sampleRanksMode = SET_RANKS;
#endif //DEBUG
    get_parameter_sets(iteratedModel); // using sampleRanks as input

#ifdef DEBUG
    Cout << "rank_combined\n" << sampleRanks << '\n';// updated by SET_GET_RANKS
#endif //DEBUG
  }

  // need to put the first set back  
  for (i=0; i<previousSamples; i++)
    copy_data(sample_values_first[i], allSamples[i], (int)numContinuousVars);
  
#ifdef DEBUG
  numSamples = samplesRef;
  Cout <<"\nallSamples\n";
  write_data(Cout, allSamples, false, true, true);
#endif //DEBUG

  // evaluate full parameter set of size samplesRef, where the first half
  // should be intercepted via restart file duplication detection
  evaluate_parameter_sets(iteratedModel, true, false);
  compute_statistics(allSamples, allResponses);
}


void NonDIncremLHSSampling::print_results(std::ostream& s)
{
  if (statsFlag && !varBasedDecompFlag) {
    s << "\nStatistics based on " << numSamples << " samples:\n";
    print_statistics(s);
  }
}

} // namespace Dakota

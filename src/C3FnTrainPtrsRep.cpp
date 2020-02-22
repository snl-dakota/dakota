/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "C3FnTrainPtrsRep.hpp"
#include "dakota_data_types.hpp"
#include <cmath>

namespace Dakota {


void C3FnTrainPtrsRep::ft_derived_functions_init_null()
{
  ft_derived_fns.allocated = false;

  ft_derived_fns.ft_nonrand = NULL;
  ft_derived_fns.ft_squared = NULL;
  ft_derived_fns.ft_squared_nonrand = NULL;
  //ft_derived_fns.ft_cubed = NULL;
  //ft_derived_fns.ft_tesseracted = NULL;

  //ft_derived_fns.ft_constant_at_mean = NULL;
  ft_derived_fns.ft_diff_from_mean = NULL;
  ft_derived_fns.ft_diff_from_mean_squared = NULL;
  ft_derived_fns.ft_diff_from_mean_cubed   = NULL;
  ft_derived_fns.ft_diff_from_mean_tesseracted = NULL;

  //ft_derived_fns.ft_diff_from_mean_normalized = NULL;
  //ft_derived_fns.ft_diff_from_mean_normalized_squared = NULL;
  //ft_derived_fns.ft_diff_from_mean_normalized_cubed   = NULL;
}


void C3FnTrainPtrsRep::
ft_derived_functions_create_av(struct MultiApproxOpts * opts,
			       const SizetArray& rand_indices, Real round_tol)
{
  // For all-variables mode, ft and ft_squared are partially integrated (random
  // vars) and partially interrogated (for a particular non-random x).  In this
  // case, the only additional ft data is forming ft_squared.  No moments are
  // estimated at this time (since x is passed at moment eval time).

  struct FunctionTrain * ft_tmp = function_train_product(ft, ft);
  ft_derived_fns.ft_squared = function_train_round(ft_tmp, round_tol, opts);
  function_train_free(ft_tmp);

  size_t  num_rand = rand_indices.size();
  size_t* rand_ptr = const_cast<size_t*>(&rand_indices[0]);
  ft_derived_fns.ft_nonrand = function_train_integrate_weighted_subset(ft,
    num_rand, rand_ptr);
  ft_derived_fns.ft_squared_nonrand = function_train_integrate_weighted_subset(
    ft_derived_fns.ft_squared, num_rand, rand_ptr);

  ft_derived_fns.allocated = true;
}


void C3FnTrainPtrsRep::
ft_derived_functions_create(struct MultiApproxOpts * opts, bool full_stats,
			    Real round_tol)
{
  struct FunctionTrain * ft_tmp = NULL;
  Real var, stdev;
  // arithmetic tolerance (same as for combination axpy)
  //   --> how accurate do you want the result of an algebraic manipulation?
  // other is a regression tolerance that is more like a noise tolerance

  // Note: standardized moments are not used by Dakota, as standardization only
  // occurs in NonDExpansion::print_moments().  We go ahead and compute them
  // here for use in accessors since additional fn trains are not required.
  // Conversely, raw moments are not computed since they incur overhead.

  // -------------
  // First moment:
  // -------------

  ft_derived_fns.first_moment = function_train_integrate_weighted(ft);

  // -------------
  // Second moment:
  // -------------

  // Skip raw moment: (this FT is used for allVars case above)
  //ft_tmp = function_train_product(ft, ft);
  //ft_derived_fns.ft_squared = function_train_round(ft_tmp, round_tol, opts);
  //function_train_free(ft_tmp);
  //ft_derived_fns.second_moment
  //  = function_train_integrate_weighted(ft_derived_fns.ft_squared);

  //ft_derived_fns.ft_constant_at_mean =
  ft_tmp = function_train_constant(-ft_derived_fns.first_moment, opts);
  ft_derived_fns.ft_diff_from_mean
    = function_train_sum(ft, /*ft_derived_fns.ft_constant_at_mean*/ft_tmp);
  function_train_free(ft_tmp);
  ft_tmp = function_train_product(ft_derived_fns.ft_diff_from_mean,
				  ft_derived_fns.ft_diff_from_mean);
  ft_derived_fns.ft_diff_from_mean_squared
    = function_train_round(ft_tmp, round_tol, opts);
  function_train_free(ft_tmp);

  var = ft_derived_fns.second_central_moment =
    function_train_integrate_weighted(ft_derived_fns.ft_diff_from_mean_squared);
  stdev = ft_derived_fns.std_dev = std::sqrt(var);

  // -------------
  // Third moment:
  // -------------

  if (full_stats) {
    // Skip raw moment:
    //ft_tmp = function_train_product(ft_derived_fns.ft_squared, ft);    
    //ft_derived_fns.ft_cubed = function_train_round(ft_tmp, round_tol, opts);
    //function_train_free(ft_tmp);
    //ft_derived_fns.third_moment
    //  = function_train_integrate_weighted(ft_derived_fns.ft_cubed);

    ft_tmp = function_train_product(ft_derived_fns.ft_diff_from_mean_squared,
				    ft_derived_fns.ft_diff_from_mean);
    ft_derived_fns.ft_diff_from_mean_cubed
      = function_train_round(ft_tmp, round_tol, opts);
    function_train_free(ft_tmp);

    ft_derived_fns.third_central_moment = function_train_integrate_weighted(
      ft_derived_fns.ft_diff_from_mean_cubed);

    // MSE: Are these additional products necessary?
    //ft_derived_fns.ft_diff_from_mean_normalized
    //  = function_train_copy(ft_derived_fns.ft_diff_from_mean);
    //function_train_scale(ft_derived_fns.ft_diff_from_mean_normalized,
    //                     1./stdev);
    //
    //ft_tmp = function_train_product(
    //  ft_derived_fns.ft_diff_from_mean_normalized,
    //  ft_derived_fns.ft_diff_from_mean_normalized);
    //ft_derived_fns.ft_diff_from_mean_normalized_squared
    //  = function_train_round(ft_tmp, round_tol, opts);      
    //function_train_free(ft_tmp);
    //
    //ft_tmp = function_train_product(
    //  ft_derived_fns.ft_diff_from_mean_normalized_squared,
    //  ft_derived_fns.ft_diff_from_mean_normalized);      
    //ft_derived_fns.ft_diff_from_mean_normalized_cubed
    //  = function_train_round(ft_tmp, round_tol, opts);      
    //function_train_free(ft_tmp);
    //
    //ft_derived_fns.skewness = function_train_integrate_weighted(
    //  ft_derived_fns.ft_diff_from_mean_normalized_cubed);

    // Compute skew w/o additional FTs by standardizing third_central_moment
    ft_derived_fns.skewness = ft_derived_fns.third_central_moment / var / stdev;

    // -------------
    // Fourth moment:
    // -------------

    // Skip raw moment:
    //ft_tmp = function_train_product(ft_derived_fns.ft_squared,
    //                                ft_derived_fns.ft_squared);
    //ft_derived_fns.ft_tesseracted
    //  = function_train_round(ft_tmp, round_tol, opts);
    //function_train_free(ft_tmp);
    //ft_derived_fns.fourth_moment = function_train_integrate_weighted(
    //  ft_derived_fns.ft_tesseracted);

    ft_tmp = function_train_product(ft_derived_fns.ft_diff_from_mean_squared,
				    ft_derived_fns.ft_diff_from_mean_squared);
    ft_derived_fns.ft_diff_from_mean_tesseracted
      = function_train_round(ft_tmp, round_tol, opts);      
    function_train_free(ft_tmp);

    ft_derived_fns.fourth_central_moment = function_train_integrate_weighted(
      ft_derived_fns.ft_diff_from_mean_tesseracted);
    ft_derived_fns.excess_kurtosis
      = ft_derived_fns.fourth_central_moment / var / var - 3.;// excess kurtosis
  }

  ft_derived_fns.allocated = true;
}


void C3FnTrainPtrsRep::ft_derived_functions_free()
{
  if (ft_derived_fns.ft_nonrand) {
    function_train_free(ft_derived_fns.ft_nonrand);
    ft_derived_fns.ft_nonrand = NULL;
  }
  if (ft_derived_fns.ft_squared) {
    function_train_free(ft_derived_fns.ft_squared);
    ft_derived_fns.ft_squared = NULL;
  }
  if (ft_derived_fns.ft_squared_nonrand) {
    function_train_free(ft_derived_fns.ft_squared_nonrand);
    ft_derived_fns.ft_squared_nonrand = NULL;
  }
  //if (ft_derived_fns.ft_cubed) {
  //  function_train_free(ft_derived_fns.ft_cubed);
  //  ft_derived_fns.ft_cubed = NULL;
  //}
  //if (ft_derived_fns.ft_tesseracted) {
  //  function_train_free(ft_derived_fns.ft_tesseracted);
  //  ft_derived_fns.ft_tesseracted = NULL;
  //}
  //if (ft_derived_fns.ft_constant_at_mean) {
  //  function_train_free(ft_derived_fns.ft_constant_at_mean);
  //  ft_derived_fns.ft_constant_at_mean = NULL;
  //}

  if (ft_derived_fns.ft_diff_from_mean) {
    function_train_free(ft_derived_fns.ft_diff_from_mean);
    ft_derived_fns.ft_diff_from_mean = NULL;
  }
  if (ft_derived_fns.ft_diff_from_mean_squared) {
    function_train_free(ft_derived_fns.ft_diff_from_mean_squared);
    ft_derived_fns.ft_diff_from_mean_squared = NULL;
  }
  if (ft_derived_fns.ft_diff_from_mean_cubed) {
    function_train_free(ft_derived_fns.ft_diff_from_mean_cubed);
    ft_derived_fns.ft_diff_from_mean_cubed = NULL;        
  }
  if (ft_derived_fns.ft_diff_from_mean_tesseracted) {
    function_train_free(ft_derived_fns.ft_diff_from_mean_tesseracted);
    ft_derived_fns.ft_diff_from_mean_tesseracted = NULL;
  }

  //if (ft_derived_fns.ft_diff_from_mean_normalized) {
  //  function_train_free(ft_derived_fns.ft_diff_from_mean_normalized);
  //  ft_derived_fns.ft_diff_from_mean_normalized = NULL;
  //}
  //if (ft_derived_fns.ft_diff_from_mean_normalized_squared) {
  //  function_train_free(ft_derived_fns.ft_diff_from_mean_normalized_squared);
  //  ft_derived_fns.ft_diff_from_mean_normalized_squared = NULL;
  //}
  //if (ft_derived_fns.ft_diff_from_mean_normalized_cubed) {
  //  function_train_free(ft_derived_fns.ft_diff_from_mean_normalized_cubed);
  //  ft_derived_fns.ft_diff_from_mean_normalized_cubed = NULL;
  //}

  ft_derived_fns.allocated = false;
}

} // namespace

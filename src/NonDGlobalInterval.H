/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGlobalInterval
//- Description: Class for interval bound estimation for epistemic UQ
//- Owner:	 Laura Swiler
//- Checked by:
//- Version:

#ifndef NOND_GLOBAL_INTERVAL_H
#define NOND_GLOBAL_INTERVAL_H

#include "NonDInterval.H"
#include "data_types.h"

namespace Dakota {


/// Class for using global nongradient-based optimization approaches
/// to calculate interval bounds for epistemic uncertainty quantification

/** The NonDGlobalInterval class supports global nongradient-based
    optimization apporaches to determining interval bounds for
    epistemic UQ.  The interval bounds may be on the entire function
    in the case of pure interval analysis (e.g. intervals on input =
    intervals on output), or the intervals may be on statistics of an
    "inner loop" aleatory analysis such as intervals on means,
    variances, or percentile levels.  The preliminary implementation
    will use a Gaussian process surrogate to determine interval bounds. */

class NonDGlobalInterval: public NonDInterval
{
public:

  //
  //- Heading: Constructors and destructor
  //

  NonDGlobalInterval(Model& model); ///< constructor
  ~NonDGlobalInterval();            ///< destructor

  //
  //- Heading: Member functions
  //

  /// Performs an optimization to determine interval bounds for an   
  /// entire function or interval bounds on a particular statistical estimator
  void quantify_uncertainty(); // pure virtual, called by run_iterator

protected:

  //
  //- Heading: New virtual functions
  //

  /// perform any required initialization
  virtual void initialize();
  /// set the optimization variable bounds for each cell
  virtual void set_cell_bounds();
  /// determine truthFnStar and approxFnStar
  virtual void get_best_sample(bool find_max, bool eval_approx);
  /// post-process a cell minimization/maximization result
  virtual void post_process_cell_results(bool minimize);
  /// post-process the interval computed for a response function
  virtual void post_process_response_fn_results();
  /// perform final post-processing
  virtual void post_process_final_results();

  //
  //- Heading: Convenience functions
  //

  /// post-process a GP-based optimization iteration: output EIF maximization
  /// results, update convergence controls, and update GP approximation
  void post_process_gp_results();

  //
  //- Heading: Data
  //

  /// LHS iterator for constructing initial GP for all response functions
  Iterator daceIterator;
  /// NCSU DIRECT optimizer for maximizing expected improvement
  Iterator gpOptimizer;

  /// GP model of response, one approximation per response function
  Model fHatModel;
  /// recast model which assimilates mean and variance to solve the
  /// max(EIF) sub-problem
  Model eifModel;

  /// approximate response corresponding to minimum/maximum truth response
  Real approxFnStar;
  /// minimum/maximum truth response function value
  Real truthFnStar;

private:

  //
  //- Heading: Convenience functions
  //

  /// static function used as the objective function in the
  /// Expected Improvement Function (EIF) for minimizing the GP
  static void EIF_objective_min(const Variables& sub_model_vars,
				const Variables& recast_vars,
				const Response& sub_model_response,
				Response& recast_response);
  /// static function used as the objective function in the
  /// Expected Improvement Function (EIF) for maximizing the GP
  static void EIF_objective_max(const Variables& sub_model_vars,
				const Variables& recast_vars,
				const Response& sub_model_response,
				Response& recast_response);

  //
  //- Heading: Data
  //

  /// pointer to the active object instance used within the static evaluator
  /// functions in order to avoid the need for static data
  static NonDGlobalInterval* nondGIInstance;

  const int seedSpec;   ///< the user seed specification (default is 0)     
  int       numSamples; ///< the number of samples used in the surrogate
  String    rngName;	///< name of the random number generator

  /// counter for number of successive iterations that the optimal EIF
  /// is less than the convergenceTol
  size_t eifConvergenceCntr;
  /// counter for number of successive iterations that the L_2 change
  /// in optimal solution is less than the convergenceTol
  size_t distConvergenceCntr;
 /// stores previous optimal points for convergence
  RealVector prevCStar;
  /// surrogate-based minimization/maximization iteration count
  size_t sbIterNum;
  /// flag indicating convergence of a GP minimization or maximization
  bool approxConverged;
  /// flag for maximal response extraction
  bool allResponsesPerIter;
};


/// Computes relative change between successive c_stars using Euclidean norm
inline Real rel_change_c_star(const RealVector& curr_c_star,
			      const RealVector& prev_c_star)
{
  double norm_prev_c_star = 0, norm_diff_c_star = 0; 
  Real normalized_dc_star;
  for (size_t i=0; i < prev_c_star.length(); i++) {
    norm_diff_c_star
      += std::pow(std::fabs((double)curr_c_star[i]-(double)prev_c_star[i]),2.0);
    norm_prev_c_star += std::pow(std::fabs((double)prev_c_star[i]),2.0);
  }
  normalized_dc_star
    = (Real)std::pow(norm_diff_c_star,0.5)/std::pow(norm_prev_c_star,0.5);
#ifdef DEBUG
  Cout << "(Prev,Curr,dist) " << "(";
  for (size_t i=0; i < curr_c_star.length(); i++)
    Cout << prev_c_star[i] << ", ";
  for (size_t i=0; i < curr_c_star.length(); i++)
    Cout <<  curr_c_star[i] << ", ";
  Cout << normalized_dc_star << ")\n";
#endif
  return normalized_dc_star; 
}

} // namespace Dakota

#endif

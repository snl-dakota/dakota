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

/**
 * \file SolisWets.h
 *
 * Defines the scolib::SolisWets class.
 */

#ifndef scolib_SolisWets_h
#define scolib_SolisWets_h

#include <acro_config.h>
#include <utilib/default_rng.h>
#include <utilib/Normal.h>
#include <utilib/NumArray.h>
#include <utilib/Uniform.h>
#include <colin/solver/ColinSolver.h>

namespace scolib {

using colin::real;
using utilib::BasicArray;
using utilib::NumArray;

/** An implemention of the
stochastic direct search algorithms described by Solis and
Wets '81.  Algorithm 1 uses normal deviations and Algorithm 2 uses 
uniform deviations.
*/
class SolisWets 
   : public colin::ColinSolver<BasicArray<double>, colin::UNLP0_problem>
{
public:

  ///
  SolisWets();

  ///
  void optimize();

  ///
  void write(std::ostream& os) const;

private:
  ///
  void reset_SolisWets();

protected:
   ///
   std::string define_solver_type() const
   { return "SolisWets"; }

  ///
  void initialize_best_point()
	{
	if ((initial_point.size() > 0) &&
	    ( problem->num_real_vars != initial_point.size() ))
	   EXCEPTION_MNGR(std::runtime_error, "initialize_best_point - user-provided best point has length " << initial_point.size() << " but the problem size is " << problem->num_real_vars << std::endl);
        initial_point.resize(problem->num_real_vars);
	}

  /// The number of consecutive successful iterations before \Ref{Delta} is expanded.
  int max_success;

  /// The number of consecutive failed iterations before \Ref{Delta} is contracted.
  int max_failure;

  /// Expansion factor for \Ref{Delta}
  double ex_factor;

  /// Contraction factor for \Ref{Delta}
  double ct_factor;

  /// Lower bound on the value of \Ref{Delta}
  double Delta_thresh;

  /// Initial value of \Ref{Delta} (set after reset is called)
  double Delta_init;

  ///
  std::string update_type;

  /// The flag that controls the types of updates for Delta
  int update_id;

  /// Flag which indicates whether the bias is used.
  bool bias_flag;

  ///
  std::string neighborhood_type;

  ///
  int neighborhood_id;

  ///
  NumArray<double> Sigma;

protected:

  ///
  void virt_debug_io(std::ostream& os, const bool finishing, const int output_level);

  ///
  void UpdateDelta(bool flag);

  ///
  virtual void gen_new_point(NumArray<double>& new_pt, 
			NumArray<double>& mean_vec, double offset,
			bool& bound_feasible);

  /// Rescale the search in each dimension
  bool auto_rescale_flag;

  /// Current step length
  double Delta;

  /// The minimum value of Delta seen so far (used for constraint penalties
  double Delta_min;

  ///
  int n_failure;

  ///
  int n_success;

  /// True if there has been one or more expansions
  bool expand_flag;

  /// A status for the current iterations
  int iteration_status;

  ///
  NumArray<double> vec1, vec2, vec3;

  ///
  NumArray<double> bias;

  ///
  utilib::Uniform unif_dev;

  ///
  utilib::Normal normal_dev;

#if 0
  ///
  void perform_evaluation(BasicArray<double>& point, SolisWets_response_t& response,
			real& val, real& cval, BasicArray<real>& cvals,
                        bool& constraint_feasible, bool& bound_feasible);
#endif

  ///
  colin::AppResponse tmp_response;

};

} // namespace scolib

#endif


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
 * \file PatternSearch
 *
 * Defines the scolib::PatternSearch class.
 */
//
// TODO:  add stochastic searcher which uses a GA-like method to bias the
// ordering of the trial steps.
//    Issue:  Is there such a thing as a biased shuffle?
// TODO: Add a biased searcher that uses the previous results to bias the
//	search
//
 
#ifndef scolib_PatternSearch_h
#define scolib_PatternSearch_h
 
#include <acro_config.h>
#include <colin/reformulation/ConstraintPenalty.h>
#include <colin/solver/ColinSolver.h>
#include <utilib/BitArray.h>
#include <utilib/DoubleVector.h>

namespace scolib {

using colin::real;
using utilib::DoubleVector;
using utilib::NumArray;
using utilib::BitArray;
using utilib::BasicArray;

/** Common Pattern Search methods defined within the framework of Generalized 
Pattern Search.

This class defines a variety of pattern search algorithms.  Although these
algorithms could have been defined with seperate classes, it has proved
easier to maintain these algorithms within a single class, using a switch
to select which algorithm is executed.  See \Ref{Pattern Search Methods}
for a complete description of these methods.

Similarly, this code provides options for the choice of several standard
patterns.  The standard maximal basis includes the 2n coordinate offsets,
the standard minimal basis include n+1 offsets which represents corners
of an n-dimensional polyhderon.  Additionally, the user may specify a
number of additional search directions which are generated randomly.

Two modes of expansion and contraction of \Ref{Delta} are currently supported:
\begin{itemize}

\item If \Ref{update_id} is 0, then \Ref{Delta} is contracted after
\Ref{ExploratoryMoves} fails. If \Ref{ExploratoryMoves} is successful
and \Ref{Delta} has never been contracted, then \Ref{Delta} is expanded.

\item If \Ref{update_id} is 1, then \Ref{Delta} is contracted after
\Ref{ExploratoryMoves} fails.  If \Ref{ExploratoryMoves} is successful
and \Ref{Delta} has never been contracted, then \Ref{Delta} is expanded.
Otherwise it is not modified.

\end{itemize}
*/
class PatternSearch
  : public colin::ColinSolver< utilib::BasicArray<double>, 
                                  colin::UNLP0_problem >
{ 
  typedef colin::AppResponse response_t;
  typedef utilib::BasicArray<double> domain_t;

  ///
  void reset_PatternSearch();

public:

  /**@name General */
  //@{
  ///
  PatternSearch();

   ///
   virtual ~PatternSearch();

   ///
   virtual void set_problem(const utilib::AnyRef& problem_);

  ///
  void optimize();

  ///
  BasicArray<double>& min_point() {return best_curr;}

  ///
  const BasicArray<double>& min_point() const {return best_curr;}

  ///
  void write(std::ostream& os) const;
  //@}

protected:
   std::string define_solver_type() const
   { return "PatternSearch"; }

  ///
  void initialize_best_point()
                {
		if ((initial_point.size() > 0) && 
		    (problem->num_real_vars != initial_point.size()))
	 	   EXCEPTION_MNGR(std::runtime_error, "initialize_best_point - user-provided best point has length " << initial_point.size() << " but the problem size is " << problem->num_real_vars << std::endl);
		initial_point.resize(problem->num_real_vars);
		}

  /**@name Configuration Controls */
  //@{
  /// The initial step length
  double Delta_init;

  /// The step length tolerance
  double Delta_thresh;

  /// Bound for accelerated contractions.  For now, we 
  //    set this as (Delta_thresh * 8), a la Tammy Kolda's
  //    similar bound for APPS.  %%%% Added 7/04 pls
  double Delta_bound;

  /// Sufficient decrease parameter
  double rho;

  /// Coefficient for computing a sufficient decrease.
  double sufficient_decrease_coef;

  /// Expansion factor
  double ex_factor;

  /// Contraction factor
  double ct_factor;

  /// Controls expansion of \Ref{Delta}.
  int max_success;

  /// Step scales (per dimension)
  BasicArray<double> Sigma;

  ///
  std::string update_str;

  /// 0=default, 1=single_expand
  int update_id;


  /// The type of basis used during search
  std::string basis_str;

  /// Integer value of the basis_str type
  unsigned int basis_id;

  /// The last point ID assigned.  %%%% Added 6/04 pls
  int last_pt_id;

  /// The type of exploratory move to be called
  std::string em_str;

  /// Array of trial step indices, which may be shuffled
  BasicArray<unsigned int> ndx;

  ///
  std::string step_selection_str;

  ///
  unsigned int step_selection_id;

  /// Used to provide a deterministic ordering of ndx based on values of
  /// previous improving steps.
  double biased_selection_factor;

  /// Vector that is used with biased_selection_factor to store the direction
  /// that appears to be improving.
  DoubleVector improving_trial;

  /// Vector used to store the bias value for steps in the pattern
  BasicArray<double> step_bias;

  ///
  void update_step_order(int id=-1);

  ///
  int em_case;

  /** If true, consider -2*Delta if a Delta step fails.  This is only
    *  used by the "best_first" method.
    */
  bool reverse_flag;

  /// The number of additional trial points that may be considered
  int num_augmented_trials;

  /** If true, then truncate "fast_adaptive" to only allow one
    *  iteration where you shrink after one feval.
    */
  bool trunc_fps;

  /** If true, we are in shrink mode.  Will affect the way speculative
   *    directions are formed. %%%% Added 7/04  pls
   */
  bool shrink_flag;

  ///
  unsigned int fps_batch;

  ///
  int extended_stopping_rule;

  ///
  int ncore_trials;

  ///
  unsigned int ntrials;

  /// number of required trials---used for prioritized version
  //  %%%% added 7/04 pls
  unsigned int nreq_trials;

  virtual void update_pattern(  DoubleVector& /*prev_iter*/,
				DoubleVector& /*curr_iter*/,
				bool /*flag*/) {}

  /** A generic hook to the update of the pattern matrix.
    * Generalized pattern search methods allow the pattern matrix to vary
    * in a slightly restricted fashion.  The variable \Ref{flag} indicates
    * whether the last call to \Ref{ExploratoryMoves} was successful.
    */
  void UpdateMatrix(bool /*flag*/) {}

  /// This method expands or contracts \Ref{Delta} based on the prior success of \Ref{ExploratoryMoves} (as indicated by \Ref{flag}).
  virtual void UpdateDelta(bool flag);
  //@}


  /**@name Iteration Controls */
  //@{
  ///
  double Delta;

  /// The smallest value of Delta seen so far
  double Delta_min;

  ///
  bool expand_flag;

  ///
  int n_success;

  ///
  int update_flag;

  /// If true, use the Hooke-Jeeves bias vector for simple and multistep PS
  bool HJ_bias;

  ///
  NumArray<double> best_curr;

  ///
  bool bc_flag;

  ///
  BasicArray<real> lower_bc;

  ///
  BasicArray<real> upper_bc;

  ///
  BasicArray<real> range;

  ///
  DoubleVector bias;

  ///
  DoubleVector tvec;

  ///
  BasicArray<NumArray<double> > pattern;

  ///
  virtual void reflect_pattern(const unsigned int id);

  ///
  virtual void generate_trial(int id, const DoubleVector& x, 
			DoubleVector& trial, 
			double scale, bool& feasible, const DoubleVector& bias);

  /** The exploratory moves algorithm samples a finite number of moves that
    * are offsets from the current iterate.  The set of these offsets
    * constitutes the {\em pattern matrix} for the pattern search
    * method. If the method is successful, \Ref{tmpx} contains a solution
    * with a lower value.
    */
  virtual bool ExploratoryMoves(DoubleVector& x, response_t& response, real& value, real& cval);

  /// Core search algorithm for "multistep" exploratory moves
  bool EM_multistep(DoubleVector& x, response_t& response, real& value, real& cval);

  /// Core search algorithm for "simple" exploratory moves
  bool EM_simple(DoubleVector& x, response_t& response, real& value, real& cval);

  /// Core search algorithm for "biased_simple" exploratory moves
  //bool EM_biased_simple(DoubleVector& x, response_t& response, real& value, real& cval);

  /// Core search algorithm for "adaptive" exploratory moves
  bool EM_adaptive(DoubleVector& x, response_t& response, real& value, real& cval);
  //@}

  // Core search algorithm for "adaptive" exploratory moves with priorities
  bool EM_test(DoubleVector& x_, response_t& response,
	       real& best_val, real& best_cval);

  /// Control data for the fast_adaptive pattern search
  struct {
	///
	int first;

	///
	bool succ_flag;

	///
	double rho;

	///
	double rho_prev;

	///
	unsigned int ctr;

	///
        int extended_ctr;
	} FA;

  ///
  int ntrial_points;

  ///
  int nfeasible_points;


  /**@name Debugging Controls */
  //@{
  ///
  bool Debug_success;

  ///
  BitArray succ_history;

  ///
  void virt_debug_io(std::ostream& os, const bool finishing, const int io_level);
  //@}

  /// If true, then rescale the search for bound-constrained problems
  bool auto_rescale_flag;

  ///
  response_t tmp_response;

  ///
  colin::ConstraintPenaltyApplication<colin::UNLP0_problem>* simple_app;

  ///
  std::map<colin::EvaluationID,int> queue_order;
};

} // namespace scolib

#endif

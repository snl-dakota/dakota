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
 * \file Grasp.h
 * 
 * Defines the \c GraspBase and \c Grasp classes
 */

#if 0

#ifndef scolib_Grasp_h
#define scolib_Grasp_h

#include <acro_config.h>
#include "ColinStdSolver.h"


 
/**
 *  Abstract base class for GRASP solvers.
 *  Provides the generic functionality
 * of a GRASP (Greedy Random Adaptive Search Proceedure).
 */
template <class DomainT>
class GraspBase : virtual public colin::StdOptSolver<DomainT>
{
public:

  /// Constructor
  GraspBase();

  /// Destructor
  virtual ~GraspBase()
		{}

  ///
  void minimize();

  ///
  void reset();

  ///
  enum constr_type {standard_constr, biased_constr};

  /// The selection type (TODO: say more...)
  constr_type selection;

  /// Set alpha
  void set_alpha(const REAL alpha_)
	{alpha_init = alpha_;}

protected:

  /// Current value of alpha
  double alpha;

  /// The initial value of alpha
  double alpha_init;

  /// Uniform random number generator
  Uniform rnd;

  /// Abstract method to execute local optimization
  virtual int exec_local_search(REAL ans) = 0;

  /// Performs a semi-greedy construction (default method provided).
  /**
   * This provides a default implementation of a semi-greedy construction,
   * which can be redefined because it is virtual.
   */
  virtual int grasp_construction();

  /// Temporary array that holds the indeces of the best candidates
  IntVector rcl;

  /// Initialize the candidate list values before construction
  virtual void init_candidate_list() = 0;

  /// Reinitialize the candidate list values during construction
  virtual int update_candidate_list() = 0;

  /// The number of candidate list values in cl_vals;
  int ncl;

  /// Array of greedy values for the restricted candidate list
  RealVector cl_vals;

  /// Direct the subclass to use the index-th candidate in the list
  virtual void use_candidate(const int index) = 0;

};



template <class DomainT>
GraspBase<DomainT>::GraspBase()
{
}



template <class DomainT>
void GraspBase<DomainT>::reset()
{
BaseOptimizer::reset();
rnd.generator(&rng);
alpha = alpha_init;
}



template <class DomainT>
void GraspBase<DomainT>::minimize()
{
opt_init();

int num_iters;
if (max_iters <= 0)
   num_iters = MAXINT;
else
   num_iters = curr_iter+max_iters;

best_val = real::positive_infinity;
debug_io(ucout);

for(curr_iter++; curr_iter <= num_iters; curr_iter++) {
  //
  // Check termination rules
  //
  if (check_convergence(min_fval))
     break;
  //
  // Setup the grasp
  //
  grasp_construction();
  //
  //
  // 
  real ans=min_fval;
  exec_local_search(ans);

  if (ans < min_fval) {
     min_fval = ans;
     }
  //
  //
  //
  debug_io(ucout);
  }

debug_io(ucout,true);
}



template <class DomainT>
void  AbstractGRASP::set_rng(RNG* rng_)
{
if (rng_ && (rng != rng_)) {
   rng=rng_;
   rnd.generator(rng_);
   if (ls_base != NULL)
      ls_base->set_rng(rng);
   }
}



int AbstractGRASP::grasp_construction(REAL alpha)
{
init_candidate_list();

bool flag;
int select;

do {
   //
   // In the standard GRASP construction, the candidate list values are used
   // to restricted the set of candidates from which a uniform selection is
   // made.
   //
   if (selection == standard_constr) {
      //
      // Find the minimum and maximum values
      //
      REAL maxval=cl_vals[0];
      REAL minval=cl_vals[0];
      for (int i=1; i<ncl; i++)
        if (maxval < cl_vals[i]) maxval=cl_vals[i];
        else if (minval > cl_vals[i]) minval=cl_vals[i];

      //
      // Compute cutoff and find satisfying points
      //
      if (ncl > rcl.len())
         rcl.resize(ncl);
      REAL cutoff = minval+alpha*(maxval-minval);
      int nrcl=0;
      for (int j=0; j<ncl; j++)
        if (cl_vals[j] <= cutoff)
           rcl[nrcl++] = j;

      select = rcl[Discretize(rnd(),1,nrcl)];
      }

   //
   // In the biased GRASP construction, the candidate list values are used to 
   // stochastically bias the selection of the candidates according to a bias
   // function.
   //
   else if (selection == biased_constr) {
      //
      // Implement using the panmictic_selection class from the ga classes
      //
      select = -1;
      }

   use_candidate(select);
   flag = update_candidate_list();
   } while (flag == TRUE);

return OK;
}
#endif

#endif

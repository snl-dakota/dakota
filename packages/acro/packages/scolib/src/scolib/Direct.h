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
 * \file Direct.h
 *
 * Defines the scolib::Direct class
 *
 * \author Steven Cox (U. Florida)
 * \author William Hart (Sandia Labs)
 */

#ifndef scolib_Direct_h
#define scolib_Direct_h

#include <acro_config.h>
#include <scolib/DirectMisc.h>
#include <colin/solver/ColinSolver.h>

namespace scolib {

using colin::real;

/**
 *  DIRECT is a Lipshitzian optimization algorithm that dynamically
 *  assesses the potential impact of different Lipshitz constants.
 *  DIRECT does not estimate the objective function's Lipshitz constant,
 *  but it identifies interesting subproblems by considering all possible
 *  Lipshitz constants that are consitent with the current subproblems.
 *  DIRECT is a global optimization technique that has proven particularly
 *  effective at engineering design problems, for which only a small
 *  number of function evaluations can be performed (less than 1000).
 */
class DIRECT 
   : public colin::ColinSolver<std::vector<double>, colin::UNLP0_problem>
{
public:

  /// Constructor
  DIRECT();

  ///
  void optimize();

  ///
  void get_final_points(std::vector<std::vector<double> >& points);

protected:
   ///
   std::string define_solver_type() const
   { return "DIRECT"; }

  /// String for the user-defined specification for division
  std::string division_str;

  /// Integer interpretation of division_str (for efficiency)
  /// 1: single  2: multi
  int division;

  /// String for the user-defined specification for constraint management
  std::string method_str;

  /// Integer interpretation of method_str (for efficiency)
  /// 1: constrained  2: penalty
  int constraint_method;

  /// If true, then apply box-penetration technique within DIRECT
  bool DIRECT_BP;

  /// If true, then DIRECT splits \i all boxes, regardless of the value of
  /// the estimated Lipshitz constants
  bool agressive;

  ///
  double min_improvement;

  ///
  double ratio;

  ///
  double Gamma;
  //@}

  ///
  double maxsize;

  ///
  double minsize2;

  ///
  double max_boxsize_ratio;

  /// Index (in fevals) of the current best point
  unsigned int ibest;

  /// Number of real parameters
  unsigned int dimen;

  /// Number of constraints
  int ncon;

  double mindist;

  /// If true, then DIRECT-BP is forcing the termination of direct
  bool directbp_converge;

  /// The constraint penalty used if violated constraints are simply added to 
  /// the objective function.
  double constraint_penalty;

  // Minimum box size needed to check for neighbors
  double minsize3;
    	
  /// The number of elements in `order` that are sorted
  int nsort;
    	
  /// Upper bounds
  std::vector<real> x_upper;

  /// Lower bounds
  std::vector<real> x_lower;

  ///
  //std::vector<real> clower;

  ///
  //std::vector<real> cupper;

  /// The range of the i-th variable: size[i] = x_upper[i] - x_lower[i];
  std::vector<real> range;

  std::vector<double> x;
  std::vector<double> g;
  std::vector<double> e;
  //double s0;

  /// Scale vector used for explicit constraint handling
  std::vector<double> constraint_scale;

  /// division_counter[i] is the number of times that dimension i has been
  /// subidivided
  std::vector<unsigned int> division_counter;
    	
  /// Is set to true by PDIRECT.  Modifies Graham and Consort.
  bool MPIVersion;
   		
  /** The list of all boxes.
    * Note that boxes are never removed from this list, since boxes that
    * are subdivided form a box that has the same center.
    */
  std::vector<direct::point> center;

  /** The list of boxes that sorted.
    * Q: do we need both 'center' and 'order'?
    */
  std::list<direct::simple_sort> order;
    	
  ///
  std::list<int> best;

  ///
  std::vector<int> local;

  /// Evaluate the given point
  void Eval(std::vector<direct::point>&);

  ///
  void Graham(std::vector<unsigned int>& optimal, double min_improvement, bool aggressive, bool post_process);

  ///
  void Consort(std::vector<unsigned int>& optimal);

  ///
  virtual void Divider(std::vector<unsigned int>& optimal);

  /// For DIRECT-BP, this method is used to find neighbors of small boxes 
  /// and adds them to order list if they are
  void Neighbor(std::list<direct::simple_sort>&);

  ///
  void LS();

  ///
  direct::bvector Basis(int,std::list<direct::NeighborHood>&,std::list<int> &);

  ///
  void virt_debug_io(std::ostream& os, const bool finishing,
                        			const int output_level);

  /// The smallest box size allowed before termination
  double min_boxsize;

  ///
  void initialize_best_point()
	{
	if ((initial_point.size() > 0) &&
	    (problem->num_real_vars != initial_point.size()))
	   EXCEPTION_MNGR(std::runtime_error, "initialize_best_point - user-provided best point has length " << initial_point.size() << " but the problem size is " << problem->num_real_vars << std::endl);
	initial_point.resize(problem->num_real_vars);
	}


  ///
  bool check_convergence();

#ifndef ACRO_HAVE_TEMPLATES_AS_TEMPLATE_ARGUMENTS
  utilib::BasicArray<colin::real> constr_buffer;
#endif

private:

  ///
  void reset_DIRECT();

};

}

#endif

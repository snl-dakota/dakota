/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Optimizer
//- Description:  Abstract base class to logically represent a variety
//-               of DAKOTA optimizer objects in a generic fashion.
//- Owner:        Mike Eldred
//- Version: $Id: DakotaOptimizer.H 7018 2010-10-12 02:25:22Z mseldre $

#ifndef DAKOTA_OPTIMIZER_H
#define DAKOTA_OPTIMIZER_H

#include "DakotaMinimizer.H"

namespace Dakota {


/// Base class for the optimizer branch of the iterator hierarchy.

/** The Optimizer class provides common data and functionality for
    DOTOptimizer, CONMINOptimizer, NPSOLOptimizer, SNLLOptimizer,
    NLPQLPOptimizer, COLINOptimizer, and JEGAOptimizer. */

class Optimizer: public Minimizer
{
public:

  /// Static helper function: third-party opt packages which are not available
  static void not_available(const std::string& package_name);

protected:

  //
  //- Heading: Constructors and destructor
  //

  Optimizer();             ///< default constructor
  Optimizer(Model& model); ///< standard constructor

  /// alternate constructor for "on the fly" instantiations
  Optimizer(NoDBBaseConstructor, Model& model);
  /// alternate constructor for "on the fly" instantiations
  Optimizer(NoDBBaseConstructor, size_t num_cv, size_t num_div, size_t num_drv,
	    size_t num_lin_ineq, size_t num_lin_eq, size_t num_nln_ineq,
	    size_t num_nln_eq);

  ~Optimizer();            ///< destructor

  //
  //- Heading: Virtual member function redefinitions
  //

  void initialize_run();
  void run();
  void post_run(std::ostream& s);
  void finalize_run();
  void print_results(std::ostream& s);

  //
  //- Heading: New virtual member functions
  //

  /// Used within the optimizer branch for computing the optimal solution.
  /// Redefines the run virtual function for the optimizer branch.
  virtual void find_optimum() = 0;

  //
  //- Heading: Data
  //
  
  /// number of objective functions (iterator view)
  size_t numObjectiveFns;

  /// flag indicating whether local recasting to a single objective is used
  bool localObjectiveRecast;

  /// pointer to Optimizer instance used in static member functions
  static Optimizer* optimizerInstance;
  /// pointer containing previous value of optimizerInstance
  Optimizer* prevOptInstance;

private:

  //
  //- Heading: Convenience/Helper functions
  //

  /// primary response conversion map for RecastModel used in scaling and 
  /// multiobjective:  transform objectives (fns, grads, Hessians)
  /// from native (user) to iterator space
  static void primary_resp_recast(const Variables& native_vars,
				  const Variables& scaled_vars,
				  const Response& native_response,
				  Response& scaled_response);

  /// forward mapping: maps multiple primary response functions to a single
  /// weighted objective for single-objective optimizers
  void objective_reduction(const Response& full_response,
			   const BoolDeque& sense, const RealVector& full_wts,
			   Response& reduced_response) const;

  /// infers MOO/NLS solution from the solution of a single-objective optimizer
  void local_objective_recast_retrieve(const Variables& vars,
				       Response& response) const;

  //
  //- Heading: Data
  //
};


inline Optimizer::Optimizer(): localObjectiveRecast(false)
{ }


inline Optimizer::~Optimizer()
{ 
  if (scaleFlag || localObjectiveRecast)
    iteratedModel.free_communicators(maxConcurrency);
}


inline void Optimizer::run()
{ find_optimum(); }


inline void Optimizer::finalize_run()
{
  // Restore previous object instance in case of recursion.
  optimizerInstance = prevOptInstance;

  Minimizer::finalize_run();
}


inline void Optimizer::not_available(const std::string& package_name)
{
  Cerr << package_name << " is not available.\n";
  abort_handler(-1);
}

} // namespace Dakota

#endif

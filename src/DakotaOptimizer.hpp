/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Optimizer
//- Description:  Abstract base class to logically represent a variety
//-               of DAKOTA optimizer objects in a generic fashion.
//- Owner:        Mike Eldred
//- Version: $Id: DakotaOptimizer.hpp 7018 2010-10-12 02:25:22Z mseldre $

#ifndef DAKOTA_OPTIMIZER_H
#define DAKOTA_OPTIMIZER_H

#include "DakotaMinimizer.hpp"

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

  /// default constructor
  Optimizer();
  /// alternate constructor; accepts a model
  Optimizer(ProblemDescDB& problem_db, Model& model);

  /// alternate constructor for "on the fly" instantiations
  Optimizer(unsigned short method_name, Model& model);
  /// alternate constructor for "on the fly" instantiations
  Optimizer(unsigned short method_name, size_t num_cv, size_t num_div,
	    size_t num_dsv, size_t num_drv, size_t num_lin_ineq,
	    size_t num_lin_eq, size_t num_nln_ineq, size_t num_nln_eq);

  /// destructor
  ~Optimizer();

  //
  //- Heading: Virtual member function redefinitions
  //

  void initialize_run();
  void post_run(std::ostream& s);
  void finalize_run();
  void print_results(std::ostream& s);

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

  /// Wrap iteratedModel in a RecastModel that performs (weighted)
  /// multi-objective or sum-of-squared residuals transformation
  void reduce_model(bool local_nls_recast, bool require_hessians);

  /// Recast callback to reduce multiple objectives or residuals to a
  /// single objective, with gradients and Hessians as needed
  static void primary_resp_reducer(const Variables& full_vars,
				   const Variables& reduced_vars,
				   const Response& full_response,
				   Response& reduced_response);

  /// forward mapping: maps multiple primary response functions to a single
  /// weighted objective for single-objective optimizers
  void objective_reduction(const Response& full_response,
			   const BoolDeque& sense, const RealVector& full_wts,
			   Response& reduced_response) const;

  //
  //- Heading: Data
  //
};


inline Optimizer::Optimizer(): localObjectiveRecast(false)
{ }


inline Optimizer::~Optimizer()
{ }


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

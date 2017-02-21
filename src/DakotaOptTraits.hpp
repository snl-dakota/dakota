/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        OptTraits
//- Description:  Traits class used in registering the various functionalities
//                supported by individual Dakota optimization TPLs.
//- Owner:        Mike Eldred
//- Version: $Id: DakotaOptTraits.hpp 0001 2017-02-21 10:35:14Z mkhalil $

#ifndef DAKOTA_OPT_TRAITS_H
#define DAKOTA_OPT_TRAITS_H

#include "DakotaMinimizer.hpp"

namespace Dakota {


/// Base class for the optimizer branch of the iterator hierarchy.

/** The Optimizer class provides common data and functionality for
    DOTOptimizer, CONMINOptimizer, NPSOLOptimizer, SNLLOptimizer,
    NLPQLPOptimizer, COLINOptimizer, and JEGAOptimizer. */

class OptTraits: public Minimizer
{
public:

  /// Static helper function: third-party opt packages which are not available
  static void not_available(const std::string& package_name);

protected:

  //
  //- Heading: Constructors and destructor
  //

  /// default constructor
  OptTraits();
  /// alternate constructor; accepts a model
  OptTraits(ProblemDescDB& problem_db, Model& model);

  /// alternate constructor for "on the fly" instantiations
  OptTraits(unsigned short method_name, Model& model);
  /// alternate constructor for "on the fly" instantiations
  OptTraits(unsigned short method_name, size_t num_cv, size_t num_div,
	    size_t num_dsv, size_t num_drv, size_t num_lin_ineq,
	    size_t num_lin_eq, size_t num_nln_ineq, size_t num_nln_eq);

  /// destructor
  ~OptTraits();

  //
  //- Heading: Convenience/Helper functions
  //

  /// Sets supportsMultiobjectives to true
  void set_supports_multiobjectives();

  /// Return the value of supportsMultiobjectives
  void get_supports_multiobjectives(bool supports_multiobjectives);

  //
  //- Heading: Data
  //

  /// number of objective functions (iterator view)
  size_t numObjectiveFns;

  /// flag indicating whether local recasting to a single objective is used
  bool localObjectiveRecast;
  
  /// flag indicating whether method supports multiobjective optimization
  bool supportsMultiobjectives;

  /// pointer to OptTraits instance used in static member functions
  static OptTraits* optTraitsInstance;
  /// pointer containing previous value of optTraitsInstance
  OptTraits* prevOptTraitsInstance;

private:

  //
  //- Heading: Convenience/Helper functions
  //

  //
  //- Heading: Data
  //
};


inline OptTraits::OptTraits()
{ }


inline OptTraits::~OptTraits()
{ }


inline void OptTraits::not_available(const std::string& package_name)
{
  Cerr << package_name << " is not available.\n";
  abort_handler(-1);
}

} // namespace Dakota

#endif

/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        LeastSq
//- Description:  Abstract base class to logically represent a variety
//-               of DAKOTA least squares objects in a generic fashion.
//- Owner:        Mike Eldred
//- Version: $Id: DakotaLeastSq.hpp 6972 2010-09-17 22:18:50Z briadam $

#ifndef DAKOTA_LEAST_SQ_H
#define DAKOTA_LEAST_SQ_H

#include "DakotaMinimizer.hpp"


namespace Dakota {

/// Base class for the nonlinear least squares branch of the iterator hierarchy.

/** The LeastSq class provides common data and functionality for
    least squares solvers (including NL2OL, NLSSOLLeastSq, and SNLLLeastSq. */

class LeastSq: public Minimizer
{
public:

protected:

  //
  //- Heading: Constructors and destructor
  //

  /// default constructor
  LeastSq();                                        
  /// standard constructor
  LeastSq(ProblemDescDB& problem_db, Model& model);
  /// alternate "on the fly" constructor
  LeastSq(unsigned short method_name, Model& model);
  /// destructor
  ~LeastSq();

  //
  //- Heading: Virtual member function redefinitions
  //

  void initialize_run();
  void core_run();
  void post_run(std::ostream& s);
  void finalize_run();
  void print_results(std::ostream& s);

  //
  //- Heading: New virtual member functions
  //

  /// Used within the least squares branch for minimizing the sum of
  /// squares residuals.  Redefines the run virtual function
  /// for the least squares branch.
  virtual void minimize_residuals() = 0;

  /// Calculate confidence intervals on estimated parameters
  void get_confidence_intervals();

  //
  //- Heading: Data
  //

  int numLeastSqTerms; ///< number of least squares terms

  /// pointer to LeastSq instance used in static member functions
  static LeastSq* leastSqInstance;
  /// pointer containing previous value of leastSqInstance
  LeastSq* prevLSqInstance;

  /// flag indicating whether weighted least squares is active
  bool weightFlag;
  /// lower bounds for confidence intervals on calibration parameters
  RealVector confBoundsLower;
  /// upper bounds for confidence intervals on calibration parameters
  RealVector confBoundsUpper;

private:

  //
  //- Heading: Convenience/Helper functions
  //

  /// Wrap iteratedModel in a RecastModel that weights the residuals
  void weight_model();

  /// Recast callback function to weight least squares residuals,
  /// gradients, and Hessians
  static void primary_resp_weighter(const Variables& unweighted_vars,
				    const Variables& weighted_vars,
				    const Response& unweighted_response,
				    Response& weighted_response);

  //
  //- Heading: Data
  //
};


inline LeastSq::LeastSq(): weightFlag(false)
{ }


inline LeastSq::~LeastSq()
{ 
  if (minimizerRecasts) 
    iteratedModel.free_communicators(maxEvalConcurrency);
}


inline void LeastSq::core_run()
{ minimize_residuals(); }


inline void LeastSq::finalize_run()
{
  // Restore previous object instance in case of recursion.
  leastSqInstance = prevLSqInstance;
  
  Minimizer::finalize_run();
}

} // namespace Dakota

#endif

/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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
  LeastSq(std::shared_ptr<TraitsBase> traits);
  /// standard constructor
  LeastSq(ProblemDescDB& problem_db, Model& model, std::shared_ptr<TraitsBase> traits);
  /// alternate "on the fly" constructor
  LeastSq(unsigned short method_name, Model& model, std::shared_ptr<TraitsBase> traits);
  /// destructor
  ~LeastSq();

  //
  //- Heading: Virtual member function redefinitions
  //

  void initialize_run();
  void post_run(std::ostream& s);
  void finalize_run();
  void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

  //
  //- Heading: New virtual member functions
  //

  /// Calculate confidence intervals on estimated parameters
  void get_confidence_intervals(const Variables& native_vars,
				const Response& iter_resp);

  //
  //- Heading: Data
  //

  size_t numLeastSqTerms; ///< number of least squares terms

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

  /// storage for iterator best primary functions (which shouldn't be
  /// stored in bestResponseArray when there are transformations)
  RealVector bestIterPriFns;
  /// whether final primary iterator space functions have been
  /// retrieved (possibly by a derived class)
  bool retrievedIterPriFns;

private:

  //
  //- Heading: Convenience/Helper functions
  //

  /// Wrap iteratedModel in a RecastModel that weights the residuals
  void weight_model();

  void archive_best_results();
  /// Write the confidence intervals to the results output
//  void archive_confidence_intervals();

  //
  //- Heading: Data
  //
};


inline LeastSq::LeastSq(std::shared_ptr<TraitsBase> traits) :
  Minimizer(traits),
  weightFlag(false)
{ }


inline LeastSq::~LeastSq()
{ }


inline void LeastSq::finalize_run()
{
  // Restore previous object instance in case of recursion.
  leastSqInstance = prevLSqInstance;
  
  Minimizer::finalize_run();
}

} // namespace Dakota

#endif

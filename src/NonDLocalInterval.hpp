/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_LOCAL_INTERVAL_H
#define NOND_LOCAL_INTERVAL_H

#include "NonDInterval.hpp"
#include "dakota_data_types.hpp"

namespace Dakota {


/// Class for using local gradient-based optimization approaches to
/// calculate interval bounds for epistemic uncertainty quantification

/** The NonDLocalInterval class supports local gradient-based
    optimization apporaches to determining interval bounds for
    epistemic UQ.  The interval bounds may be on the entire function
    in the case of pure interval analysis (e.g. intervals on input =
    intervals on output), or the intervals may be on statistics of an
    "inner loop" aleatory analysis such as intervals on means,
    variances, or percentile levels. */

class NonDLocalInterval: public NonDInterval
{
public:

  //
  //- Heading: Constructors and destructor
  //

  NonDLocalInterval(ProblemDescDB& problem_db, std::shared_ptr<Model> model); ///< constructor
  ~NonDLocalInterval() override;                                       ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_init_communicators(ParLevLIter pl_iter) override;
  void derived_set_communicators(ParLevLIter pl_iter) override;
  void derived_free_communicators(ParLevLIter pl_iter) override;

  /// Performs a gradient-based optimization to determine interval
  /// bounds for an entire function or interval bounds on a particular
  /// statistical estimator
  void core_run() override;

  void check_sub_iterator_conflict() override;
  unsigned short uses_method() const override;
  void method_recourse(unsigned short method_name) override;

protected:

  //
  //- Heading: New virtual functions
  //

  /// perform any required initialization
  virtual void initialize();
  /// set the optimization variable bounds for each cell
  virtual void set_cell_bounds();
  /// truncate initial_pt to respect current cell lower/upper bounds
  virtual void truncate_to_cell_bounds(RealVector& initial_pt);
  /// post-process a cell minimization/maximization result
  virtual void post_process_cell_results(bool maximize);
  /// post-process the interval computed for a response function
  virtual void post_process_response_fn_results();
  /// perform final post-processing
  virtual void post_process_final_results();

  //
  //- Heading: Data
  //

  /// local gradient-based optimizer
  std::unique_ptr<Iterator> minMaxOptimizer;
  /// recast model which extracts the active objective function
  std::shared_ptr<Model> minMaxModel;

private:

  //
  //- Heading: Convenience functions
  //

  /// static function used to extract the active objective function
  /// when optimizing for an interval lower or upper bound
  static void extract_objective(const Variables& sub_model_vars,
				const Variables& recast_vars,
				const Response& sub_model_response,
				Response& recast_response);

  //
  //- Heading: Data
  //

  /// pointer to the active object instance used within the static evaluator
  /// functions in order to avoid the need for static data
  static NonDLocalInterval* nondLIInstance;

  /// flag representing the gradient-based optimization algorithm
  /// selection (NPSOL SQP or OPT++ NIP)
  bool npsolFlag;
};


inline unsigned short NonDLocalInterval::uses_method() const
{ return (npsolFlag) ? SUBMETHOD_NPSOL : SUBMETHOD_OPTPP; }

} // namespace Dakota

#endif

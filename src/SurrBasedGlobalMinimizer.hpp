/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef SURR_BASED_GLOBAL_MINIMIZER_H
#define SURR_BASED_GLOBAL_MINIMIZER_H

#include "SurrBasedMinimizer.hpp"
#include "DakotaModel.hpp"

namespace Dakota {


/// The global surrogate-based minimizer which sequentially minimizes
/// and updates a global surrogate model without trust region controls


/**
 * \brief A version of TraitsBase specialized for surrogate-based global minimizer
 *
 */

class SurrBasedGlobalTraits: public TraitsBase
{
  public:

  /// default constructor
  SurrBasedGlobalTraits() { }

  /// destructor
  ~SurrBasedGlobalTraits() override { }

  /// A temporary query used in the refactor
  bool is_derived() override { return true; }

  /// Return the flag indicating whether method supports continuous variables
  bool supports_continuous_variables() override { return true; }

  /// Return the flag indicating whether method supports discrete variables
  bool supports_discrete_variables() override { return true; }
  
  /// Return the flag indicating whether method supports linear equalities
  bool supports_linear_equality() override { return true; }

  /// Return the flag indicating whether method supports linear inequalities
  bool supports_linear_inequality() override { return true; }

  /// Return the flag indicating whether method supports nonlinear equalities
  bool supports_nonlinear_equality() override { return true; }

  /// Return the flag indicating whether method supports nonlinear inequalities
  bool supports_nonlinear_inequality() override { return true; }
};


/** This method uses a SurrogateModel to perform minimization (optimization
    or nonlinear least squares) through a set of iterations.  At each
    iteration, a surrogate is built, the surrogate is minimized, and the
    optimal points from the surrogate are then evaluated with the "true"
    function, to generate new points upon which the surrogate for the next
    iteration is built. */

class SurrBasedGlobalMinimizer: public SurrBasedMinimizer
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  SurrBasedGlobalMinimizer(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib,  std::shared_ptr<Model> model);
  /// destructor
  ~SurrBasedGlobalMinimizer() override;

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// initialize graphics customized for surrogate-based iteration
  void initialize_graphics(int iterator_server_id = 1) override;

  /// Performs global surrogate-based optimization by repeatedly
  /// optimizing on and improving surrogates of the response functions.
  void core_run() override;

  // Global surrogate-based methods cannot yet accept multiple initial points
  //bool accepts_multiple_points() const;
  /// Global surrogate-based methods can return multiple points
  bool returns_multiple_points() const override;

private:

  //
  //- Heading: Data members
  //

  /// flag for replacing the previous iteration's point additions, rather
  /// than continuing to append, during construction of the next surrogate
  bool replacePoints;
};


/** This just specializes the Iterator implementation to perform
    default tabulation on the truth model instead of surrogate model. */
inline void SurrBasedGlobalMinimizer::
initialize_graphics(int iterator_server_id)
{ initialize_model_graphics(*iteratedModel->truth_model(), iterator_server_id); }


inline bool SurrBasedGlobalMinimizer::returns_multiple_points() const
{ return true; }

} // namespace Dakota

#endif

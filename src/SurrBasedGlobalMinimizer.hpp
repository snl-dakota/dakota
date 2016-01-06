/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SurrBasedGlobalMinimizer
//- Description: The global surrogate-based optimization algorithm successively
//-              invokes an iterator on an approximate model updated with the
//-              results of the iterator at each iteration.
//- Owner:       John Eddy, Laura Swiler
//- Checked by:
//- Version: $Id: SurrBasedGlobalMinimizer.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef SURR_BASED_GLOBAL_MINIMIZER_H
#define SURR_BASED_GLOBAL_MINIMIZER_H

#include "SurrBasedMinimizer.hpp"
#include "DakotaModel.hpp"

namespace Dakota {


/// The global surrogate-based minimizer which sequentially minimizes
/// and updates a global surrogate model without trust region controls

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
  SurrBasedGlobalMinimizer(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~SurrBasedGlobalMinimizer();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// Performs global surrogate-based optimization by repeatedly
  /// optimizing on and improving surrogates of the response functions.
  void core_run();

  // Global surrogate-based methods cannot yet accept multiple initial points
  //bool accepts_multiple_points() const;
  /// Global surrogate-based methods can return multiple points
  bool returns_multiple_points() const;

private:

  //
  //- Heading: Data members
  //

  /// flag for replacing the previous iteration's point additions, rather
  /// than continuing to append, during construction of the next surrogate
  bool replacePoints;
};


inline bool SurrBasedGlobalMinimizer::returns_multiple_points() const
{ return true; }

} // namespace Dakota

#endif

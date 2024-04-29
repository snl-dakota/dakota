/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef QMEA_APPROXIMATION_H
#define QMEA_APPROXIMATION_H

#include "DakotaApproximation.hpp"


namespace Dakota {

/// Derived approximation class for QMEA Quadratic Multipoint Exponential Approximation
/// (a multipoint approximation).

/** The QMEApproximation class provides a multipoint approximation
    based on matching value and gradient data from multiple points 
    (typically the current and previous iterates) in parameter space.  
    It forms an exponential approximation in terms of intervening variables. */

class QMEApproximation: public Approximation
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  QMEApproximation();
  /// standard constructor
  QMEApproximation(ProblemDescDB& problem_db,
		     const SharedApproxData& shared_data,
                     const String& approx_label);
  /// alternate constructor
  QMEApproximation(const SharedApproxData& shared_data);
  /// destructor
  ~QMEApproximation();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  int min_coefficients() const;

  //int num_constraints() const;

  void build();

  Real value(const Variables& vars);

  const RealVector& gradient(const Variables& vars);

  //const RealMatrix& hessian(const Variables& vars);

  void clear_current_active_data();

private:

  //
  //- Heading: Convenience functions
  //

  /// compute TANA coefficients based on scaled inputs
  void find_scaled_coefficients();

  /// based on minX, apply offset scaling to x to define s
  void offset(const RealVector& x, RealVector& s);

  //Real apxfn_value(const RealVector&);
  Real apxfn_value(const RealVector&);

  //
  //- Heading: Data
  //

  RealVector pExp; ///< vector of exponent values
  RealVector minX; ///< vector of minimum parameter values used in scaling
  RealVector scX1; ///< vector of scaled x1 values
  RealVector scX2; ///< vector of scaled x2 values
  Real H; ///< the scalar Hessian value in the TANA-3 approximation
  RealVector beta; ///< vector of QMEA reduced space diagonal Hessian coefficients
  RealMatrix G_reduced_xfm; ///< Grahm-Schmidt orthonormal reduced subspace transformation
  size_t numUsed; ///< number of previous data points used (size of reduced subspace)
  size_t currGradIndex; ///< index of current expansion point with gradients
  size_t prevGradIndex; ///< index of most recent previous point with gradients
};


inline QMEApproximation::QMEApproximation()
{ }


inline QMEApproximation::
QMEApproximation(const SharedApproxData& shared_data):
  // as Hessian data cannot be used, do not accept 4 bit since it could
  // affect data requirement estimations in Approximation base class
  Approximation(NoDBBaseConstructor(), shared_data)
{
  if (sharedDataRep->buildDataOrder != 3) {
    Cerr << "Error: response values and gradients required in "
	 << "QMEApproximation." << std::endl;
    abort_handler(-1);
  }
}


inline QMEApproximation::~QMEApproximation()
{ }


/** Redefine default implementation to support history mechanism. */
inline void QMEApproximation::clear_current_active_data()
{
  // This function is called from DataFitSurrModel::build_approximation(),
  // immediately prior to generation of new build data (with full derivative
  // orders: value+gradient in this case).  The state of approxData may be
  // mixed, containing zero or more points with derivatives (the last of which
  // is the expansion/anchor point) and zero or more points without derivatives
  // (rejected iterates for which gradients were never computed).

  size_t ndv = sharedDataRep->numVars, num_pts = approxData.points(), num_pop;
  currGradIndex = approxData.anchor_index();

  // demote anchor within approxData bookkeeping
  approxData.clear_anchor_index();

  // prune history of more than ndv points, while ensuring retention
  // of current expansion point
  if (currGradIndex == _NPOS) { // no exp point to preserve (should not happen)
    num_pop = (num_pts > ndv) ? num_pts - ndv : 0;
    prevGradIndex = _NPOS; // demote current to previous (for completeness)
  }
  else {
    size_t excess_pts = (num_pts > ndv) ? num_pts - ndv : 0;
    num_pop = std::min(excess_pts, currGradIndex);

    // update local indices (for completeness), demoting current to previous
    prevGradIndex = currGradIndex - num_pop;
    currGradIndex = _NPOS;
  }

  // pop points from approxData
  approxData.pop_front(num_pop); // remove oldest
}

} // namespace Dakota

#endif

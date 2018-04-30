/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        QMEApproximation
//- Description:  Class for Quadratic Multipoint Exponential Approximation.
//-               
//- Owner:        Robert A. Canfield, Virginia Tech
 
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

  void clear_current();

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
  size_t num_used; ///< number of previous data points used (size of reduced subspace)
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
inline void QMEApproximation::clear_current()
{
  // This function is called from DataFitSurrModel::build_approximation(),
  // immediately prior to generation of new build data (with full derivative
  // orders: value+gradient in this case).  The state of approxData may be
  // mixed, containing zero or more points with derivatives (the last of which
  // is the expansion/anchor point) and zero or more points without derivatives
  // (rejected iterates for which gradients were never computed).

  // demote current expansion point (if defined) to regular/previous data
  approxData.clear_anchor_index();

  // limit the number of previous points to numVars, such that subsequent
  // addition of an expansion point results in numVars+1.  In the future,
  // may want to limit aggregate value/gradient equations from mixed data,
  // but the QMEA approach is segregated among pExp and hessian estimation,
  // so a coarse-grained point count is currently sufficient.
  size_t ndv = sharedDataRep->numVars;
  while (approxData.points() > ndv)
    approxData.pop_front(); // remove oldest
}

} // namespace Dakota

#endif

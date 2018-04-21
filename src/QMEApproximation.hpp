/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        QMEApproximation
//- Description:  Class for QMEA quadratic multipoint exponential approximation.
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

  //
  //- Heading: Data
  //

  RealVector pExp; ///< vector of exponent values
  RealVector minX; ///< vector of minimum parameter values used in scaling
  RealVector scX1; ///< vector of scaled x1 values
  RealVector scX2; ///< vector of scaled x2 values
  Real H; ///< the scalar Hessian value in the TANA-3 approximation
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
  size_t ndv = sharedDataRep->numVars;
  // demote from anchor to regular/previous data
  // (for completeness; no longer uses anchor designation)
  approxData.clear_anchor_index();
  //  previous is deleted and anchor moved to previous
  if (approxData.points() > ndv+1)
    approxData.pop_front();

  /*
  approxData.clear_data();
  if (approxData.anchor()) { // anchor becomes previous expansion point
    approxData.push_back(approxData.anchor_variables(),
			 approxData.anchor_response());
    approxData.clear_anchor();
  }
  */
}

} // namespace Dakota

#endif

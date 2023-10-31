/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef TANA3_APPROXIMATION_H
#define TANA3_APPROXIMATION_H

#include "DakotaApproximation.hpp"


namespace Dakota {

/// Derived approximation class for TANA-3 two-point exponential
/// approximation (a multipoint approximation).

/** The TANA3Approximation class provides a multipoint approximation
    based on matching value and gradient data from two points (typically
    the current and previous iterates) in parameter space.  It forms an
    exponential approximation in terms of intervening variables. */

class TANA3Approximation: public Approximation
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  TANA3Approximation();
  /// standard constructor
  TANA3Approximation(ProblemDescDB& problem_db,
		     const SharedApproxData& shared_data,
                     const String& approx_label);
  /// alternate constructor
  TANA3Approximation(const SharedApproxData& shared_data);
  /// destructor
  ~TANA3Approximation();

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

  /// based on minX, offset original parameters (x) to define positive
  /// parameters (s)
  void offset(const RealVector& x, RealVector& s);

  //
  //- Heading: Data
  //

  RealVector pExp; ///< vector of exponent values
  RealVector minX; ///< vector of minimum param values used for offset/scaling
  RealVector scX1; ///< vector of scaled and/or offset x1 values
  RealVector scX2; ///< vector of scaled and/or offset x2 values
  Real H; ///< the scalar Hessian value in the TANA-3 approximation
};


inline TANA3Approximation::TANA3Approximation()
{ }


inline TANA3Approximation::
TANA3Approximation(const SharedApproxData& shared_data):
  // as Hessian data cannot be used, do not accept 4 bit since it could
  // affect data requirement estimations in Approximation base class
  Approximation(NoDBBaseConstructor(), shared_data)
{
  if (sharedDataRep->buildDataOrder != 3) {
    Cerr << "Error: response values and gradients required in "
	 << "TANA3Approximation." << std::endl;
    abort_handler(-1);
  }
}


inline TANA3Approximation::~TANA3Approximation()
{ }


/** Redefine default implementation to support history mechanism. */
inline void TANA3Approximation::clear_current_active_data()
{
  approxData.clear_anchor_index();
  approxData.history_target(1, sharedDataRep->activeKey);

  /*
  // demote from anchor to regular/previous data
  // (for completeness; TANA no longer uses anchor designation)
  approxData.clear_anchor_index();
  //  previous is deleted and anchor moved to previous
  if (approxData.points() > 1)
    approxData.pop_front();
  */
}

} // namespace Dakota

#endif

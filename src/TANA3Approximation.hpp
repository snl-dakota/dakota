/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        TANA3Approximation
//- Description:  Class for TANA-3 two-point exponential approximation.
//-               
//- Owner:        Mike Eldred, Sandia National Laboratories
 
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
  TANA3Approximation(ProblemDescDB& problem_db, size_t num_vars);
  /// alternate constructor
  TANA3Approximation(size_t num_vars, short data_order, short output_level);
  /// destructor
  ~TANA3Approximation();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  int min_coefficients() const;

  int num_constraints()  const;

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


inline TANA3Approximation::TANA3Approximation()
{ }


inline TANA3Approximation::
TANA3Approximation(size_t num_vars, short data_order, short output_level):
  // as Hessian data cannot be used, do not accept 4 bit since it could
  // affect data requirement estimations in Approximation base class
  Approximation(NoDBBaseConstructor(), num_vars, (data_order & 3), output_level)
{ approxType = "multipoint_tana"; }


inline TANA3Approximation::~TANA3Approximation()
{ }


/** Redefine default implementation to support history mechanism. */
inline void TANA3Approximation::clear_current()
{
  approxData.clear_data();
  if (approxData.anchor()) { // anchor becomes previous expansion point
    approxData.push_back(approxData.anchor_variables(),
			 approxData.anchor_response());
    approxData.clear_anchor();
  }
}

} // namespace Dakota

#endif

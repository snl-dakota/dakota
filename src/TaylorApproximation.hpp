/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef TAYLOR_APPROXIMATION_H
#define TAYLOR_APPROXIMATION_H

#include "DakotaApproximation.hpp"


namespace Dakota {

/// Derived approximation class for first- or second-order Taylor
/// series (a local approximation).

/** The TaylorApproximation class provides a local approximation based 
    on data from a single point in parameter space.  It uses a zeroth-,
    first- or second-order Taylor series expansion: f(x) = f(x_c) for
    zeroth-order, plus grad(x_c)' (x - x_c) for first- and second-order,
    and plus (x - x_c)' Hess(x_c) (x - x_c) / 2 for second-order. */

class TaylorApproximation: public Approximation
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  TaylorApproximation();
  /// standard constructor
  TaylorApproximation(ProblemDescDB& problem_db,
		      const SharedApproxData& shared_data,
                      const String& approx_label);
  /// alternate constructor
  TaylorApproximation(const SharedApproxData& shared_data);
  /// destructor
  ~TaylorApproximation() override;

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  int min_coefficients() const override;

  //int num_constraints() const; // use default implementation

  void build() override;

  Real value(const Variables& vars) override;

  const RealVector& gradient(const Variables& vars) override;

  const RealSymMatrix& hessian(const Variables& vars) override;

private:

  //
  //- Heading: Data
  //

};


inline TaylorApproximation::TaylorApproximation()
{ }


inline TaylorApproximation::
TaylorApproximation(const SharedApproxData& shared_data):
  Approximation(NoDBBaseConstructor(), shared_data)
{ }


inline TaylorApproximation::~TaylorApproximation()
{ }

} // namespace Dakota

#endif

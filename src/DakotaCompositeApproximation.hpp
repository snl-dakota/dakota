/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_COMPOSITE_APPROXIMATION_H
#define DAKOTA_COMPOSITE_APPROXIMATION_H

#include "DakotaFieldApproximation.hpp"


namespace Dakota {

/// Derived Approximation class for Composite of Scalar Approximations

/** Encapsulates common behavior for Surrogates modules, with
    specialization for specific surrogates in derived classes.  */
class CompositeApproximation: public FieldApproximation
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  CompositeApproximation() { }
  /// standard constructor: 
  CompositeApproximation(const ProblemDescDB& problem_db,
		         const SharedApproxData& shared_data,
		         const StringArray& approx_labels);
  /// destructor
  ~CompositeApproximation() { }

  //void add_approximation(Approximation approx)
  void add_approximation(std::shared_ptr<Approximation> approx)
    { approximations.push_back(approx); }

  /// return the minimum number of samples (unknowns) required to
  /// build the field approximation
  int min_coefficients() const override
    { return 0; }

  /// builds the approximation from scratch
  void build() override;

protected:

  Real value(const Variables& vars) override;

  //std::vector<Approximation> approximations;
  std::vector<std::shared_ptr<Approximation>> approximations;

};

} // namespace Dakota
#endif

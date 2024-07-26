/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_FIELD_APPROXIMATION_H
#define DAKOTA_FIELD_APPROXIMATION_H

#include "DakotaApproximation.hpp"

namespace Dakota {

/// Base class for the field-based approximation class hierarchy.

class FieldApproximation: public Approximation
{
public:

  //
  //- Heading: Constructors, destructor, assignment operator
  //

  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  FieldApproximation(BaseConstructor, const ProblemDescDB& problem_db,
                     const SharedApproxData& shared_data, 
                     const StringArray& approx_label);

  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  FieldApproximation(NoDBBaseConstructor, const SharedApproxData& shared_data);

  /// default constructor
  FieldApproximation();
  /// standard constructor for envelope
  FieldApproximation(ProblemDescDB& problem_db,
                     const SharedApproxData& shared_data,
                     const StringArray& approx_labels);

  /// destructor
  virtual ~FieldApproximation();

  /// builds the approximation from scratch
  void build(int num_resp) override;

  /// return the number of approximation components
  size_t num_components() const override
  { return numComponents; }

  /// set active field component
  void active_component(int comp)
  { activeComponent = comp; }

  /// get active field component
  int active_component(int comp) const
  { return activeComponent; }


protected:

  //
  //- Heading: Data
  //

  int activeComponent;

  size_t numComponents;

private:

  //
  //- Heading: Member functions
  //

  /// Used only by the standard envelope constructor to initialize
  /// approxRep to the appropriate derived type.
  std::shared_ptr<FieldApproximation>
  get_field_approx(ProblemDescDB& problem_db, const SharedApproxData& shared_data,
	           const StringArray& approx_label);
};

} // namespace Dakota

#endif

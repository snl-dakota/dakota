/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       MixedConstraints
//- Description: Container class of linear constraints and bounds employing 
//-              the "mixed" approach to variables usage.
//- Owner:       Mike Eldred
//- Version: $Id: MixedConstraints.H 7024 2010-10-16 01:24:42Z mseldre $

#ifndef MIXED_CONSTRAINTS_H
#define MIXED_CONSTRAINTS_H

#include "DakotaConstraints.H"
#include "DataVariables.H"


namespace Dakota {

/// Derived class within the Constraints hierarchy which employs
/// the default data view (no variable or domain type array merging).

/** Derived variable constraints classes take different views of the
    design, uncertain, and state variable types and the continuous and
    discrete domain types.  The MixedConstraints derived
    class separates the design, uncertain, and state variable types as
    well as the continuous and discrete domain types.  The result is
    separate lower and upper bounds arrays for continuous design,
    discrete design, uncertain, continuous state, and discrete state
    variables.  This is the default approach, so all iterators and
    strategies not specifically utilizing the All or Merged views use
    this approach (see Variables::get_variables(problem_db) for
    variables type selection; variables type is passed to the
    Constraints constructor in Model). */

class MixedConstraints: public Constraints
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// lightweight constructor
  MixedConstraints(const SharedVariablesData& svd);
  /// standard constructor
  MixedConstraints(const ProblemDescDB& problem_db,
		   const SharedVariablesData& svd);
  /// destructor
  ~MixedConstraints();

  //
  //- Heading: Virtual function redefinitions
  //

  void write(std::ostream& s) const;
  void read(std::istream& s);

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void reshape(const SizetArray& vc_totals);
  void build_active_views();
  void build_inactive_views();

private:

  //
  //- Heading: Convenience functions
  //
};


inline MixedConstraints::MixedConstraints(const SharedVariablesData& svd):
  Constraints(BaseConstructor(), svd)
{ reshape(svd.components_totals()); }


inline MixedConstraints::~MixedConstraints()
{ }

} // namespace Dakota

#endif

/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       MixedVarConstraints
//- Description: Derived class for variable bounds employing "mixed"
//-              continuous-discrete variables.
//- Owner:       Mike Eldred
//- Version: $Id: MixedVarConstraints.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef MIXED_VAR_CONSTRAINTS_H
#define MIXED_VAR_CONSTRAINTS_H

#include "DakotaConstraints.hpp"
#include "DataVariables.hpp"


namespace Dakota {

/// Derived class within the Constraints hierarchy which separates
/// continuous and discrete variables (no domain type array merging).

/** Derived variable constraints classes take different views of the
    design, uncertain, and state variable types and the continuous and
    discrete domain types.  The MixedVarConstraints derived class
    separates the continuous and discrete domain types (see
    Variables::get_variables(problem_db) for variables type selection;
    variables type is passed to the Constraints constructor in Model). */

class MixedVarConstraints: public Constraints
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// lightweight constructor
  MixedVarConstraints(const SharedVariablesData& svd);
  /// standard constructor
  MixedVarConstraints(const ProblemDescDB& problem_db,
		      const SharedVariablesData& svd);
  /// destructor
  ~MixedVarConstraints();

  //
  //- Heading: Virtual function redefinitions
  //

  void write(std::ostream& s) const;
  void read(std::istream& s);

protected:

  //
  //- Heading: Virtual function redefinitions
  //

private:

  //
  //- Heading: Convenience functions
  //
};


inline MixedVarConstraints::MixedVarConstraints(const SharedVariablesData& svd):
  Constraints(BaseConstructor(), svd)
{ }


inline MixedVarConstraints::~MixedVarConstraints()
{ }

} // namespace Dakota

#endif

/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       RelaxedVarConstraints
//- Description: Derived class for variable bounds employing relaxation of
//-              discrete variables.
//- Owner:       Mike Eldred
//- Version: $Id: RelaxedVarConstraints.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef RELAXED_VAR_CONSTRAINTS_H
#define RELAXED_VAR_CONSTRAINTS_H

#include "DakotaConstraints.hpp"
#include "DataVariables.hpp"


namespace Dakota {

/// Derived class within the Constraints hierarchy which employs
/// relaxation of discrete variables.

/** Derived variable constraints classes take different views of the
    design, uncertain, and state variable types and the continuous and
    discrete domain types.  The RelaxedVarConstraints derived class
    combines continuous and discrete domain types through integer
    relaxation.  The branch and bound method uses this approach (see
    Variables::get_variables(problem_db) for variables type selection;
    variables type is passed to the Constraints constructor in Model). */

class RelaxedVarConstraints: public Constraints
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// lightweight constructor
  RelaxedVarConstraints(const SharedVariablesData& svd);
  /// standard constructor
  RelaxedVarConstraints(const ProblemDescDB& problem_db,
			const SharedVariablesData& svd);
  /// destructor
  ~RelaxedVarConstraints();

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


inline RelaxedVarConstraints::
RelaxedVarConstraints(const SharedVariablesData& svd):
  Constraints(BaseConstructor(), svd)
{ }


inline RelaxedVarConstraints::~RelaxedVarConstraints()
{ }

} // namespace Dakota

#endif

/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       RelaxedVariables
//- Description: Container class of variables employing relaxation of
//-              discrete variables.
//- Owner:       Mike Eldred
//- Version: $Id: RelaxedVariables.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef RELAXED_VARIABLES_H
#define RELAXED_VARIABLES_H

#include "DakotaVariables.hpp"
#include "DataVariables.hpp"


namespace Dakota {

/// Derived class within the Variables hierarchy which employs the
/// relaxation of discrete variables.

/** Derived variables classes take different views of the design,
    uncertain, and state variable types and the continuous and
    discrete domain types.  The RelaxedVariables derived class combines
    continuous and discrete domain types but separates design,
    uncertain, and state variable types.  The
    branch and bound method uses this approach (see
    Variables::get_variables(problem_db)). */

class RelaxedVariables: public Variables
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  RelaxedVariables(const ProblemDescDB& problem_db,
		   const std::pair<short,short>& view);
  /// lightweight constructor
  RelaxedVariables(const SharedVariablesData& svd);
  /// destructor
  ~RelaxedVariables();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void read(std::istream& s);
  void write(std::ostream& s, unsigned short vars_part = ALL_VARS) const;
  void write_aprepro(std::ostream& s) const;
  void read_tabular(std::istream& s, unsigned short vars_part = ALL_VARS);
  void write_tabular(std::ostream& s, unsigned short vars_part = ALL_VARS) const;
  void write_tabular_labels(std::ostream& s,
                            unsigned short vars_part = ALL_VARS) const;

  /// Implementation of reading various formats using the specified
  /// read handler, accounting for reordering due to relaxation
  template<typename Reader>
  void read_core(std::istream& s, Reader read_handler, unsigned short vars_part);
  /// Implementation of writing various formats using the specified
  /// write handler, accounting for reordering due to relaxation
  template<typename Writer>
  void write_core(std::ostream& s, Writer write_handler,
                  unsigned short vars_part) const;

private:

  //
  //- Heading: Private data members
  //

};


inline RelaxedVariables::RelaxedVariables(const SharedVariablesData& svd):
  Variables(BaseConstructor(), svd)
{ }


inline RelaxedVariables::~RelaxedVariables()
{ }

} // namespace Dakota

#endif

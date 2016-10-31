/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       MixedVariables
//- Description: Container class of variables, employing a "mixed"
//-              continuous/discrete approach to variables usage.
//- Owner:       Mike Eldred
//- Version: $Id: MixedVariables.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef MIXED_VARS_H
#define MIXED_VARS_H

#include "DakotaVariables.hpp"
#include "DataVariables.hpp"


namespace Dakota {

/// Derived class within the Variables hierarchy which separates
/// continuous and discrete variables (no domain type array merging).

/** Derived variables classes take different views of the design, uncertain,
    and state variable types and the continuous and discrete domain types.
    The MixedVariables derived class separates the continuous and discrete
    domain types (see Variables::get_variables(problem_db)). */

class MixedVariables: public Variables
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  MixedVariables(const ProblemDescDB& problem_db,
		 const std::pair<short,short>& view);
  /// lightweight constructor
  MixedVariables(const SharedVariablesData& svd);
  /// destructor
  ~MixedVariables();

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

  /// Implementation of reading various formats using the specified read handler
  template<typename Reader>
  void read_core(std::istream& s, Reader read_handler, unsigned short vars_part);
  /// Implementation of writing various formats using the specified write handler
  template<typename Writer>
  void write_core(std::ostream& s, Writer write_handler, 
                  unsigned short vars_part) const;

private:

  //
  //- Heading: Private member functions
  //
};


inline MixedVariables::MixedVariables(const SharedVariablesData& svd):
  Variables(BaseConstructor(), svd)
{ }


inline MixedVariables::~MixedVariables()
{ }

} // namespace Dakota

#endif

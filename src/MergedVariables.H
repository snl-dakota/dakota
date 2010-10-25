/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       MergedVariables
//- Description: Container class of variables employing the "merged" approach
//-              to variables usage.
//- Owner:       Mike Eldred
//- Version: $Id: MergedVariables.H 7024 2010-10-16 01:24:42Z mseldre $

#ifndef MERGED_VARIABLES_H
#define MERGED_VARIABLES_H

#include "DakotaVariables.H"
#include "DataVariables.H"


namespace Dakota {

/// Derived class within the Variables hierarchy which employs the
/// merged data view.

/** Derived variables classes take different views of the design,
    uncertain, and state variable types and the continuous and
    discrete domain types.  The MergedVariables derived class combines
    continuous and discrete domain types but separates design,
    uncertain, and state variable types.  The result is a single
    continuous array of design variables (mergedDesignVars), a single
    continuous array of uncertain variables (uncertainVars), and a
    single continuous array of state variables (mergedStateVars).  The
    branch and bound strategy uses this approach (see
    Variables::get_variables(problem_db)). */

class MergedVariables: public Variables
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  MergedVariables(const ProblemDescDB& problem_db,
		  const std::pair<short,short>& view);
  /// lightweight constructor
  MergedVariables(const SharedVariablesData& svd);
  /// destructor
  ~MergedVariables();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void read(std::istream& s);
  void write(std::ostream& s) const;
  void write_aprepro(std::ostream& s) const;
  void read_tabular(std::istream& s);
  void write_tabular(std::ostream& s) const;

  void reshape(const SizetArray& vc_totals);

  void build_active_views();
  void build_inactive_views();

private:

  //
  //- Heading: Private data members
  //

};


inline MergedVariables::MergedVariables(const SharedVariablesData& svd):
  Variables(BaseConstructor(), svd)
{ reshape(svd.components_totals()); }


inline MergedVariables::~MergedVariables()
{ }

} // namespace Dakota

#endif

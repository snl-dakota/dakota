/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       MergedConstraints
//- Description: Container class of linear constraints and bounds employing 
//-              the "merged" approach to variables usage.
//- Owner:       Mike Eldred
//- Version: $Id: MergedConstraints.H 7024 2010-10-16 01:24:42Z mseldre $

#ifndef MERGED_CONSTRAINTS_H
#define MERGED_CONSTRAINTS_H

#include "DakotaConstraints.H"
#include "DataVariables.H"


namespace Dakota {

/// Derived class within the Constraints hierarchy which employs
/// the merged data view.

/** Derived variable constraints classes take different views of the
    design, uncertain, and state variable types and the continuous and
    discrete domain types.  The MergedConstraints derived class
    combines continuous and discrete domain types but separates
    design, uncertain, and state variable types.  The result is merged
    design bounds arrays (mergedDesignLowerBnds,
    mergedDesignUpperBnds), uncertain distribution bounds arrays
    (uncertainLowerBnds, uncertainUpperBnds), and merged state
    bounds arrays (mergedStateLowerBnds, mergedStateUpperBnds).  The
    branch and bound strategy uses this approach (see
    Variables::get_variables(problem_db) for variables type
    selection; variables type is passed to the Constraints
    constructor in Model). */

class MergedConstraints: public Constraints
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// lightweight constructor
  MergedConstraints(const SharedVariablesData& svd);
  /// standard constructor
  MergedConstraints(const ProblemDescDB& problem_db,
		    const SharedVariablesData& svd);
  /// destructor
  ~MergedConstraints();

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


inline MergedConstraints::MergedConstraints(const SharedVariablesData& svd):
  Constraints(BaseConstructor(), svd)
{ reshape(svd.components_totals()); }


inline MergedConstraints::~MergedConstraints()
{ }

} // namespace Dakota

#endif

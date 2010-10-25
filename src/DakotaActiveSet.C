/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ActiveSet
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DakotaActiveSet.H"

static const char rcsId[]="@(#) $Id";


namespace Dakota {

ActiveSet::ActiveSet(size_t num_fns, size_t num_deriv_vars)
{
  // Resize
  reshape(num_fns, num_deriv_vars);
  // Initialize
  request_values(1);
  derivative_start_value(1);
}


/// equality operator for ActiveSet
bool operator==(const ActiveSet& set1, const ActiveSet& set2)
{
  return ( set1.requestVector   == set2.requestVector &&
	   set1.derivVarsVector == set2.derivVarsVector );
}

} // namespace Dakota

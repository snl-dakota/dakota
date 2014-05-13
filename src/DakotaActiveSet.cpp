/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ActiveSet
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DakotaActiveSet.hpp"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>

static const char rcsId[]="@(#) $Id";

BOOST_CLASS_EXPORT(Dakota::ActiveSet)
//BOOST_CLASS_EXPORT_IMPLEMENT(Dakota::ActiveSet)

namespace Dakota {

ActiveSet::ActiveSet(size_t num_fns, size_t num_deriv_vars)
{
  // Resize
  reshape(num_fns, num_deriv_vars);
  // Initialize
  request_values(1);
  derivative_start_value(1);
}


ActiveSet::ActiveSet(size_t num_fns)
{ reshape(num_fns); request_values(1); }


template<class Archive>
void ActiveSet::serialize(Archive& ar, const unsigned int version)
{
  ar & requestVector;
  ar & derivVarsVector;
}

// These shouldn't be necessary, but using to avoid static linking
// issues until can find the right Boost macro ordering
template void ActiveSet::
serialize<boost::archive::binary_iarchive>(boost::archive::binary_iarchive& ar, 
					   const unsigned int version);

template void ActiveSet::
serialize<boost::archive::binary_oarchive>(boost::archive::binary_oarchive& ar, 
					   const unsigned int version);


/// equality operator for ActiveSet
bool operator==(const ActiveSet& set1, const ActiveSet& set2)
{
  return ( set1.requestVector   == set2.requestVector &&
	   set1.derivVarsVector == set2.derivVarsVector );
}

} // namespace Dakota

/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ParamResponsePair
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "ParamResponsePair.hpp"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/utility.hpp>  // for std::pair
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>

static const char rcsId[]="@(#) $Id: ParamResponsePair.cpp 6715 2010-04-02 21:58:15Z wjbohnh $";

BOOST_CLASS_EXPORT(Dakota::ParamResponsePair)

namespace Dakota {


void ParamResponsePair::read_annotated(std::istream& s)
{
  prPairParameters.read_annotated(s);
  s >> evalInterfaceIds.second;
  if (evalInterfaceIds.second == "NULL")
    evalInterfaceIds.second.clear();
  prPairResponse.read_annotated(s);
  s >> evalInterfaceIds.first;
}


void ParamResponsePair::write_annotated(std::ostream& s) const
{
  prPairParameters.write_annotated(s);
  if (evalInterfaceIds.second.empty())
    s << "NULL "; // read_annotated cannot detect an empty string
  else 
    s << evalInterfaceIds.second << ' ';
  prPairResponse.write_annotated(s);
  s << evalInterfaceIds.first << '\n';
}


template<class Archive>
void ParamResponsePair::serialize(Archive& ar, const unsigned int version)
{
  ar & prPairParameters;
  ar & evalInterfaceIds.second;
  ar & prPairResponse;
  ar & evalInterfaceIds.first;
}


// These shouldn't be necessary, but using to avoid static linking
// issues until can find the right Boost macro ordering
template void ParamResponsePair:: 
serialize<boost::archive::binary_iarchive>(boost::archive::binary_iarchive& ar, 
					   const unsigned int version); 
template void ParamResponsePair:: 
serialize<boost::archive::binary_oarchive>(boost::archive::binary_oarchive& ar, 
					   const unsigned int version); 

} // namespace Dakota

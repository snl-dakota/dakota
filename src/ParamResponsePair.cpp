/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ParamResponsePair
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "dakota_tabular_io.hpp"
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
  prpVariables.read_annotated(s);
  s >> evalInterfaceIds.second;
  // (Dakota 6.1 used EMPTY for missing ID)
  if (evalInterfaceIds.second == "NO_ID" || evalInterfaceIds.second == "EMPTY")
    evalInterfaceIds.second.clear();
  prpResponse.read_annotated(s);
  s >> evalInterfaceIds.first;
}


void ParamResponsePair::write_annotated(std::ostream& s) const
{
  prpVariables.write_annotated(s);
  if (evalInterfaceIds.second.empty())
    s << "NO_ID "; // read_annotated cannot detect an empty string
  else 
    s << evalInterfaceIds.second << ' ';
  prpResponse.write_annotated(s);
  s << evalInterfaceIds.first << '\n';
}


void ParamResponsePair::write_tabular(std::ostream& s, 
				      unsigned short tabular_format) const
{
  TabularIO::
    write_leading_columns(s, evalInterfaceIds.first, evalInterfaceIds.second, 
			  tabular_format);
  // write variables in input spec order
  prpVariables.write_tabular(s);
  prpResponse.write_tabular(s);
}


/** When the eval id or interface isn't needed, directly appeal to
    Variables and Response write_tabular_labels... */
void ParamResponsePair::write_tabular_labels(std::ostream& s,
					     unsigned short tabular_format) const
{
  TabularIO::
    write_header_tabular(s, prpVariables, prpResponse, "eval_id", tabular_format);
}


template<class Archive>
void ParamResponsePair::serialize(Archive& ar, const unsigned int version)
{
  ar & prpVariables;
  ar & evalInterfaceIds.second;
  ar & prpResponse;
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

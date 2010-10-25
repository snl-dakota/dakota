/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ParamResponsePair
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "ParamResponsePair.H"

static const char rcsId[]="@(#) $Id: ParamResponsePair.C 6715 2010-04-02 21:58:15Z wjbohnh $";


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

} // namespace Dakota

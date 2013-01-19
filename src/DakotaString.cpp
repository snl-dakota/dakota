/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Dakota::String
//- description: 
//- Owner:       
//- Checked by:
//- Version: $Id

#include "DakotaString.hpp"
#include "MPIPackBuffer.hpp"


namespace Dakota {

/** Private method which converts String to upper. Utilizes an STL
    iterator to step through the string and then calls the STL
    toupper() method.  Needs to be done this way because STL only
    provides a single char toupper method.  */
void String::upper()
{
  std::string::iterator ch_iter;
  for (ch_iter=begin(); ch_iter != end(); ch_iter++)
    *ch_iter = char(toupper(*ch_iter));
}


/** Private method which converts String to lower. Utilizes an STL
    iterator to step through the string and then calls the STL
    tolower() method.  Needs to be done this way because STL only
    provides a single char tolower method.  */
void String::lower()
{
  std::string::iterator ch_iter;
  for (ch_iter=begin(); ch_iter != end(); ch_iter++)
    *ch_iter = char(tolower(*ch_iter));
}


/* Reads String from buffer at MPI receive */
MPIPackBuffer& operator<<(MPIPackBuffer& s, const String& data)
{
  size_t len = data.length();
  s.pack(len);
  for (size_t i=0; i<len; i++)
    s.pack(data[i]);
  return s;
}


/* Writes String to buffer prior to MPI send */
MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, String& data)
{
  size_t len;
  s.unpack(len);
  data.resize(len);
  for (size_t i=0; i<len; i++)
    s.unpack(data[i]);
  return s;
}

} // namespace Dakota

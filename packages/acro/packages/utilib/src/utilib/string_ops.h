/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file string_ops.h
 *
 * Misc operations on CharString objects
 */

#ifndef utilib_string_h
#define utilib_string_h

#include <utilib_config.h>
#include <utilib/CharString.h>
#include <string.h>
#include <string>

namespace utilib
{


/// Convert a CharString to upper case
inline void toUpper(CharString& str)
{
   for (size_t i = 0; i < str.size(); i++)
      str[i] = toupper(str[i]);
}

/// Convert a CharString to lower case
inline void toLower(CharString& str)
{
   for (size_t i = 0; i < str.size(); i++)
      str[i] = tolower(str[i]);
}

/// Split a string into an array of strings
inline BasicArray<CharString> split(const CharString& str, char sep)
{
   size_type len = str.size();

   int npart = 1;
   CharString tmp = str;
   for (size_type i = 0; i < len; i++)
   {
      if (str[i] == sep)
      {
         npart++;
         tmp[i] = '\000';
      }
   }

   BasicArray<CharString> strs(npart);
   size_type ndx = 0;
   for (size_type i = 0; i < len; i++)
   {
      if (ndx < len)
         strs[i] = &(tmp[ndx]);
      while ((ndx < len) && (tmp[ndx] != '\000'))
         ndx++;
      ndx++;
   }

   return strs;
}


/** Join an array of strings into a single string */
inline CharString join(const BasicArray<CharString>& strs, char sep)
{
   CharString str;
   if (strs.size() == 0) return str;
   str += strs[0];

   for (size_type i = 1; i < strs.size(); i++)
   {
      str += sep;
      str += strs[i];
   }

   return str;
}

/// NOTE: this treats tabs as simple whitespace!
inline void wordwrap_printline(std::ostream& os, const std::string& line, const std::string& prefix, unsigned int ncols = 79)
{
   std::string::const_iterator curr = line.begin();
   std::string::const_iterator end  = line.end();
   std::string token;
   std::string ws;
   int col = 0;
   while (curr != end)
   {
      if ((*curr == ' ') || (*curr == '\t'))
      {
         if ((col + ws.size() + token.size()) < ncols)
         {
            os << ws << token;
            col = col + ws.size() + token.size();
         }
         else
         {
            os << std::endl;
            os << prefix;
            os << token;
            col = prefix.size() + token.size();
         }
         token = "";
         ws = " ";
         curr++;
         if (curr == end) break;
         while ((curr != end) && ((*curr == ' ') || (*curr == '\t')))
         {
            ws += " ";
            curr++;
         }
      }
      else if (*curr == '\n')
      {
         curr++;
         if (token == "")
         {
            os << std::endl;
         }
         else if ((col + ws.size() + token.size()) < ncols)
         {
            os << ws << token << std::endl;
            col = col + ws.size() + token.size();
         }
         else
         {
            os << std::endl;
            os << prefix;
            os << token;
            os << std::endl;
         }
         if (curr != end)
         {
            os << prefix;
            col = prefix.size();
         }
         token = "";
         ws = "";
      }
      else
      {
         token += *curr;
         curr++;
      }
   }
   if ((col + ws.size() + token.size()) < ncols)
   {
      os << ws << token << std::endl;
   }
   else
   {
      os << std::endl << prefix << token << std::endl;
   }
}

} // namespace utilib

#endif

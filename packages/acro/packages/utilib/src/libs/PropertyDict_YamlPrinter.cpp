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
 * \file PropertyDict_YamlPrinter.cpp
 */

#include <utilib/PropertyDict_YamlPrinter.h>

using std::endl;
using std::string;

namespace utilib {

void PropertyDict_YamlPrinter::print( std::ostream &output, 
                                      const PropertyDict &dict,
                                      const std::string title )
{
   output << "---";
   if ( ! title.empty() )
      output << " # " << title;
   output << endl;
   out = &output;
   string indent = "";
   print_dict(dict, indent);
   output << "..." << endl;
}


void PropertyDict_YamlPrinter::print_dict( const PropertyDict &dict,
                                           std::string &indent )
{
   if ( dict.empty() )
      return;

   std::string sub_indent = indent + string(indentSize, ' ');

   PropertyDict::const_iterator it = dict.begin();
   for( ; it != dict.end(); ++it )
   {
      *out << indent << it->first() << ": ";
      print_item(it->second(), sub_indent);
   }
}


void PropertyDict_YamlPrinter::print_list( const PropertyList &list,
                                           std::string &indent )
{
   if ( list.empty() )
      return;

   std::string sub_indent = indent + string(listHeader.size(), ' ');
   
   for(PropertyList::const_iterator it = list.begin(); it != list.end(); ++it)
   {
      *out << indent << listHeader;
      print_item(*it, sub_indent);
   }
}

void PropertyDict_YamlPrinter::print_item( const Property &item,
                                           std::string &indent )
{
   Any value = item.get();
   if ( value.is_type(typeid(PropertyDict)) )
   {
      *out << endl;
      print_dict(value.expose<PropertyDict>(), indent);
   }
   else if ( value.is_type(typeid(PropertyList)) )
   {
      *out << endl;
      print_list(value.expose<PropertyList>(), indent);
   }
   else
   {
      std::stringstream ss;
      ss << item;

      std::list<string> lines;
      string::size_type maxLen = 0;
      string s = "";
      while ( getline(ss, s) )
      {
         lines.push_back(s);
         maxLen = std::max(maxLen, s.size());
      }

      if ( lines.empty() )
      {
         *out << endl;
         return;
      }

      bool wrapLines = ( maxLineLength < indent.size() + maxLen );

      if ( lines.size() <= 1 && !wrapLines )
      {
         *out << lines.front() << endl;
         return;
      }

      if ( wrapLines )
         *out << "<" << endl;
      else
         *out << "|" << endl;
      
      while ( ! lines.empty() )
      {
         *out << indent;
         if ( wrapLines )
            wrap_line(lines.front(), indent);
         else
            *out << lines.front();
         *out << endl;
         lines.pop_front();
      }
   }
}


void PropertyDict_YamlPrinter::wrap_line( const std::string &line,
                                          std::string &indent )
{
   string::size_type pos = 0;
   string::size_type count = indent.size();
   while ( true )
   {
      string::size_type newPos = line.find(' ', pos);
      string::size_type len 
         = (newPos == string::npos ? line.size() : (newPos+1)) - pos;
      // Check if we need to break the line before printing this data.
      // The second part of the test prevents spurious blank lines from
      // being inserted at the beginning of the output
      if ( count + len > maxLineLength && count > indent.size() )
      {
         *out << endl << indent;
         count = indent.size();
      }
      *out << line.substr(pos,len);
      count += len;
      if ( newPos == string::npos )
         break;
      pos = newPos+1;
   };

   if ( count > 0 )
      *out << endl;
}

} // namespace utilib

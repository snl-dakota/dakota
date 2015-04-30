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
 * \file PropertyDict_YamlPrinter.h
 *
 * Defines the utilib::PropertyDict_YamlPrinter class.
 */

#ifndef PropertyDict_YamlPrinter_h
#define PropertyDict_YamlPrinter_h

#include <utilib/PropertyDict.h>

namespace utilib {

class PropertyDict_YamlPrinter
{
public:
   PropertyDict_YamlPrinter()
      : out(NULL),
        indentSize(3),
        maxLineLength(78),
        listHeader("- ")
   {}

   void print( std::ostream &output, 
               const PropertyDict &dict, 
               const std::string title = "" );

protected:

   void print_dict(const PropertyDict &dict, std::string &indent);

   void print_list(const PropertyList &list, std::string &indent);

   void print_item(const Property &item, std::string &indent);

   void wrap_line(const std::string &line, std::string &indent);


   std::ostream *out;

   size_t indentSize;

   size_t maxLineLength;

   std::string listHeader;
};

} // namespace utilib

#endif // PropertyDict_YamlPrinter_h

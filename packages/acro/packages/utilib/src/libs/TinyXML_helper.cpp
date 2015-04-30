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

#include <utilib/TinyXML_helper.h>
#ifdef UTILIB_HAVE_TINYXML

namespace utilib
{

std::string get_element_info(TiXmlElement* elt)
{
   std::ostringstream line;
   line << "element \"" << elt->ValueStr() << "\"";
   if ( elt->Row() > 0 )
   {
      line << " at input line " << elt->Row();
      if ( elt->Column() > 0 )
            line << ", column " << elt->Column();
   }
   return line.str();
}


std::string get_element_text(TiXmlElement* elt)
{
   if ( ! elt )
      EXCEPTION_MNGR( std::runtime_error,
                      "get_element_text(): parse error: NULL element" );

   if ( elt->FirstChild() != elt->LastChild() )
      EXCEPTION_MNGR( std::runtime_error, "get_element_text(): parse error: "
                      "element contains more than one child node at "
                      << get_element_info(elt) );
   const char * txt = elt->GetText();
   if ( txt ) 
      return string(txt);
   else if ( elt->FirstChild() )
      EXCEPTION_MNGR( std::runtime_error, "get_element_text(): parse error: "
                      "element does not contain a text node at "
                      << get_element_info(elt) );
   return "";
}

bool get_bool_attribute( TiXmlElement* elt, const char* name, 
                         bool& value, const bool& default_value )
{
   const char* attr = elt->Attribute(name);
   if ( attr == NULL )
   {
      value = default_value;
      return false;
   }
   else
   {
      string test = attr;
      std::transform(test.begin(), test.end(), test.begin(), tolower);
      if ( test.size() == 1 )
      {
         char c = test[0];
         if ( c == '1' || c == 't' || c == 'y' )
            value = true;
         else if ( c == '0' || c == 'f' || c == 'n' )
            value = false;
         else
            EXCEPTION_MNGR(std::runtime_error, "get_bool_attribute(): parse "
                           "error: invalid boolean type for attribute \"" 
                           << name << "\" in " << get_element_info(elt));
      }
      else if ( test.compare("true") == 0 )
         value = true;
      else if ( test.compare("false") == 0 )
         value = false;
      else if ( test.compare("yes") == 0 )
         value = true;
      else if ( test.compare("no") == 0 )
         value = false;
      else 
         EXCEPTION_MNGR(std::runtime_error, "get_bool_attribute(): parse "
                        "error: invalid boolean type for attribute \"" 
                        << name << "\" in " << get_element_info(elt));

      return true;
   }
}


///
void get_bool_attribute(TiXmlElement* elt, const char* name, bool& value)
{
   bool ans = 0;
   if ( ! get_bool_attribute(elt, name, ans, ans) )
      EXCEPTION_MNGR(std::runtime_error, "get_bool_attribute(): "
                     "parse error: missing required attribute \"" << name <<
                     "\" in " << get_element_info(elt));

   value = ans;
}


template<>
bool get_num_attribute
   ( TiXmlElement* elt, const char* name, 
     utilib::Ereal<double>& value, const utilib::Ereal<double>& default_value )
{
   const char* attr = elt->Attribute(name);
   if (!attr)
   {
      value = default_value;
      return false;
   }
   else
   {
      std::istringstream istr(attr);
      value.read(istr);
      return true;
   }
}

template<>
void get_num_attribute( TiXmlElement* elt, const char* name, 
                        utilib::Ereal<double>& value )
{
   const char* attr = elt->Attribute(name);
   if ( ! attr )
      EXCEPTION_MNGR(std::runtime_error, "get_num_attribute(): "
                     "parse error: missing required attribute \"" << name <<
                     "\" in " << get_element_info(elt));

   std::istringstream istr(attr);
   value.read(istr);
}


} // namespace utilib
#endif

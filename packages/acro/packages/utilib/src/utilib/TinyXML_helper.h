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
 * \file TinyXml_helper.h
 *
 * Defines fundamental helper functions for processing TinyXML elements
 * and attributes.
 */

#ifndef utilib_TinyXml_helper_h
#define utilib_TinyXml_helper_h

#include <utilib_config.h>
#ifdef UTILIB_HAVE_TINYXML
#include <utilib/Ereal.h>
#include <utilib/exception_mngr.h>
#include <tinyxml/tinyxml.h>

namespace utilib
{

/// This is a #define so that the reported file/line number match the
/// caller and not this helper's cpp file
#define UNEXPECTED_ELEMENT(ELT,WHAT)                                    \
   EXCEPTION_MNGR( std::runtime_error, WHAT                             \
                   ": Unexpected element '" << ELT->ValueStr() <<       \
                   "' encountered while processing " <<                 \
                   ( ELT->Parent()                                      \
                     ? utilib::get_element_info(ELT->Parent()->ToElement()) \
                     : "(root)" ) )

///
std::string get_element_info(TiXmlElement* elt);

///
std::string get_element_text(TiXmlElement* elt);

///
template<class ReturnT, class DefaultT>
bool get_string_attribute( TiXmlElement* elt, const char* name, 
                           ReturnT& value, const DefaultT& default_value )
{
   const char* attr = elt->Attribute(name);
   if ( attr == NULL )
   {
      value = default_value;
      return false;
   }
   else
   {
      value = attr;
      return true;
   }
}


///
template<class ReturnT>
void get_string_attribute(TiXmlElement* elt, const char* name, ReturnT& value)
{
   const char* attr = elt->Attribute(name);
   if ( attr == NULL )
      EXCEPTION_MNGR(std::runtime_error, "get_string_attribute(): "
                     "parse error: missing required attribute \"" << name <<
                     "\" in " << get_element_info(elt));

   value = attr;
}


///
bool get_bool_attribute( TiXmlElement* elt, const char* name, 
                         bool& value, const bool& default_value );

///
void get_bool_attribute(TiXmlElement* elt, const char* name, bool& value);


///
template <class Type, class DefaultType>
bool get_num_attribute( TiXmlElement* elt, const char* name, 
                        Type& value, const DefaultType& default_value )
{
   double d = 0;
   int ans = elt->QueryDoubleAttribute(name, &d);
   if ( ans == TIXML_NO_ATTRIBUTE )
   {
      value = default_value;
      return false;
   }
   if ( ans == TIXML_WRONG_TYPE )
      EXCEPTION_MNGR(std::runtime_error, "get_num_attribute(): "
                     "parse error: invalid numeric type for attribute \"" 
                     << name << "\" in " << get_element_info(elt));
      
   value = static_cast<Type>(d);
   if ( static_cast<double>(value) != d )
      EXCEPTION_MNGR(std::runtime_error, "get_num_attribute(): "
                     "parse error: numeric type out of range for attribute \"" 
                     << name << "\" in " << get_element_info(elt));
   return true;
}


///
template <class Type>
void get_num_attribute( TiXmlElement* elt, const char* name, Type& value )
{
   Type ans = 0;
   if ( ! get_num_attribute(elt, name, ans, ans) )
      EXCEPTION_MNGR(std::runtime_error, "get_num_attribute(): "
                     "parse error: missing required attribute \"" << name <<
                     "\" in " << get_element_info(elt));

   value = ans;
}


///
template<>
bool get_num_attribute( TiXmlElement* elt, const char* name, 
                        utilib::Ereal<double>& value, 
                        const utilib::Ereal<double>& default_value );

///
template<>
void get_num_attribute( TiXmlElement* elt, const char* name, 
                        utilib::Ereal<double>& value );

} // namespace utilib
#endif

#endif // utilib_TinyXml_helper_h

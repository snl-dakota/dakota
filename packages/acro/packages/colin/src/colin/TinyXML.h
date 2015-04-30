/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

#ifndef colin_TinyXML_h
#define colin_TinyXML_h

#include <colin/BoundTypeArray.h>
#include <colin/real.h>

#include <utilib/comments.h>
#include <utilib/exception_mngr.h>
#include <utilib/Any.h>
#include <utilib/stl_auxiliary.h>
#include <utilib/MixedIntVars.h>
#include <utilib/BasicArray.h>
#include <utilib/TinyXML_helper.h>

#include <boost/bimap.hpp>

#include <sstream>

namespace colin
{

///
template <class Type>
inline void
process_fixed_vars( TiXmlElement* elt, 
                    std::map<size_t, Type>& fixed_vars,
                    size_t num_vars,
                    const utilib::bimap<size_t, std::string>& labels )
{
   if (elt->GetText())
   {
      utilib::BasicArray<Type> tmp;
      read_xml_object(tmp, elt);
      if (tmp.size() != num_vars)
      {
         EXCEPTION_MNGR(std::runtime_error,
                        "SubspaceApplication_MixedIntDomain::xml_initialize"
                        " - Attempting to fix " << tmp.size() <<
                        " values but there are " << num_vars <<
                        " variables");
      }
      for (size_t i = 0; i < tmp.size(); i++)
         fixed_vars[i] = tmp[i];
   }
   else
   {
      TiXmlNode* child = elt->FirstChild();
      for (; child; child = child->NextSibling())
      {
         TiXmlElement* child_elt = child->ToElement();
         if (child_elt)
         {

            if (strcmp(child_elt->Value(), "Variable") != 0)
            {
               EXCEPTION_MNGR(std::runtime_error, "SubspaceApplication_MixedIntDomain::   xml_initialize - unexpected element within an Integer element: " << child_elt->Value());
            }
            int index;
            std::string type;
            int value;
            bound_type_enum btype;
            std::string label;
            utilib::get_string_attribute(child_elt, "label", label, "");
            if (label == "")
               utilib::get_num_attribute(child_elt, "index", index, -1);
            else
            {
               typename utilib::bimap<size_t, std::string>::to::const_iterator 
                  curr = labels.to.find(label);
               if (curr == labels.to.end())
               {
                  EXCEPTION_MNGR(std::runtime_error, "SubspaceApplication_MixedIntDomain::xml_initialize - Invalid label \"" << label << "\"");
               }
               index = curr->second + 1;
            }
            utilib::get_string_attribute(child_elt, "type", type, "hard");
            convert(type, btype);
            utilib::get_num_attribute(child_elt, "value", value, INT_MAX);
            if (index == -1)
            {
               for (size_t i = 0; i < num_vars; i++)
                  fixed_vars[i] = value;
            }
            else
            {
               if ((index < 1) || (static_cast<size_t>(index) > num_vars))
               {
                  EXCEPTION_MNGR(std::runtime_error, "SubspaceApplication_MixedIntDomain::xml_initialize - Invalid index value \"" << index << "\"");
               }
               fixed_vars[index-1] = value;
            }
         }
      }
   }
}

///
template <class Type>
inline bool process_bounds( TiXmlElement* elt, 
                            std::vector<Type>& lower,
                            std::vector<Type>& upper,
                            BoundTypeArray& lower_type,
                            BoundTypeArray& upper_type,
                            const boost::bimap<size_t, std::string>& labels,
                            const Type ninfty, 
                            const Type pinfty )
{
   typedef boost::bimap<size_t, std::string> labels_t;

   bool bounds_flag = false;
   TiXmlNode* child = elt->FirstChild();
   for (; child; child = child->NextSibling())
   {
      if (! child->ToElement())
      {
         if (child->ToText())
         {
            EXCEPTION_MNGR(std::runtime_error, 
                           "process_bounds - XML text item found: \"" << 
                           child->Value() << "\"");
         }
         else if (child->ToComment())
         {
	    // We ignore comments
	    continue;
         }
         else
         {
            EXCEPTION_MNGR(std::runtime_error, 
                           "process_bounds - Unknown XML item found: \"" << 
                           child->Value() << "\"");
         }
      }
      TiXmlElement* tmp_elt = child->ToElement();
      const char* value = tmp_elt->Value();
      if ((strcmp(value, "Lower") == 0) ||
            (strcmp(value, "Geq") == 0))
      {
         bounds_flag = true;
         int index;
         std::string type;
         Type value;
         bound_type_enum btype;
         std::string label;
         utilib::get_string_attribute(tmp_elt, "label", label, "");
         if (label == "")
            utilib::get_num_attribute(tmp_elt, "index", index, -1);
         else
         {
            labels_t::right_const_iterator curr = labels.right.find(label);
            if ( curr == labels.right.end() )
               EXCEPTION_MNGR(std::runtime_error, 
                              "process_bounds - Invalid label \"" << 
                              label << "\"");
               index = curr->second + 1;
         }
         utilib::get_string_attribute(tmp_elt, "type", type, "hard");
         convert(type, btype);
         utilib::get_num_attribute(tmp_elt, "value", value, ninfty);
         if (index == -1)
         {
            lower << value;
            lower_type << btype;
         }
         else
         {
            if ((index < 1) || (static_cast<size_t>(index) > lower.size()))
               EXCEPTION_MNGR(std::runtime_error, 
                              "process_bounds - Invalid index value \"" << 
                              index << "\"");
               lower[index-1] = value;
            lower_type[index-1] = btype;
         }
      }
      else if ((strcmp(value, "Upper") == 0) ||
               (strcmp(value, "Leq") == 0))
      {
         bounds_flag = true;
         int index;
         std::string type;
         Type value;
         bound_type_enum btype;
         std::string label;
         utilib::get_string_attribute(tmp_elt, "label", label, "");
         if (label == "")
            utilib::get_num_attribute(tmp_elt, "index", index, -1);
         else
         {
            labels_t::right_const_iterator curr = labels.right.find(label);
            if (curr == labels.right.end())
               EXCEPTION_MNGR(std::runtime_error, 
                              "process_bounds - Invalid label \"" << 
                              label << "\"");
               index = curr->second + 1;
         }
         utilib::get_string_attribute(tmp_elt, "type", type, "hard");
         convert(type, btype);
         utilib::get_num_attribute(tmp_elt, "value", value, pinfty);
         if (index == -1)
         {
            upper << value;
            upper_type << btype;
         }
         else
         {
            if ((index < 1) || (static_cast<size_t>(index) > upper.size()))
               EXCEPTION_MNGR(std::runtime_error, 
                              "process_bounds - Invalid index value \"" << 
                              index << "\"");
            upper[index-1] = value;
            upper_type[index-1] = btype;
         }
      }
      else if ((strcmp(value, "Equal") == 0) ||
               (strcmp(value, "Fixed") == 0))
      {
         bounds_flag = true;
         int index;
         std::string type;
         Type value;
         bound_type_enum btype;
         std::string label;
         utilib::get_string_attribute(tmp_elt, "label", label, "");
         if (label == "")
            utilib::get_num_attribute(tmp_elt, "index", index, -1);
         else
         {
            labels_t::right_const_iterator curr = labels.right.find(label);
            if (curr == labels.right.end())
               EXCEPTION_MNGR(std::runtime_error, 
                              "process_bounds - Invalid label \"" << 
                              label << "\"");
            index = curr->second + 1;
         }
         utilib::get_string_attribute(tmp_elt, "type", type, "hard");
         convert(type, btype);
         utilib::get_num_attribute(tmp_elt, "value", value, pinfty);
         if (index == -1)
         {
            upper << value;
            upper_type << btype;
            lower << value;
            lower_type << btype;
         }
         else
         {
            if ((index < 1) || (static_cast<size_t>(index) > upper.size()))
            {
               EXCEPTION_MNGR(std::runtime_error, 
                              "process_bounds - Invalid index value \"" << 
                              index << "\"");
            }
            upper[index-1] = value;
            upper_type[index-1] = btype;
            lower[index-1] = value;
            lower_type[index-1] = btype;
         }
      }
   }
   //
   // Perform error checking on the validity of these bounds
   //
   for (size_t i = 0; i < upper.size(); i++)
   {
      if (upper[i] < lower[i])
      {
         EXCEPTION_MNGR(std::runtime_error, "process_bounds - upper bound " 
                        << upper[i] << " is less than lower bound " << 
                        lower[i] << " at index " << (i + 1));
      }
   }
   //
   // Now process the bounds, to fill the fixed map
   //
   //for (size_t i = 0; i < upper.size(); i++)
   //{
   //   if (upper[i] == lower[i])
   //      fixed [i] = upper[i];
   //}
   return bounds_flag;
}

///
template <class ValueT, class ArrayT>
inline void parse_array(std::istringstream& istr, utilib::Any& any)
{
   std::list< ValueT > tlist;
   ValueT val;
   while (istr)
   {
      // Ignore the value if we've exceeded the end of the array
      try
      {
         istr >> utilib::whitespace;
         if (istr)
            istr >> val;
      }
      catch (std::invalid_argument& err)
      {
         std::cerr << "Error parsing XML array argument" << std::endl;
         throw;
      }
      catch (std::exception& err)
      {throw;}
      if (istr)
         tlist.push_back(val);
   }
   ArrayT& tmp = any.template set< ArrayT >();
   tmp.resize(tlist.size());
   typename std::list< ValueT >::iterator curr = tlist.begin();
   typename std::list< ValueT >::iterator end  = tlist.end();
   size_t i = 0;
   while (curr != end)
   {
      tmp[i++] = *curr;
      curr++;
   }
}

///
template <class Type>
inline void read_xml_object(Type& object, TiXmlElement* elt)
{
   static_cast<void>(elt);
   EXCEPTION_MNGR(std::runtime_error, "colin::read_xml_object - This function has not been overloaded for objects of type: " << typeid(object).name());
}

template <>
inline void read_xml_object<std::vector<double> > (std::vector<double>& object, TiXmlElement* elt)
{
   try
   {
      std::istringstream istr(elt->GetText());
      utilib::Any any(object, true, true);
      parse_array<double, std::vector<double> > (istr, any);
   }
   catch (std::exception& err)
   {
      std::cerr << "Error parsing vector<double> object data" << std::endl;
      throw;
   }
}

template <>
inline void read_xml_object<utilib::BasicArray<double> > (utilib::BasicArray<double>& object, TiXmlElement* elt)
{
   try
   {
      std::istringstream istr(elt->GetText());
      utilib::Any any(object, true, true);
      parse_array<double, utilib::BasicArray<double> > (istr, any);
   }
   catch (std::exception& err)
   {
      std::cerr << "Error parsing BasicArray<double> object data" << std::endl;
      throw;
   }
}

template <>
inline void read_xml_object<std::vector<colin::real> > (std::vector<colin::real>& object, TiXmlElement* elt)
{
   try
   {
      std::istringstream istr(elt->GetText());
      utilib::Any any(object, true, true);
      parse_array<colin::real, std::vector<colin::real> > (istr, any);
   }
   catch (std::exception& err)
   {
      std::cerr << "Error parsing vector<colin::real> object data" << std::endl;
      throw;
   }
}

template <>
inline void read_xml_object<utilib::BasicArray<colin::real> > (utilib::BasicArray<colin::real>& object, TiXmlElement* elt)
{
   try
   {
      std::istringstream istr(elt->GetText());
      utilib::Any any(object, true, true);
      parse_array<colin::real, utilib::BasicArray<colin::real> > (istr, any);
   }
   catch (std::exception& err)
   {
      std::cerr << "Error parsing BasicArray<colin::real> object data" << std::endl;
      throw;
   }
}

template <>
inline void read_xml_object<std::vector<int> > (std::vector<int>& object, TiXmlElement* elt)
{
   try
   {
      std::istringstream istr(elt->GetText());
      utilib::Any any(object, true, true);
      parse_array<int, std::vector<int> > (istr, any);
   }
   catch (std::exception& err)
   {
      std::cerr << "Error parsing vector<int> object data" << std::endl;
      throw;
   }
}

template <>
inline void read_xml_object<utilib::BasicArray<int> > (utilib::BasicArray<int>& object, TiXmlElement* elt)
{
   try
   {
      std::istringstream istr(elt->GetText());
      utilib::Any any(object, true, true);
      parse_array<int, utilib::BasicArray<int> > (istr, any);
   }
   catch (std::exception& err)
   {
      std::cerr << "Error parsing BasicArray<int> object data" << std::endl;
      throw;
   }
}

template <>
inline void read_xml_object<utilib::MixedIntVars>(utilib::MixedIntVars& object, TiXmlElement* elt)
{
   try
   {
      std::istringstream istr(elt->GetText());
      istr >> object;
   }
   catch (std::exception& err)
   {
      std::cerr << "Error parsing utilib::MixedIntVars object data" << std::endl;
      throw;
   }
}

template <>
inline void read_xml_object<utilib::BasicArray<utilib::BasicArray<double> > > (utilib::BasicArray<utilib::BasicArray<double> > & object, TiXmlElement* elt)
{
   try
   {
      std::istringstream istr(elt->GetText());
      int nrows = 0, ncols = 0;
      istr >> nrows >> ncols;
      if (!istr)
      {
         EXCEPTION_MNGR(std::runtime_error, "Problem parsing row/col info");
      }
      object.resize(nrows);
      for (int i = 0; i < nrows; i++)
      {
         object[i].resize(ncols);
         object[i] << 0.0;
      }
      int i = 0, j = 0;
      while (istr && (i < nrows))
      {
         istr >> object[i][j];
         if (j == (ncols - 1))
         {
            j = 0;
            i++;
         }
         else
            j++;
      }
      if (i < nrows)
      {
         EXCEPTION_MNGR(std::runtime_error, "Problem parsing dense matrix: too few rows were parsed");
      }
   }
   catch (std::exception& err)
   {
      std::cerr << "Error parsing BasicArray<int> object data" << std::endl;
      throw;
   }
}

inline void read_xml_matrix(utilib::Any& object, TiXmlElement* elt, std::string& mat_type)
{
   if (mat_type == "dense")
   {
      utilib::BasicArray<utilib::BasicArray<double> > & mat = object.set< utilib::BasicArray<utilib::BasicArray<double> > > ();
      read_xml_object(mat, elt);
   }
   else
   {
      EXCEPTION_MNGR(std::runtime_error, "colin::read_xml_matrix - Matrix type \"" << mat_type << "\" is not currently supported.");
   }
}

}

#endif

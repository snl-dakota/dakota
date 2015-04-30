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

/**
 * \file AppResponseXML.h
 *
 * XML utility classes.
 */

#ifndef colin_AppResponseXML_h
#define colin_AppResponseXML_h

#include <acro_config.h>
#ifdef ACRO_USING_TINYXML
#  include <tinyxml/tinyxml.h>
#endif
#include <utilib/Any.h>
#include <utilib/BasicArray.h>
#include <utilib/BitArray.h>
#include <utilib/MixedIntVars.h>
#include <colin/AppResponseInfo.h>

namespace colin
{

///
class AppResponseXMLElement
{
public:
   /// Empty virtual destructor to silence compiler warnings
   virtual ~AppResponseXMLElement() {}

   ///
   virtual const char* element_name() const = 0;

#ifdef ACRO_USING_TINYXML
   ///
   virtual utilib::Any process(TiXmlElement* node) const = 0;
#endif

   ///
   response_info_t response_info;
};


void AppResponseXML_register(const response_info_t *id,
                             AppResponseXMLElement* elt);
const AppResponseXMLElement* AppResponseXML(response_info_t id,
      AppResponseXMLElement* elt = 0);
response_info_t AppResponseXML(const char* str);


#ifdef ACRO_USING_TINYXML
template <class DomainT>
TiXmlElement* XMLParameters(const DomainT& unknown)
{
   std::ostringstream ostr;
   ostr << "Function colin::XMLParameterText not specified for domain type "
   << typeid(unknown).name();
   TiXmlElement* ans = new TiXmlElement("Domain");
   ans->LinkEndChild(new TiXmlText(ostr.str().c_str()));
   return ans;
}

template <>
inline TiXmlElement* XMLParameters(const std::vector<double>& value)
{
   std::ostringstream ostr;
   for (size_t i = 0; i < value.size(); i++)
   { ostr << value[i] << " "; }
   TiXmlElement* ans = new TiXmlElement("Domain");
   TiXmlElement* reals = new TiXmlElement("Real");
   reals->SetAttribute("size", value.size());
   ans->LinkEndChild(reals);
   reals->LinkEndChild(new TiXmlText(ostr.str().c_str()));
   return ans;
}

template <>
inline TiXmlElement* XMLParameters(const std::vector<int>& value)
{
   std::ostringstream ostr;
   for (size_t i = 0; i < value.size(); i++)
   { ostr << value[i] << " "; }
   TiXmlElement* ans = new TiXmlElement("Domain");
   TiXmlElement* ints = new TiXmlElement("Integer");
   ints->SetAttribute("size", value.size());
   ans->LinkEndChild(ints);
   ints->LinkEndChild(new TiXmlText(ostr.str().c_str()));
   return ans;
}

template <>
inline TiXmlElement* XMLParameters(const utilib::BasicArray<double>& value)
{
   std::ostringstream ostr;
   for (size_t i = 0; i < value.size(); i++)
   { ostr << value[i] << " "; }
   TiXmlElement* ans = new TiXmlElement("Domain");
   TiXmlElement* reals = new TiXmlElement("Real");
   reals->SetAttribute("size", value.size());
   ans->LinkEndChild(reals);
   reals->LinkEndChild(new TiXmlText(ostr.str().c_str()));
   return ans;
}

template <>
inline TiXmlElement* XMLParameters(const utilib::BasicArray<int>& value)
{
   std::ostringstream ostr;
   for (size_t i = 0; i < value.size(); i++)
   { ostr << value[i] << " "; }
   TiXmlElement* ans = new TiXmlElement("Domain");
   TiXmlElement* ints = new TiXmlElement("Integer");
   ints->SetAttribute("size", value.size());
   ans->LinkEndChild(ints);
   ints->LinkEndChild(new TiXmlText(ostr.str().c_str()));
   return ans;
}

template <>
inline TiXmlElement* XMLParameters(const utilib::BitArray& value)
{
   std::ostringstream ostr;
   for (size_t i = 0; i < value.size(); i++)
   { ostr << value(i) << " "; }
   TiXmlElement* ans = new TiXmlElement("Domain");
   TiXmlElement* bits = new TiXmlElement("Binary");
   bits->SetAttribute("size", value.size());
   ans->LinkEndChild(bits);
   bits->LinkEndChild(new TiXmlText(ostr.str().c_str()));
   return ans;
}

template <>
inline TiXmlElement* XMLParameters(const utilib::MixedIntVars& value)
{
   std::ostringstream ostr_r;
   std::ostringstream ostr_i;
   std::ostringstream ostr_b;

   for (size_t i = 0; i < value.Real().size(); i++)
   { ostr_r << value.Real()[i] << " "; }
   for (size_t i = 0; i < value.Integer().size(); i++)
   { ostr_i << value.Integer()[i] << " "; }
   for (size_t i = 0; i < value.Binary().size(); i++)
   { ostr_b << value.Binary()(i) << " "; }

   TiXmlElement* ans = new TiXmlElement("Domain");

   TiXmlElement* reals = new TiXmlElement("Real");
   reals->SetAttribute("size", value.Real().size());
   ans->LinkEndChild(reals);
   reals->LinkEndChild(new TiXmlText(ostr_r.str().c_str()));

   TiXmlElement* ints = new TiXmlElement("Integer");
   ints->SetAttribute("size", value.Integer().size());
   ans->LinkEndChild(ints);
   ints->LinkEndChild(new TiXmlText(ostr_i.str().c_str()));

   TiXmlElement* bits = new TiXmlElement("Binary");
   bits->SetAttribute("size", value.Binary().size());
   ans->LinkEndChild(bits);
   bits->LinkEndChild(new TiXmlText(ostr_b.str().c_str()));

   return ans;
}

#endif

}

#endif

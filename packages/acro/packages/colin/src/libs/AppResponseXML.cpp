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

#include <acro_config.h>
#include <colin/AppResponseXML.h>

#ifdef ACRO_USING_TINYXML
#  include <colin/TinyXML.h>
#endif

using utilib::Any;
using std::cerr;
using std::endl;

using std::vector;

namespace colin
{

const AppResponseXMLElement* AppResponseXMLCore(response_info_t *id,
      AppResponseXMLElement* elt,
      const char* str)
{
   static std::map<response_info_t, AppResponseXMLElement*> xml_element;
   static std::map<std::string, response_info_t> response_id;
   static std::map<const response_info_t*, AppResponseXMLElement*>
   registration_queue;

   //
   // Register a response XML element
   //
   if (elt != 0)
   {
      registration_queue[id] = elt;
      return elt;
   }

   // flush the registration queue if people are starting to *use* the registry
   while (! registration_queue.empty())
   {
      response_info_t _id = *registration_queue.begin()->first;
      AppResponseXMLElement* _elt = registration_queue.begin()->second;

      //std::cerr << "flushing registrations: " << _id << std::endl;
      xml_element[ _id ] = _elt;
      response_id[ _elt->element_name()] = _id;
      registration_queue.erase(registration_queue.begin());
   }

   //
   // Given a response id, return the XML element
   //
   if (str == 0)
   {
      std::map<response_info_t, AppResponseXMLElement*>::iterator curr
      = xml_element.find(*id);
      if (curr == xml_element.end())
      {
         EXCEPTION_MNGR(std::runtime_error, "Cannot find an AppResponse XML "
                        "element handler: " << AppResponseInfo().name(*id));
      }
      return curr->second;
   }
   //
   //
   //
   if (str != 0)
   {
      std::map<std::string, response_info_t>::iterator curr
      = response_id.find(str);
      if (curr == response_id.end())
      {
         EXCEPTION_MNGR(std::runtime_error, "Cannot find an AppResponse XML "
                        "element with name " << str);
      }
      *id = curr->second;
      return 0;
   }

   // JDS: I don't know if this is the right result, but a non-void
   // function must return *something*
   return NULL;
}


void AppResponseXML_register(const response_info_t *id,
                             AppResponseXMLElement* elt)
{
   AppResponseXMLCore(const_cast<response_info_t*>(id), elt, 0);
}

const AppResponseXMLElement* AppResponseXML(response_info_t id,
      AppResponseXMLElement* elt)
{
   return AppResponseXMLCore(&id, elt, 0);
}


response_info_t AppResponseXML(const char* str)
{
   response_info_t tmp;
   AppResponseXMLCore(&tmp, 0, str);
   return tmp;
}



//----------------------------------------------------------------
// AppResponseXMLElement helper classes
//----------------------------------------------------------------

class F_AppResponseXMLElement : public AppResponseXMLElement
{
public:
   /// Empty virtual destructor to silence compiler warnings
   virtual ~F_AppResponseXMLElement() {}

   const char* element_name() const
      { return "FunctionValue"; }

#ifdef ACRO_USING_TINYXML
   utilib::Any process(TiXmlElement* node) const
   {
      Any any;
      try
      {
         std::istringstream istr(node->GetText());
         istr >> any.set<colin::real>();
      }
      catch (std::exception& err)
      {
         cerr << "Error parsing FunctionValue XML value" << endl;
         throw;
      }
      return any;
   }
#endif
};

class MF_AppResponseXMLElement : public AppResponseXMLElement
{
public:
   /// Empty virtual destructor to silence compiler warnings
   virtual ~MF_AppResponseXMLElement() {}

   const char* element_name() const
      { return "FunctionValues"; }

#ifdef ACRO_USING_TINYXML
   utilib::Any process(TiXmlElement* node) const
   {
      Any any;
      try
      {
         std::istringstream istr(node->GetText());
         parse_array<colin::real, vector<colin::real> > (istr, any);
      }
      catch (std::exception& err)
      {
         cerr << "Error parsing FunctionValues XML value" << endl;
         throw;
      }
      return any;
   }
#endif
};


class CF_AppResponseXMLElement : public AppResponseXMLElement
{
public:
   /// Empty virtual destructor to silence compiler warnings
   virtual ~CF_AppResponseXMLElement() {}

   const char* element_name() const
      { return "NonlinearConstraintValues"; }

#ifdef ACRO_USING_TINYXML
   utilib::Any process(TiXmlElement* node) const
   {
      Any any;
      try
      {
         std::istringstream istr(node->GetText());
         parse_array<colin::real, vector<colin::real> > (istr, any);
      }
      catch (std::exception& err)
      {
         cerr << "Error parsing NonlinearConstraintValues XML value" << endl;
         throw;
      }
      return any;
   }
#endif
};


class G_AppResponseXMLElement : public AppResponseXMLElement
{
public:
   /// Empty virtual destructor to silence compiler warnings
   virtual ~G_AppResponseXMLElement() {}

   const char* element_name() const
      { return "Gradient"; }

#ifdef ACRO_USING_TINYXML
   utilib::Any process(TiXmlElement* node) const
   {
      Any any;
      try
      {
         std::istringstream istr(node->GetText());
         parse_array<colin::real, vector<colin::real> > (istr, any);
      }
      catch (std::exception& err)
      {
         cerr << "Error parsing Gradient XML value" << endl;
         throw;
      }
      return any;
   }
#endif
};


class CG_AppResponseXMLElement : public AppResponseXMLElement
{
public:
   /// Empty virtual destructor to silence compiler warnings
   virtual ~CG_AppResponseXMLElement() {}

   const char* element_name() const
      { return "Jacobian"; }

#ifdef ACRO_USING_TINYXML
   utilib::Any process(TiXmlElement* node) const
   {
      Any any;
      try
      {
         std::istringstream istr(node->GetText());
         parse_array<colin::real, vector<colin::real> > (istr, any);
      }
      catch (std::exception& err)
      {
         cerr << "Error parsing Jacobian XML value" << endl;
         throw;
      }
      return any;
   }
#endif
};


namespace {

template <class ElementT>
bool info_register(const response_info_t *id)
{
   static ElementT element;
   AppResponseXML_register(id, &element);
   return true;
}

const bool f_reg = info_register<F_AppResponseXMLElement>(&f_info);
const bool mf_reg = info_register<MF_AppResponseXMLElement>(&mf_info);
//const bool cf_reg = info_register<CF_AppResponseXMLElement>(&cf_info);
const bool nlcf_reg = info_register<CF_AppResponseXMLElement>(&nlcf_info);
const bool g_reg = info_register<G_AppResponseXMLElement>(&g_info);
const bool cg_reg = info_register<CG_AppResponseXMLElement>(&cg_info);

} // namespace (local)



} // namespace colin

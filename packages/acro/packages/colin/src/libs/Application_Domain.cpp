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

#include <colin/application/Domain.h>

#include <utilib/MixedIntVars.h>
#include <utilib/TinyXML_helper.h>


using std::string;
using std::map;

using boost::bind;

using utilib::Any;
using utilib::MixedIntVars;

namespace colin {

namespace Domain {

namespace {

int lexicalCast_MixedIntVars_realPart(const Any& from_, Any& to_)
{
   const MixedIntVars& from = from_.expose<MixedIntVars>();
   RealComponent &ans = to_.set<RealComponent>();
   ans.data << from.Real();
   ans.source_index.reserve(ans.data.size());
   size_t n = from.Binary().size() + from.Integer().size();
   for(size_t i = ans.data.size(); i > 0; --i)
      ans.source_index.push_back(n++);
   return OK;
}

int lexicalCast_MixedIntVars_intPart(const Any& from_, Any& to_)
{
   const MixedIntVars& from = from_.expose<MixedIntVars>();
   IntegerComponent &ans = to_.set<IntegerComponent>();
   ans.data << from.Integer();
   ans.source_index.reserve(ans.data.size());
   size_t n = from.Binary().size();
   for(size_t i = ans.data.size(); i > 0; --i)
      ans.source_index.push_back(n++);
   return OK;
}

int lexicalCast_MixedIntVars_binPart(const Any& from_, Any& to_)
{
   const MixedIntVars& from = from_.expose<MixedIntVars>();
   BinaryComponent &ans = to_.set<BinaryComponent>();

   size_t idx = 0;
   size_t n = from.Binary().size();
   ans.data.reserve(n);
   ans.source_index.reserve(n);
   for(size_t i = 0; i < n; ++i)
   {
      ans.data.push_back(from.Binary()[i]);
      ans.source_index.push_back(idx++);
   }
   return OK;
}

int lexicalCast_MixedIntVars_discretePart(const Any& from_, Any& to_)
{
   const MixedIntVars& from = from_.expose<MixedIntVars>();
   DiscreteComponent &ans = to_.set<DiscreteComponent>();

   size_t idx = 0;
   size_t n = from.Binary().size() + from.Integer().size();
   ans.data.reserve(n);
   ans.source_index.reserve(n);

   n = from.Binary().size();
   for(size_t i = 0; i < n; ++i)
   {
      ans.data.push_back(from.Binary()[i]);
      ans.source_index.push_back(idx++);
   }
   n = from.Integer().size();
   for(size_t i = 0; i < n; ++i)
   {
      ans.data.push_back(from.Integer()[i]);
      ans.source_index.push_back(idx++);
   }
   return OK;
}

int lexicalCast_MixedIntVars_differentiablePart(const Any& from_, Any& to_)
{
   const MixedIntVars& from = from_.expose<MixedIntVars>();
   DifferentiableComponent &ans = to_.set<DifferentiableComponent>();
   ans.data << from.Real();
   ans.source_index.reserve(ans.data.size());
   size_t n = from.Binary().size() + from.Integer().size();
   for(size_t i = ans.data.size(); i > 0; --i)
      ans.source_index.push_back(n++);
   return OK;
}

bool LocalRegistrations()
{
   utilib::TypeManager()->register_lexical_cast
      (typeid(MixedIntVars), typeid(RealComponent), 
       &lexicalCast_MixedIntVars_realPart);
   
   utilib::TypeManager()->register_lexical_cast
      (typeid(MixedIntVars), typeid(IntegerComponent), 
       &lexicalCast_MixedIntVars_intPart);
   
   utilib::TypeManager()->register_lexical_cast
      (typeid(MixedIntVars), typeid(BinaryComponent), 
       &lexicalCast_MixedIntVars_binPart);

   utilib::TypeManager()->register_lexical_cast
      (typeid(MixedIntVars), typeid(DiscreteComponent), 
       &lexicalCast_MixedIntVars_discretePart);

   utilib::TypeManager()->register_lexical_cast
      (typeid(MixedIntVars), typeid(DifferentiableComponent), 
       &lexicalCast_MixedIntVars_differentiablePart);

   return true;
}

const bool local_registrations = LocalRegistrations();

} // namespace colin::Domain::(local)
} // namespace colin::Domain

namespace {

// A no-op property updater
void noop_set(  Any&, const Any )
{}

} // namespace colin::(local)


//--------------------------------------------------------------------
// Application_Domain Public member functions
//--------------------------------------------------------------------

Application_Domain::Application_Domain()
   : _enforcing_domain_bounds(utilib::Property::Bind<bool>(false)),
     domain_size(),
     enforcing_domain_bounds(_enforcing_domain_bounds.set_readonly())
{
   utilib::Privileged_Property _domain_size
      (&noop_set, bind(&Application_Domain::cb_get_domain_size, this, _1, _2));
   domain_size = _domain_size.set_readonly();

   ObjectType t = ObjectType::get(this);
   register_application_component(t);
   properties.declare("enforcing_domain_bounds", _enforcing_domain_bounds, t);
   properties.declare("domain_size", _domain_size, t);

   initializer("Domain").connect
      (boost::bind(&Application_Domain::cb_initialize, this, _1));

   _enforcing_domain_bounds = false;
}


/// Virtual destructor
Application_Domain::~Application_Domain()
{}


//--------------------------------------------------------------------
// Application_Domain private member functions
//--------------------------------------------------------------------

///
void Application_Domain::cb_initialize(TiXmlElement* element)
{
   TiXmlElement* node = element->FirstChildElement();
   while ( node != NULL )
   {
      cb_initializer_t &init = domain_initializer(node->ValueStr());
      if ( init.empty() )
         EXCEPTION_MNGR(std::runtime_error, "Application_Domain::"
                        "cb_initialize(): Unrecognized domain " << 
                        utilib::get_element_info(node));

      init(node);
      node = node->NextSiblingElement();
   }
}

/*
///
void 
Application_Domain::
register_initialize_domain( fcn_cb_tixml_t fcn, std::string element )
{
   if ( fcn == NULL )
      init_domain_map.erase(element);
   else if ( ! init_domain_map.insert( std::make_pair(element, fcn) ).second )
      EXCEPTION_MNGR(std::runtime_error,
                     "Duplicate derived Application_Domain::"
                     "init_domain(TiXmlElement*) registration for "
                     "element " << element);
}
*/

} // namespace colin

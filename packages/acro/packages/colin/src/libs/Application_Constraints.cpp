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
 * \file Application_Constraints.cpp
 *
 * Defines the colin::Application_Constraints class members.
 */

#include <colin/application/Constraints.h>
#include <colin/real.h>

#include <utilib/TinyXML_helper.h>

#include <limits>

using std::cerr;
using std::endl;
using std::string;

using std::vector;
using std::list;
using std::map;
using std::pair;
using std::make_pair;

using utilib::seed_t;
using utilib::Any;
using utilib::Property;
using utilib::Privileged_Property;

using boost::bind;

namespace colin {

namespace {

// A no-op property updater
void noop_set(  utilib::Any&, const utilib::Any )
{}

} // namespace colin::(local)

//--------------------------------------------------------------------
// Application_Constraints Public member functions
//--------------------------------------------------------------------

Application_Constraints::Application_Constraints()
   : equality_epsilon(),
     num_constraints()
{
   Privileged_Property _num_constraints
      ( &noop_set, 
        bind(&Application_Constraints::cb_get_num_constraints, this, _1, _2) );
   num_constraints = _num_constraints.set_readonly();

   Privileged_Property _equality_epsilon
      ( Property::Bind(std::numeric_limits<double>::epsilon() * 8.0) );
   equality_epsilon = _equality_epsilon;

   ObjectType type = ObjectType::get(this);
   register_application_component(type);
   properties.declare( "equality_epsilon", _equality_epsilon, type );
   properties.declare( "num_constraints", _num_constraints, type );


   request_expansion_signal.connect
      (boost::bind(&Application_Constraints::cb_expand_request, this, _1));
   request_transform_signal.connect
      (boost::bind(&Application_Constraints::cb_map_request, this, _1, _2));

   response_transform_signal.connect
      (boost::bind( &Application_Constraints::cb_map_response, this, 
                    _1, _2, _3, _4 ));

   initializer("Constraints").connect
      (boost::bind(&Application_Constraints::cb_initialize, this, _1));
}


/// Virtual destructor
Application_Constraints::~Application_Constraints()
{ }

//--------------------------------------------------------------------
// Application_Constraints public information methods
//--------------------------------------------------------------------


//--------------------------------------------------------------------
// Application_Constraints public request & evaluation methods
//--------------------------------------------------------------------


// The following define the implementations of the Eval* and AsyncEval*
// convenience functions declared as part of the DECLARE_REQUEST()
// macro.  See application/Base.h for macro expansion.

DEFINE_REQUEST( Application_Constraints, 
                CF,     cf_info,     constraint );

DEFINE_REQUEST( Application_Constraints, 
                CFViol, cvf_info,    constraint violation );

DEFINE_REQUEST( Application_Constraints, 
                EqCF,   eqcf_info,   equality constraint );

DEFINE_REQUEST( Application_Constraints, 
                IneqCF, ineqcf_info, inequality constraint );


//--------------------------------------------------------------------
// Application_Constraints public access methods
//--------------------------------------------------------------------


/// Return constraint bounds
void
Application_Constraints::
get_constraint_bounds(utilib::AnyFixedRef lower,
                      utilib::AnyFixedRef upper) const
{
   bool dataPresent = true;
   AppResponse::response_map_t dummy;

   Any lo = collect_components(dataPresent, dummy, LOWER_BOUND, BOTH);
   Any up = collect_components(dataPresent, dummy, UPPER_BOUND, BOTH);

   if ( ! dataPresent )
      EXCEPTION_MNGR(std::runtime_error, "Application_Constraints::"
                     "get_constraint_bounds(): component returned no data.");

   utilib::TypeManager()->lexical_cast(lo, lower);
   utilib::TypeManager()->lexical_cast(up, upper);
}


//--------------------------------------------------------------------
// Application_Constraints protected member functions
//--------------------------------------------------------------------


//--------------------------------------------------------------------
// Application_Constraints private member functions
//--------------------------------------------------------------------

void
Application_Constraints::
cb_expand_request( AppRequest::request_map_t &requests ) const
{
   // Expand the generic Constraint requests to compute the constraint
   // components
   if ( requests.count(cf_info) || 
        requests.count(cvf_info) || 
        requests.count(eqcf_info) || 
        requests.count(ineqcf_info) )
      constraint_request_signal(requests);
}


void
Application_Constraints::
cb_map_request( const AppRequest::request_map_t &outer_requests,
                AppRequest::request_map_t &inner_requests ) const
{
   static_cast<void>(outer_requests);
   // Remove the "pseudo requests" that this class inserts into the
   // request map
   inner_requests.erase(cf_info);
   inner_requests.erase(cvf_info);
   inner_requests.erase(eqcf_info);
   inner_requests.erase(ineqcf_info);
}


int
Application_Constraints::
cb_map_response( const utilib::Any &domain,
                 const AppRequest::request_map_t& outer_request,
                 const AppResponse::response_map_t& inner_response,
                 AppResponse::response_map_t& response) const
{
   //std::cerr << utilib::demangledName(typeid(this)) << std::endl;
   static_cast<void>(domain);
   static_cast<void>(inner_response);

   Any tmp;
   int ans = 0;

   if ( outer_request.count(cf_info) && ! response.count(cf_info) )
   {
      bool DataPresent = true;
      tmp = collect_components(DataPresent, response, VALUE, BOTH);
      if ( ! DataPresent )
         return ans;

      response.insert(make_pair(cf_info, tmp)); 
      ++ans;
   }

   if ( outer_request.count(cvf_info) && ! response.count(cvf_info) )
   {
      bool DataPresent = true;
      tmp = collect_components(DataPresent, response, VIOLATION, BOTH);
      if ( ! DataPresent )
         return ans;

      response.insert(make_pair(cvf_info, tmp)); 
      ++ans;
   }

   if ( outer_request.count(eqcf_info) && ! response.count(eqcf_info) )
   {
      bool DataPresent = true;
      tmp = collect_components(DataPresent, response, VALUE, EQUALITY);
      if ( ! DataPresent )
         return ans;

      response.insert(make_pair(eqcf_info, tmp)); 
      ++ans;
   }

   if ( outer_request.count(ineqcf_info) && ! response.count(ineqcf_info) )
   {
      bool DataPresent = true;
      tmp = collect_components(DataPresent, response, VALUE, INEQUALITY);
      if ( ! DataPresent )
         return ans;

      response.insert(make_pair(ineqcf_info, tmp)); 
      ++ans;
   }

   return -1;
}

void
Application_Constraints::
cb_initialize(TiXmlElement* element)
{
   TiXmlElement* node = element->FirstChildElement();
   while ( node != NULL )
   {
      cb_initializer_t &init = constraint_initializer(node->ValueStr());
      if ( init.empty() )
         EXCEPTION_MNGR(std::runtime_error, "Application_Constraints::"
                        "cb_initialize(): Unrecognized constraints " << 
                        utilib::get_element_info(node));

      init(node);
      node = node->NextSiblingElement();
   }
}



utilib::Any
Application_Constraints::
collect_components( bool& dataPresent, AppResponse::response_map_t& response, 
                    VectorType type, EqualityFilter equality ) const
{
   list<Any> parts;
   size_t count = 
      constraint_info_signal(type, equality, dataPresent, parts, response);

   if ( ! dataPresent )
      return Any();

   Any ans;
   // FIXME: This restricts the type space for domain and response to reals!
   //    We should consider creating an AnyVector that can support merging 
   //    vectors of unknown types.
   vector<real> &buffer = ans.set<vector<real> >();
   buffer.reserve(count);

   // This trick saves a copy if the cb returned the correct type
   while ( ! parts.empty() )
   {
      Any tmp;
      utilib::TypeManager()->lexical_cast
         (parts.front(), tmp, typeid(vector<real>));

      const vector<real> &tmp_v = tmp.expose<vector<real> >();
      buffer.insert(buffer.end(), tmp_v.begin(), tmp_v.end());
      parts.pop_front();
   }

   if ( buffer.size() != count )
      EXCEPTION_MNGR(std::runtime_error, "Application_Constraints::"
                     "collect_components(): collected vector size does not "
                     "match reported count (" << buffer.size() << " != " 
                     << count << ")");

   return ans;
}

size_t
Application_Constraints::
count_components(EqualityFilter equality) const
{
   list<Any> t1;
   AppResponse::response_map_t t2;

   bool dataPresent = true;
   size_t ans = constraint_info_signal(COUNT, equality, dataPresent, t1, t2);
   if ( ! dataPresent )
      EXCEPTION_MNGR(std::runtime_error, "Application_Constraints::"
                     "count_components(): component returned no data.");
   return ans;
}


} // namespace colin

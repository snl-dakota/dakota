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
 * \file Application_Jacobian.cpp
 *
 * Defines the colin::Application_Jacobian class members.
 */

#include <colin/application/Jacobian.h>
#include <utilib/SparseMatrix.h>

using std::cerr;
using std::endl;
using std::make_pair;
using std::vector;
using std::list;
using utilib::Any;

namespace colin {

typedef utilib::RMSparseMatrix<double>  jacobian_t;

//--------------------------------------------------------------------
// Application_Jacobian Public member functions
//--------------------------------------------------------------------

/// Constructor
Application_Jacobian::Application_Jacobian()
{
   register_application_component(ObjectType::get(this));

   request_expansion_signal.connect
      (boost::bind(&Application_Jacobian::cb_expand_request, this, _1));
   request_transform_signal.connect
      (boost::bind(&Application_Jacobian::cb_map_request, this, _1, _2));

   response_transform_signal.connect
      (boost::bind( &Application_Jacobian::cb_map_response, this, 
                    _1, _2, _3, _4 ));
}


/// Virtual destructor
Application_Jacobian::~Application_Jacobian()
{}


/// Register Jacobian matrix evaluation
DEFINE_REQUEST(Application_Jacobian, CG, cg_info, Jacobian);
/// Register equality Jacobian matrix evaluation
DEFINE_REQUEST( Application_Jacobian, EqCG, eqcg_info, 
                equality constraint Jacobian );
/// Register inequality Jacobian matrix evaluation
DEFINE_REQUEST( Application_Jacobian, IneqCG, ineqcg_info, 
                inequality constraint Jacobian );


//--------------------------------------------------------------------
// Application_Jacobian private member functions
//--------------------------------------------------------------------

void
Application_Jacobian::
cb_expand_request( AppRequest::request_map_t &requests ) const
{
   // Expand the generic Constraint requests to compute the constraint
   // components
   if ( requests.count(cg_info) || 
        requests.count(eqcg_info) || 
        requests.count(ineqcg_info) )
      jacobian_request_signal(requests);
}


void
Application_Jacobian::
cb_map_request( const AppRequest::request_map_t &outer_requests,
                AppRequest::request_map_t &inner_requests ) const
{
   static_cast<void>(outer_requests);
   // Remove the "pseudo requests" that this class inserts into the
   // request map
   inner_requests.erase(cg_info);
   inner_requests.erase(eqcg_info);
   inner_requests.erase(ineqcg_info);
}


int
Application_Jacobian::
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

   if ( outer_request.count(cg_info) && ! response.count(cg_info) )
   {
      bool DataPresent = true;
      tmp = collect_components(DataPresent, response, BOTH);
      if ( ! DataPresent )
         return ans;

      response.insert(make_pair(cg_info, tmp)); 
      ++ans;
   }

   if ( outer_request.count(eqcg_info) && ! response.count(eqcg_info) )
   {
      bool DataPresent = true;
      tmp = collect_components(DataPresent, response, EQUALITY);
      if ( ! DataPresent )
         return ans;

      response.insert(make_pair(eqcg_info, tmp)); 
      ++ans;
   }

   if ( outer_request.count(ineqcg_info) && ! response.count(ineqcg_info) )
   {
      bool DataPresent = true;
      tmp = collect_components(DataPresent, response, INEQUALITY);
      if ( ! DataPresent )
         return ans;

      response.insert(make_pair(ineqcg_info, tmp)); 
      ++ans;
   }

   return -1;
}


utilib::Any
Application_Jacobian::
collect_components( bool& dataPresent, AppResponse::response_map_t& response, 
                    EqualityFilter equality ) const
{
   list<Any> parts;
   size_t count = jacobian_info_signal(equality, dataPresent, parts, response);

   if ( ! dataPresent )
      return Any();

   Any ans;
   // FIXME: This restricts the type space for domain and response to reals!
   //    We should consider creating an AnyVector that can support merging 
   //    vectors of unknown types.
   jacobian_t &buffer = ans.set<jacobian_t>();

   // This trick saves a copy if the cb returned the correct type
   while ( ! parts.empty() )
   {
      Any tmp;
      utilib::TypeManager()->lexical_cast
         (parts.front(), tmp, typeid(jacobian_t));

      buffer.adjoinRows(tmp.expose<jacobian_t>());
      parts.pop_front();
   }

   if ( static_cast<size_t>(buffer.get_nrows()) != count )
      EXCEPTION_MNGR(std::runtime_error, "Application_Jacobian::"
                     "collect_components(): collected matrix size does not "
                     "match reported count (" << buffer.get_nrows() << " != " 
                     << count << ")");

   return ans;
}

} // namespace colin

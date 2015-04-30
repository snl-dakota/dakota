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
 * \file Application_Jacobian.h
 *
 * Defines the colin::Application_Jacobian class.
 */

#ifndef colin_Application_Jacobian_h
#define colin_Application_Jacobian_h

#include <acro_config.h>
#include <colin/application/Base.h>

namespace colin
{

//============================================================================
//============================================================================
// Class Application_Jacobian
//============================================================================
//============================================================================

/**
 *  Defines the elements of an Application that pertain to problems
 *  where Jacobian (constraint gradient) information is available.
 */
class Application_Jacobian : virtual public Application_Base
{
public: // methods

   /// Constructor
   Application_Jacobian();

   /// Virtual destructor
   virtual ~Application_Jacobian();

   /// Register a Jacobian evaluation computation
   DECLARE_REQUEST(CG);
   /// Register a Jacobian evaluation computation
   DECLARE_REQUEST(EqCG);
   /// Register a Jacobian evaluation computation
   DECLARE_REQUEST(IneqCG);

protected: // methods

   enum EqualityFilter { BOTH, EQUALITY, INEQUALITY };

protected: // data

   boost::signal<void(AppRequest::request_map_t&)>  jacobian_request_signal;

   boost::signal< size_t( EqualityFilter,
                          bool&,
                          std::list<utilib::Any>&,
                          AppResponse::response_map_t& ),
                  boost_extras::sum<size_t> >  jacobian_info_signal;

private:  // methods

   ///
   void
   cb_expand_request( AppRequest::request_map_t &requests ) const;
   
   ///
   void
   cb_map_request( const AppRequest::request_map_t &outer_,
                   AppRequest::request_map_t &remote ) const;

   int
   cb_map_response( const utilib::Any &domain,
                    const AppRequest::request_map_t& outer_request,
                    const AppResponse::response_map_t& inner_response,
                    AppResponse::response_map_t& response) const;

   utilib::Any
   collect_components(bool& dataPresent, AppResponse::response_map_t& response,
                      EqualityFilter equality) const;
};



} // namespace colin

#endif // defined colin_Application_Jacobian_h

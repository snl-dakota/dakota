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
 * \file Application_Constraints.h
 *
 * Declares the colin::Application_Constraints class.
 */

#ifndef colin_Application_Constraints_h
#define colin_Application_Constraints_h

#include <acro_config.h>
#include <colin/application/Base.h>
#include <colin/BoostExtras.h>

namespace colin
{

//============================================================================
//============================================================================
// Class Application_Constraints
//============================================================================
//============================================================================

/**
 *  Defines the elements of an Application that pertain to problems
 *  where constraints information is available.
 */
class Application_Constraints 
   : virtual public Application_Base
{
public:
   /// Constructor
   Application_Constraints();

   /// Virtual destructor
   virtual ~Application_Constraints();


   /* Information methods */

   /// Returns the number of equality constraints
   size_t numEqConstraints() const
   { return count_components(EQUALITY); }

   /// Returns the number of inequality constraints
   size_t numIneqConstraints() const
   { return count_components(INEQUALITY); }


   /* Request & Evaluation declarations */

   /// Queue general constraints evaluation
   DECLARE_REQUEST(CF);      // cf_info
   /// Queue general constraint violation evaluation
   DECLARE_REQUEST(CFViol);  // cvf_info
   /// Queue general equality constraints evaluation
   DECLARE_REQUEST(EqCF);    // eqcf_info
   /// Queue general inequality constraints evaluation
   DECLARE_REQUEST(IneqCF);  // ineqcf_info


   /* Access methods */

   /// Get constraint bounds - dense format
   void get_constraint_bounds(utilib::AnyFixedRef lower,
                              utilib::AnyFixedRef upper) const;

public: // data

   /// The maximum difference between upper and lower bound for a
   /// constraint to be considered an "equality" constrint
   utilib::Property equality_epsilon;

   /// The total number of constraints in the problem domain
   utilib::ReadOnly_Property num_constraints;

protected: // methods

   /// Register derived class callback function for initialize()
   cb_initializer_t& constraint_initializer(std::string element)
   { return initializer("Constraints::" + element); }

   enum VectorType { COUNT, VALUE, VIOLATION, LOWER_BOUND, UPPER_BOUND };

   enum EqualityFilter { BOTH, EQUALITY, INEQUALITY };

protected: // data

   boost::signal<void(AppRequest::request_map_t&)>  constraint_request_signal;

   boost::signal< size_t( VectorType,
                          EqualityFilter,
                          bool&,
                          std::list<utilib::Any>&,
                          AppResponse::response_map_t& ),
                  boost_extras::sum<size_t> >  constraint_info_signal;

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

   void cb_get_num_constraints( const utilib::Any&, utilib::Any& ans)
   { ans = count_components(BOTH); }

   void cb_initialize(TiXmlElement*);

   utilib::Any
   collect_components(bool& dataPresent, AppResponse::response_map_t& response,
                      VectorType type, EqualityFilter equality) const;

   size_t count_components(EqualityFilter equality) const;

};



} // namespace colin

#endif // defined colin_Application_Constraints_h

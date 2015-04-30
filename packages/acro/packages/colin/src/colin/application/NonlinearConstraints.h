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
 * \file Application_NonlinearConstraints.h
 *
 * Defines the colin::Application_NonlinearConstraints class.
 */

#ifndef colin_Application_NonlinearConstraints_h
#define colin_Application_NonlinearConstraints_h

#include <acro_config.h>
#include <colin/application/Constraints.h>
#include <colin/application/Jacobian.h>

namespace colin {

// forward declaration of the unit testing class
namespace unittest { class Application_NonlinearConstraints; }


//============================================================================
//============================================================================
// Class Application_NonlinearConstraints
//============================================================================
//============================================================================

/**
 *  Defines the elements of a Application that pertain to nonlinear
 *  constraints.
 *
 *  Any concrete application implementing this class MUST provide
 *  responses to the nlcf_info response info type.  This interface will
 *  calculate nleqcf_info and nlineqcf_info requests on the concrete
 *  application's behalf.
 */
class Application_NonlinearConstraints
   : virtual public Application_Constraints
{
   friend class unittest::Application_NonlinearConstraints;
protected:
   utilib::Privileged_Property _num_nonlinear_constraints;
   utilib::Privileged_Property _nonlinear_constraint_lower_bounds;
   utilib::Privileged_Property _nonlinear_constraint_upper_bounds;
   utilib::Privileged_Property _nonlinear_constraint_labels;

public: // methods

   /// Constructor
   Application_NonlinearConstraints();

   /// Destructor
   virtual ~Application_NonlinearConstraints();


   /* Information methods */

   /// Returns the number of nonlinear equality constraints
   size_t numNonlinearEqConstraints() const;

   /// Returns the number of nonlinear inequality constraints
   size_t numNonlinearIneqConstraints() const;


   /* Request & Evaluation declarations */

   /// Register a nonlinear constraint evaluation computation
   DECLARE_REQUEST(NLCF);
   /// Register a nonlinear constraint evaluation computation
   DECLARE_REQUEST(NLCFViol);
   /// Register a nonlinear equality constraint evaluation computation
   DECLARE_REQUEST(NLEqCF);
   /// Register a nonlinear inequality constraint evaluation computation
   DECLARE_REQUEST(NLIneqCF);


   /* Access methods */

   /// Get the bounds for a specific nonlinear constraint 
   void nonlinearConstraintBound( size_t index,
                                  utilib::AnyFixedRef lower,
                                  utilib::AnyFixedRef upper ) const;

   /// Get nonlinear equality constraint bounds - dense format
   void nonlinearEqConstraintBounds( utilib::AnyFixedRef rhs ) const;

   /// Get nonlinear inequality constraint bounds - dense format
   void nonlinearIneqConstraintBounds( utilib::AnyFixedRef lower,
                                       utilib::AnyFixedRef upper ) const;

   /// Get the label for a specific nonlinear constraint
   std::string nonlinearConstraintLabel(size_t i) const;

public: // data

   /// Total number of linear constraints
   utilib::ReadOnly_Property num_nonlinear_constraints;

   /// The lower bounds for all linear constraint residuals
   utilib::ReadOnly_Property nonlinear_constraint_lower_bounds;

   /// The upper bounds for all linear constraint residuals
   utilib::ReadOnly_Property nonlinear_constraint_upper_bounds;

   /// The labels for all linear constraint residuals
   utilib::ReadOnly_Property nonlinear_constraint_labels;


protected: // methods

   /// Set a single label for the nonlinear constraint
   void _setNonlinearConstraintLabel(const size_t i, const std::string &label);

private: // methods
   ///
   void cb_onChange_num( const utilib::ReadOnly_Property &prop );
   
   ///
   bool cb_validate_vector( const utilib::ReadOnly_Property &prop,
                            const utilib::Any &value );

   ///
   bool cb_validate_matrix( const utilib::ReadOnly_Property &prop,
                            const utilib::Any &value );

   ///
   bool cb_validate_labels( const utilib::ReadOnly_Property &prop,
                            const utilib::Any &value );

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

   void cb_constraint_request(AppRequest::request_map_t&) const;

   size_t cb_constraint_info( VectorType type, 
                              EqualityFilter equality, 
                              bool& dataPresent,
                              std::list<utilib::Any>& components, 
                              AppResponse::response_map_t& response ) const;

   ///
   void cb_print(std::ostream& os);

   void cb_initialize(TiXmlElement*);

};


//============================================================================
//============================================================================
// Class Application_NonlinearConstraintGradients
//============================================================================
//============================================================================

class Application_NonlinearConstraintGradients
   : virtual public Application_Jacobian,
     public Application_NonlinearConstraints
{
public: // methods

   /// Constructor
   Application_NonlinearConstraintGradients();

   /// Destructor
   virtual ~Application_NonlinearConstraintGradients();


   /* Request & Evaluation declarations */

   /// Register a nonlinear constraint evaluation computation
   DECLARE_REQUEST(NLCG);
   /// Register a nonlinear equality constraint evaluation computation
   DECLARE_REQUEST(NLEqCG);
   /// Register a nonlinear inequality constraint evaluation computation
   DECLARE_REQUEST(NLIneqCG);

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

   void cb_jacobian_request(AppRequest::request_map_t&) const;

   size_t cb_jacobian_info( Application_Jacobian::EqualityFilter equality, 
                            bool& dataPresent,
                            std::list<utilib::Any>& components, 
                            AppResponse::response_map_t& response ) const;
};



template<bool active, bool grad>
class Application_Include_NonlinearConstraints
   : public Application_NonlinearConstraintGradients
{};

template<>
class Application_Include_NonlinearConstraints<true, false>
   : public Application_NonlinearConstraints
{};

template<bool grad>
class Application_Include_NonlinearConstraints<false, grad>
{};

} // namespace colin

#endif




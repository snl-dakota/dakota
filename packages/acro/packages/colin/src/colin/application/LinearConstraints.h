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
 * \file Application_LinearConstraints.h
 *
 * Defines the colin::Application_LinearConstraints class.
 */

#ifndef colin_Application__LinearConstraints_h
#define colin_Application__LinearConstraints_h

#include <acro_config.h>
#include <colin/application/Constraints.h>
#include <colin/application/Jacobian.h>

// forward declaration of the unit testing class
class Test_Application_LinearConstraints;

namespace colin {

// forward declaration of the unit testing class
namespace unittest { class Application_LinearConstraints; }


//============================================================================
//============================================================================
// Class Application_LinearConstraints
//============================================================================
//============================================================================

/**
 *  Defines the elements of an Application that pertain to linear
 *  constraints.
 *
 *  Any concrete application implementing this class MUST provide
 *  responses to the lcf_info response info type.  This interface will
 *  calculate leqcf_info and lineqcf_info requests on the concrete
 *  application's behalf.
 */
class Application_LinearConstraints
   : virtual public Application_Constraints
{
   friend class unittest::Application_LinearConstraints;
protected:
   utilib::Privileged_Property _num_linear_constraints;
   utilib::Privileged_Property _linear_constraint_lower_bounds;
   utilib::Privileged_Property _linear_constraint_upper_bounds;
   utilib::Privileged_Property _linear_constraint_labels;

public:
   /// Constructor
   Application_LinearConstraints();

   /// Destructor
   virtual ~Application_LinearConstraints();


   /* Information methods */

   /// Returns the number of linear equality constraints
   size_t numLinearEqConstraints() const;

   /// Returns the number of linear inequality constraints
   size_t numLinearIneqConstraints() const;


   /* Request & Evaluation declarations */

   /// Register a linear constraint evaluation computation
   DECLARE_REQUEST(LCF);
   /// Register a linear constraint evaluation computation
   DECLARE_REQUEST(LCFViol);
   /// Register a linear constraint evaluation computation
   DECLARE_REQUEST(LEqCF);
   /// Register a linear constraint evaluation computation
   DECLARE_REQUEST(LIneqCF);


   /* Access methods */

   /// Return linear constraint bounds for a specific constraint
   void linearConstraintBound( size_t index,
                               utilib::AnyFixedRef lower,
                               utilib::AnyFixedRef upper ) const;

   /// Get linear equality constraint bounds - dense format
   void linearEqConstraintBounds( utilib::AnyFixedRef rhs ) const;

   /// Get linear inequality constraint bounds - dense format
   void linearIneqConstraintBounds( utilib::AnyFixedRef lower,
                                    utilib::AnyFixedRef upper ) const;

   /// Get the label for a specific linear constraint
   std::string linearConstraintLabel(size_t i) const;

public: // data

   /// Total number of linear constraints
   utilib::ReadOnly_Property num_linear_constraints;

   /// The lower bounds for all linear constraint residuals
   utilib::ReadOnly_Property linear_constraint_lower_bounds;

   /// The upper bounds for all linear constraint residuals
   utilib::ReadOnly_Property linear_constraint_upper_bounds;

   /// The labels for all linear constraint residuals
   utilib::ReadOnly_Property linear_constraint_labels;


protected: // methods

   /// Set a single label for the linear constraint
   void _setLinearConstraintLabel(const size_t i, const std::string &label);

private: // methods
   ///
   void cb_onChange_num( const utilib::ReadOnly_Property &prop );
   
   ///
   bool cb_validate_vector( const utilib::ReadOnly_Property &prop,
                            const utilib::Any &value );

   ///
   bool cb_validate_labels( const utilib::ReadOnly_Property &prop,
                            const utilib::Any &value );

   ///
   utilib::Any
   compute_lcf( const utilib::Any &domain ) const;

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
// Class Application_LinearConstraintGradients
//============================================================================
//============================================================================

class Application_LinearConstraintGradients
   : virtual public Application_Jacobian,
     public Application_LinearConstraints
{
protected:
   utilib::Privileged_Property _linear_constraint_matrix;

public: // methods

   /// Constructor
   Application_LinearConstraintGradients();

   /// Destructor
   virtual ~Application_LinearConstraintGradients();


   /* Request & Evaluation declarations */

   /// Register a nonlinear constraint evaluation computation
   DECLARE_REQUEST(LCG);
   /// Register a nonlinear equality constraint evaluation computation
   DECLARE_REQUEST(LEqCG);
   /// Register a nonlinear inequality constraint evaluation computation
   DECLARE_REQUEST(LIneqCG);

public: // data

   /// The linear constraint matrix (if known)
   utilib::ReadOnly_Property linear_constraint_matrix;

private:  // methods
   ///
   void cb_onChange_num( const utilib::ReadOnly_Property &prop );
   
   ///
   bool cb_validate_matrix( const utilib::ReadOnly_Property &prop,
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

   void cb_jacobian_request(AppRequest::request_map_t&) const;

   size_t cb_jacobian_info( Application_Jacobian::EqualityFilter equality, 
                            bool& dataPresent,
                            std::list<utilib::Any>& components, 
                            AppResponse::response_map_t& response ) const;

   void cb_initialize(TiXmlElement*);
};




template<bool active, bool grad>
class Application_Include_LinearConstraints
   : public Application_LinearConstraintGradients
{};

template <>
class Application_Include_LinearConstraints<true, false> 
   : public Application_LinearConstraints
{};

template <bool grad>
class Application_Include_LinearConstraints<false, grad> 
{};


} // namespace colin

#endif // defined colin_Application__LinearConstraints_h

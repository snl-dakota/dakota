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
 * \file Application_NonD_Constraints.h
 *
 * Defines the colin::Application_NonD_Constraints class.
 */

#ifndef colin_Application_NonD_Constraints_h
#define colin_Application_NonD_Constraints_h

#include <acro_config.h>
#include <colin/application/Constraints.h>
#include <colin/application/Jacobian.h>


namespace colin
{

// forward declaration of the unit testing class
namespace unittest { class Application_NonD_Constraints; }


//============================================================================
//============================================================================
// Class Application_NonD_Constraints
//============================================================================
//============================================================================

/**
 *  Defines the elements of a Application that pertain to nondeterministic
 *  constraints.
 *
 *  Any concrete application implementing this class MUST provide
 *  responses to the ndcf_info response info type.  This interface will
 *  calculate ndeqcf_info and ndineqcf_info requests on the concrete
 *  application's behalf.
 */
class Application_NonD_Constraints
   : virtual public Application_Constraints
{
   friend class unittest::Application_NonD_Constraints;
protected:
   utilib::Privileged_Property _num_nondeterministic_constraints;
   utilib::Privileged_Property _nondeterministic_constraint_lower_bounds;
   utilib::Privileged_Property _nondeterministic_constraint_upper_bounds;
   utilib::Privileged_Property _nondeterministic_constraint_labels;

public: // methods

   /// Constructor
   Application_NonD_Constraints();

   /// Destructor
   virtual ~Application_NonD_Constraints();


   /* Information methods */

   /// Returns the number of nondeterministic equality constraints
   size_t numNondeterministicEqConstraints() const;

   /// Returns the number of nondeterministic inequality constraints
   size_t numNondeterministicIneqConstraints() const;


   /* Request & Evaluation declarations */

   /// Register a nondeterministic constraint evaluation computation
   DECLARE_REQUEST(NDCF);
   /// Register a nondeterministic constraint violation computation
   DECLARE_REQUEST(NDCFViol);
   /// Register a nondeterministic equality constraint evaluation computation
   DECLARE_REQUEST(NDEqCF);
   /// Register a nondeterministic inequality constraint evaluation computation
   DECLARE_REQUEST(NDIneqCF);


   /* Access methods */

   /// Get the bounds for a specific nondeterministic constraint 
   void nondeterministicConstraintBound( size_t index,
                                  utilib::AnyFixedRef lower,
                                  utilib::AnyFixedRef upper ) const;

   /// Get nondeterministic equality constraint bounds - dense format
   void nondeterministicEqConstraintBounds( utilib::AnyFixedRef rhs ) const;

   /// Get nondeterministic inequality constraint bounds - dense format
   void nondeterministicIneqConstraintBounds( utilib::AnyFixedRef lower,
                                       utilib::AnyFixedRef upper ) const;

public: // data

   /// Total number of linear constraints
   utilib::ReadOnly_Property num_nondeterministic_constraints;

   /// The lower bounds for all linear constraint residuals
   utilib::ReadOnly_Property nondeterministic_constraint_lower_bounds;

   /// The upper bounds for all linear constraint residuals
   utilib::ReadOnly_Property nondeterministic_constraint_upper_bounds;

   /// The labels for all linear constraint residuals
   utilib::ReadOnly_Property nondeterministic_constraint_labels;


protected: // methods

   /// Set a single label for the nondeterministic constraint
   void _setNondeterministicConstraintLabel( const size_t i, 
                                             const std::string &label );

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
// Class Application_NonD_ConstraintGradients
//============================================================================
//============================================================================

class Application_NonD_ConstraintGradients
   : virtual public Application_Jacobian,
     public Application_NonD_Constraints
{
public: // methods

   /// Constructor
   Application_NonD_ConstraintGradients();

   /// Destructor
   virtual ~Application_NonD_ConstraintGradients();


   /* Request & Evaluation declarations */

   /// Register a nondeterministic constraint evaluation computation
   DECLARE_REQUEST(NDCG);
   /// Register a nondeterministic equality constraint evaluation computation
   DECLARE_REQUEST(NDEqCG);
   /// Register a nondeterministic inequality constraint evaluation computation
   DECLARE_REQUEST(NDIneqCG);

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
class Application_Include_NonD_Constraints
   : public Application_NonD_ConstraintGradients
{};

template<>
class Application_Include_NonD_Constraints<true, false>
   : public Application_NonD_Constraints
{};

template<bool grad>
class Application_Include_NonD_Constraints<false, grad>
{};

} // namespace colin

#endif




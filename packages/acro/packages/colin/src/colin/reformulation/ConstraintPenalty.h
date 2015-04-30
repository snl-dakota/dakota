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
 * \file ConstraintPenaltyApplication.h
 *
 * Defines the colin::ConstraintPenaltyApplication class
 */

#ifndef colin_ConstraintPenaltyApplication_h
#define colin_ConstraintPenaltyApplication_h

#include <acro_config.h>

#include <colin/Application.h>
#include <colin/reformulation/Base.h>
#include <colin/Utilities.h>

#include <utilib/TinyXML_helper.h>
#include <utilib/SparseMatrix.h>

namespace colin
{

template <class ProblemT>
class ConstraintPenaltyApplication 
   : public Application<ProblemT>,
     public ReformulationApplication
{
public:
   
   ///
   ConstraintPenaltyApplication()
      : constraint_penalty(utilib::Property::Bind<double>()),
        convergence_factor(utilib::Property::Bind<double>()),
        apply_convergence_factor(utilib::Property::Bind<bool>())
   {  
      constructor();
   }

   ///
   ConstraintPenaltyApplication(ApplicationHandle src)
      : constraint_penalty(utilib::Property::Bind<double>()),
        convergence_factor(utilib::Property::Bind<double>()),
        apply_convergence_factor(utilib::Property::Bind<bool>())
   {  
      constructor();
      // set the base problem that we are wrapping
      reformulate_application(src);
   }

   ///
   virtual ~ConstraintPenaltyApplication()
   {}

   /// (constant) factor multiplied by the sum squared constraint violation
   utilib::Property constraint_penalty;
   /// (Dynamic) factor multiplied by the sum squared constraint violation
   utilib::Property convergence_factor;
   /// If false, the convergence_factor will not be used to calculate
   /// the penalty (default == true)
   utilib::Property apply_convergence_factor;

protected:

   virtual void validate_reformulated_application(ApplicationHandle handle)
   {
      if ( ( ( handle->problem_type() 
               & ~ ProblemTrait(linear_constraints) 
               & ~ ProblemTrait(nonlinear_constraints) )
             != this->problem_type() )
           || ( this->problem_type() == handle->problem_type() ) )
         EXCEPTION_MNGR(std::runtime_error, "ConstraintPenaltyApplication::"
                        "validate_reformulated_application(): invalid base "
                        "application type " << handle->problem_type_name() << 
                        " for ConstraintPenaltyApplication<" << 
                        this->problem_type_name() << ">");
   }

   virtual void configure_reformulated_application()
   {
      // reference all appropriate external info (including the
      // objective... the properties aren't changing, only the way we
      // calculate it.)
      this->reference_reformulated_application_properties();
   }

private:  // central constructor method (called by all constructors)

   void constructor()
   {
      typedef ConstraintPenaltyApplication this_t;

      constraint_penalty = 1e3;
      convergence_factor = 1.0;
      apply_convergence_factor = true;

      properties.declare( "constraint_penalty", constraint_penalty,
                          ObjectType::get(this), true );
      properties.declare( "convergence_factor", convergence_factor,
                          ObjectType::get(this), true );
      properties.declare( "apply_convergence_factor", apply_convergence_factor,
                          ObjectType::get(this), true );

      option.add("constraint_penalty", constraint_penalty, 
                 "Constraint Penalty reformulation constraint "
                 "violation multiplier");

      if (HasProblemTrait(ProblemT, multiple_objectives))
         EXCEPTION_MNGR(std::logic_error, "ConstraintPenaltyApplication<"
                        << this->problem_type_name() << ">(): "
                        "cannot be created as a multi-objective application.");

      if (HasProblemTrait(ProblemT, hessians))
         EXCEPTION_MNGR(std::logic_error, "ConstraintPenaltyApplication<"
                        << this->problem_type_name() << ">(): "
                        "cannot be created with Hessian information.");

      initializer("Penalty").connect
         ( boost::bind(&this_t::cb_initialize, this, _1) );

      request_transform_signal.connect
         ( boost::bind(&this_t::cb_map_request, this, _1, _2) );

      response_transform_signal.connect
         ( boost::bind(&this_t::cb_map_f_response, this, _1, _2, _3, _4) );
      response_transform_signal.connect
         ( boost::bind(&this_t::cb_map_g_response, this, _1, _2, _3, _4));
   }


private: // callback methods

   ///
   void cb_initialize(TiXmlElement* elt)
   {
      double tmp;
      utilib::get_num_attribute(elt, "value", tmp);
      constraint_penalty = tmp;
   }
   

   void 
   cb_map_request( const AppRequest::request_map_t &outer_requests,
                   AppRequest::request_map_t &requests )
   {
      if ( outer_requests.find(f_info) != outer_requests.end() ) 
         requests.insert(AppRequest::request_pair_t(cvf_info, utilib::Any()));
      if ( outer_requests.find(g_info) != outer_requests.end() )
      {
         requests.insert(AppRequest::request_pair_t(cvf_info, utilib::Any()));
         requests.insert(AppRequest::request_pair_t(cg_info, utilib::Any()));
      }
   }


   int
   cb_map_f_response( const utilib::Any &domain,
                      const AppRequest::request_map_t &requests,
                      const AppResponse::response_map_t &sub_response,
                      AppResponse::response_map_t &response )
   {
      static_cast<void>(domain);
      typedef AppResponse::response_map_t::const_iterator sub_iterator;

      // remove the f_info that is automatically propagated up from the
      // sub_response
      response.erase(f_info);

      //std::cerr << utilib::demangledName(typeid(this)) << "(f)" << std::endl;
      sub_iterator f_it = sub_response.find(f_info);
      if ( f_it == sub_response.end() )
         return requests.count(f_info) ? 0 : -1;

      sub_iterator c_it = sub_response.find(cvf_info);
      if ( c_it == sub_response.end() )
         return 0;

      utilib::Any cv;
      utilib::TypeManager()->lexical_cast( c_it->second, cv, 
                                           typeid(std::vector<real>) );
      const std::vector<real> &cviol_v = cv.expose<std::vector<real> >();
      
      real f_val = 0;
      utilib::TypeManager()->lexical_cast(f_it->second, f_val);

      double p = constraint_penalty.expose<double>();
      if ( apply_convergence_factor.expose<bool>() )
         p *= convergence_factor.expose<double>();

      // Because I share objective info with the wrapped application, I
      // can just check MY objective info...
      if ( this->_sense == minimization )
         f_val += p * l2_norm_sq(cviol_v);
      else
         f_val -= p * l2_norm_sq(cviol_v);

      response.insert(AppResponse::response_pair_t(f_info, f_val));
      return -1;
   }


   /**  \brief Calculate the gradient for the penalized objective.  
    *
    *   Define:
    *     f = function; 
    *    cv = constraint violation; 
    *     x = domain; 
    *     r = residual;
    *     p = penalty value;
    *
    *   Note:
    *     cv = r - upper_bound (if r > upper bound)
    *     cv = 0               (if lower_bound < r < upper bound)
    *     cv = r - lower_bound (if r < lower bound)
    *   Thus, dcv/dx == dr/dx iif cv != 0; otherwise dcv/dx == 0
    *
    *   As we are penalizing the objective with the violation squared:
    *     F = f + p*cv^2
    *     dF/dx = df/dx + p * 2 * cv * dcv/dx
    *           = df/dx + p * 2 * cv * dr/dx
    *           = g + p * 2 * cv * cg
    */
   int
   cb_map_g_response( const utilib::Any &domain,
                      const AppRequest::request_map_t &requests,
                      const AppResponse::response_map_t &sub_response,
                      AppResponse::response_map_t &response )
   { 
      static_cast<void>(domain);
      typedef AppResponse::response_map_t::const_iterator sub_iterator;

      // remove the g_info that is automatically propagated up from the
      // sub_response
      response.erase(g_info);

      //std::cerr << utilib::demangledName(typeid(this)) << "(g)" << std::endl;
      sub_iterator g_it = sub_response.find(g_info);
      if ( g_it == sub_response.end() )
         return requests.count(g_info) ? 0 : -1;

      sub_iterator c_it = sub_response.find(cvf_info);
      if ( c_it == sub_response.end() )
         return 0;

      sub_iterator cg_it = sub_response.find(cg_info);
      if ( cg_it == sub_response.end() )
         return 0;

      double p = constraint_penalty.expose<double>();
      if ( apply_convergence_factor.expose<bool>() )
         p *= convergence_factor.expose<double>();

      utilib::Any any_cv;
      utilib::TypeManager()->lexical_cast
         ( c_it->second, any_cv, typeid(std::vector<real>) );
      const std::vector<real>& cv = any_cv.expose<std::vector<real> >();

      utilib::Any any_cg;
      utilib::TypeManager()->lexical_cast
         ( cg_it->second, any_cg, typeid(utilib::RMSparseMatrix<double>) );
      const utilib::RMSparseMatrix<double> &cg 
         = any_cg.expose<utilib::RMSparseMatrix<double> >();
      
      utilib::Any ans;
      std::vector<real> &g_val = ans.set<std::vector<real> >();
      utilib::TypeManager()->lexical_cast(g_it->second, g_val);

      // If it's a maximization problem, temporarily convert to minimization
      if ( this->sense == maximization )
         for( size_t i = g_val.size(); i > 0; g_val[--i] *= -1.0 );

      // The gradient is: g + sum(p*2*cv*cg)
      size_t row = cg.get_nrows(); 
      while ( row > 0 )
      {
         --row;
         size_t i = cg.get_matbeg()[row];
         size_t n = cg.get_matcnt()[row];
         while ( n > 0 )
         {
            g_val[cg.get_matind()[i]] += p*2.0*cv[row]*cg.get_matval()[i];
            ++i;
            --n;
         }
         
      }

      // If it's a maximization problem, convert back to maximization
      if ( this->sense == maximization )
         for( size_t i = g_val.size(); i > 0; g_val[--i] *= -1.0 );

      // Return the result
      response.insert(AppResponse::response_pair_t(g_info, ans));
      return -1;
   }

};


} // namespace colin

#endif // defined colin_ConstraintPenaltyApplication_h

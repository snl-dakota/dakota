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
 * \file FiniteDifference.h
 *
 * Defines the colin::FiniteDifferenceApplication class
 */

#ifndef colin_FiniteDifference_h
#define colin_FiniteDifference_h

#include <acro_config.h>
#include <colin/Application.h>
#include <colin/AsynchronousApplication.h>
#include <utilib/TinyXML_helper.h>

namespace colin
{

class FiniteDifferenceApplication_Core 
   : virtual public Application_Base,
     public AsynchronousApplication
{
public:
   FiniteDifferenceApplication_Core();

   ~FiniteDifferenceApplication_Core();


   /// The finite difference step size for continuous variables
   utilib::Property step_size;

   /// The finite difference mode: forward, centered, backward
   utilib::Property difference_mode;

   enum difference_mode_t {
      backward = -1,
      central  =  0,
      forward  =  1
   };

   void reformulate_application(ApplicationHandle handle);

protected:

   ///
   struct EvalInfo {
      EvalInfo()
         : compute_fd(0), pending(0), requests(), evalIDs(), response_buf()
      {}

      static const int gradient = 1;
      static const int mo_gradient = 2;
      static const int l_jacobian = 4;
      static const int nl_jacobian = 8;
      static const int nd_jacobian = 16;

      int                         compute_fd;
      unsigned int                pending;
      std::list<response_info_t>  requests;
      std::list<EvaluationID>     evalIDs;
      std::map<EvaluationID, AppResponse>  response_buf;
   };
   ///
   typedef std::map<size_t, EvalInfo>  eval_set_t;
   ///
   typedef std::map<EvaluationID, eval_set_t::iterator> eval_lookup_t;


   virtual const EvaluationManager_Handle default_eval_mngr();

   ///
   virtual void queue_fd_steps( eval_set_t::iterator s_id, 
                                const utilib::Any &domain, 
                                utilib::seed_t seed ) = 0;

   virtual utilib::Any 
   spawn_evaluation_impl( const utilib::Any &domain,
                     const AppRequest::request_map_t &requests,
                     utilib::seed_t &seed );
   virtual utilib::Any 
   async_collect_evaluation( AppResponse::response_map_t &responses,
                             utilib::seed_t &seed );

   virtual bool 
   async_evaluation_available();

private:
   void calculate_scalar_gradient( response_info_t val_info,
                                   response_info_t grad_info,
                                   AppResponse::response_map_t &responses,
                                   EvalInfo &data,
                                   AppResponse &masked_response );

   void calculate_vector_gradient( size_t num,
                                   response_info_t val_info,
                                   response_info_t grad_info,
                                   AppResponse::response_map_t &responses,
                                   EvalInfo &data,
                                   AppResponse &masked_response );

   void cb_initialize(TiXmlElement* elt);

protected:
   ///
   EvaluationManager_Handle localEvalMngr;

   ///
   ApplicationHandle remote_app;

   ///
   size_t  last_set_id;

   ///
   eval_set_t  eval_sets;

   ///
   eval_lookup_t  pending_evals;   
};


template<bool>
class FiniteDifferenceApplication_Domain
   : public FiniteDifferenceApplication_Core
{
public:
   virtual bool
   map_domain(const utilib::Any &src, utilib::Any &native, bool forward) const;

   virtual void queue_fd_steps( typename eval_set_t::iterator s_id,
                                const utilib::Any &domain,
                                utilib::seed_t seed );
};


template <class ProblemT>
class FiniteDifferenceApplication
   : public Application <ProblemT>,
     public FiniteDifferenceApplication_Domain
       < HasProblemTrait(ProblemT, integers) >
{
public:
   ///
   FiniteDifferenceApplication()
   {
      constructor();
   }

   ///
   FiniteDifferenceApplication(ApplicationHandle src)
   {
      constructor();
      // set the base problem that we are wrapping
      this->reformulate_application(src);
   }

   ///
   virtual ~FiniteDifferenceApplication()
   {}


private:
   ///
   void constructor()
   {
      //if (HasProblemTrait(ProblemT, multiple_objectives))
      //{
      //   EXCEPTION_MNGR(std::logic_error, "FiniteDifferenceApplication - "
      //                  "Attempting to create finite difference for a "
      //                  "multiobjective application.");
      //}
      if ( ! HasProblemTrait(ProblemT, gradients) )
      {
         EXCEPTION_MNGR(std::logic_error, "FiniteDifferenceApplication - "
                        "Attempting to create finite difference for an "
                        "application without gratient information?!?");
      }
      if ( HasProblemTrait(ProblemT, hessians) )
      {
         EXCEPTION_MNGR(std::logic_error, "FiniteDifferenceApplication - "
                        "Does not support calculating Hessian information");
      }
   }
};


} // namespace colin

#endif // defined colin_FiniteDifference_h

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
 * \file SamplingApplication.h
 *
 * Defines the colin::SamplingApplication class
 */

#ifndef colin_SamplingApplication_h
#define colin_SamplingApplication_h

#include <acro_config.h>

#include <colin/Application.h>
#include <colin/reformulation/Base.h>
#include <colin/AsynchronousApplication.h>
#include <utilib/TinyXML_helper.h>

namespace colin
{

class ResponseFunctor;

class SamplingApplication_Core 
   : public BasicReformulationApplication,
     public AsynchronousApplication
{
public:
   SamplingApplication_Core();

   ~SamplingApplication_Core();


   /// The finite difference step size for continuous variables
   utilib::Property sample_size;

   /// The finite difference mode: forward, centered, backward
   utilib::Property sample_seed;

protected:

   ///
   struct EvalInfo {
      EvalInfo()
         : pending(0), evalIDs(), response_buf()
      {}

      typedef std::map<EvaluationID, AppResponse>  response_buf_t;

      unsigned int             pending;
      std::list<EvaluationID>  evalIDs;
      response_buf_t           response_buf;
   };
   ///
   typedef std::map<size_t, EvalInfo>  eval_set_t;
   ///
   typedef std::map<EvaluationID, eval_set_t::iterator> eval_lookup_t;
   ///
   typedef boost::function
   <void(response_info_t, EvalInfo&, AppResponse::response_map_t&)>
   response_fcn_t;
   ///
   typedef std::map<response_info_t, response_fcn_t>  sample_handler_map_t;


   virtual const EvaluationManager_Handle default_eval_mngr();

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

   void cb_update_seeds();

   void cb_configure();

   void cb_initialize(TiXmlElement* elt);

protected:
   /// Our local (serial) evaluation manager
   EvaluationManager_Handle localEvalMngr;

   ///
   size_t  last_set_id;

   ///
   eval_set_t  eval_sets;

   ///
   eval_lookup_t  pending_evals;   

   ///
   std::vector<utilib::seed_t>  seeds;

   ///
   sample_handler_map_t  sampled_info_handlers;
};


template<bool>
class SamplingApplication_SingleObjective
   : virtual public SamplingApplication_Core
{
public:
   SamplingApplication_SingleObjective();

   ~SamplingApplication_SingleObjective();

   /// This takes ownership of the functor pointer and will delete it when done
   void setObjectiveFunctor(ResponseFunctor* fcn);

private:
   ResponseFunctor *obj_functor;

   void cb_configure();

   void cb_update_nond(const utilib::ReadOnly_Property &prop);

   void cb_response( response_info_t info, EvalInfo& data, 
                     AppResponse::response_map_t& response );
};

template<>
class SamplingApplication_SingleObjective<false>
{};



template<bool>
class SamplingApplication_MultiObjective
   : virtual public SamplingApplication_Core
{
public:
   SamplingApplication_MultiObjective();

   ~SamplingApplication_MultiObjective();

   /// This takes ownership of the functor pointer and will delete it when done
   void setObjectiveFunctor(size_t objective, ResponseFunctor* fcn);

private:
   typedef std::map<size_t, ResponseFunctor*> obj_functor_t;
   obj_functor_t  obj_functor;

   void cb_configure();

   void cb_update_nond(const utilib::ReadOnly_Property &prop);

   void cb_response( response_info_t info, EvalInfo& data, 
                     AppResponse::response_map_t& response );
};

template<>
class SamplingApplication_MultiObjective<false>
{};


template<bool>
class SamplingApplication_Constraint
   : virtual public SamplingApplication_Core
{
public:
   SamplingApplication_Constraint();

   ~SamplingApplication_Constraint();

   /// This takes ownership of the functor pointer and will delete it when done
   void setConstraintFunctor(size_t constraint, ResponseFunctor* fcn);

private:
   typedef std::vector<ResponseFunctor*> con_functor_t;
   con_functor_t  con_functor;

   void cb_configure();

   void cb_update_nond(const utilib::ReadOnly_Property &prop);

   void cb_response( response_info_t info, EvalInfo& data, 
                     AppResponse::response_map_t& response );
};

template<>
class SamplingApplication_Constraint<false>
{};



template <class ProblemT>
class SamplingApplication
   : public Application <ProblemT>,
     public SamplingApplication_SingleObjective
        < !HasProblemTrait(ProblemT, multiple_objectives) >,
     public SamplingApplication_MultiObjective
        < HasProblemTrait(ProblemT, multiple_objectives) >,
     public SamplingApplication_Constraint
        < HasProblemTrait(ProblemT, nonlinear_constraints) >
{
public:
   SamplingApplication()
   {
      this->validate_reformulated_application.connect
         ( boost::bind(&SamplingApplication::cb_validate, this, _1) );
   }

private:
   void cb_validate(ApplicationHandle handle)
   {
      if ( handle->problem_type() != 
           ( this->problem_type() |
             ProblemTrait(nondeterministic_response) ) )
         EXCEPTION_MNGR(std::runtime_error, "SamplingApplication::"
                        "cb_validate(): invalid base "
                        "application type " << handle->problem_type_name() << 
                        " for SamplingApplication<" << 
                        this->problem_type_name() << ">");
   }
};

} // namespace colin

#endif // colin_SamplingApplication_h

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

#include <colin/reformulation/Sampling.h>
#include <colin/ApplicationMngr.h>
#include <colin/ResponseFunctors.h>

#include <utilib/PM_LCG.h>

using std::pair;
using std::make_pair;
using utilib::Any;
using utilib::Property;
using utilib::TypeManager;

namespace colin {

namespace StaticInitializers {

namespace {

template <class FROM, class TO>
int lexical_cast_sampling(const Any& from, Any& to)
{
   pair<ApplicationHandle,SamplingApplication<TO>*> tmp
      = ApplicationHandle::create<SamplingApplication<TO> >();
   tmp.second->reformulate_application
      ( from.template expose<Problem<FROM> >()->get_handle() );

   to.template set< Problem<TO> >().set_application(tmp.second);
   return 0;
}


bool RegisterSampling()
{
   // ------------------------------------------------------------------
   // Register creatable instances with the ApplicationMngr

   ApplicationMngr().declare_application_type
      <SamplingApplication<NLP0_problem> >("SamplingReformulation");

   ApplicationMngr().declare_application_type
      <SamplingApplication<UNLP0_problem> >("SamplingReformulation");

   ApplicationMngr().declare_application_type
      <SamplingApplication<MINLP0_problem> >("SamplingReformulation");

   ApplicationMngr().declare_application_type
      <SamplingApplication<UMINLP0_problem> >("SamplingReformulation");


   // ------------------------------------------------------------------
   // Register default problem transformations with the ProblemMngr

   ProblemMngr().register_lexical_cast
      (typeid(Problem<SNLP0_problem>), 
       typeid(Problem<NLP0_problem>), 
       &lexical_cast_sampling<SNLP0_problem, NLP0_problem>,
       1);

   ProblemMngr().register_lexical_cast
      (typeid(Problem<SUNLP0_problem>), 
       typeid(Problem<UNLP0_problem>), 
       &lexical_cast_sampling<SUNLP0_problem, UNLP0_problem>,
       1);

   ProblemMngr().register_lexical_cast
      (typeid(Problem<SMINLP0_problem>), 
       typeid(Problem<MINLP0_problem>), 
       &lexical_cast_sampling<SMINLP0_problem, MINLP0_problem>,
       1);

   ProblemMngr().register_lexical_cast
      (typeid(Problem<SUMINLP0_problem>), 
       typeid(Problem<UMINLP0_problem>), 
       &lexical_cast_sampling<SUMINLP0_problem, UMINLP0_problem>,
       1);

   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool sampling = RegisterSampling();

} // namespace colin::StaticInitializers



// ------------------------------------------------------------------
// ------------------------------------------------------------------


SamplingApplication_Core::SamplingApplication_Core()
   : sample_size  (Property::Bind<size_t>(25)),
     sample_seed  (Property::Bind<utilib::seed_t>(1)),
     localEvalMngr(),
     last_set_id  (0)
{
   typedef SamplingApplication_Core this_t;

   ObjectType type = ObjectType::get(this);
   properties.declare( "sample_size", sample_size, type, true );
   properties.declare( "sample_seed", sample_seed, type, true );

   sample_size.onChange().connect
      ( boost::bind(&this_t::cb_update_seeds, this) );
   sample_seed.onChange().connect
      ( boost::bind(&this_t::cb_update_seeds, this) );

   configure_reformulated_application.connect
      ( boost::bind(&this_t::cb_configure, this) );

   initializer("Samplings").connect
      (boost::bind(&this_t::cb_initialize, this, _1));

   this->set_rng(utilib::AnyRNG(new utilib::PM_LCG));

   // Update the sample seeds
   cb_update_seeds();
}


SamplingApplication_Core::~SamplingApplication_Core()
{}


const EvaluationManager_Handle
SamplingApplication_Core::default_eval_mngr()
{ 
   // We are *always* going to use a SerialEvaluator, because this
   // application does not actually *do* any evaluation.  We will
   // defer the actual evaluation to the wrapped application's
   // evaluator.  We don't want to actually USE the wrapped evaluator
   // because *this* evaluation would tie up an evaluator
   // pseudo-process and potentially deadlock parallel systems.
   if ( localEvalMngr.empty() )
      localEvalMngr = EvalManagerFactory().create("Serial");
   return localEvalMngr; 
}


/// Actually spawn the calculation(s) requested by the solver
utilib::Any
SamplingApplication_Core::
spawn_evaluation_impl( const utilib::Any &domain,
                       const AppRequest::request_map_t &requests,
                       utilib::seed_t & )
{
   Application_Base* remote = remote_app.object();
   //
   // The info we need to remember to process the results
   //
   eval_set_t::iterator set_id 
      = eval_sets.insert( eval_sets.end(), 
                          make_pair(++last_set_id, EvalInfo()) );
   EvalInfo &data = set_id->second;

   //
   // Prepare the incoming request
   //
   //AppRequest request = remote->set_domain(StochasticDomain(seeds[0], domain));
   AppRequest request = remote->set_domain(domain, false, seeds[0]);

   Any none;
   std::set<response_info_t> samples;
   AppRequest::request_map_t::const_iterator it = requests.begin();
   AppRequest::request_map_t::const_iterator itEnd = requests.end();
   for ( ; it != itEnd; ++it )
   {
      if ( sampled_info_handlers.count(it->first) )
         samples.insert(it->first);

      this->record_remote_compute_task( remote, it->first, request, none );
      //data.requests.push_back(it->first);
   }

   // Queue the main request
   {
      EvaluationID id = remote->eval_mngr().queue_evaluation(request);
      pending_evals.insert(pending_evals.end(), make_pair(id, set_id));
      data.evalIDs.push_back(id);
      ++data.pending;
   }

   //
   // Prepare the samples
   //
   if ( ! samples.empty() )
   {
      std::set<response_info_t>::iterator r = samples.begin();
      std::set<response_info_t>::iterator rEnd = samples.end();
      for(size_t i=1; sample_size > i; ++i)
      {
         //AppRequest sample = 
         //    remote->set_domain(StochasticDomain(seeds[i], domain));
         AppRequest sample = remote->set_domain(domain, false, seeds[i]);
         
         for( r = samples.begin(); r != rEnd; ++r )
            this->record_remote_compute_task(remote, *r, sample, none);
         
         EvaluationID id = remote->eval_mngr().queue_evaluation(sample);
         pending_evals.insert(pending_evals.end(), make_pair(id,set_id));
         data.evalIDs.push_back(id);
         ++data.pending;
      }
   }

   return set_id->first;
}


utilib::Any 
SamplingApplication_Core::
async_collect_evaluation( AppResponse::response_map_t &responses,
                          utilib::seed_t &seed )
{
   // We won't be touching seed here...
   static_cast<void>(seed);

   //
   // Spin-lock until all the evaluations we need have returned
   //
   eval_lookup_t::iterator it;
   while ( true )
   {
      std::pair<EvaluationID, AppResponse> eval
         = remote_app->eval_mngr().next_response();

      it = pending_evals.find(eval.first);
      if ( it == pending_evals.end() )
         EXCEPTION_MNGR(std::runtime_error, "SamplingApplication::"
                        "collect_evaluation(): received unexpected "
                        "EvaluationID");

      it->second->second.response_buf.insert(eval);

      if ( --(it->second->second.pending) == 0 )
         break;

      pending_evals.erase(it);
      assert( pending_evals.empty() == false );
   }
   EvalInfo &data = it->second->second;

   //
   // Process the evaluation set that we are going to return
   //
   // 1) transfer over all requested data
   responses = data.response_buf[data.evalIDs.front()].get();

   // 2) calculate all the sampled data
   sample_handler_map_t::iterator s_it = sampled_info_handlers.begin();
   sample_handler_map_t::iterator s_itEnd = sampled_info_handlers.end();
   for ( ; s_it != s_itEnd; ++s_it )
      if ( responses.count(s_it->first) )
         (s_it->second)(s_it->first, data, responses);

   size_t ans = it->second->first;
   eval_sets.erase(it->second);
   pending_evals.erase(it);
   return ans;
}


///
bool
SamplingApplication_Core::async_evaluation_available()
{
   return remote_app->eval_mngr().response_available();
}



void
SamplingApplication_Core::cb_update_seeds()
{
   rng.set_seed(sample_seed);
   size_t n = sample_size;
   seeds.resize(n);
   for ( size_t i=0; i < n; seeds[i++] = rng.asLong() );
}


void SamplingApplication_Core::cb_configure()
{
   // reference all appropriate external info EXCEPT the nonlinear
   // constraints -- we may need to augment them to include the
   // nondeterministic constraints
   std::set<ObjectType> exclude;
   exclude.insert(ObjectType::get<Application_NonlinearConstraints>());
   this->reference_reformulated_application_properties(exclude);
}


///
void 
SamplingApplication_Core::cb_initialize(TiXmlElement* elt)
{
   static_cast<void>(elt);
}


// ------------------------------------------------------------------
// ------------------------------------------------------------------

//
// NB: Because we need to specialize the member functions before they
// are used, the members are listed in the reverse order as they appear
// in the header file.
//

template<>
void SamplingApplication_SingleObjective<true>::
cb_response( response_info_t info, EvalInfo& data, 
             AppResponse::response_map_t& response )
{
   EvalInfo::response_buf_t::iterator it = data.response_buf.begin();
   EvalInfo::response_buf_t::iterator itEnd = data.response_buf.end();
   Any state = obj_functor->initialize(data.response_buf.size());
   for( ; it != itEnd; ++it)
      state = (*obj_functor)(state, it->second.get(info));
   response.erase(info);
   response.insert(make_pair(info, obj_functor->result(state)));
}

template<>
void SamplingApplication_SingleObjective<true>::
cb_update_nond(const utilib::ReadOnly_Property& prop)
{
   typedef SamplingApplication_SingleObjective<true> this_t;

   utilib::BitArray nond = prop;
   if ( nond[0] )
   {
      if ( ! obj_functor )
      {
         obj_functor = new MeanDoubleFunctor;
         sampled_info_handlers[f_info] = 
            boost::bind(&this_t::cb_response, this, _1, _2, _3);
      }
   }
   else
   {
      sampled_info_handlers.erase(f_info);
      delete obj_functor;
      obj_functor = NULL;
   }
}

template<>
void SamplingApplication_SingleObjective<true>::cb_configure()
{
   typedef SamplingApplication_SingleObjective<true> this_t;

   remote_app->property("nond_objective").onChange().connect
      ( boost::bind(&this_t::cb_update_nond, this, _1) );

   this_t::cb_update_nond(remote_app->property("nond_objective"));
}

template<>
void SamplingApplication_SingleObjective<true>::
setObjectiveFunctor(ResponseFunctor* fcn)
{
   if ( ! fcn )
      EXCEPTION_MNGR(std::runtime_error, "SamplingApplication_SingleObjective"
                     "::setObjectiveFunctor(): cannot set NULL functor");
   if ( ! obj_functor )
      EXCEPTION_MNGR(std::runtime_error, "SamplingApplication_SingleObjective"
                     "::setObjectiveFunctor(): cannot set functor for "
                     "a deterministic objective");
   delete obj_functor;
   obj_functor = fcn;
}


template<>
SamplingApplication_SingleObjective<true>::
SamplingApplication_SingleObjective()
   : obj_functor(NULL)
{
   typedef SamplingApplication_SingleObjective<true> this_t;

   configure_reformulated_application.connect
      ( boost::bind(&this_t::cb_configure, this) );
}

template<>
SamplingApplication_SingleObjective<true>::
~SamplingApplication_SingleObjective()
{
   delete obj_functor;
}


// ------------------------------------------------------------------
// ------------------------------------------------------------------

//
// NB: Because we need to specialize the member functions before they
// are used, the members are listed in the reverse order as they appear
// in the header file.
//

template<>
void SamplingApplication_MultiObjective<true>::
cb_response( response_info_t info, EvalInfo& data, 
             AppResponse::response_map_t& response )
{
   Any ans;
   std::vector<real>& objectives = ans.set<std::vector<real> >();
   TypeManager()->lexical_cast(response[info], objectives);

   std::map<size_t, Any> state;
   obj_functor_t::iterator o_it = obj_functor.begin();
   obj_functor_t::iterator o_itEnd = obj_functor.end();
   size_t n = data.response_buf.size();
   for( ; o_it != o_itEnd; ++o_it)
      state.insert( state.end(), 
                    make_pair(o_it->first, o_it->second->initialize(n)) );

   std::map<size_t, Any>::iterator s_it = state.begin();
   std::map<size_t, Any>::iterator s_itEnd = state.end();

   EvalInfo::response_buf_t::iterator it = data.response_buf.begin();
   EvalInfo::response_buf_t::iterator itEnd = data.response_buf.end();
   for( ; it != itEnd; ++it)
   {
      std::vector<real> tmp;
      TypeManager()->lexical_cast(it->second.get(info), tmp);
      for( o_it = obj_functor.begin(), s_it = state.begin(); 
           o_it != o_itEnd; 
           ++o_it, ++s_it )
         s_it->second = (*o_it->second)(s_it->second, tmp[o_it->first]);
   }

   for( s_it = state.begin(); s_it != s_itEnd; ++s_it )
      TypeManager()->lexical_cast( o_it->second->result(s_it->second), 
                                   objectives[s_it->first] );

   response.erase(info);
   response.insert(make_pair(info, ans));
}

template<>
void SamplingApplication_MultiObjective<true>::
cb_update_nond(const utilib::ReadOnly_Property& prop)
{
   typedef SamplingApplication_MultiObjective<true> this_t;

   obj_functor_t::iterator it = obj_functor.begin();
   obj_functor_t::iterator itEnd = obj_functor.end();

   utilib::BitArray nond = prop;
   for( size_t i = 0; i < nond.size(); ++i )
   {
      while ( it != itEnd && it->first < i )
      {
         delete it->second;
         obj_functor.erase(it++);
      }
      if ( it == itEnd || it->first != i )
         obj_functor.insert(it, make_pair(i, new MeanDoubleFunctor));
      else
         ++it;
   }

   if ( obj_functor.empty() )
      sampled_info_handlers.erase(mf_info);
   else
      sampled_info_handlers[mf_info] = 
         boost::bind(&this_t::cb_response, this, _1, _2, _3);
}

template<>
void SamplingApplication_MultiObjective<true>::cb_configure()
{
   typedef SamplingApplication_MultiObjective<true> this_t;

   remote_app->property("nond_objective").onChange().connect
      ( boost::bind(&this_t::cb_update_nond, this, _1) );

   this_t::cb_update_nond(remote_app->property("nond_objective"));
}

template<>
void SamplingApplication_MultiObjective<true>::
setObjectiveFunctor(size_t i, ResponseFunctor* fcn)
{
   if ( ! fcn )
      EXCEPTION_MNGR(std::runtime_error, "SamplingApplication_MultiObjective"
                     "::setObjectiveFunctor(): cannot set NULL functor");

   obj_functor_t::iterator it = obj_functor.find(i);
   if ( it == obj_functor.end() )
      EXCEPTION_MNGR(std::runtime_error, "SamplingApplication_MultiObjective"
                     "::setObjectiveFunctor(): cannot set functor for "
                     "a deterministic or invalid objective (" << i << ")");
   delete it->second;
   it->second = fcn;
}


template<>
SamplingApplication_MultiObjective<true>::
SamplingApplication_MultiObjective()
   : obj_functor()
{
   typedef SamplingApplication_MultiObjective<true> this_t;

   configure_reformulated_application.connect
      ( boost::bind(&this_t::cb_configure, this) );
}

template<>
SamplingApplication_MultiObjective<true>::
~SamplingApplication_MultiObjective()
{
   while( ! obj_functor.empty() )
   {
      delete obj_functor.begin()->second;
      obj_functor.erase(obj_functor.begin());
   }
}


// ------------------------------------------------------------------
// ------------------------------------------------------------------

//
// NB: Because we need to specialize the member functions before they
// are used, the members are listed in the reverse order as they appear
// in the header file.
//

template<>
void SamplingApplication_Constraint<true>::
cb_response( response_info_t info, EvalInfo& data, 
             AppResponse::response_map_t& response )
{
   std::vector<Any> state;
   size_t n = data.response_buf.size();
   for(size_t i = 0; i < con_functor.size(); ++i)
      state[i] = con_functor[i]->initialize(n);

   EvalInfo::response_buf_t::iterator it = data.response_buf.begin();
   EvalInfo::response_buf_t::iterator itEnd = data.response_buf.end();
   for( ; it != itEnd; ++it)
   {
      std::vector<real> tmp;
      TypeManager()->lexical_cast(it->second.get(info), tmp);
      for(size_t i = 0; i < con_functor.size(); ++i)
         state[i] = (*con_functor[i])(state[i], tmp[i]);
   }

   Any ans;
   std::vector<real>& nonlinear = ans.set<std::vector<real> >();
   TypeManager()->lexical_cast(response[nlcf_info], nonlinear);
   size_t nl = nonlinear.size();
   nonlinear.resize(nl+con_functor.size());

   for(size_t i = 0; i < con_functor.size(); ++i)
      TypeManager()->lexical_cast( con_functor[i]->result(state[i]),
                                   nonlinear[i+nl] );

   response.erase(info);
   response.erase(nlcf_info);
   response.insert(make_pair(nlcf_info, ans));
}

template<>
void SamplingApplication_Constraint<true>::
cb_update_nond(const utilib::ReadOnly_Property& prop)
{
   typedef SamplingApplication_Constraint<true> this_t;

   size_t n = prop;
   size_t old = con_functor.size();

   for( size_t i = old; i > n; delete con_functor[--i] );

   con_functor.resize(n, NULL);

   for( size_t i = old; i < n; con_functor[i++] = new MeanDoubleFunctor );

   if ( con_functor.empty() )
      sampled_info_handlers.erase(ndcf_info);
   else
      sampled_info_handlers[ndcf_info] = 
         boost::bind(&this_t::cb_response, this, _1, _2, _3);
}

template<>
void SamplingApplication_Constraint<true>::cb_configure()
{
   typedef SamplingApplication_Constraint<true> this_t;

   remote_app->property("num_nondeterministic_constraints").onChange().connect
      ( boost::bind(&this_t::cb_update_nond, this, _1) );

   this_t::cb_update_nond
      ( remote_app->property("num_nondeterministic_constraints") );
}

template<>
void SamplingApplication_Constraint<true>::
setConstraintFunctor(size_t i, ResponseFunctor* fcn)
{
   if ( ! fcn )
      EXCEPTION_MNGR(std::runtime_error, "SamplingApplication_Constraint"
                     "::setConstraintFunctor(): cannot set NULL functor");

   if ( i >= con_functor.size() )
      EXCEPTION_MNGR(std::runtime_error, "SamplingApplication_Constraint"
                     "::setConstraintFunctor(): cannot set functor for "
                     "an invalid constraint (" << i << ")");
   delete con_functor[i];
   con_functor[i] = fcn;
}


template<>
SamplingApplication_Constraint<true>::
SamplingApplication_Constraint()
   : con_functor()
{
   typedef SamplingApplication_Constraint<true> this_t;

   configure_reformulated_application.connect
      ( boost::bind(&this_t::cb_configure, this) );
}

template<>
SamplingApplication_Constraint<true>::
~SamplingApplication_Constraint()
{
   for(size_t i = con_functor.size(); i > 0; delete con_functor[--i]);
}




} // namespace colin

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

#include <colin/reformulation/FiniteDifference.h>
#include <colin/ApplicationMngr.h>

using std::cerr;
using std::endl;
using std::pair;
using utilib::Any;
using utilib::Property;

namespace colin {

namespace StaticInitializers {

namespace {


template <class FROM, class TO>
int lexical_cast_ContinuousFiniteDiff(const Any& from, Any& to)
{
   typedef FiniteDifferenceApplication<TO> fd_t;
   pair<ApplicationHandle, fd_t*> tmp = ApplicationHandle::create<fd_t>();
   tmp.second->reformulate_application
      ( from.template expose<Problem<FROM> >()->get_handle() );

   to.template set< Problem<TO> >().set_application(tmp.second);
   return 0;
}

template <class FROM, class TO>
int lexical_cast_MixedFiniteDiff(const Any& from, Any& to)
{
   typedef FiniteDifferenceApplication<TO> fd_t;
   pair<ApplicationHandle, fd_t*> tmp = ApplicationHandle::create<fd_t>();
   tmp.second->reformulate_application
      ( from.template expose<Problem<FROM> >()->get_handle() );

   to.template set< Problem<TO> >().set_application(tmp.second);
   return 0;
}


bool RegisterFiniteDiff()
{
   // ------------------------------------------------------------------
   // Register creatable instances with the ApplicationMngr

   ApplicationMngr().declare_application_type
      <FiniteDifferenceApplication<NLP1_problem> >
      ("FiniteDiffReformulation");

   ApplicationMngr().declare_application_type
      <FiniteDifferenceApplication<UNLP1_problem> >
      ("FiniteDiffReformulation");

   ApplicationMngr().declare_application_type
      <FiniteDifferenceApplication<MINLP1_problem> >
      ("FiniteDiffReformulation");

   ApplicationMngr().declare_application_type
      <FiniteDifferenceApplication<UMINLP1_problem> >
      ("FiniteDiffReformulation");


   // ------------------------------------------------------------------
   // Register default problem transformations with the ProblemMngr

   ProblemMngr().register_lexical_cast
      (typeid(Problem<NLP0_problem>), 
       typeid(Problem<NLP1_problem>), 
       &lexical_cast_ContinuousFiniteDiff<NLP0_problem, NLP1_problem>,
       1);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<UNLP0_problem>), 
       typeid(Problem<UNLP1_problem>), 
       &lexical_cast_ContinuousFiniteDiff<UNLP0_problem, UNLP1_problem>,
       1);

   ProblemMngr().register_lexical_cast
      (typeid(Problem<MINLP0_problem>), 
       typeid(Problem<MINLP1_problem>), 
       &lexical_cast_MixedFiniteDiff<MINLP0_problem, MINLP1_problem>,
       1);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<UMINLP0_problem>), 
       typeid(Problem<UMINLP1_problem>), 
       &lexical_cast_MixedFiniteDiff<UMINLP0_problem, UMINLP1_problem>,
       1);

   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool finite_difference = RegisterFiniteDiff();

} // namespace colin::StaticInitializers


// ------------------------------------------------------------------
// ------------------------------------------------------------------


FiniteDifferenceApplication_Core::FiniteDifferenceApplication_Core()
   : step_size      (Property::Bind<double>(1e-6)),
     difference_mode(Property::Bind<difference_mode_t>(forward)),
     localEvalMngr  (),
     last_set_id    (0)
{
   ObjectType type = ObjectType::get(this);
   properties.declare( "difference_mode", difference_mode, type, true );
   properties.declare( "step_size", step_size, type, true );

   initializer("FiniteDifferences").connect
      (boost::bind(&FiniteDifferenceApplication_Core::cb_initialize,this,_1));
}


FiniteDifferenceApplication_Core::~FiniteDifferenceApplication_Core()
{}


void
FiniteDifferenceApplication_Core::
reformulate_application(ApplicationHandle handle)
{
   if ( handle->problem_type() != 
        (problem_type() & ~ ProblemTrait(gradients)) )
      EXCEPTION_MNGR(std::runtime_error, "FiniteDifferenceApplication_Core::"
                     "reformulate_application(): invalid base "
                     "application type " << handle->problem_type_name() << 
                     " for FiniteDifferenceApplication<" << 
                     this->problem_type_name() << ">");

   remote_app = handle;

   // clear out any remote refereces from a previous reformulation
   properties.dereference_all();
      
   // reference all appropriate external info EXCEPT the gradients
   std::set<utilib::Any> exclude;
   exclude.insert(ObjectType::get<Application_Gradient>());
   EXCEPTION_TEST(remote_app.empty(), std::runtime_error, 
                  "Cannot reference a NULL application handle");
      
   referencePropertiesFrom( remote_app.object(), exclude, 
                            std::set<std::string>() );
}


const EvaluationManager_Handle
FiniteDifferenceApplication_Core::default_eval_mngr()
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
FiniteDifferenceApplication_Core::
spawn_evaluation_impl( const utilib::Any &domain,
                       const AppRequest::request_map_t &requests,
                       utilib::seed_t &seed )
{
   Application_Base* remote = remote_app.object();
   //
   // The info we need to remember to process the results
   //
   eval_set_t::iterator set_id 
      = eval_sets.insert( eval_sets.end(), 
                          std::make_pair(++last_set_id, EvalInfo()) );
   EvalInfo &data = set_id->second;

   //
   // Prepare the incoming request
   //
   AppRequest request = remote->set_domain( domain, false, seed );

   utilib::Any noAny;
   AppRequest::request_map_t::const_iterator it = requests.begin();
   AppRequest::request_map_t::const_iterator itEnd = requests.end();
   for ( ; it != itEnd; ++it )
   {
      // filter out g_info requests
      if ( it->first == g_info )
      {
         if ( properties["num_objectives"] > 1 )
            data.compute_fd += EvalInfo::mo_gradient;
         else
            data.compute_fd += EvalInfo::gradient;
      }
      // filter out various cg_info components
      else if ( it->first == lcg_info )
         data.compute_fd += EvalInfo::l_jacobian;
      else if ( it->first == nlcg_info )
         data.compute_fd += EvalInfo::nl_jacobian;
      else if ( it->first == ndcg_info )
         data.compute_fd += EvalInfo::nd_jacobian;
      else
      {
         this->record_remote_compute_task( remote, it->first, 
                                           request, noAny );
         data.requests.push_back(it->first);
      }
   }

   if ( data.compute_fd && difference_mode != central )
   {
      // If we are doing forward or backward diff and the original point
      // was not requested...
      if ( ( data.compute_fd & EvalInfo::gradient ) && 
           requests.find(f_info) == requests.end() )
         this->record_remote_compute_task( remote, f_info,
                                           request, noAny );
      if ( ( data.compute_fd & EvalInfo::mo_gradient ) && 
           requests.find(mf_info) == requests.end() )
         this->record_remote_compute_task( remote, mf_info,
                                           request, noAny );
      if ( ( data.compute_fd & EvalInfo::l_jacobian ) && 
           requests.find(lcf_info) == requests.end() )
         this->record_remote_compute_task( remote, lcf_info,
                                           request, noAny );
      if ( ( data.compute_fd & EvalInfo::nl_jacobian ) && 
           requests.find(nlcf_info) == requests.end() )
         this->record_remote_compute_task( remote, nlcf_info,
                                           request, noAny );
      if ( ( data.compute_fd & EvalInfo::nd_jacobian ) && 
           requests.find(ndcf_info) == requests.end() )
         this->record_remote_compute_task( remote, ndcf_info,
                                           request, noAny );
   }

   // Queue the main request
   if ( request.size() > 0 )
   {
      EvaluationID id = remote->eval_mngr().queue_evaluation(request);
      pending_evals.insert(pending_evals.end(), std::make_pair(id, set_id));
      data.evalIDs.push_back(id);
      ++data.pending;
   }

   //
   // Prepare the FD steps
   //
   if ( data.compute_fd )
      queue_fd_steps(set_id, domain, seed);

   return set_id->first;
}


utilib::Any 
FiniteDifferenceApplication_Core::
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
         EXCEPTION_MNGR(std::runtime_error, "FiniteDifferenceApplication::"
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
   AppResponse &masked_response = data.response_buf[data.evalIDs.front()];
   bool otherRequests = ! data.requests.empty();
   while ( ! data.requests.empty() )
   {
      response_info_t info = data.requests.front();
      data.requests.pop_front();

      responses.insert( responses.end(), AppResponse::response_pair_t
                        ( info, masked_response.get(info) ) );
   }

   if ( otherRequests || difference_mode != central )
      data.evalIDs.pop_front();

   // 2) calculate the constraint gradients
   if ( data.compute_fd & EvalInfo::l_jacobian )
      calculate_vector_gradient
         ( properties["num_linear_constraints"],
           lcf_info, lcg_info, responses, data, masked_response );
   if ( data.compute_fd & EvalInfo::nl_jacobian )
      calculate_vector_gradient
         ( properties["num_nonlinear_constraints"],
           nlcf_info, nlcg_info, responses, data, masked_response );
   if ( data.compute_fd & EvalInfo::nd_jacobian )
      calculate_vector_gradient
         ( properties["num_nondeterministic_constraints"],
           ndcf_info, ndcg_info, responses, data, masked_response );

   if ( data.compute_fd & EvalInfo::gradient )
      calculate_scalar_gradient
         ( f_info, g_info, responses, data, masked_response );
   if ( data.compute_fd & EvalInfo::mo_gradient )
      calculate_vector_gradient
         ( properties["num_objectives"],
           mf_info, g_info, responses, data, masked_response );

   size_t ans = it->second->first;
   eval_sets.erase(it->second);
   pending_evals.erase(it);
   return ans;
}


///
bool
FiniteDifferenceApplication_Core::async_evaluation_available()
{
   return remote_app->eval_mngr().response_available();
}


void
FiniteDifferenceApplication_Core::
calculate_scalar_gradient( response_info_t val_info,
                           response_info_t grad_info,
                           AppResponse::response_map_t &responses,
                           EvalInfo &data,
                           AppResponse &masked_response )
{
   utilib::Any ans;
   std::vector<real> &g = ans.set<std::vector<real> >();
         
   std::list<EvaluationID>::iterator evalID = data.evalIDs.begin();
   if ( difference_mode == central )
   {
      // central finite diff
      g.reserve(data.evalIDs.size()/2);
      real forward;
      real backward;
      while ( evalID != data.evalIDs.end() )
      {  
         data.response_buf[*evalID].get(val_info, forward);
         ++evalID;
         assert(evalID != data.evalIDs.end() && "Missing response");
         data.response_buf[*evalID].get(val_info, backward);
         ++evalID;

         g.push_back( (forward - backward) / step_size );
      }
   }
   else
   {
      // forward/backward finite diff
      g.reserve(data.evalIDs.size());
      real base;
      real forward;
      masked_response.get(val_info, base);
      while ( evalID != data.evalIDs.end() )
      {
         data.response_buf[*evalID].get(val_info, forward);
         ++evalID;

         g.push_back( (forward - base) / step_size );
      }
      if ( difference_mode == backward )
         for(size_t i=g.size(); i > 0; g[--i] *= -1.0 );
   }

   responses.insert(AppResponse::response_pair_t(grad_info, ans));
}


void
FiniteDifferenceApplication_Core::
calculate_vector_gradient( size_t num,
                           response_info_t val_info,
                           response_info_t grad_info,
                           AppResponse::response_map_t &responses,
                           EvalInfo &data,
                           AppResponse &masked_response )
{
   utilib::Any ans;
   std::vector<std::vector<real> > &g 
      = ans.set<std::vector<std::vector<real> > >();
   g.resize(num);

   std::list<EvaluationID>::iterator evalID = data.evalIDs.begin();
   if ( difference_mode == central )
   {
      // central finite diff
      size_t n = data.evalIDs.size()/2;
      for( size_t i = g.size(); i > 0; g[--i].reserve(n) );

      std::vector<real> forward;
      std::vector<real> backward;
      while ( evalID != data.evalIDs.end() )
      {  
         data.response_buf[*evalID].get(val_info, forward);
         ++evalID;
         assert(evalID != data.evalIDs.end() && "Missing response");
         data.response_buf[*evalID].get(val_info, backward);
         ++evalID;

         assert(forward.size() == backward.size() && 
                forward.size() == g.size());
         for(size_t i=0; i<forward.size(); ++i)
            g[i].push_back( (forward[i] - backward[i]) / step_size );
      }
   }
   else
   {
      // forward/backward finite diff
      size_t n = data.evalIDs.size();
      for( size_t i = g.size(); i > 0; g[--i].reserve(n) );

      std::vector<real> forward;
      std::vector<real> base;
      masked_response.get(val_info, base);
      while ( evalID != data.evalIDs.end() )
      {  
         data.response_buf[*evalID].get(val_info, forward);
         ++evalID;

         assert(forward.size() == base.size() && 
                forward.size() == g.size());
         for(size_t i=0; i<forward.size(); ++i)
            g[i].push_back( (forward[i] - base[i]) / step_size );
      }
      if ( difference_mode == backward )
         for(size_t j=0; j<n; ++j)
            for(size_t i=g.size(); i > 0; g[--i][j] *= -1.0 );
   }

   responses.insert(AppResponse::response_pair_t(grad_info, ans));
}


///
void 
FiniteDifferenceApplication_Core::cb_initialize(TiXmlElement* elt)
{
   std::string method = "";
   utilib::get_string_attribute(elt, "method", method, "forward");
   if ( method.compare("forward") == 0 )
      difference_mode = forward;
   else if ( method.compare("central") == 0 )
      difference_mode = central;
   else if ( method.compare("backward") == 0 )
      difference_mode = backward;
   else
      EXCEPTION_MNGR(std::runtime_error, 
                     "FiniteDifferenceApplication_Core::cb_initialize(): "
                     "unknown differencing method, \"" << method << "\"");

   double size = -1.0;
   utilib::get_num_attribute(elt, "step", size, 0);
   if ( size > 0 )
      step_size = size;
}


//========================================================================
//========================================================================


template <>
bool
FiniteDifferenceApplication_Domain<false>::
map_domain(const utilib::Any &src, utilib::Any &native, bool forward) const
{
   static_cast<void>(forward);
   // Because we are not going to EDIT the domain (permanently), we
   // can cast directly from one Any to the other.
   return utilib::TypeManager()->lexical_cast
      ( src, native, typeid(std::vector<real>) ) == 0;
}


template <>
void
FiniteDifferenceApplication_Domain<false>::
queue_fd_steps( eval_set_t::iterator s_id,
                const utilib::Any &domain, 
                utilib::seed_t seed )
{
   const std::vector<real> &d = domain.expose<std::vector<real> >();
   FiniteDifferenceApplication_Core::EvalInfo &data = s_id->second;
      
   std::vector<real> tmp_d = d;
   utilib::Any noAny;

   Application_Base* remote = this->remote_app.object();
   double step = this->step_size.expose<double>();
   bool central_diff = this->difference_mode == this->central;
   if ( central_diff ) 
      step *= 0.5;
   else if ( this->difference_mode == this->backward )
      step *= -1.0;

   bool compute_g = data.compute_fd
      & FiniteDifferenceApplication_Core::EvalInfo::gradient;
   bool compute_mo_g = data.compute_fd
      & FiniteDifferenceApplication_Core::EvalInfo::mo_gradient;
   bool compute_lcg = data.compute_fd
      & FiniteDifferenceApplication_Core::EvalInfo::l_jacobian;
   bool compute_nlcg = data.compute_fd
      & FiniteDifferenceApplication_Core::EvalInfo::nl_jacobian;
   bool compute_ndcg = data.compute_fd
      & FiniteDifferenceApplication_Core::EvalInfo::nd_jacobian;

   EvaluationID id;
   for( size_t i = 0; i < d.size(); ++i )
   {
      real &index = tmp_d[i];
      real orig = index;
      index += step;
      AppRequest tmp_r1 = remote->set_domain(tmp_d, true, seed);
      if ( compute_g )
         this->record_remote_compute_task(remote, f_info, tmp_r1, noAny);
      if ( compute_mo_g )
         this->record_remote_compute_task(remote, mf_info, tmp_r1, noAny);
      if ( compute_lcg )
         this->record_remote_compute_task(remote, lcf_info, tmp_r1, noAny);
      if ( compute_nlcg )
         this->record_remote_compute_task(remote, nlcf_info, tmp_r1, noAny);
      if ( compute_ndcg )
         this->record_remote_compute_task(remote, ndcf_info, tmp_r1, noAny);
      id = remote->eval_mngr().queue_evaluation(tmp_r1);
      this->pending_evals.insert( this->pending_evals.end(), 
                                  std::make_pair(id, s_id) );
      data.evalIDs.push_back(id);
      ++data.pending;

      index = orig;

      if ( ! central_diff )
         continue;

      //
      // For central differences...
      //

      index -= step;

      AppRequest tmp_r2 = remote->set_domain(tmp_d, false, seed); 
      if ( compute_g )
         this->record_remote_compute_task(remote, f_info, tmp_r2, noAny);
      if ( compute_mo_g )
         this->record_remote_compute_task(remote, mf_info, tmp_r2, noAny);
      if ( compute_lcg )
         this->record_remote_compute_task(remote, lcf_info, tmp_r2, noAny);
      if ( compute_nlcg )
         this->record_remote_compute_task(remote, nlcf_info, tmp_r2, noAny);
      if ( compute_ndcg )
         this->record_remote_compute_task(remote, ndcf_info, tmp_r2, noAny);
      id = remote->eval_mngr().queue_evaluation(tmp_r2);
      this->pending_evals.insert( this->pending_evals.end(),
                                  std::make_pair(id, s_id) );
      data.evalIDs.push_back(id);
      ++data.pending;

      index = orig;
   }
}


//========================================================================
//========================================================================


template <>
bool
FiniteDifferenceApplication_Domain<true>::
map_domain(const utilib::Any &src, utilib::Any &native, bool forward) const
{
   static_cast<void>(forward);
   // Because we are not going to EDIT the domain (permanently), we
   // can cast directly from one Any to the other.
   return utilib::TypeManager()->lexical_cast
      ( src, native, typeid(utilib::MixedIntVars) ) == 0;
}


template <>
void
FiniteDifferenceApplication_Domain<true>::
queue_fd_steps( eval_set_t::iterator s_id,
                const utilib::Any &domain,
                utilib::seed_t seed )
{
   const utilib::MixedIntVars &d = domain.expose<utilib::MixedIntVars>();
   FiniteDifferenceApplication_Core::EvalInfo &data = s_id->second;
      
   utilib::Any tmp_domain;
   utilib::MixedIntVars &tmp_d = tmp_domain.set(d);
   utilib::Any noAny;

   Application_Base* remote = this->remote_app.object();
   double step = this->step_size.expose<double>();
   bool central_diff = this->difference_mode == this->central;
   if ( central_diff ) 
      step *= 0.5;
   else if ( this->difference_mode == this->backward )
      step *= -1.0;

   bool compute_g = data.compute_fd
      & FiniteDifferenceApplication_Core::EvalInfo::gradient;
   bool compute_mo_g = data.compute_fd
      & FiniteDifferenceApplication_Core::EvalInfo::mo_gradient;
   bool compute_lcg = data.compute_fd
      & FiniteDifferenceApplication_Core::EvalInfo::l_jacobian;
   bool compute_nlcg = data.compute_fd
      & FiniteDifferenceApplication_Core::EvalInfo::nl_jacobian;
   bool compute_ndcg = data.compute_fd
      & FiniteDifferenceApplication_Core::EvalInfo::nd_jacobian;

   EvaluationID id;
   for( size_t i = 0; i < d.Real().size(); ++i )
   {
      double &index = tmp_d.Real()[i];
      double orig = index;
      index += step;

      AppRequest tmp_r1 = remote->set_domain(tmp_d, false, seed);
      if ( compute_g )
         this->record_remote_compute_task(remote, f_info, tmp_r1, noAny);
      if ( compute_mo_g )
         this->record_remote_compute_task(remote, mf_info, tmp_r1, noAny);
      if ( compute_lcg )
         this->record_remote_compute_task(remote, lcf_info, tmp_r1, noAny);
      if ( compute_nlcg )
         this->record_remote_compute_task(remote, nlcf_info, tmp_r1, noAny);
      if ( compute_ndcg )
         this->record_remote_compute_task(remote, ndcf_info, tmp_r1, noAny);
      id = remote->eval_mngr().queue_evaluation(tmp_r1);
      this->pending_evals.insert( this->pending_evals.end(), 
                                  std::make_pair(id, s_id) );
      data.evalIDs.push_back(id);
      ++data.pending;

      index = orig;

      if ( ! central_diff )
         continue;

      //
      // For central differences...
      //

      index -= step;

      AppRequest tmp_r2 = remote->set_domain(tmp_d, false, seed);
      if ( compute_g )
         this->record_remote_compute_task(remote, f_info, tmp_r2, noAny);
      if ( compute_mo_g )
         this->record_remote_compute_task(remote, mf_info, tmp_r2, noAny);
      if ( compute_lcg )
         this->record_remote_compute_task(remote, lcf_info, tmp_r2, noAny);
      if ( compute_nlcg )
         this->record_remote_compute_task(remote, nlcf_info, tmp_r2, noAny);
      if ( compute_ndcg )
         this->record_remote_compute_task(remote, ndcf_info, tmp_r2, noAny);
      id = remote->eval_mngr().queue_evaluation(tmp_r2);
      this->pending_evals.insert( this->pending_evals.end(),
                                  std::make_pair(id, s_id) );
      data.evalIDs.push_back(id);
      ++data.pending;

      index = orig;
   }
}


//========================================================================
//========================================================================


} // namespace colin

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

//
// Coliny_MultiStatePS.cpp
//
// A "multi-state" pattern search algorithm built on COLIN's AsyncEvaluator.
//

#include <acro_config.h>

#include <scolib/MultiStatePS.h>

//#include <utilib/_math.h>
//#include <utilib/stl_auxiliary.h>
#include <colin/cache/Factory.h>
#include <colin/SolverMngr.h>


#include <cmath>

#define DEBUG_MULTISTATE_PS 0


using std::runtime_error;
using std::string;
using std::cerr;
using std::endl;
using std::map;
using std::multimap;
using std::pair;

using colin::EvaluationID;
using colin::f_info;

namespace {
string indent = "";
string indent_chars = "  ";
size_t indent_amount = indent_chars.size();
}

namespace scolib {


/*--------------------------------------------------------------------
 * MultiStatePS Pattern States
 *------------------------------------------------------------------*/
namespace MultiState_Patterns {

typedef MultiStatePS::PatternState   PatternState;
typedef MultiStatePS::pseudoQueue_t  pseudoQueue_t;
typedef MultiStatePS::evalPriority_t evalPriority_t;
typedef MultiStatePS::response_t     response_t;
typedef MultiStatePS::request_t      request_t;
typedef MultiStatePS::queueSet_t     queueSet_t;
typedef MultiStatePS::StateContainer StateContainer;


// ---------------------------------------------------------------------
// FirstImproving pattern state
//
class FirstImproving : public MultiStatePS::PatternState
{
public:
   FirstImproving()
      : PatternState(),
        improving_step_count(1),
        improving_found(false),
        pattern_info(NULL)
   {}        

   FirstImproving( const FirstImproving *old_state, response_t center_point)
      : PatternState(old_state, center_point),
        improving_step_count(old_state->improving_step_count),
        improving_found(false),
        pattern_info(old_state->pattern_info)
   { ++(pattern_info->refCount); }

   virtual ~FirstImproving()
   {
      if ( --(pattern_info->refCount) == 0 )
         delete pattern_info;
   }


   virtual long pattern_id() const
   {
      return ( pattern_info == NULL ? 0 : pattern_info->pattern_id );
   }


   virtual void initialize_pattern( MultiStatePS *msps,
                                    response_t center_point )
   {
      PatternState::initialize_pattern(msps, center_point);

      assert(pattern_info == NULL);
      pattern_info = new FirstImproving_PatternInfo(msps);
      ++(pattern_info->refCount);

      double fval = center_point.value<double>(f_info);
      fval *= (colin::optimizationSense)msps->get_problem()->sense;
      StateContainer state = this;
      explore(state, fval, pattern_info->improving_queue);
   }


   
   virtual void process( MultiStatePS::response_t response, 
                         colin::EvaluationID eval_id )
   {
      if ( DEBUG_MULTISTATE_PS > 1 )
      {
         indent += indent_chars;
         cerr << indent << "entering FirstImproving::process()" << endl;
      }

      MultiStatePS         *msps = pattern_info->msps;
      double                sense 
         = (colin::optimizationSense)msps->get_problem()->sense;
      StateContainer        newState;
      FirstImproving       *newStatePtr = NULL;
      std::list<request_t>  newRequest;

      double response_fval = response.value<double>(f_info) * sense;
      double base_fval = center.value<double>(f_info) * sense;

      if ( response_fval < base_fval )
      {
         // found an improving point 
         improving_found = true;

         if ( DEBUG_MULTISTATE_PS > 2 )
         {
            cerr << indent << "improving point found" << endl;
         }
          
         newState = newStatePtr = new FirstImproving(this, response);

         // should we expand the step size?
         // NB: we use <= & not < because the count begins at 1 and not 0.
         if ( improving_step_count <= config->max_success )
         { 
            ++(newStatePtr->improving_step_count);
         }
         else
         { 
            if ( DEBUG_MULTISTATE_PS )
            {
               cerr << indent << "FirstImproving: expanding around state "
                    << id() << ", new step = " << 
                  step*config->expansion_factor << endl;
            }

            newStatePtr->step = step*config->expansion_factor;
         }

         explore(newState, response_fval, pattern_info->improving_queue);

         // Record if this is a new best point.
         if ( msps->best().value() * sense > response_fval )
         {
            msps->best().point = response.get_domain();
            msps->best().point.to_value_any();
            msps->best() << response;
            
            DEBUGPRX(100, msps, cerr << "New best point: value = " 
                    << msps->best().value() << endl);
         }
      }
      else
      {
         if ( DEBUG_MULTISTATE_PS > 2 )
         {
            cerr << indent << "non-improving point found" << endl;
         }
          
         // oh well... the point is NOT improving
         newState = newStatePtr = new FirstImproving(this, response);
         newStatePtr->improving_step_count = 0;
         explore(newState, newStatePtr->step, 
                 pattern_info->secondary_queue);

         // check if we should contract
         if ( ( pending == 0 ) && ( ! improving_found ) && 
              ( improving_step_count != 0 ) )
         { 
            double newStep = config->contraction_factor * step;
            if ( newStep >= config->delta_thresh )
            { 
               if ( DEBUG_MULTISTATE_PS )
               {
                  cerr << indent << "FirstImproving::process(): "
                     "contracting around state " << id() 
                       << ", new step = " << newStep << endl;
               }

               newState = newStatePtr = new FirstImproving(this, center);
               newStatePtr->improving_step_count = 1;
               newStatePtr->step = newStep;
               explore(newState, newStep, pattern_info->improving_queue);
            }
            else
            {
               if ( DEBUG_MULTISTATE_PS )
               {
                  cerr << indent << "FirstImproving::process(): minimum "
                     "step size reached." << endl;
               }

               // re-prioritize the search to balance with the
               // secondary exploration
               map<pseudoQueue_t, double> alloc;
               alloc[pattern_info->improving_queue] 
                  = alloc[pattern_info->secondary_queue] 
                  = 0.5;
               msps->set_pseudo_queue_alloc(pattern_info->queueSet, alloc);

               DEBUGPRX(100, msps, cerr << "Minimum delta reached.  No "
                        "further contraction allowed for state " << id()
                        << ", value = " << center.value<double>(f_info)
                        << endl);
            }
         }
      }

      if ( DEBUG_MULTISTATE_PS > 1 )
      {
         cerr << indent << "FirstImproving::process(): done" << endl;
         indent.resize(indent.size() - indent_amount);
      }
   }

private:
   void explore( StateContainer &state, 
                 evalPriority_t &priority,
                 pseudoQueue_t &queue )
   {
      if ( DEBUG_MULTISTATE_PS > 1 )
      {
         indent += indent_chars;
         cerr << indent << "entering FirstImproving::explore()" << endl;
      }

      FirstImproving *myState 
         = static_cast<FirstImproving*>(state.operator->());
      MultiStatePS &msps = *(pattern_info->msps);

      std::list<request_t> reqList;
      msps.expand_pattern_cartesian(myState->center, myState->step, reqList);

      while ( ! reqList.empty() )
      {
         msps.queue_state_eval(state, reqList.front(), priority, queue);
         reqList.pop_front();
      }

      if ( DEBUG_MULTISTATE_PS > 1 )
      {
         cerr << indent << "FirstImproving::explore() done." << endl;
         indent.resize(indent.size() - indent_amount);
      }
   }

private:
   struct FirstImproving_PatternInfo
   {
      FirstImproving_PatternInfo(MultiStatePS *ps)
         : msps(ps),
           refCount(0),
           pattern_id(ps->get_next_pattern_id()),
           queueSet(ps->new_pseudo_queue_set()),
           improving_queue(ps->new_pseudo_queue(queueSet)),
           secondary_queue(ps->new_pseudo_queue(queueSet))
      {
         // set the initial eval priority to ONLY improving points
         std::map<pseudoQueue_t, double> alloc;
         alloc[improving_queue] = 1.0;
         alloc[secondary_queue] = 0.0;
         msps->set_pseudo_queue_alloc(queueSet, alloc);
      }
      ~FirstImproving_PatternInfo()
      {
         msps->release_pseudo_queue_set(queueSet); 
      }

      MultiStatePS  *msps;
      size_t         refCount;
      long           pattern_id;
      queueSet_t     queueSet;
      pseudoQueue_t  improving_queue;
      pseudoQueue_t  secondary_queue;
   };

   int   improving_step_count;
   bool  improving_found;
   FirstImproving_PatternInfo *pattern_info;
};



// ---------------------------------------------------------------------
// BestImproving pattern state
//
class BestImproving : public MultiStatePS::PatternState
{
public:
   BestImproving()
      : PatternState(),
        improving_step_count(1),
        pattern_info(NULL)
   {}        

   BestImproving( const BestImproving *old_state, response_t center_point)
      : PatternState(old_state, center_point),
        improving_step_count(old_state->improving_step_count),
        pattern_info(old_state->pattern_info)
   { ++(pattern_info->refCount); }

   virtual ~BestImproving()
   {
      if ( --(pattern_info->refCount) == 0 )
         delete pattern_info;
   }


   virtual long pattern_id() const
   {
      return ( pattern_info == NULL ? 0 : pattern_info->pattern_id );
   }


   virtual void initialize_pattern( MultiStatePS *msps,
                                    response_t center_point )
   {
      PatternState::initialize_pattern(msps, center_point);

      assert(pattern_info == NULL);
      pattern_info = new BestImproving_PatternInfo(msps);
      ++(pattern_info->refCount);

      double fval = center_point.value<double>(f_info);
      fval *= (colin::optimizationSense)pattern_info->msps->get_problem()->sense;
      StateContainer state = this;
      explore(state, fval, pattern_info->improving_queue);
   }


   
   virtual void process( MultiStatePS::response_t response, 
                         colin::EvaluationID eval_id )
   {
      if ( DEBUG_MULTISTATE_PS > 1 )
      {
         indent += indent_chars;
         cerr << indent << "entering BestImproving::process()" << endl;
      }

      double sense = (colin::optimizationSense)pattern_info->msps->get_problem()->sense;
      response_buf.insert(pair<double,MultiStatePS::response_t>
                          ( (response.value<double>(f_info) * sense), 
                            response));

      if ( pending != 0 )
      {
         if ( DEBUG_MULTISTATE_PS > 1 )
         {
            cerr << indent << "BestImproving::process(): done (waiting for " 
                 << pending << " evals)" << endl;
            indent.resize(indent.size() - indent_amount);
         }
         return;
      }
      

      // All evals have been returned... make use of the fact that
      // multimaps are ordered to queue from the best to the worst...
      MultiStatePS         *msps = pattern_info->msps;
      StateContainer        newState;
      BestImproving        *newStatePtr = NULL;
      std::list<request_t>  newRequest;

      double base_fval = center.value<double>(f_info) * sense;
      double response_fval = response_buf.begin()->first;

      // Record if we have found a new best point.
      if ( msps->best().value() * sense > response_fval )
      {
         msps->best().point = response_buf.begin()->second.get_domain();
         msps->best().point.to_value_any();
         msps->best() << response_buf.begin()->second;
         
         DEBUGPRX(100, msps, cerr << "New best point: value = " 
                  << msps->best().value() << endl);
      }

      // check if we should contract
      if (( improving_step_count != 0 ) && ( base_fval <= response_fval ))
      { 
         double newStep = config->contraction_factor * step;
         if ( newStep >= config->delta_thresh )
         { 
            if ( DEBUG_MULTISTATE_PS )
            {
               cerr << indent << "BestImproving::process(): "
                  "contracting around state " << id() 
                    << ", new step = " << newStep << endl;
            }

            newState = newStatePtr = new BestImproving(this, center);
            newStatePtr->improving_step_count = 1;
            newStatePtr->step = newStep;
            explore(newState, newStep, pattern_info->improving_queue);
         }
         else
         {
            if ( DEBUG_MULTISTATE_PS )
            {
               cerr << indent << "BestImproving::process(): minimum "
                  "step size reached." << endl;
            }

            // re-prioritize the search to balance with the
            // secondary exploration
            map<pseudoQueue_t, double> alloc;
            alloc[pattern_info->improving_queue] 
               = alloc[pattern_info->secondary_queue] 
               = 0.5;
            msps->set_pseudo_queue_alloc(pattern_info->queueSet, alloc);

            DEBUGPRX(100, msps, cerr << "Minimum delta reached.  No "
                     "further contraction allowed for state " << id()
                     << ", value = " << center.value<double>(f_info)
                     << endl);
         }
      }

      while ( ! response_buf.empty() )
      {
         response_fval = response_buf.begin()->first;
         
         if ( response_fval < base_fval )
         {
            newState = newStatePtr 
               = new BestImproving(this, response_buf.begin()->second);

            // should we expand the step size?
            // NB: we use <= & not < because the count begins at 1 and not 0.
            if ( improving_step_count <= config->max_success )
            { 
               ++(newStatePtr->improving_step_count);
            }
            else
            { 
               if ( DEBUG_MULTISTATE_PS )
               {
                  cerr << indent << "BestImproving: expanding around state "
                       << id() << ", new step = " << 
                     step*config->expansion_factor << endl;
               }

               newStatePtr->step = step*config->expansion_factor;
            }

            explore(newState, response_fval, pattern_info->improving_queue);
         }
         else
         {
            if ( DEBUG_MULTISTATE_PS > 2 )
            {
               cerr << indent << "non-improving point found" << endl;
            }
         
            // oh well... the point is NOT improving
            newState = newStatePtr 
               = new BestImproving(this, response_buf.begin()->second);
            newStatePtr->improving_step_count = 0;
            explore(newState, newStatePtr->step, 
                    pattern_info->secondary_queue);
         }

         response_buf.erase(response_buf.begin());
      }

      if ( DEBUG_MULTISTATE_PS > 1 )
      {
         cerr << indent << "BestImproving::process(): (state complete) done"
              << endl;
         indent.resize(indent.size() - indent_amount);
      }
   }

private:
   void explore( StateContainer &state, 
                 evalPriority_t &priority,
                 pseudoQueue_t &queue )
   {
      if ( DEBUG_MULTISTATE_PS > 1 )
      {
         indent += indent_chars;
         cerr << indent << "entering BestImproving::explore()" << endl;
      }

      BestImproving *myState 
         = static_cast<BestImproving*>(state.operator->());
      MultiStatePS &msps = *(pattern_info->msps);

      std::list<request_t> reqList;
      msps.expand_pattern_cartesian(myState->center, myState->step, reqList);

      while ( ! reqList.empty() )
      {
         msps.queue_state_eval(state, reqList.front(), priority, queue);
         reqList.pop_front();
      }

      if ( DEBUG_MULTISTATE_PS > 1 )
      {
         cerr << indent << "BestImproving::explore() done." << endl;
         indent.resize(indent.size() - indent_amount);
      }
   }

private:
   struct BestImproving_PatternInfo
   {
      BestImproving_PatternInfo(MultiStatePS *ps)
         : msps(ps),
           refCount(0),
           pattern_id(ps->get_next_pattern_id()),
           queueSet(ps->new_pseudo_queue_set()),
           improving_queue(ps->new_pseudo_queue(queueSet)),
           secondary_queue(ps->new_pseudo_queue(queueSet))
      {
         // set the initial eval priority to ONLY improving points
         std::map<pseudoQueue_t, double> alloc;
         alloc[improving_queue] = 1.0;
         alloc[secondary_queue] = 0.0;
         msps->set_pseudo_queue_alloc(queueSet, alloc);
      }
      ~BestImproving_PatternInfo()
      {
         msps->release_pseudo_queue_set(queueSet); 
      }

      MultiStatePS  *msps;
      size_t         refCount;
      long           pattern_id;
      queueSet_t     queueSet;
      pseudoQueue_t  improving_queue;
      pseudoQueue_t  secondary_queue;
   };

   int  improving_step_count;
   multimap<double, MultiStatePS::response_t>  response_buf;
   BestImproving_PatternInfo *pattern_info;
};


} // namespace MultiState_Patterns

/*--------------------------------------------------------------------
 * MultiStatePS methods
 *------------------------------------------------------------------*/

MultiStatePS::MultiStatePS()
{ 
   //utilib::OptionParser& opt = this->option;

   config.delta_init = 1.0;
   this->properties.declare
      ( "initial_step", 
        "Initial step length",
        utilib::Privileged_Property(config.delta_init) );
   //opt.add("initial_step", config.delta_init,
   //        "Initial step length");
   //opt.alias("initial_step","initial_stepsize");
   //opt.alias("initial_step","initial_steplength");
  
   config.delta_thresh = 1e-5;
   this->properties.declare
      ( "step_tolerance", 
        "Convergence tolerance step length",
        utilib::Privileged_Property(config.delta_thresh) );
   //opt.add("step_tolerance", config.delta_thresh,
   //        "Convergence tolerance step length");

   config.contraction_factor = 0.5;
   this->properties.declare
      ( "contraction_factor", 
        "Contraction factor",
        utilib::Privileged_Property(config.contraction_factor) );
   //opt.add("contraction_factor", config.contraction_factor,
   //        "Contraction factor");

   config.max_success = 5;
   this->properties.declare
      ( "max_success", 
        "Number of successful iterations before step length is expanded",
        utilib::Privileged_Property(config.max_success) );
   //opt.add("max_success", config.max_success,
   //        "Number of successful iterations before step length is expanded");

   config.sigma.resize(1);
   config.sigma << 1.0;
   this->properties.declare
      ( "step_scales", 
        "The scale factors for each dimension",
        utilib::Privileged_Property(config.sigma) );
   //opt.add("step_scales", config.sigma,
   //        "The scale factors for each dimension");

   config.sufficient_decrease_coef = 0.01;
   this->properties.declare
      ( "alpha", 
        "Sufficient decrease parameter",
        utilib::Privileged_Property(config.sufficient_decrease_coef) );
   //opt.add("alpha", config.sufficient_decrease_coef,
   //        "Sufficient decrease parameter");

   config.expansion_factor = 2.0; 
   this->properties.declare
      ( "expansion_factor", 
        "Expansion factor",
        utilib::Privileged_Property(config.expansion_factor) );
   //opt.add("expansion_factor", config.expansion_factor,
   //        "Expansion factor");

   reset_signal.connect(boost::bind(&MultiStatePS::reset_MultiStatePS, this));
}



void MultiStatePS::reset_MultiStatePS()
{
   if ( problem.empty() ) 
      return; 

   unsigned int nvars = problem->num_real_vars;
   if ( nvars == 0 )
      return; 

   config.sigma.resize(nvars);
   config.sigma << 1.0;

   eval_mngr().release_queue();
}



void MultiStatePS::optimize()
{
   if ( DEBUG_MULTISTATE_PS > 1 )
      cerr << "entering optimize(); " << endl; 

   //
   // Misc initialization of the optimizer
   //
   //opt_init();
   //if ( DEBUG_MULTISTATE_PS > 1 )
   //   cerr << "optimize(): opt_init done" << endl; 


   if ( ! this->initial_point_flag )
   {
      EXCEPTION_MNGR(runtime_error,
                     "MultiStatePS::optimize - no initial point specified.");
   }

   if ( problem->num_real_vars != initial_point.size() )
   {
      EXCEPTION_MNGR
         ( runtime_error, "MultiStatePS::optimize - problem has " <<
           problem->num_real_vars << " real params, but initial point has "
           << initial_point.size() );
   }
   if ( initial_point.size() == 0 ) 
   {
      solver_status.termination_info = "No-Real-Params";
      return;
   }

   if (( max_neval == 0 ) && ( max_neval_curr == 0 ) && ( max_iters == 0 ) && 
       ( std::fabs(max_time) < 0.001 ))
   {
      cerr << "(INFO): none of max_time, max_iters, max_neval, or "
           << "max_neval_curr set." << endl
           << "        MultiStatePS currently has no termination criteria; "
           << endl
           << "        setting max_neval = 100,000." << endl;
      max_neval = 100000;
   }

   if ( DEBUG_MULTISTATE_PS )
   {
      write(cerr);
      cerr << "----------------" << endl;
   }


   /*
   //
   //
   unsigned int num_iters;
   if ( max_iters <= 0 )
   { num_iters = MAXINT; }
   else
   { num_iters = curr_iter + max_iters; }
   */

   if ( ! problem->testBoundsFeasibility(initial_point) )
   {
      EXCEPTION_MNGR
         ( runtime_error,
           "MultiStatePS::optimize - initial point is not bound-feasible" );
   }

   
   if ( DEBUG_MULTISTATE_PS > 1 )
   {
      cerr << "optimize(): evaluating initial point" << endl; 
      cerr << indent_chars << "initial point: " << initial_point << endl;
   }

   // NB: the point won't be in the cache, but this will make sure it is added
   response_t response = checkCache(initial_point);
   if ( ! response.is_computed(f_info) )
   {
      if ( DEBUG_MULTISTATE_PS > 1 )
      {
         cerr << indent_chars << "computing response for initial point." 
              << endl;
      }

      colin::AppRequest req = problem->set_domain(response.get_domain());
      problem->Request_F(req);
      eval_mngr().perform_evaluation(req);

      if ( DEBUG_MULTISTATE_PS > 1 )
      {
         cerr << indent_chars << "initial point computed." 
              << endl;
      }
   }
   best().point = initial_point;
   best() << response;



   if ( DEBUG_MULTISTATE_PS > 1 )
      cerr << "optimize(): exploring initial point" << endl; 

   // initialize the patterns we will use.
   //(new MultiState_Patterns::FirstImproving())
   //   ->initialize_pattern(this, response);
   (new MultiState_Patterns::BestImproving())
      ->initialize_pattern(this, response);

   // set the target allocation evenly among all patterns
   set_pseudo_queue_set_alloc(map<queueSet_t, double>());


   if ( DEBUG_MULTISTATE_PS > 1 )
      cerr << "entering optimize()::loop; " << endl; 
   std::pair<EvaluationID, colin::AppResponse> eval;
   while ( ! (eval = eval_mngr().next_response()).first.empty() )
   {
      
      // I don't really have a concept of an "iteration", but to make this
      // somewhat consistent with other Colin solvers, I will make every
      // processed evaluation an iteration.
      ++curr_iter;

      if ( DEBUG_MULTISTATE_PS )
      { 
         cerr << "optimize(): best = " << best().value() 
              << ", total eval = " << neval() << endl;
      }
      if ( DEBUG_MULTISTATE_PS > 1 )
      { 
         cerr << indent_chars << "processing eval_id " << eval.first << endl;
      }

      if ( check_convergence() )
         break; 

      //
      // TODO: check feasibility here...
      //

      // process the 'callback'
      map<EvaluationID, StateContainer>::iterator eval_cb 
         = m_pending.find(eval.first);
      if ( eval_cb == m_pending.end() )
      {
         EXCEPTION_MNGR( runtime_error,
                         "MultiStatePS::optimize - invalid evalID returned" );
      }

      --(eval_cb->second->pending);
      eval_cb->second->process(eval.second, eval.first);
      m_pending.erase(eval_cb);
   }

   if ( eval.first.empty() )
   {
      solver_status.termination_info = "No remaining queued evaluations";
   }

   final_points->insert(best().response);
   if ( DEBUG_MULTISTATE_PS )
      cerr << "optimize(): complete" << endl; 
}



void MultiStatePS::write(std::ostream& os) const
{
   ConcreteOptSolver_t::write(os);

   os << "##\n## MultiState Pattern Search Controls\n##\n";

   os << "max_success         " << config.max_success << endl;
   os << "delta_init          " << config.delta_init << endl;
   os << "delta_thresh        " << config.delta_thresh << endl;
   os << "contraction_factor  " << config.contraction_factor << endl;
   os << "expansion_factor    " << config.expansion_factor << endl;
   os << "sigma:              " << config.sigma << endl;
}



MultiStatePS::response_t
MultiStatePS::checkCache(MultiStatePS::domain_t &point)
{
   static colin::Cache* cache = NULL;
#if 0
   if ( cache == NULL )
      cache = colin::CacheFactory().create("Local", "Epsilon");
#endif
   EXCEPTION_MNGR(std::logic_error, 
                  "MSPS is currently broken and needs to be reworked "
                  "to use new colin caching system");
   return response_t();
}


colin::EvaluationID
MultiStatePS::queue_state_eval( StateContainer state, 
                                request_t request,
                                evalPriority_t priority,
                                pseudoQueue_t queue ) 
{
   colin::EvaluationID eval_id 
      = eval_mngr().queue_evaluation(request, priority, queue);
   m_pending[eval_id] = state;
   ++(state->pending);

   if ( DEBUG_MULTISTATE_PS > 1 )
   {
      cerr << indent << indent_chars 
           << "request queued as eval_id " << eval_id << endl;
   }
   return eval_id;
}


void
MultiStatePS::expand_pattern_cartesian( response_t  newCenter, 
                                        double step,
                                        std::list<request_t> & new_pts )
{
   if ( DEBUG_MULTISTATE_PS > 1 )
   {
      indent += indent_chars;
      cerr << indent << "entering expand_pattern_cartesian()" << endl; 
   }

   unsigned int  nvars = problem->num_real_vars;
   response_t    rs;
   domain_t      trial;
   newCenter.get_domain(trial);

   if ( DEBUG_MULTISTATE_PS > 1 )
   {
      cerr << indent << "expand_pattern_cartesian(): center fval = " 
           << newCenter.value<double>(f_info) << endl;
      cerr << indent << indent_chars << "domain = " << trial << endl;
   }
  
   if ( DEBUG_MULTISTATE_PS > 1 )
   { 
      cerr << indent << "expand_pattern_cartesian(): entering point "
         "generation" << endl; 
   }

   for(unsigned int i = 0; i < nvars; ++i)
   {
      double origVal = trial[i];
      bool positive = false;

      do 
      {
         if ( positive )
            trial[i] += step * (config.sigma[i]); 
         else
            trial[i] -= step * (config.sigma[i]); 
         rs = checkCache(trial);

         if ( DEBUG_MULTISTATE_PS > 1 )
         { 
            cerr << indent << indent_chars << "trial(" 
                 << (positive ? "+" : "-") << "): " 
                 << trial;// << " [" << rs.number_computed(f_info) << "]"; 
         }

         if ( ! rs.is_computed(f_info) )
         { 
            colin::AppRequest rq = problem->set_domain(rs.get_domain());
            problem->Request_F(rq);
            new_pts.push_back(rq);
         }
         trial[i] = origVal;

         if ( DEBUG_MULTISTATE_PS > 1 )
            cerr << endl; 

         positive = !positive;
      } while ( positive );
   }

   if ( DEBUG_MULTISTATE_PS > 1 )
   { 
      cerr << indent << "expand_pattern_cartesian(): done" << endl; 
      indent.resize(indent.size() - indent_amount);
   }
}


MultiStatePS::queueSet_t
MultiStatePS::new_pseudo_queue_set()
{
   ++m_lastQueueSet;

   // rescale the current queueSet allocations so that the new set gets 1/N
   double scale = static_cast<double>(m_queueSets.size());
   scale /= (scale+1.0);

   map<queueSet_t, QueueSetInfo>::iterator qs_it = m_queueSets.begin();
   map<queueSet_t, QueueSetInfo>::iterator qs_itEnd = m_queueSets.end();
   for( ; qs_it != qs_itEnd; ++qs_it )
   {
      qs_it->second.alloc *= scale;
   }

   // We don't have to actually reallocate the underlying eval_mngr
   // allocation because there aren't any new queues.  Since the
   // eval_mngr will normalize everything it would result in a no-op.

   // "create" the new queueSet
   m_queueSets[m_lastQueueSet].alloc = 1.0 / ( 1.0 - scale );
   return m_lastQueueSet;
}


void
MultiStatePS::release_pseudo_queue_set(queueSet_t queueSet)
{
   map<queueSet_t, QueueSetInfo>::iterator qs_it = m_queueSets.find(queueSet);
   if ( qs_it == m_queueSets.end() )
      return;

   // release all queues owned by the queue set.
   map<pseudoQueue_t, double> &queueList = qs_it->second.queues;
   while ( ! queueList.empty() )
   {
      eval_mngr().release_queue(queueList.begin()->first);
      queueList.erase(queueList.begin());
   }

   // "delete" this queue set
   double scale = qs_it->second.alloc;
   if ( scale < 1.0 )
      scale = 1.0 / (1.0 - scale);
   m_queueSets.erase(qs_it);

   // rescale all our queue set allocations.  NB: we don't have to do
   // anything with the underlying eval_mngr because it will
   // automatically re-allocate the allocation that belonged to each
   // pseudo queue porportionally to all remaining queue sets.
   map<queueSet_t, QueueSetInfo>::iterator qs_itEnd = m_queueSets.end();
   for(qs_it = m_queueSets.begin() ; qs_it != qs_itEnd; ++qs_it )
   {
      qs_it->second.alloc *= scale;
   }
}


void
MultiStatePS::get_pseudo_queue_set_alloc(std::map<queueSet_t, double> &alloc)
{
   map<queueSet_t, QueueSetInfo>::iterator it = m_queueSets.begin();
   map<queueSet_t, QueueSetInfo>::iterator itEnd = m_queueSets.end();
   
   alloc.clear();
   map<queueSet_t, double>::iterator ans = alloc.begin();
   for( ; it != itEnd; ++it )
   {
      ans = alloc.insert(ans, pair<queueSet_t, double>
                          (it->first, it->second.alloc) );
   }
}


void
MultiStatePS::set_pseudo_queue_set_alloc(const std::map<queueSet_t, double>
                                         &alloc)
{
   // normalize the provided allocations
   map<queueSet_t, double>::const_iterator new_it = alloc.begin();
   map<queueSet_t, double>::const_iterator new_itEnd = alloc.end();
   double min = 0;
   double norm = 0;
   for (; new_it != new_itEnd; ++new_it)
   {
      if (min > new_it->second)
      { min = new_it->second; }
      norm += new_it->second;
   }
   if (min < 0)
   { norm -= min * alloc.size(); }
   if (( m_queueSets.size() > 0 ) && ( alloc.size() > 0 ))
   {
      norm *= ( static_cast<double>(m_queueSets.size()) / 
                static_cast<double>(alloc.size()) );
   }

   double default_alloc = m_queueSets.size();
   if (default_alloc > 0)
   { default_alloc = 1.0 / default_alloc; }

   // update the actual assignemnts
   new_it = alloc.begin();
   map<queueSet_t, QueueSetInfo>::iterator it = m_queueSets.begin();
   map<queueSet_t, QueueSetInfo>::iterator itEnd = m_queueSets.end();
   while (it != itEnd)
   {
      if ((new_it == new_itEnd) || (it->first < new_it->first))
      {
         it->second.alloc = default_alloc;
         ++it;
         continue;
      }
      if (it->first == new_it->first)
      {
         if ( norm == 0 )
         { it->second.alloc = 0; }
         else
         { it->second.alloc = (new_it->second - min) / norm; }

         ++it;
         ++new_it;
         continue;
      }
      if (it->first > new_it->first)
      {
         EXCEPTION_MNGR(std::runtime_error, "MultiStatePS::"
                        "set_pseudo_queue_set_alloc(): specified new "
                        "allocation for unknown queue set ID.");
      }
   }
   if (new_it != new_itEnd)
   {
      EXCEPTION_MNGR(std::runtime_error, "MultiStatePS::"
                     "set_pseudo_queue_set_alloc(): specified new "
                     "allocation for unknown queue set ID.");
   }

   // now actually record the queue allocations with the eval manager
   set_eval_mngr_allocations();
}


MultiStatePS::pseudoQueue_t
MultiStatePS::new_pseudo_queue(queueSet_t queueSet)
{
   map<queueSet_t, QueueSetInfo>::iterator qs_it = m_queueSets.find(queueSet);
   if ( qs_it == m_queueSets.end() )
   {
      EXCEPTION_MNGR(std::runtime_error, "MultiStatePS::"
                     "new_pseudo_queue(): invalid queueSet.");
   }

   pseudoQueue_t pseudoQueue = eval_mngr().get_new_queue_id();

   // rescale current pseudo queue allocations so that the new one gets 1/N
   double scale = static_cast<double>(qs_it->second.queues.size());
   scale /= (scale+1.0);

   map<pseudoQueue_t, double>::iterator it = qs_it->second.queues.begin();
   map<pseudoQueue_t, double>::iterator itEnd = qs_it->second.queues.end();
   for( ; it != itEnd; ++it )
   {
      it->second *= scale;
   }

   // "create" the new queue
   qs_it->second.queues[pseudoQueue] = 1.0 / ( 1.0 - scale );

   // now actually record the queue allocations with the eval manager
   set_eval_mngr_allocations();

   return pseudoQueue;
}


void
MultiStatePS::release_pseudo_queue(queueSet_t queueSet, pseudoQueue_t pseudo)
{
   map<queueSet_t, QueueSetInfo>::iterator qs_it = m_queueSets.find(queueSet);
   if ( qs_it == m_queueSets.end() )
   {
      EXCEPTION_MNGR(std::runtime_error, "MultiStatePS::"
                     "release_pseudo_queue(): invalid queueSet.");
   }

   map<pseudoQueue_t, double>::iterator it = qs_it->second.queues.find(pseudo);
   map<pseudoQueue_t, double>::iterator itEnd = qs_it->second.queues.end();
   if ( it == itEnd )
   {
      EXCEPTION_MNGR(std::runtime_error, "MultiStatePS::"
                     "release_pseudo_queue(): invalid pseudo queue.");
   }

   eval_mngr().release_queue(pseudo);

   // "delete" this queue set
   double scale = it->second;
   if ( scale < 1.0 )
      scale = 1.0 / (1.0 - scale);
   qs_it->second.queues.erase(it);

   // rescale all our other pseudo queue allocations.  
   for(it = qs_it->second.queues.begin() ; it != itEnd; ++it )
   {
      it->second *= scale;
   }

   // NB: since the eval_mngr will automatically re-allocate the
   // allocation that belonged to the pseudo queue porportionally to all
   // remaining queues (and not just to those within this queue set), we
   // need to manually re-allocate things.
   set_eval_mngr_allocations();
}


void
MultiStatePS::get_pseudo_queue_alloc( queueSet_t queueSet, 
                                      std::map<pseudoQueue_t, double> &alloc )
{
   map<queueSet_t, QueueSetInfo>::iterator qs_it = m_queueSets.find(queueSet);
   if ( qs_it == m_queueSets.end() )
   {
      EXCEPTION_MNGR(std::runtime_error, "MultiStatePS::"
                     "get_pseudo_queue_alloc(): invalid queueSet.");
   }

   alloc = qs_it->second.queues;
}


void
MultiStatePS::set_pseudo_queue_alloc( queueSet_t queueSet, 
                                      const std::map<pseudoQueue_t, double>
                                      &alloc )
{
   map<queueSet_t, QueueSetInfo>::iterator qs_it = m_queueSets.find(queueSet);
   if ( qs_it == m_queueSets.end() )
   {
      EXCEPTION_MNGR(std::runtime_error, "MultiStatePS::"
                     "set_pseudo_queue_alloc(): invalid queueSet.");
   }

   // normalize the provided allocations
   map<pseudoQueue_t, double>::const_iterator new_it = alloc.begin();
   map<pseudoQueue_t, double>::const_iterator new_itEnd = alloc.end();
   double min = 0;
   double norm = 0;
   for (; new_it != new_itEnd; ++new_it)
   {
      if (min > new_it->second)
      { min = new_it->second; }
      norm += new_it->second;
   }
   if (min < 0)
   { norm -= min * alloc.size(); }
   if (( qs_it->second.queues.size() > 0 ) && ( alloc.size() > 0 ))
   {
      norm *= ( static_cast<double>(qs_it->second.queues.size()) / 
                static_cast<double>(alloc.size()) );
   }

   double default_alloc = qs_it->second.queues.size();
   if (default_alloc > 0)
   { default_alloc = 1.0 / default_alloc; }

   // update the actual assignemnts
   new_it = alloc.begin();
   map<pseudoQueue_t, double>::iterator it = qs_it->second.queues.begin();
   map<pseudoQueue_t, double>::iterator itEnd = qs_it->second.queues.end();
   while (it != itEnd)
   {
      if ((new_it == new_itEnd) || (it->first < new_it->first))
      {
         it->second = default_alloc;
         ++it;
         continue;
      }
      if (it->first == new_it->first)
      {
         if ( norm == 0 )
         { it->second = 0; }
         else
         { it->second = (new_it->second - min) / norm; }

         ++it;
         ++new_it;
         continue;
      }
      if (it->first > new_it->first)
      {
         EXCEPTION_MNGR(std::runtime_error, "MultiStatePS::"
                        "set_pseudo_queue_alloc(): specified new "
                        "allocation for unknown pseudo queue ID.");
      }
   }
   if (new_it != new_itEnd)
   {
      EXCEPTION_MNGR(std::runtime_error, "MultiStatePS::"
                     "set_pseudo_queue_alloc(): specified new "
                     "allocation for unknown pseudo queue ID.");
   }

   // now actually record the queue allocations with the eval manager
   set_eval_mngr_allocations();
}


void
MultiStatePS::set_eval_mngr_allocations()
{
   map<colin::QueueManager::queueID_t, double> alloc;

   map<queueSet_t, QueueSetInfo>::iterator qs_it = m_queueSets.begin();
   map<queueSet_t, QueueSetInfo>::iterator qs_itEnd = m_queueSets.end();
   map<pseudoQueue_t, double>::iterator it;
   map<pseudoQueue_t, double>::iterator itEnd;
   for( ; qs_it != qs_itEnd; ++qs_it )
   {
      itEnd = qs_it->second.queues.end();
      for( it = qs_it->second.queues.begin(); it != itEnd; ++it )
      {
         alloc[it->first] = it->second * qs_it->second.alloc;
      }
   }
   eval_mngr().set_queue_alloc(alloc);
}


// Defines scolib::StaticInitializers::MultiStatePS_bool
REGISTER_COLIN_SOLVER_WITH_ALIAS
  ( MultiStatePS, "sco:MultiStatePS", "sco:msps", 
    "The SCO Multi-State Pattern Search optimizer" )

} // namespace scolib

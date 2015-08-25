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

#include <acro_config.h>
#include <scolib/StateMachineLS.h>
#include <colin/SolverMngr.h>
#include <colin/EvaluationID.h>
#include <colin/cache/View_Pareto.h>

#include <ctime>

using utilib::Any;
using utilib::Property;

using std::string;
using std::ifstream;
using std::stringstream;
using std::vector;
using std::list;
using std::pair;
using std::make_pair;
using std::cerr;
using std::cout;
using std::endl;

namespace scolib {

struct StateMachineLS::Data {
   template<typename VAR, typename VAL>
   class StateBase : public std::pair<VAR, VAL>
   {
   public:
      StateBase()
         : std::pair<VAR, VAL>(), 
           index(this->first), 
           value(this->second)
      {}

      StateBase(VAR index_, VAL value_)
         : std::pair<VAR, VAL>(index_, value_), 
           index(this->first), 
           value(this->second)
      {}

      StateBase(const StateBase& rhs)
         : std::pair<VAR, VAL>(rhs), 
           index(this->first), 
           value(this->second)
      {}

      VAR & index;
      VAL & value;
   };

   //typedef std::pair<size_t, int> State;
   typedef StateBase<int, int> State;

   struct CoReq {
      CoReq( int index_, int value_, char op_ )
         : index(index_),
           value(value_),
           op(op_)
      {}

      bool satisfied(const std::vector<int> &position)
      {
         if ( op == '=' )
            return position[index] == value;
         if ( op == '!' )
            return position[index] != value;
         if ( op == '<' )
            return position[index] < value;
         if ( op == '>' )
            return position[index] > value;
         if ( op == 'L' )
            return position[index] <= value;
         if ( op == 'G' )
            return position[index] >= value;
         return false;
      }

      int index;
      int value;
      char op;
   };
   
   struct StateTransition {
      std::list<CoReq> coreq;
      std::list<State> move;
   };

   Data()
      : states_loaded(false)
   {}

   typedef std::multimap<State, StateTransition> states_t;

   states_t state_transitions;
   states_t index_transitions;
   states_t value_transitions;

   bool states_loaded;
};


/*----------------------------------------------------------------------*/
namespace {

int algorithm_verbosity = 0;


template<typename T>
inline void
read_token(size_t lineno, stringstream &buf, string &str, T &token)
{
   buf >> token >> std::ws;
   if ( buf.fail() )
   {
      string remainder;
      buf >> remainder;
      EXCEPTION_MNGR(std::runtime_error, "StateMachineLS::read_token(): "
                     "Error reading state token: expected value at "
                     "line " << lineno << ", character " << 
                     (str.size() - remainder.size()) << ":" << endl
                     << "\t" << str << endl);
   }
}

inline char
read_sep(size_t lineno, stringstream &buf, string &str, const char* legal)
{
   char ans;
   buf >> ans >> std::ws;
   if ( buf.fail() )
   {
      string remainder;
      buf >> remainder;
      EXCEPTION_MNGR(std::runtime_error, "StateMachineLS::read_token(): "
                     "Error reading separator token: expected one of \""
                     << string(legal) << "\" at line " 
                     << lineno << ", character " << 
                     (str.size() - remainder.size()) << ":" << endl
                     << "\t" << str << endl );
   }
   if ( ! std::strchr(legal, ans) )
   {
      string remainder;
      buf >> remainder;
      EXCEPTION_MNGR(std::runtime_error, "StateMachineLS::read_sep(): "
                     "Invalid separator token: expected one of \""
                     << string(legal) << "\" but found \"" << string(1,ans) 
                     << "\" at line " << lineno << ", character " << 
                     (str.size() - remainder.size()) << ":" << endl
                     << "\t" << str << endl );
   }
   return ans;
}

template<typename T>
inline void 
validate_token( size_t lineno, stringstream &buf, string &str, 
                T token, T lb, T ub )
{
   if ( token < lb || token >= ub )
   {
      string remainder;
      buf >> remainder;
      EXCEPTION_MNGR(std::runtime_error, "StateMachineLS::validate_token(): "
                     "State token out of range: expected value " 
                     << lb << " <= x < " << ub << ", but found " << token
                     << " at line " << lineno << ", character " << 
                     (str.size() - remainder.size()) << ":" << endl
                     << "\t" << str << endl);
   }
}


bool 
test_and_prep_step( std::vector<int> &current, 
                    std::vector<int> &move, 
                    StateMachineLS::Data::StateTransition &transition,
                    int index )
{
   list<StateMachineLS::Data::CoReq>::iterator c_it = transition.coreq.begin();
   for ( ; c_it != transition.coreq.end(); ++c_it )
      if ( ! c_it->satisfied(current) )
         return false;

   move = current;
   list<StateMachineLS::Data::State>::iterator m_it = transition.move.begin();
   for ( ; m_it != transition.move.end(); ++m_it )
      move[m_it->first == -1 ? index : m_it->first] = m_it->second;
   if ( algorithm_verbosity >= 3 )
      cout << "   ...trial point " << move << endl;
   return true;//f(move);
}

} // namespace scolib::(local)
/*----------------------------------------------------------------------*/


StateMachineLS::StateMachineLS()
   : state_definition_file(Property::Bind<string>("StateMachineLS.states")),
     max_iterations(Property::Bind<size_t>(0)),
     max_fcn_evaluations(Property::Bind<size_t>(0)),
     max_time(Property::Bind<double>(0)),
     verbosity(Property::Bind<int>(1)),
     data(new Data)
{
   properties.declare
      ("state_definition_file",
       "Input file defining the state machine of valid moves "
       "[default: StateMachineLS.states]",
       state_definition_file);

   properties.declare
      ("max_iterations",
       "Maximum number of algorithm iterations (0 == unlimited)"
       "[default: 0]",
       max_iterations);

   properties.declare
      ("max_fcn_evaluations",
       "Maximum number of function evaluations (0 == unlimited)"
       "[default: 0]",
       max_fcn_evaluations);

   properties.declare
      ("max_time",
       "Maximum solver wall clock run time (0 == unlimited)"
       "[default: 0]",
       max_time);

   properties.declare
      ("verbosity",
       "Set the algorithm verbosity level"
       "[default: 1]",
       verbosity);
}


void StateMachineLS::reset()
{
   load_states();
}

void StateMachineLS::optimize()
{
   std::time_t start_time = std::time(NULL);

   if ( ! data->states_loaded )
      load_states();

   solver_status.model_status = colin::model_intermediate_nonoptimal;

   int numVar = problem->num_int_vars;
   bool minimizing = problem->sense == colin::minimization;

   // pull in initial points
   colin::AppResponse currentResponse;
   vector<int> currentPoint;
   double currentObj = 0;

   colin::cache::View_Pareto best_point;
   best_point.application_context = problem->get_handle();
   best_point.set_core_cache(initial_points.cache());
   if ( initial_points->size() == 0 )
   {
      // Arbitrarily pick a point at the origin (if allowable), or
      // else in the middle of the range
      vector<int> lb = problem->int_lower_bounds;
      vector<int> ub = problem->int_upper_bounds;
      currentPoint.resize(numVar);
      for( size_t i = 0; i < numVar; ++i )
         if ( lb[i] <= 0 && ub[i] >= 0 )
            currentPoint[i] = 0;
         else
            currentPoint[i] = lb[i] + (ub[i]-lb[i]/2);
      currentResponse = problem->EvalF(eval_mngr(), currentPoint, currentObj);
   }
   else
   {
      if ( best_point.size() == 0 )
      {
         // No f_info are available... go ahead and evaluate everything
         // and pick the best one
         for ( colin::Cache::iterator it = initial_points->begin();
               it != initial_points->end(); 
               ++it )
         {
            problem->AsyncEvalF
               ( eval_mngr(), it->second.asResponse(problem).get_domain() );
         }
         eval_mngr().synchronize();
      }
      currentResponse = best_point.begin()->second.asResponse(problem);
      currentResponse.get_domain(currentPoint);
      currentResponse.get(colin::f_info, currentObj);
   }

   size_t maxIter = max_iterations;
   size_t maxNumEval = max_fcn_evaluations;
   double max_run_time = max_time;

   pair<colin::EvaluationID, colin::AppResponse> eval;
   double newObj;

   algorithm_verbosity = verbosity;

   size_t iter = 0;
   vector<int> tmpDomain;
   Data::states_t::iterator it;
   while ( ( iter++ < maxIter || ! maxIter ) && 
           ( this->neval() < maxNumEval || ! maxNumEval ) &&
           ( ( max_run_time == 0 ) || 
             ( max_run_time > std::difftime(std::time(NULL), start_time) ) ) )
   {
      if ( algorithm_verbosity )
         cout << "StateMachineLS: " << endl
              << "   Iteration: " << iter << endl
              << "   Fcn Evals: " << this->neval() << endl
              << "   Incumbent solution:" << endl
              << "      Objective: " << currentObj << endl
              << "      Domain:    " << currentPoint << endl;

      Data::states_t::iterator s_itEnd = data->state_transitions.end();
      Data::states_t::iterator i_itEnd = data->index_transitions.end();
      Data::states_t::iterator v_itEnd = data->value_transitions.end();
      for ( int i = 0; i < numVar; ++i )
      {
         int v = currentPoint[i];

         // First, queue the explicit state transitions
         for ( it = data->state_transitions.find(Data::State(i, v));
               it != s_itEnd && it->first.index == i && it->first.value == v;
               ++it )
            if ( test_and_prep_step(currentPoint, tmpDomain, it->second, i) )
               problem->AsyncEvalF(eval_mngr(), tmpDomain);

         // Next, queue the implict index-based transitions
         for ( it = data->index_transitions.find(Data::State(i, 0));
               it != i_itEnd && it->first.index == i;
               ++it )
            if ( test_and_prep_step(currentPoint, tmpDomain, it->second, i) )
               problem->AsyncEvalF(eval_mngr(), tmpDomain);

         // Finally, queue the implicit value-based transitions
         for ( it = data->value_transitions.find(Data::State(-1, v));
               it != v_itEnd && it->first.value == v;
               ++it )
            if ( test_and_prep_step(currentPoint, tmpDomain, it->second, i) )
               problem->AsyncEvalF(eval_mngr(), tmpDomain);
      }

      // Evaluate all queued steps
      newObj = currentObj;
      double obj_trial;
      while ( ( this->neval() < maxNumEval || ! maxNumEval )
              && ! (eval = eval_mngr().next_response()).first.empty() )
      {
         eval.second.get(colin::f_info, obj_trial);
         bool improving = false;
         if ( minimizing )
            improving = obj_trial < newObj;
         else
            improving = obj_trial > newObj;
         if ( improving )
         {
            newObj = obj_trial;
            eval.second.get_domain(tmpDomain, problem);
            currentResponse = eval.second;
            if ( algorithm_verbosity >= 2 )
               cout << "   ...improving solution " << obj_trial 
                    << " at " << tmpDomain << endl;
         }
         else if ( algorithm_verbosity >= 10 )
         {
            cout << "   ...non-improving solution " << obj_trial 
                 << " at " << tmpDomain << endl;
         }
      }

      // Check if none of the moves results in an improvement
      if ( newObj == currentObj )
      {
         solver_status.termination_condition=colin::termination_locallyOptimal;
         solver_status.model_status=colin::model_locally_optimal;
         break;
      }

      // Update the current position and move on
      currentObj = newObj;
      currentPoint = tmpDomain;
   }

   final_points->insert(currentResponse);
   if ( maxNumEval && this->neval() >= maxNumEval )
      solver_status.termination_condition = colin::termination_maxEvaluations;
   if ( maxIter && iter >= maxIter )
      solver_status.termination_condition = colin::termination_maxIterations;
}


void StateMachineLS::load_states()
{
   data->state_transitions.clear();
   data->index_transitions.clear();
   data->value_transitions.clear();

   ifstream file;
   file.open(state_definition_file.as<string>().c_str());
   if ( file.fail() )
      EXCEPTION_MNGR(std::runtime_error, "StateMachineLS::load_states(): "
                     "Error opening state definition file, " << 
                     state_definition_file);

   size_t lineno = 0;
   string line_buffer;
   std::stringstream line;
   
   int numInt = problem->num_int_vars;
   int index = -1;
   int value = 0;
   Data::states_t::iterator it;
   while ( file.good() )
   {
      ++lineno;
      std::getline(file, line_buffer);
      line.str(line_buffer);
      line.clear();
      line >> std::noskipws;
      line >> std::ws;
      if ( ! line.good() )
         continue;

      // s = full state, i = variable index only, v = variable value only
      char sep = read_sep(lineno, line, line_buffer, "siv#");
      if ( sep == '#' )
         continue;
      if ( sep != 'v' )
      {
         read_token(lineno, line, line_buffer, index);
         validate_token(lineno, line, line_buffer, index, -1, numInt);
      }
      else
         index = -1;
      
      if ( sep == 's' )
         read_sep(lineno, line, line_buffer, ",");
      
      if ( sep != 'i' )
         read_token(lineno, line, line_buffer, value);
      else
         value = 0;

      Data::states_t *storage = NULL;
      if ( sep == 's' )
         storage = & data->state_transitions;
      else if ( sep == 'i' )
         storage = & data->index_transitions;
      else // if ( sep == 'v' )
         storage = & data->value_transitions;
      it = storage->insert( storage->end(), 
                            make_pair( Data::State(index, value), 
                                       Data::StateTransition() ) );

      sep = read_sep(lineno, line, line_buffer, ":|");
      while ( sep != '|' )
      {
         read_token(lineno, line, line_buffer, index);
         validate_token(lineno, line, line_buffer, index, -1, numInt);
         read_sep(lineno, line, line_buffer, ",");
         read_token(lineno, line, line_buffer, value);
         read_sep(lineno, line, line_buffer, ",");
         sep = read_sep(lineno, line, line_buffer, "=<>!LG");
         it->second.coreq.push_back(Data::CoReq(index, value, sep));

         sep = read_sep(lineno, line, line_buffer, ";|");
      }
      while ( line.good() )
      {
         read_token(lineno, line, line_buffer, index);
         validate_token(lineno, line, line_buffer, index, -1, numInt);
         read_sep(lineno, line, line_buffer, ",");
         read_token(lineno, line, line_buffer, value);
         it->second.move.push_back(Data::State(index, value));
         if ( line.eof() )
            break;
         read_sep(lineno, line, line_buffer, ";");
      }
   }

   data->states_loaded = true;
}


REGISTER_COLIN_SOLVER_WITH_ALIAS
(StateMachineLS, "sco:StateMachineLS", "sco:smls", 
"The SCO State Machine based local search optimizer");

} // namespace scolib

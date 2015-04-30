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
 * \file EAbase.h
 *
 * Defines the scolib::EAbase class.
 */

#ifndef scolib_EAbase_h
#define scolib_EAbase_h

#include <acro_config.h>
#include <scolib/EAindividual.h>
#include <colin/solver/ColinSolver.h>
#include <utilib/std_headers.h>
#include <utilib/_math.h>
#include <utilib/Uniform.h>
#include <utilib/IntVector.h>
#include <utilib/BitArray.h>

namespace scolib {

using colin::real;
using utilib::BasicArray;
using utilib::BitArray;

/// Base class for evolutionary algorithms
template <class PointT, class DomainT, class ProblemT>
class EAbase : public colin::ColinSolver<DomainT, ProblemT>
{
public:

  ///
  typedef colin::AppResponse response_t;

  ///
  friend class EAindividual<PointT,DomainT>;

  ///
  EAbase();

  ///
  virtual ~EAbase();

  ///
  void optimize();

  ///
  void write(std::ostream& os) const;

  ///
  virtual unsigned int popsize() const
		{return popsize_;}

  ///
  virtual unsigned int numgenerated() const
		{return num_trial_points;}

  ///
  std::string Mutation_type;

  ///
  std::string Xover_type;

  ///
  bool always_eval_flag;

  ///
  int neval_crit;

  ///
  double neval_thresh;

protected:

  ///
  virtual void init(const unsigned int popsize);

  /// number of calls to optimize()
  int run_count;

  ///
  unsigned int popsize_;

  ///
  bool init_unique;

  ///
  std::string init_filename;

  ///
  unsigned int num_trial_points;

  ///
  virtual void setup_ls(EAindividual<PointT, DomainT >& ) {}

  /// frequency (int iterations) with which LS is applied
  int ls_iters;

  ///
  int ls_id;

  ///
  double ls_freq;

  ///
  bool Lamarck_flag;

  /// true if redo LS nonLamarckian LS
  bool ls_redo_flag;

  /// 1 if recalc true\_eval after Lamarkian LS; 2 if recalc eval after Lamarkian LS.
  bool ls_eval_flag;

  ///
  bool ls_first;


  /**@name Debugging Controls */
  //@{
  ///
  int Debug_pop_points;

  ///
  int Debug_pop_stats;

  ///
  bool Debug_pop_distribution;

  ///
  bool Debug_search_steps;

  ///
  bool Debug_time_eval;

  ///
  bool Debug_constraints;

  ///
  bool Debug_num_identical;

  ///
  bool Offline_ls;

  ///
  void virt_debug_io(std::ostream& os, const bool finishing, const int io_level);

  ///
  virtual void print_points(std::ostream& os, int flag=false) const;

  /// Optimization statistics
  virtual void update_stats(bool first_time=false);

  ///
  double max_feval;

  ///
  EAindividual<PointT,DomainT> max_pt;

  ///
  double prev_min_feval;

  ///
  EAindividual<PointT,DomainT> min_pt;

  ///
  double true_max_feval;

  ///
  EAindividual<PointT,DomainT> true_max_pt;

  ///
  double true_min_feval;

  ///
  EAindividual<PointT,DomainT> true_min_pt;

  /// number of identical solutions in population
  int num_identical;

  ///
  double pop_avg;

  /// Max point in current generation
  double pop_max;

  /// Index to Max point
  int pop_argmax;

  /// Min point in current generation
  double pop_min;

  /// Index to Min point
  int pop_argmin;

  ///
  double pop_true_avg;

  /// Max point in current generation
  double pop_true_max;

  /// Index to Max point
  int pop_true_argmax;

  /// Min point in current generation
  double pop_true_min;

  /// Index to Min point
  int	pop_true_argmin;
	///
  double avg_ls_len;
  //@}

  /// Solutions from the current population
  EAindividual<PointT,DomainT>& population(const unsigned int id)
		{ return *(pop_array[id]); }

  /// Solutions from the current population
  const EAindividual<PointT,DomainT>& population(const unsigned int id) const
		{ return *(pop_array[id]); }

	/// New solutions that will be used to construct the next population
  EAindividual<PointT,DomainT>& trial(const unsigned int id)
		{ return *(trial_array[id]); }

   ///
  int new_individual_id()
		{ return id_counter++; }

  ///
  int id_counter;


  /**@name Main Routines Used to Execute the GA */
  //@{
  ///
  virtual void exec_generation();

  ///
  virtual void generate_new_solutions() = 0;

  ///
  virtual void perform_replacement() = 0;

  ///
  virtual int exec_evaluate(int num_eval, BasicArray<EAindividual<PointT,DomainT>* >& pt);

  ///
  virtual void exec_local_search();
  //@}

  /**@name Operator Controls */
  //@{
  ///
  double xover_rate;

  ///
  double mutation_rate;

  ///
  virtual void apply_xover(EAindividual<PointT,DomainT>& parent1, EAindividual<PointT,DomainT>& parent2,
			EAindividual<PointT,DomainT>& child) = 0;

  ///
  virtual bool apply_mutation(EAindividual<PointT,DomainT>& point, int parent_ndx) = 0;

  ///
  virtual void set_op_rng(utilib::AnyRNG&) = 0;
  //@}

  ///
  void Eval(EAindividual<PointT,DomainT>& pt, const bool synchronous=false, const bool always_eval_flag=false);

  /**@name Local Search Controls */
  //@{
  ///
  utilib::Uniform rnd;

  ///
  BitArray lsMask;

  ///
  bool ls_scale_flag;

  /// Operations that need to be performed before local search is applied to an individual
  virtual void init_ls(EAindividual<PointT,DomainT>& child);

  /// Operations that need to be performed after local search is applied to an individual
  virtual void fini_ls(EAindividual<PointT,DomainT>& ) {}

  ///
  void perform_local_search(EAindividual<PointT,DomainT>& parent, 
						EAindividual<PointT,DomainT>& child);

  /// Select individuals on which local search will be performed
  virtual void setup_local_search();
  //@}

  /// Used to compute the number of identical points
  BitArray bitndx;

  ///
  BasicArray<EAindividual<PointT,DomainT>* > pop_array;

  ///
  BasicArray<EAindividual<PointT,DomainT>* > trial_array;

  ///
  response_t default_response;

  ///
  //BasicArray<double> clower, cupper;

  ///
  void reset_impl();

};

template <class PointT, class DomainT, class ProblemT>
EAbase<PointT,DomainT,ProblemT>::EAbase()
   : run_count(0),
     num_trial_points(0),
     ls_iters(1),
     ls_id(0),
     Lamarck_flag(true),
     ls_redo_flag(false),
     ls_eval_flag(false),
     Debug_pop_points(false),
     Debug_pop_stats(false),
     Debug_pop_distribution(false),
     Debug_search_steps(false),
     Debug_time_eval(false),
     Debug_constraints(false),
     Debug_num_identical(false),
     Offline_ls(false)
{
   neval_crit = 0;
   neval_thresh = 0.0;
   id_counter = 0;

   //utilib::OptionParser &opt = this->option;

   ls_first = true;
   this->properties.declare
      ( "ls_first", 
        "If true, then perform local search before the first generation",
        utilib::Privileged_Property(ls_first) );
   //opt.add("ls_first",ls_first,
   //        "If true, then perform local search before the first generation");

   always_eval_flag = false;
   this->properties.declare
      ( "always_eval", 
        "If false, then perform bookkeeping to detect redundant function "
        "evaluations",
        utilib::Privileged_Property(always_eval_flag) );
   //opt.add("always_eval",always_eval_flag,
   //        "If false, then perform bookkeeping to detect redundant function "
   //        "evaluations");

   Mutation_type = "unknown";
   this->properties.declare
      ( "mutation_type", 
        "The type of mutation used by the EA",
        utilib::Privileged_Property(Mutation_type) );
   //opt.add("mutation_type",Mutation_type,
   //        "The type of mutation used by the EA");

   mutation_rate = 1.0;
   this->properties.declare
      ( "mutation_rate", 
        "The probability that an individual has a mutation operator "
        "applied to it",
        utilib::Privileged_Property(mutation_rate) );
   //opt.add("mutation_rate",mutation_rate,
   //        "The probability that an individual has a mutation operator "
   //        "applied to it");

   xover_rate = 0.8;
   this->properties.declare
      ( "xover_rate", 
        "The probability that xover is applied to generate new individuals",
        utilib::Privileged_Property(xover_rate) );
   //opt.add("xover_rate",xover_rate,
   //        "The probability that xover is applied to generate new "
   //        "individuals");

   ls_freq = -1.0;
   this->properties.declare
      ( "ls_freq", 
        "The probability that local search is applied to any individual.  "
        "This defaults to 1.0 if there exists one or more local searchers.",
        utilib::Privileged_Property(ls_freq) );
   //opt.add("ls_freq",ls_freq,
   //        "The probability that local search is applied to any individual.  "
   //        "This defaults to 1.0 if there exists one or more local searchers.");

   Xover_type = "unknown";
   this->properties.declare
      ( "xover_type", 
        "The type of crossover used by the EA",
        utilib::Privileged_Property(Xover_type) );
   //opt.add("xover_type",Xover_type,
   //        "The type of crossover used by the EA");

   popsize_ = 100;
   this->properties.declare
      ( "population_size", 
        "The population size used by the EA",
        utilib::Privileged_Property(popsize_) );
   //opt.add("population_size",popsize_,
   //        "The population size used by the EA");

   init_filename = "";
   this->properties.declare
      ( "init_filename", 
        "A file that can be used to load an initial population",
        utilib::Privileged_Property(init_filename) );
   //opt.add("init_filename",init_filename,
   //        "A file that can be used to load an initial population");

   init_unique = true;
   this->properties.declare
      ( "population_unique", 
        "If true, then try to form a population of unique points.  "
        "Otherwise, use the standard random initialization strategy.",
        utilib::Privileged_Property(init_unique) );
   //opt.add("population_unique", init_unique,
   //        "If true, then try to form a population of unique points.  "
   //        "Otherwise, use the standard random initialization strategy.");
   //opt.alias("population_unique","distinct_initial_population");
}

template <class PointT, class DomainT, class ProblemT>
EAbase<PointT,DomainT,ProblemT>::~EAbase()
{
}
 

#if 0
template <class PointT, class DomainT, class ProblemT>
void EAbase<PointT,DomainT,ProblemT>::exec_eval_and_local_search(int first)
{
if (!first || (first && ls_first))
   exec_local_search();
if (!first)
   exec_evaluate(numgenerated(),trial_array);
else
   exec_evaluate(popsize(),pop_array);
}
#endif


template <class PointT, class DomainT, class ProblemT>
void EAbase<PointT,DomainT,ProblemT>::reset_impl()
{
this->eval_mngr().clear_evaluations();

//this->problem->get_nonlinear_constraint_bounds(clower,cupper);

rnd.generator(&(this->rng));
set_op_rng(this->rng);

if (ls_freq < 0.0) {
   if (this->sub_solver.size() > 0) 
      ls_freq=1.0;
   else
      ls_freq=0.0;
   }
if (ls_freq > 0.0) {
   this->properties["realarray_mutation_selfadaptation"] = false;
   lsMask.resize(popsize());
   lsMask.reset();
   }

/*
default_response.resize(this->problem->num_objectives,
			this->problem->numNonlinearConstraints(),
			this->num_real_vars(),
			mode_f | mode_cf);
*/
//
// Double check whether we need to all of the stuff in 'init' with the reset, but at least
// we should make sure to resize the problem dimensions
//
init(popsize());
}



template <class PointT, class DomainT, class ProblemT>
void EAbase<PointT,DomainT,ProblemT>::optimize()
{
//exit_fn();
//this->opt_init();
++run_count;

exec_evaluate(popsize(),pop_array);
if (ls_first)
   exec_local_search();

OUTPUTPR(2,ucout << "EAbase::Updating stats\n");
update_stats(true);
this->debug_io(ucout);

this->curr_iter++;
while (!(this->check_convergence())) {

  exec_generation();

  this->debug_io(ucout);
  this->curr_iter++;
  }

// Return the final population
for(size_t i = 0; i < this->pop_array.size(); ++i)
{
   this->final_points->insert(pop_array[i]->response);
}

// Return the best point ever found
this->final_points->insert(this->opt_response.response);

this->debug_io(ucout,true);
this->eval_mngr().clear_evaluations();
}



template <class PointT, class DomainT, class ProblemT>
void EAbase<PointT,DomainT,ProblemT>::virt_debug_io(std::ostream& os, const bool finishing, const int io_level)
{
if (io_level >= 0) {
   if (io_level == 3) {
      os << std::endl;
      print_points(os, true);
      }

   return;
   }

if (this->debug == 0) return;

static int p_neval=0;

if (this->Debug_neval || this->Debug_all) {
   int tmp = this->neval();
   os << "[Neval-Percent:\t" << ((double)(tmp-p_neval))
			/popsize() << "]\n";
   p_neval = tmp;
   }

if (Debug_time_eval || this->Debug_all) {
   os << "[TimeEval:\t" << this->time_eval << "]\n";
   }

if (this->Debug_opt_stats || this->Debug_all) {
   os << "[Max:\t" << max_feval << "]\n";
   os << "[Max-point:\n\t" << max_pt.pt << "]\n";
   os << "[True-max:\t" << true_max_feval << "]\n";
   os << "[True-max-point:\n\t" << true_max_pt.pt <<"]\n";
   os << "[True-min:\t" << true_min_feval << "]\n";
   os << "[True-min-point:\n\t" << true_min_pt.pt <<"]\n";
   }

if (Debug_num_identical || this->Debug_all) 
   os << "[Num-identical:\t" << num_identical <<"]\n";

//
// Go through list of population stats
//
if ((this->Debug_all && !finishing) ||
    ((this->Debug_pop_stats > 0) && 
     (((this->curr_iter % this->Debug_pop_stats) == 0) ^ finishing))) {
   os << "[PopMax:\t" << pop_max << "]\n";
   os << "[PopMin:\t" << pop_min << "]\n";
   os << "[PopAvg:\t" << pop_avg << "]\n";
   os << "[PopTrueMax:\t" << pop_true_max << "]\n";
   os << "[PopTrueMin:\t" << pop_true_min << "]\n";
   os << "[PopTrueAvg:\t" << pop_true_avg << "]\n";

   }

//
// Print current population
//
if ( (this->Debug_all && !finishing) ||
     ((this->Debug_pop_points > 0) && 
     (((this->curr_iter % this->Debug_pop_points) == 0) ^ finishing))) {
   os << "[Population:\n";
   print_points(os, true);
   os << "]\n";
   }
}

template <class PointT, class DomainT, class ProblemT>
void EAbase<PointT,DomainT,ProblemT>::update_stats(bool first_time)
{
if (Debug_num_identical) {
   bitndx.set();

   num_identical=0;
   for (unsigned int i=0; i<popsize()-1; i++)
     if (bitndx(i))
        for (unsigned int j=i+1; j<popsize(); j++)
          if (population(i) == population(j)) {
             num_identical++;
             bitndx.reset(j);
             }
   }

pop_true_avg = mean(pop_array, EAindividual_array_accessor<PointT,DomainT>(false));
pop_true_argmin = argmin(pop_array, EAindividual_array_compare<PointT,DomainT>(false));
pop_true_argmax = argmax(pop_array, EAindividual_array_compare<PointT,DomainT>(false));
pop_true_min = pop_array[pop_true_argmin]->Val;
pop_true_max = pop_array[pop_true_argmax]->Val;

pop_avg = mean(pop_array, EAindividual_array_accessor<PointT,DomainT>(true));
pop_argmax = argmax(pop_array, EAindividual_array_compare<PointT,DomainT>(true));
pop_argmin = argmin(pop_array, EAindividual_array_compare<PointT,DomainT>(true));
pop_min = pop_array[pop_argmin]->Sel_Val;
pop_max = pop_array[pop_argmax]->Sel_Val; 

if (first_time == true) {
   min_pt.copy(population(pop_argmin));
   max_feval = pop_max;
   max_pt.copy(population(pop_argmax));

   true_min_feval = pop_true_min;
   true_min_pt.copy(population(pop_true_argmin));
   true_max_feval = pop_true_max;
   true_max_pt.copy(population(pop_true_argmax));
   }
else {
   if (this->best().value() > pop_min) {
      min_pt.copy(population(pop_argmin));
      }
   if (max_feval < pop_max) {
      max_feval = pop_max;
      max_pt.copy(population(pop_argmax));
      }  

   if (true_min_feval > pop_true_min) {
      true_min_feval = pop_true_min;
      true_min_pt.copy(population(pop_true_argmin));
      }
   if (true_max_feval < pop_true_max) {
      true_max_feval = pop_true_max;
      true_max_pt.copy(population(pop_true_argmax));
      }
   }
//
// Update the opt_response
//
if (first_time || (pop_array[pop_argmin]->Sel_Val < this->best().value())) {
   this->best().value() = pop_array[pop_argmin]->Sel_Val;
   //this->best().constraint_violation = pop_array[pop_argmin]->Cval;
   this->best().point = pop_array[pop_argmin]->pt.point;
   this->best().response = pop_array[pop_argmin]->response;
   }
}



template <class PointT, class DomainT, class ProblemT>
void EAbase<PointT,DomainT,ProblemT>::init(const unsigned int popsize__)
{
//
// For simplicity, we just delete the old EAindividual objects
// and then reallocate them.  init() is not called very often
//
if ((pop_array.size() != popsize__)) {
   for (unsigned int i=0; i<pop_array.size(); i++)
     delete pop_array[i];
   pop_array.resize(popsize__);
   bitndx.resize(popsize__);
   popsize_ = popsize__;
   for (unsigned int i=0; i<pop_array.size(); i++)
     pop_array[i] = new EAindividual<PointT,DomainT>(default_response);
   }

if (num_trial_points != trial_array.size()) {
   for (unsigned int i=0; i<trial_array.size(); i++)
     delete trial_array[i];
   trial_array.resize(num_trial_points);
   for (unsigned int i=0; i<trial_array.size(); i++)
     trial_array[i] = new EAindividual<PointT,DomainT>(default_response);
   }
}


template <class PointT, class DomainT, class ProblemT>
void EAbase<PointT,DomainT,ProblemT>::exec_generation()
{
OUTPUTPR(2,ucout << "EAbase::Generating New Solutions\n");
generate_new_solutions();

OUTPUTPR(2,ucout << "EAbase::Evaluate New Solutions\n");
exec_evaluate(numgenerated(),trial_array);

OUTPUTPR(2,ucout << "EAbase::Combining Populations\n");
perform_replacement();

OUTPUTPR(2,ucout << "EAbase::Performing Local Search\n");
exec_local_search();
 
OUTPUTPR(2,ucout << "EAbase::Updating stats\n");
update_stats();
 
OUTPUTPR(2,ucout << "EAbase::Finishing generation\n");
}



template <class PointT, class DomainT, class ProblemT>
//int EAbase<PointT,DomainT,ProblemT>::exec_evaluate(int num_eval, 
//		EAindividual<PointT,DomainT>& (EAbase<PointT,DomainT>::*pt)(const unsigned int i))
int EAbase<PointT,DomainT,ProblemT>::exec_evaluate(int num_eval, BasicArray<EAindividual<PointT,DomainT>* >& pt)
{
int i,flag=false;
double time_tmp;

for (i=0; i<num_eval; i++) {
  OUTPUTPR(4,
           ucout << "Evaluating Pt #" << i << std::endl;
           ucout << (*pt[i]) << std::endl;
           );
  if ((pt[i]->eval_flag == true) || (always_eval_flag == true)) {

    if (Debug_time_eval == true) {
       time_tmp = CPUSeconds();
       Eval(*pt[i],false,always_eval_flag);
       this->time_eval += (CPUSeconds() - time_tmp);
       }
    else {
       Eval(*pt[i],false,always_eval_flag);
       }

    flag = true;
    }
  }
/// WEH - is this equivalent to performing execute_evaluations???
this->eval_mngr().synchronize();

#if 0
int ndx=-1;
for (i=0; i<num_eval; i++) {
  pt[i]->update_from_response();
  if (pt[i]->Sel_Val < this->best().value) {
     if (ndx == -1) ndx=i;
     else if (pt[i]->Sel_Val < pt[ndx]->Sel_Val) ndx=i;
     }

  }
#else
for (i=0; i<num_eval; i++) {
   pt[i]->update_from_response();
   colin::CacheHandle cache = colin::CacheFactory().evaluation_cache();
   if ( ! cache.empty() )
   {
      colin::Cache::iterator it =
         cache->find(this->problem, pt[i]->pt.point).first;
      if ( it != cache->end() )
      {
         std::stringstream ss;
         ss << "ea-" << run_count << ".generated";
         if ( it->second.annotations.find(ss.str()) == it->second.annotations.end() )
            cache->annotate(it, ss.str(), this->curr_iter);
      }
   }

  OUTPUTPR(4,
           ucout << "Evaluated Pt #" << i << std::endl;
           ucout << *pt[i] << std::endl;
           );
  }
#endif

#if 0
WEH - this should be done in the 'update_stats' method
if (ndx != -1) {
   //
   // Found an improving best point, so archive it.
   //
   this->best().value = pt[ndx]->Sel_Val;
   this->best().constraint_violation = pt[ndx]->Cval;
   this->best().point << pt[ndx]->pt.point;
   this->best().response << pt[ndx]->response;
}
#endif

return( flag );
}


template <class PointT, class DomainT, class ProblemT>
void EAbase<PointT,DomainT,ProblemT>::Eval(EAindividual<PointT,DomainT>& pt, const bool synchronous, const bool always_eval_flag_)
{
if ((pt.eval_flag == false) && (always_eval_flag_ == false))
   return;

// Setup the evaluation that will be performed
colin::AppRequest request = this->problem->set_domain(pt.pt.point);
this->problem->Request_response(request,pt.response);
this->problem->Request_F(request,pt.Sel_Val);
if (synchronous) {
   // Perform the evaluation synchronously
   pt.response = this->problem->eval_mngr().perform_evaluation(request);
   // Update EAindividual information
   pt.update_from_response();
   }
else {
   // Perform the evaluation asynchronously
   this->eval_mngr().queue_evaluation(request);
   }
}


template <class PointT, class DomainT, class ProblemT>
void EAbase<PointT,DomainT,ProblemT>::print_points(std::ostream& os, int verbose) const
{
if (verbose == false) {
   for (unsigned int i=0; i<popsize(); i++) {
     if (this->debug)
        os << population(i) << "\n";
     else {
	population(i).write_point(os);
	os << std::endl;
	}
     }
   }

else {
   os << "Current Population Statistics" << std::endl; 
   os << "Maximum: " << pop_max << " :: ";
   population(pop_argmax).write_point(os);
   os << std::endl;
   os << "Minimum: " << pop_min << " :: ";
   population(pop_argmin).write_point(os);
   os << std::endl;

   os << "ID\tParents\tOps\tValue\t\t\tPoint" << std::endl;
   for (unsigned int i=0; i<popsize(); i++) {
     os << i << ")\t" << population(i).parent1_id << " " <<
	   population(i).parent2_id;
     os << "\t" << (population(i).gen_status > 1 ? "x" : "-")
		   << ((population(i).gen_status == 1) || 
		       (population(i).gen_status == 3) ? "m" : "-");
     os << (population(i).ls_flag ? "L" : "-");
     os << "\t" << population(i).Sel_Val << "\t";
     population(i).write_point(os);
     os << std::endl;
     }
   }
}



template <class PointT, class DomainT, class ProblemT>
void EAbase<PointT,DomainT,ProblemT>::write(std::ostream& os) const
{
colin::ColinSolver<DomainT,ProblemT>::write(os);

os << "##" << std::endl;
os << "## EAbase:  Base Evolutionary Algorithm" << std::endl;
os << "##" << std::endl;

os << "#\n# Basic EA Controls\n#\n";
os << "mutation_rate        " << mutation_rate << std::endl;
os << "xover_rate           " << xover_rate << std::endl;
os << "popsize              " << popsize_ << "\n";
os << "always_eval_flag     " << always_eval_flag << std::endl;
//os << "using_phenotype      " << using_phenotype << std::endl;
//os << "using_pheno_geno_map " << using_pheno_geno_map << std::endl;

os << "#\n# Population Statistics\n#\n";
os << "avg    " << pop_avg << std::endl;
os << "min    " << pop_min << std::endl;
os << "max    " << pop_max << std::endl;
os << "argmin " << pop_argmin << "\n";
os << "argmax " << pop_argmax << "\n";

os << "#\n# Misc Statistics/Properties\n#\n";
os << "num_trial_points " << num_trial_points << std::endl;
os << "avg_ls_len       " << avg_ls_len << std::endl;

os << "#\n# Debugging Controls\n#\n";
os << "Debug_pop_points       " << Debug_pop_points << std::endl;
os << "Debug_pop_stats        " << Debug_pop_stats << std::endl;
os << "Debug_pop_distribution " << Debug_pop_distribution << std::endl;
os << "Debug_time_eval        " << Debug_time_eval << std::endl;
os << "Debug_constraints      " << Debug_constraints << std::endl;
os << "Debug_num_identical    " << Debug_num_identical << std::endl;


os << "#\n# Local Search Controls\n#\n";
os << "ls_freq         " << ls_freq << std::endl;
os << "ls_iters        " << ls_iters << std::endl;
os << "ls_Lamarck_flag " << Lamarck_flag << std::endl;
os << "ls_redo_flag    " << ls_redo_flag << std::endl;
os << "ls_eval_flag    " << ls_eval_flag << std::endl;
os << "ls_id           " << ls_id << std::endl;
os << "Offline_ls      " << Offline_ls << std::endl;
}


template <class PointT, class DomainT, class ProblemT>
void EAbase<PointT,DomainT,ProblemT>::setup_local_search()
{
//
// Determine the number of local searches to perform
//
double fraction = ls_freq*popsize() - std::floor(ls_freq*popsize());
int num_ls;
if (fraction == 0.0)
   num_ls = (int) (ls_freq*popsize());
else         
   num_ls = (int) (rnd() > fraction ? std::floor(ls_freq*popsize()) :
                                     std::ceil(ls_freq*popsize()));
//
// Setup a bit array for the local search elements, and then shuffle
//
lsMask.reset();
for (int i=0; i<num_ls; i++)
  lsMask.set(i);
shuffle(lsMask,&(this->rng));
}



template <class PointT, class DomainT, class ProblemT>
void EAbase<PointT,DomainT,ProblemT>::exec_local_search()
{
if (this->sub_solver.size() == 0)
   return;
if (this->curr_iter % ls_iters != 0)
   return;
if (ls_freq == 0.0)
   return;
//
// Select points for local search
//
setup_local_search();
OUTPUTPR(10, ucout << "LS Num: " << lsMask.nbits() 
         << " Mask: " << lsMask << std::endl);
//
// Perform local search on the selected points
//
for (size_type i=0; i<lsMask.size(); i++) {
  if (lsMask(i))
     perform_local_search(population(i), population(i));
  }
OUTPUTPR(1000, print_points(ucout,true));
}
 

template <class PointT, class DomainT, class ProblemT>
void EAbase<PointT,DomainT,ProblemT>::init_ls(EAindividual<PointT,DomainT>& child)
{
setup_ls(child);
(this->sub_solver[0].first)->reset();
}


template <class PointT, class DomainT, class ProblemT>
void EAbase<PointT,DomainT,ProblemT>::perform_local_search(EAindividual<PointT,DomainT>& parent, EAindividual<PointT,DomainT>& child)
{
   OUTPUTPR(1000, 
            ucout << "(Begin-LS:" << std::endl;
            );
child.copy(parent);
if ((child.eval_flag == false) &&
    (child.ls_flag == true) &&
    (Lamarck_flag == false) &&
    (ls_redo_flag == false)) {
   OUTPUTPR(10,
            ucout << "Don't repeat non-Lam LS: eval_flag " << child.eval_flag
                  << "  ls_flag " << child.ls_flag
                  << "  Lam_flag " << Lamarck_flag
                  << "  ls_eval " << ls_redo_flag << "\n";
            );
   return;
   }
 
OUTPUTPR(10, ucout << "[Starting pt: " << child << "]\n");
Eval(child,true);
(this->sub_solver[0].first)->add_initial_point(child.pt.point);

int debug_neval = -1;
OUTPUTPR(1000, 
         ucout << "[Iter:\t" << this->curr_iter << " ]" << std::endl;
         ucout << "[LS_Init_Val:\t" << child.Sel_Val << " ]" << std::endl;
         debug_neval = this->neval();
         );

//
// Perform the actual local search
//
init_ls(child);
(this->sub_solver[0].first)->optimize();
fini_ls(child);


#if 0
  ERROR - this assumes that the final point is being returned with a
  function value!
OUTPUTPR(1000,
         ucout << "[LS_Fini_Val:\t"
               << (this->sub_solver[0].first)->opt_response.value() << " ]" 
               << std::endl;
         ucout << "[LS_Num_Eval:\t" << (this->neval()-debug_neval) << " ]" 
               << std::endl;
         );
#endif
//
// If we are performing offline local search, then we don't actually
// change anything in the child
//
if (Offline_ls)
   return;
//
// Update the child's status
//
child.ls_flag = true;
child.eval_flag=false;
#if 0
  WEH - ERROR this assumes that the final point has a value!
typename EAbase<PointT,DomainT,ProblemT>::response_t& lresponse = (this->sub_solver[0].first)->opt_response.response; 
child.update((this->sub_solver[0].first)->opt_response, Lamarck_flag);
#endif

#if 0
//
// When performing Lamarckian LS, update the current point
// and other data for the point
//
if (Lamarck_flag == true) {
   phenotype(child)->point << (*(this->sub_solver[0].first)).best().point;
   /// TODO - revisit this code!!!!

   if (!using_phenotype) {
      child.true_eval() = child.eval();
      child.eval_flag = false;
      }
   else {
      if (using_pheno_geno_map) {
         child.gen_genotype();
         if (ls_eval_flag > 0) { 	// Calculate the true_eval
            double tmp;
            tmp = child.eval();
            child.phenotype_flag = true;
            child.gen_phenotype();
            child.eval_flag = true;
            Eval(child,true);
            if (ls_eval_flag == 1)
               child.eval() = tmp;
            }
         else
            child.eval_flag = false;
         }
      else
         EXCEPTION_MNGR(runtime_error,"EAbase::perform_local_search : Lamarckian local search with geno-pheno distinction, without pheno_geno_func");
      }
   }
#endif
 
OUTPUTPR(10, ucout << "[Child: " << child << " ]\n");
OUTPUTPR(1000,
         ucout << ")" << std::endl;
         );
}

} // namespace scolib

#endif

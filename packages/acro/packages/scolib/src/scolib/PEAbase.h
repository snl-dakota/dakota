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
 * \file PEAbase.h
 *
 * Defines the scolib::PEAbase class.
 */

#ifndef scolib_PEAbase_h
#define scolib_PEAbase_h

#include <algorithm>
#include <acro_config.h>
#include <utilib/sort.h>
#include <scolib/EAbase.h>
#include <scolib/panmictic_selection.h>

#define RANDOM_REPLACEMENT	0
#define CHC_REPLACEMENT		1
#define ELITIST_REPLACEMENT	2
#define EXPONENTIAL_REPLACEMENT	3

namespace scolib {


/** Base header for EA's using panmictic populations. */
template <class PointT, class DomainT, class ProblemT>
class PEAbase : public virtual EAbase<PointT,DomainT,ProblemT> {

public:

  typedef EAbase<PointT,DomainT,ProblemT> base_t;
  #if !defined(SOLARIS)
  using base_t::num_trial_points;
  using base_t::mutation_rate;
  using base_t::ls_freq;
  using base_t::curr_iter;
  using base_t::popsize_;
  using base_t::pop_array;
  using base_t::trial_array;
  #endif

  /**@name General Information */
  //@{
  ///
  PEAbase();

  ///
  virtual ~PEAbase();

  ///
  virtual void init_population(const int popsize, bool unique=true)
		{this->init_unique=unique; this->init(popsize);}

  ///
  virtual void init_population(const int popsize, const char* filename)
		{this->init_filename=filename; this->init(popsize);}

  ///
  void reset_impl();

  ///
  void write(std::ostream& os) const;

  ///
  void read(std::istream& is);
  //@}

  ///
  panmictic_selection<EAindividual<PointT,DomainT>*,
		      EAindividual_array_accessor<PointT,DomainT>,
		      EAindividual_array_compare<PointT,DomainT> >
		selection_obj;

  ///
  void get_final_points(std::vector<DomainT>& points)
        {
        points.resize(this->popsize());
        order(pop_order,pop_array,EAindividual_array_compare<PointT,DomainT>(true));
        for (unsigned int i=0; i<points.size(); i++) {
          points[i] = this->population(pop_order[i]).pt.point;
	  //ucout << "Final Point " << i << " " << this->population(pop_order[i]).Sel_Val << std::endl;
          }
        }

protected:

  /**@name Configuration Controls */
  //@{
  /// The probability of accepting each ranked element
  double exp_replacement_factor;

  ///
  std::string replacement_str;

  /// id of replacement algorithm
  unsigned int replacement_flag;

  /// number of individuals NOT replaced
  unsigned int keep_num; 

  /// Map used by \Ref{generate_new_points}
  BasicArray<int> map;		
  //@}

  /**@name Iteration Controls */
  //@{
  ///
  #if 0
  void perform_selection(BasicArray<real >& vals,
                        BasicArray<int>& selections, int num)
	{
	selection_obj.initialize(vals,
		EAindividual_array_accessor<PointT,DomainT>(true),
		EAindividual_array_compare<PointT,DomainT>(true));
   	selection_obj.perform_selection(selections,num);
	}
  #endif

  ///
  BasicArray<int> pop_order;

  ///
  BasicArray<int> trial_order;

  ///
  void generate_new_solutions();

  ///
  void perform_replacement();
  //@}
 
};


 
template <class PointT, class DomainT, class ProblemT>
PEAbase<PointT,DomainT,ProblemT>::PEAbase()
   : selection_obj(this->properties)
{
   popsize_=0;
   this->init(0);

   replacement_str = "elitist";
   this->properties.declare
      ( "replacement_method", 
        "Controls how points from the previous population are replaced by the "
        "newly generated points.  If more than (population_size-keep_num) "
        "new points are generated, then the best of these points are used "
        "for replacement.\n"
        "   random   - new points replace randomly selection points "
        "in the previous population\n"
        "   elitist  - new points replace the (popsize-keep_num) worst "
        "points in the previous population\n"
        "   chc      - the keep_num best individuals from BOTH the "
        "previous population and the new points are kept, and the "
        "remaining individuals are randomly selected from both sets of "
        "points to fill the (popsize-keep_num) remaining slots\n"
        "exponential - new points replace the worst points in the "
        "previous population with a probability that decreases "
        "exponentially as their rank increases",
        utilib::Privileged_Property(replacement_str) );
   //this->option.add("replacement_method", replacement_str,
   //                 "Controls how points from the previous population are replaced by the newly generated points.  If more than (population_size-keep_num) new points are generated, then the best of these points are used for replacement.\n \
   // random      - new points replace randomly selection points in the previous population\n \
   // elitist     - new points replace the (popsize-keep_num) worst points in the previous population\n \
   // chc         - the keep_num best individuals from BOTH the previous population and the new points are kept, and the remaining individuals are randomly selected from both sets of points to fill the (popsize-keep_num) remaining slots\n \
   // exponential - new points replace the worst points in the previous population with a probability that decreases exponentially as their rank increases");
   //this->option.categorize("replacement_method", "Selection/Replacement");
   //this->option.alias("replacement_method","replacement_type");

   num_trial_points=0;
   this->properties.declare
      ( "num_trial_points", 
        "The number of new trial points created in each iteration",
        utilib::Privileged_Property(num_trial_points) );
   //this->option.add("num_trial_points",num_trial_points,
   //                 "The number of new trial points created in each iteration");
   //this->option.categorize("num_trial_points", "Selection/Replacement");

   this->keep_num=1;
   this->properties.declare
      ( "keep_num", 
        "The number of best points kept in chc and elitist replacement",
        utilib::Privileged_Property(keep_num) );
   //this->option.add("keep_num",keep_num,
   //                 "The number of best points kept in chc and elitist replacement");
   //this->option.categorize("keep_num", "Selection/Replacement");

   exp_replacement_factor=0.5;
   this->properties.declare
      ( "exponential_replacement_factor", 
        "The factor used to define the exponential decay of probabilities "
        "in exponential replacement.  Factors near 1.0 provide a uniform "
        "selection probability.",
        utilib::Privileged_Property(exp_replacement_factor) );
   //this->option.add("exponential_replacement_factor", exp_replacement_factor,
   //                 "The factor used to define the exponential decay of probabilities in exponential replacement.  Factors near 1.0 provide a uniform selection probability.");
   //this->option.categorize("exponential_replacement_factor", "Selection/Replacement");
}


template <class PointT, class DomainT, class ProblemT>
PEAbase<PointT,DomainT,ProblemT>::~PEAbase()
{ }


template <class PointT, class DomainT, class ProblemT>
void PEAbase<PointT,DomainT,ProblemT>::generate_new_solutions()
{
selection_obj.initialize(this->pop_array, 
		EAindividual_array_accessor<PointT,DomainT>(true),
		EAindividual_array_compare<PointT,DomainT>(true));

//
// Reset parent counters in initial population
//
for (unsigned int i=0; i<popsize_; i++) {
  this->population(i).parent1_id = i;
  this->population(i).parent2_id = -1;
  this->population(i).gen_status = 0;
  }

//
// Compute the number of xovers
//
double tmp = this->xover_rate*num_trial_points;
double fraction = tmp - std::floor(tmp);
unsigned int num_xovers;
if (fraction == 0.0)
   num_xovers = (unsigned int) tmp;
else
   num_xovers = (unsigned int) 
		(this->rnd() > fraction ?  std::floor(tmp) : std::ceil(tmp));

#ifdef ACRO_VALIDATING
int counter_both_ls=0;
int counter_one_ls=0;
#endif
if (num_xovers > 0) {
   //
   // Select xover parents
   //
   DEBUGPR(3, map << -1);
   selection_obj.perform_selection(map, 2*num_xovers);
   subshuffle(map,&(this->rng),0,2*num_xovers);
   DEBUGPR(3, ucout << "num_xoversp: " << num_xovers << std::endl);
   DEBUGPR(3, ucout << "Xover Map: " << map << std::endl);

   //
   // Generate xovers
   //
   for (unsigned int i=0; i<num_xovers; i++) {
#ifdef ACRO_VALIDATING
     DEBUGPR(1000,
        if (this->population(map[(size_t)2*i]).ls_flag && this->population(map[(size_t)2*i+1]).ls_flag)
           counter_both_ls++;
        else if (this->population(map[(size_t)2*i]).ls_flag || this->population(map[(size_t)2*i+1]).ls_flag)
           counter_one_ls++;
             );
#endif

     OUTPUTPR(3,
              ucout << "(Individual #" << i << ": Xover from " 
                    << map[2*i] << " and "
                    << map[2*i+1] << std::endl
              );
     this->apply_xover(this->population(map[(size_t)2*i]), 
	      this->population(map[(size_t)2*i+1]),
              this->trial(i));
     if (this->trial(i).eval_flag == ON) {
        this->trial(i).parent1_id = map[(size_t)2*i];
        this->trial(i).parent2_id = map[(size_t)2*i+1];
        this->trial(i).gen_status = 2;
	}
     else {
	if (this->trial(i) == this->population(map[(size_t)2*i]))
           this->trial(i).parent1_id = map[(size_t)2*i];
	else
           this->trial(i).parent1_id = map[(size_t)2*i+1];
        this->trial(i).gen_status = 0;
	}
     OUTPUTPR(3, ucout << ")" << std::endl);
     }
   }

//
// Select and copy remaining solutions
//
if (num_xovers < num_trial_points) {
   selection_obj.perform_selection(map,num_trial_points-num_xovers);
   for (unsigned int i=num_xovers; i<num_trial_points; i++) {
     OUTPUTPR(3,
              ucout << "(Individual #" << i << ": Copied from " 
                    << map[i-num_xovers] << std::endl
              );
     this->trial(i).copy(this->population(map[(size_t)i-num_xovers]));
     //this->trial(i).parent1_id = map[i-num_xovers];
     //this->trial(i).parent2_id = -1;
     //this->trial(i).gen_status = 0;
     OUTPUTPR(3, ucout << ")" << std::endl);
     }
   }

//
// Compute the number of mutations
//
tmp = mutation_rate*num_trial_points;
fraction = tmp - std::floor(tmp);
int num_mutations;
if (fraction == 0)
   num_mutations = (int) tmp;
else
   num_mutations = (int) (this->rnd()>fraction ?  std::floor(tmp) : std::ceil(tmp));

if (num_mutations > 0) {
   //
   // Select mutation points
   //
   for (unsigned int j=0; j<trial_order.size(); j++)   // trial_order is num_trial_points long
     trial_order[j] = j;
   shuffle(trial_order,&(this->rng),num_mutations);   // 1st num_mut elements are random
   
   //
   // Generate mutations
   //
   for (int i=0; i<num_mutations; i++) {
     OUTPUTPR(3,
              ucout << "(Individual #" << trial_order[i] 
                    << ": Mutation" << std::endl
              );
     bool status = this->apply_mutation(this->trial(trial_order[i]), 
		(trial_order[i] >= (int)num_xovers ? map[(size_t)trial_order[i]-num_xovers] : -1));
     if (status) {
        this->trial(trial_order[i]).eval_flag == ON;
        this->trial(trial_order[i]).gen_status += 1;
        }
     OUTPUTPR(3, ucout << ")" << std::endl);
     }
   }


#ifdef ACRO_VALIDATING
DEBUGPR(1000,if (ls_freq != 0.0) {
   ucout << "(Begin-Ops:" << std::endl;
   ucout << "[Iter:\t" << curr_iter << " ]" << std::endl;
   ucout << "[Num-Both-Parents-From-LS:\t" << counter_both_ls << " ]" << std::endl;
   ucout << "[Num-One--Parents-From-LS:\t" << counter_one_ls << " ]" << std::endl;
   ucout << ")" << std::endl;
   }
        );
#else
   DEBUGPR(1000,
           if (ls_freq != 0.0) {
              ucout << "(Begin-Ops:" << std::endl;
              ucout << "[Iter:\t" << curr_iter << " ]" << std::endl;
              ucout << ")" << std::endl;
           }
           );
#endif
}



template <class PointT, class DomainT, class ProblemT>
void PEAbase<PointT,DomainT,ProblemT>::perform_replacement()
{
//
// Get the order of the new trial points
//
order(trial_order,trial_array,EAindividual_array_compare<PointT,DomainT>(true));
OUTPUTPR(4, ucout << "Trial Order: " << trial_order << '\n');


switch (replacement_flag) {

  //
  // num_trial_points individuals are created, of which the best
  // popsize-keep_num replace the same number of individuals randomly 
  // selected from the previous population.
  //
  case RANDOM_REPLACEMENT:
	{
	shuffle(pop_order,&(this->rng));

	for (unsigned int i=0; i<std::min(trial_array.size(),(pop_array.size()-keep_num)); i++)
	  std::swap(pop_array[pop_order[i+keep_num]],trial_array[trial_order[i]]);
        OUTPUTPR(4, ucout << "Pop Order: " << pop_order << '\n');
	}
	break;

  //
  // num_trial_points new individuals are created, of which the best
  // replace the popsize-keep_num worst individuals in the
  // previous population.
  //
  case ELITIST_REPLACEMENT:
	{
        order(pop_order,pop_array,EAindividual_array_compare<PointT,DomainT>(true));
        OUTPUTPR(4, ucout << "Elitist Order: " << pop_order << '\n');

        //
        // If we have done fewer trial points than popsize - keep_num
        // then we subshuffle the remaining points to randomize our replacement.
        //
        if (num_trial_points < (popsize_-keep_num))
           subshuffle(pop_order,&(this->rng),keep_num,popsize_);
        //
        // Replace the non-elite points
        //
        for (unsigned int i=0; i<std::min(trial_array.size(),(pop_array.size()-keep_num)); i++)
          std::swap(pop_array[pop_order[(size_t)i+keep_num]], trial_array[trial_order[i]]);
	}
	break;

  //
  // num_trial_points new individuals are created, of which the best
  // replace the popsize-keep_num worst individuals in the
  // previous population.
  //
  case EXPONENTIAL_REPLACEMENT:
	{
        order(pop_order,pop_array,EAindividual_array_compare<PointT,DomainT>(true));
        OUTPUTPR(4, ucout << "Exponential Order: " << pop_order << '\n');

	size_type i = static_cast<size_type>(keep_num);
	size_type j = 0;
	int ctr=0;
	int total=(popsize_-keep_num);
	double sum=0.0, ptr=this->rnd();
	map << 0;
        double factor = exp_replacement_factor;
	while ((ctr < total) && 
	       ((i < pop_order.size()) || (j < trial_order.size()))) {
	  bool flag = (((i == pop_order.size()) && ((j+1) == trial_order.size()))
			||
	              (((i+1) == pop_order.size()) && (j == trial_order.size())));
	  if ((i < pop_order.size()) && ((j == trial_order.size()) ||
	      (pop_array[pop_order[i]]->Sel_Val < trial_array[trial_order[j]]->Sel_Val))) {
	     sum += total*factor;
             if (sum>ptr || flag) {
                ptr++;
                map[pop_order[i]] = 1;
                ctr++;
	        }
	     i++;
	     }
	  else {
	     sum += total*factor;
             if ((sum>ptr) || flag) {
                ptr++;
                map[pop_order.size()+trial_order[j]] = 1;
                ctr++;
	        }
	     j++;
	     }
          factor *= exp_replacement_factor;
	  }
        if (ctr < total)
	   EXCEPTION_MNGR(std::runtime_error,"Exponential Replacement didn't select enough individuals");

        j=keep_num;
        for (i=keep_num; i<pop_order.size(); i++)
          if (map[pop_order[i]]==1)
             std::swap(pop_array[j++],pop_array[pop_order[i]]);
        for (i=0; i<trial_order.size(); i++)
          if (map[pop_order.size()+trial_order[i]]==1)
             std::swap(pop_array[j++],trial_array[trial_order[i]]);
	}
	break;

  //
  // num_trial_points individuals are created (defaults to popsize-keep_num)
  // The keep_num best individuals from BOTH populations are
  // kept, and the remaining individuals are randomly selected from
  // both populations to fill the popsize-keep_num remaining slots.
  //
  case CHC_REPLACEMENT:
        {
        order(pop_order,pop_array,EAindividual_array_compare<PointT,DomainT>(true));
        OUTPUTPR(3, ucout << "Pop Order:   " << pop_order << "\n" );
        //
        // Identify the best individuals
        //
        unsigned int tn=0, n=0;
        for (unsigned int i=0; i<keep_num; i++) {
          if ((tn >= num_trial_points) ||
              ((n < popsize_) && 
	       ( pop_array[(size_t)pop_order[n]]->Sel_Val <=
                trial_array[trial_order[tn]]->Sel_Val)) ) {
             n++;
             }
          else {
             tn++;
             }
          }
        //
        // Swap the best individuals into the population
        //
        unsigned int j=0;
        for (unsigned int i=n; i<keep_num; i++, j++)
          std::swap(pop_array[pop_order[i]], trial_array[trial_order[j]]);
        //
        // Randomly select the remaining solutions
        // 
        if ((keep_num < popsize_) && (tn < trial_array.size())) {
           if ((num_trial_points-tn) < (popsize_-keep_num))
              subshuffle(pop_order,&(this->rng),keep_num,popsize_);
           double frac = (popsize_-keep_num)/
                                ((double)((popsize_-keep_num)+(num_trial_points-tn)));
           j=tn;
           for (unsigned int i=keep_num; (i<popsize_) && (j<trial_array.size()); i++) {
             if (this->rnd() >= frac) {
                std::swap(pop_array[pop_order[i]],trial_array[trial_order[j]]);
                j++;
                }
             }
           }
        }
	break;
  }
}




template <class PointT, class DomainT, class ProblemT>
void PEAbase<PointT,DomainT,ProblemT>::write(std::ostream& os) const
{
EAbase<PointT,DomainT,ProblemT>::write(os);

os << "##\n## PEAbase: Selection Information\n##\n";

os << "#\n# Replacement Info\n#\n";
os << "replacement_flag   ";
switch(replacement_flag) {
  case RANDOM_REPLACEMENT:	
	os << "random" << std::endl; 
	os << "\tgenerating " << num_trial_points << " new points" << std::endl;
	os << "\trandomly replace " << (popsize_ - keep_num) << " out of " 
		<< popsize_ << " points in previous population" << std::endl;
	break;

  case CHC_REPLACEMENT:		
	os << "chc" << std::endl;
	os << "\tgenerating " << num_trial_points << " new points" << std::endl;
	os << "\tkeep the " << keep_num << " best points from new+old points" << std::endl;
	os << "\trandomly selection " << (popsize_ - keep_num) << " points from new+old-selected" << std::endl;
	break;

  case ELITIST_REPLACEMENT:	
	os << "elitist" << std::endl; 
	os << "\tgenerating " << num_trial_points << " new points" << std::endl;
	os << "\treplace " << (popsize_ - keep_num) << " worst points in previous population" << std::endl;
	break;

  case EXPONENTIAL_REPLACEMENT:	
	os << "exponential" << std::endl; 
	os << "\tgenerating " << num_trial_points << " new points" << std::endl;
	os << "\titeratively tries to replace points in previous population with probability " << exp_replacement_factor << ", from worst to best" << std::endl;
	break;
  };
os << "#\n# Selection Object\n#\n";
os << selection_obj;
}


template <class PointT, class DomainT, class ProblemT>
void PEAbase<PointT,DomainT,ProblemT>::read(std::istream& is)
{
std::string temp;
is >> temp;
is >> selection_obj;

EAbase<PointT,DomainT,ProblemT>::read(is);
}


template <class PointT, class DomainT, class ProblemT>
void PEAbase<PointT,DomainT,ProblemT>::reset_impl()
{
if ((popsize_ > 0) && (popsize_ < keep_num))
   keep_num=popsize_;
if ((popsize_ > 0) && (num_trial_points == 0)) {
   if (popsize_ > keep_num)
      num_trial_points = popsize_ - keep_num;
   else
      num_trial_points = 1;
   }

base_t::reset_impl();
selection_obj.set_rng(&(this->rng));

if (popsize_ != pop_array.size())
   init_population(popsize_);

if (popsize_ != 0) {
   if (keep_num > popsize_)
      EXCEPTION_MNGR(std::runtime_error, "PEAbase::reset - the parameter 'keep_num' must be no greater than the population size");

   pop_order.resize(popsize_);
   for (size_t i=0; i<pop_order.size(); i++)
     pop_order[i] = i;
   trial_order.resize(num_trial_points);
   for (size_t i=0; i<trial_order.size(); i++)
     trial_order[i] = i;

   map.resize(std::max(num_trial_points*2,num_trial_points+popsize_));

   if (replacement_str == "random") {
      replacement_flag=0;
      }
   else if (replacement_str == "chc") {
      replacement_flag=1;
      }
   else if (replacement_str == "elitist")
      replacement_flag = 2;
   else if (replacement_str == "exponential")
      replacement_flag = 3;
   else
      EXCEPTION_MNGR(std::runtime_error, "PEAbase::reset - bad replacement method: \"" << replacement_str << "\"");
   }

selection_obj.debug=this->debug;
selection_obj.reset();
}

} // namespace scolib

#endif

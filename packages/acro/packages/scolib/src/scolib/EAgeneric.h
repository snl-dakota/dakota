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
 * \file EAgeneric.h
 *
 * Defines the scolib::EAgeneric class.
 */

#ifndef scolib_EAgeneric_h
#define scolib_EAgeneric_h

#include <acro_config.h>
#include <scolib/EApoint.h>
#include <scolib/PEAbase.h>

namespace scolib {


///
template <class DomainT, class ProblemT, class InfoT, class OpClassT>
class EAgeneric_name
{
protected:

  ///
  const char* get_name() const { return "EAgeneric"; }
};

///
/// An evolutionary algorithm with a generic problem representation
///
template <class DomainT, class ProblemT, class InfoT, class OpClassT>
class EAgeneric 
   : public PEAbase<EApoint<DomainT,InfoT>, DomainT, ProblemT >, 
     public EAgeneric_name<DomainT,ProblemT,InfoT,OpClassT>
{
  #if !defined(DOXYGEN)
  typedef EApoint<DomainT,InfoT > point_t;
  typedef PEAbase<point_t,DomainT,ProblemT> base_t;
  #if !defined(SOLARIS)
  using base_t::popsize_;
  #endif
  #endif

protected:
   ///
   std::string define_solver_type() const
   { return this->get_name(); }

public:

   ///
   EAgeneric()
      : PEAbase<EApoint<DomainT,InfoT>, DomainT, ProblemT >()
   {
      search_ops = new OpClassT(this->properties);
#if 0
      ParameterSet::create_parameter
         ( "distinct_initial_population",
           init_unique, "<bool>", "true",
           "If true, then the initial population is composed of distinct "
           "points");
#endif
      //augment_parameters(*search_ops);

      this->reset_signal.connect(boost::bind(&EAgeneric::cb_reset, this));
   }

  ///
  void optimize()
	{
	if (search_ops->check_domain(this->problem,this->best().termination_info)) 
	   return;
	base_t::optimize();
	}

  ///
  void write(std::ostream& os) const
	{
	base_t::write(os);
	search_ops->write(os);
	}

protected:

  ///
  OpClassT* search_ops;

  ///
  void initialize_best_point()
	{ search_ops->initialize_point(this->best().point, best_point_info); }

  ///
  void set_op_rng(utilib::AnyRNG& rng_)
	{
	search_ops->set_rng(rng_);
	}

  ///
  void randomize()
	{
	unsigned int i=0;
	//
	// Read up to popsize_ elements from this file
	//
	if (this->init_filename != "") {
   	   std::ifstream ifstr(this->init_filename.c_str());
   	   if (ifstr) {
      	      search_ops->read_point(ifstr,this->population(i).pt.point);
	      if (ifstr) {
       	         this->population(i).reset();
                 i++;
		 }
      	      while (i < popsize_) {
      	        search_ops->read_point(ifstr,this->population(i).pt.point);
		if (!ifstr) break;
	        this->population(i).reset();
       	        i++;
       	        }
      	      }
	   else
    	      EXCEPTION_MNGR(std::runtime_error, "EAgeneric::randomize "
                             "- bad initial point file name.");
	   DEBUGPR(1,ucout << "Initial population initialized with " << i 
                   << " points from the file " << this->init_filename.c_str()
                   << std::endl);
   	   }

        unsigned int loaded_pts = i;
        colin::Cache::iterator c_it 
           = this->initial_points->begin(this->problem);
        while ( i < popsize_ && c_it != this->initial_points->end() )
        {
           colin::AppResponse response = c_it->second.asResponse(this->problem);
           utilib::Any domain = response.get_domain();
           if ( response.is_computed(colin::f_info, this->problem) )
              this->population(i).update(response, domain);
           else
           {
              utilib::TypeManager()->lexical_cast(domain, this->population(i).pt.point);
              this->population(i).reset();
           }
           ++c_it;
           ++i;
        }
        
        if ( c_it != this->initial_points->end() )
        {
           ucerr << std::endl
                 << "WARNING: the " << this->get_name()
                 << " has a population size of " << popsize_ 
                 << ", but was passed ";
           if ( loaded_pts )
              ucerr << loaded_pts << " points from an input file and ";
           ucerr << this->initial_points->size(this->problem)
                 << " initial points from the intersolver cache." << std::endl
                 << std::endl;
        }

	//
	// Generate remaining points randomly, possibly testing for uniqueness
	//
	// TODO - pass an entire array of points.  This would facilitate more
	// effective randomization strategies.
	///
	for (; i<popsize_; i++) {
	  int ctr=0;
	  bool flag = false;
	  while (!flag) {
	    search_ops->randomize( this->population(i).pt.point, this->population(i).pt.info );
	    if ((flag == 0) || !init_unique || (ctr == 20)) {
	       flag=true;
	       continue;
	       }
	    flag=true;
            for (unsigned int k=0; k<i; k++)
              if (this->population(i).pt.point == this->population(k).pt.point) {
                 flag=false;
                 break;
                 }
	    ctr++;
	    }
	  this->population(i).reset();
	  }
	}

  ///
  void apply_xover(EAindividual<point_t,DomainT>& parent1,
		   EAindividual<point_t,DomainT>& parent2,
                   EAindividual<point_t,DomainT>& child)
	{
	if (parent1==parent2) {
	   child.copy(parent1);
	   return;
           }

	point_t& p1 = parent1.pt;
	point_t& p2 = parent2.pt;
	point_t& c1 = child.pt;

	int status = search_ops->apply_xover(p1.point,p1.info,
				p2.point,p2.info,c1.point,c1.info);
	OUTPUTPR(4, search_ops->write(ucout) );

        if (status == 1) 
           child.copy(parent1);
        else if (status == 2)
           child.copy(parent2);
        else
           child.reset();
	}

  ///
  bool apply_mutation(EAindividual<point_t,DomainT>& point, int parent_ndx)
	{
	point_t& p1 = point.pt;
	bool status = search_ops->apply_mutation(p1.point,p1.info, parent_ndx);
	OUTPUTPR(4, search_ops->write(ucout) );
        point.reset();
	return status;
#if 0
        //
        // Print the progress of the best point in the population
        //
        if (eval_flag &&
            (Debug_search_steps) && 
            (parent_ndx != -1) && (parent_ndx < num_min)) {
           ucout << "[" << endl << "Curr_best_point: ";
           population(parent_ndx).write_point(ucout);
           ucout << "  New_point: ";
           point.write_point(ucout);
           ucout << endl << "]" << endl;
           }
#endif
	}

  ///
  InfoT best_point_info;

  ///
  bool init_unique;

  ///
  void init(const unsigned int popsize)
	{
	EAbase<EApoint<DomainT,InfoT>, DomainT,ProblemT>::init(popsize);
	for (unsigned int i=0; i<this->pop_array.size(); i++)
	  search_ops->initialize_point( this->pop_array[i]->pt.point, 
					this->pop_array[i]->pt.info );
	for (unsigned int i=0; i<this->trial_array.size(); i++)
	  search_ops->initialize_point( this->trial_array[i]->pt.point, 
					this->trial_array[i]->pt.info );
        search_ops->initialize_point( this->min_pt.pt.point,
				      this->min_pt.pt.info);
        search_ops->initialize_point( this->true_min_pt.pt.point,
				      this->true_min_pt.pt.info);
        search_ops->initialize_point( this->max_pt.pt.point,
				      this->max_pt.pt.info);
        search_ops->initialize_point( this->true_max_pt.pt.point,
				      this->true_max_pt.pt.info);
	}

private:
   ///
   void cb_reset()
   {
      if ( this->problem.empty() ) 
         return;
      if ( search_ops->check_domain( this->problem, 
                                     this->best().termination_info ) )
         return;
      if ( popsize_ == 0 )
         popsize_ = 100;
      search_ops->debug = this->debug;
      // Resize the best point
      search_ops->initialize(this->problem,popsize_, this->initial_point, 
                             this->xover_rate, this->mutation_rate);
      base_t::reset_impl(); 
      if (this->xover_rate == 0) {
         this->properties["binary_xover_type"] = std::string("none");
         this->properties["intarray_xover_type"] = std::string("none");
         this->properties["realarray_xover_type"] = std::string("none");
         //this->option.set_parameter("binary_xover_type","none");
         //this->option.set_parameter("intarray_xover_type","none");
         //this->option.set_parameter("realarray_xover_type","none");
      }
      search_ops->reset();
      this->init_population(popsize_);
      randomize();
   }

};

} // namespace scolib

#endif

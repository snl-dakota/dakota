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
 * \file DomainOpsMixedInteger.h
 *
 * Defines the scolib::DomainOpsMixedInteger class.
 */

#ifndef scolib_DomainOpsMixedInteger_h
#define scolib_DomainOpsMixedInteger_h

#include <acro_config.h>
#include <utilib/MixedIntVars.h>
#include <scolib/DomainOpsIntArray.h>
#include <scolib/DomainOpsRealArray.h>
#include <scolib/DomainOpsBinary.h>

namespace scolib {


///
template <class InfoT>
class DomainOpsMixedInteger : public utilib::CommonIO
{
public:

  ///
  typedef utilib::MixedIntVars point_t;

  ///
  typedef InfoT info_t;

  ///
  DomainOpsMixedInteger(utilib::PropertyDict& properties);

  ///
  template <class ProblemT>
  bool check_domain(ProblemT& problem, std::string& termination_info)
  {
    if ( ( problem->num_real_vars.template as<size_t>() + 
           problem->num_int_vars.template as<size_t>() > 0 )
         && ! problem->finite_bound_constraints() )
    {
       termination_info = "Missing-Bound-Constraints";
       return true;
    }
    return false;
  }

  ///
  template <class ProblemT>
  void initialize(ProblemT& problem, unsigned int popsize_, point_t& best_pt, double xover_rate, double m_rate)
	{
	binary_ops.initialize(problem,popsize_,xover_rate,m_rate);
	intarray_ops.initialize(problem,popsize_,xover_rate,m_rate);
	realarray_ops.initialize(problem,popsize_,xover_rate,m_rate);
        best_pt.resize(problem->num_binary_vars, problem->num_int_vars,
			problem->num_real_vars);
	num_ints = problem->num_int_vars;
	num_binary = problem->num_binary_vars;
	num_reals = problem->num_real_vars;
	num_vars = num_reals + num_ints + num_binary;
	}

  ///
  void reset()
	{
	binary_ops.debug = debug;
	intarray_ops.debug = debug;
	realarray_ops.debug = debug;
	binary_ops.reset();
	intarray_ops.reset();
	realarray_ops.reset();
	}

  ///
  void write(std::ostream& os) const
	{
	binary_ops.write(os);
	intarray_ops.write(os);
	realarray_ops.write(os);
	}

  ///
  void read_point(std::istream& istr, point_t& point)
	{
	// Read the binary values
	point.Binary().reset();
	for (unsigned int i=0; i<point.Binary().size(); i++) {
	  int tmp;
          istr >> tmp;
	  if (!istr) return;
	  if (tmp) point.Binary().set(i);
	  else     point.Binary().reset(i);
	  }
	// Read the integer values
	for (unsigned int i=0; i<point.Integer().size(); i++) {
          istr >> point.Integer()[i];
	  if (point.Integer()[i] < intarray_ops.lower[i])
              point.Integer()[i] = intarray_ops.lower[i];
	  if (point.Integer()[i] > intarray_ops.upper[i])
              point.Integer()[i] = intarray_ops.upper[i];
          }
	// Read the double values
	for (unsigned int i=0; i<point.Real().size(); i++) {
          istr >> point.Real()[i];
	  if (point.Real()[i] < realarray_ops.lower[i])
              point.Real()[i] = realarray_ops.lower[i];
	  if (point.Real()[i] > realarray_ops.upper[i])
              point.Real()[i] = realarray_ops.upper[i];
	  }
	}

  ///
  void initialize_point(point_t& point, info_t& info)
	{
	binary_ops.initialize_point(point.Binary(), info);
	intarray_ops.initialize_point(point.Integer(), info);
	realarray_ops.initialize_point(point.Real(), info);
	}

  ///
  void randomize(point_t& point, info_t& info)
	{
	binary_ops.randomize(point.Binary(), info);
	intarray_ops.randomize(point.Integer(), info);
	realarray_ops.randomize(point.Real(), info);
	}

  ///
  int apply_xover(point_t& parent1, info_t& info1,
		  point_t& parent2, info_t& info2,
		  point_t& child,   info_t& info_c)
	{
	int bval = binary_ops.apply_xover(parent1.Binary(), info1,
			       parent2.Binary(), info2,
			       child.Binary(),   info_c);
	int ival = intarray_ops.apply_xover(parent1.Integer(), info1,
			       parent2.Integer(), info2,
			       child.Integer(),   info_c);
	int rval = realarray_ops.apply_xover(parent1.Real(), info1,
			       parent2.Real(), info2,
			       child.Real(),   info_c);
	if ((bval == 3) || (ival == 3) || (rval == 3)) return 3;
	if ((bval == ival) && (ival == rval)) return bval;
	return 3;
	}

  ///
  bool  apply_mutation(point_t& point, info_t& info, int parent_ndx)
	{
	if (mutate_independently) {
	   double tmp = num_vars*rnd();
	   if (tmp < num_binary)
	      return binary_ops.apply_mutation(point.Binary(),info,parent_ndx);
	   else if (tmp < (num_binary+num_ints))
	      return intarray_ops.apply_mutation(point.Integer(),info,parent_ndx);
	   else
	      return realarray_ops.apply_mutation(point.Real(),info,parent_ndx);
	   }
	bool beval, ieval, reval;
	beval = binary_ops.apply_mutation(point.Binary(),info,parent_ndx);
	ieval = intarray_ops.apply_mutation(point.Integer(),info,parent_ndx);
	reval = realarray_ops.apply_mutation(point.Real(),info,parent_ndx);
        OUTPUTPR(1000,
                 ucout << "beval " << beval << " ieval " << ieval 
                       << " reval " << reval << std::endl;
                 );
	return (beval | ieval | reval);
	}

  ///
  void set_rng(utilib::AnyRNG& rng_)
	{
	rng = rng_;
	rnd.generator(&rng_);
	binary_ops.set_rng(rng_);
	intarray_ops.set_rng(rng_);
	realarray_ops.set_rng(rng_);
	}

protected:

  ///
  DomainOpsBinary<InfoT>    binary_ops;

  ///
  DomainOpsIntArray<InfoT>  intarray_ops;

  ///
  DomainOpsRealArray<InfoT> realarray_ops;

  ///
  utilib::AnyRNG rng;

  ///
  utilib::Uniform rnd;

  ///
  bool mutate_independently;

  unsigned int num_binary;
  unsigned int num_ints;
  unsigned int num_reals;
  unsigned int num_vars;
};


template <class InfoT>
DomainOpsMixedInteger<InfoT>::
DomainOpsMixedInteger(utilib::PropertyDict& properties) :
  binary_ops(properties),
  intarray_ops(properties),
  realarray_ops(properties),
  mutate_independently(false)
{
   properties.declare( "mint_mutate_independently", 
                       "If true, then independently mutate binary, integer "
                       "and real arrays",
                       utilib::Privileged_Property(mutate_independently) );
   //option.add("mint_mutate_independently", mutate_independently,
   //        "If true, then independently mutate binary, integer and real arrays");
   //option.categorize("mint_mutate_independently", "Mutation");
}

}

#endif

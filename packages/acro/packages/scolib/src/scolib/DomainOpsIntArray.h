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
 * \file DomainOpsIntArray.h
 *
 * Defines the scolib::DomainOpsIntArray class.
 */

#ifndef scolib_DomainIntOpsArray_h
#define scolib_DomainIntOpsArray_h

#include <acro_config.h>
#include <scolib/DomainOpsArray.h>
#include <colin/BoundTypeArray.h>
#include <utilib/_math.h>
#include <utilib/Uniform.h>

#define INT_ARRAY_MUTATION_UNIFORM  1
#define INT_ARRAY_MUTATION_INTERVAL 2

namespace scolib {

template <class InfoT>
class DomainOpsIntArray : public DomainOpsArray<int, InfoT>
{
public:

  ///
  typedef typename DomainOpsArray<int,InfoT>::info_t  info_t;

  ///
  typedef typename DomainOpsArray<int,InfoT>::point_t point_t;

  ///
  DomainOpsIntArray(utilib::PropertyDict& properties);

  ///
  void reset();

  ///
  template <class ProblemT>
  void initialize(ProblemT& problem, unsigned int popsize_,double xover_rate, double m_rate)
	{
	DomainOpsArray<int,InfoT>::initialize( problem->num_int_vars, 
                                               popsize_, xover_rate, m_rate);
        if ((problem->num_int_vars > 0) && 
	    problem->enforcing_domain_bounds.template as<bool>()) {
           lower = problem->int_lower_bounds;
           upper = problem->int_upper_bounds;
	   lbound_type = problem->int_lower_bound_types;
	   ubound_type = problem->int_upper_bound_types;
	   range.resize(lower.size());
	   for (unsigned int i=0; i<lower.size(); i++) {
	     range[i] = upper[i]-lower[i];
             }
           }
	}

  ///
   void initialize_point(point_t& point, info_t& /*info*/)
        { point.resize(lower.size()); }

  ///
   void randomize(point_t& point, InfoT& /*info*/)
	{
	for (unsigned int j=0; j<this->nvars; j++)
          point[j] = utilib::Discretize(this->rnd(),lower[j],upper[j]);
	}

  ///
  point_t lower;

  ///
  point_t upper;

  ///
  point_t range;

  ///
  colin::BoundTypeArray lbound_type;

  ///
  colin::BoundTypeArray ubound_type;

protected:

  ///
  int mutation_range;

  ///
  void mutate_value(int i, int& val, info_t& info);

};



template <class InfoT>
DomainOpsIntArray<InfoT>::DomainOpsIntArray(utilib::PropertyDict& properties)
{
   this->crossover_blocksize=1;
   properties.declare
      ( "intarray_xover_blocksize", 
        "Block size used with array-based two-point and uniform crossover",
        utilib::Privileged_Property(this->crossover_blocksize) );
   //option.add("intarray_xover_blocksize", this->crossover_blocksize,
   //           "Block size used with array-based two-point and uniform crossover");
   //option.categorize("intarray_xover_blocksize","Crossover");

   this->crossover_str="twopoint";
   properties.declare
      ( "intarray_xover_type", 
        "Crossover type:\n"
        "   onepoint - standard one-point mutation\n"
        "   twopoint - standard two-point mutation\n"
        "   uniform  - standard uniform mutation",
        utilib::Privileged_Property(this->crossover_str) );
   //option.add("intarray_xover_type", this->crossover_str,
   //           "Crossover type\n                       \
   // onepoint      - standard one-point mutation\n  \
   // twopoint      - standard two-point mutation\n  \
   // uniform       - standard uniform mutation");
   //option.categorize("intarray_xover_type","Crossover");

   this->mutation_str = "uniform";
   properties.declare
      ( "intarray_mutation_type", 
        "Integer mutation type:\n"
        "   uniform  - replace the value with a uniformly random variable\n"
        "   interval - replace the value with a uniform value in a local interval",
        utilib::Privileged_Property(this->mutation_str) );
   //option.add("intarray_mutation_type", this->mutation_str,
   //           "Integer mutation type\n                                \
   // uniform  - replace the value with a uniformly random variable\n \
   // interval - replace the value with a uniform value in a local interval");
   //option.categorize("intarray_mutation_type","Mutation");

   this->mutation_range=1;
   properties.declare
      ( "intarray_mutation_range", 
        "Range of mutation used for 'interval' mutation",
        utilib::Privileged_Property(this->mutation_range) );
   //option.add("intarray_mutation_range", this->mutation_range,
   //           "Range of mutation used for 'interval' mutation");
   //option.categorize("intarray_mutation_range","Mutation");

   this->mutate_independently=false;
   properties.declare
      ( "intarray_mutate_independently", 
        "If true, then only mutate a single dimension.  Note that if this "
        "value is true, then a single dimension is always mutated, so the "
        "mutation allele rate is ignored.",
        utilib::Privileged_Property(this->mutate_independently) );
   //option.add("intarray_mutate_independently", this->mutate_independently,
   //           "If true, then only mutate a single dimension.  Note that if this value is true, then a single dimension is always mutated, so the mutation allele rate is ignored.");
   //option.categorize("intarray_mutate_independently","Mutation");

   this->mutation_allele_rate=1.0;
   properties.declare
      ( "intarray_mutation_allele_rate", 
        "The probability that any given dimension of the intarray is "
        "mutated given that the individual is mutated",
        utilib::Privileged_Property(this->mutation_allele_rate) );
   //option.add("intarray_mutation_allele_rate", this->mutation_allele_rate,
   //           "The probability that any given dimension of the intarray is mutated given that the individual is mutated");
   //option.categorize("intarray_mutation_allele_rate","Mutation");
}



template <class InfoT>
void DomainOpsIntArray<InfoT>::reset()
{
DomainOpsArray<int,InfoT>::reset();

if ((this->mutation_str == "uniform") || (this->mutation_str == "offset_uniform")) {
   this->mutation_type = INT_ARRAY_MUTATION_UNIFORM;
   this->mutation_allele_rate =
      (this->mutation_allele_rate < 0.0 ? std::sqrt(M_E/(double)this->nvars)/(double)this->popsize :
        this->mutation_allele_rate);
   }

else if ((this->mutation_str == "interval") || (this->mutation_str == "replace_uniform")) {
   this->mutation_type = INT_ARRAY_MUTATION_INTERVAL;
   this->mutation_allele_rate =
      (this->mutation_allele_rate < 0.0 ? std::sqrt(M_E/(double)this->nvars)/(double)this->popsize :
        this->mutation_allele_rate);
   }

else 
   EXCEPTION_MNGR(std::runtime_error,"DomainOpsIntArray::reset - bad mutation type: \"" << this->mutation_str << "\".\n\t\tValid types are uniform and interval\n");

if (this->crossover_str == "none")
   this->crossover_type = ARRAY_XOVER_NONE;
else if (this->crossover_str == "twopoint")
   this->crossover_type = ARRAY_XOVER_TWOPOINT;
else if (this->crossover_str == "uniform")
   this->crossover_type = ARRAY_XOVER_UNIFORM;
else
   EXCEPTION_MNGR(std::runtime_error, "DomainOpsIntArray::reset -- bad xover type: \"" << this->crossover_str << "\".\n\t\tValid types are twopoint and uniform\n");
}


template <class InfoT>
void DomainOpsIntArray<InfoT>::mutate_value(int i, int& value, info_t& /*info*/)
{
switch (this->mutation_type) {
  case INT_ARRAY_MUTATION_INTERVAL:
                value = utilib::Discretize(this->rnd(),lower[i],upper[i]);
		break;

  case INT_ARRAY_MUTATION_UNIFORM:
                int tmp = utilib::Discretize(this->rnd(),
					std::max(lower[i],value-mutation_range),
					std::min(upper[i]-1,value+mutation_range-1));
		value = (tmp < value ? tmp : tmp+1);
		break;
  };
if (value > upper[i]) {
   if (ubound_type[i] == colin::hard_bound) value = upper[i];
   else if (ubound_type[i] == colin::periodic_bound)
     while (value > upper[i]) value -= range[i];
   }
if (value < lower[i]) {
   if (lbound_type[i] == colin::hard_bound) value = lower[i];
   else if (lbound_type[i] == colin::periodic_bound)
     while (value < lower[i]) value += range[i];
   }
}

}

#endif

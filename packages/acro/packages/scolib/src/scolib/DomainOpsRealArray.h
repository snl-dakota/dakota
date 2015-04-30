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
 * \file DomainRealOpsArray.h
 *
 * Defines the scolib::DomainOpsRealArray.h class.
 */

#ifndef scolib_DomainRealOpsArray_h
#define scolib_DomainRealOpsArray_h

#include <acro_config.h>
#include <scolib/DomainOpsArray.h>
#include <colin/BoundTypeArray.h>
#include <colin/real.h>
#include <utilib/Normal.h>
#include <utilib/Cauchy.h>

#define REAL_ARRAY_MUTATION_UNIFORM  1
#define REAL_ARRAY_MUTATION_INTERVAL 2
#define REAL_ARRAY_MUTATION_CAUCHY   3
#define REAL_ARRAY_MUTATION_NORMAL   4
#define REAL_ARRAY_MUTATION_STEP     5

#define REAL_ARRAY_XOVER_BLEND       (ARRAY_XOVER_UNIFORM+1)
#define REAL_ARRAY_XOVER_NONE        (ARRAY_XOVER_UNIFORM+2)

namespace scolib {


///
template <class InfoT>
class DomainOpsRealArray : public DomainOpsArray<double,InfoT>
{
  #if !defined(SOLARIS)
  using DomainOpsArray<double,InfoT>::mutation_str;
  using DomainOpsArray<double,InfoT>::crossover_str;
  using DomainOpsArray<double,InfoT>::nvars;
  using DomainOpsArray<double,InfoT>::mutation_type;
  using DomainOpsArray<double,InfoT>::mutation_allele_rate;
  using DomainOpsArray<double,InfoT>::crossover_type;
  using DomainOpsArray<double,InfoT>::popsize;
  #endif

public:

  ///
  typedef typename DomainOpsArray<double,InfoT>::info_t  info_t;

  ///
  typedef typename DomainOpsArray<double,InfoT>::point_t point_t;

  ///
  typedef DomainOpsArray<double,InfoT> base_t;

  ///
   DomainOpsRealArray(utilib::PropertyDict& properties);

  ///
  void reset();

  ///
  void set_rng(utilib::AnyRNG& rng_)
        {
        DomainOpsArray<double,InfoT>::set_rng(rng_);
        cauchy_rnd.generator(&rng_);
        normal_rnd.generator(&rng_);
        }

  ///
  template <class ProblemT>
  void initialize(ProblemT& problem, unsigned int popsize_, double xover_rate, double m_rate)
        {
        DomainOpsArray<double,InfoT>::initialize(problem->num_real_vars,
                                                 popsize_, xover_rate, m_rate);
        if ((problem->num_real_vars > 0) && 
	    problem->enforcing_domain_bounds.template as<bool>()) {
           lower = problem->real_lower_bounds;
           upper = problem->real_upper_bounds;
	   lbound_type = problem->real_lower_bound_types;
	   ubound_type = problem->real_upper_bound_types;
           range.resize(this->nvars);
           for (size_type i=0; i<this->nvars; i++) { 
             range[i] = upper[i] - lower[i];
             }
           }
        tau = 1.0/std::sqrt(2*std::sqrt( static_cast<double>(this->nvars) ));
        taudash = 1.0/std::sqrt( 2.0*(this->nvars) );
        mutation_direction.resize(this->nvars);
        }

  ///
  void initialize_point(utilib::BasicArray<double>& point, info_t& info)
        {
        point.resize(this->nvars);
        info.scale.resize(this->nvars);
        if (auto_rescale)
              info.scale << range;
        else
              info.scale << 1.0;
        }
 
  ///
   void randomize(point_t& point, InfoT& /*info*/)
        {
        for (unsigned int j=0; j<this->nvars; j++)
          point[j] = range[j]*(this->rnd()) + lower[j];
        }

  ///
  bool apply_mutation(point_t& point, info_t& info, int parent_ndx)
        {
        N = normal_rnd();
        return DomainOpsArray<double,InfoT>::apply_mutation(point,info,
                                                parent_ndx);
        }

  ///
  int apply_xover(point_t& parent1, info_t& info1,
                          point_t& parent2, info_t& info2,
                          point_t& child,   info_t& info_c);

  ///
  utilib::BasicArray<double> range;

  ///
  utilib::BasicArray<double> lower;

  ///
  utilib::BasicArray<double> upper;

  ///
  colin::BoundTypeArray lbound_type;

  ///
  colin::BoundTypeArray ubound_type;

protected:

  ///
  bool auto_rescale;

  ///
  utilib::Cauchy cauchy_rnd;

  ///
  utilib::Normal normal_rnd;

  ///
  double crossover_alpha;

  ///
  bool mutation_self_adaptation;

  ///
  double min_self_adaptive_scale_factor;

  ///
  double mutation_scale;

  ///
  double tau;

  ///
  double taudash;

  ///
  double N;

  ///
  void mutate_value(int i, double& val, info_t& info);

  ///
  utilib::BitArray mutation_direction;

};


template <class InfoT>
DomainOpsRealArray<InfoT>::DomainOpsRealArray(utilib::PropertyDict& properties)
{
   this->crossover_blocksize=1;
   properties.declare
      ( "realarray_xover_blocksize", 
        "Block size used with array-based two-point and uniform crossover",
        utilib::Privileged_Property(this->crossover_blocksize) );
   //option.add("realarray_xover_blocksize", this->crossover_blocksize,
   //option.categorize("realarray_xover_blocksize","Crossover");

   this->crossover_alpha=0.5;
   properties.declare
      ( "realarray_xover_alpha", 
        "Extent of blend crossover beyond the line connecting the two points",
        utilib::Privileged_Property(this->crossover_alpha) );
   //option.add("realarray_xover_alpha", this->crossover_alpha,
   //           "Extent of blend crossover beyond the line connecting the two points");
   //option.categorize("realarray_xover_alpha","Crossover");

   this->crossover_str="twopoint";
   properties.declare
      ( "realarray_xover_type", 
        "Crossover type:\n"
        "   blend    - an alpha-blend mutaiton\n"
        "   twopoint - standard two-point mutation\n"
        "   uniform  - standard uniform mutation",
        utilib::Privileged_Property(this->crossover_str) );
   //option.add("realarray_xover_type", this->crossover_str,
   //           "Crossover type\n               \
   // blend         - an alpha-blend mutaiton\n      \
   // twopoint      - standard two-point mutation\n  \
   // uniform       - standard uniform mutation");
   //option.categorize("realarray_xover_type", "Crossover");

   mutation_str = "step";
   properties.declare
      ( "realarray_mutation_type", 
        "Mutation type:\n"
        "   uniform  - replace the value with a uniformly random value\n"
        "   normal   - replace the value with a normal random variable\n"
        "   cauchy   - replace the value with a cauchy random variable\n"
        "   step     - replace the value with a simple coordinate step\n"
        "   interval - replace the value with a uniform value in a local interval",
        utilib::Privileged_Property(this->mutation_str) );
   //option.add("realarray_mutation_type", mutation_str,
   //           "Mutation type\n                                \
   //uniform  - replace the value with a uniformly random value\n    \
   //normal   - replace the value with a normal random variable\n    \
   //cauchy   - replace the value with a cauchy random variable\n    \
   //step     - replace the value with a simple coordinate step\n    \
   //interval - replace the value with a uniform value in a local interval");
   //option.categorize("realarray_mutation_type","Mutation");

   auto_rescale=true;
   properties.declare
      ( "realarray_autorescale", 
        "Automatically rescale mutation steps as a fraction of the "
        "parameter range",
        utilib::Privileged_Property(auto_rescale) );
   //option.add("realarray_autorescale", auto_rescale,
   //           "Automatically rescale mutation steps as a fraction of the parameter range");
   //option.categorize("realarray_autorescale","Mutation");

   mutation_scale=0.1;
   properties.declare
      ( "realarray_mutation_scale", 
        "Global scale multiplied types search scale",
        utilib::Privileged_Property(mutation_scale) );
   //option.add("realarray_mutation_scale", mutation_scale,
   //           "Global scale multiplied types search scale");
   //option.categorize("realarray_mutation_scale","Mutation");

   mutation_self_adaptation=true;
   properties.declare
      ( "realarray_mutation_selfadaptation", 
        "If true, then the step scale used for mutation is self-adapted.",
        utilib::Privileged_Property(mutation_self_adaptation) );
   //option.add("realarray_mutation_selfadaptation", mutation_self_adaptation, 
   //           "If true, then the step scale used for mutation is self-adapted.");
   //option.categorize("realarray_mutation_selfadaptation","Mutation");

   this->min_self_adaptive_scale_factor=1e-6;
   properties.declare
      ( "min_self_adaptive_scale_factor", 
        "A minimum threshold for the self_adaptive scale factor.",
        utilib::Privileged_Property(this->min_self_adaptive_scale_factor) );
   //option.add("min_self_adaptive_scale_factor",
   //           this->min_self_adaptive_scale_factor,
   //           "A minimum threshold for the self_adaptive scale factor.");
   //option.categorize("min_self_adaptive_scale_factor","Mutation");

   this->mutate_independently=false;
   properties.declare
      ( "realarray_mutate_independently", 
        "If true, then only mutate a single dimension.  Note that if "
        "this value is true, then a single dimension is always mutated, "
        "so the mutation allele rate is ignored.",
        utilib::Privileged_Property(this->mutate_independently) );
   //option.add("realarray_mutate_independently", this->mutate_independently,
   //           "If true, then only mutate a single dimension.  Note that if this value is true, then a single dimension is always mutated, so the mutation allele rate is ignored.");
   //option.categorize("realarray_mutate_independently","Mutation");

   this->mutation_allele_rate=1.0;
   properties.declare
      ( "realarray_mutation_allele_rate", 
        "The probability that any given dimension of the realarray is "
        "mutated given that the individual is mutated",
        utilib::Privileged_Property(this->mutation_allele_rate) );
   //option.add("realarray_mutation_allele_rate",
   //           this->mutation_allele_rate,
   //           "The probability that any given dimension of the realarray is mutated given that the individual is mutated");
   //option.categorize("realarray_mutation_allele_rate","Mutation");
}



template <class InfoT>
void DomainOpsRealArray<InfoT>::reset()
{
DomainOpsArray<double,InfoT>::reset();

mutation_allele_rate = (mutation_allele_rate < 0.0 ? 
                                std::sqrt(M_E/(double)this->nvars)/(double)popsize :
                                mutation_allele_rate);
if ((mutation_str == "uniform") || (mutation_str == "offset_uniform"))
   mutation_type = REAL_ARRAY_MUTATION_UNIFORM;
else if ((mutation_str == "interval") || (mutation_str == "replace_uniform"))
   mutation_type = REAL_ARRAY_MUTATION_INTERVAL;
else if ((mutation_str == "normal") || (mutation_str == "offset_normal"))
   mutation_type = REAL_ARRAY_MUTATION_NORMAL;
else if ((mutation_str == "cauchy") || (mutation_str == "offset_cauchy"))
   mutation_type = REAL_ARRAY_MUTATION_CAUCHY;
else if ((mutation_str == "step") || (mutation_str == "offset_step"))
   mutation_type = REAL_ARRAY_MUTATION_STEP;
else 
   EXCEPTION_MNGR(std::runtime_error,"DomainOpsArray::reset - bad mutation type: \"" << mutation_str << "\".\n\t\tValid types are uniform, interval, normal, cauchy, and step\n");

if (crossover_str == "none")
   crossover_type = ARRAY_XOVER_NONE;
else if (crossover_str == "twopoint")
   crossover_type = ARRAY_XOVER_TWOPOINT;
else if (crossover_str == "uniform")
   crossover_type = ARRAY_XOVER_UNIFORM;
else if (crossover_str == "blend")
   crossover_type = REAL_ARRAY_XOVER_BLEND;
else
   EXCEPTION_MNGR(std::runtime_error, "DomainOpsArray::reset -- bad xover type: \"" << crossover_str << "\".\n\t\tValid types are blend, twopoint and uniform\n");

if ((crossover_type != ARRAY_XOVER_NONE) && (this->xover_rate > 0.0))
   mutation_self_adaptation = false;
}


template <class InfoT>
void DomainOpsRealArray<InfoT>::mutate_value(int i, double& value, info_t& info)
{
//
// This dimension cannot be mutated
//
if (range[i] == 0.0) return;
//
// Uniform mutation does not use scale info, so do this type separately
//
if (mutation_type == REAL_ARRAY_MUTATION_UNIFORM) {
   value = range[i]*this->rnd() + lower[i];
   //
   // These checks are only included to account for numerical round-off
   //
   if (value > upper[i]) {
      if (ubound_type[i] == colin::hard_bound) value = upper[i];
      else if (ubound_type[i] == colin::periodic_bound) value -= range[i];
      }
   if (value < lower[i]) {
      if (lbound_type[i] == colin::hard_bound) value = lower[i];
      else if (lbound_type[i] == colin::periodic_bound) value += range[i];
      }
   return;
   }
//
// If mutating independently, then use the 'mutation_direction' array to
// force mutation directions to change from one mutation to the next (for
// the same dimension).  This enforces a type of implicit derandomization that
// is helpful.
//
double bias=0.0;
if (this->mutate_independently) {
   if (mutation_direction(i)) {
      bias = 1.0;
      mutation_direction.reset(i);
      }
   else {
      bias = -1.0;
      mutation_direction.set(i);
      }
   }
double tmp=-1.0;
switch (mutation_type) {
  case REAL_ARRAY_MUTATION_INTERVAL:
                tmp = 2.0*this->rnd()-1.0;
                break;

  case REAL_ARRAY_MUTATION_NORMAL:
                tmp = normal_rnd();
                break;

  case REAL_ARRAY_MUTATION_CAUCHY:
                tmp = cauchy_rnd();
                break;

  case REAL_ARRAY_MUTATION_STEP:
                tmp = ((bias < 0.0) || (this->rnd() < 0.5) ? -1.0 : 1.0);
                break;

  default:
                break;
  };
if (bias != 0.0)
   tmp = bias*fabs(mutation_scale*info.scale[i]*tmp);
else
   tmp *= mutation_scale*info.scale[i];
OUTPUTPR(1000,ucout << "   MScale: " << mutation_scale 
         << " AdaptiveScale: " << info.scale[i] 
         << " Offset: " << tmp << std::endl);
value += tmp;

if (value > upper[i]) {
   if (ubound_type[i] == colin::hard_bound)
      value = upper[i];
   else if (ubound_type[i] == colin::periodic_bound) {
      while (value > upper[i])
        value -= range[i];
      }
   }
if (value < lower[i]) {
   if (lbound_type[i] == colin::hard_bound)
      value = lower[i];
   else if (lbound_type[i] == colin::periodic_bound) {
      while (value < lower[i])
        value += range[i];
      }
   }
if (mutation_self_adaptation) {
   double tmp = info.scale[i];
   info.scale[i] *= std::exp(taudash * N + tau * normal_rnd());
   if (info.scale[i] > 10.0*tmp)
      info.scale[i] = 10.0*tmp;
   else if (info.scale[i] < tmp/10.0)
      info.scale[i] = tmp/10.0;
   else if (info.scale[i] < this->min_self_adaptive_scale_factor)
      info.scale[i] = this->min_self_adaptive_scale_factor;
   }
}


template <class InfoT>
int DomainOpsRealArray<InfoT>::apply_xover(point_t& parent1, info_t& info1,
                          point_t& parent2, info_t& info2,
                          point_t& child,   info_t& info_c)
{
if (this->nvars == 0) return 0;

switch (this->crossover_type) {
  case REAL_ARRAY_XOVER_BLEND:
        {
        double lambda;
        double alpha1 = - crossover_alpha;
        double alpha2 = 1.0 + crossover_alpha;
        //
        // If enforcing the bound constraints, then limit the value of
        // lambda to feasible values
        //
        for (unsigned int i=0; i<this->nvars; i++) {
          if ((lower[i] != upper[i]) && (parent1[i] != parent2[i])) {
             double alpha1_value = alpha1 * parent1[i] +
                                           (1.0 - alpha1) * parent2[i];
             if (alpha1_value < lower[i])
                   alpha1 = (lower[i] - parent2[i])/
                                        (parent1[i]-parent2[i]);
             else if (alpha1_value > upper[i])
                   alpha1 = (upper[i] - parent2[i])/
                                        (parent1[i]-parent2[i]);

             double alpha2_value = alpha2 * parent1[i] + 
                                      (1.0 - alpha2) * parent2[i];
             if (alpha2_value > upper[i])
                   alpha2 = (upper[i] - parent2[i])/
                                        (parent1[i]-parent2[i]);
             if (alpha2_value < lower[i])
                   alpha2 = (lower[i] - parent2[i])/
                                        (parent1[i]-parent2[i]);
             }
           }
        //
        // If using linear constraints, then further limit the value of
        // lambda to feasible values.
        //
        // Note: should we store equality and inequality constraints seperately?
        // This code is wasteful when equality constraints exist as well.
        //
           /*
        if (this->problem->numLinearConstraints() > 0) {
           EXCEPTION_MNGR(runtime_error,"linear constraints not supported");
           search_dir << p1;
           search_dir -= p2;
           double amax,amin;
           compute_linear_range(amax,amin,p2,search_dir,problem,
                                        xover_tmp1,xover_tmp2);
           if (amax < alpha1) alpha1=amax;
           if (amin > alpha2) alpha2=amin;
           }
           */
        lambda = (alpha2-alpha1)*(this->rnd()) + alpha1;
        //
        // Generate new point
        //
        for (unsigned int i=0; i<this->nvars; i++)
          if (lower[i] != upper[i])
             child[i] = lambda * parent1[i] + (1.0 - lambda) * parent2[i];
        //
        // Adapt the mutation scale parameters as well
        //
#if 0
        if (crossover.adaptation)
           for (int i=0; i<nvars(); i++)
             if (lower[i] != upper[i])
                ch.mutation_scale[i] = lambda * parent1.mutation_scale[i] +
                                        (1.0 - lambda) * p2.mutation_scale[i];
#endif

        OUTPUTPR(4,
                 ucout << "Xover-Blend : " << child << "\n";
                 ucout << "Lambda      : " << lambda << '\n';
                 );
        }
        break;

  default:
        return base_t::apply_xover(parent1,info1,parent2,info2,child,info_c);
  };

//
// Check for similarity with parents
//
if (child == parent1)
   return 1;
if (child == parent2)
   return 2;
return 3;
}


}

#endif

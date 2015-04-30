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
 * \file DomainOpsArray.h
 *
 * Defines the scolib::DomainOpsArray class.
 */

#ifndef scolib_DomainOpsArray_h
#define scolib_DomainOpsArray_h

#include <acro_config.h>
#include <utilib/AnyRNG.h>
#include <utilib/BasicArray.h>
#include <utilib/Uniform.h>
#include <utilib/ParameterSet.h>
#include <utilib/DUniform.h>
#include <utilib/_math.h>
#include <scolib/DomainOpsBase.h>

#if !defined(DOXYGEN)
#define ARRAY_XOVER_NONE              0
#define ARRAY_XOVER_TWOPOINT          1
#define ARRAY_XOVER_UNIFORM           2

#define ARRAY_MUTATION_STD            0
#endif

namespace scolib {


/// This is meant to be a base class for array-based problem representations
template <class ValueT, class InfoT>
class DomainOpsArray
   : public utilib::CommonIO, 
     public DomainOpsBase<utilib::BasicArray<ValueT>, InfoT >
{
public:

  ///
  typedef typename utilib::BasicArray<ValueT> point_t;

  ///
  typedef InfoT info_t;

  ///
  DomainOpsArray();

  ///
  void initialize(unsigned int nvars_, unsigned int popsize_, double xover_rate_, double m_rate_)
	{
	nvars = nvars_;
	popsize = popsize_;
	xover_rate=xover_rate_;
	m_rate=m_rate_;
	}

  ///
  void reset();

  ///
   void write(std::ostream& /*os*/) const {}

  ///
  void initialize_point(point_t& point, info_t& info);

  ///
  int apply_xover(point_t& parent1, info_t& info1,
		  point_t& parent2, info_t& info2,
		  point_t& child,   info_t& info_c);

  ///
  bool apply_mutation(point_t& point, info_t& info,
				int parent_ndx);

  ///
  void set_rng(utilib::AnyRNG& rng_)
	{ rng = rng_; rnd.generator(&rng_); }

protected:

  ///
  bool mutate_independently;

  ///
  utilib::BasicArray<int> mutation_indices;

  ///
  unsigned int mutation_ndx;

  ///
  utilib::AnyRNG rng;

  ///
  utilib::Uniform rnd;

  ///
  double mutation_allele_rate;

  ///
  unsigned int nvars;

  ///
  unsigned int popsize;

  ///
  int mutation_type;

  ///
  std::string mutation_str;

  ///
  int Mu_next;

  ///
  int crossover_type;

  ///
  std::string crossover_str;

  ///
  int crossover_blocksize;

  ///
  bool crossover_Fstat_flag;

  ///
  virtual void mutate_value(int i, ValueT& val, info_t& info) = 0;

  ///
  double xover_rate;

  ///
  double m_rate;
};




template <class ValueT, class InfoT>
inline DomainOpsArray<ValueT,InfoT>::DomainOpsArray() :
  nvars(0)
{
mutate_independently=false;
crossover_type = ARRAY_XOVER_TWOPOINT;
}



template <class ValueT, class InfoT>
void DomainOpsArray<ValueT,InfoT>::reset()
{
if (mutate_independently) {
   mutation_indices.resize(nvars);
   for (unsigned int i=0; i<nvars; i++)
     mutation_indices[i]=i;
   shuffle(mutation_indices,&rng);
   mutation_ndx=0;
   }
}



template<class ValueT, class InfoT>
void DomainOpsArray<ValueT,InfoT>::initialize_point(point_t& point, 
                                                    info_t& /*info*/)
{
point.resize(nvars);
}



template<class ValueT, class InfoT>
int DomainOpsArray<ValueT,InfoT>::apply_xover(point_t& parent1, info_t& /*info1*/,
					      point_t& parent2, info_t& /*info2*/,
					      point_t& child,   info_t& /*info_c*/)
{
if (nvars == 0) return 0;

if (nvars == 1) {
   if (rnd() < 0.5)
      child[0] = parent1[0];
   else
      child[0] = parent2[0];
   }
else {
   switch (crossover_type) {

     case ARRAY_XOVER_TWOPOINT:
        {
	if (nvars == 1) {
	   if (rnd() < 0.5)
              child[0] = parent1[0];
           else
              child[0] = parent2[0];
	   break;
           }
        //
        // Figure out xover points
        //
        int lambda1 = utilib::Discretize<int>(rnd(), 1,
                                nvars/crossover_blocksize-1);
        int lambda2 = utilib::Discretize<int>(rnd(), 1,
                                nvars/crossover_blocksize-2);
        if (lambda2 >= lambda1)
           lambda2++;   // This keeps lambda2 unif on nvars-1 other points
        else {
           int tmp;
           tmp = lambda1; lambda1 = lambda2; lambda2 = tmp;
           }
        lambda1 *= crossover_blocksize;
        lambda2 *= crossover_blocksize;
        child.set_subvec(0,lambda1,parent1,0);
        child.set_subvec(lambda1,lambda2-lambda1,parent2,lambda1);
        child.set_subvec(lambda2,nvars-lambda2,parent1,lambda2);
        OUTPUTPR(4, ucout << "Xover-TwoPoint : " << child << "\n");
        }
        break;

     case ARRAY_XOVER_UNIFORM:
        {
        //
        // Construct new point
        //
        for (unsigned int i=0; i<nvars; i++) {
          double val = rnd();
          child.set_subvec(i*crossover_blocksize, crossover_blocksize,
                        (val >= 0.5? parent1: parent2), i*crossover_blocksize);
          }
        OUTPUTPR(4, ucout << "Xover-Uniform : " << child << "\n");
        }
        break;

     };
}

//
// Check for similarity with parents
//
if (child == parent1)
   return 1;
if (child == parent2)
   return 2;
return 3;
}




template<class ValueT, class InfoT>
bool DomainOpsArray<ValueT,InfoT>::apply_mutation(point_t& point, info_t& info,
                                                  int /*parent_ndx*/)
{
//
// Perform independent mutation
//
if (mutate_independently) {
   int i = mutation_indices[mutation_ndx++];
   if (mutation_ndx == nvars) {
      mutation_ndx = 0;
      shuffle(mutation_indices,&rng);
      }
   OUTPUTPR(10, ucout << "   Mutation index: " << i << std::endl);
   mutate_value(i,point[i], info);
   return true;
   }
//
// Otherwise ... return if the allele rate is zero
//
bool eval_flag=false;
if (mutation_allele_rate == 0.0)
   return eval_flag;
//
// Perform mutation, by computing where the next mutation point will
// occur.  This is more efficient when the per_bit mutation rate is low.
//
Mu_next=0;
while (1) {
  //
  // Update next mutation location
  //
  if (mutation_allele_rate < 1.0) {
     double r = rnd();
     Mu_next += (int) std::ceil (std::log(r) / std::log(1.0 - mutation_allele_rate));
     }
  else
     Mu_next += 1;

  if (Mu_next > (int)nvars)
     break;
  int i= Mu_next-1;
  if (debug > 1) {
     ucout << "Fancy Mutate #" << i << " ";
     ucout << point[i];
     }
  eval_flag = true;
  OUTPUTPR(10, ucout << "   Mutation index: " << i << std::endl);
  mutate_value(i,point[i],info);
  }
return eval_flag;
}

}

#endif

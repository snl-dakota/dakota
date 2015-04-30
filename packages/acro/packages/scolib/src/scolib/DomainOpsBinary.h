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
 * \file DomainOpsBinary.h
 *
 * Defines the DomainOpsBinary class.
 */

#ifndef scolib_DomainOpsBinary_h
#define scolib_DomainOpsBinary_h

#include <acro_config.h>
#include <scolib/DomainOpsBase.h>
#include <utilib/DUniform.h>
#include <utilib/_math.h>
#include <utilib/AnyRNG.h>
#include <utilib/BitArray.h>
#include <utilib/BasicArray.h>
#include <utilib/Uniform.h>

#if !defined(DOXYGEN)
#define BINARY_XOVER_TWOPOINT          0
#define BINARY_XOVER_UNIFORM           1
//#define BINARY_XOVER_TWOPOINT_DIFF     2

#define BINARY_MUTATION_STD            0
#define BINARY_MUTATION_XCHG           1
#endif


namespace scolib {


///
template <class InfoT>
class DomainOpsBinary : public utilib::CommonIO
{
public:

  ///
  typedef utilib::BitArray point_t;

  ///
  typedef InfoT info_t;

  ///
  DomainOpsBinary(utilib::PropertyDict& properties);

  ///
  template <class ProblemT>
  void initialize(ProblemT& problem, unsigned int popsize_, double , double )
	{
	nvars = problem->num_binary_vars;
	popsize = popsize_;
	}

  ///
  void reset();

  ///
   void write(std::ostream& /*os*/) const {}

  ///
  void initialize_point(point_t& point, info_t& info);

  ///
  void randomize(point_t& point, info_t& info);

  ///
  int apply_xover(point_t& parent1, info_t& info1,
		  point_t& parent2, info_t& info2,
		  point_t& child,   info_t& info_c);

  ///
  bool apply_mutation(point_t& point, info_t& info, int parent_ndx);

  ///
  void set_rng(utilib::AnyRNG& rng_)
	{ rng = rng_; rnd.generator(&rng_); }

protected:

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
  int mutation_xchg_num;

  ///
  int num_bits_on;

  ///
  point_t xchg_on;

  ///
  point_t xchg_off;

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
  utilib::BasicArray<int> xover_xchg;

};


template <class InfoT>
DomainOpsBinary<InfoT>::DomainOpsBinary(utilib::PropertyDict& properties) 
   : nvars(0),
     popsize(0),
     num_bits_on(-1),
     Mu_next(0)
{
   crossover_blocksize=1;
   properties.declare
      ( "binary_xover_blocksize", 
        "Block size used with binary two-point and uniform crossover",
        utilib::Privileged_Property(crossover_blocksize) );
   //option.add("binary_xover_blocksize", crossover_blocksize,
   //           "Block size used with binary two-point and uniform crossover");
   //option.categorize("binary_xover_blocksize",	"Mutation");

   crossover_str="twopoint";
   properties.declare
      ( "binary_xover_type", 
        "Crossover type:\n"
        "   twopoint - standard two-point xover\n"
        "   uniform  - standard uniform xover",
        utilib::Privileged_Property(crossover_str) );
   //option.add("binary_xover_type", crossover_str,
   //           "Crossover type\n               \
   // twopoint      - standard two-point xover\n     \
   // uniform       - standard uniform xover");
   //option.categorize("binary_xover_type", "Crossover");

   mutation_xchg_num=1;
   properties.declare
      ( "binary_mutation_xchg", 
        "Mutation exchange number",
        utilib::Privileged_Property(mutation_xchg_num) );
   //option.add("binary_mutation_xchg", mutation_xchg_num,
   //           "Mutation exchange number");
   //option.categorize("binary_mutation_xchg", "Mutation");

   mutation_str="standard";
   properties.declare
      ( "binary_mutation_type",  
        "Mutation type:\n"
        "   standard - simple bit flips\n"
        "   exchange - keep a fixed number of bits on",
        utilib::Privileged_Property(mutation_str) );
   //option.add("binary_mutation_type", mutation_str,
   //           "Mutation type\n                \
   // standard - simple bit flips\n                  \
   // exchange - keep a fixed number of bits on");
   //option.categorize("binary_mutation_type", "Mutation");

   mutation_allele_rate=-1.0;
   properties.declare
      ( "binary_mutation_allele_rate", 
        "The probability that any given bit of an individual is "
        "mutated given that the individual is mutated",
        utilib::Privileged_Property(mutation_allele_rate) );
   //option.add("binary_mutation_allele_rate", mutation_allele_rate,
   //           "The probability that any given bit of an individual is mutated given that the individual is mutated");
   //option.categorize("binary_mutation_allele_rate", "Mutation");
}



template <class InfoT>
void DomainOpsBinary<InfoT>::reset()
{
if (mutation_str == "standard") {
   mutation_type = BINARY_MUTATION_STD;
   mutation_allele_rate =
      (mutation_allele_rate < 0.0 ? std::sqrt(M_E/(double)nvars)/(double)popsize :
        mutation_allele_rate);
   }
else if (mutation_str == "exchange") {
   mutation_type = BINARY_MUTATION_XCHG;
   }

if (crossover_str == "twopoint")
   crossover_type = BINARY_XOVER_TWOPOINT;
else if (crossover_str == "uniform")
   crossover_type = BINARY_XOVER_UNIFORM;
//else if (crossover_str == "twopoint_diff")
//  crossover_type = BINARY_XOVER_TWOPOINT_DIFF;

#if 0
if (num_bits_on != -1) {
   if (mutation_type == BINARY_MUTATION_STD)
      ErrAbort("Standard mutation will not enforce the fixed number of bits constraint!");

   if ((mutation_type == BINARY_MUTATION_XCHG) &&
       ((nvars - num_bits_on) < mutation.xchg_num))
      ErrAbort(errmsg("Exchange mutation trying to use %d bits when only %d are free",mutation.xchg_num,(nvars - num_bits_on)));

   xchg_off.resize(num_bits_on);
   xchg_off.reset();
   xchg_on.resize(nvars-num_bits_on);
   xchg_on.reset();
   for (int i=0; i<mutation_xchg_num; i++) {
     xchg_off.set(i);
     xchg_on.set(i);
     }
   xover_xchg.resize(nvars);
   for (int j=0; j<nvars; j++)
     xover_xchg[j] = j;
   }
else {
   if (mutation.type == BINARY_MUTATION_XCHG)
      ErrAbort("Exchange mutation only works with the fixed number of bits constraint!");
   }
#endif
}



template <class InfoT>
void DomainOpsBinary<InfoT>::initialize_point(point_t& point, info_t& /*info*/)
{ point.resize(nvars); }


template <class InfoT>
void DomainOpsBinary<InfoT>::randomize(point_t& point, info_t& /*info*/)
{
/// Every individual has a fixed number of bits on
if (num_bits_on != -1) {
   point.reset();
   for (int j=0; j<num_bits_on; j++)
     point.set(j);
   shuffle(point,&rng);
   }

/// Every individual is a random value
else {
   for (unsigned int j=0; j<nvars; j++)
     point.set(rnd()>=0.5);
   }
}



template <class InfoT>
int DomainOpsBinary<InfoT>::apply_xover(point_t& parent1, info_t& /*info1*/,
					point_t& parent2, info_t& /*info2*/,
					point_t& child,   info_t& /*info_c*/)
{
if ( nvars == 0 )
   return 0;

switch (crossover_type) {

  case BINARY_XOVER_TWOPOINT:
        {
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
        //
        // Construct new point
        //
        if (num_bits_on == -1) {
           //
           // Standard
           //
           for (int i=0; i<lambda1; i++)
             child[i] = parent1[i];
           for (int i=lambda1; i<lambda2; i++)
             child[i] = parent2[i];
           for (int i=lambda2; i<(int)nvars; i++)
             child[i] = parent1[i];
           }
        else {
#if 0
           //
           // Fixed number of points
           //
           utilib::shuffle(xover_xchg,rng);
           int ctr=0;
           for (int i=0; i<nvars; i++) {
             int curr;
             if (i < lambda1)
                ch.put(xover_xchg[i],curr = parent1(xover_xchg[i]));
             else if (i >= lambda2)
                ch.array.put(xover_xchg[i],curr = parent1.array(xover_xchg[i]));
             else
                ch.array.put(xover_xchg[i],curr = parent2.array(xover_xchg[i]));
             ctr += curr;
             if (ctr == num_bits_on)
                break;
             }
           if (ctr < num_bits_on) {
              utilib::shuffle(xover_xchg,rng);
              for (int i=0; ctr < num_bits_on; i++)
                if (ch.array(xover_xchg[i]) == 0) {
                   ch.array.set(xover_xchg[i]);
                   ctr++;
                   }
              }
#endif
           }
        }
        break;

  case BINARY_XOVER_UNIFORM:
        {
        //
        // Construct new point
        //
        if (num_bits_on == -1) {
           for (int i=0; i<(int)nvars; i++) {
             if (rnd() >= 0.5)
                child[i] = parent1[i];
             else
                child[i] = parent2[i];
             }
           }
        else {
#if 0
           utilib::shuffle(xover_xchg,rng);
           ch.array.reset();
           int ctr=0;
           for (int i=0; i<nvars(); i++) {
             int curr;
             if (rnd() >= 0.5)
                ch.array.put(xover_xchg[i],curr = parent1(xover_xchg[i]));
             else
                ch.array.put(xover_xchg[i],curr = parent2.array(xover_xchg[i]));
             ctr += curr;
             if (ctr == num_bits_on)
                break;
             }
           utilib::shuffle(xover_xchg,rng);
           if (ctr < num_bits_on) {
              utilib::shuffle(xover_xchg,rng);
              for (int i=0; ctr < num_bits_on; i++)
                if (ch.array(xover_xchg[i]) == 0) {
                   ch.array.set(xover_xchg[i]);
                   ctr++;
                   }
              }
#endif
           }
        }
        break;

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




template <class InfoT>
bool DomainOpsBinary<InfoT>::apply_mutation(point_t& point, info_t& /*info*/,
                                            int /*parent_ndx*/)
{
bool eval_flag=false;

if (mutation_type == BINARY_MUTATION_STD) {
   if (mutation_allele_rate == 0.0)
      return eval_flag;

   //
   // Perform mutation, by computing where the next mutation point will
   // occur.  This is more efficient when the per_bit mutation rate is low.
   //
   Mu_next=0;
   while (1) {
                           // update next mutation location
     if (mutation_allele_rate < 1.0) {
        double r = rnd();
        Mu_next += (int) std::ceil (std::log(r) / std::log(1.0 - mutation_allele_rate));
        }
     else
        Mu_next += 1;

     if (Mu_next > (int)nvars)
        break;
     else {
        if (debug > 1) {
           ucout << "Fancy Mutate #" << (Mu_next-1) << " ";
           ucout << point(Mu_next-1);
           }
        int i= Mu_next-1;
        eval_flag = true;
        point.flip(i);
        }
     }
   }

#if 0
if (mutation_type == BINARY_MUTATION_XCHG) {
   eval_flag=true;
   //
   // Initial Information
   //
   if (debug > 1)
      ucout << "Start: " << point << endl;
   //
   // Get the mutation moves
   //
   utilib::shuffle(xchg_off,rng);   // Pick pts to turn off
   utilib::shuffle(xchg_on,rng);    // Pick pts to turn on
   //
   // Do the mutations
   //
   int on_ctr=0,off_ctr=0;
   for (int i=0; i<nvars; i++) {
     if (point(i)) {
        if (xchg_off(off_ctr++))
           point.reset(i);
        }
     else {
        if (xchg_on(on_ctr++))
           point.set(i);
        }
     }
   if (debug > 1)
      ucout << "Final: " << point << endl;
   }
#endif
return eval_flag;
}

}


#endif

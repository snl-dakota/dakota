/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file SampleGenerators.h
 *
 * Classes that define generic wrappers for sample generators:
 * utilib::SampleGenerator and utilib::UniformSampleGenerator
 */

#ifndef utilib_SampleGenerators_h
#define utilib_SampleGenerators_h

#include <utilib_config.h>
#include <utilib/AnyRNG.h>
#include <utilib/Uniform.h>
#include <utilib/BasicArray.h>
#include <utilib/MixedIntVars.h>
#include <utilib/_math.h>

namespace utilib {

/** 
 *  The base class for sample generator objects
 */
template <class DomainT>
class SampleGenerator
{
public:

  /// Reset the sample generator
  virtual void reset()
		{}

  /// Generate a new point
  virtual void new_point(DomainT& ) = 0;

  /// Set a random number generator
  virtual void set_rng(AnyRNG& ) = 0;

};


/** 
 * Sample generator objects that generate uniform samples
 */
template <class DomainT>
class UniformSampleGenerator
{
public:

  /// Reset the sample generator
  void reset() {}

  /// Generate a new point
  void new_point(DomainT& )
	{ EXCEPTION_MNGR(runtime_error, "UniformSampleGenerator undefined for this datatype"); }

  /// Set a random number generator
  void set_rng(AnyRNG& ) {}

};


/** 
 * Sample generator objects that generate uniform samples
 */
template <>
class UniformSampleGenerator<BasicArray<double> >
{
public:

  /// Reset the sample generator
  void reset();

  /// Generate a new point
  void new_point(BasicArray<double>& pt);

  /// Set a random number generator
  void set_rng(AnyRNG& rng)
	{ urnd.generator(&rng); }

  /// The lower bounds
  BasicArray<double> l_bound;

  /// The upper bounds
  BasicArray<double> u_bound;

  /// A uniform random number generator
  Uniform urnd;
};


inline void UniformSampleGenerator<BasicArray<double> >::reset()
{
if (l_bound.size() == 0)
   EXCEPTION_MNGR(runtime_error, "UniformSampleGenerator<BasicArray<double> - No lower bounds provided");

if (u_bound.size() == 0)
   EXCEPTION_MNGR(runtime_error, "UniformSampleGenerator<BasicArray<double> - No upper bounds provided");

if (l_bound.size() != u_bound.size())
   EXCEPTION_MNGR(runtime_error, "UniformSampleGenerator<BasicArray<double> - Inconsistent lower and upper bounds: " << l_bound.size() << " " << u_bound.size());
}


inline void UniformSampleGenerator<BasicArray<double> >::new_point(BasicArray<double>& pt)
{
pt.resize(l_bound.size());

for (size_type i=0; i<l_bound.size(); i++)
  pt[i] = (u_bound[i] - l_bound[i])*urnd() + l_bound[i];
}



/** 
 * Sample generator objects that generate uniform samples for MixedIntVars
 */
template <>
class UniformSampleGenerator<MixedIntVars>
{
public:

  /// Constructor
  UniformSampleGenerator()
	: nbinary(0) {}

  /// Reset the sample generator
  void reset();

  /// Generate a new point
  void new_point(MixedIntVars& pt);

  /// Set a random number generator
  void set_rng(AnyRNG& rng)
	{ urnd.generator(&rng); }

  /// The number of binary variables
  unsigned int nbinary;

  /// The double lower bounds
  BasicArray<double> d_l_bound;

  /// The double upper bounds
  BasicArray<double> d_u_bound;

  /// The integer lower bounds
  BasicArray<int> i_l_bound;

  /// The integer upper bounds
  BasicArray<int> i_u_bound;

  /// A uniform random number generator
  Uniform urnd;
};


inline void UniformSampleGenerator<MixedIntVars>::reset()
{
if (d_l_bound.size() == 0)
   EXCEPTION_MNGR(runtime_error, "UniformSampleGenerator<> - No lower bounds provided for doubles");

if (d_u_bound.size() == 0)
   EXCEPTION_MNGR(runtime_error, "UniformSampleGenerator<> - No upper bounds provided for doubles");

if (i_l_bound.size() == 0)
   EXCEPTION_MNGR(runtime_error, "UniformSampleGenerator<> - No lower bounds provided for integers");

if (i_u_bound.size() == 0)
   EXCEPTION_MNGR(runtime_error, "UniformSampleGenerator<> - No upper bounds provided for integers");

if (d_l_bound.size() != d_u_bound.size())
   EXCEPTION_MNGR(runtime_error, "UniformSampleGenerator<> - Inconsistent lower and upper bounds for doubles: " << d_l_bound.size() << " " << d_u_bound.size());

if (i_l_bound.size() != i_u_bound.size())
   EXCEPTION_MNGR(runtime_error, "UniformSampleGenerator<> - Inconsistent lower and upper bounds for integers: " << i_l_bound.size() << " " << i_u_bound.size());
}


inline void UniformSampleGenerator<MixedIntVars>::new_point(MixedIntVars& pt)
{
pt.resize(nbinary, i_l_bound.size(), d_l_bound.size());

NumArray<double>& real = pt.Real();
for (size_type i=0; i<d_l_bound.size(); i++)
  real[i] = (d_u_bound[i] - d_l_bound[i])*urnd() + d_l_bound[i];

NumArray<int>& integer = pt.Integer();
for (size_type i=0; i<i_l_bound.size(); i++) {
  int temp = (int)std::floor(d_l_bound[i] + (d_u_bound[i]-d_l_bound[i]+1) * urnd());
  if (temp > i_u_bound[i])
     temp = i_u_bound[i];        // This handles the case when urand == 1.0
  integer[i] = temp;
  }

BitArray& binary = pt.Binary();
unsigned int nbits = std::min( (unsigned int)std::floor((nbinary+1)*urnd()), nbinary);
binary.reset();
for (unsigned int i=0; i<nbits; i++)
  binary.set(i);
utilib::shuffle(binary,&(urnd.generator()));
}


} // namespace utilib

#endif

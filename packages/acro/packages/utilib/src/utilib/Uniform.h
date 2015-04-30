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
 * \file Uniform.h
 *
 * Defines the utilib::Uniform class
 */

#ifndef utilib_Uniform_h
#define utilib_Uniform_h

#include <utilib/std_headers.h>
#include <utilib/exception_mngr.h>
#include <utilib/RandomVariable.h>

namespace utilib {

#if defined(SGI)
using std::floor;
#endif

/**
 * A class that generates uniformly distributed random variables.
 * Adapted from GNU code by Dirk Grunwald.
 */
class Uniform: public SimpleRandomVariable<double>
{
public:

  /// Constructor
  Uniform() : pLow(0.0), pHigh(1.0), delta(1.0) {}

  /// Templated constructor
  template <class RNGT>
  Uniform(RNGT* gen, double low=0.0, double high=1.0)
	: SimpleRandomVariable<double>()
	{
	generator(gen);
    	pLow = (low < high) ? low : high;
    	pHigh = (low < high) ? high : low;
    	delta = pHigh - pLow;
	}

  /// Returns the lower limit of the r.v.
  double low()
	{ return pLow; }

  /// Sets the lower limit of the r.v.
  double low(double x)
	{
  	double tmp = pLow;
  	pLow = x;
  	delta = pHigh - pLow;
  	return tmp;
	}

  /// Returns the upper limit of the r.v.
  double high()
	{ return pHigh; }

  /// Sets the upper limit of the r.v.
  double high(double x)
	{
  	double tmp = pHigh;
  	pHigh = x;
  	delta = pHigh - pLow;
  	return tmp;
	}

  ///  Generate an instance of this random variable
  virtual double operator()(double low_, double high_)
	{
	double tmp_low=pLow;
	double tmp_high=pHigh;
	double tmp_delta=delta;
	low(low_);
	high(high_);
	double ans = operator()();
	pLow=tmp_low;
	pHigh=tmp_high;
	delta=tmp_delta;
	return ans;
	}

  ///
  virtual double operator()()
	{
	if (!pGenerator)
   	   EXCEPTION_MNGR(runtime_error, "Uniform::operator() : Attempting to use a NULL RNG.");
	double temp = (double) (pHigh-pLow)*(pGenerator.asDouble()) + pLow;
	return(temp);
	} 

protected:

  /// The lower limit of the r.v.
  double pLow;

  /// The upper limit of the r.v.
  double pHigh;

  /// The difference between \c pHigh and \c pLow.
  double delta;

};


/** Discretize a value in [0.0,1.0] into {low, ..., high} */
template <class IntType>
IntType Discretize(double urand, IntType low, IntType high) 
{
IntType ans = static_cast<IntType>(std::floor(low + (high-low+1) * urand));
if (ans > high)
   ans = high;		// This handles the case when urand == 1.0
return ans;
}


} // namespace utilib

#endif

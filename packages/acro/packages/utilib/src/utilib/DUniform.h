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
 * \file DUniform.h
 *
 * Defines the utilib::DUniform class
 */

#ifndef utilib_DUniform_h
#define utilib_DUniform_h

#include <utilib_config.h>
#include <utilib/std_headers.h>
#include <utilib/exception_mngr.h>
#include <utilib/RandomVariable.h>

namespace utilib {

/**
 * A discrete uniform distribution.
 */
template <class IntType>
class DUniform: public SimpleRandomVariable<IntType>
{
public:

  /// Constructor
  DUniform() : pLow(0), pHigh(1), delta(1) {}

  /// Constructor templated on rng
  template <class RNGT>
  DUniform(RNGT* gen, IntType low=0, IntType high=1)
	: SimpleRandomVariable<IntType>()
	{
	this->generator(gen);
    	pLow = (low < high) ? low : high;
    	pHigh = (low < high) ? high : low;
    	delta = pHigh - pLow;
	}

  /// Returns the lower limit of the r.v.
  IntType low()
	{ return pLow; }

  /// Sets the lower limit of the r.v.
  IntType low(IntType x)
	{
  	IntType tmp = pLow;
  	pLow = x;
  	delta = pHigh - pLow;
  	return tmp;
	}

  /// Returns the upper limit of the r.v.
  IntType high()
	{ return pHigh; }

  /// Sets the upper limit of the r.v.
  IntType high(IntType x)
	{
  	IntType tmp = pHigh;
  	pHigh = x;
  	delta = pHigh - pLow;
  	return tmp;
	}

  ///
  IntType operator()()
	{
	if (!this->pGenerator)
   	   EXCEPTION_MNGR( std::runtime_error , "DUniform::operator() : Attempting to use a NULL RNG.");

	IntType temp = (IntType)std::floor(pLow + (pHigh-pLow+1) * this->pGenerator.asDouble());
	if (temp > pHigh)
   	   temp = pHigh;	// This handles the case when urand == 1.0
	return(temp);
	}

  /// Generates a random variable given a lower and upper limit for the r.v.
  IntType operator()(IntType low_, IntType high_)
	{
	IntType tmp_low = pLow;
	IntType tmp_high = pHigh;
	IntType tmp_delta = delta;
	low(low_);
	high(high_);
	IntType ans = (*this)();
	pLow=tmp_low;
	pHigh=tmp_high;
	delta=tmp_delta;
	return ans;
	}

protected:

  /// The lower limit of the r.v.
  IntType pLow;

  /// The upper limit of the r.v.
  IntType pHigh;

  /// The difference between \c pHigh and \c pLow.
  IntType delta;

};

} // namespace utilib

#endif

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
 * \file Exponential.h
 *
 * Defines the utilib::Exponential class
 */

#ifndef utilib_Exponential_h
#define utilib_Exponential_h

#include <utilib_config.h>
#include <utilib/std_headers.h>
#include <utilib/Uniform.h>

namespace utilib {


/**
 * A class that generates exponentially distributed random variables.
 */
class Exponential: public SimpleRandomVariable<double>
{
public:

  /// Constructor
  Exponential() : b(1.0) {}

  /// Constructor templated on rng
  template <class RNGT>
  Exponential(RNGT* gen, double _meanval=1.0)
	: SimpleRandomVariable<double>(), uvar()
	{ generator(gen); uvar.generator(gen); b=_meanval;}

  /// Returns the mean of this distribution.
  double meanval()
	{return b;}

  /// Sets the mean of this distribution.
  void meanval(double _b)
	{b=_b;}

  /// Generate an instance of this random variable
  double operator()(double _b)
	{
	double tmp_b=b;
	b=_b;
	double ans = operator()();
	b=tmp_b;
	return ans;
	}

  ///
  double operator()();

protected:

  ///
  void reset_generator()
        {uvar.generator(&pGenerator);}

  /// The mean of this distribution.
  double b;

  /// Used to generate the r.v.
  Uniform uvar;

};


inline double Exponential::operator()()
{
if (!pGenerator)
   EXCEPTION_MNGR(runtime_error, "Exponential::operator() : Attempting to use a NULL RNG.");

double temp = - b * std::log( uvar() );
return(temp);
}

} // namespace utilib

#endif

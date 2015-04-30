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
 * \file Geometric.h
 *
 * Defines the utilib::Geometric class
 */

#ifndef utilib_Geometric_h
#define utilib_Geometric_h

#include <utilib_config.h>
#include <utilib/std_headers.h>
#include <utilib/Uniform.h>

namespace utilib {


/**
 * A class that generates exponentially distributed random variables.
 */
class Geometric: public SimpleRandomVariable<int>
{
public:

  /// Constructor
  Geometric() : p(0.0) {}

  /// Constructor templated on rng
  template <class RNGT>
  Geometric(RNGT* gen, double _p=0.0)
	: SimpleRandomVariable<int>(), uvar()
	{ generator(gen); uvar.generator(gen); p=_p; }

  /// Returns the probability of each success
  double probability()
	{return p;}

  /// Sets the probability of each success
  void probability(double _p)
	{p=_p;}

  /// Generate an instance of this random variable
  int operator()(double _p)
	{
	double tmp_p=p;
	p = _p;
	int ans = operator()();
	p=tmp_p;
	return ans;
	}

  ///
  int operator()();

protected:

  ///
  void reset_generator()
        {uvar.generator(&pGenerator);}

  /// The probability of success for each trial
  double p;

  /// Used to generate the r.v.
  Uniform uvar;

};


inline int Geometric::operator()()
{
if (!pGenerator)
   EXCEPTION_MNGR(runtime_error, "Geometric::operator() : Attempting to use a NULL RNG.");

if (p == 0.0)
   EXCEPTION_MNGR(runtime_error, "Geometric::operator() : Attempting to use parameter p=0.0.");

if (p == 1.0)
   EXCEPTION_MNGR(runtime_error, "Geometric::operator() : Attempting to use parameter p=1.0.");

int temp = (int) std::ceil(std::log( uvar() ) / std::log(1-p))-1;
return(temp);
}

} // namespace utilib

#endif

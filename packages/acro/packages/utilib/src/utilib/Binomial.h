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
 * \file Binomial.h
 *
 * Defines the utilib::Binomial class.
 */

#ifndef utilib_Binomial_h
#define utilib_Binomial_h

#include <utilib_config.h>
#include <utilib/std_headers.h>
#include <utilib/Uniform.h>

namespace utilib {


/**
 * A class that generates exponentially distributed random variables.
 */
class Binomial: public SimpleRandomVariable<int>
{
public:

  /// Constructor
  Binomial() : p(0.0), num(0) {}

  /// Constructor
  template <class RNGT>
  Binomial(RNGT* gen, double _p=0.0, int _num=0)
	: SimpleRandomVariable<int>(), uvar()
	{ generator(gen); uvar.generator(gen); p=_p; num=_num;}

  /// Returns the probability of each success
  double probability()
	{return p;}

  /// Sets the probability of each success
  void probability(double _p)
	{p=_p;}

  /// Returns the number of trials
  int num_trials()
	{return num;}

  /// Sets the number of trials
  void num_trials(int _num)
	{num=_num;}

  /**
   * Return an integer value generated at random with parameters \a p_
   * and \a num_.
   */
  int operator()(double p_, int num_)
	{
	double tmp_p=p;
	int tmp_num=num;
	p = p_;
	num = num_;
	int ans = operator()();
	p=tmp_p;
	num=tmp_num;
	return ans;
	}

  /// Return an integer value generated at random.
  int operator()();

protected:

  /// Reset the generator
  void reset_generator()
	{uvar.generator(&pGenerator);}

  /// The probability of success for each trial
  double p;

  /// The number of trials
  int num;

  /// Used to generate the r.v.
  Uniform uvar;

};


inline int Binomial::operator()()
{
if (!pGenerator)
   EXCEPTION_MNGR(runtime_error, "Binomial::operator() : Attempting to use a NULL RNG.");

if (num == 0)
   EXCEPTION_MNGR(runtime_error, "Binomial::operator() : The parameter 'n' is zero.");

int temp = 0;
for (int i=0; i<num; i++)
  temp += (uvar() < p);
return(temp);
}

} // namespace utilib

#endif

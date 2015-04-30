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
 * \file Cauchy.h
 *
 * Defines the utilib::Cauchy class
 */

#ifndef utilib_Cauchy_h
#define utilib_Cauchy_h

#include <utilib/RandomVariable.h>
#include <utilib/Random.h>

namespace utilib {


/**
 * Class that generates Cauchy (Lorentian) deviates.
 * \sa ranlib.
 */
class Cauchy: public ExternalRandomVariable<double> {

public:

  /// Constructor
  Cauchy() : alpha(0.0), beta(1.0) {}

  /// Constructor templated on rng type
  template <class RNGT>
  explicit Cauchy(RNGT* gen, double alpha_=0.0, double beta_=1.0)
	: ExternalRandomVariable<double>()
	{ generator(gen); alpha=alpha_; beta=beta_;}

  /// Cauchy alpha parameter.
  double alpha;

  /// Cauchy beta parameter.
  double beta;

  /// Generate an instance of this random variable
  double operator()(double alpha_, double beta_)
	{
	double tmp_alpha=alpha;
	double tmp_beta=beta;
	alpha=alpha_;
	beta=beta_;
	double ans = ExternalRandomVariable<double>::operator()();
	alpha=tmp_alpha;
	beta=tmp_beta;
	return ans;
	}

  ///
  double operator()()
	{return ExternalRandomVariable<double>::operator()();}

protected:

  /// Evaluate a C function to compute the random number generator
  double eval_c_func()
	{ return rcauchy(alpha,beta); }

};

} // namespace utilib

#endif

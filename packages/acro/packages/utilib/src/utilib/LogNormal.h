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
 * \file LogNormal.h
 *
 * Defines the utilib::LogNormal class
 */

#ifndef utilib_LogNormal_h
#define utilib_LogNormal_h

#include <utilib_config.h>
#include <utilib/RandomVariable.h>
#include <utilib/Random.h>

namespace utilib {


/** 
 * A class that generates log-normal deviates with a given scale 
 *	and shape
 */
class LogNormal: public ExternalRandomVariable<double>
{
public:

  /// Constructor
  LogNormal() : scale(1.0), shape(1.0) {}

  /// Templated constructor with rng
  template <class RNGT>
  LogNormal(RNGT* gen, double _m=0.0, double _s=1.0)
	: ExternalRandomVariable<double>()
	{
	generator(gen);
	this->scale = _m;
	this->shape = _s;
	}

  /// Scale parameter
  double scale;

  /// Shape parameter
  double shape;

  /// Generate an instance of this random variable
  double operator()(double _scale, double _shape)
        {
	double tmp_scale = scale;
	double tmp_shape = shape;
	scale=_scale;
	shape=_shape;
	double ans = ExternalRandomVariable<double>::operator()();
	scale=tmp_scale;
	shape=tmp_shape;
	return ans;
        }

  ///
  double operator()()
	{return ExternalRandomVariable<double>::operator()();}

protected:

  ///
  double eval_c_func()
	{ return scale*std::exp(shape*gennor(0.0,1.0)); }

};

} // namespace utilib

#endif

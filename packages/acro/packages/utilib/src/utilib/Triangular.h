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
 * \file Triangular.h
 *
 * Defines the utilib::Triangular class
 */

#ifndef utilib_Triangular_h
#define utilib_Triangular_h

#include <utilib_config.h>
#include <utilib/_math.h>
#include <utilib/Uniform.h>

namespace utilib {

using std::min;
using std::max;

#if !defined(DOXYGEN)
#define PROBABILITY_BALANCED_TRIANGULAR	0
#define EXPECTATION_BALANCED_TRIANGULAR 1
#define TRUNCATED_TRIANGULAR		2
#endif


/**
 * Implements the standard triangular distribution, as well as a number of
 * variants that adapt this distribution in the presence of bound
 * constraints.
 *
 * \sa ranlib.
 */
class Triangular: public SimpleRandomVariable<double>
{
public:

  /// Constructor
  Triangular() : scale(1.0), trunc_flag(TRUNCATED_TRIANGULAR) {}

  /// Templated constructor
  template <class RNGT>
  Triangular(RNGT* gen, double _scale=1.0, 
		int _trunc_flag=TRUNCATED_TRIANGULAR)
   	: SimpleRandomVariable<double>(), urnd(), scale(_scale), 
	  trunc_flag(_trunc_flag)
	{
	generator(gen);
	urnd.generator(gen);
	}

  /// Return a triangular r.v. given upper and lower bounds.
  double operator()(const double lbound, const double rbound);

  ///
  double operator()()
	{return operator()(-HUGE_VAL,HUGE_VAL);}

  /// Set the truncation flag.
  void truncation(const int flag)
	{ trunc_flag = flag; }

protected:

  ///
  void reset_generator()
        {urnd.generator(&pGenerator);}

  /// The Uniform r.v. used by this distribution.
  Uniform urnd;

  /// TODO.
  double scale;

  /// TODO.
  int trunc_flag;
};


//
// The area of a half-triangular distribution with scale 'scale' that is
// truncated at 'c'.
//
#if !defined(DOXYGEN)
#define Area(c)	((2.0 - (c)/scale)*(c)/scale)
#endif



inline double Triangular::operator()(const double lbound, const double ubound)
{
if (!pGenerator)
   EXCEPTION_MNGR(runtime_error, "Triangular::operator() -- Attempting to use a NULL RNG.");

if (lbound > 0.0)
   EXCEPTION_MNGR(runtime_error, "Triangular::operator -- lower bound must be negative!");

if (ubound < 0.0)
   EXCEPTION_MNGR(runtime_error, "Triangular::operator -- upper bound must be positive!");

double lrange = min(scale,-lbound);
double urange = min(scale,ubound);

//
// Simple triangular distribution
//
if ((lrange == scale) && (urange == scale))
   return (scale*(urnd() + urnd() - 1.0));

double value;
switch (trunc_flag) {
  case PROBABILITY_BALANCED_TRIANGULAR:
	if (urnd() < 0.5)
           value = (- scale * (1.0 - std::sqrt(1.0 - Area(lrange)*urnd())));
	else
           value = (  scale * (1.0 - std::sqrt(1.0 - Area(urange)*urnd())));
	break;

  case EXPECTATION_BALANCED_TRIANGULAR:
	{
	double uArea = Area(urange);
	double lArea = Area(lrange);
	double A = (1 - (2 * lrange)/(3 * scale))
			* lrange * lrange / (scale * lArea);
	double B = (1 - (2 * urange)/(3 * scale))
			* urange * urange / (scale * uArea);
	double lambda = B/(A+B);
    if (urnd() < lambda)
           value = (- scale * (1.0 - std::sqrt(1.0 - Area(lrange)*urnd())));
	else
           value = (  scale * (1.0 - std::sqrt(1.0 - Area(urange)*urnd())));
	}
	break;

  case TRUNCATED_TRIANGULAR:
	{
	double uArea = Area(urange);
	double lArea = Area(lrange);
	if (urnd() < (lArea/(uArea+lArea)))
           value = (- scale * (1.0 - std::sqrt(1.0 - Area(lrange)*urnd())));
	else
           value = (  scale * (1.0 - std::sqrt(1.0 - Area(urange)*urnd())));
	}
	break;

  default:
	value = 0.0;
  };

return value;
}

} // namespace utilib

#endif

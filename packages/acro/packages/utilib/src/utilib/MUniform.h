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
 * \file MUniform.h
 *
 * Defines the utilib::MUniform class
 */

#ifndef utilib_MUniform_h
#define utilib_MUniform_h

#include <utilib_config.h>
#include <utilib/Uniform.h>
#include <utilib/DoubleVector.h>

namespace utilib {


/**
 * A class that generates multivariate uniform deviates with 
 * given means and ranges.
 */
class MUniform: public GeneralRandomVariable<DoubleVector>
{
public:

  /// Empty constructor.
  MUniform()
	{ range_const = 1.0; }

  /// Constructor with means and a vector of ranges.
  template <class RNGT>
  MUniform(RNGT* gen, DoubleVector& mean_, DoubleVector& range_vec_)
	: GeneralRandomVariable<DoubleVector>()
	{
	generator(gen);
	unif_dev.generator(gen);
	set(mean_, range_vec_);
	}

  /// Constructor with means and a constant vector of ranges.
  template <class RNGT>
  MUniform(RNGT *gen, DoubleVector& mean_, double range_const_)
	: GeneralRandomVariable<DoubleVector>()
	{
	generator(gen);
	unif_dev.generator(gen);
	set(mean_, range_const_);
	}

  /// Dummy virtual destructor
  virtual ~MUniform() {}

  /// Set the means and vector of ranges.
  void set(DoubleVector& mean, DoubleVector& range_vec_);

  /// Set the means and constant vector of ranges.
  void set(DoubleVector& mean, double range_const_);

  ///
  DoubleVector& operator()()
	{
	operator()(results);
	return results;
	}

  /// Generate an instance of this random variable
  void operator()(DoubleVector& new_vec);

protected:

  ///
  void reset_generator()
        {unif_dev.generator(&pGenerator);}

  /// A Uniform r.v. used by this class.
  Uniform unif_dev;

  /// The vector of means.
  DoubleVector mean;

  /// The vector of ranges.
  DoubleVector range_vec;

  /// The constant vector of ranges.
  double range_const;

};


inline void MUniform::operator()(DoubleVector& new_result)
{
if (!pGenerator)
   EXCEPTION_MNGR(runtime_error, "MUniform::operator() : Attempting to use a NULL generator.");

new_result.resize(results.size());
for (size_type ii=0; ii<new_result.size(); ii++)
  if (range_const == -1)
     new_result[ii] = unif_dev()*range_vec[ii];
  else
     new_result[ii] = unif_dev()*range_const;

new_result += mean;
}


inline void MUniform::set(DoubleVector& mean_, double range_const_)
{
this->mean.resize(mean_.size());
this->mean << mean_;
range_const = range_const_;
results.resize(mean_.size());
}


inline void MUniform::set(DoubleVector& mean_, DoubleVector& range_vec_)
{
if (mean_.size() != range_vec_.size())
   EXCEPTION_MNGR(runtime_error, "MUniform::set : Inconsistent size between mean vector (" << mean_.size() << ") and range vector (" << range_vec_.size() << ")");

range_const = -1;

mean.resize(mean_.size());
mean << mean_;
range_vec.resize(mean_.size());
range_vec << range_vec_;
results.resize(mean_.size());
}

} // namespace utilib

#endif

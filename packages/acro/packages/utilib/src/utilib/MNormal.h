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
 * \file MNormal.h
 *
 * Defines the utilib::MNormal class
 */

#ifndef utilib_MNormal_h
#define utilib_MNormal_h

#include <utilib_config.h>
#include <utilib/_math.h>
#include <utilib/Normal.h>
#include <utilib/DoubleMatrix.h>
#include <utilib/DoubleVector.h>

namespace utilib {


/**
 * A class that generates multivariate normal (Gaussian) deviates with 
 * given covariance matrix and vector of means.
 */
class MNormal: public GeneralRandomVariable<DoubleVector>
{
public:

  /// Constructor
  MNormal() { const_covariance = 1.0; }

  /// Constructor templated on rng, with single mean/covariance values
  template <class RNGT>
  MNormal(RNGT* gen, int ndim=0, double meanval=0.0, double covval=1.0)
	: GeneralRandomVariable<DoubleVector>() 
	{
	generator(gen);
	norm_rng.generator(gen); 
	set(ndim,meanval,covval);
	}

  /// Constructor templated on rng, with vector/matrix mean/covariance values
  template <class RNGT>
  MNormal(RNGT *gen, DoubleVector& mean_vec, DoubleMatrix& covariance)
	: GeneralRandomVariable<DoubleVector>()
	{
	generator(gen);
	norm_rng.generator(gen);
	set(mean_vec, covariance);
	}

  /// Constructor templated on rng, with vector/diagonal mean/covariance values
  template <class RNGT>
  MNormal(RNGT *gen, DoubleVector& mean_vec, DoubleVector& covariance)
	: GeneralRandomVariable<DoubleVector>()
	{
	generator(gen);
	norm_rng.generator(gen);
	set(mean_vec, covariance);
	}

  /// Constructor templated on rng, with vector mean and constant 
  /// diagonal covariance
  template <class RNGT>
  MNormal(RNGT *gen, DoubleVector& mean_vec, double covariance)
	: GeneralRandomVariable<DoubleVector>()
	{
	generator(gen);
	norm_rng.generator(gen);
	set(mean_vec, covariance);
	}

  /// Set the means and the matrix of covariances
  void set(DoubleVector& mean_vec, DoubleMatrix& covariance);

  /// Set the means and the diagonal covariance matrix
  void set(DoubleVector& mean_vec, DoubleVector& covariance);

  /// Set the means and the constant diagonal covariance matrix
  void set(DoubleVector& mean_vec, double covariance);

  /// Set the means and the constant diagonal covariance matrix
  void set(int dimension, double mean_val=0.0, double cov_val=1.0)
	{
	results.resize(dimension);
	mean_vec.resize(dimension);
	mean_vec << mean_val;
	const_covariance = std::sqrt(cov_val);
	}

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
	{ norm_rng.generator(&pGenerator); }

  /// The Normal r.v. used by this class.
  Normal norm_rng;

  /// The covariance matrix.
  DoubleMatrix CF_matrix;

  /// The vector of means.
  DoubleVector mean_vec;

  /// The diagonal covariance matrix.
  DoubleVector diag_covariance;

  /// The constant diagonal covariance matrix.
  double const_covariance;

  /// TODO.
  int pd_flag;

};


inline void MNormal::set(DoubleVector& _mean_vec, double covariance)
{
mean_vec.resize(_mean_vec.size());
mean_vec << _mean_vec;
const_covariance = std::sqrt(covariance);
results.resize(mean_vec.size());
}


inline void MNormal::set(DoubleVector& _mean_vec, DoubleVector& covariance)
{
if (_mean_vec.size() != covariance.size())
   EXCEPTION_MNGR(runtime_error, "MNormal::set : Inconsistent size between mean_vec vector (" << _mean_vec.size() << ") and covariance vector (" << covariance.size() << ")");

const_covariance = -1;

mean_vec.resize(_mean_vec.size());
mean_vec << _mean_vec;
diag_covariance.resize(mean_vec.size());
diag_covariance << covariance;
lapply(diag_covariance,(double (*)(double)) std::sqrt);
results.resize(mean_vec.size());
}


inline void MNormal::set(DoubleVector& _mean_vec, DoubleMatrix& covariance)
{
if ((_mean_vec.size() != covariance.nrows()) ||
    (_mean_vec.size() != covariance.ncols()))
   EXCEPTION_MNGR(runtime_error, "MNormal::set : Inconsistent size between mean_vec vector (" << _mean_vec.size() << ") and covariance matrix (" << covariance.nrows() << "," << covariance.ncols() << ")");

const_covariance = -1;

mean_vec.resize(_mean_vec.size());
mean_vec << _mean_vec;
results.resize(mean_vec.size());
CF_matrix.resize(this->mean_vec.size(), this->mean_vec.size());

for (size_type i=0; i<this->mean_vec.size(); i++)
  for (size_type j=0; j<=i; j++)
    CF_matrix[i][j] = covariance[i][j];

pd_flag = cholesky(CF_matrix, CF_matrix, mean_vec.size());
}


inline void MNormal::operator()(DoubleVector& new_result)
{
if (!pGenerator)
   EXCEPTION_MNGR(runtime_error, "MNormal::operator() : Attempting to use a NULL generator.");

if (pd_flag == ERR)
   EXCEPTION_MNGR(runtime_error, "MNormal::operator() : Attempting to use a bad correlation matrix.");

if (results.size() == 0)
   EXCEPTION_MNGR(runtime_error, "MNormal::operator() : Attempting to use with zero-dimensional arrays.");
   
new_result.resize(results.size());
for (size_type ii=0; ii<new_result.size(); ii++)
  new_result[ii] = norm_rng();

if (CF_matrix.ncols() == 0) {
   if (const_covariance == -1)
      new_result *= diag_covariance;
   else
      new_result *= const_covariance;
   }
else {
   double tmp;
   for (size_type j=new_result.size()-1; ; j--) {
     tmp = 0.0;
     for (size_type k=0; k<=j; k++)
       tmp += new_result[k]*CF_matrix[j][k];
     new_result[j] = tmp;
     if (j == 0) break;
     }
   }
new_result += mean_vec;
}

} // namespace utilib

#endif

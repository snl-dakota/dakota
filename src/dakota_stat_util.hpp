/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Classes     : BootstrapSamplerBase, BootstrapSampler, BootstrapSamplerWithGS
//- Description : Functors for performing bootstrap sampling on a dataset
//- Owner       : Carson Kent
//- Checked by  :
//- Version     :

#ifndef __DAKOTA_STAT_UTIL_H__
#define __DAKOTA_STAT_UTIL_H__

#include <iostream>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include "Teuchos_SerialDenseHelpers.hpp"

#include "dakota_data_types.hpp"

namespace Dakota {

/// Base class/interface for the bootstrap sampler

/** BootstrapSamplerBase defines the minimum interface for a bootstrap sampler
    and handles initialization of the random variate generation used by the
    bootstrap. Functor is templated on the data type, but does not actually
    define a data member. */
template<typename Data>
class BootstrapSamplerBase
{
public:

  //
  //- Heading: Type definitions and aliases
  //

  /// Used for size and indexing
  typedef unsigned int uint;

  //
  //- Heading: Static members for access to random variate generation
  //

  static void set_seed(uint seed)
  {
    bootstrapRNG.seed(seed);
  }

  //
  //- Heading: Constructors and destructor
  //

  /// Constructor for the bootstrap functor base
  BootstrapSamplerBase(uint data_size) : dataSize(data_size),
                                         boostrapSampler(0, data_size - 1)
  {
  }

  /// Destructor
  virtual ~BootstrapSamplerBase()
  {
    /* empty destructor */
  }

  //
  //- Heading: Public members functions that perform for boostrap sampling
  //

  /// Generate and store a new boostrapped sample into bootstrapped_sample
  virtual void operator()(uint num_samp, Data& bootstrapped_sample) = 0;

  /// Generate and store an dataSize out of dataSize boostrap sample
  virtual void operator()(Data& bootstrapped_sample)
  {
    (*this)(dataSize, bootstrapped_sample);
  }

protected:

  // Internal static members for random variate generation

  /// Random number generator to use for sampling
  static boost::random::mt19937 bootstrapRNG;

  // Internal instance members

  /// Size of the dataset defining the empirical distribution
  const uint dataSize;

  /// Uniform distribution to provide samples from the empirical distribution
  boost::random::uniform_int_distribution<> boostrapSampler;

};

/// The boostrap random number generator
template<typename Data>
boost::random::mt19937 BootstrapSamplerBase<Data>::bootstrapRNG;


/// Acutal boostrap sampler implementation for common data types

template<typename Data>
class BootstrapSampler : public BootstrapSamplerBase<Data>
{
public:

  //
  //- Heading: Type definitions and aliases
  //

  /// Used for size and indexing
  typedef unsigned int uint;

  //
  //- Heading: Constructors and destructor
  //

  /// Constructor for the sampler. Current requirement is that data_size and
  /// the size of orig_data must agree.
  /// TODO: Provide methods to avoid the specification of data_size
  BootstrapSampler(uint data_size, const Data& orig_data) :
                    BootstrapSamplerBase<Data>::BootstrapSamplerBase(data_size),
                    origData(orig_data)
  {
  }

  /// Destructor
  virtual ~BootstrapSampler()
  {
    /* empty destructor */
  }

protected:

  // Internal instance members

  /// Original data defining the empirical distribution
  /// TODO: Consider if it should be const (breaks Teuchos)
  Data origData;
};


/// Bootstrap sampler that is specialized to allow for the boostrapping of
/// RealMatrix

template<>
class BootstrapSampler<RealMatrix> : public BootstrapSamplerBase<RealMatrix>
{
public:

  /// Constructor for the sampler. Current requirement is that data_size and
  /// the size of orig_data must agree.
  /// TODO: Provide methods to avoid the specification of data_size
  BootstrapSampler(uint data_size, const RealMatrix& orig_data);

  /// Destructor
  virtual ~BootstrapSampler();

  //
  //- Heading: Public members functions that perform boostrap sampling
  //

  /// Generate and store a new boostrapped sample into bootstrapped_sample
  virtual void operator()(uint num_samp, RealMatrix& bootstrapped_sample);

  /// Generate and return a new boostrapped sample
  virtual RealMatrix operator()(uint num_samp);

protected:

  // Internal instance members

  /// Original data defining the empirical distribution
  RealMatrix origData;
};


/// A derived sampler to allow for user specification of the accessor methods

template<typename Data, typename Getter, typename Setter>
class BootstrapSamplerWithGS : public BootstrapSampler<Data>
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// Constructor with extra arguments for the accessor methods
  BootstrapSamplerWithGS(uint data_size,
                   const Data& orig_data,
                   Getter getter_method,
                   Setter setter_method) :
        BootstrapSampler<Data>::BootstrapSampler(data_size, orig_data),
        getterMethod(getter_method),
        setterMethod(setter_method)
  {
  }

  /// Destructor
 virtual  ~BootstrapSamplerWithGS()
  {
    /* empty destructor */
  }

  //
  //- Heading: Public members functions that perform for boostrap sampling
  //

  /// Generate and store a new boostrapped sample into bootstrapped_sample
  /// TODO: bounds checking
  virtual void operator()(uint num_samp, Data& bootstrapped_sample)
  {
    if(num_samp > this->dataSize)
    {
      std::cout << "Warning: number of boostrap samples requested is larger"
                << "than the data size used for the empirical distribution."
                << std::endl;
    }

    for(uint i = 0; i < num_samp; ++i)
    {
      setterMethod(i, getterMethod(this->boostrapSampler(this->bootstrapRNG),
                                   this->origData), bootstrapped_sample);
    }
  }

protected:

  // Internal instance members

  /// Function to obtain a single sample from a Data object. Function should
  /// take a Data object and an unsigned integer corresponding to a sample
  /// index and return the sample
  Getter getterMethod;

  /// Function to place a single sample into a Data object. Function should
  /// take a Data object and an unsigned integer corresponding to the sample
  /// index to set.
  Setter setterMethod;
};


/// Getter method for RealMatrices allowing for boostrapping where several
/// different functions make up an observation
template<unsigned int N>
RealMatrix real_mat_getter_method(int index, const RealMatrix& orig_data)
{
  return RealMatrix(Teuchos::View, orig_data, orig_data.numRows(), N, 0,
                    index * N);
}

/// Setter method for RealMatrices allowing for boostrapping where several
/// different functions make up an observation
template<unsigned int N>
void real_mat_setter_method(int index, RealMatrix bootstrap_sample,
                                  RealMatrix& bootstrapped_samples)
{
  for(int i =0; i < N; ++i)
  {
    // TODO: Change this to single memcpy
    Teuchos::setCol(Teuchos::getCol(Teuchos::View, bootstrap_sample, i),
                    index * (int)N + i, bootstrapped_samples);
  }
}

}

#endif // __DAKOTA_STAT_UTIL_H__

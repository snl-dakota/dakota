/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Classes     : BootstrapSamplerBase, BootstrapSampler, BootstrapSamplerWithGS
//- Description : Functors for performing bootstrap sampling on a dataset
//- Owner       : Brian Adams
//- Checked by  :
//- Version     :

#ifndef __DAKOTA_BOOTSTRAP_SAMPLER_H__
#define __DAKOTA_BOOTSTRAP_SAMPLER_H__


#include <iostream>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include "Teuchos_SerialDenseVector.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"

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
  //- Heading: Static methods for access to random variate generation
  //

  static void set_seed(size_t seed)
  {
    bootstrapRNG.seed(seed);
  }

  //
  //- Heading: Constructors and destructor
  //

  /// Constructor for the bootstrap functor base
  BootstrapSamplerBase(size_t data_size, Data orig_data) : dataSize(data_size),
                          origData(orig_data), sampler(0, data_size - 1)
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
  virtual void operator()(size_t num_samp, Data& bootstrapped_sample) = 0;

  /// Obatin the number of samples used in the empirical distribution
  virtual size_t getDataSize()
  {
    return dataSize;
  }

  /// Generate and store an dataSize out of dataSize boostrap sample
  virtual void operator()(Data& bootstrapped_sample)
  {
    (*this)(dataSize, bootstrapped_sample);
  }

  /// Return boostrapped sample
  virtual Data operator()()
  {
    Data sample(origData);
    (*this)(dataSize, sample);
    return sample;
  }

protected:

  // Internal static members for random variate generation

  /// Random number generator to use for sampling
  static boost::random::mt19937 bootstrapRNG;

  // Internal instance members

  /// Uniform distribution to provide samples from the empirical distribution
  boost::random::uniform_int_distribution<> sampler;

  /// Size of the dataset defining the empirical distribution
  const size_t dataSize;

  /// Original data defining the empirical distribution
  /// TODO: Consider if it should be const (breaks Teuchos)
  Data origData;
};

/// The boostrap random number generator
template<typename Data>
boost::random::mt19937 BootstrapSamplerBase<Data>::bootstrapRNG;


/// Actual boostrap sampler implementation for common data types

/** Template requires the given type to support an STL-like interface, including
    a size method and begin and end methods returning random access iterators */
template<typename Data>
class BootstrapSampler : public BootstrapSamplerBase<Data>
{
public:

  //
  //- Heading: Type definitions and aliases
  //

  using BootstrapSamplerBase<Data>::operator();

  //
  //- Heading: Constructors and destructor
  //

  /// Constructor for the sampler
  BootstrapSampler(const Data& orig_data, size_t block_size = 1) :
    BootstrapSamplerBase<Data>::BootstrapSamplerBase(
      block_size ? orig_data.size()/block_size : orig_data.size(), orig_data),
      blockSize(block_size ? block_size : 1)
  {
    if(block_size &&
      (block_size > this->dataSize || orig_data.size() % block_size != 0))
        throw "Boostrap sampler data size must be a multiple of block size.";
  }

  /// Destructor
  virtual ~BootstrapSampler()
  {
    /* empty destructor */
  }

  //
  //- Heading: Public members functions that perform for boostrap sampling
  //

  /// \copydoc
  virtual void operator()(size_t num_samp, Data& bootstrapped_sample)
  {
    if(num_samp > bootstrapped_sample.size()/blockSize)
      throw
        std::out_of_range("Number of samples exceeds the size of container");

    typename Data::iterator beg_data = this->origData.begin();
    for(typename Data::iterator sample = bootstrapped_sample.begin();
        sample != bootstrapped_sample.end(); sample += blockSize)
    {
      typename Data::iterator beg_block = beg_data + 
	      this->sampler(this->bootstrapRNG) * blockSize;
      for(size_t i = 0; i < blockSize; ++i)
      {
        *(sample + i) = *(beg_block + i);
      }
    }
  }

protected:

  // Internal instance members

  /// Size of the block defining a sample
  size_t blockSize;
};


/// Bootstrap sampler that is specialized to allow for the boostrapping of
/// RealMatrix
template<typename OrdinalType, typename ScalarType>
class BootstrapSampler<Teuchos::SerialDenseMatrix<OrdinalType, ScalarType> > :
  public BootstrapSamplerBase<Teuchos::
                              SerialDenseMatrix<OrdinalType, ScalarType> >
{
public:

  //
  //- Heading: Type definitions and aliases
  //

  /// Convenience definition
  typedef Teuchos::SerialDenseMatrix<OrdinalType, ScalarType> MatType;

  using BootstrapSamplerBase<MatType>::operator();

  /// Constructor for the sampler.
  BootstrapSampler(const MatType& orig_data, size_t block_size = 1) :
    BootstrapSamplerBase<MatType>::BootstrapSamplerBase(
      block_size ? orig_data.numCols()/block_size : orig_data.numCols(),
      orig_data),
      blockSize(block_size ? block_size : 1)
  {
    if(block_size &&
      (block_size > this->dataSize || orig_data.numCols() % block_size != 0))
        throw "Boostrap sampler data size must be a multiple of block size.";
  }

  /// Destructor
  virtual ~BootstrapSampler()
  {
    /* empty destructor */
  }

  //
  //- Heading: Public members functions that perform boostrap sampling
  //

  /// \copydoc
  virtual void operator()(size_t num_samp, MatType& bootstrapped_sample)
  {
    OrdinalType stride = this->origData.stride();
    if(stride != bootstrapped_sample.stride())
      throw
          std::out_of_range("Dimension of a boostrapped sample differs from "
                            "the dimension of the original dataset");

    if(num_samp > bootstrapped_sample.numCols()/blockSize)
      throw
        std::out_of_range("Number of samples exceeds the size of container");

    for(int i = 0; i < num_samp * blockSize; i += blockSize)
    {
      std::memcpy(bootstrapped_sample[i],
        this->origData[this->sampler(this->bootstrapRNG) * blockSize],
        blockSize * stride * sizeof(ScalarType));
    }
  }

protected:

  // Internal instance members

  /// Size of the block defining a sample
  size_t blockSize;
};


/// A derived sampler to allow for user specification of the accessor methods
template<typename Data, typename Getter, typename Setter>
class BootstrapSamplerWithGS : public BootstrapSampler<Data>
{
public:

  //
  //- Heading: Type definitions and aliases
  //

  using BootstrapSampler<Data>::operator();

  //
  //- Heading: Constructors and destructor
  //

  /// Constructor with extra arguments for the accessor methods
  BootstrapSamplerWithGS(const Data& orig_data,
                         Getter getter_method,
                         Setter setter_method) :
                         BootstrapSampler<Data>::BootstrapSampler(orig_data),
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
  virtual void operator()(size_t num_samp, Data& bootstrapped_sample)
  {
    for(size_t i = 0; i < num_samp; ++i)
    {
      setterMethod(i, getterMethod(this->sampler(this->bootstrapRNG),
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

}

#endif // __DAKOTA_BOOTSTRAP_SAMPLER_H__

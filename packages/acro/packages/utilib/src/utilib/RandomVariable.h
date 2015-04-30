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
 * \file RandomVariable.h
 *
 * Defines the utilib::RandomVariableBase, utilib::SimpleRandomVariable,
 * utilib::GeneralRandomVariable, and utilib::ExternalRandomVariable 
 * classes
 */

#ifndef utilib_RandomVariable_h
#define utilib_RandomVariable_h

#include <utilib/std_headers.h>
#include <utilib/exception_mngr.h>
#include <utilib/AnyRNG.h>

namespace utilib {

/// Function for defining a global random number generator
extern void set_global_RNG(AnyRNG* rng);


/**
 * Abstract class for random variables.
 */
class RandomVariableBase
{
public:

  /// Constructor
  RandomVariableBase() : pGenerator() {}

  /// Destructor
  virtual ~RandomVariableBase() {}

  /// Returns the rng used by this instance
  const AnyRNG& generator() const
	{ return pGenerator ; }

  /// Returns the rng used by this instance
  AnyRNG& generator()
	{ return pGenerator ; }

  /// Set the rng used by this instance
  template <class RNGT>
  void generator(RNGT *p)
	{ pGenerator = p; reset_generator(); }

  /// Set the rng used by this instance
  void generator(AnyRNG* p)
	{ pGenerator = *p; reset_generator(); }

protected:

  /// The rng used by this instance
  AnyRNG pGenerator;

  /// Reset the generator
  virtual void reset_generator() {}

};



/**
 * A random variable class that generates values in the given
 * templated domain, returning them by value
 */
template <class DomainT>
class SimpleRandomVariable : public RandomVariableBase
{
public:

  /// Destructor
  virtual ~SimpleRandomVariable() {}

  /// Operator which generates a random value.
  virtual DomainT operator()() = 0;

};



/**
 * A random variable class that generates values in the given
 * templated domain, returning them by reference
 */
template <class DomainT>
class GeneralRandomVariable : public RandomVariableBase
{
public:

  /// Destructor
  virtual ~GeneralRandomVariable() {}

  /// Operator which generates a random value.
  virtual DomainT& operator()() = 0;

protected:

  /// Local copy of the results
  DomainT results;

};



/**
 * A class that extends the SimpleRandomVariable class to perform the 
 * random number generation with an external function.
 */
template <class DomainT>
class ExternalRandomVariable : public SimpleRandomVariable<DomainT>
{
public:

  /// Destructor
  virtual ~ExternalRandomVariable() {}

  /// Generate an instance of this random variable
  DomainT operator()();

private:

  /// The external function used by this r.v.
  virtual DomainT eval_c_func() = 0;

};


template <class DomainT>
DomainT ExternalRandomVariable<DomainT>::operator()()
{
if (!(this->pGenerator))
   EXCEPTION_MNGR(std::runtime_error, "ExternalRandomVariable::operator() : Attempting to use a NULL generator.");

set_global_RNG(&(this->pGenerator));
return eval_c_func();
}


} // namespace utilib

#endif

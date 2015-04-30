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
 * \file RNG.h
 *
 * Defines the utilib::RNG class.
 */

#ifndef utilib_RNG_h
#define utilib_RNG_h

#include <utilib_config.h>
#include <utilib/std_headers.h>

namespace utilib {

#ifndef UTILIB_SEED_TYPE
#define UTILIB_SEED_TYPE
typedef unsigned int seed_t;
#endif

/**
 * Abstract class for random number generators.  Adapted from GNU class
 *	by Dirk Grunwald.
 */
class RNG
{
public:
  /// Empty virtual destructor to silence compiler warnings
  virtual ~RNG() {}

  /// Return a long in 0 ... LONG_MAX.
  virtual unsigned long asLong() = 0;

  /** Reset the number generator to prepare it for use.
    * If the initial see was 'random', this resets the generator
    * to the value of that initial seed.
    */
  virtual void reset() = 0;

  /// Return float in [0,1[ 
  virtual float asFloat();

  /// Return double in [0,1[ 
  virtual double asDouble();

  /// Write the rng state to an output stream.
  virtual void write(std::ostream& output) const = 0;

  /// Read an rng state from an input stream.
  virtual void read(std::istream& input) = 0;

  /// A generic API for setting seeds for the RNG
  virtual void set_seed(seed_t seed=0) = 0;

  /// A generic API for getting seeds for the RNG
  virtual seed_t get_seed() const = 0;
   
};


inline double RNG::asDouble()
{
double ans = static_cast<double>(asLong())/ static_cast<double>(LONG_MAX);
return ans;
}


inline float RNG::asFloat()
{
float  ans = (float) asDouble();
return ans;
}

} // namespace utilib

/// Write a RNG object state
inline std::ostream& operator << (std::ostream& output, const utilib::RNG& rng)
{
rng.write(output);
return(output);
}

/// Read a RNG object state
inline std::istream& operator >> (std::istream& input, utilib::RNG& rng)
{
rng.read(input);
return(input);
}

#endif

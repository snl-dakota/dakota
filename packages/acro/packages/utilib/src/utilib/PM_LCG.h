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
 * \file PM_LCG.h
 *
 * Defines the utilib::PM_LCG class.
 */

#ifndef utilib_PM_LCG_h
#define utilib_PM_LCG_h

#include <utilib_config.h>
#include <utilib/RNG.h>

namespace utilib {


/**
 * A portable linear congruential generator based on the rng developed by
 * Park and Miller.  This rng is implemented in \c PMrand function.
 */
class PM_LCG : public RNG
{
public:

  /** Constructor.
   * If no argument is provided, the seed is set to zero, so the rng
   * is initialized with the current time information.
   */
  PM_LCG(seed_t seed = 0) {jseed = seed; reset();}

  /// Empty virtual destructor to silence compiler warnings
  virtual ~PM_LCG() {}

#if 0
  /** Reseed the rng with a new seed value.
   *  A seed value with generate a 'random' initial seed.
   */
  void reseed(int seed=0) {jseed = seed; reset();}

  /// Returns the seed used to initialize the rng.
  int getSeed() {return jseed;}
#endif

  ///
  void set_seed(seed_t seed=0) {jseed = seed; reset();}

  ///
  seed_t get_seed() const {return jseed;}

  /// Returns the state of the LCG 
  int getState() {return state;}

  ///
  void reset();

  ///
  unsigned long asLong();

  ///
  double asDouble();

  ///
  void write(std::ostream& output) const;

  ///
  void read(std::istream& input);

private:

  /// The state for this rng.
  int state;

  /**
   * Use to initialize the generator.
   * If zero, time is used to generate the random
   * seed.
   */
  seed_t jseed;

};

} // namespace utilib

#endif

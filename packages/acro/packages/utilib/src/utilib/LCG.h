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
 * \file LCG.h
 *
 * Defines the utilib::LCG class.
 */

#ifndef utilib_LCG_h
#define utilib_LCG_h

#include <utilib_config.h>
#include <utilib/RNG.h>

namespace utilib {

/**
 * A linear congruential random number generator class.
 * This is a simple wrapper for the Unix nrand48 routine, and thus it is
 * somewhat portable.
 */
class LCG : public RNG {

public:

  /**
   * Constructor.
   * If no argument is provided, the seed is set to zero, so the rng
   * is initialized with the current time information.
   */
  LCG(unsigned short seed = 0);

  /// Reseed the rng with a new seed value.
  void reseed(unsigned seed);

  /// Returns the seed used to initialize the rng.
  unsigned short getSeed() { return Seed; }

  /// 
  void reset();

  ///
  unsigned long	asLong();

/*
** TODO: Is the issue const support?  The AC_C_CONST
** has taken care of this.
*/
#if defined (COUGAR) || defined(CPLANT) || defined(DOXYGEN)
  /// Write this object
  virtual void write(ostream& output) const;

  /// Read this object
  virtual void read(istream& input);
#else
  /// Write this object
  virtual void write(std::ostream& output) const;
  /// Read this object
  virtual void read(std::istream& input);
#endif

private:

  /// The state for this rng.
  unsigned short State[3];

  /**
   * Use to initialize the generator.
   * If zero, time is used to generate the random
   * seed.
   */
  unsigned short Seed;

};

} // namespace utilib

#endif

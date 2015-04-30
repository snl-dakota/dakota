/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

/**
 * \file KeyGenerator_EpsilonMatch.h
 *
 * Defines the colin::KeyGenerator_EpsilonMatch class.
 */

#ifndef colin_KeyGenerator_EpsilonMatch_h
#define colin_KeyGenerator_EpsilonMatch_h

#include <acro_config.h>

#include <colin/Cache.h>

namespace colin
{

/** 
 *  \c KeyGenerator_EpsilonMatch provides a cache key that performs
 *  approximate (epsilon) matching for domain points.  \c
 *  KeyGenerator_EpsilonMatch can be used for any problem, provided that
 *  the \c TypeManager() knows a lexical cast path from the
 *  application's native domain to a \c colin::MixedIntVars.
 *
 *  <B>WARNING</B>: \c KeyGenerator_EpsilonMatch matches floating point
 *  domain values that are within epsilon bits of each other.  The first
 *  value encountered by the Cache will be the domain point used for
 *  that domain region, and thus will not necessarily be centered in the
 *  region.  While perhaps not the best approach, it avoids a
 *  requirement for a cast route from \c MixedIntVars to the actual
 *  domain.
 *
 *  Rounding the floating point values (doubles) is done using a
 *  home-grown binary round-to-even method.  This routine rounds off the
 *  last N bits of the double in a statistically valid manner without
 *  performing "*" or "/" operations (and at most 1 "+" or "-" ).  N can
 *  be selected to be anything in the range [0, bits_in_char).  If 0,
 *  then no rounding occurs, and \c KeyGenerator_EpsilonMatch reverts to
 *  a slower version of \c KeyGenerator_ExactMatch.
 *
 *  The use of this "round-then-compare" method is superior to the
 *  typical "compare within a tolerance" method as it preserves the
 *  transitive property of the equality comparison.  (i.e. for the
 *  "compare within a tolerance" method, if A == B and B == C, it is not
 *  guaranteed that A == C).
 */
class KeyGenerator_EpsilonMatch : public Cache::KeyGenerator
{
public:
   /// Constructor
   KeyGenerator_EpsilonMatch();

   /// Destructor
   virtual ~KeyGenerator_EpsilonMatch();

   /// Set the epsilon factor (in bits). Valid range [0,8); Default = 2.
   void set_epsilon(int epsilon);

   /// Generate the indexing key from the domain point
   virtual Cache::Key operator()(const utilib::Any domain);

private:
   struct Implementation;
   ///
   Implementation* data;
};


} // namespace colin

#endif // defined colin_KeyGenerator_EpsilonMatch_h

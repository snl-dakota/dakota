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
 * \file KeyGenerator_EpsilonMatch.cpp
 *
 * Implements the colin::KeyGenerator_EpsilonMatch class.
 */

#include <colin/cache/KeyGenerator_EpsilonMatch.h>
#include <colin/cache/Factory.h>

#include <utilib/MixedIntVars.h>

#include <cmath>
#include <limits>

using std::cerr;
using std::endl;

using std::map;
using std::list;
using std::pair;
using std::string;

using utilib::Any;
using utilib::MixedIntVars;

namespace {

/// A wrapper around a MixedIntVars for storing the "rounded domain"
struct EpsilonMixedDomain {
   MixedIntVars domain;

   bool operator==(const EpsilonMixedDomain& rhs) const
   { return domain == rhs.domain; }
   bool operator<(const EpsilonMixedDomain& rhs) const
   { return domain < rhs.domain; }
};

} // namespace (local)

namespace utilib {
DEFINE_DEFAULT_ANY_COMPARATOR(EpsilonMixedDomain);
} // namespace utilib

namespace colin {

namespace StaticInitializers {

namespace {

/// Function to create a new KeyGenerator_EpsilonMatch
Cache::KeyGenerator* create_epsilon_indexer()
{
   return new KeyGenerator_EpsilonMatch;
}

bool RegisterEpsilonMatch()
{
   CacheFactory().declare_indexer_type("Epsilon", create_epsilon_indexer);
   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool epsilon_match = RegisterEpsilonMatch();

} // namespace colin::StaticInitializers




//================================================================
// KeyGenerator_EpsilonMatch::Implementation Definition
//================================================================

struct KeyGenerator_EpsilonMatch::Implementation {
   // initialize the rounding functionality
   Implementation();
   /// Set the epsilon factor (in bits). Valid range [0,8); Default = 2.
   void set_epsilon(int epsilon);
   /// utility function for rounding the elements in the domain
   void roundRealDomain(utilib::MixedIntVars &domain) const;


   /// The offset of the LSB for a double on this platform
   int lsb;
   /// The bit in the lsb at which to round off
   unsigned char precision_bit;
   /// The mask to truncate the lsb for rounding down
   unsigned char trunc_mask;
   /// The mask to test for rounding up
   unsigned char round_up_mask;
   /// The amount to add when rounding up
   double shift;
};


KeyGenerator_EpsilonMatch::Implementation::Implementation()
   : lsb(-1)
{
   double d = 1.0;
   double d1 = ( d + std::numeric_limits<double>::epsilon() );
   unsigned char* c = reinterpret_cast<unsigned char*>(&d);
   unsigned char* c1 = reinterpret_cast<unsigned char*>(&d1);
   for(size_t i = 0; i < sizeof(double); ++i)
   {
      if ( c[i] != c1[i] )
      {
         if ( lsb != -1 )
         { 
            EXCEPTION_MNGR(std::runtime_error, "KeyGenerator_EpsilonMatch(): "
                           "Error: multiple lsb identified (" 
                           << lsb << "," << i << ")");
         }
         lsb = i;
      }
   }

   if ( lsb < 0 )
   {
      EXCEPTION_MNGR(std::runtime_error, "KeyGenerator_EpsilonMatch(): "
                     "Error: radix lsb not identified.  "
                     "Does numeric_limits lie?");
   }

   if ( c[lsb] ^ c1[lsb] != 1 )
   {
      EXCEPTION_MNGR(std::runtime_error, "KeyGenerator_EpsilonMatch(): "
                     "Error: radix lsb not the same as the byte lsb.  "
                     "Please re-think rounding algorithm");
   }

   // default to rounding off the last 2 bits of all doubles
   set_epsilon(2);
}


void 
KeyGenerator_EpsilonMatch::Implementation::set_epsilon(int epsilon)
{
   if (( epsilon < 0 ) || 
       ( epsilon >= std::numeric_limits<unsigned char>::digits ))
   {
      EXCEPTION_MNGR(std::runtime_error, "KeyGenerator_EpsilonMatch::"
                     "set_epsilon(): invalid value of epsilon: " << epsilon 
                     << ", limits = [0," 
                     << std::numeric_limits<unsigned char>::digits << ")" );
   }
   
   shift = std::ldexp(std::numeric_limits<double>::epsilon(), epsilon - 1);

   // check for the trivial case, "no rounding"
   if ( epsilon == 0 )
      precision_bit = 0;
   else
      precision_bit = 1 << ( epsilon - 1 );

   trunc_mask = ~static_cast<unsigned char>(0) << epsilon;
   round_up_mask = ~trunc_mask ^ precision_bit ^ ( precision_bit << 1 );

   /*
   int i;
   cerr << "shift         = " << shift << endl;
   i = precision_bit;
   cerr << "precision_bit = " << i << endl;
   i = trunc_mask;
   cerr << "trunc_mask    = " << i << endl;
   i = round_up_mask;
   cerr << "round_up_mask = " << i << endl;
   */
}


void
KeyGenerator_EpsilonMatch::Implementation
::roundRealDomain(utilib::MixedIntVars &domain) const
{
   int  d_exp = 0;
   double  mantissa = 0;
   unsigned char &c = *(reinterpret_cast<unsigned char*>(&mantissa) + lsb);

   utilib::NumArray<double> &dbl_vec = domain.Real();
   utilib::NumArray<double>::iterator it = dbl_vec.begin();
   utilib::NumArray<double>::iterator itEnd = dbl_vec.end();
   for( ; it != itEnd; ++it )
   {
      mantissa = std::frexp(*it, &d_exp);

      if (( c & precision_bit ) && ( c & round_up_mask ))
      {
         c &= trunc_mask;
         if ( *it < 0 )
            mantissa -= shift;
         else
            mantissa += shift;
      }
      else
         c &= trunc_mask;

      // reassemble the rounded double
      *it = std::ldexp(mantissa, d_exp);
   }
}


//================================================================
// KeyGenerator_EpsilonMatch Public members
//================================================================

KeyGenerator_EpsilonMatch::KeyGenerator_EpsilonMatch()
   : data(new Implementation)
{}


KeyGenerator_EpsilonMatch::~KeyGenerator_EpsilonMatch()
{
   delete data;
}


Cache::Key
KeyGenerator_EpsilonMatch::operator()(const utilib::Any domain)
{
   Any ans;
   MixedIntVars &rounded = ans.set<EpsilonMixedDomain>().domain;
   utilib::TypeManager()->lexical_cast(domain, rounded);
   data->roundRealDomain(rounded);
   return form_key(ans);
}

void 
KeyGenerator_EpsilonMatch::set_epsilon(int epsilon)
{
   data->set_epsilon(epsilon);
}

} // namespace colin

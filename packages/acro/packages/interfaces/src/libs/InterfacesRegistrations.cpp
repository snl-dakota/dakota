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

#include <acro_config.h>
#include <interfaces/InterfacesRegistrations.h>

namespace interfaces {

/** See the colin/ColinRegistrations.h header file for 
 *  discussion of this registration strategy.
 */
namespace StaticInitializers {

// solver registrations

#ifdef ACRO_USING_OPTPP
extern const volatile bool OptppNewton1_bool;
#endif
#ifdef ACRO_USING_COBYLA
extern const volatile bool Cobyla_bool;
#endif
#ifdef ACRO_USING_THREEPO
extern const volatile bool Hooke_bool;
#endif
#ifdef ACRO_USING_APPSPACK
extern const volatile bool APPS_bool;
#endif
#ifdef ACRO_USING_NSGA2
extern const volatile bool NSGA2_bool;
#endif
#if defined(ACRO_USING_MOMHLIB) && defined(ACRO_USING_SCOLIB)
extern const volatile bool momh_nsga_bool;
#endif

namespace {

long RegisterAll()
{
   long ans = 0;

   // solver registrations
   #ifdef ACRO_USING_OPTPP
   ans = ans<<1 + (OptppNewton1_bool ? 0 : 1);
   #endif
   #ifdef ACRO_USING_COBYLA
   ans = ans<<1 + (Cobyla_bool ? 0 : 1);
   #endif
   #ifdef ACRO_USING_THREEPO
   ans = ans<<1 + (Hooke_bool ? 0 : 1);
   #endif
   #ifdef ACRO_USING_APPSPACK
   ans = ans<<1 + (APPS_bool ? 0 : 1);
   #endif
   #ifdef ACRO_USING_NSGA2
   ans = ans<<1 + (NSGA2_bool ? 0 : 1);
   #endif
   #if defined(ACRO_USING_MOMHLIB) && defined(ACRO_USING_SCOLIB)
   ans = ans<<1 + (momh_nsga_bool ? 0 : 1);
   #endif

   //if ( ans != 0 )
   //   std::cerr << "WARNING: Incomplete Acro Interfaces registrations: " 
   //             << ans << std::endl;

   return ans;
}

} // namespace interfaces::StaticInitializers::(local)

#ifdef ACRO_USING_DYNAMIC_LIBRARIES
extern const volatile long static_interfaces_registrations = -1;
#else
extern const volatile long static_interfaces_registrations = RegisterAll();
#endif

} // namespace interfaces::StaticInitializers
} // namespace interfaces

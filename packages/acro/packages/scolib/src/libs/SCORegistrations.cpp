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
#include <scolib/SCORegistrations.h>

namespace scolib {

/** See the colin/ColinRegistrations.h header file for 
 *  discussion of this registration strategy.
 */
namespace StaticInitializers {

// solver registrations

extern const volatile bool DIRECT_bool;
extern const volatile bool DockingPS_bool;
extern const volatile bool EAminlp_bool;
//extern const volatile bool MultiStart_bool;
extern const volatile bool MultiStatePS_bool;
extern const volatile bool PatternSearch_bool;
extern const volatile bool PLGO_bool;
extern const volatile bool SolisWets_bool;
extern const volatile bool StateMachineLS_bool;
extern const volatile bool TGPO_bool;
extern const volatile bool PIDOMS_bool;

namespace {

long RegisterAll()
{
   long ans = 0;

   // solver registrations
   ans = ans<<1 + (DIRECT_bool ? 0 : 1);
   ans = ans<<1 + (DockingPS_bool ? 0 : 1);
   ans = ans<<1 + (EAminlp_bool ? 0 : 1);
   //ans &= ans<<1 + (MultiStart_bool ? 0 : 1);
   ans = ans<<1 + (MultiStatePS_bool ? 0 : 1);
   ans = ans<<1 + (PatternSearch_bool ? 0 : 1);
   ans = ans<<1 + (PLGO_bool ? 0 : 1);
   ans = ans<<1 + (SolisWets_bool ? 0 : 1);
   ans = ans<<1 + (StateMachineLS_bool ? 0 : 1);
   ans = ans<<1 + (TGPO_bool ? 0 : 1);
   ans = ans<<1 + (PIDOMS_bool ? 0 : 1);

   //if ( ans != 0 )
   //   std::cerr << "WARNING: Incomplete Acro SCOlib registrations: " 
   //             << ans << std::endl;

   return ans;
}

} // namespace interfaces::StaticInitializers::(local)

#ifdef ACRO_USING_DYNAMIC_LIBRARIES
extern const volatile long static_scolib_registrations = -1;
#else
extern const volatile long static_scolib_registrations = RegisterAll();
#endif

} // namespace interfaces::StaticInitializers
} // namespace scolib

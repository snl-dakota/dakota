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
#include <colin/ColinRegistrations.h>

namespace colin {

/** The StaticInitializers namespace is a place to declare "volatile
 *  const"s that can be used to "trick" the linker into including
 *  additional object data from an archive (static library) when
 *  building an executable.  An archive (i.e. ".a" static library)
 *  contains one or more object files concatenated together.  When the
 *  linker interrogates an archive for symbols, it only includes those
 *  .o components that are referenced (directly or indirectly) by the
 *  main calling code.  This is problematic for codes build around
 *  plugin architectures, as most plugins are code islands: .o files that
 *  are only referenced through a factory and never directly referenced
 *  by the main code.
 *
 *  To guarantee that the linker will include plugins from a static
 *  library, we create extern volatile consts that are declared here and
 *  defined in a code island.  We also define a central registration
 *  variable (StaticInitializers::static_colin_registrations) that is
 *  initialized by referencing the "volatile const"s for all islands.
 *  This way, as long as a component of the main program references
 *  static_colin_registrations, all plugins will be indirectly
 *  referenced and subsequently included in the final object.
 *
 *  It is critical that these consts are declared "volatile": this
 *  prevents the compiler from optimizing the no-op references away (and
 *  defeating the whole point of this exercise).
 *
 *  Currently, static_colin_registrations is referenced by
 *  ApplicationMngr.cpp and ContextMngr.cpp.
 *
 *  \b Note: This is only an issue for executables built using the Colin
 *  static library.  If you build Colin into a dynamic library, all
 *  object files on the link line will be included (and registered) in
 *  the library.  If you want to create a dynamic library with only some
 *  of the plugins that are registered through the
 *  static_colin_registrations link, you will need to break these
 *  automatic references by defining ACRO_USING_DYNAMIC_LIBRARIES and
 *  recompiling this file.
 *
 *  If you ever need to check if the plugins were included using this
 *  trick, the value of static_colin_registrations is true if the
 *  plugins were references through the linkage, and false if the link
 *  was broken.
 */
namespace StaticInitializers {

// application registrations
extern const volatile bool analysis_code;
extern const volatile bool constraint_penalty;
extern const volatile bool downcast;
extern const volatile bool finite_difference;
extern const volatile bool relaxable_mixed_int_domain;
extern const volatile bool sampling;
extern const volatile bool subspace;
extern const volatile bool uncon_multiobj;
extern const volatile bool upcast;
extern const volatile bool weighted_sum;
extern const volatile bool ampl;

// solver registrations
extern const volatile bool gradient_descent;
extern const volatile bool simple_mi_local_search;
extern const volatile bool random_moo;

// evaluator registrations
extern const volatile bool serial_evaluator;
extern const volatile bool concurrent_evaluator;

// cache-related registrations
extern const volatile bool local_cache;
extern const volatile bool master_slave_cache;
extern const volatile bool subset_view;
extern const volatile bool pareto_view;
extern const volatile bool epsilon_match;

// process managers
extern const volatile bool local_process_mngr;
extern const volatile bool mpi_process_mngr;

// execute functors
extern const volatile bool library_loader;


namespace {

bool RegisterAll()
{
   bool ans = true;
   // application registrations
   ans &= analysis_code;
   ans &= constraint_penalty;
   ans &= downcast;
   ans &= finite_difference;
   ans &= relaxable_mixed_int_domain;
   ans &= sampling;
   ans &= subspace;
   ans &= uncon_multiobj;
   ans &= upcast;
   ans &= weighted_sum;
   ans &= ampl;
   
   // solver registrations
   ans &= gradient_descent;
   ans &= simple_mi_local_search;
   ans &= random_moo;

   // evaluator registrations
   ans &= serial_evaluator;
   ans &= concurrent_evaluator;

   // cache-related registrations
   ans &= local_cache;
   ans &= master_slave_cache;
   ans &= subset_view;
   ans &= pareto_view;
   ans &= epsilon_match;

   // process managers
   ans &= local_process_mngr;

   // execute functors
   ans &= library_loader;

   return ans;
}

} // namespace colin::StaticInitializers::(local)

#ifdef ACRO_USING_DYNAMIC_LIBRARIES
extern const volatile bool static_colin_registrations = false;
#else
extern const volatile bool static_colin_registrations = RegisterAll();
#endif

} // namespace colin::StaticInitializers
} // namespace colin

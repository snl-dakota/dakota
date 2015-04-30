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

#include <colin/reformulation/Subspace.h>
#include <colin/ApplicationMngr.h>

using std::pair;
using utilib::Any;

namespace colin {

namespace StaticInitializers {

namespace {

/** This is a specialized casting function that will look at the
 *  incoming problem.  If the problem is relaxable, it will return the
 *  relaxed problem.  Otherwise, it will attempt to instantiate and
 *  return a SubspaceApplication.
 */
template <class FROM, class TO>
int lexical_cast_mixed_to_continuous(const Any& from_, Any& to)
{
   const Problem<FROM>& from = from_.template expose<Problem<FROM> >();
   // If this is a relaxable application, return the relaxed application!
   if ( ! from->relaxed_application().empty() )
   {
      to.template set< Problem<TO> >().set_application
         ( from->relaxed_application()->get_problem()
           .template expose<Application<TO>*>() );
      return 0;
   }

   if ( from->num_int_vars != 0 /*from->get_fixed_int_vars().size()*/ )
   {
      return utilib::Type_Manager::CastError::InvalidSource;
      //EXCEPTION_MNGR(std::runtime_error, "Cannot lexically cast a "
      //               "Mixed Integer problem to a Continuous problem when "
      //               "integer parameters are not all fixed.");
   }
   if ( from->num_binary_vars != 0 /*from->get_fixed_binary_vars().size()*/ )
   {
      return utilib::Type_Manager::CastError::InvalidSource;
      //EXCEPTION_MNGR(std::runtime_error, "Cannot lexically cast a "
      //               "Mixed Integer problem to a Continuous problem when "
      //               "binary parameters are not all fixed.");
   }

   pair<ApplicationHandle,SubspaceApplication<TO>*> tmp
      = ApplicationHandle::create<SubspaceApplication<TO> >();
   tmp.second->reformulate_application( from->get_handle() );
   //tmp.second->set_fixed_vars( from->get_fixed_real_vars(), 
   //                            from->get_fixed_int_vars(), 
   //                            from->get_fixed_binary_vars() );

   to.template set< Problem<TO> >().set_application(tmp.second);
   return 0;
}

template <class FROM, class TO>
int lexical_cast_mixed_to_integer(const Any& from_, Any& to)
{
   const Problem<FROM>& from = from_.template expose<Problem<FROM> >();

   if ( from->num_real_vars != 0 /*from->get_fixed_int_vars().size()*/ )
   {
      return utilib::Type_Manager::CastError::InvalidSource;
      //EXCEPTION_MNGR(std::runtime_error, "Cannot lexically cast a "
      //               "Mixed Integer problem to a Continuous problem when "
      //               "integer parameters are not all fixed.");
   }

   pair<ApplicationHandle,SubspaceApplication<TO>*> tmp
      = ApplicationHandle::create<SubspaceApplication<TO> >();
   tmp.second->reformulate_application( from->get_handle() );
   to.template set< Problem<TO> >().set_application(tmp.second);
   return 0;
}


bool RegisterSubspace()
{
   // ------------------------------------------------------------------
   // Register creatable instances with the ApplicationMngr

   ApplicationMngr().declare_application_type
      <SubspaceApplication<NLP0_problem> >("SubspaceReformulation");

   ApplicationMngr().declare_application_type
      <SubspaceApplication<UNLP0_problem> >("SubspaceReformulation");

   ApplicationMngr().declare_application_type
      <SubspaceApplication<MINLP0_problem> >("SubspaceReformulation");

   ApplicationMngr().declare_application_type
      <SubspaceApplication<MO_MINLP0_problem> >("SubspaceReformulation");

   ApplicationMngr().declare_application_type
      <SubspaceApplication<MO_UMINLP0_problem> >("SubspaceReformulation");

   ApplicationMngr().declare_application_type
      <SubspaceApplication<MO_NLP0_problem> >("SubspaceReformulation");

   ApplicationMngr().declare_application_type
      <SubspaceApplication<MO_UNLP0_problem> >("SubspaceReformulation");

   ApplicationMngr().declare_application_type
      <SubspaceApplication<INLP_problem> >("SubspaceReformulation");

   ApplicationMngr().declare_application_type
      <SubspaceApplication<UINLP_problem> >("SubspaceReformulation");


   // ------------------------------------------------------------------
   // Register default problem transformations with the ProblemMngr

   // NB: We will call these "exact" as they will only work if the
   // problem is either (a) relaxable or (b) has no integer/binary
   // variables.

   ProblemMngr().register_lexical_cast
      (typeid(Problem<MINLP0_problem>), 
       typeid(Problem<NLP0_problem>), 
       &lexical_cast_mixed_to_continuous<MINLP0_problem, NLP0_problem>
       );
   /*
   ProblemMngr().register_lexical_cast
      (typeid(Problem<MINLP1_problem>), 
       typeid(Problem<NLP1_problem>), 
       &lexical_cast_mixed_to_continuous<MINLP1_problem, NLP1_problem>
       );
   ProblemMngr().register_lexical_cast
      (typeid(Problem<MINLP2_problem>), 
       typeid(Problem<NLP2_problem>), 
       &lexical_cast_mixed_to_continuous<MINLP2_problem, NLP2_problem>
       );
   */

   ProblemMngr().register_lexical_cast
      (typeid(Problem<UMINLP0_problem>), 
       typeid(Problem<UNLP0_problem>), 
       &lexical_cast_mixed_to_continuous<UMINLP0_problem, UNLP0_problem>
       );
   /*
   ProblemMngr().register_lexical_cast
      (typeid(Problem<UMINLP1_problem>), 
       typeid(Problem<UNLP1_problem>), 
       &lexical_cast_mixed_to_continuous<UMINLP1_problem, UNLP1_problem>
       );
   ProblemMngr().register_lexical_cast
      (typeid(Problem<UMINLP2_problem>), 
       typeid(Problem<UNLP2_problem>), 
       &lexical_cast_mixed_to_continuous<UMINLP2_problem, UNLP2_problem>
       );
   */

   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_MINLP0_problem>), 
       typeid(Problem<MO_NLP0_problem>), 
       &lexical_cast_mixed_to_continuous<MO_MINLP0_problem, MO_NLP0_problem>
       );
   /*
   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_MINLP1_problem>), 
       typeid(Problem<MO_NLP1_problem>), 
       &lexical_cast_mixed_to_continuous<MO_MINLP1_problem, MO_NLP1_problem>
       );
   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_MINLP2_problem>), 
       typeid(Problem<MO_NLP2_problem>), 
       &lexical_cast_mixed_to_continuous<MO_MINLP2_problem, MO_NLP2_problem>
       );
   */

   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_UMINLP0_problem>), 
       typeid(Problem<MO_UNLP0_problem>), 
       &lexical_cast_mixed_to_continuous<MO_UMINLP0_problem, MO_UNLP0_problem>
       );

   ProblemMngr().register_lexical_cast
      (typeid(Problem<MINLP0_problem>), 
       typeid(Problem<INLP_problem>), 
       &lexical_cast_mixed_to_integer<MINLP0_problem, INLP_problem>
       );

   ProblemMngr().register_lexical_cast
      (typeid(Problem<UMINLP0_problem>), 
       typeid(Problem<UINLP_problem>), 
       &lexical_cast_mixed_to_integer<UMINLP0_problem, UINLP_problem>
       );

   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool subspace = RegisterSubspace();

} // namespace colin::StaticInitializers

} // namespace colin

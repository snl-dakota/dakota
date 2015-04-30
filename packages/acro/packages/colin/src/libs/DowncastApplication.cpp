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

#include <colin/reformulation/Downcast.h>
#include <colin/ApplicationMngr.h>

using std::pair;
using utilib::Any;

namespace colin {

namespace StaticInitializers {

namespace {

template <class FROM, class TO>
int lexical_cast_DowncastProblem(const Any& from, Any& to)
{
   pair<ApplicationHandle,DowncastApplication<TO>*> tmp
      = ApplicationHandle::create<DowncastApplication<TO> >();
   tmp.second->reformulate_application
      ( from.template expose<Problem<FROM> >()->get_handle() );

   to.template set< Problem<TO> >().set_application(tmp.second);
   return 0;
}


bool RegisterDowncast()
{
   // ------------------------------------------------------------------
   // Register creatable instances with the ApplicationMngr

   ApplicationMngr().declare_application_type
      <DowncastApplication<NLP0_problem> >("DowncastReformulation");

   ApplicationMngr().declare_application_type
      <DowncastApplication<NLP1_problem> >("DowncastReformulation");

   ApplicationMngr().declare_application_type
      <DowncastApplication<UNLP0_problem> >("DowncastReformulation");

   ApplicationMngr().declare_application_type
      <DowncastApplication<UNLP1_problem> >("DowncastReformulation");

   ApplicationMngr().declare_application_type
      <DowncastApplication<MINLP0_problem> >("DowncastReformulation");

   ApplicationMngr().declare_application_type
      <DowncastApplication<MINLP1_problem> >("DowncastReformulation");

   ApplicationMngr().declare_application_type
      <DowncastApplication<MO_NLP0_problem> >("DowncastReformulation");

   ApplicationMngr().declare_application_type
      <DowncastApplication<MO_NLP1_problem> >("DowncastReformulation");

   ApplicationMngr().declare_application_type
      <DowncastApplication<MO_UNLP0_problem> >("DowncastReformulation");

   ApplicationMngr().declare_application_type
      <DowncastApplication<MO_UNLP1_problem> >("DowncastReformulation");

   ApplicationMngr().declare_application_type
      <DowncastApplication<MO_MINLP0_problem> >("DowncastReformulation");

   ApplicationMngr().declare_application_type
      <DowncastApplication<MO_MINLP1_problem> >("DowncastReformulation");

   ApplicationMngr().declare_application_type
      <DowncastApplication<MO_UMINLP0_problem> >("DowncastReformulation");

   ApplicationMngr().declare_application_type
      <DowncastApplication<MO_UMINLP1_problem> >("DowncastReformulation");



   // ------------------------------------------------------------------
   // Register default problem transformations with the ProblemMngr

   ProblemMngr().register_lexical_cast
      (typeid(Problem<NLP2_problem>), 
       typeid(Problem<NLP1_problem>), 
       &lexical_cast_DowncastProblem<NLP2_problem, NLP1_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<NLP1_problem>), 
       typeid(Problem<NLP0_problem>), 
       &lexical_cast_DowncastProblem<NLP1_problem, NLP0_problem>);

   ProblemMngr().register_lexical_cast
      (typeid(Problem<MINLP2_problem>), 
       typeid(Problem<MINLP1_problem>), 
       &lexical_cast_DowncastProblem<MINLP2_problem,MINLP1_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<MINLP1_problem>), 
       typeid(Problem<MINLP0_problem>), 
       &lexical_cast_DowncastProblem<MINLP1_problem,MINLP0_problem>);

   ProblemMngr().register_lexical_cast
      (typeid(Problem<UNLP2_problem>), 
       typeid(Problem<UNLP1_problem>), 
       &lexical_cast_DowncastProblem<UNLP2_problem, UNLP1_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<UNLP1_problem>), 
       typeid(Problem<UNLP0_problem>), 
       &lexical_cast_DowncastProblem<UNLP1_problem, UNLP0_problem>);

   ProblemMngr().register_lexical_cast
      (typeid(Problem<UMINLP2_problem>), 
       typeid(Problem<UMINLP1_problem>), 
       &lexical_cast_DowncastProblem<UMINLP2_problem,UMINLP1_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<UMINLP1_problem>), 
       typeid(Problem<UMINLP0_problem>), 
       &lexical_cast_DowncastProblem<UMINLP1_problem,UMINLP0_problem>);

   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_NLP2_problem>), 
       typeid(Problem<MO_NLP1_problem>), 
       &lexical_cast_DowncastProblem<MO_NLP2_problem, MO_NLP1_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_NLP1_problem>), 
       typeid(Problem<MO_NLP0_problem>), 
       &lexical_cast_DowncastProblem<MO_NLP1_problem, MO_NLP0_problem>);

   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_MINLP2_problem>), 
       typeid(Problem<MO_MINLP1_problem>), 
       &lexical_cast_DowncastProblem<MO_MINLP2_problem,MO_MINLP1_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_MINLP1_problem>), 
       typeid(Problem<MO_MINLP0_problem>), 
       &lexical_cast_DowncastProblem<MO_MINLP1_problem,MO_MINLP0_problem>);

   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_UNLP2_problem>), 
       typeid(Problem<MO_UNLP1_problem>), 
       &lexical_cast_DowncastProblem<MO_UNLP2_problem, MO_UNLP1_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_UNLP1_problem>), 
       typeid(Problem<MO_UNLP0_problem>), 
       &lexical_cast_DowncastProblem<MO_UNLP1_problem, MO_UNLP0_problem>);

   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_UMINLP2_problem>), 
       typeid(Problem<MO_UMINLP1_problem>), 
       &lexical_cast_DowncastProblem<MO_UMINLP2_problem,MO_UMINLP1_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_UMINLP1_problem>), 
       typeid(Problem<MO_UMINLP0_problem>), 
       &lexical_cast_DowncastProblem<MO_UMINLP1_problem,MO_UMINLP0_problem>);

   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool downcast = RegisterDowncast();

} // namespace colin::StaticInitializers

} // namespace colin

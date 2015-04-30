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

#include <colin/reformulation/Upcast.h>
#include <colin/ApplicationMngr.h>

using std::pair;
using utilib::Any;

namespace colin {

namespace StaticInitializers {

namespace {

template <class FROM, class TO>
int lexical_cast_UpcastProblem(const Any& from, Any& to)
{
   pair<ApplicationHandle,UpcastApplication<TO>*> tmp
      = ApplicationHandle::create<UpcastApplication<TO> >();
   tmp.second->reformulate_application
      ( from.template expose<Problem<FROM> >()->get_handle() );

   to.template set< Problem<TO> >().set_application(tmp.second);
   return 0;
}


bool RegisterUpcast()
{
   // ------------------------------------------------------------------
   // Register creatable instances with the ApplicationMngr

   ApplicationMngr().declare_application_type
      <UpcastApplication<NLP0_problem> >("UpcastReformulation");

   ApplicationMngr().declare_application_type
      <UpcastApplication<NLP1_problem> >("UpcastReformulation");

   ApplicationMngr().declare_application_type
      <UpcastApplication<NLP2_problem> >("UpcastReformulation");

   ApplicationMngr().declare_application_type
      <UpcastApplication<MINLP0_problem> >("UpcastReformulation");

   ApplicationMngr().declare_application_type
      <UpcastApplication<MINLP1_problem> >("UpcastReformulation");

   ApplicationMngr().declare_application_type
      <UpcastApplication<MINLP2_problem> >("UpcastReformulation");

   ApplicationMngr().declare_application_type
      <UpcastApplication<UMINLP0_problem> >("UpcastReformulation");

   ApplicationMngr().declare_application_type
      <UpcastApplication<UMINLP1_problem> >("UpcastReformulation");

   ApplicationMngr().declare_application_type
      <UpcastApplication<UMINLP2_problem> >("UpcastReformulation");

   ApplicationMngr().declare_application_type
      <UpcastApplication<MO_MINLP0_problem> >("UpcastReformulation");

   ApplicationMngr().declare_application_type
      <UpcastApplication<MO_MINLP1_problem> >("UpcastReformulation");

   ApplicationMngr().declare_application_type
      <UpcastApplication<MO_MINLP2_problem> >("UpcastReformulation");


   // ------------------------------------------------------------------
   // Register default problem transformations with the ProblemMngr

   ProblemMngr().register_lexical_cast
      (typeid(Problem<UNLP0_problem>), 
       typeid(Problem<NLP0_problem>), 
       &lexical_cast_UpcastProblem<UNLP0_problem,NLP0_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<UNLP1_problem>), 
       typeid(Problem<NLP1_problem>), 
       &lexical_cast_UpcastProblem<UNLP1_problem,NLP1_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<UNLP2_problem>), 
       typeid(Problem<NLP2_problem>), 
       &lexical_cast_UpcastProblem<UNLP2_problem,NLP2_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<UMINLP0_problem>), 
       typeid(Problem<MINLP0_problem>), 
       &lexical_cast_UpcastProblem<UMINLP0_problem,MINLP0_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<UMINLP1_problem>), 
       typeid(Problem<MINLP1_problem>), 
       &lexical_cast_UpcastProblem<UMINLP1_problem,MINLP1_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<UMINLP2_problem>), 
       typeid(Problem<MINLP2_problem>), 
       &lexical_cast_UpcastProblem<UMINLP2_problem,MINLP2_problem>);

   ProblemMngr().register_lexical_cast
      (typeid(Problem<NLP0_problem>), 
       typeid(Problem<MINLP0_problem>), 
       &lexical_cast_UpcastProblem<NLP0_problem, MINLP0_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<NLP1_problem>), 
       typeid(Problem<MINLP1_problem>), 
       &lexical_cast_UpcastProblem<NLP1_problem, MINLP1_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<NLP2_problem>), 
       typeid(Problem<MINLP2_problem>), 
       &lexical_cast_UpcastProblem<NLP2_problem, MINLP2_problem>);

   ProblemMngr().register_lexical_cast
      (typeid(Problem<UNLP0_problem>), 
       typeid(Problem<UMINLP0_problem>), 
       &lexical_cast_UpcastProblem<UNLP0_problem, UMINLP0_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<UNLP1_problem>), 
       typeid(Problem<UMINLP1_problem>), 
       &lexical_cast_UpcastProblem<UNLP1_problem, UMINLP1_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<UNLP2_problem>), 
       typeid(Problem<UMINLP2_problem>), 
       &lexical_cast_UpcastProblem<UNLP2_problem, UMINLP2_problem>);

   ProblemMngr().register_lexical_cast
      (typeid(Problem<INLP_problem>), 
       typeid(Problem<MINLP0_problem>), 
       &lexical_cast_UpcastProblem<INLP_problem, MINLP0_problem>);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<UINLP_problem>), 
       typeid(Problem<UMINLP0_problem>), 
       &lexical_cast_UpcastProblem<UINLP_problem, UMINLP0_problem>);

   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool upcast = RegisterUpcast();

} // namespace colin::StaticInitializers

} // namespace colin

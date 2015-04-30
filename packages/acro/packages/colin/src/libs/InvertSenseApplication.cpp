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

#include <colin/reformulation/InvertSense.h>
#include <colin/ApplicationMngr.h>

using std::pair;
using utilib::Any;

namespace colin {

namespace StaticInitializers {

namespace {

template <class FROM, class TO>
int lexical_cast_ConstraintPenalty(const Any& from, Any& to)
{
   pair<ApplicationHandle,InvertSenseApplication<TO>*> tmp
      = ApplicationHandle::create<InvertSenseApplication<TO> >();
   tmp.second->set_base_application
      ( from.template expose<Problem<FROM> >()->get_handle() );

   to.template set< Problem<TO> >().set_application(tmp.second);
   return 0;
}



bool RegisterConstraintPenalty()
{
   // ------------------------------------------------------------------
   // Register creatable instances with the ApplicationMngr

   ApplicationMngr().declare_application_type
      <InvertSenseApplication<UNLP0_problem> >
      ("ConstraintPenaltyReformulation");

   ApplicationMngr().declare_application_type
      <InvertSenseApplication<UMINLP0_problem> >
      ("ConstraintPenaltyReformulation");

   ApplicationMngr().declare_application_type
      <InvertSenseApplication<NLP0_problem> >
      ("ConstraintPenaltyReformulation");

   ApplicationMngr().declare_application_type
      <InvertSenseApplication<MINLP0_problem> >
      ("ConstraintPenaltyReformulation");


   // ------------------------------------------------------------------
   // Register default problem transformations with the ProblemMngr

   ProblemMngr().register_lexical_cast
      (typeid(Problem<NLP0_problem>), 
       typeid(Problem<UNLP0_problem>), 
       &lexical_cast_ConstraintPenalty<NLP0_problem, UNLP0_problem>,
       1);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<MINLP0_problem>), 
       typeid(Problem<UMINLP0_problem>), 
       &lexical_cast_ConstraintPenalty<MINLP0_problem, UMINLP0_problem>,
       1);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<NLP0_problem>), 
       typeid(Problem<UNLP0_problem>), 
       &lexical_cast_ConstraintPenalty<NLP0_problem, UNLP0_problem>,
       1);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<MINLP0_problem>), 
       typeid(Problem<UMINLP0_problem>), 
       &lexical_cast_ConstraintPenalty<MINLP0_problem, UMINLP0_problem>,
       1);

   // NB: I am disabling this because the ConstraintPenalty app does NOT 
   //   propatate constraint derivatives into the objective derivative
   //ProblemMngr().register_lexical_cast
   //   (typeid(Problem<NLP1_problem>), 
   //    typeid(Problem<UNLP1_problem>), 
   //    &lexical_cast_ConstraintPenalty<NLP1_problem, UNLP1_problem>,
   //    false);


   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool constraint_penalty = RegisterConstraintPenalty();

} // namespace colin::StaticInitializers

} // namespace colin

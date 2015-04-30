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

#include <colin/reformulation/UnconMultiobj.h>
#include <colin/ApplicationMngr.h>

using std::pair;
using utilib::Any;

namespace colin {

namespace StaticInitializers {

namespace {


template <class FROM, class TO>
int lexical_cast_UnconMultiobjProblem(const Any& from, Any& to)
{
   pair<ApplicationHandle,UnconMultiobjApplication<TO>*> tmp
      = ApplicationHandle::create<UnconMultiobjApplication<TO> >();
   tmp.second->reformulate_application
      ( from.template expose<Problem<FROM> >()->get_handle() );

   to.template set< Problem<TO> >().set_application(tmp.second);
   return 0;
}

bool RegisterUnconMultiobj()
{
   // ------------------------------------------------------------------
   // Register creatable instances with the ApplicationMngr

   ApplicationMngr().declare_application_type
      <UnconMultiobjApplication<MO_UNLP0_problem> >
      ("UnconstrainedMultiobjReformulation");

   ApplicationMngr().declare_application_type
      <UnconMultiobjApplication<MO_UMINLP0_problem> >
      ("UnconstrainedMultiobjReformulation");

   ApplicationMngr().declare_application_type
      <UnconMultiobjApplication<SMO_UNLP0_problem> >
      ("UnconstrainedMultiobjReformulation");

   ApplicationMngr().declare_application_type
      <UnconMultiobjApplication<SMO_UMINLP0_problem> >
      ("UnconstrainedMultiobjReformulation");


   // ------------------------------------------------------------------
   // Register default problem transformations with the ProblemMngr

   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_NLP0_problem>), 
       typeid(Problem<MO_UNLP0_problem>), 
       &lexical_cast_UnconMultiobjProblem<MO_NLP0_problem, MO_UNLP0_problem>,
       10);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_MINLP0_problem>), 
       typeid(Problem<MO_UMINLP0_problem>), 
       &lexical_cast_UnconMultiobjProblem< MO_MINLP0_problem, 
                                           MO_UMINLP0_problem >,
       10);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<SMO_NLP0_problem>), 
       typeid(Problem<SMO_UNLP0_problem>), 
       &lexical_cast_UnconMultiobjProblem<SMO_NLP0_problem, SMO_UNLP0_problem>,
       10);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<SMO_MINLP0_problem>), 
       typeid(Problem<SMO_UMINLP0_problem>), 
       &lexical_cast_UnconMultiobjProblem< SMO_MINLP0_problem, 
                                           SMO_UMINLP0_problem >,
       10);

   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool uncon_multiobj = RegisterUnconMultiobj();

} // namespace colin::StaticInitializers

} // namespace colin

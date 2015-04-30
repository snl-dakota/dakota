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

#include <colin/reformulation/WeightedSum.h>
#include <colin/ApplicationMngr.h>

using std::pair;
using utilib::Any;

namespace colin {

namespace StaticInitializers {

namespace {

template <class FROM, class TO>
int lexical_cast_WeightedSumProblem(const Any& from, Any& to)
{
   pair<ApplicationHandle,WeightedSumApplication<TO>*> tmp
      = ApplicationHandle::create<WeightedSumApplication<TO> >();
   tmp.second->reformulate_application
      ( from.template expose<Problem<FROM> >()->get_handle() );

   to.template set< Problem<TO> >().set_application(tmp.second);
   return 0;
}


bool RegisterWeightedSum()
{
   // ------------------------------------------------------------------
   // Register creatable instances with the ApplicationMngr

   ApplicationMngr().declare_application_type
      <WeightedSumApplication<NLP0_problem> >("WeightedSumReformulation");

   ApplicationMngr().declare_application_type
      <WeightedSumApplication<MINLP0_problem> >("WeightedSumReformulation");

   ApplicationMngr().declare_application_type
      <WeightedSumApplication<NLP1_problem> >("WeightedSumReformulation");

   ApplicationMngr().declare_application_type
      <WeightedSumApplication<MINLP1_problem> >("WeightedSumReformulation");

   ApplicationMngr().declare_application_type
      <WeightedSumApplication<NLP2_problem> >("WeightedSumReformulation");

   ApplicationMngr().declare_application_type
      <WeightedSumApplication<MINLP2_problem> >("WeightedSumReformulation");


   // ------------------------------------------------------------------
   // Register default problem transformations with the ProblemMngr

   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_NLP0_problem>), 
       typeid(Problem<NLP0_problem>), 
       &lexical_cast_WeightedSumProblem<MO_NLP0_problem, NLP0_problem>,
       1);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_MINLP0_problem>), 
       typeid(Problem<MINLP0_problem>), 
       &lexical_cast_WeightedSumProblem<MO_MINLP0_problem, MINLP0_problem>,
       1);

   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_NLP1_problem>), 
       typeid(Problem<NLP1_problem>), 
       &lexical_cast_WeightedSumProblem<MO_NLP1_problem, NLP1_problem>,
       1);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_MINLP1_problem>), 
       typeid(Problem<MINLP1_problem>), 
       &lexical_cast_WeightedSumProblem<MO_MINLP1_problem, MINLP1_problem>,
       1);

/*   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_NLP2_problem>), 
       typeid(Problem<NLP2_problem>), 
       &lexical_cast_WeightedSumProblem<MO_NLP2_problem, NLP2_problem>,
       1);
   ProblemMngr().register_lexical_cast
      (typeid(Problem<MO_MINLP2_problem>), 
       typeid(Problem<MINLP2_problem>), 
       &lexical_cast_WeightedSumProblem<MO_MINLP2_problem, MINLP2_problem>,
       1); */

   // NB: I am disabling this because if I enable it, there is an
   //   ambiguous path from MO_MINLP0 -> UMINLP0, and going through this
   //   transformation is less desirable.
   //ProblemMngr().register_lexical_cast
   //   (typeid(Problem<MO_UMINLP0_problem>), 
   //    typeid(Problem<UMINLP0_problem>), 
   //    &lexical_cast_OptProblem_MO_UMINLP0_UMINLP0);

   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool weighted_sum = RegisterWeightedSum();

} // namespace colin::StaticInitializers

} // namespace colin

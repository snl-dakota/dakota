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


#include <colin/AnalysisCodeApplication.h>
#include <colin/ApplicationMngr.h>

using utilib::MixedIntVars;
using std::vector;

namespace colin {

namespace StaticInitializers {

namespace {

bool RegisterAnalysisCode()
{
   ApplicationMngr().declare_application_type
      <AnalysisCodeApplication<MINLP0_problem, MixedIntVars> >
      ("AnalysisCode");

   ApplicationMngr().declare_application_type
      <AnalysisCodeApplication<MINLP1_problem, MixedIntVars> >
      ("AnalysisCode");

   ApplicationMngr().declare_application_type
      <AnalysisCodeApplication<MINLP2_problem, MixedIntVars> >
      ("AnalysisCode");

   ApplicationMngr().declare_application_type
      <AnalysisCodeApplication<UMINLP0_problem, MixedIntVars> >
      ("AnalysisCode");

   ApplicationMngr().declare_application_type
      <AnalysisCodeApplication<NLP0_problem,vector<double> > >
      ("AnalysisCode");

   ApplicationMngr().declare_application_type
      <AnalysisCodeApplication<NLP1_problem,vector<double> > >
      ("AnalysisCode");

   ApplicationMngr().declare_application_type
      <AnalysisCodeApplication<NLP2_problem,vector<double> > >
      ("AnalysisCode");

   ApplicationMngr().declare_application_type
      <AnalysisCodeApplication<UNLP0_problem,vector<double> > >
      ("AnalysisCode");

   ApplicationMngr().declare_application_type
      <AnalysisCodeApplication<UNLP1_problem,vector<double> > >
      ("AnalysisCode");


   ApplicationMngr().declare_application_type
      <AnalysisCodeApplication<MO_MINLP0_problem, MixedIntVars> >
      ("AnalysisCode");

   ApplicationMngr().declare_application_type
      <AnalysisCodeApplication<MO_UMINLP0_problem, MixedIntVars> >
      ("AnalysisCode");

   ApplicationMngr().declare_application_type
      <AnalysisCodeApplication<MO_NLP0_problem,vector<double> > >
      ("AnalysisCode");

   ApplicationMngr().declare_application_type
      <AnalysisCodeApplication<MO_UNLP0_problem,vector<double> > >
      ("AnalysisCode");

   ApplicationMngr().declare_application_type
      <AnalysisCodeApplication<SNLP0_problem,vector<double> > >
      ("AnalysisCode");

   ApplicationMngr().declare_application_type
      <AnalysisCodeApplication<UINLP_problem, MixedIntVars> >
      ("AnalysisCode");

   ApplicationMngr().declare_application_type
      <AnalysisCodeApplication<INLP_problem, MixedIntVars> >
      ("AnalysisCode");

   ApplicationMngr().declare_application_type
      <AnalysisCodeApplication<ILP_problem, MixedIntVars> >
      ("AnalysisCode");


   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool analysis_code = RegisterAnalysisCode();

} // namespace colin::StaticInitializers

} // namespace colin

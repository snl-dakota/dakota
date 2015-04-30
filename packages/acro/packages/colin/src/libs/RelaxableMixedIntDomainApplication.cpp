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

#include <colin/reformulation/RelaxableMixedIntDomain.h>
#include <colin/ApplicationMngr.h>

using std::pair;
using utilib::Any;

namespace colin {

namespace StaticInitializers {

namespace {

bool RegisterRelaxableMixedIntDomain()
{
   // ------------------------------------------------------------------
   // Register creatable instances with the ApplicationMngr

   ApplicationMngr().declare_application_type
      <RelaxableMixedIntDomainApplication<MINLP0_problem> >
      ("RelaxableMixedIntDomainReformulation");
  
   ApplicationMngr().declare_application_type
      <RelaxableMixedIntDomainApplication<MINLP1_problem> >
      ("RelaxableMixedIntDomainReformulation");

   ApplicationMngr().declare_application_type
      <RelaxableMixedIntDomainApplication<MINLP2_problem> >
      ("RelaxableMixedIntDomainReformulation");

   ApplicationMngr().declare_application_type
      <RelaxableMixedIntDomainApplication<UMINLP0_problem> >
      ("RelaxableMixedIntDomainReformulation");
  
   ApplicationMngr().declare_application_type
      <RelaxableMixedIntDomainApplication<UMINLP1_problem> >
      ("RelaxableMixedIntDomainReformulation");

   ApplicationMngr().declare_application_type
      <RelaxableMixedIntDomainApplication<UMINLP2_problem> >
      ("RelaxableMixedIntDomainReformulation");


   ApplicationMngr().declare_application_type
      <RelaxableMixedIntDomainApplication<MO_MINLP0_problem> >
      ("RelaxableMixedIntDomainReformulation");

   ApplicationMngr().declare_application_type
      <RelaxableMixedIntDomainApplication<MO_UMINLP0_problem> >
      ("RelaxableMixedIntDomainReformulation");

   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool relaxable_mixed_int_domain =
    RegisterRelaxableMixedIntDomain();

} // namespace colin::StaticInitializers

} // namespace colin

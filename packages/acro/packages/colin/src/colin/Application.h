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

/**
 * \file Application.h
 *
 * Defines the colin::Application class.
 */
#ifndef colin_Application_h
#define colin_Application_h

#include <acro_config.h>
#include <tinyxml/tinyxml.h>
#include <colin/application/Base.h>
//#include <colin/application/Bounds.h>
//#include <colin/application/Jacobian.h>
#include <colin/application/RealDomain.h>
#include <colin/application/IntDomain.h>
#include <colin/application/LinearConstraints.h>
#include <colin/application/NonlinearConstraints.h>
#include <colin/application/NonD_Constraints.h>
#include <colin/application/SingleObjective.h>
#include <colin/application/MultiObjective.h>
#include <colin/application/NonD_Objective.h>
#include <colin/application/Gradient.h>
#include <colin/application/Hessian.h>
#include <colin/ProblemTraits.h>
#include <colin/Problem.h>

#include <utilib/MixedIntVars.h>

namespace colin
{

template <class ProblemT>
class Application;

template <class ProblemT>
void set_bounds(const char* format, Application<ProblemT>* app);

/**
 * \class Application
 *
 * The class that defines the components that are integrated into an
 * Application object.  This class can be specialized based on the
 * template type to include other, user-defined, components.
 */
template <class ProblemT >
class Application
   : public Application_Include_SingleObjective
        < !HasProblemTrait(ProblemT, multiple_objectives) > ,
     public Application_Include_MultiObjective
        < HasProblemTrait(ProblemT, multiple_objectives) > ,
     public Application_Include_NonD_Objective
        < HasProblemTrait(ProblemT, nondeterministic_response) > ,
     public Application_Include_RealDomain<HasProblemTrait(ProblemT, reals)>,
     public Application_Include_IntDomain<HasProblemTrait(ProblemT, integers)>,
     public Application_Include_LinearConstraints
        < HasProblemTrait(ProblemT, linear_constraints),
          HasProblemTrait(ProblemT, gradients) > ,
     public Application_Include_NonlinearConstraints
        < HasProblemTrait(ProblemT, nonlinear_constraints),
          HasProblemTrait(ProblemT, gradients) > ,
     public Application_Include_NonD_Constraints
        < HasProblemTrait(ProblemT, nondeterministic_response)
             && ( HasProblemTrait(ProblemT, nonlinear_constraints) ),
          HasProblemTrait(ProblemT, gradients) > ,
     public Application_Include_Gradient<HasProblemTrait(ProblemT, gradients)>,
     public Application_Include_Hessian<HasProblemTrait(ProblemT, hessians)>
{
public:

   typedef ProblemT problem_t;

   virtual ~Application()
   {}

   ///
   virtual unsigned long problem_type() const
   { return ProblemT::value; }

   virtual std::string problem_type_name() const
   { return ProblemType<ProblemT>::name(); }

   virtual utilib::Any get_problem() 
   { 
      return Problem<ProblemT>(this); 
   }

   ///
   bool finite_bound_constraints() const
   {
      return 
         Application_Include_RealDomain<HasProblemTrait(ProblemT, reals)>
         ::finiteBoundConstraints() &&
         Application_Include_IntDomain<HasProblemTrait(ProblemT, integers)>
         ::finiteBoundConstraints();
   }

   ///
   bool testBoundsFeasibility(const utilib::Any& point)
    {
       // FIXME: Ideally, we would use the Application to convert the
       // incoming data into it's appropriate native type.  However, the
       // current DirectSimpleApplication allows the consumer to specify
       // the domain type (even if that type does not make any sense).
       // In order to get the regression tests passing again, we will
       // explicitly convert things to a MixedIntDomain, however this
       // should only be a temporary fix.
#if 0
       AppRequest tmpRequest = set_domain(point);
       utilib::Any domain = tmpRequest.domain(this);
#else
       utilib::Any domain;
       utilib::TypeManager()->lexical_cast( point, domain, 
                                            typeid(utilib::MixedIntVars) );
#endif

       bool warn = utilib::TypeManager()->warningExceptions();
       utilib::TypeManager()->setWarningExceptions(false);

       //std::cerr << "testBoundsFeasibility: type = " 
       //          << utilib::demangledName(domain.type())
       //          << std::endl;
       
       try {
          bool ans = 
             Application_Include_RealDomain<HasProblemTrait(ProblemT, reals)>
               ::testBoundsFeasibility(domain) &&
             Application_Include_IntDomain<HasProblemTrait(ProblemT, integers)>
               ::testBoundsFeasibility(domain);
          utilib::TypeManager()->setWarningExceptions(warn);
          return ans;
       } catch ( ... ) {
          utilib::TypeManager()->setWarningExceptions(warn);
          throw;
       }
    }

};

} // namespace colin

#endif // colin_Application_h

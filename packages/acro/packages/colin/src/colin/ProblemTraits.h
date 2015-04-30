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
 * \file ProblemTraits.h
 *
 * The colin::ProblemTraits template class is used to define features
 * of canonical optimization problem types.  These traits are used,
 * for example, to tailor the methods included in the Problem class
 * for a specific type of problem.
 */

#ifndef colin_ProblemTraits_h
#define colin_ProblemTraits_h

#include <acro_config.h>
#include <utilib/std_headers.h>
//#include <utilib/exception_mngr.h>
//#include <colin/Factory.h>

namespace colin
{

typedef unsigned long  problem_bitvector_t;

template <class ProblemT>
class ProblemType
{
public:
   static const char* name() { return "unknown"; }
};

/// Return the ProblemT that corresponds to a given bitvector id
template<problem_bitvector_t id>
struct GetProblem {};

/// Calculate the bitvector id for a specific problem type
template <class ProblemT, int id>
struct GetProblemValue {
   static const problem_bitvector_t value = 0;
};

} // namespace colin


/** Macro used to declare (but not define!) a problem type.
  */
#define DeclareProblemType(ProblemT)                            \
   namespace colin {                                            \
      class ProblemT ## _problem;                               \
      template<> class ProblemType< ProblemT ## _problem >      \
      {                                                         \
      public:                                                   \
         static const char* name() { return #ProblemT ; }       \
      };                                                        \
   }

/** Macro used to declare that I am done messing with the definition of
  * this problem type.  This will acrually define the ProblemT class and
  * calculate the bitvector id.
  */
#define FinalizeProblemType(ProblemT)                                   \
   namespace colin {                                                    \
      class ProblemT {                                                  \
      public:                                                           \
         static const problem_bitvector_t value =                       \
            GetProblemValue<ProblemType< ProblemT >, 0>::value;         \
      };                                                                \
      template<>                                                        \
      struct GetProblem<ProblemT::value> {                              \
         typedef ProblemT type;                                         \
      };                                                                \
   }

/** Macro used to declare a problem trait.
  */
#define DeclareProblemTrait(id,trait,default)                           \
   namespace colin {                                                    \
      template <class ProblemT>                                         \
      struct ProblemTrait_ ## trait {                                   \
         static const bool value = default;                             \
      };                                                                \
      template <class ProblemT>                                         \
      struct GetProblemValue<ProblemT, id> {                            \
         static const problem_bitvector_t value =                       \
            GetProblemValue<ProblemT, id+1>::value                      \
            + ( ProblemTrait_ ## trait<ProblemT>::value ? 1 << id : 0 ); \
      };                                                                \
      static const problem_bitvector_t                                  \
         ProblemTraitValue_ ## trait = 1 << id;                         \
   }


/** Macro used to set a domain trait.
  */
#define SetProblemTrait(ProblemT,trait,newvalue)                        \
   namespace colin {                                                    \
      template<>                                                        \
      struct ProblemTrait_ ## trait < ProblemType< ProblemT > >{        \
         static const bool value = newvalue;                            \
      };                                                                \
   }


/** Macro used to get the bitvector for a specific trait
 */
#define ProblemTrait(trait)             \
   ( colin::ProblemTraitValue_ ## trait )

/** Macro used to get if a trait is present for a given problem type.
  */
#define HasProblemTrait(ProblemT,trait)                                 \
   ( colin::ProblemTrait_ ## trait < ProblemType< ProblemT > >::value )

/** Macro used to get the bitvector for a specific trait if it is
    present in the problem type
 */
#define GetProblemTraitValue(ProblemT,trait)    \
   ( HasProblemTrait(ProblemT,trait) ? ProblemTrait(trait) : 0 )


#if !defined(DOXYGEN)
//============================================================================
//
// Standard ProblemTrait declarations
//
// Other traits can be added in a transparent manner
//
DeclareProblemTrait(0,reals, false)
DeclareProblemTrait(1,integers, false)
DeclareProblemTrait(2,binarys, false)
DeclareProblemTrait(3,nonlinear_constraints, false)
DeclareProblemTrait(4,linear_constraints, false)
DeclareProblemTrait(5,gradients, false)
DeclareProblemTrait(6,hessians, false)
DeclareProblemTrait(7,linear_objective, false)
DeclareProblemTrait(8,multiple_objectives, false)
DeclareProblemTrait(9,nondeterministic_response, false)
#endif // !defined(DOXYGEN)


#if !defined(DOXYGEN)
//============================================================================
//
// Some standard problem types
//
DeclareProblemType(UNLP0)
SetProblemTrait(UNLP0_problem, reals, true)
FinalizeProblemType(UNLP0_problem)

DeclareProblemType(UNLP1)
SetProblemTrait(UNLP1_problem, reals, true)
SetProblemTrait(UNLP1_problem, gradients, true)
FinalizeProblemType(UNLP1_problem)

DeclareProblemType(UNLP2)
SetProblemTrait(UNLP2_problem, reals, true)
SetProblemTrait(UNLP2_problem, gradients, true)
SetProblemTrait(UNLP2_problem, hessians, true)
FinalizeProblemType(UNLP2_problem)


DeclareProblemType(NLP0)
SetProblemTrait(NLP0_problem, reals, true)
SetProblemTrait(NLP0_problem, linear_constraints, true)
SetProblemTrait(NLP0_problem, nonlinear_constraints, true)
FinalizeProblemType(NLP0_problem)

DeclareProblemType(NLP1)
SetProblemTrait(NLP1_problem, reals, true)
SetProblemTrait(NLP1_problem, gradients, true)
SetProblemTrait(NLP1_problem, linear_constraints, true)
SetProblemTrait(NLP1_problem, nonlinear_constraints, true)
FinalizeProblemType(NLP1_problem)

DeclareProblemType(NLP2)
SetProblemTrait(NLP2_problem, reals, true)
SetProblemTrait(NLP2_problem, gradients, true)
SetProblemTrait(NLP2_problem, hessians, true)
SetProblemTrait(NLP2_problem, linear_constraints, true)
SetProblemTrait(NLP2_problem, nonlinear_constraints, true)
FinalizeProblemType(NLP2_problem)


DeclareProblemType(MINLP0)
SetProblemTrait(MINLP0_problem, reals, true)
SetProblemTrait(MINLP0_problem, integers, true)
SetProblemTrait(MINLP0_problem, binarys, true)
SetProblemTrait(MINLP0_problem, linear_constraints, true)
SetProblemTrait(MINLP0_problem, nonlinear_constraints, true)
FinalizeProblemType(MINLP0_problem)

DeclareProblemType(MINLP1)
SetProblemTrait(MINLP1_problem, reals, true)
SetProblemTrait(MINLP1_problem, integers, true)
SetProblemTrait(MINLP1_problem, binarys, true)
SetProblemTrait(MINLP1_problem, linear_constraints, true)
SetProblemTrait(MINLP1_problem, nonlinear_constraints, true)
SetProblemTrait(MINLP1_problem, gradients, true)
FinalizeProblemType(MINLP1_problem)

DeclareProblemType(MINLP2)
SetProblemTrait(MINLP2_problem, reals, true)
SetProblemTrait(MINLP2_problem, integers, true)
SetProblemTrait(MINLP2_problem, binarys, true)
SetProblemTrait(MINLP2_problem, linear_constraints, true)
SetProblemTrait(MINLP2_problem, nonlinear_constraints, true)
SetProblemTrait(MINLP2_problem, gradients, true)
SetProblemTrait(MINLP2_problem, hessians, true)
FinalizeProblemType(MINLP2_problem)


DeclareProblemType(UMINLP0)
SetProblemTrait(UMINLP0_problem, reals, true)
SetProblemTrait(UMINLP0_problem, integers, true)
SetProblemTrait(UMINLP0_problem, binarys, true)
FinalizeProblemType(UMINLP0_problem)

DeclareProblemType(UMINLP1)
SetProblemTrait(UMINLP1_problem, reals, true)
SetProblemTrait(UMINLP1_problem, integers, true)
SetProblemTrait(UMINLP1_problem, binarys, true)
SetProblemTrait(UMINLP1_problem, gradients, true)
FinalizeProblemType(UMINLP1_problem)

DeclareProblemType(UMINLP2)
SetProblemTrait(UMINLP2_problem, reals, true)
SetProblemTrait(UMINLP2_problem, integers, true)
SetProblemTrait(UMINLP2_problem, binarys, true)
SetProblemTrait(UMINLP2_problem, gradients, true)
SetProblemTrait(UMINLP2_problem, hessians, true)
FinalizeProblemType(UMINLP2_problem)


DeclareProblemType(MO_NLP0)
SetProblemTrait(MO_NLP0_problem, reals, true)
SetProblemTrait(MO_NLP0_problem, linear_constraints, true)
SetProblemTrait(MO_NLP0_problem, nonlinear_constraints, true)
SetProblemTrait(MO_NLP0_problem, multiple_objectives, true)
FinalizeProblemType(MO_NLP0_problem)

DeclareProblemType(MO_NLP1)
SetProblemTrait(MO_NLP1_problem, reals, true)
SetProblemTrait(MO_NLP1_problem, linear_constraints, true)
SetProblemTrait(MO_NLP1_problem, nonlinear_constraints, true)
SetProblemTrait(MO_NLP1_problem, gradients, true)
SetProblemTrait(MO_NLP1_problem, multiple_objectives, true)
FinalizeProblemType(MO_NLP1_problem)

DeclareProblemType(MO_NLP2)
SetProblemTrait(MO_NLP2_problem, reals, true)
SetProblemTrait(MO_NLP2_problem, linear_constraints, true)
SetProblemTrait(MO_NLP2_problem, nonlinear_constraints, true)
SetProblemTrait(MO_NLP2_problem, gradients, true)
SetProblemTrait(MO_NLP2_problem, hessians, true)
SetProblemTrait(MO_NLP2_problem, multiple_objectives, true)
FinalizeProblemType(MO_NLP2_problem)

DeclareProblemType(MO_UNLP0)
SetProblemTrait(MO_UNLP0_problem, reals, true)
SetProblemTrait(MO_UNLP0_problem, multiple_objectives, true)
FinalizeProblemType(MO_UNLP0_problem)

DeclareProblemType(MO_UNLP1)
SetProblemTrait(MO_UNLP1_problem, reals, true)
SetProblemTrait(MO_UNLP1_problem, gradients, true)
SetProblemTrait(MO_UNLP1_problem, multiple_objectives, true)
FinalizeProblemType(MO_UNLP1_problem)

DeclareProblemType(MO_UNLP2)
SetProblemTrait(MO_UNLP2_problem, reals, true)
SetProblemTrait(MO_UNLP2_problem, gradients, true)
SetProblemTrait(MO_UNLP2_problem, hessians, true)
SetProblemTrait(MO_UNLP2_problem, multiple_objectives, true)
FinalizeProblemType(MO_UNLP2_problem)

DeclareProblemType(MO_MINLP0)
SetProblemTrait(MO_MINLP0_problem, reals, true)
SetProblemTrait(MO_MINLP0_problem, integers, true)
SetProblemTrait(MO_MINLP0_problem, binarys, true)
SetProblemTrait(MO_MINLP0_problem, linear_constraints, true)
SetProblemTrait(MO_MINLP0_problem, nonlinear_constraints, true)
SetProblemTrait(MO_MINLP0_problem, multiple_objectives, true)
FinalizeProblemType(MO_MINLP0_problem)

DeclareProblemType(MO_MINLP1)
SetProblemTrait(MO_MINLP1_problem, reals, true)
SetProblemTrait(MO_MINLP1_problem, integers, true)
SetProblemTrait(MO_MINLP1_problem, binarys, true)
SetProblemTrait(MO_MINLP1_problem, linear_constraints, true)
SetProblemTrait(MO_MINLP1_problem, nonlinear_constraints, true)
SetProblemTrait(MO_MINLP1_problem, gradients, true)
SetProblemTrait(MO_MINLP1_problem, multiple_objectives, true)
FinalizeProblemType(MO_MINLP1_problem)

DeclareProblemType(MO_MINLP2)
SetProblemTrait(MO_MINLP2_problem, reals, true)
SetProblemTrait(MO_MINLP2_problem, integers, true)
SetProblemTrait(MO_MINLP2_problem, binarys, true)
SetProblemTrait(MO_MINLP2_problem, linear_constraints, true)
SetProblemTrait(MO_MINLP2_problem, nonlinear_constraints, true)
SetProblemTrait(MO_MINLP2_problem, gradients, true)
SetProblemTrait(MO_MINLP2_problem, hessians, true)
SetProblemTrait(MO_MINLP2_problem, multiple_objectives, true)
FinalizeProblemType(MO_MINLP2_problem)

DeclareProblemType(MO_UMINLP0)
SetProblemTrait(MO_UMINLP0_problem, reals, true)
SetProblemTrait(MO_UMINLP0_problem, integers, true)
SetProblemTrait(MO_UMINLP0_problem, binarys, true)
SetProblemTrait(MO_UMINLP0_problem, multiple_objectives, true)
FinalizeProblemType(MO_UMINLP0_problem)

DeclareProblemType(MO_UMINLP1)
SetProblemTrait(MO_UMINLP1_problem, reals, true)
SetProblemTrait(MO_UMINLP1_problem, integers, true)
SetProblemTrait(MO_UMINLP1_problem, binarys, true)
SetProblemTrait(MO_UMINLP1_problem, gradients, true)
SetProblemTrait(MO_UMINLP1_problem, multiple_objectives, true)
FinalizeProblemType(MO_UMINLP1_problem)

DeclareProblemType(MO_UMINLP2)
SetProblemTrait(MO_UMINLP2_problem, reals, true)
SetProblemTrait(MO_UMINLP2_problem, integers, true)
SetProblemTrait(MO_UMINLP2_problem, binarys, true)
SetProblemTrait(MO_UMINLP2_problem, gradients, true)
SetProblemTrait(MO_UMINLP2_problem, hessians, true)
SetProblemTrait(MO_UMINLP2_problem, multiple_objectives, true)
FinalizeProblemType(MO_UMINLP2_problem)

DeclareProblemType(SNLP0)
SetProblemTrait(SNLP0_problem, reals, true)
SetProblemTrait(SNLP0_problem, linear_constraints, true)
SetProblemTrait(SNLP0_problem, nonlinear_constraints, true)
SetProblemTrait(SNLP0_problem, nondeterministic_response, true)
FinalizeProblemType(SNLP0_problem)

DeclareProblemType(SNLP1)
SetProblemTrait(SNLP1_problem, reals, true)
SetProblemTrait(SNLP1_problem, linear_constraints, true)
SetProblemTrait(SNLP1_problem, nonlinear_constraints, true)
SetProblemTrait(SNLP1_problem, nondeterministic_response, true)
SetProblemTrait(SNLP1_problem, gradients, true)
FinalizeProblemType(SNLP1_problem)

DeclareProblemType(SNLP2)
SetProblemTrait(SNLP2_problem, reals, true)
SetProblemTrait(SNLP2_problem, linear_constraints, true)
SetProblemTrait(SNLP2_problem, nonlinear_constraints, true)
SetProblemTrait(SNLP2_problem, nondeterministic_response, true)
SetProblemTrait(SNLP2_problem, gradients, true)
SetProblemTrait(SNLP2_problem, hessians, true)
FinalizeProblemType(SNLP2_problem)

DeclareProblemType(SUNLP0)
SetProblemTrait(SUNLP0_problem, reals, true)
SetProblemTrait(SUNLP0_problem, nondeterministic_response, true)
FinalizeProblemType(SUNLP0_problem)

DeclareProblemType(SMINLP0)
SetProblemTrait(SMINLP0_problem, reals, true)
SetProblemTrait(SMINLP0_problem, integers, true)
SetProblemTrait(SMINLP0_problem, binarys, true)
SetProblemTrait(SMINLP0_problem, linear_constraints, true)
SetProblemTrait(SMINLP0_problem, nonlinear_constraints, true)
SetProblemTrait(SMINLP0_problem, nondeterministic_response, true)
FinalizeProblemType(SMINLP0_problem)

DeclareProblemType(SMINLP1)
SetProblemTrait(SMINLP1_problem, reals, true)
SetProblemTrait(SMINLP1_problem, integers, true)
SetProblemTrait(SMINLP1_problem, binarys, true)
SetProblemTrait(SMINLP1_problem, linear_constraints, true)
SetProblemTrait(SMINLP1_problem, nonlinear_constraints, true)
SetProblemTrait(SMINLP1_problem, nondeterministic_response, true)
SetProblemTrait(SMINLP1_problem, gradients, true)
FinalizeProblemType(SMINLP1_problem)

DeclareProblemType(SMINLP2)
SetProblemTrait(SMINLP2_problem, reals, true)
SetProblemTrait(SMINLP2_problem, integers, true)
SetProblemTrait(SMINLP2_problem, binarys, true)
SetProblemTrait(SMINLP2_problem, linear_constraints, true)
SetProblemTrait(SMINLP2_problem, nonlinear_constraints, true)
SetProblemTrait(SMINLP2_problem, nondeterministic_response, true)
SetProblemTrait(SMINLP2_problem, gradients, true)
SetProblemTrait(SMINLP2_problem, hessians, true)
FinalizeProblemType(SMINLP2_problem)

DeclareProblemType(SUMINLP0)
SetProblemTrait(SUMINLP0_problem, reals, true)
SetProblemTrait(SUMINLP0_problem, integers, true)
SetProblemTrait(SUMINLP0_problem, binarys, true)
SetProblemTrait(SUMINLP0_problem, nondeterministic_response, true)
FinalizeProblemType(SUMINLP0_problem)

DeclareProblemType(SMO_NLP0)
SetProblemTrait(SMO_NLP0_problem, reals, true)
SetProblemTrait(SMO_NLP0_problem, linear_constraints, true)
SetProblemTrait(SMO_NLP0_problem, nonlinear_constraints, true)
SetProblemTrait(SMO_NLP0_problem, multiple_objectives, true)
SetProblemTrait(SMO_NLP0_problem, nondeterministic_response, true)
FinalizeProblemType(SMO_NLP0_problem)

DeclareProblemType(SMO_UNLP0)
SetProblemTrait(SMO_UNLP0_problem, reals, true)
SetProblemTrait(SMO_UNLP0_problem, multiple_objectives, true)
SetProblemTrait(SMO_UNLP0_problem, nondeterministic_response, true)
FinalizeProblemType(SMO_UNLP0_problem)

DeclareProblemType(SMO_MINLP0)
SetProblemTrait(SMO_MINLP0_problem, reals, true)
SetProblemTrait(SMO_MINLP0_problem, integers, true)
SetProblemTrait(SMO_MINLP0_problem, binarys, true)
SetProblemTrait(SMO_MINLP0_problem, linear_constraints, true)
SetProblemTrait(SMO_MINLP0_problem, nonlinear_constraints, true)
SetProblemTrait(SMO_MINLP0_problem, multiple_objectives, true)
SetProblemTrait(SMO_MINLP0_problem, nondeterministic_response, true)
FinalizeProblemType(SMO_MINLP0_problem)

DeclareProblemType(SMO_UMINLP0)
SetProblemTrait(SMO_UMINLP0_problem, reals, true)
SetProblemTrait(SMO_UMINLP0_problem, integers, true)
SetProblemTrait(SMO_UMINLP0_problem, binarys, true)
SetProblemTrait(SMO_UMINLP0_problem, multiple_objectives, true)
SetProblemTrait(SMO_UMINLP0_problem, nondeterministic_response, true)
FinalizeProblemType(SMO_UMINLP0_problem)


DeclareProblemType(LP)
SetProblemTrait(LP_problem, reals, true)
SetProblemTrait(LP_problem, linear_constraints, true)
SetProblemTrait(LP_problem, linear_objective, true)
SetProblemTrait(LP_problem, gradients, true)
FinalizeProblemType(LP_problem)

DeclareProblemType(MILP)
SetProblemTrait(MILP_problem, reals, true)
SetProblemTrait(MILP_problem, integers, true)
SetProblemTrait(MILP_problem, binarys, true)
SetProblemTrait(MILP_problem, linear_constraints, true)
SetProblemTrait(MILP_problem, linear_objective, true)
SetProblemTrait(MILP_problem, gradients, true)
FinalizeProblemType(MILP_problem)

namespace colin {
// Alias MILP to MIP
typedef MILP_problem MIP_problem;
} // namespace colin

DeclareProblemType(MO_LP)
SetProblemTrait(MO_LP_problem, multiple_objectives, true)
SetProblemTrait(MO_LP_problem, reals, true)
SetProblemTrait(MO_LP_problem, linear_constraints, true)
SetProblemTrait(MO_LP_problem, linear_objective, true)
SetProblemTrait(MO_LP_problem, gradients, true)
FinalizeProblemType(MO_LP_problem)

DeclareProblemType(MO_MILP)
SetProblemTrait(MO_MILP_problem, multiple_objectives, true)
SetProblemTrait(MO_MILP_problem, reals, true)
SetProblemTrait(MO_MILP_problem, integers, true)
SetProblemTrait(MO_MILP_problem, binarys, true)
SetProblemTrait(MO_MILP_problem, linear_constraints, true)
SetProblemTrait(MO_MILP_problem, linear_objective, true)
FinalizeProblemType(MO_MILP_problem)



DeclareProblemType(ILP)
SetProblemTrait(ILP_problem, integers, true)
SetProblemTrait(ILP_problem, linear_constraints, true)
FinalizeProblemType(ILP_problem)

namespace colin {
// Alias ILP to IP
typedef ILP_problem IP_problem;
} // namespace colin

DeclareProblemType(INLP)
SetProblemTrait(INLP_problem, integers, true)
SetProblemTrait(INLP_problem, linear_constraints, true)
SetProblemTrait(INLP_problem, nonlinear_constraints, true)
FinalizeProblemType(INLP_problem)

DeclareProblemType(UINLP)
SetProblemTrait(UINLP_problem, integers, true)
FinalizeProblemType(UINLP_problem)

DeclareProblemType(SUINLP)
SetProblemTrait(SUINLP_problem, integers, true)
SetProblemTrait(SUINLP_problem, nondeterministic_response, true)
FinalizeProblemType(SUINLP_problem)

#endif // !defined(DOXYGEN)


#endif // colin_ProblemTraits_h

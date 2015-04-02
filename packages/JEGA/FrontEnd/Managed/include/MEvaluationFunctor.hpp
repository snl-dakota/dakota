/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Definition of class EvaluationFunctor.

    NOTES:

        See notes under Class Definition section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Feb 09 15:11:02 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the EvaluationFunctor class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_MANAGED_MEVALUATIONFUNCTOR_HPP
#define JEGA_FRONTEND_MANAGED_MEVALUATIONFUNCTOR_HPP

#pragma once


/*
================================================================================
Includes
================================================================================
*/
#include <MConfig.hpp>







/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/








/*
================================================================================
Namespace Aliases
================================================================================
*/
#pragma managed







/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace FrontEnd {
        namespace Managed {





/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/







/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/








/*
================================================================================
Class Definition
================================================================================
*/
/**
 * \brief An interface class for any class that wishes to be called by an
 *        MEvaluator to perform objective function and constraint
 *        calculations.
 *
 * This interface requires implementation of only one method in which
 * raw objective function and constraint values should be computed and
 * recorded.  Although JEGA is a genetic algorithm package, do not attempt
 * to compute "fitnesses" in place of proper responses unless you REALLY know
 * what you're doing.  Leave that to JEGA.
 */
MANAGED_INTERFACE(public, MEvaluationFunctor)
{
    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /**
         * \brief Override this method to compute responses at the supplied
         *        design point \a des.
         *
         * Do not write change the variable values of \a des during the
         * execution of this method.  Managed code does not support the const
         * keyword or it would not be allowed.
         *
         * Any exceptions thrown out of this method will be absorbed by the
         * caller and will have the same effect as if the method had returned
         * false.
         *
         * \param des The Design containing the variable values at which to
         *            evaluate the objectives and constraints.
         * \return Your method should return true if the evaluation completed
         *         successfully and false otherwise.  This provides you a
         *         mechanism to reject certain design configurations as
         *         "un-evaluatable".  If you return false, the objectives and
         *         constraints will be ignored and the solution point will be
         *         marked as illconditioned.
         */
        virtual
        bool
        Evaluate(
            MDesign MOH des,
            System::Collections::Generic::List<MDesign MOH> MOH injections
            ) = 0;

        virtual
        bool
        Evaluate(
            DesignVector MOH designs,
            System::Collections::Generic::List<MDesign MOH> MOH injections
            ) = 0;

        virtual
        bool
        IsBatchEvaluator(
            ) = 0;

        virtual
        bool
        MayInjectDesigns(
            ) = 0;

}; // class EvaluationFunctor

//
//MANAGED_ABSTRACT_CLASS(public, MBasicEvaluationFunctorImpl) :
//    public MEvaluationFunctor
//{
//    public:
//
//        virtual
//        bool
//        Evaluate(
//            MDesign MOH des
//            ) = 0;
//
//        virtual
//        bool
//        Evaluate(
//            DesignVector MOH designs
//            )
//        {
//            bool ret = true;
//            for each (MDesign MOH des in designs)
//                ret &= this->Evaluate(des);
//            return ret;
//        }
//
//}; // class MBasicEvaluationFunctorImpl

/*
================================================================================
End Namespace
================================================================================
*/
        } // namespace Managed
    } // namespace FrontEnd
} // namespace JEGA








/*
================================================================================
Include Inlined Functions File
================================================================================
*/
// Not using an Inlined Functions File.



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_FRONTEND_MANAGED_MEVALUATIONFUNCTOR_HPP

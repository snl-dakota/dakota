/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Definition of class MGeneticAlgorithmEvaluator.

    NOTES:

        See notes under "Document this File" section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Feb 23 15:08:29 2009 - Original Version ()

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MGeneticAlgorithmEvaluator class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_MANAGED_MGENETICALGORITHMEVALUATOR_HPP
#define JEGA_FRONTEND_MANAGED_MGENETICALGORITHMEVALUATOR_HPP

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
namespace JEGA
{
    namespace Algorithms
    {
        class GeneticAlgorithmEvaluator;
    }
}






/*
================================================================================
Namespace Aliases
================================================================================
*/
#pragma managed
#using <mscorlib.dll>







/*
================================================================================
Namespace Aliases
================================================================================
*/








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
MANAGED_CLASS_FORWARD_DECLARE(public, MGeneticAlgorithmEvaluator);







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
 * \brief
 *
 *
 */
MANAGED_CLASS(public, MGeneticAlgorithmEvaluator)
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        const JEGA::Algorithms::GeneticAlgorithmEvaluator* _evaler;




    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Returns the current number of evaluations completed.
        /**
         * \return The number of evaluations recorded so far for this
         *         evaluator.
         */
        std::size_t
        GetNumberEvaluations(
            );

        /// Gets the maximum allowable number of evaluations for the algorithm.
        /**
         * \return The maximum allowable number of evaluations.
         */
        std::size_t
        GetMaxEvaluations(
            );

        /// Allows access to the current value of evaluation concurrency.
        /**
         * \return The current evaluation concurrency used by this evaluator.
         */
        std::size_t
        GetEvaluationConcurrency(
            );


    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Indicates if the maximum number of evaluations has been exceeded.
        /**
         * \return True if the current number of evaluations is strictly
         *         greater than the max allowable.
         */
        bool
        IsMaxEvalsExceeded(
            );

        /**
         * \brief Indicates if the maximum number of evaluations has been
         *        reached or exceeded.
         *
         * \return True if the current number of evaluations is >= the max
         *         allowable.
         */
        bool
        IsMaxEvalsReached(
            );

        void
        Detach(
            );

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:





    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        virtual
        const JEGA::Algorithms::GeneticAlgorithmEvaluator&
        Manifest(
            );

    protected:


    private:


    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:





    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /**
         * \brief Constructs a Managed GA Evaluator wrapper using the supplied
         *        GeneticAlgorithmEvaluator.
         *
         * \param toWrap The evaluator used to construct this managed wrapper
         *               class.
         */
        MGeneticAlgorithmEvaluator(
            const JEGA::Algorithms::GeneticAlgorithmEvaluator* toWrap
            );



}; // class MGeneticAlgorithmEvaluator



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
#endif // JEGA_FRONTEND_MANAGED_MGENETICALGORITHMEVALUATOR_HPP

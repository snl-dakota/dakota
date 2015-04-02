/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class SimpleFunctorEvaluatorCreator.

    NOTES:

        See notes under Class Definition section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.1.0

    CHANGES:

        Wed Oct 18 15:58:41 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the SimpleFunctorEvaluatorCreator class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_SIMPLEFUNCTOREVALUATORCREATOR_HPP
#define JEGA_FRONTEND_SIMPLEFUNCTOREVALUATORCREATOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Evaluators/SimpleFunctorEvaluator.hpp>
#include <../FrontEnd/Core/include/EvaluatorCreator.hpp>








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








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace FrontEnd {





/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
class SimpleFunctorEvaluatorCreator;







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
 * \brief An EvaluatorCreator for the creation of a SimpleFunctorEvaluator.
 *
 * This is a front end convenience to aid in the use of the simplest evaluator
 * hook into JEGA.
 */
class SimpleFunctorEvaluatorCreator :
    public EvaluatorCreator
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

        /// The evaluation functor that will be used by any created evaluators.
        JEGA::Algorithms::SimpleFunctorEvaluator::Functor* _theFunc;




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


    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:





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

        /// Overriden to return a newly created SimpleFunctorEvaluator.
        /**
         * The GA will assume ownership of the evaluator so you needn't worry
         * about keeping track of it for destruction.
         *
         * \param algorithm The GA for which the evaluator is to be created.
         * \return A pointer to a newly created SimpleFunctorEvaluator.
         */
        virtual
        JEGA::Algorithms::GeneticAlgorithmEvaluator*
        CreateEvaluator(
            JEGA::Algorithms::GeneticAlgorithm& algorithm
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
         * \brief Constructs a new SimpleFunctorEvaluatorCreator to create a
         *        SimpleFunctorEvaluator using the supplied functor.
         *
         * \param theFunc The evaluation functor that will be used by any
         *                created evaluators to perform function evaluations.
         */
        SimpleFunctorEvaluatorCreator(
            JEGA::Algorithms::SimpleFunctorEvaluator::Functor* theFunc
            );



}; // class SimpleFunctorEvaluatorCreator



/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace FrontEnd
} // namespace JEGA







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
#include "inline/SimpleFunctorEvaluatorCreator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_FRONTEND_SIMPLEFUNCTOREVALUATORCREATOR_HPP

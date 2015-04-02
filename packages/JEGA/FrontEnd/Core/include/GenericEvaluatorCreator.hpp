/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Front End

    CONTENTS:

        Definition of class GenericEvaluatorCreator.

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

        Tue Nov 21 09:26:51 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the GenericEvaluatorCreator class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_GENERICEVALUATORCREATOR_HPP
#define JEGA_FRONTEND_GENERICEVALUATORCREATOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

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
template <typename EvalT>
class GenericEvaluatorCreator;







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
 * \brief Simple implementation of the EvaluatorCreator interface that creates
 *        a new EvalT using a constructor taking the algorithm.
 *
 * \param EvalT Must be a JEGA::Algorithms::GeneticAlgorithmEvaluator
 *              derivative.
 */
template <typename EvalT>
class GenericEvaluatorCreator :
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

        /// Overriden to return a newly created EvalT.
        /**
         * The GA will assume ownership of the evaluator so you needn't worry
         * about keeping track of it for destruction.
         *
         * \param algorithm The GA for which the evaluator is to be created.
         * \return A pointer to a newly created EvalT.
         */
        virtual
        JEGA::Algorithms::GeneticAlgorithmEvaluator*
        CreateEvaluator(
            JEGA::Algorithms::GeneticAlgorithm& algorithm
            )
        {
            return new EvalT(algorithm);
        }

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





}; // class GenericEvaluatorCreator



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
// not using an inlined functions file.



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_FRONTEND_GENERICEVALUATORCREATOR_HPP

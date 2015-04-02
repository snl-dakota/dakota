/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Front End

    CONTENTS:

        Definition of class EvaluatorCreator.

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

        Fri Feb 10 15:59:03 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the EvaluatorCreator class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_EVALUATORCREATOR_HPP
#define JEGA_FRONTEND_EVALUATORCREATOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>





/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace Algorithms
    {
        class GeneticAlgorithm;
        class GeneticAlgorithmEvaluator;
    }
}






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
class EvaluatorCreator;







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
 * \brief An interface for a class that must be provided to an AlgorithmConfig
 *        that creates and returns a GeneticAlgorithmEvaluator.
 *
 * The application class will use this creator when it comes time to install
 * an evaluator into the GA that is solving the problem.  Sublcasses must
 * implement the CreateEvaluator method.
 */
class JEGA_SL_IEDECL EvaluatorCreator
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

        /// Override to return a newly created GA Evaluator derivative.
        /**
         * The GA will assume ownership of the evaluator so you needn't worry
         * about keeping track of it for destruction.
         *
         * \param algorithm The GA for which the evaluator is to be created.
         * \return A pointer to a newly created Evaluator.
         */
        virtual
        JEGA::Algorithms::GeneticAlgorithmEvaluator*
        CreateEvaluator(
            JEGA::Algorithms::GeneticAlgorithm& algorithm
            ) = 0;

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

        /// Destructs an EvaluatorCreator
        virtual
        ~EvaluatorCreator(
            ) {}



}; // class EvaluatorCreator



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
// Not using an Inlined Functions File.



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_FRONTEND_EVALUATORCREATOR_HPP

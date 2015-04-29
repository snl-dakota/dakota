/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Definition of class MGeneticAlgorithm.

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

        Wed Feb 14 16:01:13 2007 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MGeneticAlgorithm class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_MANAGED_MGENETICALGORITHM_HPP
#define JEGA_FRONTEND_MANAGED_MGENETICALGORITHM_HPP

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
namespace JEGA {
    namespace Algorithms {
        class GeneticAlgorithm;
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
MANAGED_CLASS_FORWARD_DECLARE(public, MSolution);
MANAGED_CLASS_FORWARD_DECLARE(public, MGeneticAlgorithm);
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
MANAGED_CLASS(public, MGeneticAlgorithm) :
    public System::IDisposable
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

        JEGA::Algorithms::GeneticAlgorithm* _theAlg;

        MGeneticAlgorithmEvaluator MOH _wrappedEvaler;



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

        /**
         * \brief Allows access to the JEGA genetic algorithm object wrapped
         *        by this managed type.
         *
         * \return The JEGA genetic algorithm that will actually be used by
         *         JEGA to complete an optimization.
         */
        JEGA::Algorithms::GeneticAlgorithm&
        Manifest(
            );

        void
        DissociateGA(
            );

        /// Gets one best design in the current population.
        /**
         * This is a very subjective matter especially for a MOGA.  This method
         * is only here to support the needs of DAKOTA and should be avoided
         * at all costs.
         *
         * \return The single best Design currently in the population.
         */
        MSolution MOH
        GetBestDesign(
            );

        /// Override to return the current set of solutions of this algorithm.
        /**
         * This should return whatever this algorithm would claim to be the
         * optimal solutions if it had to stop on the spot.  This can be
         * an expensive call and is meant primarly for use after the call
         * to AlgorithmFinalize at which time this should return the final
         * solution found by the algorithm.
         *
         * This is different from the GetBestDesign method in that it may
         * return multiple Designs.
         *
         * \return The set of solutions currently found by this algorithm
         */
        SolutionVector MOH
        GetCurrentSolution(
            );

        SolutionVector MOH
        GetCurrentPopulation(
            );

        eddy::utilities::uint64_t
        GetGenerationNumber(
            );

        eddy::utilities::uint64_t
        GetPopulationSize(
            );

        eddy::utilities::uint64_t
        GetNDV(
            );

        eddy::utilities::uint64_t
        GetNOF(
            );

        eddy::utilities::uint64_t
        GetNCN(
            );

        void
        FlushLogStreams(
            );

        MGeneticAlgorithmEvaluator MOH
        GetEvaluator(
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

        /// Implementation of the Dispose method for the IDisposable interface.
        /**
         * This method destroys the underlying JEGA algorithm configuration.
         * It is safe to call this more than once but it is not safe to use
         * the other methods of this class once this has been called.
         *
         * The destructor should call this method.
         */
        virtual
        void
        MANAGED_DISPOSE(
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

        MGeneticAlgorithm(
            JEGA::Algorithms::GeneticAlgorithm* wrapped
            );

        ~MGeneticAlgorithm(
            );


}; // class MGeneticAlgorithm



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
#endif // JEGA_FRONTEND_MANAGED_MGENETICALGORITHM_HPP

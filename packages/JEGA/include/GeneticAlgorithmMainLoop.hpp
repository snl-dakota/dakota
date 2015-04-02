/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class GeneticAlgorithmMainLoop

    NOTES:

        See notes under section "Class Definition" of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed May 21 15:26:50 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the GeneticAlgorithmMainLoop class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_GENETICALGORITHMMAINLOOP_HPP
#define JEGA_ALGORITHMS_GENETICALGORITHMMAINLOOP_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmOperator.hpp>
#include <utilities/include/int_types.hpp>



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
    namespace Algorithms {







/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
class GeneticAlgorithmMainLoop;




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

/// The base class for all GeneticAlgorithm main loops.
/**
 * The main loop is responsible for executing the generations.  It determines
 * in which order the operators will be called.  It is not responsible for
 * initializing the population.  That is the job of the
 * GeneticAlgorithm::AlgorithmInitialize method which uses a
 * GeneticAlgorithmInitializer.
 *
 * This base class provides a means of tracking the number of generations
 * executed which must be done by derived classes if it is to be done at all.
 * It also requires of it's derivatives implementation of the RunGeneration
 * method.  It requires no configuration input beyond that required by its base
 * class.
 */
class JEGA_SL_IEDECL GeneticAlgorithmMainLoop :
    public GeneticAlgorithmOperator
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

        /// The current generation number
        /**
         * This variable is useful to determine which generation is being run
         * which also indicates how many have been run previously.
         */
        eddy::utilities::uint64_t _currGen;



    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the current generation number to \a num.
        /**
         * \param num The number of the current generation.
         */
        inline
        void
        SetCurrentGeneration(
            eddy::utilities::uint64_t num
            );




    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Gets the current generation number of this main loop.
        /**
         * \return The current generation number of this main loop.
         */
        inline
        eddy::utilities::uint64_t
        GetCurrentGeneration(
            ) const;




    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Increments the current generation by \a by.
        /**
         * \param by The number by which to increment the current generation
         *           number.  The default is 1.
         */
        inline
        void
        IncCurrentGeneration(
            eddy::utilities::uint64_t by = 1
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

        /// This is the method in which derived main-loops do whatever they do.
        /**
         * It is called by the genetic algorithm
         * once per generation.  Required information can be retrieved
         * through the GeneticAlgorithmOperator::GetAlgorithm()
         * base class method.
         *
         * The method should return true if it wishes to be called again
         * at some later time and false otherwise.  Typically, it will return
         * true when the converger indicates that convergence has occurred.
         *
         * \return Should return true if the algorithm should continue and
         *         false otherwise.  This is the logical negation of whether
         *         or not the algorithm has converged.
         */
        virtual
        bool
        RunGeneration(
            ) = 0;

        /// Retrieves specific parameters using Get...FromDB methods.
        /**
         * This method is used to extract needed information for this operator.
         * It does so using the "Get...FromDB" class of methods from the
         * GeneticAlgorithmOperator base class.
         *
         * This version does nothing.
         *
         * \param db The database of parameters from which the configuration
         *           information can be retrieved.
         * \return true if the extraction completed successfully and false
         *         otherwise.
         */
        virtual
        bool
        PollForParameters(
            const JEGA::Utilities::ParameterDatabase& db
            );

        /// Returns the name of the type of this operator.
        /**
         * \return The string "Main Loop".
         */
        virtual
        std::string
        GetType(
            ) const;

    protected:

        /**
         * \brief Logs relevant information about the population and state of
         *        the algorithm.
         */
        virtual
        void
        PostLoopReport(
            );

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

        /// Constructs a GeneticAlgorithmMainLoop for use by \a algorithm.
        /**
         * \param algorithm The GA for which this main loop is being
         *                  constructed.
         */
        GeneticAlgorithmMainLoop(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a GeneticAlgorithmMainLoop.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        GeneticAlgorithmMainLoop(
            const GeneticAlgorithmMainLoop& copy
            );

        /// Copy constructs a GeneticAlgorithmMainLoop for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this main loop is being
         *                  constructed.
         */
        GeneticAlgorithmMainLoop(
            const GeneticAlgorithmMainLoop& copy,
            GeneticAlgorithm& algorithm
            );



}; // class GeneticAlgorithmMainLoop


/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA







/*
================================================================================
Include Inlined Methods File
================================================================================
*/
#include "./inline/GeneticAlgorithmMainLoop.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_GENETICALGORITHMMAINLOOP_HPP

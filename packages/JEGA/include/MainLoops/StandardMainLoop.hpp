/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class StandardMainLoop.

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

        Wed Jun 04 11:05:38 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the StandardMainLoop class.
 */





/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_STANDARDMAINLOOP_HPP
#define JEGA_ALGORITHMS_STANDARDMAINLOOP_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmMainLoop.hpp>








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
class StandardMainLoop;







/*
================================================================================
Class Definition
================================================================================
*/

/// Executes the operators in typical order.
/**
 * This main loop executes the operators in the following order:
 * \verbatim
        Crossover
        Mutation
        Evaluation
        Fitness Assessment
        Selection
        Niche Pressure Application
        Convergence Testing
   \endverbatim
 * and does nothing unusual in between.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class StandardMainLoop :
    public GeneticAlgorithmMainLoop
{
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





    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:





    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the proper name of this operator.
        /**
         * \return The string "standard".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This main loop executes the typical main loop of a GA.
            The algorithm proceeds as shown (recall that initialization
            has already occurred):
                while not Converged
                     Crossover
                     Mutate
                     Evaluate
                     Assess Fitness
                     Niche Pressure Application
                     Select
                end while
           \endverbatim.
         *
         * \return A description of the operation of this operator.
         */
        static
        const std::string&
        Description(
            );

        /**
         * \brief Returns a new instance of this operator class for use by
         *        \a algorithm.
         *
         * \param algorithm The GA for which the new main loop is to be used.
         * \return A new, default instance of a StandardMainLoop.
         */
        static
        GeneticAlgorithmOperator*
        Create(
            GeneticAlgorithm& algorithm
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

        /// Performs one trip through the main loop.
        /**
         * The operations are executed in the order described in the
         * documetation of this class.
         *
         * \return True if this loop wishes to be called again and false
         *         otherwise which means that convergence has occurred.
         */
        virtual
        bool
        RunGeneration(
            );

        /// Returns the proper name of this operator.
        /**
         * \return See Name().
         */
        virtual
        std::string
        GetName(
            ) const;

        /// Returns a full description of what this operator does and how.
        /**
         * \return See Description().
         */
        virtual
        std::string
        GetDescription(
            ) const;

        /**
         * \brief Creates and returns a pointer to an exact duplicate of this
         *        operator.
         *
         * \param algorithm The GA for which the clone is being created.
         * \return A clone of this operator.
         */
        virtual
        GeneticAlgorithmOperator*
        Clone(
            GeneticAlgorithm& algorithm
            ) const;

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

        /// Constructs an StandardMainLoop for use by \a algorithm.
        /**
         * \param algorithm The GA for which this main loop is
         *                  being constructed.
         */
        StandardMainLoop(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an StandardMainLoop.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        StandardMainLoop(
            const StandardMainLoop& copy
            );

        /// Copy constructs an StandardMainLoop for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this main loop is
         *                  being constructed.
         */
        StandardMainLoop(
            const StandardMainLoop& copy,
            GeneticAlgorithm& algorithm
            );

}; // class StandardMainLoop


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
#include "./inline/StandardMainLoop.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_STANDARDMAINLOOP_HPP

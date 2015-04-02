/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class BestFitnessTrackerConverger.

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

        Tue Jul 22 16:10:25 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the BestFitnessTrackerConverger class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_BESTFITNESSTRACKERCONVERGER_HPP
#define JEGA_ALGORITHMS_BESTFITNESSTRACKERCONVERGER_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Convergers/MetricTrackerConvergerBase.hpp>






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
class BestFitnessTrackerConverger;







/*
================================================================================
Class Definition
================================================================================
*/

/**
 * \brief A converger that keeps track of the Best fitness over some
 * number of generations.
 *
 * This converger reports converged if the percent change in the best
 * fitness in the population is below the supplied percent change over
 * the supplied number of generations.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class BestFitnessTrackerConverger :
    public MetricTrackerConvergerBase
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
         * \return The string "best_fitness_tracker".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This converger returns true if the best observed fitness
            value changes by less than the input percent change over the input
            number of generations.
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
         * \param algorithm The GA for which the new converger is to be used.
         * \return A new, default instance of a BestFitnessTrackerConverger.
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


        /// This method returns the best fitness value in \a group.
        /**
         * \param group The group to use in the convergence check.
         * \param fitnesses The fitnesses of the designs in \a group.
         * \return The value for this converger to track based on the designs
         *         in \a group and their associated \a fitnesses.  In this case
         *         it is the fitness of the Design in \a group with the best
         *         (highest) associated fitness.
         */
        virtual
        double
        GetMetricValue(
            const JEGA::Utilities::DesignGroup& group,
            const FitnessRecord& fitnesses
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


        /**
         * \brief Constructs a BestFitnessTrackerConverger for use by \a
         *         algorithm.
         *
         * \param algorithm The GA for which the new converger is to be used.
         */
        BestFitnessTrackerConverger(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a BestFitnessTrackerConverger.
        /**
         * \param copy The converger from which properties are to be duplicated
         *             into this.
         */
        BestFitnessTrackerConverger(
            const BestFitnessTrackerConverger& copy
            );

        /**
         * \brief Copy constructs a BestFitnessTrackerConverger for use by
         *        \a algorithm.
         *
         * \param copy The converger from which properties are to be duplicated
         *             into this.
         * \param algorithm The GA for which the new converger is to be used.
         */
        BestFitnessTrackerConverger(
            const BestFitnessTrackerConverger& copy,
            GeneticAlgorithm& algorithm
            );

}; // class BestFitnessTrackerConverger



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
#include "./inline/BestFitnessTrackerConverger.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_BESTFITNESSTRACKERCONVERGER_HPP

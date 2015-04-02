/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class AverageFitnessTrackerConverger.

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

        Tue Jul 29 09:50:11 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the AverageFitnessTrackerConverger class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_AVERAGEFITNESSTRACKERCONVERGER_HPP
#define JEGA_ALGORITHMS_AVERAGEFITNESSTRACKERCONVERGER_HPP







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
class AverageFitnessTrackerConverger;







/*
================================================================================
Class Definition
================================================================================
*/

/**
 * \brief A converger that keeps track of the Average fitness over some
 *        number of generations.
 *
 * This converger reports converged if the percent change in the average
 * fitness is below the supplied percent change over the supplied number
 * of generations.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class AverageFitnessTrackerConverger :
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


        /// Returns the proper name of this operator.
        /**
         * \return The string "average_fitness_tracker".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This converger returns true if the average observed fitness
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
         * \return A new, default instance of an
         *         AverageFitnessTrackerConverger.
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
         * \param algorithm The GA for which the new converger is to be used.
         * \return A new AverageFitnessTrackerConverger with properties
         *         duplicated from this.
         */
        virtual
        GeneticAlgorithmOperator*
        Clone(
            GeneticAlgorithm& algorithm
            ) const;

    protected:


        /// This method returns the average fitness value in \a group.
        /**
         * \param group The group to use in the convergence check.
         * \param fitnesses The fitnesses of the designs in \a group.
         * \return The value for this converger to track based on the designs
         *         in \a group and their associated \a fitnesses.  In this case
         *         it is the average of all fitnesses of all Designs in
         *         \a group.
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
         * \brief Constructs a AverageFitnessTrackerConverger for use by \a
         *         algorithm.
         *
         * \param algorithm The GA for which the new converger is to be used.
         */
        AverageFitnessTrackerConverger(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a AverageFitnessTrackerConverger.
        /**
         * \param copy The converger from which properties are to be duplicated
         *             into this.
         */
        AverageFitnessTrackerConverger(
            const AverageFitnessTrackerConverger& copy
            );

        /**
         * \brief Copy constructs a AverageFitnessTrackerConverger for use by
         *        \a algorithm.
         *
         * \param copy The converger from which properties are to be duplicated
         *             into this.
         * \param algorithm The GA for which the new converger is to be used.
         */
        AverageFitnessTrackerConverger(
            const AverageFitnessTrackerConverger& copy,
            GeneticAlgorithm& algorithm
            );

}; // class AverageFitnessTrackerConverger


/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
#include "./inline/AverageFitnessTrackerConverger.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_AVERAGEFITNESSTRACKERCONVERGER_HPP


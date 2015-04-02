/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class WeightedSumOnlyFitnessAssessor.

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

        Wed Sep 17 08:18:58 2008 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the WeightedSumOnlyFitnessAssessor class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_WEIGHTEDSUMONLYFITNESSASSESSOR_HPP
#define JEGA_ALGORITHMS_WEIGHTEDSUMONLYFITNESSASSESSOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../SOGA/include/FitnessAssessors/PenaltyFitnessAssessorBase.hpp>






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
class WeightedSumOnlyFitnessAssessor;







/*
================================================================================
Class Definition
================================================================================
*/
/**
 * \brief A penalty fitness assessor that applies no penalty for constraint
 *        violation.
 *
 * This fitness assessor assigns the fitness of a Design to be the (negated)
 * weighted sum of objectives.  No penalty is assigned for constraint violation.
 * The multiplier obtained by the base class is not used.  Effectively, this is
 * a penalty fitness assessor that does not install a penalty.  Because of this,
 * it is only appropriate to use this assessor with a selector that accounts for
 * penalties explicitly or in a group of operators in which constraints and
 * variable bounds are handled in some other way.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class WeightedSumOnlyFitnessAssessor :
    public PenaltyFitnessAssessorBase
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
         * \return The string "weighted_sum_only".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
                This fitness assessor specializes the PenaltyFitnessAssessor
                but does not alter the weighted sum values.
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
         * \param algorithm The GA for which the new fitness assessor is to
         *                  be used.
         * \return A new, default instance of an
         *         WeightedSumOnlyFitnessAssessor.
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

        /// This method assigns fitness according to the weighted sums.
        /**
         * this is called by the genetic algorithm periodically.  Required
         * information can be retrieved through the
         * GeneticAlgorithmOperator::GetAlgorithm() base class method.
         *
         * This method considers all Designs in all groups of \a groups
         * and assess a singular fitness value to each.
         *
         * This version adds no penalty values to the weighted sums of the
         * designs in \a groups computed using the weights supplied to the
         * algorithm.
         *
         * \param groups The groups of Designs that should have their fitnesses
         *               assessed.
         * \return A record of the fitness values assessed to each of the
         *         designs in the passed in groups.
         */
        virtual
        const FitnessRecord*
        AssessFitness(
            const JEGA::Utilities::DesignGroupVector& groups
            );

        /**
         * \brief This method applies the exterior penalty to the Designs of
         *        \a groups.
         *
         * \param groups The groups of designs to which to assess constraint
         *               penalties.
         * \return A mapping of the designs in \a groups to the penalty values
         *         assigned by this fitness assessor.
         */
        virtual
        DesignDoubleMap
        ApplyPenalties(
            const JEGA::Utilities::DesignGroupVector& groups
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

        /**
         * \brief Constructs an WeightedSumOnlyFitnessAssessor for use by
         *        \a algorithm.
         *
         * \param algorithm The GA for which this fitness assessor is being
         *                  constructed.
         */
        WeightedSumOnlyFitnessAssessor(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an WeightedSumOnlyFitnessAssessor.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        WeightedSumOnlyFitnessAssessor(
            const WeightedSumOnlyFitnessAssessor& copy
            );

        /**
         * \brief Copy constructs an WeightedSumOnlyFitnessAssessor for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this fitness assessor is being
         *                  constructed.
         */
        WeightedSumOnlyFitnessAssessor(
            const WeightedSumOnlyFitnessAssessor& copy,
            GeneticAlgorithm& algorithm
            );


}; // class WeightedSumOnlyFitnessAssessor


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
#include "./inline/WeightedSumOnlyFitnessAssessor.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_WEIGHTEDSUMONLYFITNESSASSESSOR_HPP

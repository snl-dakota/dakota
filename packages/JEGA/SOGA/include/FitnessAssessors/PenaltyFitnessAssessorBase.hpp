/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class PenaltyFitnessAssessorBase.

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

        Tue Jul 29 10:46:50 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the PenaltyFitnessAssessorBase class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_PENALTYFITNESSASSESSORBASE_HPP
#define JEGA_ALGORITHMS_PENALTYFITNESSASSESSORBASE_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/JEGATypes.hpp>
#include <GeneticAlgorithmFitnessAssessor.hpp>






/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace Utilities
    {
        template <typename VT>
        class DesignValueMap;
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
    namespace Algorithms {






/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
class PenaltyFitnessAssessorBase;





/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/

/// A shorthand for a DesignValueMap mapping designs to doubles.
typedef
JEGA::Utilities::DesignValueMap<double>
DesignDoubleMap;




/*
================================================================================
Class Definition
================================================================================
*/

/**
 * \brief A base class for fitness assessors using penalty functions
 *
 * This fitness assessor uses the formulation:
 * \verbatim
       Psi(x) = Ft(x) + P(x)
   \endverbatim
 * where Psi is the pseudo objective function value, Ft(x) is the weighted
 * sum of objectives, and P(x) is the penalty function value.
 *
 * Derived classes can specialize the means by which P(x) is computed.
 *
 * The penalty multiplier is extracted from the parameter database using the
 * name "method.constraint_penalty".  It is extracted as a double value.  If it
 * is not supplied in the parameter database, the default value as defined by
 * DEFAULT_MULTIPLIER will be used.  The weights to be used in computing the
 * weighted sum values are extracted from the parameter database using the name
 * "responses.multi_objective_weights".  If the weights are not in the database,
 * the default value of 1/nof is used for each.  If they are in but incomplete,
 * then the missing weights are given the value 1. These are required in
 * addition to any requirements of the base class.
 */
class PenaltyFitnessAssessorBase :
    public GeneticAlgorithmFitnessAssessor
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The default penalty multiplier value.
        static const double DEFAULT_MULTIPLIER;

        /// The penalty multiplier for those who wish to use it.
        /**
         * Be cautious if using the UnconstrainedStatistician.  It may be
         * the case that you need only inform it of the multiplier value.  Some
         * of the penalty assessment methods of the statistician account for
         * the multiplier and so if you do also, it will be doubly applied.
         */
        double _multiplier;

        /// This is the vector of weights to apply to the objective functions
        JEGA::DoubleVector _weights;


    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the current value of the penalty multiplier.
        /**
         * This method logs a verbose level logging entry informing of the new
         * multiplier value.
         *
         * \param multiplier The new constraint penalty multiplier value.
         */
        void
        SetMultiplier(
            double multiplier
            );

        /// Sets the weights for this algorithm to use.
        /**
         * \param weights The weights used by this algorithm to create weighted
         *                sums of objectives.
         */
        void
        SetWeights(
            const JEGA::DoubleVector& weights
            );

    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Gets the current value of the penalty multiplier.
        /**
         * \return The current value that multiplies the constraint penalty.
         */
        inline
        double
        GetMultiplier(
            ) const;

        /// Returns the set of weights for this algorithm by const_reference
        /**
         * \return The weights currently being used by this operator to create
         *         weighted sums of objectives.
         */
        inline
        const JEGA::DoubleVector&
        GetWeights(
            ) const;


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

        /// This method assigns fitness according to a penalty formulation.
        /**
         * This is called by the genetic algorithm periodically.  Required
         * information can be retrieved through the
         * GeneticAlgorithmOperator::GetAlgorithm() base class method.
         *
         * This method should consider all Designs in all groups of \a groups
         * and assess a singular fitness value to each.
         *
         * This version adds the penalty values computed in derived classes
         * using the ApplyPenalties method to the weighted sums of the designs
         * in \a groups computed using the weights supplied to the algorithm.
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

        /// Retrieves specific parameters using Get...FromDB methods.
        /**
         * This method is used to extract needed information for this
         * operator.  It does so using the "Get...FromDB" class
         * of methods from the GeneticAlgorithmOperator base class.
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

    protected:


        /// Derived classes must specialize this to apply penalties.
        /**
         * \param groups The groups of designs to which to assess constraint
         *               penalties.
         * \return A mapping of the designs in \a groups to the penalty values
         *         assigned by this fitness assessor.
         */
        virtual
        DesignDoubleMap
        ApplyPenalties(
            const JEGA::Utilities::DesignGroupVector& groups
            ) = 0;

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

        /// Constructs an PenaltyFitnessAssessorBase for use by \a algorithm.
        /**
         * \param algorithm The GA for which this fitness assessor is being
         *                  constructed.
         */
        PenaltyFitnessAssessorBase(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an PenaltyFitnessAssessorBase.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        PenaltyFitnessAssessorBase(
            const PenaltyFitnessAssessorBase& copy
            );

        /**
         * \brief Copy constructs an PenaltyFitnessAssessorBase for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this fitness assessor is being
         *                  constructed.
         */
        PenaltyFitnessAssessorBase(
            const PenaltyFitnessAssessorBase& copy,
            GeneticAlgorithm& algorithm
            );


}; // class PenaltyFitnessAssessorBase


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
#include "./inline/PenaltyFitnessAssessorBase.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_PENALTYFITNESSASSESSORBASE_HPP

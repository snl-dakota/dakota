/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class ExteriorPenaltyFitnessAssessor.

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

        Wed Jul 23 08:18:58 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the ExteriorPenaltyFitnessAssessor class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_EXTERIORPENALTYFITNESSASSESSOR_HPP
#define JEGA_ALGORITHMS_EXTERIORPENALTYFITNESSASSESSOR_HPP







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
class ExteriorPenaltyFitnessAssessor;







/*
================================================================================
Class Definition
================================================================================
*/
/**
 * \brief A penalty fitness assessor that uses the exterior penalty function
 *        formulation.
 *
 * This fitness assessor assigns the fitness of a Design to be the (negated)
 * sum of the weighted sum of objectives and a penalty term computed using
 * the exterior penalty function formulation.  A static multiplier can be
 * supplied for the penalty term.  Static meaning that its value does not
 * change as the optimization progresses.
 *
 * As this does not function in the same manner as a SUMT method, it is not
 * necessary (or even sensible) to vary the multiplier.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class ExteriorPenaltyFitnessAssessor :
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
         * \return The string "exterior_penalty".
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
                for the exterior penalty function formulation.
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
         *         ExteriorPenaltyFitnessAssessor.
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
         * \brief Constructs an ExteriorPenaltyFitnessAssessor for use by
         *        \a algorithm.
         *
         * \param algorithm The GA for which this fitness assessor is being
         *                  constructed.
         */
        ExteriorPenaltyFitnessAssessor(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an ExteriorPenaltyFitnessAssessor.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        ExteriorPenaltyFitnessAssessor(
            const ExteriorPenaltyFitnessAssessor& copy
            );

        /**
         * \brief Copy constructs an ExteriorPenaltyFitnessAssessor for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this fitness assessor is being
         *                  constructed.
         */
        ExteriorPenaltyFitnessAssessor(
            const ExteriorPenaltyFitnessAssessor& copy,
            GeneticAlgorithm& algorithm
            );


}; // class ExteriorPenaltyFitnessAssessor


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
#include "./inline/ExteriorPenaltyFitnessAssessor.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_EXTERIORPENALTYFITNESSASSESSOR_HPP

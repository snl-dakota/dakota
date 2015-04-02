/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class NullFitnessAssessor.

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

        Thu May 29 09:26:26 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the NullFitnessAssessor class.
 */





/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_NULLFITNESSASSESSOR_HPP
#define JEGA_ALGORITHMS_NULLFITNESSASSESSOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmFitnessAssessor.hpp>








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
class NullFitnessAssessor;







/*
================================================================================
Class Definition
================================================================================
*/

/// The null object of fitness assessors.
/**
 * This fitness assessor does nothing.  It merely serves as a null object.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class NullFitnessAssessor :
    public GeneticAlgorithmFitnessAssessor
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
         * \return The string "null_fitness".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This fitness assessor does nothing.  It is intended to serve as
            a null object for fitness assessors.  Use it if you do not wish
            to perform any fitness assessment.
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
         * \param algorithm The GA for which the new fitness assessor is to be
         *        used.
         * \return A new, default instance of a NullFitnessAssessor.
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


        /// This version of AssessFitness does absolutely nothing.
        /**
         * \param groups The groups of all Designs that should be assessed for
         *               fitness together.
         * \return A record of all fitnesses computed by this assessor mapped
         *         to the Design to which that value is associated.
         */
        virtual
        const FitnessRecord*
        AssessFitness(
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

        /// Constructs an NullFitnessAssessor for use by \a algorithm.
        /**
         * \param algorithm The GA for which this fitness assessor is being
         *                  constructed.
         */
        NullFitnessAssessor(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an NullFitnessAssessor.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        NullFitnessAssessor(
            const NullFitnessAssessor& copy
            );

        /// Copy constructs an NullFitnessAssessor for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this fitness assessor is being
         *                  constructed.
         */
        NullFitnessAssessor(
            const NullFitnessAssessor& copy,
            GeneticAlgorithm& algorithm
            );



}; // class NullFitnessAssessor


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
#include "./inline/NullFitnessAssessor.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_NULLFITNESSASSESSOR_HPP

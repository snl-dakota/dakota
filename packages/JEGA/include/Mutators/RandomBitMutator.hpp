/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class RandomBitMutator.

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

        Thu Jun 12 10:12:12 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the RandomBitMutator class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_RANDOMBITMUTATION_HPP
#define JEGA_ALGORITHMS_RANDOMBITMUTATION_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmMutator.hpp>







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
class RandomBitMutator;







/*
================================================================================
Class Definition
================================================================================
*/


/// Mutates by flipping bits of a binary representation.
/**
 * This mutator introduces random variation by first converting a randomly
 * chosen variable of a randomly chosen Design into a binary string.  It then
 * flips a randomly chosen bit in the string from a 1 to a 0 or visa versa.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class RandomBitMutator :
    public GeneticAlgorithmMutator
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
         * \return The string "bit_random".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This mutator performs mutation by randomly selecting a
            design variable and converting it to a binary
            representation, then randomly choosing a bit from that
            string and toggling it.

            The binary representation is
            created using a BitManipulator.  The number of mutations is
            the rate times the size of the group passed in rounded
            to the nearest whole number.
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
         * \param algorithm The GA for which the new mutator is to be used.
         * \return A new, default instance of a RandomBitMutator.
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

        /// Performs random bit mutation.
        /**
         * This mutator only mutates members of the population but places the
         * resulting mutated designs into the children.
         *
         * \param pop The population of designs to which to apply mutation.
         * \param cldrn The group of child designs to which to apply mutation.
         */
        virtual
        void
        Mutate(
            JEGA::Utilities::DesignGroup& pop,
            JEGA::Utilities::DesignGroup& cldrn
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


        /// Constructs an RandomBitMutator for use by \a algorithm.
        /**
         * \param algorithm The GA for which this mutator is
         *                  being constructed.
         */
        RandomBitMutator(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an RandomBitMutator.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        RandomBitMutator(
            const RandomBitMutator& copy
            );

        /**
         * \brief Copy constructs an RandomBitMutator for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this mutator is
         *                  being constructed.
         */
        RandomBitMutator(
            const RandomBitMutator& copy,
            GeneticAlgorithm& algorithm
            );

}; // class RandomBitMutator


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
#include "./inline/RandomBitMutator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_RANDOMBITMUTATION_HPP

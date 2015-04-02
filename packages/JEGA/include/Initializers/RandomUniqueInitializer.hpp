/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class RandomUniqueInitializer.

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

        Wed Jun 04 08:04:34 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the RandomUniqueInitializer class.
 */





/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_RANDOMUNIQUEINITIALIZER_HPP
#define JEGA_ALGORITHMS_RANDOMUNIQUEINITIALIZER_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmInitializer.hpp>









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
class RandomUniqueInitializer;







/*
================================================================================
Class Definition
================================================================================
*/

/// Performs random unique initialization.
/**
 * This initializer creates Designs with random unique variable sets.  This is
 * done by checking each new design configuration as it is created against
 * all previously generated design configurations.
 *
 * This initializer will finish prematurely if a certain number of consecutive
 * failures occur.  This prevents infinite looping in the case where the
 * requested number of initializations cannot be supplied (combinatorial
 * shortage).
 *
 * This operator requires only the configuration inputs of the base class.
 */
class RandomUniqueInitializer :
    public GeneticAlgorithmInitializer
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
         * \return The string "unique_random".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This initializer creates a group of unique initial Designs.
            It does so by testing each newly created design against those
            that already exist.  Any that are not unique to all others are
            discarded.

            Variable values are chosen to be in bounds using a uniform
            random number distribution.

            The supplied initial size is the number of designs that
            this operator will try to create.  It may be the case that
            there are not enough possibilities (combinatorially speaking)
            for this operator to complete its task.  It does not explicitly
            check for that case.  Instead, it stops trying after it has
            failed for max(100, "initial size") consecutive creations.
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
         * \param algorithm The GA for which the new initializer is to be used.
         * \return A new, default instance of a RandomUniqueInitializer.
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

        /// Performs the random unique initialization.
        /**
         * \param into The group into which to place the newly created designs.
         */
        virtual
        void
        Initialize(
            JEGA::Utilities::DesignGroup& into
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

        virtual
        bool
        CanProduceInvalidVariableValues(
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

        /// Constructs an RandomUniqueInitializer for use by \a algorithm.
        /**
         * \param algorithm The GA for which this initializer is
         *                  being constructed.
         */
        RandomUniqueInitializer(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an RandomUniqueInitializer.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        RandomUniqueInitializer(
            const RandomUniqueInitializer& copy
            );

        /// Copy constructs an RandomUniqueInitializer for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this initializer is
         *                  being constructed.
         */
        RandomUniqueInitializer(
            const RandomUniqueInitializer& copy,
            GeneticAlgorithm& algorithm
            );

}; // class RandomUniqueInitializer


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
#include "./inline/RandomUniqueInitializer.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_RANDOMUNIQUEINITIALIZER_HPP

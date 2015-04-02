/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class RandomInitializer.

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

        Wed Jun 04 08:04:48 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the RandomInitializer class.
 */


/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_RANDOMINITIALIZER_HPP
#define JEGA_ALGORITHMS_RANDOMINITIALIZER_HPP







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
class RandomInitializer;







/*
================================================================================
Class Definition
================================================================================
*/

/// Performs initialization randomly.
/**
 * This initializer creates Designs with random variable values.  It gives
 * no consideration to any of the previously generated Designs.  It is purely
 * random.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class RandomInitializer :
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
         * \return The string "random".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This initializer creates the required number of initial
            Designs randomly without consideration for uniqueness.
            Variable values are chosen to be in bounds using a uniform
            random number distribution.

            The supplied initial size is the number of designs that
            will be created.
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
         * \return A new, default instance of a RandomInitializer.
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

        /// Performs random initialization.
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

        /// Constructs an RandomInitializer for use by \a algorithm.
        /**
         * \param algorithm The GA for which this initializer is
         *                  being constructed.
         */
        RandomInitializer(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an RandomInitializer.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        RandomInitializer(
            const RandomInitializer& copy
            );

        /// Copy constructs an RandomInitializer for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this initializer is
         *                  being constructed.
         */
        RandomInitializer(
            const RandomInitializer& copy,
            GeneticAlgorithm& algorithm
            );

}; // class RandomInitializer


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
#include "./inline/RandomInitializer.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_RANDOMINITIALIZER_HPP

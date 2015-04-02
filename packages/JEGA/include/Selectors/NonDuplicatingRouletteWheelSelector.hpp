/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class NonDuplicatingRouletteWheelSelector.

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

        Wed Jun 25 11:33:48 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the NonDuplicatingRouletteWheelSelector
 *        class.
 */





/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_NONDUPLICATINGROULETTEWHEELSELECTOR_HPP
#define JEGA_ALGORITHMS_NONDUPLICATINGROULETTEWHEELSELECTOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Selectors/RouletteWheelSelector.hpp>









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
class NonDuplicatingRouletteWheelSelector;







/*
================================================================================
Class Definition
================================================================================
*/

/**
 * \brief This selector implements the common roulette wheel selection without
 *        duplication.
 *
 * This is the same as regular roulette wheel selection with the exception that
 * a Design may only be selected once.  See the documentation for the
 * RouletteWheelSelector for a brief description of roulette wheel selection.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class NonDuplicatingRouletteWheelSelector :
    public RouletteWheelSelector
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
         * \return The string "unique_roulette_wheel".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This selector implements the common roulette wheel selection
            without duplication.  This is the same as regular roulette wheel
            selection with the exception that a Design may only be selected
            once.  See the documentation for the RouletteWheelSelector for a
            brief description of roulette wheel selection.
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
         * \param algorithm The GA for which the new selector is to be used.
         * \return A new, default instance of a
         *         NonDuplicatingRouletteWheelSelector.
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


        /// Performs non duplicating roulette wheel selection.
        /**
         * \param from The groups of designs from which to choose the next
         *             generation's population.
         * \param into The group into which to place those designs that have
         *             been selected from the groups of \a from.
         * \param count The desired minimum number of selections.  This can be
         *              ignored if desired.
         * \param fitnesses The record of the fitnesses of the designs in
         *                  \a from.  All designs in from should have an
         *                  associated fitness value in \a fitnesses.
         */
        virtual
        void
        Select(
            JEGA::Utilities::DesignGroupVector& from,
            JEGA::Utilities::DesignGroup& into,
            std::size_t count,
            const FitnessRecord& fitnesses
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
        CanSelectSameDesignMoreThanOnce(
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
         * \brief Constructs an NonDuplicatingRouletteWheelSelector for use by
         *        \a algorithm.
         *
         * \param algorithm The GA for which this selector is being
         *                  constructed.
         */
        NonDuplicatingRouletteWheelSelector(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an NonDuplicatingRouletteWheelSelector.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        NonDuplicatingRouletteWheelSelector(
            const NonDuplicatingRouletteWheelSelector& copy
            );

        /**
         * \brief Copy constructs an NonDuplicatingRouletteWheelSelector for
         *        use by \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this selector is being
         *                  constructed.
         */
        NonDuplicatingRouletteWheelSelector(
            const NonDuplicatingRouletteWheelSelector& copy,
            GeneticAlgorithm& algorithm
            );

}; // class NonDuplicatingRouletteWheelSelector


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
#include "./inline/NonDuplicatingRouletteWheelSelector.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_NONDUPLICATINGROULETTEWHEELSELECTOR_HPP

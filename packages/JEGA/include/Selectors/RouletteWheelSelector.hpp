/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class RouletteWheelSelector.

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

        Mon Jun 23 16:10:28 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the RouletteWheelSelector class.
 */





/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_ROULETTEWHEELSELECTOR_HPP
#define JEGA_ALGORITHMS_ROULETTEWHEELSELECTOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <map>
#include <GeneticAlgorithmSelector.hpp>

#include <utilities/include/int_types.hpp>









/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace Utilities
    {
        class Design;
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
class RouletteWheelSelector;









/*
================================================================================
In Namespace File Scope Typedefs
================================================================================
*/








/*
================================================================================
Class Definition
================================================================================
*/

/// This selector implements the common roulette wheel selection scheme.
/**
 * Conceptually, each Design is allotted a portion of a wheel proportional
 * to its fitness relative to the fitnesses of the other Designs.  Then
 * portions of the wheel are chosen at random and the Design occupying those
 * portions are duplicated into the next population.  Those Designs allotted
 * larger portions of the wheel are more likely to be selected (potentially
 * many times).
 *
 * This operator requires only the configuration inputs of the base class.
 */
class RouletteWheelSelector :
    public GeneticAlgorithmSelector
{
    /*
    ============================================================================
    Typedefs
    ============================================================================
    */
    public:

    protected:

        /// This is a way to map a cumulative probability to a Design.
        typedef
        std::map<double, JEGA::Utilities::Design*>
        CumulativeProbabilityMap;

    private:


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

    protected:

        /// Used as a flag for selected Designs
        /**
         * This variable is assigned a value corresponding to one of the
         * unused Design class attribute bits.  It is used to indicate
         * which Designs have been selected and which have not.
         */
        static const std::size_t SEL_ATT;



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
         * \return The string "roulette_wheel".
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
            scheme.  Conceptually, each Design is allotted a portion of a
            wheel proportional to its fitness relative to the fitnesses of the
            other Designs.  Then portions of the wheel are chosen at random and
            the Design occupying those portions are duplicated into the next
            population.  Those Designs allotted larger portions of the wheel
            are more likely to be selected potentially many times.
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
         * \return A new, default instance of a RouletteWheelSelector.
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

        /**
         * \brief Returns the \a desnum'th Design in order of DV going
         *        group-at-a-time.
         *
         * For example, if \a from had 2 groups each with 100 designs, the
         * call SelectDesign(from, 150) would return the 50'th design in
         * the second group according to the design variable sort order.
         *
         * \param from The vector of groups from which to select the
         *             \a desnum'th design.
         * \param desnum The number or index of the desired design in the
         *               groups if they were considered to be one large group.
         */
        JEGA::Utilities::Design*
        SelectDesign(
            const JEGA::Utilities::DesignGroupVector& from,
            std::size_t desnum
            );

        /**
         * \brief Fills \a into with the cumulative probabilities of the
         *        Designs in \a from.
         *
         * \param from The group of designs for which to compute the
         *             cumulative probabilities.
         * \param ftns The fitnesses of the designs in \a from.
         * \param into The map of cumulative probabilites into which to place
         *             the computed values for the designs in \a from.
         */
        void
        GenerateCumulativeProbabilities(
            const JEGA::Utilities::DesignGroupVector& from,
            const FitnessRecord& ftns,
            CumulativeProbabilityMap& into,
            bool skipSelected = true
            ) const;

        /// Unsets the \a SEL_ATT for each Design in \a from.
        /**
         * It is fine if some of the designs in \a from do not have the
         * selected attribute set.  They will be unchanged.
         *
         * \param from The designs from which to clear the selected attribute.
         */
        void
        ClearSelectionAttributes(
            const JEGA::Utilities::DesignGroupVector& from
            ) const;

        /**
         * \brief Moves all Designs with \a SEL_ATT set from \a from into
         *        \a into.
         *
         * \param from The groups from which to remove selected designs for
         *             placement into \a into.
         * \param into The group into which to place those designs with the
         *             selection attribute.
         */
        void
        RecordSelectedDesigns(
            const JEGA::Utilities::DesignGroupVector& from,
            JEGA::Utilities::DesignGroup& into
            ) const;


    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:


        /// Performs Roulette wheel selection.
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

        /// Constructs an RouletteWheelSelector for use by \a algorithm.
        /**
         * \param algorithm The GA for which this selector is being
         *                  constructed.
         */
        RouletteWheelSelector(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an RouletteWheelSelector.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        RouletteWheelSelector(
            const RouletteWheelSelector& copy
            );

        /**
         * \brief Copy constructs an RouletteWheelSelector for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this selector is being
         *                  constructed.
         */
        RouletteWheelSelector(
            const RouletteWheelSelector& copy,
            GeneticAlgorithm& algorithm
            );

}; // class RouletteWheelSelector


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
#include "./inline/RouletteWheelSelector.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_ROULETTEWHEELSELECTOR_HPP

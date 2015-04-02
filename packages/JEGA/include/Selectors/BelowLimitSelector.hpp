/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class BelowLimitSelector.

    NOTES:

        See notes under section "Class Definition" of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Wed Jan 11 07:12:50 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the BelowLimitSelector class.
 */





/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_BELOWLIMITSELECTOR_HPP
#define JEGA_ALGORITHMS_BELOWLIMITSELECTOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmSelector.hpp>









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
class BelowLimitSelector;







/*
================================================================================
Class Definition
================================================================================
*/

/**
 * \brief This selector discriminates based on fitnesses compared to a max
 *        allowable value.
 *
 * If the NEGATED fitness value associated with a design is less than a
 * certain cutoff value, then it is kept.  Otherwise, it may be discarded
 * subject to a minimum required number of selections.
 *
 * To enforce this the minimum number of selections, this selector first makes
 * all the selections it would make anyway and if that is not enough, it
 * effectively relaxes the limit and makes selections from what remains
 * unselected.  In actual implementation, it simply takes the best of what
 * remains until it has made enough selections.
 *
 * Selections are made from the vector of groups passed into the Select
 * method.  The selected designs are removed from their group and placed
 * in the "into" group.
 *
 * The limiting value and shrinkage percentage are extracted from the parameter
 * database using the names "method.jega.fitness_limit" and
 * "method.jega.shrinkage_percentage" respectively.  They are both extracted as
 * double parameters.  If they are not supplied in the parameter database, the
 * default values as defined by DEFAULT_LIMIT and DEFAULT_SHRINK_PCT
 * respectively will be used.  These are required in addition to any
 * requirements of the base class.
 */
class BelowLimitSelector :
    public GeneticAlgorithmSelector
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    public:

        /// The default value for the shrinkage percentage.
        static const double DEFAULT_SHRINK_PCT;

        /// The default limiting value.
        static const double DEFAULT_LIMIT;

        /// The default absolute minimum number of selections.
        static const std::size_t DEFAULT_MIN_SELS;

    private:

        /// The percentage of the requested selections required.
        /**
         * This value determines the minimum number of selections that
         * will take place if enough Designs are available.  It is
         * interpreted as a percentage of the \a count value passed into
         * the Select method.
         *
         * So if \a count comes in as 100 and \a _shrinkage is 0.9, the
         * minimum of the number of designs passed in (which should always
         * be greater than \a count) and 90 (0.9*100) will be the minimum
         * number of selections that take place.  The number of selections
         * could be greater.  This is to prevent the population from
         * bottlenecking.
         */
        double _shrinkage;

        /// The cutoff fitness value below which designs survive.
        /**
         * Designs whose NEGATED fitnesses are below this value are kept.
         * The rest are discarded subject to the shrinkage percentage.
         */
        double _limit;

        /// The absolute minimum number of designs that must survive.
        /**
         * This typically serves as a minimum allowable population size and is
         * respected if at all possible.  It may not be possible if there simply
         * are not this many to choose from.  This operator cannot generate new
         * designs.
         */
        std::size_t _minSels;

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the shrinkage percentage to \a shrinkage.
        /**
         * This method warns if shrinkage is greater than 1 (greater than 100%
         * requested selections at each generation and disallows values less
         * than 0.  It outputs an indication of the new value.
         *
         * \param shrinkage The new value for the shrinkage percentage.
         */
        void
        SetShrinkagePercentage(
            double shrinkage
            );

        /// Sets the limit value to \a limit.
        /**
         * This is the value below which designs are selected and above which
         * they are not.
         *
         * \param limit The new limiting value of this below limit selector.
         */
        void
        SetLimit(
            double limit
            );

        /// Sets the minimum selections value to \a minSels.
        /**
         * This is the minimum number of selections that must occur.  This takes
         * precedence over the shrinkage percentage.
         *
         * This typically serves as a minimum allowable population size and is
         * respected if at all possible.  It may not be possible if there simply
         * are not this many to choose from.  This operator cannot generate new
         * designs.
         *
         * \param minSels The new number of selections that must take place.
         */
        void
        SetMinimumSelections(
            std::size_t minSels
            );

    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Gets the current shrinkage percentage value.
        /**
         * \return The current shrinkage percentage value of this below limit
         *         selector.
         */
        inline
        double
        GetShrinkagePercentage(
            ) const;

        /// Gets the current limit value.
        /**
         * \return The current limiting value of this below limit selector.
         */
        inline
        double
        GetLimit(
            ) const;

        /// Gets the current minimum allowable number of selections.
        /**
         * \return The current minimum number of selections of this below limit
         *         selector.
         */
        inline
        std::size_t
        GetMinimumSelections(
            ) const;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the proper name of this operator.
        /**
         * \return The string "below_limit".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This selection operator chooses only those designs that
            have a fitness value below a certain limit (
            subject to a minimum number of selections).
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
         * \return A new, default instance of a BelowLimitSelector.
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

        /// Performs below limit selection.
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

        /// Determines if all the passed in groups are empty.
        /**
         * This simply iterates over the groups and checks their sizes.
         *
         * \param groups The groups to check to see if they are all empty.
         * \return True if all groups in \a groups are empty and false
         *         otherwise.
         */
        bool
        AreAllGroupsEmtpy(
            const JEGA::Utilities::DesignGroupVector& groups
            ) const;

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:


        /// Constructs an BelowLimitSelector for use by \a algorithm.
        /**
         * \param algorithm The GA for which this selector is
         *                  being constructed.
         */
        BelowLimitSelector(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an BelowLimitSelector.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        BelowLimitSelector(
            const BelowLimitSelector& copy
            );

        /**
         * \brief Copy constructs an BelowLimitSelector for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this selector is
         *                  being constructed.
         */
        BelowLimitSelector(
            const BelowLimitSelector& copy,
            GeneticAlgorithm& algorithm
            );

}; // class BelowLimitSelector


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
#include "./inline/BelowLimitSelector.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_BELOWLIMITSELECTOR_HPP

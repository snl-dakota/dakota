/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class MaxDesignsNichePressureApplicator.

    NOTES:

        See notes under "Document this File" section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.7.0

    CHANGES:

        Wed Dec 21 16:25:44 2011 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MaxDesignsNichePressureApplicator
 *        class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_MAXDESIGNSNICHEPRESSUREAPPLICATOR_HPP
#define JEGA_ALGORITHMS_MAXDESIGNSNICHEPRESSUREAPPLICATOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/JEGATypes.hpp>
#include <../Utilities/include/DesignValueMap.hpp>
#include <GeneticAlgorithmNichePressureApplicator.hpp>








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
class MaxDesignsNichePressureApplicator;







/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/








/*
================================================================================
Class Definition
================================================================================
*/
/**
 * \brief
 *
 *
 */
class MaxDesignsNichePressureApplicator :
    public GeneticAlgorithmNichePressureApplicator
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:


    protected:


    private:

        typedef
        JEGA::Utilities::DesignValueMap<std::size_t>
        NicheCountMap;

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    public:

        static const std::size_t DEFAULT_MAX_DESIGNS;

        /// The default value used for all distance percentages.
        static const double DEFAULT_DIST_PCT;

    private:

        /// The percentages of the ranges of objectives to use as the radii.
        JEGA::DoubleVector _distPcts;

        /// The percentages of the ranges of objectives to use as the radii.
        std::size_t _maxDesigns;



    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the current list of percentages to \a pcts.
        /**
         * This method will log warnings if there are not the correct number of
         * values and will log the value of each and to which objective it will
         * be applied.
         *
         * \param pcts The new vector of percentages to be applied to the range
         *             of the objectives in the Pareto frontier when computing
         *             required distances.
         */
        void
        SetDistancePercentages(
            const JEGA::DoubleVector& pcts
            );

        /// Sets all percentages to the same value.
        /**
         * This will set all percentages for all objective dimensions to the
         * same supplied value.
         *
         * An informative log entry will be issued at the verbose level for
         * each dimension.
         *
         * \param pct The new percentage for all dimensions.
         */
        void
        SetDistancePercentages(
            double pct
            );

        /// Sets the percentage used for the \a of'th objective.
        /**
         * This method will log a verbose level message informing of the new
         * value.  It also enforces a minimum value of 0 and a maximum of 1
         * along with other sanity checks.
         *
         * \param of The index of the objective for which the distance is being
         *           supplied.
         * \param pct The new percent value for objective #\a of.
         */
        void
        SetDistancePercentage(
            std::size_t of,
            double pct
            );

        void
        SetMaximumDesigns(
            std::size_t maxDesigns
            );

    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /**
         * \brief Allows access to the current set of percentages in order
         *        of the objectives.
         *
         * \return The vector of percentages associated with the objectives.
         */
        inline
        const JEGA::DoubleVector&
        GetDistancePercentages(
            ) const;

        /**
         * \brief Returns the current percentage for the supplied objective.
         *
         * \param of The objective for which the current percentage is of
         *           interest.
         * \return The percentage currently being used for the supplied
         *         objective.
         */
        inline
        double
        GetDistancePercentage(
            std::size_t of
            ) const;


        inline
        const std::size_t&
        GetMaximumDesigns(
            ) const;

    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the proper name of this operator.
        /**
         * \return The string "max_designs".
         */
        static
        const std::string&
        Name(
            );

        static
        const std::string&
        Description(
            );

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
         * \brief Determines the distances that must exist between designs for
         *        each performance dimension.
         *
         * The distance for each dimension is the range for that dimension
         * multiplied by the percentage for that dimension.  The range is
         * provided by the \a paretoExtremes.
         *
         * \param paretoExtremes The extremes of the current Pareto frontier.
         * \return The distances computed using the know percentages and the
         *         ranges for the objectives as determined from the supplied
         *         Pareto extremes.
         */
        JEGA::DoubleVector
        ComputeCutoffDistances(
            const eddy::utilities::DoubleExtremes& paretoExtremes
            ) const;

        /**
         * \brief Returns the distance between the two supplied designs along
         *        the supplied objective distance.
         *
         * This value can be compared to the minimum distance as computed using
         * the percent change and Pareto extremes to determine if the two
         * designs are too close, far enough apart, etc.
         *
         * \param des1 The first design for which to compute the distance.
         * \param des2 The second design for which to compute the distance.
         * \param of The index of the objective along which the distance is
         *           sought.
         * \return The distance between \a des1 and \a des2 for objective
         *         \a of.
         */
        static
        double
        ComputeObjectiveDistance(
            const JEGA::Utilities::Design& des1,
            const JEGA::Utilities::Design& des2,
            std::size_t of
            );

    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /**
         * \brief Called prior to the selection operators to prepare for the
         *        selection operations.
         *
         * The selection operators include fitness assessment, selection, and
         * niche pressure,  This method is called prior to them all.  This
         * implementation of it re-assimilates any bufferred designs back into
         * the population.
         *
         * \param population The current population prior to selection.  It is
         *                   into this group that bufferred designs will be
         *                   placed.
         */
        virtual
        void
        PreSelection(
            JEGA::Utilities::DesignGroup& population
            );

        /**
         * \brief Overriden to carry out the specific niche pressure algorithm.
         *
         * Applies niche pressure to the supplied group according to the
         * assigned fitnesses as can be found in the supplied fitness record.
         *
         * \param population The group (presumably the population of the GA)
         *                   to which to apply niche pressure.
         * \param fitnesses A record of the fitness values assigned to each of
         *                  the members of the supplied group.
         */
        virtual
        void
        ApplyNichePressure(
            JEGA::Utilities::DesignGroup& population,
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
         * This method is used to extract needed information for this operator.
         * It does so using the "Get...FromDB" class of methods from the
         * GeneticAlgorithmOperator base class.  The return value is indicative
         * of the success of the method.
         *
         * This implementation retrieves the distance percentages for each
         * objective.
         *
         * \param db The database of parameter values from which to do
         *           retrieval.
         * \return true if polling completed successfully and false otherwise.
         */
        virtual
        bool
        PollForParameters(
            const JEGA::Utilities::ParameterDatabase& db
            );

    protected:


    private:


    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:


        NicheCountMap
        ComputeNicheCounts(
            const JEGA::Utilities::DesignOFSortSet& designs,
            const eddy::utilities::DoubleExtremes& paretoExtremes
            ) const;




    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /**
         * \brief Constructs an MaxDesignsNichePressureApplicator for use by
         *        \a algorithm.
         *
         * \param algorithm The GA for which this niche pressure applicator is
         *                  being constructed.
         */
        MaxDesignsNichePressureApplicator(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an MaxDesignsNichePressureApplicator.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        MaxDesignsNichePressureApplicator(
            const MaxDesignsNichePressureApplicator& copy
            );

        /**
         * \brief Copy constructs an MaxDesignsNichePressureApplicator for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this niche pressure  applicator is
         *                  being constructed.
         */
        MaxDesignsNichePressureApplicator(
            const MaxDesignsNichePressureApplicator& copy,
            GeneticAlgorithm& algorithm
            );

}; // class MaxDesignsNichePressureApplicator



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
#include "inline/MaxDesignsNichePressureApplicator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_MAXDESIGNSNICHEPRESSUREAPPLICATOR_HPP

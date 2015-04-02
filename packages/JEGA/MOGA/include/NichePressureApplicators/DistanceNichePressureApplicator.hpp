/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class DistanceNichePressureApplicator.

    NOTES:

        See notes under Class Definition section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Fri Apr 28 10:45:56 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the DistanceNichePressureApplicator class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_DISTANCENICHEPRESSUREAPPLICATOR_HPP
#define JEGA_ALGORITHMS_DISTANCENICHEPRESSUREAPPLICATOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/JEGATypes.hpp>
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
class DistanceNichePressureApplicator;







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
 * \brief Applies niche pressure by removing designs within a minimum distance
 *        along all of the dimensions individually.
 *
 * Removal proceeds from the beginning of the collection of designs to the
 * end and the only variation from the removal rule is that this will not
 * remove an extreme design.  See the base class IsExtremeDesign method for
 * a definition of an extremem Design.
 *
 * The distances are calculated using percentages of the ranges of the
 * objectives.  If a supplied percentage is 0.1, then the required minimum
 * distance is simply that percentage of the range or:
 * \verbatim
     range(i)*0.1 where i is the number of the current objective.
   \endverbatim
 * In order to survive the process, a Design must be further than the amount
 * calculated for at least 1 objective dimension.
 *
 * The distance percentages extracted from the parameter database using the
 * name "method.jega.niche_vector".  These values are extracted as a double
 * vector. If they are not supplied in the parameter database, the default
 * value as defined by DEFAULT_DIST_PCT will be used for all of them.  These
 * are required in addition to any requirements of the base class.
 */
class DistanceNichePressureApplicator :
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


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    public:

        /// The default value used for all distance percentages.
        static const double DEFAULT_DIST_PCT;

    private:

        /// The percentages of the ranges of objectives to use as the radii.
        JEGA::DoubleVector _distPcts;



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
         * \return The string "distance".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This niche pressure applicator enforces a minimum distance between
            any two points in the performance space along each dimension
            individually.  Any points that is too close to another along all
            dimensions is removed.  This begins with the first design in the
            objective list and proceeds in order removing designs.  One
            exception to the distance rules is that any extreme designs are
            kept regardless where an extreme design is one that has an extreme
            value for at least N-1 objectives where N is the number of
            objectives.
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
         * \param algorithm The GA for which the new niche pressure applicator
         *        is to be used.
         * \return A new, default instance of a
         *         DistanceNichePressureApplicator.
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

        /**
         * \brief Computes and returns the theoretical limit on the number of
         *        points that can survive this niching operation.
         *
         * This does not take into account the variable types.  Just the
         * parameters to this operator.  So for example, it may be the case
         * that because some variables are discrete, that it is not possible
         * to obtain this theoretical limit.
         *
         * \return The theoretical maximum number of points that can survive
         *         application of this niche pressure applicator on this
         *         problem.
         */
        std::size_t
        ComputePointCountBound(
            ) const;


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





    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /**
         * \brief Constructs an DistanceNichePressureApplicator for use by
         *        \a algorithm.
         *
         * \param algorithm The GA for which this niche pressure applicator is
         *                  being constructed.
         */
        DistanceNichePressureApplicator(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an DistanceNichePressureApplicator.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        DistanceNichePressureApplicator(
            const DistanceNichePressureApplicator& copy
            );

        /**
         * \brief Copy constructs an DistanceNichePressureApplicator for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this niche pressure  applicator is
         *                  being constructed.
         */
        DistanceNichePressureApplicator(
            const DistanceNichePressureApplicator& copy,
            GeneticAlgorithm& algorithm
            );

}; // class DistanceNichePressureApplicator



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
#include "inline/DistanceNichePressureApplicator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_DISTANCENICHEPRESSUREAPPLICATOR_HPP

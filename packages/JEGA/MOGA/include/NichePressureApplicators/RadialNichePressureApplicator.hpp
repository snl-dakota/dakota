/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class RadialNichePressureApplicator.

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

        Wed Jan 18 10:45:56 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the RadialNichePressureApplicator class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_RADIALNICHEPRESSUREAPPLICATOR_HPP
#define JEGA_ALGORITHMS_RADIALNICHEPRESSUREAPPLICATOR_HPP







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
class RadialNichePressureApplicator;







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
 * \brief Applies niche pressure by removing designs within a minimum radius
 *        of one another.
 *
 * Conceptually, this applicator creates a circle of a particular radius around
 * each design and removes any other designs that exist within that sphere.
 *
 * Removal proceeds from the beginning of the collection of designs to the
 * end and the only variation from the removal rule is that this will not
 * remove an extreme design.  See the base class IsExtremeDesign method for
 * a definition of an extremem Design.
 *
 * The radius is calculated using percentages of the ranges of the objectives.
 * If all supplied percentages are 0.1, then the required minimum distance is
 * the Euclidean distance:
 * \verbatim
     sqrt(sum-over-i[(range(i)*0.1)^2]) for all i = 1, nof
   \endverbatim
 *
 * The percentages used to compute the radius are extracted from the parameter
 * database using the name "method.jega.niche_vector".  These values are
 * extracted as a double vector. If they are not supplied in the parameter
 * database, the default value as defined by DEFAULT_RAD_PCT will be used for
 * all of them.  These are required in addition to any requirements of the base
 * class.
 */
class RadialNichePressureApplicator :
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

        /// The default value used for all radius percentages.
        static const double DEFAULT_RAD_PCT;

    private:

        /// The percentages of the ranges of objectives to use as the radii.
        JEGA::DoubleVector _radPcts;




    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the radius percentages to the values in \a pcts.
        /**
         * This method issues various messages, errors, and warnings when
         * appropriate.  For example, if \a pcts does not have the proper
         * number of elements, if any of them have unusable values, etc.
         *
         * \param pcts The new percentages for this radial nicher.
         */
        void
        SetRadiusPercentages(
            const JEGA::DoubleVector& pcts
            );

        /// Sets all percentages to the same value.
        /**
         * This method issues various messages, errors, and warnings when
         * appropriate.  For example, \a pct is unusable, etc.
         *
         * \param pct The percentage value to use for all objective dimensions.
         */
        void
        SetRadiusPercentages(
            double pct
            );

        /// Sets the percentage used for the \a of'th objective.
        /**
         * This method issues various messages, errors, and warnings when
         * appropriate.  For example, \a pct is unusable, \a of is not a valid
         * objective index, etc.
         *
         * \param of The index of the objective for which to set the
         *           percentage.
         * \param pct The new percentage value for the supplied objective.
         */
        void
        SetRadiusPercentage(
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
         * \brief Allows access to the vector of percentages for each objective
         *        used to compute the radius for this nicher.
         *
         * \return The vector of percentages for the objectives in order of
         *         the objectives.
         */
        inline
        const JEGA::DoubleVector&
        GetRadiusPercentages(
            ) const;

        /// Allows access to the \a of'th objective radius percentage.
        /**
         * \param of The index of the objective for which the current radius
         *           percentage is sought.
         */
        inline
        double
        GetRadiusPercentage(
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
         * \return The string "radial".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This operator provides niche pressure by enforcing a minimum
            distance between non-dominated designs in the performance space.
            It requires a percentage value for each objective, each of which is
            interpreted as a percentage of the range observed for that
            objective within the subset of non-dominated designs.  The
            resulting distances are merged using a square-root-sum-of-squares
            into the radius about each design within which another design may
            not exist.
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
         * \return A new, default instance of a RadialNichePressureApplicator.
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
         * \brief Computes and returns the square-root-sum-of-squares of the
         *        distances for each objective as computed using the radius
         *        percentages.
         *
         * The ranges for the objectives by which the distances are normalized
         * are retrieved from the supplied Pareto extremes.
         *
         * \param paretoExtremes The extremes of the Pareto frontier to which
         *                       this nicher is being applied.
         * \return The radius in the objective space below which two designs
         *         are considered to be too close together.
         */
        double
        ComputeCutoffDistance(
            const eddy::utilities::DoubleExtremes& paretoExtremes
            ) const;

        /// Computes the total normalized distance between the two designs.
        /**
         * This is the N-dimensional Euclidean distance between the two designs
         * in the performance space each component of which is normalized by
         * the range for that objective.
         *
         * \param des1 The first design in the distance calculation.
         * \param des2 The second design in the distance calculation.
         * \param paretoExtremes The extremes of the Pareto frontier to which
         *                       this nicher is being applied.
         */
        double
        GetNormalizedDistance(
            const JEGA::Utilities::Design& des1,
            const JEGA::Utilities::Design& des2,
            const eddy::utilities::DoubleExtremes& paretoExtremes
            ) const;

        /**
         * \brief Computes the distance between the two designs along the given
         *        objective dimension normalized by the supplied range.
         *
         * \param des1 The first design in the distance calculation.
         * \param des2 The second design in the distance calculation.
         * \param of The index of the objective of interest.
         * \param range The full range by which to normalized the distance
         *              between the two designs.
         * \return The distance between \a des1 and \a des2 along objective
         *         \a of normalized by the supplied \a range.
         */
        double
        GetNormalizedObjectiveDistance(
            const JEGA::Utilities::Design& des1,
            const JEGA::Utilities::Design& des2,
            std::size_t of,
            double range
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
         * GeneticAlgorithmOperator base class. The return value is indicative
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
         * \brief Constructs an RadialNichePressureApplicator for use by
         *        \a algorithm.
         *
         * \param algorithm The GA for which this niche pressure applicator is
         *                  being constructed.
         */
        RadialNichePressureApplicator(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an RadialNichePressureApplicator.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        RadialNichePressureApplicator(
            const RadialNichePressureApplicator& copy
            );

        /**
         * \brief Copy constructs an RadialNichePressureApplicator for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this niche pressure  applicator is
         *                  being constructed.
         */
        RadialNichePressureApplicator(
            const RadialNichePressureApplicator& copy,
            GeneticAlgorithm& algorithm
            );


}; // class RadialNichePressureApplicator



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
#include "inline/RadialNichePressureApplicator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_RADIALNICHEPRESSUREAPPLICATOR_HPP

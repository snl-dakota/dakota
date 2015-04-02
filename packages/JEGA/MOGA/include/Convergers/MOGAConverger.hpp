/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class MOGAConverger.

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

        Mon Jul 07 09:24:18 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MOGAConverger class.
 */





/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_MOGACONVERGER_HPP
#define JEGA_ALGORITHMS_MOGACONVERGER_HPP








/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/extremes.hpp>
#include <../Utilities/include/DesignMultiSet.hpp>
#include <Convergers/MetricTrackerConvergerBase.hpp>





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
class MOGAConverger;







/*
================================================================================
Class Definition
================================================================================
*/

/**
 * \brief Converges a MOGA based on observations of the evolution of the
 *        non-dominated frontier.
 *
 * This converger tracks three metrics in an attempt to determine when the
 * non-dominated frontier stops changing.  When each of the three metrics
 * have stopped changing by a minimum of the supplied percent change over
 * the supplied number of generations, the algorithm stops.
 *
 * The three metrics are as follows:
 * <OL>
 *      <LI>Maximum percentage change in range along any objective dimension.
 *      <LI>Percentage change in density of frontier.
 *      <LI>Percentage of previous population dominated by current population.
 * </OL>
 *
 * The expansion metric is computed by tracking the extremes of the
 * non-dominated frontier from one generation to the next.  Any movement of
 * the extreme values is noticed and the maximum percentage movement is
 * computed as:
 * \verbatim
    Em = max over j of abs((range(j,i) - range(j,i-1)) / range(j,i-1)) j=1,nof
   \endverbatim
 * where Em is the max expansion metric, j is the objective function index,
 * i is the current generation number, and nof is the total number of
 * objectives.  The range is the difference between the largest value along
 * an objective and the smallest when considering only non-dominated designs.
 *
 * The density metric is computed as the number of non-dominated points
 * divided by the hypervolume of the non-dominated region of space.  Therefore,
 * changes in the density can be caused by changes in the number of
 * non-dominated points or by changes in size of the non-dominated space or
 * both.  The size of the non-dominated space is computed as:
 * \verbatim
    Vps(i) = product over j of range(j,i)   j=1,nof
   \endverbatim
 * where Vps(i) is the hypervolume of the non-dominated space at generation i
 * and all other terms have the same meanings as above.
 *
 * The density of the a given non-dominated space is then:
 * \verbatim
    Dps(i) = Pct(i) / Vps(i)
   \endverbatim
 * where Pct(i) is the number of points on the non-dominated frontier at
 * generation i.
 *
 * The percentage increase in density of the frontier is then calculated as
 * \verbatim
    Cd = abs((Dps(i) - Dps(i-1)) / Dps(i-1))
   \endverbatim
 * where Cd is the change in density metric.
 *
 * The final metric is computed by considering each design in the previous
 * population and determining if it is dominated by any designs in the
 * current population.  All that are determined to be dominated are counted.
 * The metric is the ratio of the number that are dominated to the total number
 * that exist in the previous population.
 *
 * Each of these metrics is a percentage.  The tracker records the largest of
 * these three at each generation.  Once the recorded percentage is below the
 * supplied percent change for the supplied number of generations
 * consecutively, the algorithm is converged.
 *
 * This operator requires only the configuration inputs of the base class.
 */
class MOGAConverger :
    public MetricTrackerConvergerBase
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        JEGA_IF_MESSAGE_BOARD(
            static JEGA::Utilities::MessageInfo DENSITY_MSG_INFO;
            static JEGA::Utilities::MessageInfo EXPANSE_MSG_INFO;
            static JEGA::Utilities::MessageInfo DEPTH_MSG_INFO;
            )

    private:

        /// This is the previous set of Designs considered by this converger.
        /**
         * This amounts to the previous non-dominated subset of the population
         * of the GA this converger is being used by.  It is duplicated
         * completely.  Each Design is copy constructed.  This is so that this
         * operator does not corrupt the Designs in use by the algorithm.
         */
        JEGA::Utilities::DesignOFSortSet _prevParSet;

        /**
         * \brief This is the set of extremes associated with the previous
         *        set of non-dominated designs.
         *
         * These values are used among other things to track the expansion of
         * the Pareto frontier from generation to generation.
         */
        eddy::utilities::DoubleExtremes _prevParExtremes;

        eddy::utilities::DoubleExtremes _prevPopExtremes;

        std::size_t _prevPopSize;

        JEGA_IF_MESSAGE_BOARD(
            JEGA::Utilities::MessageInfo _densityMsgInfo;
            JEGA::Utilities::MessageInfo _expanseMsgInfo;
            JEGA::Utilities::MessageInfo _depthMsgInfo;
            )

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
         * \return The string "moga_converger".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            "This converger computes various metrics for the "
            "population and determines if sufficient improvement "
            "is being made.  If not, this converger returns true."
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
         * \param algorithm The GA for which the new converger is to be used.
         * \return A new, default instance of a MOGAConverger.
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
         * \brief Computes the maximum percentage change in range for any
         *        objective of the Designs in \a _prevParExtremes and
         *        \a newExtremes.
         *
         * The changes can be increases or decreases but the return will
         * always be a positive number representing the largest absolute
         * percentage change in range.
         *
         * \param newExtremes The new set of Extremes to compare to
         *                    \a _prevParExtremes.
         * \return The max absolute change in extremes between \a newExtremes
         *         and \a _prevParExtremes of any objective dimension.
         */
        double
        GetMaxRangeChange(
            const eddy::utilities::DoubleExtremes& newExtremes
            ) const;

        /**
         * \brief Computes the change in density between the supplied
         *        \a newSet and the existing \a _prevParSet.
         *
         * The return is an absolute value so the density may have increased
         * or decreased.
         *
         * \param newSet The new non-dominated set of points to compare to the
         *               stored \a _prevParSet.
         * \param newExtremes The extremes of the \a newSet supplied separately
         *                    because it is common to have them available prior
         *                    to this call.  If not, compute them with the
         *                    MultiObjectiveStatistician and send them in.
         * \return The change in density between \a newSet and \a _prevParSet as
         *         an absolute value.
         */
        double
        GetDensityChange(
            const JEGA::Utilities::DesignOFSortSet& newPop,
            const eddy::utilities::DoubleExtremes& newExtremes
            ) const;

        /**
         * \brief Returns the fraction of Designs in \a _prevParSet that are
         *        domianted by Designs in \a curr.
         *
         * This is the count of dominated divided by the total number which
         * always results in a non-negative number.
         *
         * \param curr The current set of non-dominated points to compare to
         *             the previous set.
         * \return The ratio of dominated to total number of points in \a curr.
         */
        double
        GetFractionDominated(
            const JEGA::Utilities::DesignOFSortSet& curr
            ) const;


        /**
         * \brief Flushes the previous set then refills it with with duplicates
         *        of the Designs in the supplied set.
         *
         * \param newSet The set of Designs to use in updating the \a _prevParSet.
         */
        void
        UpdatePreviousParetoSet(
            const JEGA::Utilities::DesignOFSortSet& newSet
            );

        /**
         * \brief Replaces the \a _prevParExtremes with \a newExtremes.
         *
         * \param newExtremes The new set of extremes associated with the
         *                    current or soon to be current \a _prevParSet.
         */
        inline
        void
        UpdateParetoExtremes(
            const eddy::utilities::DoubleExtremes& newExtremes
            );

        /**
         * \brief Replaces the \a _prevPopExtremes with \a newExtremes.
         *
         * \param newExtremes The new set of extremes associated with the
         *                    current or soon to be current \a _prevParSet.
         */
        inline
        void
        UpdatePopulationExtremes(
            const eddy::utilities::DoubleExtremes& newExtremes
            );

        /**
         * \brief Finds the Designs with the best fitness value in \a fitnesses
         *        and returns them in a set.
         *
         * There may be multiple Designs with equally good fitnesses.
         *
         * \param of The set of Designs to find the best of.
         * \param fitnesses The record of fitness values that must contain
         *                  records for each of the Designs in \a of.
         * \return A collection of all the Designs that have the best fitness
         *         value of any in \a of according to \a fitnesses.
         */
        JEGA::Utilities::DesignOFSortSet
        GetBest(
            const JEGA::Utilities::DesignOFSortSet& of,
            const FitnessRecord& fitnesses
            );

    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /// Tests for convergence of a MOGA
        /**
         * This method checks to see if the algorithm should stop according
         * to the algorithm described in the documentation of this class.
         *
         * \param group The latest group representing the MOGA's current state
         *              which should be the current population.
         * \param fitnesses The fitness values assigned to each of the members
         *                  of \a group by a fitness assessor.
         * \return True if convergence is attained and false otherwise.
         */
        virtual
        bool
        CheckConvergence(
            const JEGA::Utilities::DesignGroup& group,
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

        /**
         * \brief Determines the metric that is to be tracked by this metric
         *        tracker style converger.
         *
         * This implementation chooses the maximum of the 3 metrics described
         * in the class documentation as computed by the associated methods.
         *
         * \param group The group of Designs for which to compute metrics
         *              against the \a _prevParSet, \a _prevParExtremes, and
         *              \a _prevPopExtremes.
         * \param fitnesses The record of fitness values for each Design in
         *                  \a group.
         * \return The final singular metric value that should be tracked by
         *         this converger.
         */
        virtual
        double
        GetMetricValue(
            const JEGA::Utilities::DesignGroup& group,
            const FitnessRecord& fitnesses
            );


    private:





    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:

        /**
         * \brief Computes and returns the volume of the space bounded by the
         *        supplied \a extremes.
         *
         * \param extremes The extremes object that contains the maxs and mins
         *                 of multiple dimensions.
         * \return The product of all ranges contained within \a extremes.
         */
        static
        double
        ComputeVolume(
            const eddy::utilities::DoubleExtremes& extremes
            );



    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:


        /// Constructs a MOGAConverger for use by \a algorithm.
        /**
         * \param algorithm The GA for which this converger is being
         *                  constructed.
         */
        MOGAConverger(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a MOGAConverger.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        MOGAConverger(
            const MOGAConverger& copy
            );

        /// Copy constructs a MOGAConverger for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this converger is being
         *                  constructed.
         */
        MOGAConverger(
            const MOGAConverger& copy,
            GeneticAlgorithm& algorithm
            );

        /// Destructs a MOGAConverger.
        virtual
        ~MOGAConverger(
            );



}; // class MOGAConverger


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
#include "./inline/MOGAConverger.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_MOGACONVERGER_HPP

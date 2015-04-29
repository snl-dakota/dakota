/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class MetricTrackerConvergerBase.

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

        Tue Jul 22 15:43:27 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MetricTrackerConvergerBase class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_METRICTRACKERCONVERGERBASE_HPP
#define JEGA_ALGORITHMS_METRICTRACKERCONVERGERBASE_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <MetricTracker.hpp>
#include <../Utilities/include/MessageBoard.hpp>
#include <Convergers/MaxGenEvalTimeConverger.hpp>







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
class MetricTrackerConvergerBase;







/*
================================================================================
Class Definition
================================================================================
*/


/// This base class contains a single MetricTracker for tracking metrics.
/**
 * This converger allows subclasses to easily track the progress of
 * any metric such as average fitness, best fitness, whatever.
 *
 * The default CheckConvergence method implementation returns true if
 * the stored metric changes by less than _change over the supplied number
 * of generations.  The current value is compared to all stored values and
 * if the percent change between any of them is greater than the supplied
 * minimum change, then convergence does not occur.
 *
 * The number of generations over which to track and the percent change to seek
 * are extracted from the parameter database using the names
 * "method.jega.num_generations" and "method.jega.percent_change" respectively.
 * The number of generations is extracted as a size type value and the percent
 * change is extracted as a double. If they are not in the parameter database,
 * the default values as defined by DEFAULT_NUM_GENS and DEFAULT_CHNG
 * respectively will be used.  These are required in addition to any
 * requirements of the base class.
 */
class MetricTrackerConvergerBase :
    public MaxGenEvalTimeConverger
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    public:

        /// The default number of generations over which to track the metric.
        static const DoubleStack::size_type DEFAULT_NUM_GENS;

        /// The default percent change in the metric being sought.
        static const double DEFAULT_CHNG;

    private:

        JEGA_IF_MESSAGE_BOARD(
            static JEGA::Utilities::MessageInfo METRIC_MSG_INFO;
            static JEGA::Utilities::MessageInfo CURR_PCT_CHNG_INFO;
            )

    private:

        /// The stack for tracking a metric.
        MetricTracker _metricTracker;

        /// The percent change under which convergence has occurred.
        double _change;

        bool _absolute;

        JEGA_IF_MESSAGE_BOARD(
            JEGA::Utilities::MessageInfo _metricMsgInfo;
            JEGA::Utilities::MessageInfo _pctChngMsgInfo;
            )

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


        /// Sets the percent change below which convergence occurs.
        /**
         * This method logs a verbose level entry informing of the new
         * percent change.
         *
         * \param pctChange The new percent change of interest to this metric
         *                  tracker converger.
         */
        void
        SetPercentChange(
            double pctChange
            );

        /// Returns the number of generations over which the metric is tracked.
        /**
         * This method logs a verbose level entry informing of the new
         * number of generations.
         *
         * \param numGen The new number of generations over which to track the
         *               metric of interest to this metric tracker converger.
         */
        void
        SetNumGenerations(
            std::size_t numGen
            );


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Returns the percent change below which convergence occurs.
        /**
         * \return The current percent change of interest to this metric
         *         tracker converger.
         */
        inline
        double
        GetPercentChange(
            ) const;

        /// Returns the number of generations over which metric is tracked.
        /**
         * \return The current number of generations of interest to this metric
         *         tracker converger.
         */
        inline
        std::size_t
        GetNumGenerations(
            ) const;

    protected:

        /// Returns the metric tracker by reference.
        /**
         * \return A reference to the metric tracker being used by this
         *         converger to track the metrics of interest.
         */
        inline
        MetricTracker&
        GetMetricTracker(
            );

        /// Returns the metric tracker by const reference.
        /**
         * \return A const reference to the metric tracker being used by this
         *         converger to track the metrics of interest.
         */
        inline
        const MetricTracker&
        GetMetricTracker(
            ) const;


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:





    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:


        /// Places a metric value on the stack.
        /**
         * \param value The new metric value to add to the metric tracker
         *              stack.
         */
        inline
        void
        AddMetricValue(
            double value
            );

        /**
         * \brief Returns the percent changed from the \a older entry to the
         *        \a newer.
         *
         * \param older The index of the "older" value to compare.  To be older
         *              is to have been pushed in at an earlier generation.
         * \param newer The index of the "newer" value to compare.  To be newer
         *              is to have been pushed in at a later generation.
         * \return The percent change between the value at the \a older
         *         generation and the one at the \a newer generation.
         */
        inline
        double
        GetProgressPercentage(
            std::size_t older,
            std::size_t newer
            ) const;

        /// Sets the max number of metrics to keep in the stack at a time.
        /**
         * \param maxDepth The maximum number of values to keep in the metric
         *                 value stack at any given time.
         */
        inline
        void
        SetMetricStackMaxDepth(
            std::size_t maxDepth
            );

        /**
         * \brief Computes and returns the required number of display decimal
         *        places to use when printing percentages.
         *
         * This makes sure that at least 1 greater than the number of
         * significant digits in the percent change appears when printing
         * percentages.
         *
         * This assumes that percentages are to be displayed in the range
         * 0-100 (not 0-1).
         *
         * \return The number of digits to round percentages to when
         *         displaying.
         */
        int
        GetNumDP(
            ) const;


    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /// Tests for convergence
        /**
         * This method checks to see if the percent change in the
         * metric over the requested number of generations has fallen below
         * \a _change.  If so, this returns converged, otherwise not.
         *
         * \param group The group to use in the convergence check.
         * \param fitnesses The fitnesses of the designs in \a group.
         * \return true if convergence has been achieved and false otherwise.
         */
        virtual
        bool
        CheckConvergence(
            const JEGA::Utilities::DesignGroup& group,
            const FitnessRecord& fitnesses
            );

        /// Retrieves specific parameters using Get...FromDB methods.
        /**
         * This method is used to extract needed information for this
         * operator.  It does so using the "Get...FromDB" class
         * of methods from the GeneticAlgorithmOperator base class.
         *
         * This version extracts the number of generations over which to
         * track the metric and the minimum percent change that must occur
         * in order to indicate convergence.
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

    protected:

        /**
         * \brief Override to return the metric value to track.
         *
         * \param group The group to use in the convergence check.
         * \param fitnesses The fitnesses of the designs in \a group.
         * \return The value for this converger to track based on the designs
         *         in \a group and their associated \a fitnesses.
         */
        virtual
        double
        GetMetricValue(
            const JEGA::Utilities::DesignGroup& group,
            const FitnessRecord& fitnesses
            ) = 0;

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
         * \brief Constructs a MetricTrackerConvergerBase for use by \a
         *         algorithm.
         *
         * \param algorithm The GA for which the new converger is to be used.
         */
        MetricTrackerConvergerBase(
            GeneticAlgorithm& algorithm,
            bool absolute
            );

        /// Copy constructs a MetricTrackerConvergerBase.
        /**
         * \param copy The converger from which properties are to be duplicated
         *             into this.
         */
        MetricTrackerConvergerBase(
            const MetricTrackerConvergerBase& copy
            );

        /**
         * \brief Copy constructs a MetricTrackerConvergerBase for use by
         *        \a algorithm.
         *
         * \param copy The converger from which properties are to be duplicated
         *             into this.
         * \param algorithm The GA for which the new converger is to be used.
         */
        MetricTrackerConvergerBase(
            const MetricTrackerConvergerBase& copy,
            GeneticAlgorithm& algorithm
            );


}; // class MetricTrackerConvergerBase


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
#include "./inline/MetricTrackerConvergerBase.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_METRICTRACKERCONVERGERBASE_HPP

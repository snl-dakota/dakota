/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class DistanceNichingPostProcessor.

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

        Tue Sep 05 14:31:05 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the DistanceNichingPostProcessor class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMSS_DISTANCENICHINGPOSTPROCESSOR_HPP
#define JEGA_ALGORITHMSS_DISTANCENICHINGPOSTPROCESSOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmPostProcessor.hpp>
#include <../Utilities/include/JEGATypes.hpp>








/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace eddy
{
    namespace utilities
    {
        template <typename T> class extremes;
        typedef extremes<double> DoubleExtremes;
    }
}


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
class DistanceNichingPostProcessor;







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
 * The distance percentages extracted from the parameter database using the
 * name "method.jega.niche_vector".  These values are extracted as a double
 * vector. If they are not supplied in the parameter database, the default
 * value as defined by DEFAULT_DIST_PCT will be used for all of them.  These
 * are required in addition to any requirements of the base class.
 */
class DistanceNichingPostProcessor :
    public GeneticAlgorithmPostProcessor
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
         * \return The string "distance_postprocessor".
         */
        static
        const std::string&
        Name(
            );

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
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
         * \param algorithm The GA for which the new post processor is to be
         *        used.
         * \return A new, default instance of a DistanceNichingPostProcessor.
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

    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /// This version of PostProcess does absolutely nothing.
        /**
         * \param group The group of designs to perform post processing on.
         */
        virtual
        void
        PostProcess(
            JEGA::Utilities::DesignGroup& group
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

        /// Constructs an DistanceNichingPostProcessor for use by \a algorithm.
        /**
         * \param algorithm The GA for which this post processor is being
         *                  constructed.
         */
        DistanceNichingPostProcessor(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an DistanceNichingPostProcessor.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        DistanceNichingPostProcessor(
            const DistanceNichingPostProcessor& copy
            );

        /**
         * \brief Copy constructs an DistanceNichingPostProcessor for use by
         *        \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this post processor is being
         *                  constructed.
         */
        DistanceNichingPostProcessor(
            const DistanceNichingPostProcessor& copy,
            GeneticAlgorithm& algorithm
            );





}; // class DistanceNichingPostProcessor



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
#include "inline/DistanceNichingPostProcessor.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMSS_DISTANCENICHINGPOSTPROCESSOR_HPP

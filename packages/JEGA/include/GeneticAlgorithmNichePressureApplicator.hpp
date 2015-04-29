/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class GeneticAlgorithmNichePressureApplicator.

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

        Thu Jan 05 10:13:06 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the
 *        GeneticAlgorithmNichePressureApplicator class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_GENETICALGORITHMNICHEPRESSUREAPPLICATOR_HPP
#define JEGA_ALGORITHMS_GENETICALGORITHMNICHEPRESSUREAPPLICATOR_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <GeneticAlgorithmOperator.hpp>
#include <../Utilities/include/DesignMultiSet.hpp>








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
        class DesignGroup;
        class DesignOFSortSet;
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
class FitnessRecord;
class GeneticAlgorithmNichePressureApplicator;







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
 * \brief The base class for all GeneticAlgorithm niche pressure applicators.
 *
 * The job of a niche pressure applicator is to enforce some sort of
 * anti-clustering rule on the designs in a group.  This may be done in either
 * the design or performance space but would typically be done in the
 * performance space.
 *
 * It is recommended that any derived nichers be capable of caching niched
 * designs for re-insertion prior to selection in the next generation so that
 * the only operator that does selection is the Selection operator.  The
 * reinsertion should take place in the PreSelection method.  This
 * capability is supported by this base class which keeps a flag indicating
 * whether or not caching should take place.  This is a configuration flag
 * controlled by the user.
 *
 * The cache flag value is extracted from the parameter database using the name
 * "method.jega.cache_niched_designs".  It is extracted as a boolean parameter.
 * If it is not supplied in the parameter database, the default value as
 * defined by DEFAULT_CACHE_FLAG will be used. This is required in addition to
 * any requirements of the base class.
 */
class JEGA_SL_IEDECL GeneticAlgorithmNichePressureApplicator :
    public GeneticAlgorithmOperator
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

        /// The default value for the "caching designs" flag.
        static const bool DEFAULT_CACHE_FLAG;

    private:

        /**
         * \brief A flag that indicates whether or not niched-out designs
         *        should be cached for re-insertion prior to the next
         *        selection phase.
         */
        bool _cacheDesigns;

        /// The temporary storage for removed designs.
        JEGA::Utilities::DesignDVSortSet _desBuffer;



    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Indicates whether or not this nicher will cache niched designs.
        /**
         * \return True if the niched designs should be cached and false
         *         otherwise.
         */
        inline
        bool
        GetCacheDesigns(
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
         * \brief Sets the value of the flag that indicates whether or not
         *        niched-out designs should be cached and reinserted.
         *
         * If caching is set to false here and there are cached designs, they
         * will be flushed out to the target.
         *
         * \param cache Whether or not to cache niched designs.
         */
        void
        SetCacheDesigns(
            bool cache
            );

    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        inline
        bool
        TestBufferForClone(
            const JEGA::Utilities::Design& ofDes
            );

        template <typename DesContT>
        std::size_t
        TestBufferForClones(
            const DesContT& against
            );
            




    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /// Adds any buffered designs back into the supplied group.
        /**
         * If configuration allows, this nicher will store any removed designs
         * for re-insertion prior to the next selection cycle.  If that is the
         * case, this method will place those buffered designs into the
         * supplied group and remove them from the buffer.
         *
         * \param intoGroup The group into which to place any buffered designs.
         */
        void
        ReAssimilateBufferedDesigns(
            JEGA::Utilities::DesignGroup& intoGroup
            );

        /**
         * \brief Inserts the supplied Design into the design buffer if caching
         *        is enabled.
         *
         * \param des The Design that has been niched out and is to be cached
         *            if caching is enabled.
         * \return True if the design was cached and false otherwise.  It would
         *         not be cached only if caching is not enabled.  If that is
         *         the case, nothing is done with it.  It is then up to the
         *         derived operator to handle \a des.
         */
        bool
        BufferDesign(
            const JEGA::Utilities::Design* des
            );



    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /// Called prior to the call to the first of the selection operators.
        /**
         * The first of the selection operators is typically the fitness
         * assessor.  Derived class needn't override this method unless there
         * is something specific that they wish to do.  See the
         * RadialNichePressureApplicator for an example.
         *
         * \param population The group (presumably the population of the GA)
         *                   which will eventually be passed into the
         *                   ApplyNichePressure method after having been
         *                   modified by other selection operators.
         */
        virtual
        void
        PreSelection(
            JEGA::Utilities::DesignGroup& population
            );

        /**
         * \brief Override this method to carry out the specific niche pressure
         *        algorithm.
         *
         * Derived nichers should apply niche pressure to the supplied group
         * according to the assigned fitnesses as can be found in the supplied
         * fitness record.
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
            ) = 0;

        /// Retrieves specific parameters using Get...FromDB methods.
        /**
         * This method is used to extract needed information for this
         * operator.  It does so using the "Get...FromDB" class
         * of methods from the GeneticAlgorithmOperator base class.
         *
         * This base class implementation does nothing.
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

        /**
         * \brief Called prior to deletion of this operator to do any necessary
         *        finalization.
         *
         * This implementation passes all bufferred designs along to the
         * target.  The GA may retrieve any of them that are optimal from the
         * target upon finalization.
         *
         * \return The result of a call to the base class implementation.
         */
        virtual
        bool
        Finalize(
            );

        /// Returns the name of the type of this operator.
        /**
         * \return The string "Niche Pressure Applicator".
         */
        virtual
        std::string
        GetType(
            ) const;

        /**
         * \brief Gathers all Designs in \a of with maximal fitness as stored
         *        in \a fitnesses
         *
         * \param of The set from which to extract the best Designs.
         * \param fitnesses The record of fitnesses used to determine which
         *                  Designs are the best.
         * \return The set of all Designs with maximal fitness.
         */
        static
        JEGA::Utilities::DesignOFSortSet
        GetBest(
            const JEGA::Utilities::DesignOFSortSet& of,
            const FitnessRecord& fitnesses
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
         * \brief Constructs a GeneticAlgorithmNichePressureApplicator for use
         *        by \a algorithm.
         *
         * \param algorithm The GA for which this niche pressure applicator is
         *                  being constructed.
         */
        GeneticAlgorithmNichePressureApplicator(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs a GeneticAlgorithmNichePressureApplicator.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        GeneticAlgorithmNichePressureApplicator(
            const GeneticAlgorithmNichePressureApplicator& copy
            );

        /**
         * \brief Copy constructs a GeneticAlgorithmNichePressureApplicator for
         *        use by \a algorithm.
         *
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this niche pressure applicator is
         *                  being constructed.
         */
        GeneticAlgorithmNichePressureApplicator(
            const GeneticAlgorithmNichePressureApplicator& copy,
            GeneticAlgorithm& algorithm
            );




}; // class GeneticAlgorithmNichePressureApplicator



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
#include "inline/GeneticAlgorithmNichePressureApplicator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_GENETICALGORITHMNICHEPRESSUREAPPLICATOR_HPP

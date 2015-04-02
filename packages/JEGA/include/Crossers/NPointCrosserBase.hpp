/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class NPointCrosserBase.

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

        Tue Jun 17 07:11:01 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the NPointCrosserBase class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_NPOINTCROSSERBASE_HPP
#define JEGA_ALGORITHMS_NPOINTCROSSERBASE_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <set>
#include <functional>
#include <GeneticAlgorithmCrosser.hpp>
#include <utilities/include/int_types.hpp>








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
class NPointCrosserBase;







/*
================================================================================
Class Definition
================================================================================
*/

/// Base class for all N-Point crossers.
/**
 * This base class provides access to N for any N-point crossover routines.
 * The interpretation of N is up to the derived crosser.
 *
 * The number of crossover points (N) is extracted from the parameter database
 * using the name "method.jega.num_cross_points".  It is extracted as a size
 * type parameter.  If it is not supplied in the parameter database, the
 * default value as defined by DEFAULT_NUM_CROSS_PTS will be used.  This is
 * required in addition to any requirements of the base class.
 */
class NPointCrosserBase:
    public GeneticAlgorithmCrosser
{
    /*
    ============================================================================
    Typedefs
    ============================================================================
    */
    public:

    protected:

        /// A set to store the unique, sorted crossover points.
        typedef
        std::set<eddy::utilities::uint32_t>
        CrossPointSet;

        /// A set to store the unique, sorted crossover points.
        typedef
        std::set<
            eddy::utilities::uint32_t,
            std::greater<eddy::utilities::uint32_t>
            >
        ReverseCrossPointSet;

    private:

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    public:

        /// The default number of crossover points (N)
        static const eddy::utilities::uint32_t DEFAULT_NUM_CROSS_PTS;

    private:

        /// The number of crossover points to use (N).
        eddy::utilities::uint32_t _ncpts;



    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the number of crossover points (N) to \a val.
        /**
         * This does not place any restrictions on the value of \a val.  It
         * will log a verbose level entry indicating the new value.
         *
         * \param val The new number of crossover points for this N-Point
         *            crossover routine.
         */
        void
        SetNumCrossPoints(
            eddy::utilities::uint32_t val
            );



    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Gets the number of crossover points (N).
        /**
         * \return The current number of crossover points for this crosser (N).
         */
        inline
        eddy::utilities::uint32_t
        GetNumCrossPoints(
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


        /**
         * \brief Returns a set of unique, sorted crossover points in the
         *        supplied range.
         *
         * The crossover points will be numbers in the supplied range [lo, hi]
         * and will be sorted from smallest to largest.  The set will be the
         * size of min(_ncpts, hi-lo+1).
         *
         * \param lo The lowest allowable crossover point value.
         * \param hi The highest allowable crossover point value.
         * \return A set of crossover points in sorted order all in the range
         *         [lo, hi].
         */
        CrossPointSet
        GetCrossoverPoints(
            eddy::utilities::uint32_t lo,
            eddy::utilities::uint32_t hi
            ) const;

        /**
         * \brief Returns a set of unique, sorted crossover points in the
         *        supplied range.
         *
         * The crossover points will be numbers in the supplied range [lo, hi]
         * and will be sorted from largest to smallest.  The set will be the
         * size of min(_ncpts, hi-lo+1).
         *
         * This method is only necessary b/c the "Forte Developer 7 C++ 5.4
         * Patch 111715-14 2004/03/11"'s support for reverse iteration stinks.
         *
         * \param lo The lowest allowable crossover point value.
         * \param hi The highest allowable crossover point value.
         * \return A set of crossover points in sorted order all in the range
         *         [lo, hi].
         */
        ReverseCrossPointSet
        GetReverseCrossoverPoints(
            eddy::utilities::uint32_t lo,
            eddy::utilities::uint32_t hi
            ) const;

        /**
         * \brief Returns a set of unique, sorted crossover points in the
         *        supplied range.
         *
         * The crossover points will be numbers in the supplied range [lo, hi]
         * and will be sorted from smallest to largest.  The set will be the
         * size of min(ncpts, hi-lo+1).
         *
         * This method is not static so that it can call other non static
         * members and do detailed logging.
         *
         * \param lo The lowest allowable crossover point value.
         * \param hi The highest allowable crossover point value.
         * \param ncpts The number of crossover points needed.
         * \return A set of crossover points in sorted order all in the range
         *         [lo, hi].
         */
        CrossPointSet
        CreateCrossoverPointSet(
            eddy::utilities::uint32_t lo,
            eddy::utilities::uint32_t hi,
            eddy::utilities::uint32_t ncpts
            ) const;

        /**
         * \brief Returns a set of unique, sorted crossover points in the
         *        supplied range.
         *
         * The crossover points will be numbers in the supplied range [lo, hi]
         * and will be sorted from smallest to largest.  The set will be the
         * size of min(ncpts, hi-lo+1).
         *
         * This method is not static so that it can call other non static
         * members and do detailed logging.
         *
         * \param lo The lowest allowable crossover point value.
         * \param hi The highest allowable crossover point value.
         * \param ncpts The number of crossover points needed.
         * \return A set of crossover points in sorted order all in the range
         *         [lo, hi].
         */
        ReverseCrossPointSet
        CreateReverseCrossoverPointSet(
            eddy::utilities::uint32_t lo,
            eddy::utilities::uint32_t hi,
            eddy::utilities::uint32_t ncpts
            ) const;



    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        /// Retrieves specific parameters using Get...FromDB methods.
        /**
         * This method is used to extract needed information for this
         * operator.  It does so using the "Get...FromDB" class
         * of methods from the GeneticAlgorithmOperator base class.
         *
         * This version extracts the number of crossover points.  Derived
         * crossers may choose to use this value in any way they wish.
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


    private:





    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:

        /**
         * \brief Generates the required number of unique crossover points and
         *        loads them into an instance of a CPSet which must be a std::
         *        set or derivative.
         *
         * The set will be the size of min(ncpts, hi-lo+1).
         *
         * \param lo The lowest allowable crossover point value.
         * \param hi The highest allowable crossover point value.
         * \param ncpts The number of crossover points needed.
         * \return A set of crossover points in sorted order all in the range
         *         [lo, hi].
         */
        template <typename CPSet>
        CPSet
        _LoadCrossoverPoints(
            eddy::utilities::uint32_t lo,
            eddy::utilities::uint32_t hi,
            eddy::utilities::uint32_t ncpts
            ) const;



    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs an NPointCrosserBase for use by \a algorithm.
        /**
         * \param algorithm The GA for which this crosser is being
         *                  constructed.
         */
        NPointCrosserBase(
            GeneticAlgorithm& algorithm
            );

        /// Copy constructs an NPointCrosserBase.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         */
        NPointCrosserBase(
            const NPointCrosserBase& copy
            );

        /// Copy constructs an NPointCrosserBase for use by \a algorithm.
        /**
         * \param copy The instance from which properties should be copied into
         *             this.
         * \param algorithm The GA for which this crosser is being
         *                  constructed.
         */
        NPointCrosserBase(
            const NPointCrosserBase& copy,
            GeneticAlgorithm& algorithm
            );



};



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
#include "./inline/NPointCrosserBase.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_NPOINTCROSSERBASE_HPP

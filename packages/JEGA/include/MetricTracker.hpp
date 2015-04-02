/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class MetricTracker.

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

        Tue Jul 22 10:29:01 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MetricTracker class.
 */





/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_METRICTRACKER_HPP
#define JEGA_ALGORITHMS_METRICTRACKER_HPP







/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <deque>








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
class MetricTracker;






/*
================================================================================
In Namespace File Scope Typedefs
================================================================================
*/
/// A vector that will serve as the stack.
typedef
std::deque<double>
DoubleStack;



/*
================================================================================
Class Definition
================================================================================
*/

/// Class that keeps a stack of values and provides info about them.
/**
 * This class keeps a stack of specified depth containing values.  It has
 * a number of methods available with which you can obtain information.
 *
 * It can serve as a base or a member.
 */
class MetricTracker
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// A pair that describes a location and the value at that location.
        typedef
        std::pair<DoubleStack::size_type, double>
        LocPair;

    protected:


    private:


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:


        /// The underlying container that serves as a stack.
        DoubleStack _stack;

        /// The requested maximum depth of the stack.
        DoubleStack::size_type _maxDepth;


    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


        /// Sets the desired max depth of the stack.
        /**
         * If the passed in depth is less than the current depth,
         * elements on the bottom of the stack will be erased.
         *
         * \param depth The new depth of the stack.
         * \return The number of elements removed from the bottom.
         */
        std::size_t
        SetStackMaxDepth(
            std::size_t depth
            );



    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Gets the current depth of the stack.
        /**
         * \return The current number of metric values in the stack.
         */
        inline
        std::size_t
        GetStackDepth(
            ) const;

        /// Gets the desired max depth of the stack.
        /**
         * \return The maximum allowable number of metric values in the stack.
         */
        inline
        std::size_t
        GetStackMaxDepth(
            ) const;

        /// Allows immutable access to the underlying stack.
        /**
         * \return A const reference to the underlying stack data structure.
         */
        inline
        const DoubleStack&
        GetStack(
            ) const;



    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Pushes a value onto the top of the stack.
        /**
         * If the stack is full, the bottom value is automatically
         * erased.
         *
         * \param val The new metric value to put on the top of the stack.
         */
        inline
        void
        Push(
            double val
            );

        /// Pops a value off the top of the stack.
        /**
         * This removes the last (most recent) value pushed onto the stack.
         */
        inline
        void
        Pop(
            );

        /// Pops a value off the bottom of the stack.
        /**
         * This removes the oldest value in the stack.
         */
        inline
        void
        PopBottom(
            );

        /// Returns the percent difference between the first and last entries.
        /**
         * The percent difference is calculated as:
         * \verbatim
            |(Top - Bottom) / Bottom|
           \endverbatim
         *
         * \return The percentage change that has occurred in the metric
         *         between the oldest and newest entries.
         */
        inline
        double
        GetPercentDifference(
            ) const;

        /// Returns the percent difference between the two specified entries.
        /**
         * The percent difference is calculated as:
         * \verbatim
            |(at(elem2) - at(elem1)) / at(elem1)|
           \endverbatim
         *
         * Typically, \a elem1 would be a value less than that of \a elem2
         * since the oldest value has index 0 and the newest has a maximal
         * index.  You could reverse this if you wanted the answer with the
         * values switched.
         *
         * \param elem1 The index of the older of the two values between which
         *              to compute the percent difference.
         * \param elem2 The index of the newer of the two values between which
         *              to compute the percent difference.
         * \return The percentage change that has occurred in the metric
         *         between the elem1'th and elem2'th entries.
         */
        double
        GetPercentDifference(
            std::size_t elem1,
            std::size_t elem2
            ) const;

        /// Returns the last value pushed on the stack.
        /**
         * \return The value that was most recently pushed onto the stack.
         */
        inline
        double
        Top(
            ) const;

        /// returns the "deepest" value of the stack.
        /**
         * \return The value that was pushed onto the stack longest ago of
         *         all that remain.
         */
        inline
        double
        Bottom(
            ) const;

        /// Returns true if the stack contains as many entries as it can.
        /**
         * \return this->GetStackDepth() >= this->GetMaxStackDepth()
         */
        inline
        bool
        IsFull(
            ) const;

        /// Gets the largest value currently contained in the tracker.
        /**
         * \return The largest of all values in the stack along with it's
         *         location.
         */
        LocPair
        MaxValue(
            ) const;

        /// Gets the smallest value currently contained in the tracker.
        /**
         * \return The smallest of all values in the stack along with it's
         *         location.
         */
        LocPair
        MinValue(
            ) const;

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /// Removes any elements from the bottom in excess of \a _maxDepth
        void
        PruneTheStack(
            );



    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:


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


        /// Default constructs a MetricTracker.
        MetricTracker(
            );

        /// Copy constructs a MetricTracker
        /**
         * \param copy The object to copy when creating this new object.
         */
        MetricTracker(
            const MetricTracker& copy
            );

        /// Constructs a MetricTracker with a max depth of \a maxDepth
        /**
         * \param maxDepth The initial maximum allowable depth of the stack.
         */
        MetricTracker(
            std::size_t maxDepth
            );


}; // class MetricTracker


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
#include "./inline/MetricTracker.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_METRICTRACKER_HPP

/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Definition of class extremes.

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

        Fri May 16 12:46:53 2003 - Original Version (JE)

================================================================================
*/



/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the extremes class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_UTILITIES_EXTREMES_HPP
#define EDDY_UTILITIES_EXTREMES_HPP







/*
================================================================================
Includes
================================================================================
*/
#include <vector>
#include <utility>
#include "config.hpp"








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
namespace eddy {
    namespace utilities {








/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
template <typename T>
class extremes;




/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/

/// extremes class for use with double arguments.
typedef
extremes<double>
DoubleExtremes;

/// extremes class for use with float arguments.
typedef
extremes<float>
FloatExtremes;

/// extremes class for use with int arguments.
typedef
extremes<int>
IntExtremes;









/*
================================================================================
Class Definition
================================================================================
*/

/// Class to contain a set of extremes
/**
 * This class can be used to store and access a set of extremes.  For example,
 * it may be used to store the upper and lower bounds on some set of
 * quantities.
 *
 * A few typedefs are provided for convenience including:
 * \verbatim
        DoubleExtremes
        IntExtremes
        FloatExtremes
   \endverbatim
 */
template <typename T>
class extremes
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    private:

        /// This is how this class stores each set of upper and lower bounds.
        typedef
        std::pair<T, T>
        min_max_pair;

        /// This is a container for sets of upper and lower bounds.
        typedef
        std::vector<min_max_pair>
        extremes_vector;

    public:

        /// The type of this class (short hand)
        typedef
        extremes<T>
        my_type;

        /// Type representing the size of this container
        typedef
        typename extremes_vector::size_type
        size_type;

        /// A vector of T's.
        typedef
        std::vector<T>
        value_vector;

    protected:




    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// Underlying container in which data is stored.
        extremes_vector _data;

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /// Sets the upper bound of \a elem to \a val
        /**
         * \param elem The index of the element of interest in these extremes.
         * \param val The value to set as the new max for element at \a elem.
         */
        inline
        void
        set_max(
            size_type elem,
            const T& val
            );

        /// Sets the lower bound of \a elem to \a val
        /**
         * \param elem The index of the element of interest in these extremes.
         * \param val The value to set as the new min for element at \a elem.
         */
        inline
        void
        set_min(
            size_type elem,
            const T& val
            );




    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Returns the upper bound of \a elem.
        /**
         * \param elem The index of the element of interest in these extremes.
         * \return The max value for the extreme set at \a elem.
         */
        inline
        const T&
        get_max(
            size_type elem
            ) const;

        /// Returns the lower bound of \a elem.
        /**
         * \param elem The index of the element of interest in these extremes.
         * \return The minx value for the extreme set at \a elem.
         */
        inline
        const T&
        get_min(
            size_type elem
            ) const;

        /// Returns the current number of elements in this container.
        /**
         * \return The number of min/max pairs in this extremes set.
         */
        inline
        size_type
        size(
            ) const;


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /**
         * \brief Compares \a val to the current max at \a elem and takes it if
         *        larger.
         *
         * \param elem The index of the element of interest in these extremes.
         * \param val The value to conditionally take as the new max for theS
         *            element at \a elem.
         * \return true if the value was taken and false otherwise.
         */
        inline
        bool
        take_if_max(
            size_type elem,
            const T& val
            );

        /**
         * \brief Compares \a val to the current min at \a elem and takes it if
         *        smaller.
         *
         * \param elem The index of the element of interest in these extremes.
         * \param val The value to conditionally take as the new min for the
         *            element at \a elem.
         * \return true if the value was taken and false otherwise.
         */
        inline
        bool
        take_if_min(
            size_type elem,
            const T& val
            );

        /// Returns true if \a val is equal to the max stored for \a elem.
        /**
         * The comparison is made using operator == which must be supported
         * by type T.
         *
         * \param elem The index of the element of interest in these extremes.
         * \param val The value to test for equality to the max.
         * \return true if val == get_max(elem) and false otherwise.
         */
        inline
        bool
        equals_max(
            size_type elem,
            const T& val
            ) const;

        /// Returns true if \a val is equal to the min stored for \a elem.
        /**
         * The comparison is made using operator == which must be supported
         * by type T.
         *
         * \param elem The index of the element of interest in these extremes.
         * \param val The value to test for equality to the min.
         * \return true if val == get_min(elem) and false otherwise.
         */
        inline
        bool
        equals_min(
            size_type elem,
            const T& val
            ) const;

        /**
         * \brief Returns true if \a val is equal to the max or min stored for
         *        \a elem.
         *
         * The comparison is made using operator == which must be supported
         * by type T.
         *
         * \param elem The index of the element of interest in these extremes.
         * \param val The value to test for equality to the max or min.
         * \return true if val == get_max(elem) || val == get_min(elem)
         *         and false otherwise.
         */
        inline
        bool
        equals_max_or_min(
            size_type elem,
            const T& val
            ) const;

        /**
         * \brief Compares \a val to the current max and min at \a elem and
         *        takes it if larger or smaller respectively.
         *
         * \param elem The index of the element of interest in these extremes.
         * \param val The value to conditionally take as the new min and/or max
         *            for the element at \a elem.
         * \return -1 if val was taken as the min, 1 if taken as the max, 2 if
         *         taken as both, and 0 if neither.
         */
        int
        take_if_either(
            size_type elem,
            const T& val
            );

        /// Takes the larger of \a val1 and \a val2 as the max at \a elem.
        /**
         * \param elem The index of the element of interest in these extremes.
         * \param val1 The frist value to consider as a possible max.
         * \param val2 The second value to consider as a possible max.
         * \return -1 if val1 was taken and 1 of val2.
         */
        int
        take_max_of(
            size_type elem,
            const T& val1,
            const T& val2
            );

        /// Takes the smaller of \a val1 and \a val2 as the min at \a elem.
        /**
         * \param elem The index of the element of interest in these extremes.
         * \param val1 The frist value to consider as a possible min.
         * \param val2 The second value to consider as a possible min.
         * \return -1 if val1 was taken and 1 of val2.
         */
        int
        take_min_of(
            size_type elem,
            const T& val1,
            const T& val2
            );

        /// Returns the difference between the max and min at \a elem.
        /**
         * \param elem The index of the element of interest in these extremes.
         * \return The range of the element at \a elem.
         */
        inline
        T
        get_range(
            size_type elem
            ) const;

        /// Returns true if no extremes have been stored here yet.
        /**
         * \return true if there are no min/max pairs yet and false otherwise.
         */
        inline
        bool
        empty(
            ) const;

        /// Prepares this object to store \a size sets of extremes.
        /**
         * Default values are used to initialize maxs and mins.
         * After this, empty will return false.
         *
         * \param size The number of locations to allocate for min/max pairs.
         */
        inline
        void
        allocate(
            size_type size
            );

        /// Prepares this object to store \a size sets of extremes.
        /**
         * Supplied values are used to initialize maxs and mins.
         * After this, empty will return false.
         *
         * \param size The number of locations to allocate for min/max pairs.
         * \param minVal The default minimum value to apply to the new pairs.
         * \param maxVal The default maximum value to apply to the new pairs.
         */
        inline
        void
        allocate(
            size_type size,
            const T& minVal,
            const T& maxVal
            );

        /// Resets all extreme values to default values.
        /**
         * This method does not change the size of this extremes object.  If
         * empty returned false before, it will still return false after this
         * call.
         */
        inline
        void
        clear(
            );

        /// Resets all extreme values to supplied values.
        /**
         * This method does not change the size of this extremes object.  If
         * empty returned false before, it will still return false after this
         * call.
         *
         * \param minVal The default minimum value to apply to the new pairs.
         * \param maxVal The default maximum value to apply to the new pairs.
         */
        inline
        void
        clear(
            const T& minVal,
            const T& maxVal
            );

        /// Removes all elements from this set of extremes.
        /**
         * empty will return true after this action.
         */
        inline
        void
        flush(
            );

        /// Assigns the extremes of rhs to this.
        /**
         * \param rhs The existing extremes from which to copy properties into
         *            this.
         * \return this after assignment has taken place.
         */
        const my_type&
        operator = (
            const my_type& rhs
            );

        /**
         * \brief Assimilates rhs into this taking the greater of the entries
         *        for each element.
         *
         * \param rhs The existing extremes from which to add the contents of
         *            into this.
         * \return true if any of the extremes have changed and false
         *         otherwise.
         */
        bool
        merge(
            const my_type& rhs
            );

        /// Returns a vector of just the max's of this extremes object.
        /**
         * \return A vector containing only the maximum values in this extremes
         *         object.
         */
        value_vector
        get_maxs(
            ) const;

        /// Returns a vector of just the min's of this extremes object.
        /**
         * \return A vector containing only the maximum values in this extremes
         *         object.
         */
        value_vector
        get_mins(
            ) const;


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

        /// Default constructs an empty extremes object
        inline
        extremes(
            );

        /// Construcs an extremes object with size default entries.
        /**
         * \param size The number of locations to allocate for min/max pairs.
         */
        inline
        extremes(
            size_type size
            );

        /// Construcs an extremes object with size entries.
        /**
         * This constructor uses minVal as the initial value of all mins and
         * maxVal ans the initial value of all maxs.
         *
         * \param size The number of locations to allocate for min/max pairs.
         * \param minVal The default minimum value to apply to the new pairs.
         * \param maxVal The default maximum value to apply to the new pairs.
         */
        inline
        extremes(
            size_type size,
            const T& minVal,
            const T& maxVal
            );

        /// Copy construcs an extremes.
        /**
         * \param copy The existing extremes object to duplicate into this new
         *             one.
         */
        inline
        extremes(
            const my_type& copy
            );

}; // class extremes






/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace utilities
} // namespace eddy







/*
================================================================================
Include Inlined Methods File
================================================================================
*/
#include "./inline/extremes.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_UTILITIES_EXTREMES_HPP

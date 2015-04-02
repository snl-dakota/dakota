/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Definition of class numeric_limits.

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

        Tue Feb 07 08:14:40 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the numeric_limits class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_UTILITIES_NUMERIC_LIMITS_HPP
#define EDDY_UTILITIES_NUMERIC_LIMITS_HPP







/*
================================================================================
Includes
================================================================================
*/
#include <limits>
#include <cfloat>
#include <climits>
#include <cstddef>
#include "config.hpp"
#include "int_types.hpp"






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
template <typename ArT>
class numeric_limits;







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
 * \brief A base class for all Eddy specialized numeric limits types.
 */
template <typename ArT>
class numeric_limits :
    public std::numeric_limits<ArT>
{
    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the smallest value that this arithmetic type can hold.
        /**
         * This will be a large negative number for signed types and 0 for
         * unsigned types.
         *
         * The default is to return 0 so no specializations are needed for
         * the unsigned types.  Otherwise, only the specializations provide
         * meaningful results.
         *
         * \return The smallest value that can be held by type ArT.
         */
        static
        ArT
        smallest(
            ) throw()
        {
            return 0;
        }

}; // class numeric_limits

/**
 * \brief A specialization of eddy::utilities::numeric_limits for use on
 *        eddy::utilities::int8_ts.
 */
template<>
class numeric_limits<eddy::utilities::int8_t> :
    public std::numeric_limits<eddy::utilities::int8_t>
{
    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the smallest value that this arithmetic type can hold.
        /**
         * \return CHAR_MIN
         */
        static
        eddy::utilities::int8_t
        smallest(
            ) throw()
        {
            return SCHAR_MIN;
        }

}; // class numeric_limits<eddy::utilities::int8_t>

/**
 * \brief A specialization of eddy::utilities::numeric_limits for use on
 *        eddy::utilities::int16_ts.
 */
template<>
class numeric_limits<eddy::utilities::int16_t> :
    public std::numeric_limits<eddy::utilities::int16_t>
{
    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the smallest value that this arithmetic type can hold.
        /**
         * \return SHRT_MIN
         */
        static
        eddy::utilities::int16_t
        smallest(
            ) throw()
        {
            return SHRT_MIN;
        }

}; // class numeric_limits<eddy::utilities::int16_t>

/**
 * \brief A specialization of eddy::utilities::numeric_limits for use on
 *        eddy::utilities::int32_ts.
 */
template<>
class numeric_limits<eddy::utilities::int32_t> :
    public std::numeric_limits<eddy::utilities::int32_t>
{
    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the smallest value that this arithmetic type can hold.
        /**
         * \return INT_MIN
         */
        static
        eddy::utilities::int32_t
        smallest(
            ) throw()
        {
            return INT_MIN;
        }

}; // class numeric_limits<eddy::utilities::int32_t>

/**
 * \brief A specialization of eddy::utilities::numeric_limits for use on
 *        eddy::utilities::int64_ts.
 */
template<>
class numeric_limits<eddy::utilities::int64_t> :
    public std::numeric_limits<eddy::utilities::int64_t>
{
    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the smallest value that this arithmetic type can hold.
        /**
         * \return -std::numeric_limits<eddy::utilities::int64_t>::max()-1;
         */
        static
        eddy::utilities::int64_t
        smallest(
            ) throw()
        {
            return -std::numeric_limits<eddy::utilities::int64_t>::max()-1;
        }

}; // class numeric_limits<eddy::utilities::int64_t>

/**
 * \brief A specialization of eddy::utilities::numeric_limits for use on
 *        floats.
 */
template<>
class numeric_limits<float> :
    public std::numeric_limits<float>
{
    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the smallest value that this arithmetic type can hold.
        /**
         * \return -FLT_MAX
         */
        static
        float
        smallest(
            ) throw()
        {
            return -FLT_MAX;
        }

}; // class numeric_limits<float>

/**
 * \brief A specialization of eddy::utilities::numeric_limits for use on
 *        doubles.
 */
template<>
class numeric_limits<double> :
    public std::numeric_limits<double>
{
    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Returns the smallest value that this arithmetic type can hold.
        /**
         * \return -DBL_MAX
         */
        static
        double
        smallest(
            ) throw()
        {
            return -DBL_MAX;
        }

}; // class numeric_limits<double>




/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace utilities
} // namespace eddy






/*
================================================================================
Include Inlined Functions File
================================================================================
*/
// Not using an Inlined Functions File.



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_UTILITIES_NUMERIC_LIMITS_HPP

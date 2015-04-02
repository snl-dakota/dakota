/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Portable collection of sized integer types.

    NOTES:



    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Dec 14 09:32:21 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains portable declarations of sized integer types.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_UTILITIES_INT_TYPES_HPP
#define EDDY_UTILITIES_INT_TYPES_HPP







/*
================================================================================
Includes
================================================================================
*/
#include "config.hpp"

#if defined(_MSC_VER)
// no includes needed
#else
#include <inttypes.h>
#endif






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







/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/

#if defined(_MSC_VER)

    /// An 8 bit signed integer type.
    typedef
    signed char
    int8_t;

    /// A 16 bit signed integer type.
    typedef
    signed short
    int16_t;

    /// A 32 bit signed integer type.
    typedef
    signed int
    int32_t;

    /// A 64 bit signed integer type.
    typedef
    signed __int64
    int64_t;

    /// An 8 bit unsigned integer type.
    typedef
    unsigned char
    uint8_t;

    /// A 16 bit unsigned integer type.
    typedef
    unsigned short
    uint16_t;

    /// A 32 bit unsigned unsiged integer type.
    typedef
    unsigned int
    uint32_t;

    /// A 64 bit unsigned integer type.
    typedef
    unsigned __int64
    uint64_t;

#else

    /// Use standard 8 bit integer type.
    typedef
    ::int8_t
    int8_t;

    /// Use standard 16 bit integer type.
    typedef
    ::int16_t
    int16_t;

    /// Use standard 32 bit integer type.
    typedef
    ::int32_t
    int32_t;

    /// Use standard 64 bit integer type.
    typedef
    ::int64_t
    int64_t;

    /// Use standard 8 bit unsigned integer type.
    typedef
    ::uint8_t
    uint8_t;

    /// Use standard 16 bit unsigned integer type.
    typedef
    ::uint16_t
    uint16_t;

    /// Use standard 32 bit unsigned integer type.
    typedef
    ::uint32_t
    uint32_t;

    /// Use standard 64 bit unsigned integer type.
    typedef
    ::uint64_t
    uint64_t;

#endif

/// A type to serve as the largest possible signed integer.
typedef
int64_t
intmax_t;

/// A type to serve as the largest possible unsigned integer.
typedef
uint64_t
uintmax_t;

/// A type to serve as the bit-holder in an 8 bit bitmask.
typedef
uint8_t
bitmask8_t;

/// A type to serve as the bit-holder in a 16 bit bitmask.
typedef
uint16_t
bitmask16_t;

/// A type to serve as the bit-holder in a 32 bit bitmask.
typedef
uint32_t
bitmask32_t;

/// A type to serve as the bit-holder in a 64 bit bitmask.
typedef
uint64_t
bitmask64_t;


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
#endif // EDDY_UTILITIES_INT_TYPES_HPP

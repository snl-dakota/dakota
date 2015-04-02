/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of logging project threadsafe utilities.

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

        Sat Jan 29 15:45:44 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains various threadsafe utilities for the logging project.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_DETAIL_THREADSAFE_HPP
#define EDDY_LOGGING_DETAIL_THREADSAFE_HPP





/*
================================================================================
Conditional Inclusion
================================================================================
*/
// don't put these together b/c threadsafe ends before logging on.
#ifdef EDDY_LOGGING_ON
#ifdef EDDY_THREADSAFE




/*
================================================================================
Includes
================================================================================
*/
#include "../include/config.hpp"
#include "../../threads/include/mutex_lock.hpp"





/*
================================================================================
MACRO definitions
================================================================================
*/
#ifndef EDDY_NO_EXCEPTIONS

    /// A macro to open a try block with a scoped lock on (m).
    /**
     * This macro expands to nothing if EDDY_THREADSAFE is
     * not defined.
     *
     * Also, if EDDY_THREADSAFE_NO_EXCEPTIONS is defined, this macro does
     * not open a try block but does open a scoped block.
     *
     * \param l The name to give to the created lock object.
     * \param m The mutex to be locked by (l).
     *
     * \ingroup dev_macros
     */
#   define EDDY_LOGGING_TRY_SCOPEDLOCK(l, m)                              \
            try { EDDY_SCOPEDLOCK(l, m)

    /// A macro to close a try block and catch/pass on any error.
    /**
     * This macro expands to nothing if EDDY_THREADSAFE is
     * not defined.
     *
     * Also, if EDDY_THREADSAFE_NO_EXCEPTIONS is defined, this only expands to
     * the closing of a scoped block.
     *
     * \ingroup dev_macros
     */
#   define EDDY_LOGGING_CATCH_THREAD_EXCEPTION } catch(...) { throw; }

#else // defined(EDDY_NO_EXCEPTIONS)

#   define EDDY_LOGGING_TRY_SCOPEDLOCK(l, m)                              \
            { EDDY_SCOPEDLOCK(l, m)

#   define EDDY_LOGGING_CATCH_THREAD_EXCEPTION }

#endif


#else // if !EDDY_THREADSAFE

#   define EDDY_LOGGING_TRY_SCOPEDLOCK(l, m)

#   define EDDY_LOGGING_CATCH_THREAD_EXCEPTION

#endif // EDDY_THREADSAFE



/*
================================================================================
Begin Namespace
================================================================================
*/
namespace eddy {
    namespace logging {
        namespace detail {




/*
================================================================================
End Namespace
================================================================================
*/
        } // namespace detail
    } // namespace logging
} // namespace eddy



/*
================================================================================
End of Conditional Inclusion
================================================================================
*/
#endif // EDDY_LOGGING_ON

/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_LOGGING_DETAIL_THREADSAFE_HPP

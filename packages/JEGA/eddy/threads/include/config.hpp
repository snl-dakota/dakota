/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Thread Safety project configuration code.

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

        Fri Feb 10 15:45:44 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains various Logging project configuration code.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_THREADS_CONFIG_HPP
#define EDDY_THREADS_CONFIG_HPP


/*
================================================================================
Includes
================================================================================
*/
#include "../../config/include/config.hpp"



/*
================================================================================
Version Information
================================================================================
*/
/// The current version of the Eddy threads project.
#define EDDY_THREADS_VERSION "1.0.0"

/// The name of the project complete with version number.
#define EDDY_THREADS_PACKAGE "Eddy Threadsafe Project v"EDDY_THREADS_VERSION



/*
================================================================================
Macros
================================================================================
*/

#ifdef EDDY_THREADSAFE

    /**
     * \brief This is useful in constructor initialization lists when things
     *        like mutexes only appear if threadsafe is on.
     */
#   define EDDY_COMMA_IF_THREADSAFE ,

    /// A macro to conditionally include code when EDDY_THREADSAFE is defined.
    /**
     * If EDDY_THREADSAFE is defined, the following macro expands to exactly
     * \a a.  Otherwise, it expands to nothing.
     *
     * So the following line:
     * \code
            EDDY_IF_THREADSAFE(mutex.Lock();)
       \endcode
     * would become:
     * \code
            mutex.Lock();
       \endcode
     * if EDDY_THREADSAFE were defined and would disappear otherwise.
     *
     * \param a The statement to conditionally create.
     */
#   define EDDY_IF_THREADSAFE(a) a

    /**
     * \brief A macro to conditionally include code when EDDY_THREADSAFE is NOT
     *        defined.
     *
     * If EDDY_THREADSAFE is not defined, the following macro expands to
     * exactly \a a. Otherwise, it expands to nothing.
     *
     * So the following line:
     * \code
            EDDY_IF_NO_THREADSAFE(int i;)
       \endcode
     * would become:
     * \code
            int i;
       \endcode
     * if EDDY_THREADSAFE weren't defined and would disappear otherwise.
     *
     * \param a The statement to conditionally create.
     */
#   define EDDY_IF_NO_THREADSAFE(a)

#else

    /// Expands to nothing because EDDY_THREADSAFE is not defined.
#   define EDDY_COMMA_IF_THREADSAFE

    /// Expands to nothing because EDDY_THREADSAFE is not defined.
#   define EDDY_IF_THREADSAFE(a)

    /// Expands to \a a because EDDY_THREADSAFE is not defined.
#   define EDDY_IF_NO_THREADSAFE(a) a

#endif


/// A means of inlining a function if thread safe is off and not otherwise.
#define EDDY_NOTHREAD_INLINE EDDY_IF_NO_THREADSAFE(inline)

/// A macro to create a scoped lock instance named (l) on a mutex (m).
/**
 * This macro expands to nothing if EDDY_THREADSAFE is
 * not defined.
 *
 * \param l The name to give to the created lock object.
 * \param m The mutex to be locked by (l).
 */
#define EDDY_SCOPEDLOCK(l, m)                                               \
    EDDY_IF_THREADSAFE(eddy::threads::mutex_lock l(m);)


/// A macro to declare a mutex named (m).
/**
 * This macro expands to nothing if EDDY_THREADSAFE is
 * not defined.
 *
 * \param m The name of the mutex to be declared.
 */
#define EDDY_DECLARE_MUTEX(m)                                               \
    EDDY_IF_THREADSAFE(eddy::threads::mutex m;)

/// A macro to declare a mutable mutex named (m).
/**
 * This should only be used to declare class member data instances
 * of mutexes.
 *
 * This macro expands to nothing if EDDY_THREADSAFE is
 * not defined.
 *
 * \param m The name of the mutex to be declared.
 */
#define EDDY_DECLARE_MUTABLE_MUTEX(m)                                       \
    EDDY_IF_THREADSAFE(mutable eddy::threads::mutex m;)

/// A macro to initialize a mutex in a constructor initialization list.
/**
 * This macro expands to nothing if EDDY_THREADSAFE is
 * not defined.
 *
 * The delcaration will not be followed by a comma.  Use the
 * EDDY_COMMA_IF_THREADSAFE macro to put one if desired.
 *
 * \param m The name of the mutex to be initialized in a constructor
 *          initialization list.
 * \param arg The argument to pass into the mutex constructor
 */
#define EDDY_INIT_MUTEX(m, arg) EDDY_IF_THREADSAFE(m(arg))

/*
================================================================================
Begin Namespace
================================================================================
*/
namespace eddy {
    namespace threads {




/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace threads
} // namespace eddy




/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_THREADS_CONFIG_HPP

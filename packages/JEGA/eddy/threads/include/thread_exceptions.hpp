/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Definition of exception classes used by the Eddy C++ Thread Safety
        Project.

    NOTES:

        This project is modeled heavily after the boost thread library.  Some
        Serious shortcommings of the boost thread library make this one
        necessary such as the lack of ability to cancel threads.

        Hopefully, the shortcommings of the boost library will be taken care of
        at which time this library will likely be changed to use it.

        This file contains a number of exception derivative classes to be
        thrown by various methods/functions of the classes in the Multi-
        Threading Module.  Each exception class has a description associated
        with it.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Mar 02 11:21:20 2004 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definitions of the exception classes
 * used by the Eddy C++ Thread Safety Project.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_THREADS_THREAD_EXCEPTIONS_HPP
#define EDDY_THREADS_THREAD_EXCEPTIONS_HPP







/*
================================================================================
Includes
================================================================================
*/
// config.hpp should be the first include.
#include "../include/config.hpp"

#include <stdexcept>






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
    namespace threads {





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







/*
================================================================================
Class Definition
================================================================================
*/
/// This error represents a failure to allocate a thread resource
/**
 * An example of using this error properly would be throwing it when a
 * thread creation fails b/c the maximum number has been reached or resources
 * are unavailable or whatever.
 */
class EDDY_SL_IEDECL resource_error :
    public std::runtime_error
{
    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Default constructs a resource_error.
        /**
         * The default message is "Thread resource error encountered."  The
         * message is what will be returned by the what method.
         */
        resource_error(
            ) throw() :
                runtime_error("Thread resource error encountered.")
        {
        } // resource_error::resource_error

        /// Constructs a resource_error with message \a msg.
        /**
         * \param msg The message to be returned by this' what method.
         */
        resource_error(
            const std::string& msg
            ) throw() :
                runtime_error(msg)
        {
        } // resource_error::resource_error

}; // class resource_error


/// This error represents a failure in program locking logic.
/**
 * An example of using this error properly would be throwing it when a
 * thread attempts to unlock a mutex that it doesn't hold.
 */
class EDDY_SL_IEDECL lock_error :
    public std::logic_error
{
    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Default constructs a lock_error.
        /**
         * The default message is "Thread lock-related error encountered."  The
         * message is what will be returned by the what method.
         */
        inline
        lock_error(
            ) throw() :
                logic_error("Thread lock-related error encountered.")
        {
        } // lock_error::lock_error

        /// Constructs a lock_error with message \a msg and \a err_code.
        /**
         * The error code is added into the message as additional text.
         *
         * \param msg The text to be returned by the what method.
         * \param err_code The error code returned by the failed mutex lock
         *                 operation.
         */
        lock_error(
            const std::string& msg,
            int err_code
            ) throw();

        /// Constructs a lock_error with message \a msg.
        /**
         * \param msg The text to be returned by the what method.
         */
        inline
        lock_error(
            const std::string& msg
            ) throw() :
                logic_error(msg)
        {
        } // lock_error::lock_error

}; // class lock_error


/// This error represents a failure in logic of a program.
/**
 * An example of using this error properly would be throwing it when an
 * invalid argument is passed into a thread function.
 */
class EDDY_SL_IEDECL logical_error :
    public std::logic_error
{
    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Default constructs a logical_error.
        /**
         * The default message is "Logical error in thread module."  The
         * message is what will be returned by the what method.
         */
        logical_error(
            ) throw() :
                logic_error("Logical error in thread module.")
        {
        } // logical_error::logical_error

        /// Constructs a logical_error with message \a msg.
        /**
         * \param msg The message to be returned by this' what method.
         */
        logical_error(
            const std::string& msg
            ) throw() :
                logic_error(msg)
        {
        } // logical_error::logical_error

}; // class resource_error

/*
================================================================================
End Namespace
================================================================================
*/
    } //  namespace threads
} // namespace eddy







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
// Not using an inlined functions file.



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_THREADS_THREAD_EXCEPTIONS_HPP

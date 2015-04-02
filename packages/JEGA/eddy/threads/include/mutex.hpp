/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Definition of class mutex.

    NOTES:

        This project is modeled heavily after the boost thread library.  Some
        Serious shortcommings of the boost thread library make this one
        necessary such as the lack of ability to cancel threads.

        Hopefully, the shortcommings of the boost library will be taken care of
        at which time this library will likely be changed to use it.

        Also, see notes under Class Definition section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Mar 01 18:11:58 2004 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the mutex class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_THREADS_MUTEX_HPP
#define EDDY_THREADS_MUTEX_HPP







/*
================================================================================
Includes
================================================================================
*/
// config.hpp should be the first include.
#include "../include/config.hpp"

#include <pthread.h>
#include "thread_exceptions.hpp"






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
class mutex;







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
/// This class serves as a mutex type for this project.
/**
 * The guts of this class type is a pthread_mutex_t member.  The interface
 * allows locking and unlocking of the mutex and for friends, casting to a
 * pthread_mutex_t.
 */
class EDDY_SL_IEDECL mutex
{
    /*
    ============================================================================
    Friend Declarations
    ============================================================================
    */
    // The condition is a friend so that it can get at the pthread mutex
    // conversion operator needed for the pthread condition function.
    friend class condition;

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The mutex wrapped by this class object.
        mutable pthread_mutex_t _mutex;


    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:


        /// Locks this mutex.
        /**
         * This method may throw a lock_error if the mutex cannot be locked.
         */
        inline
        void
        lock(
            ) const;

        /// Unlocks this mutex.
        /**
         * This method may throw a lock_error if the mutex cannot be unlocked.
         */
        inline
        void
        unlock(
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


        /// Allows conversion of this mutex into a pthread_mutex_t
        /**
         * \return The underlying pthread mutex of this mutex object.
         */
        inline
        operator pthread_mutex_t (
            ) const;

        /// Allows conversion of this mutex into a pthread_mutex_t*
        /**
         * \return A pointer to the underlying pthread mutex of this mutex
         *         object.
         */
        inline
        operator pthread_mutex_t* (
            ) const;



    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:


        /// Constructs a mutex of the specified type.
        /**
         * \param type The type of mutex being created.  One of:
         * \verbatim
            PTHREAD_MUTEX_NORMAL - a normal mutex cannot be locked recursively
                                   nor can it be re-locked if a thread failed
                                   to unlock it before exiting.  Attempts at
                                   these causes deadlock.
            PTHREAD_MUTEX_RECURSIVE - a recursive mutex can be locked
                                      repeatedly by the owner and becomes
                                      unlocked when the owner unlocks it as
                                      many times as it locked it.
            PTHREAD_MUTEX_ERRORCHECK - an errorcheck mutex checks for deadlock
                                       conditions that occur when a
                                       thread re-locks an already held mutex.
                                       If a thread attempts to relock a mutex
                                       that it already holds, the lock request
                                       fails with the EDEADLK error.
            PTHREAD_MUTEX_DEFAULT = PTHREAD_MUTEX_NORMAL
           \endverbatim
         */
        mutex(
            int type = PTHREAD_MUTEX_DEFAULT
            );


        /// Destructs a mutex.
        /**
         * Destruction causes destruction of the underlying pthread mutex.
         */
        ~mutex(
            ) throw();


}; // class mutex



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
#include "./inline/mutex.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_THREADS_MUTEX_HPP

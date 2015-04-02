/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Definition of class mutex_lock.

    NOTES:

        See notes under Class Definition section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Sat Sep 13 14:02:10 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the mutex_lock class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_THREADS_MUTEX_LOCK_HPP
#define EDDY_THREADS_MUTEX_LOCK_HPP







/*
================================================================================
Includes
================================================================================
*/
// config.hpp should be the first include.
#include "../include/config.hpp"

#include "mutex.hpp"
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
class mutex_lock;







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
/// This is meant to provide scoped locking of mutexes.
/**
 * The constructor of this class takes a mutex and immediately locks it.
 * The destructor unlocks that mutex.
 */
class EDDY_SL_IEDECL mutex_lock
{
    /*
    ============================================================================
    Friend Declarations
    ============================================================================
    */
    friend class condition;


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:


        /// A pointer to the mutex to lock.
        mutex* _mutex;



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

        /// Locks the mutex held in this lock.
        inline
        void
        lock(
            );

        /// Unlocks the mutex held in this lock.
        inline
        void
        unlock(
            );

        /// Allows access to the mutex held in this lock to friends.
        /**
         * Only friends can get at this mutex and they should not lock or
         * unlock it unless they really know what they are doing since this
         * object will attempt to unlock it in the destructor.  There is the
         * potential for an unbalance of lock and unlock calls.
         *
         * \return A reference to the mutex that is locked by this lock object.
         */
        inline
        mutex&
        get_mutex(
            );

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:


        /// Constructs a mutex_lock
        /**
         * Construction causes locking of the supplied \a mutex.
         *
         * \param mutex The mutext to lock in this constructor and unlock in
         *              the destructor.
         * \throw lock_error If \a mutex cannot be locked.
         */
        inline
        mutex_lock(
            mutex& mutex
            );

        /// Destructs a mutex_lock
        /**
         * Destruction causes unlocking of _mutex.
         */
        inline
        ~mutex_lock(
            ) throw();



}; // class mutex_lock



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
#include "./inline/mutex_lock.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_THREADS_MUTEX_LOCK_HPP

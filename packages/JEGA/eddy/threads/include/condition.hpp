/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Definition of class condition.

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

        Mon Mar 01 18:50:46 2004 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the condition class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_THREADS_CONDITION_HPP
#define EDDY_THREADS_CONDITION_HPP







/*
================================================================================
Includes
================================================================================
*/
// config.hpp should be the first include.
#include "../include/config.hpp"

#include <pthread.h>







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
class condition;
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
/// A condition that may be wait on or broadcast for thread notification.
/**
 * Conditions are primarily used as something on which a thread would wait.
 * For example, a processing thread may wait for a condition set in response
 * to a user input instructing it to resume.
 *
 * This class is implemented by keeping a pthread_cond_t member and using
 * the usual posix pthread functions.
 */
class EDDY_SL_IEDECL condition
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The underlying pthread condition variable.
        pthread_cond_t _condition;




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


        /**
         * \brief Causes the calling thread to wait on this condition using the
         *        mutex in lock.
         *
         * The argument is a lock because this method must have a locked mutex.
         * It will unlock the mutex and then relock it before returning.
         *
         * \param lock The mutex_lock with a locked mutex for this condition to
         *             wait on.
         * \throws lock_error if the wait fails for some reason.
         */
        void
        wait(
            mutex_lock& lock
            );

        /**
         * \brief Causes the calling thread to wait on this condition using the
         * mutex in lock for no more than \a msecs milliseconds.
         *
         * The argument is a lock because this method must have a locked mutex.
         * It will unlock the mutex and then relock it before returning.
         *
         * \param lock The mutex_lock with a locked mutex for this condition to
         *             wait on.
         * \param msecs The maximum time in milliseconds for which to wait.
         * \return true if the condition was signaled and false if the time
         *         limit was reached.
         * \throws lock_error if the wait fails for some reason.
         */
        bool
        timed_wait(
            mutex_lock& lock,
            unsigned int msecs
            );

        /// Signals a thread waiting on this condition to awake.
        /**
         * If the signaling fails, this method throws a lock_error.
         */
        void
        notify_one(
            );

        /// Signals all threads waiting on this condition to awake.
        /**
         * If the signaling fails, this method throws a lock_error.
         */
        void
        notify_all(
            );


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


        /// Constructs a condition of the specified type.
        /**
         * \param sharing The mode of sharing of the condition being created.
         *                One of:
         * \verbatim
            PTHREAD_PROCESS_PRIVATE - a private process condition is one that
                                      can be only be used by threads in the
                                      process in which it was created.
            PTHREAD_PROCESS_SHARED - a shared process condition is one that can
                                     be used by threads in processes other than
                                     the one in which it was created.
           \endverbatim
         *
         * This constructor may throw a resource_error.
         */
        condition(
            int sharing = PTHREAD_PROCESS_PRIVATE
            );

        /// Destructs a condition.
        /**
         * Destruction causes destruction of the underlying pthread condition.
         */
        ~condition(
            ) throw();


}; // class condition



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
#include "./inline/condition.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_THREADS_CONDITION_HPP

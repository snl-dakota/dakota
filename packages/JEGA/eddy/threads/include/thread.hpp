/*
================================================================================
    PROJECT:

        Eddy C++ Thread Safety Project

    CONTENTS:

        Definition of class thread.

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

        Mon Mar 01 15:39:24 2004 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the thread class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_THREADS_THREAD_HPP
#define EDDY_THREADS_THREAD_HPP







/*
================================================================================
Includes
================================================================================
*/
// config.hpp should be the first include.
#include "../include/config.hpp"

#include <map>
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
class thread;







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
/// A class that is a thread of execution.
/**
 * This class is implemented using the pthread_t as a primary type.
 *
 * A thread object exists independently of the actual thread of execution
 * in much the way a file stream exists independently of a file.
 *
 * It can be used as a means of managing a particular thread of exectution.
 * It provides a useful interface ...
 */
class EDDY_SL_IEDECL thread
{

    /*
    ============================================================================
    Typedefs
    ============================================================================
    */
    public:

        /**
         * \brief The type of function that must be used as the main function
         *        for a thread.
         */
        typedef
        void*
        (*main_func)(
            void*
            );

    protected:


    private:

        /// The map type that relates old cancel states to pthread_t 's.
        typedef
        std::map<pthread_t*, int>
        old_cancel_state_map;

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:


        /// The actual thread represented by this.
        pthread_t _thread;

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

        /// The previous cancel states for all created pthread_t 's
        /**
         * Initially, these take on the default value for the cancel state of
         * PTHREAD_CANCEL_DEFERRED.
         *
         * \return The map of threads to previous or "old" cancel states.
         */
        static
        old_cancel_state_map&
        old_cancel_types(
            );

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:


        /**
         * \brief Causes the calling thread to halt execution for the specified
         *        number of milliseconds.
         *
         * \param msecs The number of milliseconds for this thread to sleep.
         */
        static
        void
        sleep(
            unsigned int msecs
            );

        /**
         * \brief Causes the calling thread to forfeit the remainder of its
         *        processing chunk so that another thread may execute.
         */
        static
        void
        yield(
            );

        /// Returns true if the calling thread is the thread represented here.
        /**
         * \return True if the calling thread is the thread represented by this
         *         thread object and false otherwise.
         */
        bool
        is_this_thread(
            ) const;

        /**
         * \brief Causes the attached thread to be canceled at the next
         *        cancelation point.
         */
        void
        cancel(
            );

        /// Sets the current thread cancelation type to asynchronous.
        /**
         * This does not cancel the thread.  Only sets the cancelization type.
         * The cancelation type prior to this call can be restored by calling
         * restore_cancelation_state.
         */
        static
        void
        set_asynch_cancelation(
            );

        /// Sets the current thread cancelation type to deferred.
        /**
         * This does not cancel the thread.  Only sets the cancelization type.
         * The cancelation type prior to this call can be restored by calling
         * restore_cancelation_state.
         */
        static
        void
        set_deferred_cancelation(
            );

        /// Sets the current thread cancelation type to disabled.
        /**
         * This does not cancel the thread.  Only sets the cancelization type.
         * The cancelation type prior to this call can be restored by calling
         * restore_cancelation_state.
         */
        static
        void
        disable_cancelation(
            );

        /// Sets the current thread cancelation type to enable.
        /**
         * This does not cancel the thread.  Only sets the cancelization type.
         * The cancelation type prior to this call can be restored by calling
         * restore_cancelation_state.
         */
        static
        void
        enable_cancelation(
            );

        /**
         * \brief Restores the thread cancelization state to what it was prior
         *        to a call to set_deferred_cancelation, disable_cancelation,
         *        set_asynch_cancelation, etc.
         *
         * This only keeps one previous state and so repeated calls without
         * calls to the above methods will result in state toggling.
         */
        static
        void
        restore_cancelation_state(
            );

        /// Returns true if \a other wraps the same thread as this.
        /**
         * \param other The other thread to test for equality to this.
         * \return True of \a other wraps the same thread as this and false
         *         otherwise.
         */
        bool
        operator ==(
            const thread& other
            ) const;


        /**
         * \brief Causes this thread to represent the same thread as that of
         *        \a other.
         *
         * \param other The thread object representing the pthread that this is
         *              now to represent.
         * \return This thread after modification.
         */
        const thread&
        operator =(
            const thread& other
            );

        /// Returns true if \a other does not wrap the same thread as this.
        /**
         * \param other The thread to test for equality to this.
         * \return True if other wraps the same thread as this and false
         *         otherwise.
         */
        bool
        operator !=(
            const thread& other
            ) const;

        /// Waits for the supplied thread to join the calling thread.
        /**
         * This is useful if you have a reference to a thread and would like
         * the current thread of execution to join it.
         *
         * \param other The existing thread to join with the calling thread.
         */
        static
        void
        join(
            thread& other
            );

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:


        /// Sets the cancellation type of the calling thread to \a state.
        /**
         * \param state The new cancellation type of the calling thread.
         */
        static
        void
        set_cancelation(
            int state
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


        /// Default constructs a thread object.
        /**
         * This causes the thread object to use the current thread as the
         * wrapped thread.
         */
        thread(
            );

        /// Copy constructs a thread object.
        /**
         * \param copy The thread object whose underlying thread is to be used
         *             by this thread object.
         */
        thread(
            const thread& copy
            );

        /**
         * \brief Constructs a thread object with a new thread using \a func as
         *        the main.
         *
         * This version creates a new thread and uses the passed in function
         * as the main function for the thread.
         *
         * \param func The function to be used as the main function of the
         *             newly created thread.
         * \param data A pointer to the data to be used by the new thread.
         * \param detachstate The detach state for the newly created pthread.
         * \param inheritsched The scheduling parameter for the newly created
         *        pthread.
         */
        thread(
            main_func func,
            void* data = 0,
            int detachstate = PTHREAD_CREATE_JOINABLE,
            int inheritsched = PTHREAD_INHERIT_SCHED
            );

        /// Destructs a thread object.
        /**
         * This does not do anything to the underlying thread.  It exists when
         * its main function returns.
         */
        ~thread(
            ) throw();


}; // class thread



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
#include "./inline/thread.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_THREADS_THREAD_HPP
